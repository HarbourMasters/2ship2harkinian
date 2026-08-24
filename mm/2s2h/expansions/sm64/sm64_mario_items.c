/*
 * Mario Mode power-ups, Fire Flower projectiles and Cappy for 2 Ship 2 Harkinian.
 * Included by z_player.c after sm64_mario.c.
 */

#include "expansions/sm64/sm64_mario.h"
#include "overlays/actors/ovl_En_Bom/z_en_bom.h"
#include "overlays/actors/ovl_En_Boom/z_en_boom.h"
#include "overlays/actors/ovl_En_Arrow/z_en_arrow.h"
#include "objects/gameplay_keep/gameplay_keep.h"
#include <math.h>
#include <string.h>
// Mario-mode power-up timer / cooldown system.
//
// Four D-pad power-ups, each with a USE timer (activeDur) and a TIMEOUT
// (cooldown). All durations are in OOT frames (the game runs at 60 fps). The
// cooldown length scales with how long the cap was ACTUALLY used:
//
//     cooldown = (framesUsed / activeDur) * maxCooldown
//
// so a half-used Metal Cap (30s of its 60s) cools down for half of 3:00 = 1:30.
// Used to full → full cooldown; switched on then immediately off → 0 cooldown.
//
// THIS module is the source of truth: libsm64's own capTimer is driven to its
// max on activate, and the special-cap flags are cleared via sm64_set_mario_state
// on deactivate, so this timer — not libsm64 — decides when a cap ends.
// Everything advances only while Mario mode is genuinely active (Sm64MarioCaps_Tick
// runs from the normal path of Sm64Mario_HandleItems), so the timers FREEZE
// during loading suspends, cutscenes, and while Mario mode is toggled off, and
// resume where they left off — matching the "freeze, don't burn real time" rule.
//
// Only ONE cap is ACTIVE at a time. Pressing the active cap again toggles it
// off; pressing a different cap switches (old → proportional cooldown, new →
// active if READY). Pressing a recharging cap is rejected. Mario-mode-end and
// scene-change route through Sm64MarioCaps_OnSuspend → the active cap drops to
// its proportional cooldown (state persists, frozen, until Mario mode resumes).
// =============================================================================

// Slot / panel order (top→bottom in the corner HUD): Wing, Metal, Vanish, Fire.
#define SM64_CAP_SLOT_WING 0
#define SM64_CAP_SLOT_METAL 1
#define SM64_CAP_SLOT_VANISH 2
#define SM64_CAP_SLOT_FIRE 3
#define SM64_CAP_SLOT_COUNT 4

typedef struct {
    u16 btn;         // D-pad bind
    u32 capFlag;     // libsm64 cap flag; 0 = stub (Fire Flower — timer only, no effect yet)
    s32 activeDur;   // frames of use at full duration (60 fps)
    s32 maxCooldown; // frames of cooldown after full use (60 fps)
    s32 sfx;
    const char* name;
} Sm64CapDef;

// Balance (60 fps): Wing 30s/30s, Metal 60s/180s, Vanish 30s/15s, Fire 60s/90s.
// Ordered to match SM64_CAP_SLOT_* above (index == slot).
static const Sm64CapDef kCapDefs[SM64_CAP_SLOT_COUNT] = {
    { BTN_DDOWN, SM64_MARIO_WING_CAP, 30 * 60, 30 * 60, NA_SE_PL_MAGIC_WIND_NORMAL, "Wing" },
    { BTN_DLEFT, SM64_MARIO_METAL_CAP, 60 * 60, 180 * 60, NA_SE_PL_MAGIC_SOUL_NORMAL, "Metal" },
    { BTN_DRIGHT, SM64_MARIO_VANISH_CAP, 30 * 60, 15 * 60, NA_SE_PL_MAGIC_FIRE, "Vanish" },
    { BTN_DUP, 0, 60 * 60, 90 * 60, NA_SE_PL_MAGIC_FIRE, "Fire" },
};

typedef struct {
    u8 phase;        // SM64_CAP_PHASE_*
    s32 elapsed;     // frames elapsed in the current phase
    s32 cooldownDur; // proportional cooldown (frames) computed when COOLDOWN entered
} Sm64CapState;

static Sm64CapState sCapStates[SM64_CAP_SLOT_COUNT];
static s32 sActiveCap = -1; // index of the ACTIVE cap, or -1
static u8 sCapStatesInited = 0;

static void Sm64Caps_EnsureInit(void) {
    if (sCapStatesInited)
        return;
    for (s32 i = 0; i < SM64_CAP_SLOT_COUNT; i++) {
        sCapStates[i].phase = SM64_CAP_PHASE_READY;
        sCapStates[i].elapsed = 0;
        sCapStates[i].cooldownDur = 0;
    }
    sActiveCap = -1;
    sCapStatesInited = 1;
}

// Clear the special-cap flags in libsm64 and restore Mario's normal red cap, so
// the special-cap effect ends immediately WITHOUT the cap-on SFX/anim that a
// re-call to interact_cap would play. No-op if the Mario instance is gone (a
// scene change already deleted it; the recreated Mario starts cap-less).
static void Sm64Caps_ClearLibsm64Cap(void) {
    if (sSm64MarioId < 0 || !p_sm64_set_mario_state)
        return;
    u32 f = sSm64OutState.flags;
    f &= ~(SM64_MARIO_VANISH_CAP | SM64_MARIO_METAL_CAP | SM64_MARIO_WING_CAP);
    f |= SM64_MARIO_NORMAL_CAP | SM64_MARIO_CAP_ON_HEAD;
    p_sm64_set_mario_state(sSm64MarioId, f);
}

// Move the active cap into its proportional cooldown. clearLib removes the
// libsm64 cap effect (skip it when the Mario instance is being torn down).
static void Sm64Caps_DeactivateActive(u8 clearLib) {
    if (sActiveCap < 0)
        return;
    s32 idx = sActiveCap;
    Sm64CapState* s = &sCapStates[idx];
    const Sm64CapDef* d = &kCapDefs[idx];

    s32 used = s->elapsed;
    if (used > d->activeDur)
        used = d->activeDur;
    s32 cd = (s32)(((f32)used / (f32)d->activeDur) * (f32)d->maxCooldown);

    if (clearLib && d->capFlag != 0) {
        Sm64Caps_ClearLibsm64Cap();
    }

    // Stop the SM64 cap jingle. interact_cap(playMusic=1) plays it on
    // SEQ_PLAYER_LEVEL and tracks it as the current background music; because
    // we end the cap ourselves (set_mario_state) instead of letting libsm64's
    // capTimer hit 0, libsm64 never runs stop_cap_music — so without this the
    // jingle loops forever after the power-up ends / is disabled / on scene
    // change. Mirrors stop_cap_music: stop the current background seq. Runs on
    // every deactivation path (auto-expire, toggle, switch, suspend).
    if ((d->capFlag != 0 || idx == SM64_CAP_SLOT_FIRE) && p_sm64_stop_background_music &&
        p_sm64_get_current_background_music) {
        uint16_t cur = p_sm64_get_current_background_music();
        if (cur != 0) {
            p_sm64_stop_background_music(cur);
        }
    }

    s->elapsed = 0;
    if (cd <= 0) {
        s->phase = SM64_CAP_PHASE_READY;
        s->cooldownDur = 0;
    } else {
        s->phase = SM64_CAP_PHASE_COOLDOWN;
        s->cooldownDur = cd;
    }
    sActiveCap = -1;
}

static void Sm64Caps_Activate(s32 idx) {
    const Sm64CapDef* d = &kCapDefs[idx];
    Sm64CapState* s = &sCapStates[idx];

    s->phase = SM64_CAP_PHASE_ACTIVE;
    s->elapsed = 0;
    sActiveCap = idx;

    if (d->capFlag != 0 && p_sm64_mario_interact_cap && sSm64MarioId >= 0) {
        // Drive libsm64's capTimer to its uint16 max so it never auto-expires
        // first — this module removes the cap when the use timer ends. The
        // interact_cap call clears any previous special cap and plays the
        // cap-on anim/SFX itself.
        p_sm64_mario_interact_cap(sSm64MarioId, d->capFlag, 0xFFFF, 1);
        // Cap-on pose, grounded-only (don't snap a mid-air Mario to a stand).
        if (p_sm64_set_mario_action && !(sSm64OutState.action & SM64_ACT_FLAG_AIR)) {
            p_sm64_set_mario_action(sSm64MarioId, SM64_ACT_PUTTING_ON_CAP);
        }
    } else if (idx == SM64_CAP_SLOT_FIRE && sSm64MarioId >= 0) {
        // Fire cap has no libsm64 cap flag, so interact_cap (and its jingle)
        // never runs — play the SAME POWERUP cap music the Wing/Vanish caps use
        // (SEQUENCE_ARGS(4, SEQ_EVENT_POWERUP) = 0x040E on SEQ_PLAYER_LEVEL=0)
        // and do the cap-on pose so it feels like a real cap.
        if (p_sm64_play_music) {
            p_sm64_play_music(0, 0x040E, 0);
        }
        if (p_sm64_set_mario_action && !(sSm64OutState.action & SM64_ACT_FLAG_AIR)) {
            p_sm64_set_mario_action(sSm64MarioId, SM64_ACT_PUTTING_ON_CAP);
        }
    }
    Audio_PlaySfx(d->sfx);
    lusprintf(__FILE__, __LINE__, 2, "[SM64Caps] activate %s (flag 0x%X)", d->name, d->capFlag);
}

// Handle a D-pad press for slot idx.
static void Sm64Caps_Press(s32 idx) {
    Sm64Caps_EnsureInit();
    Sm64CapState* s = &sCapStates[idx];

    if (s->phase == SM64_CAP_PHASE_ACTIVE) {
        // Toggle off → proportional cooldown.
        Sm64Caps_DeactivateActive(1);
        return;
    }
    if (s->phase == SM64_CAP_PHASE_COOLDOWN) {
        // Recharging — reject (matches the design's disabled slot).
        Audio_PlaySfx(NA_SE_SY_ERROR);
        return;
    }
    // READY → switch the currently-active cap into cooldown, then activate this.
    if (sActiveCap >= 0 && sActiveCap != idx) {
        Sm64Caps_DeactivateActive(1);
    }
    Sm64Caps_Activate(idx);
}

// Per-frame tick — advances the active use timer and every cooling slot.
void Sm64MarioCaps_Tick(void) {
    Sm64Caps_EnsureInit();
    for (s32 i = 0; i < SM64_CAP_SLOT_COUNT; i++) {
        Sm64CapState* s = &sCapStates[i];
        const Sm64CapDef* d = &kCapDefs[i];
        if (s->phase == SM64_CAP_PHASE_ACTIVE && i == sActiveCap) {
            s->elapsed++;
            if (s->elapsed >= d->activeDur) {
                // Full use → full cooldown (DeactivateActive clamps used = activeDur).
                Sm64Caps_DeactivateActive(1);
            }
        } else if (s->phase == SM64_CAP_PHASE_COOLDOWN) {
            s->elapsed++;
            if (s->elapsed >= s->cooldownDur) {
                s->phase = SM64_CAP_PHASE_READY;
                s->elapsed = 0;
                s->cooldownDur = 0;
            }
        }
    }
}

// Mario-mode end / scene change: drop the active cap to its proportional
// cooldown. Does NOT clear the persistent per-cap timer state (it stays frozen
// until Mario mode resumes). clearLib=0 — the Mario instance is being deleted.
void Sm64MarioCaps_OnSuspend(void) {
    Sm64Caps_EnsureInit();
    Sm64Caps_DeactivateActive(0);
    Sm64Mario_KillAllFireballs(); // don't leave fire colliders/balls frozen mid-flight
    Sm64Cappy_Kill();             // recall the thrown cap on detransform/scene change
}

// --- HUD read accessors -----------------------------------------------------

u8 Sm64MarioCaps_GetPhase(s32 idx) {
    if (idx < 0 || idx >= SM64_CAP_SLOT_COUNT)
        return SM64_CAP_PHASE_READY;
    Sm64Caps_EnsureInit();
    return sCapStates[idx].phase;
}

// Charge 0..1: ACTIVE drains 1→0, COOLDOWN fills 0→1, READY = 1.
f32 Sm64MarioCaps_GetCharge(s32 idx) {
    if (idx < 0 || idx >= SM64_CAP_SLOT_COUNT)
        return 1.0f;
    Sm64Caps_EnsureInit();
    Sm64CapState* s = &sCapStates[idx];
    const Sm64CapDef* d = &kCapDefs[idx];
    if (s->phase == SM64_CAP_PHASE_ACTIVE) {
        f32 c = 1.0f - (f32)s->elapsed / (f32)d->activeDur;
        return c < 0.0f ? 0.0f : c;
    }
    if (s->phase == SM64_CAP_PHASE_COOLDOWN) {
        if (s->cooldownDur <= 0)
            return 1.0f;
        f32 c = (f32)s->elapsed / (f32)s->cooldownDur;
        return c > 1.0f ? 1.0f : c;
    }
    return 1.0f;
}

// Whole seconds remaining in the ACTIVE or COOLDOWN phase (0 when READY).
s32 Sm64MarioCaps_GetRemainingSeconds(s32 idx) {
    if (idx < 0 || idx >= SM64_CAP_SLOT_COUNT)
        return 0;
    Sm64Caps_EnsureInit();
    Sm64CapState* s = &sCapStates[idx];
    const Sm64CapDef* d = &kCapDefs[idx];
    s32 rem;
    if (s->phase == SM64_CAP_PHASE_ACTIVE) {
        rem = d->activeDur - s->elapsed;
    } else if (s->phase == SM64_CAP_PHASE_COOLDOWN) {
        rem = s->cooldownDur - s->elapsed;
    } else {
        return 0;
    }
    if (rem < 0)
        rem = 0;
    return (rem + 59) / 60; // ceil to whole seconds
}

s32 Sm64MarioCaps_GetActiveIndex(void) {
    Sm64Caps_EnsureInit();
    return sActiveCap;
}

// True while the Fire cap (slot 3, D-Up) is the active cap. Fire has no libsm64
// cap flag (capFlag=0) — it's a pure OOT-side cap (classic recolor + B-fireball),
// so the renderer and the fire handler query this instead of sSm64OutState.flags.
u8 Sm64MarioCaps_IsFireActive(void) {
    Sm64Caps_EnsureInit();
    return (sActiveCap == SM64_CAP_SLOT_FIRE);
}

// =============================================================================
// Fire Flower fireballs — classic SMB-style bouncing projectiles. Thrown forward
// on a B press while the Fire cap is active; each ball arcs with gravity, bounces
// off floors a few times, and carries a fire AT collider (DMG_FIRE_ARROW → lights
// torches, burns cobwebs, hurts enemies). A flame VFX rides the ball. Self-
// contained fixed pool; updated every frame (Sm64Mario_UpdateFireballs) so balls
// already in flight finish even after the cap toggles off. B is NOT suppressed at
// the input level any more — Mario still punches; the fireball is an extra.
// =============================================================================
#define MARIO_FB_MAX 6
#define MARIO_FB_GRAVITY 1.5f    // per-frame downward accel (Triforce-drop feel)
#define MARIO_FB_FWD_SPEED 10.0f // forward launch speed
#define MARIO_FB_UP_SPEED 7.0f   // initial upward kick (gives the first arc)
#define MARIO_FB_BOUNCE 0.78f    // Y restitution on floor hit — bouncy, keeps popping
#define MARIO_FB_HFRICTION 0.98f // horizontal speed kept per bounce (ice-slide → travels far)
#define MARIO_FB_LIFE 150        // max frames alive (long enough for several bounces)
#define MARIO_FB_MAX_BOUNCE 8    // despawn after this many floor bounces

typedef struct {
    u8 active;
    u8 colInited;
    s16 life;
    u8 bounces;
    s16 fxScroll; // flame texture-scroll / flicker phase
    Vec3f pos;
    Vec3f vel;
    ColliderCylinder col;
} MarioFireball;

static MarioFireball sMarioFireballs[MARIO_FB_MAX];
static s16 sMarioFireballCooldown = 0;

// Boss super-damage GRACE window. Bosses that key off a collider hit (BUMP_HIT,
// e.g. King Dodongo) read that flag ONE frame after the fireball's AT set it —
// but actors update in category order (PLAYER=2 before BOSS=9), so on that next
// frame the fireball (updated inside the Player update) sees its own AT_HIT and
// bursts/clears `active` BEFORE the boss updates. Result: BossSuperDamage_IsActive()
// was already false when the boss looked, so the super hit was silently dropped —
// the "fire sometimes doesn't damage the boss" bug survived even after the dmgFlags
// fix made the hit register. This grace keeps FireballActive()/FireballNear()
// reporting true for a few frames AT THE IMPACT POINT so the boss's deferred read
// still sees the fire as active.
static s16 sFireGraceTimer = 0;
static Vec3f sFireGracePos = { 0.0f, 0.0f, 0.0f };
#define MARIO_FB_GRACE 5

// Fire AT collider — models sFireRodProjColInit (item_rod_fire.h): player-owned
// AT, fire hit effect (0x01). Toucher deals DMG_FIRE_ARROW | DMG_SWORD:
//   - DMG_FIRE_ARROW keeps the fire reactions (torches light, cobwebs/enemies burn).
//   - DMG_SWORD makes bosses REGISTER the hit (set BUMP_HIT). The boss-super-
//     damage rework keys off BUMP_HIT (any accepted hit) + IsActive, NOT the
//     vanilla damage value — exactly how FD's sword reliably triggers it. With
//     DMG_FIRE_ARROW alone the hit never registered on the (fire-immune) bosses, so
//     the super-damage path never ran — that was the "fire sometimes doesn't
//     damage the boss" inconsistency. DMG_SWORD is accepted by every boss's AC
//     bumper (you can always sword them), so the fireball now lands on all of
//     them. Radius bumped 13→22 so a thrown ball reliably overlaps a big boss.
// Sunlight-arrow flags on the toucher (Skijer's NEI):
//   - DMG_LIGHT_ARROW (1<<0xD): light-arrow damage — undead (ReDead/Gibdo) are weak
//     to it, and it activates the SunlightArrows-mode sun switch collider (0x00202000).
//   - DMG_LIGHT_RAY (1<<0x15): the mirror-ray flag the VANILLA Obj_Lightswitch bumper
//     accepts (0x00200000). Adding it lets the fireball toggle sun switches even WITHOUT
//     the SunlightArrows enhancement (both bumper variants include DMG_LIGHT_RAY). Normal
//     enemies ignore DMG_LIGHT_RAY (their DMG_DEFAULT bumpers exclude it), so no side effect
//     beyond the sun switches.
static ColliderCylinderInit sMarioFireballColInit = {
    { COL_MATERIAL_NONE, AT_ON | AT_TYPE_PLAYER, AC_NONE, OC1_NONE, OC2_NONE, COLSHAPE_CYLINDER },
    { ELEM_MATERIAL_UNK2,
      { DMG_FIRE_ARROW | DMG_SWORD | DMG_LIGHT_ARROW | DMG_LIGHT_RAY, 0x01, 8 },
      { 0, 0, 0 },
      ATELEM_ON | ATELEM_SFX_NORMAL,
      ACELEM_NONE,
      OCELEM_NONE },
    { 22, 30, -6, { 0, 0, 0 } }
};

static void Sm64Fireball_Kill(PlayState* play, MarioFireball* fb) {
    fb->active = 0;
    if (fb->colInited) {
        Collider_DestroyCylinder(play, &fb->col);
        fb->colInited = 0;
    }
}

// Free all in-flight fireballs and their colliders. Called on detransform /
// scene change / suspend so colliders never leak and the pool can't get stuck
// full of frozen balls when Mario mode flips off mid-throw. Uses gPlayState
// since the suspend hook carries no PlayState.
void Sm64Mario_KillAllFireballs(void) {
    s32 i;
    if (gPlayState == NULL)
        return;
    for (i = 0; i < MARIO_FB_MAX; i++) {
        if (sMarioFireballs[i].active || sMarioFireballs[i].colInited) {
            Sm64Fireball_Kill(gPlayState, &sMarioFireballs[i]);
        }
    }
}

// --- Boss super-damage hooks (read by boss_super_damage / transformation_masks) -
// A Fire Flower fireball in flight counts as an active "super attack", so the boss
// super-damage system treats fire like FD's slash and lets it break/kill bosses.
u8 Sm64Mario_FireballActive(void) {
    s32 i;
    // Grace window after a recent impact (see sFireGraceTimer) so a boss reading
    // BUMP_HIT one frame late still sees the fire as active.
    if (sFireGraceTimer > 0) {
        return 1;
    }
    for (i = 0; i < MARIO_FB_MAX; i++) {
        if (sMarioFireballs[i].active) {
            return 1;
        }
    }
    return 0;
}

// True if any in-flight fireball is within `range` (XYZ) of `pos`. Drives the
// geometric boss-super-damage reach test so a part breaks when fire touches it.
u8 Sm64Mario_FireballNear(Vec3f* pos, f32 range) {
    s32 i;
    if (pos == NULL) {
        return 0;
    }
    for (i = 0; i < MARIO_FB_MAX; i++) {
        if (sMarioFireballs[i].active) {
            f32 dx = sMarioFireballs[i].pos.x - pos->x;
            f32 dy = sMarioFireballs[i].pos.y - pos->y;
            f32 dz = sMarioFireballs[i].pos.z - pos->z;
            if ((dx * dx + dy * dy + dz * dz) < (range * range)) {
                return 1;
            }
        }
    }
    // Grace: a fireball impacted here within the last few frames (see
    // sFireGraceTimer) — let the boss's deferred BUMP_HIT read still land.
    if (sFireGraceTimer > 0) {
        f32 dx = sFireGracePos.x - pos->x;
        f32 dy = sFireGracePos.y - pos->y;
        f32 dz = sFireGracePos.z - pos->z;
        if ((dx * dx + dy * dy + dz * dz) < (range * range)) {
            return 1;
        }
    }
    return 0;
}

// Fire Flower: launch a bouncing fireball on a fresh B press (Fire cap active).
// Consumes the B press so the proximity grab / item handler don't fire on it,
// but the punch still happens because Mario's punch reads in->cur (not press).
void Sm64Mario_FireballOnBPress(PlayState* play, Player* player) {
    Input* in;
    MarioFireball* fb;
    s16 yaw;
    s32 i;

    if (play == NULL || player == NULL)
        return;
    if (sMarioFireballCooldown > 0)
        sMarioFireballCooldown--;
    in = &play->state.input[0];
    if (sMarioFireballCooldown > 0)
        return;
    if (!CHECK_BTN_ALL(in->press.button, BTN_B))
        return;
    in->press.button &= ~BTN_B; // consume so grab doesn't also fire (punch uses cur)

    // First free slot.
    fb = NULL;
    for (i = 0; i < MARIO_FB_MAX; i++) {
        if (!sMarioFireballs[i].active) {
            fb = &sMarioFireballs[i];
            break;
        }
    }
    if (fb == NULL)
        return; // pool full — drop this shot

    yaw = player->actor.shape.rot.y;
    if (!fb->colInited) {
        Collider_InitCylinder(play, &fb->col);
        Collider_SetCylinder(play, &fb->col, &player->actor, &sMarioFireballColInit);
        fb->colInited = 1;
    }
    fb->active = 1;
    fb->life = MARIO_FB_LIFE;
    fb->bounces = 0;
    fb->fxScroll = 0;
    fb->pos.x = player->actor.world.pos.x + Math_SinS(yaw) * 18.0f;
    fb->pos.y = player->actor.world.pos.y + 16.0f;
    fb->pos.z = player->actor.world.pos.z + Math_CosS(yaw) * 18.0f;
    fb->vel.x = Math_SinS(yaw) * MARIO_FB_FWD_SPEED;
    fb->vel.y = MARIO_FB_UP_SPEED;
    fb->vel.z = Math_CosS(yaw) * MARIO_FB_FWD_SPEED;

    sMarioFireballCooldown = 8; // min gap between shots
    Audio_PlaySfx(NA_SE_PL_MAGIC_FIRE);
}

// Sunlight-arrow effect applied to whatever a fireball hits (in addition to the
// fire + DMG_SWORD super-boss damage). Mirrors item_rod_light.c: undead (ReDeads,
// Gibdos, Poes, Stalchildren, hands…) are weak to sunlight and get the white "Sun's
// Song" paralysis; every other enemy gets a plain stun. SunlightArrows.cpp handles
// the OTHER half — the fireball is an AT_TYPE_PLAYER attack, so with that enhancement
// on it already activates Obj_Lightswitch sun switches (its AC is AC_TYPE_PLAYER).
#define MARIO_FB_SUNLIGHT_STUN 80

static u8 Sm64Fireball_IsUndead(Actor* actor) {
    switch (actor->id) {
        case ACTOR_EN_RD:         // ReDead / Gibdo
        case ACTOR_EN_POH:        // Poe
        case ACTOR_EN_PO_SISTERS: // Poe Sisters
        case ACTOR_EN_SKB:        // Stalchild
        case ACTOR_EN_WALLMAS:    // Wallmaster
        case ACTOR_EN_FLOORMAS:   // Floormaster
            return 1;
        default:
            return 0;
    }
}

static void Sm64Fireball_ApplySunlight(PlayState* play, Actor* hitActor) {
    if (hitActor == NULL || hitActor->update == NULL) {
        return;
    }
    if (hitActor->category != ACTORCAT_ENEMY && hitActor->category != ACTORCAT_BOSS) {
        return;
    }
    if (Sm64Fireball_IsUndead(hitActor)) {
        // White color filter (-0x8000 flag) = the Sun's Song / Gibdo sunlight paralysis.
        Actor_SetColorFilter(hitActor, -0x8000, 0xC8, 0, MARIO_FB_SUNLIGHT_STUN);
    } else {
        Actor_SetColorFilter(hitActor, 0, 0xFF, 0, MARIO_FB_SUNLIGHT_STUN);
    }
    hitActor->freezeTimer = MARIO_FB_SUNLIGHT_STUN;
    Actor_PlaySfx(hitActor, NA_SE_EN_LIGHT_ARROW_HIT);
}

// Advance every in-flight fireball one frame: gravity, integrate, floor bounce,
// fire collider, life/hit despawn. Called unconditionally each normal frame from
// Sm64Mario_Update so balls finish even after the Fire cap ends. The flame VFX is
// NOT an actor-attached particle (those follow Mario's yaw) — it's drawn directly
// at the ball in Sm64Mario_DrawFireballs so it reads as ONE moving fireball.
//
// Bounce physics mirror the Triforce drop (TriforceThief.cpp StepDropPhysics):
// gravity per frame, then a floor raycast shot from ABOVE max(prevY,newY) so a
// fast fall can't tunnel through the floor and fall forever; on contact the Y
// velocity reverses (restitution) while horizontal speed barely decays (ice-
// slide) so it keeps bouncing forward like a classic SMB fireball.
void Sm64Mario_UpdateFireballs(PlayState* play) {
    s32 i;

    if (play == NULL)
        return;

    // Tick down the boss super-damage grace once per frame (set on impact below).
    if (sFireGraceTimer > 0) {
        sFireGraceTimer--;
    }

    for (i = 0; i < MARIO_FB_MAX; i++) {
        MarioFireball* fb = &sMarioFireballs[i];
        CollisionPoly* poly;
        f32 floorY;
        f32 prevY;
        f32 queryTop;
        Vec3f queryPos;

        if (!fb->active)
            continue;

        fb->fxScroll++;

        // Hit registered by last frame's CollisionCheck_AT pass — burst + die.
        if (fb->colInited && (fb->col.base.atFlags & AT_HIT)) {
            Vec3f zero = { 0.0f, 0.0f, 0.0f };
            fb->col.base.atFlags &= ~AT_HIT;
            Sm64Fireball_ApplySunlight(play, fb->col.base.at); // sunlight-arrow paralysis/stun on the hit actor
            EffectSsBomb2_SpawnLayered(play, &fb->pos, &zero, &zero, 10, 5);
            // Open the boss super-damage grace at the impact point so a boss that
            // reads its BUMP_HIT one frame later (after this ball is gone) still
            // sees the fire as active and applies the super hit.
            sFireGraceTimer = MARIO_FB_GRACE;
            sFireGracePos = fb->pos;
            Sm64Fireball_Kill(play, fb);
            continue;
        }

        // Gravity + integrate.
        prevY = fb->pos.y;
        fb->vel.y -= MARIO_FB_GRAVITY;
        fb->pos.x += fb->vel.x;
        fb->pos.y += fb->vel.y;
        fb->pos.z += fb->vel.z;

        // Floor bounce — raycast DOWN from above the swept span so a fast fall
        // can't start the ray below the floor (which returns BGCHECK_Y_MIN and
        // tunnels the ball through the world).
        queryTop = (prevY > fb->pos.y) ? prevY : fb->pos.y;
        queryPos.x = fb->pos.x;
        queryPos.y = queryTop + 20.0f;
        queryPos.z = fb->pos.z;
        poly = NULL;
        floorY = BgCheck_EntityRaycastFloor1(&play->colCtx, &poly, &queryPos);
        if (poly != NULL && floorY > BGCHECK_Y_MIN && fb->pos.y <= floorY + 3.0f) {
            fb->pos.y = floorY + 3.0f;
            if (fb->vel.y < 0.0f) {
                fb->vel.y = -fb->vel.y * MARIO_FB_BOUNCE;
            }
            fb->vel.x *= MARIO_FB_HFRICTION;
            fb->vel.z *= MARIO_FB_HFRICTION;
            fb->bounces++;
            Audio_PlaySfx(NA_SE_EV_FLAME_IGNITION);
            if (fb->bounces > MARIO_FB_MAX_BOUNCE) {
                Sm64Fireball_Kill(play, fb);
                continue;
            }
        }

        // Arm the fire AT collider at the ball this frame.
        if (fb->colInited) {
            fb->col.dim.pos.x = (s16)fb->pos.x;
            fb->col.dim.pos.y = (s16)fb->pos.y;
            fb->col.dim.pos.z = (s16)fb->pos.z;
            fb->col.base.atFlags |= AT_ON;
            CollisionCheck_SetAT(play, &play->colChkCtx, &fb->col.base);
        }

        if (--fb->life <= 0) {
            Sm64Fireball_Kill(play, fb);
        }
    }
}

// Draw one cohesive flame billboard per in-flight fireball, camera-facing, at the
// ball's absolute world position. Modeled on EffectSsEnFire_Draw (z_eff_ss_en_fire
// .c) but free-standing (no actor → never follows Mario's facing) so the fire
// tracks the ball's independent trajectory. Called from Sm64Mario_Draw.
void Sm64Mario_DrawFireballs(PlayState* play) {
    GraphicsContext* gfxCtx;
    s16 camYaw;
    s32 i;

    if (play == NULL)
        return;
    gfxCtx = play->state.gfxCtx;
    camYaw = Camera_GetCamDirYaw(GET_ACTIVE_CAM(play)) + 0x8000;

    OPEN_DISPS(gfxCtx);
    Gfx_SetupDL25_Xlu(gfxCtx);

    for (i = 0; i < MARIO_FB_MAX; i++) {
        MarioFireball* fb = &sMarioFireballs[i];
        f32 scale;

        if (!fb->active)
            continue;

        Matrix_Translate(fb->pos.x, fb->pos.y + 5.0f, fb->pos.z, MTXMODE_NEW);
        Matrix_RotateYF(camYaw * (3.14159265358979323846f / 0x8000), MTXMODE_APPLY);
        // Steady fireball size with a small flame flicker (always positive).
        scale = 0.0058f + Math_SinS(fb->fxScroll * 0x1500) * 0.0009f;
        Matrix_Scale(scale, scale, scale, MTXMODE_APPLY);
        gSPMatrix(POLY_XLU_DISP++, Matrix_Finalize(gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

        gDPSetEnvColor(POLY_XLU_DISP++, 255, 40, 0, 0);
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0x80, 255, 220, 0, 255);
        gSPSegment(POLY_XLU_DISP++, 0x08,
                   Gfx_TwoTexScrollEx(gfxCtx, 0, 0, 0, 0x20, 0x40, 1, 0, (fb->fxScroll * -0x14) & 0x1FF, 0x20, 0x80, 0,
                                      0, 0, -0x14));
        gSPDisplayList(POLY_XLU_DISP++, gEffFire1DL);
    }

    CLOSE_DISPS(gfxCtx);
}

// D-pad controls are captured at the very beginning of Player_Update, before
// MM/custom item dispatch can consume them. The pending press is handled later,
// after Mario has completed its normal update.
static u16 sSm64CapPress = 0;

static void Sm64Mario_CaptureCapInputs(PlayState* play) {
    Input* in;
    const u16 reserved = BTN_DUP | BTN_DDOWN | BTN_DLEFT | BTN_DRIGHT;

    if (play == NULL || !Sm64Mario_IsActive()) {
        sSm64CapPress = 0;
        return;
    }

    in = &play->state.input[0];
    sSm64CapPress = in->press.button & reserved;
    in->press.button &= ~reserved;
    in->cur.button &= ~reserved;
}

static void Sm64Mario_HandleCapDpad(PlayState* play) {
    u16 capPress;

    if (play == NULL) {
        return;
    }

    capPress = sSm64CapPress;
    sSm64CapPress = 0;
    for (s32 i = 0; i < SM64_CAP_SLOT_COUNT; i++) {
        if (!CHECK_BTN_ALL(capPress, kCapDefs[i].btn)) {
            continue;
        }
        Sm64Caps_Press(i);
        break;
    }
}
// =============================================================================
// Cappy — the thrown cap (Odyssey moveset). C-Left flings it; the throw VARIANT
// depends on input at the press:
//   • grounded, neutral          → FORWARD throw (flies out, hovers, returns)
//   • grounded, stick up          → UP throw (arcs up — vertical cap-jump setup)
//   • grounded, stick spun        → SPIN throw (orbits Mario, wide hit)
//   • airborne                    → DIVE throw (fast down-forward)
// Forward/Up/Dive home onto the nearest enemy in range. A boomerang-type AT
// collider stuns enemies the whole flight. While the cap hovers (or rises), if
// Mario falls onto it the host fires ACT_CAP_BOUNCE for the Odyssey jump boost.
// One cap at a time. Visual is a camera-facing spinning disc placeholder (the
// real tiara mesh from omm_tiara_geo.bin replaces it once integrated).
// =============================================================================
#define CAPPY_OUT_SPEED 12.0f // short throw → the cap hovers close & reachable
#define CAPPY_OUT_FRAMES 12   // ≈ 144 units forward, a quick jump away
#define CAPPY_HOVER_FRAMES 60 // long hover so the cap-jump window is reliable
#define CAPPY_RETURN_SPEED 30.0f
#define CAPPY_CATCH_DIST 26.0f
#define CAPPY_BOUNCE_XZ 46.0f     // generous landing radius for the cap-bounce
#define CAPPY_HOMING_RANGE 220.0f // only nudge toward CLOSE enemies (keeps it reachable)
#define CAPPY_ORBIT_FRAMES 30
#define CAPPY_ORBIT_RADIUS 78.0f

enum { CAPPY_OUT = 0, CAPPY_HOVER, CAPPY_RETURN, CAPPY_ORBIT };

typedef struct {
    u8 active;
    u8 phase;
    u8 mode; // SM64_CAPPY_*
    u8 colInited;
    u8 homing;
    u8 bounced; // cap-jump fired this throw (one-shot)
    s16 timer;
    s16 fxScroll;
    s16 yaw;
    s16 orbitAng;
    Vec3f pos;
    Vec3f vel;
    Actor* target; // homing target (nearest enemy), or NULL
    ColliderCylinder col;
} Cappy;

static Cappy sCappy;

// Boomerang-type AT (stuns enemies like a thrown object), no fire.
static ColliderCylinderInit sCappyColInit = { { COL_MATERIAL_NONE, AT_ON | AT_TYPE_PLAYER, AC_NONE, OC1_NONE, OC2_NONE,
                                                COLSHAPE_CYLINDER },
                                              { ELEM_MATERIAL_UNK2,
                                                { DMG_ZORA_BOOMERANG, 0x00, 0x08 },
                                                { 0, 0, 0 },
                                                ATELEM_ON | ATELEM_SFX_NORMAL,
                                                ACELEM_NONE,
                                                OCELEM_NONE },
                                              { 18, 28, -12, { 0, 0, 0 } } };

// Mario's red cap — extracted from the SM64 decomp (libsm64 model.inc.c) by
// apps/sm64_model_extract.py into mario_cap_model.c and #included here (rides this
// TU, no VS project change). Flat-lit (red dome + brown brim, no texture): the
// render sets the red light + a shade combiner; mario_cap_unused_base_dl draws it.
#include "expansions/sm64/mario_cap_model.c"

void Sm64Cappy_Kill(void) {
    if (sCappy.colInited && gPlayState != NULL) {
        Collider_DestroyCylinder(gPlayState, &sCappy.col);
        sCappy.colInited = 0;
    }
    // Cap caught → put it back on Mario's head.
    if (sCappy.active && sSm64MarioId >= 0 && p_sm64_set_mario_state) {
        u32 f = sSm64OutState.flags | SM64_MARIO_NORMAL_CAP | SM64_MARIO_CAP_ON_HEAD;
        p_sm64_set_mario_state(sSm64MarioId, f);
    }
    sCappy.active = 0;
    sCappy.target = NULL;
}

// Nearest living enemy within homing range (XZ distance from `from`).
static Actor* Sm64Cappy_FindTarget(PlayState* play, Vec3f* from) {
    Actor* a = play->actorCtx.actorLists[ACTORCAT_ENEMY].first;
    Actor* best = NULL;
    f32 bestD = CAPPY_HOMING_RANGE;
    while (a != NULL) {
        if (a->update != NULL) {
            f32 dx = a->world.pos.x - from->x;
            f32 dz = a->world.pos.z - from->z;
            f32 d = sqrtf(dx * dx + dz * dz);
            if (d < bestD) {
                bestD = d;
                best = a;
            }
        }
        a = a->next;
    }
    return best;
}

// Cap-jump: when Mario's body CROSSES the hovering cap — in ANY state (jump,
// dive, roll, freefall, even running into it) — force ACT_CAP_BOUNCE for the
// double-jump-height launch + flip. Pure 3D-overlap test (no velocity/air gate):
// set_mario_action overrides whatever he was doing. One-shot per throw, with a
// few frames of arm delay so the throw itself doesn't insta-bounce. Returns 1 if
// it fired (caller sends the cap home).
static u8 Sm64Cappy_TryBounce(PlayState* play, Vec3f* mpos, Player* player) {
    f32 dx, dy, dz;
    (void)player;
    if (sCappy.bounced || sCappy.fxScroll < 5)
        return 0;
    dx = mpos->x - sCappy.pos.x;
    dy = mpos->y - sCappy.pos.y;
    dz = mpos->z - sCappy.pos.z;
    if ((dx * dx + dz * dz) < (CAPPY_BOUNCE_XZ * CAPPY_BOUNCE_XZ) && dy > -54.0f && dy < 66.0f) {
        if (p_sm64_set_mario_action && sSm64MarioId >= 0) {
            p_sm64_set_mario_action(sSm64MarioId, SM64_ACT_CAP_BOUNCE);
        }
        Audio_PlaySfx(NA_SE_EV_BOMB_BOUND);
        sCappy.bounced = 1;
        sCappy.phase = CAPPY_RETURN;
        return 1;
    }
    return 0;
}

void Sm64Cappy_Throw(PlayState* play, s32 mode, u8 homing) {
    Player* player;
    s16 yaw;
    f32 fwd = CAPPY_OUT_SPEED;
    if (play == NULL)
        return;
    player = GET_PLAYER(play);

    if (!sCappy.colInited) {
        Collider_InitCylinder(play, &sCappy.col);
        Collider_SetCylinder(play, &sCappy.col, &player->actor, &sCappyColInit);
        sCappy.colInited = 1;
    }
    sCappy.active = 1;
    sCappy.mode = (u8)mode;
    sCappy.homing = homing;
    sCappy.bounced = 0;
    sCappy.fxScroll = 0;
    yaw = player->actor.shape.rot.y;
    sCappy.yaw = yaw;
    sCappy.target = homing ? Sm64Cappy_FindTarget(play, &player->actor.world.pos) : NULL;
    sCappy.pos.x = player->actor.world.pos.x + Math_SinS(yaw) * 18.0f;
    sCappy.pos.y = player->actor.world.pos.y + 24.0f;
    sCappy.pos.z = player->actor.world.pos.z + Math_CosS(yaw) * 18.0f;

    switch (mode) {
        case SM64_CAPPY_DIVE:
            sCappy.vel.x = Math_SinS(yaw) * fwd * 1.3f;
            sCappy.vel.z = Math_CosS(yaw) * fwd * 1.3f;
            sCappy.vel.y = -fwd * 0.5f;
            sCappy.phase = CAPPY_OUT;
            sCappy.timer = CAPPY_OUT_FRAMES;
            break;
        case SM64_CAPPY_SPIN:
            sCappy.orbitAng = yaw;
            sCappy.vel.x = sCappy.vel.y = sCappy.vel.z = 0.0f;
            sCappy.phase = CAPPY_ORBIT;
            sCappy.timer = CAPPY_ORBIT_FRAMES;
            break;
        default: // SM64_CAPPY_FWD
            sCappy.vel.x = Math_SinS(yaw) * fwd;
            sCappy.vel.z = Math_CosS(yaw) * fwd;
            sCappy.vel.y = 0.0f;
            sCappy.phase = CAPPY_OUT;
            sCappy.timer = CAPPY_OUT_FRAMES;
            break;
    }
    Audio_PlaySfx(NA_SE_IT_SWORD_SWING);
}

void Sm64Cappy_Update(PlayState* play) {
    Player* player;
    Vec3f mpos;
    f32 dx, dy, dz, dist;

    if (play == NULL || !sCappy.active)
        return;
    player = GET_PLAYER(play);
    mpos = player->actor.world.pos;
    sCappy.fxScroll++;

    // Mario goes cap-less while the cap is in flight — clear CAP_ON_HEAD every
    // frame so libsm64 renders the bare-head model (restored on catch in _Kill).
    if (sSm64MarioId >= 0 && p_sm64_set_mario_state) {
        u32 f = sSm64OutState.flags & ~SM64_MARIO_CAP_ON_HEAD;
        p_sm64_set_mario_state(sSm64MarioId, f);
    }

    switch (sCappy.phase) {
        case CAPPY_OUT:
            if (sCappy.mode == SM64_CAPPY_DIVE)
                sCappy.vel.y -= 1.2f;
            // Homing: GENTLE nudge toward a close enemy — never enough to fling the
            // cap far (so it still hovers near where you threw it, for the cap-jump).
            if (sCappy.homing && sCappy.target != NULL && sCappy.target->update != NULL) {
                f32 tx = sCappy.target->world.pos.x - sCappy.pos.x;
                f32 ty = (sCappy.target->world.pos.y + 20.0f) - sCappy.pos.y;
                f32 tz = sCappy.target->world.pos.z - sCappy.pos.z;
                f32 td = sqrtf(tx * tx + ty * ty + tz * tz);
                if (td > 1.0f) {
                    f32 spd =
                        sqrtf(sCappy.vel.x * sCappy.vel.x + sCappy.vel.y * sCappy.vel.y + sCappy.vel.z * sCappy.vel.z);
                    if (spd < 10.0f)
                        spd = 10.0f;
                    sCappy.vel.x += ((tx / td) * spd - sCappy.vel.x) * 0.12f;
                    sCappy.vel.y += ((ty / td) * spd - sCappy.vel.y) * 0.12f;
                    sCappy.vel.z += ((tz / td) * spd - sCappy.vel.z) * 0.12f;
                }
            }
            sCappy.pos.x += sCappy.vel.x;
            sCappy.pos.y += sCappy.vel.y;
            sCappy.pos.z += sCappy.vel.z;
            if (Sm64Cappy_TryBounce(play, &mpos, player))
                break;
            if (--sCappy.timer <= 0) {
                sCappy.phase = CAPPY_HOVER;
                sCappy.timer = CAPPY_HOVER_FRAMES;
            }
            break;
        case CAPPY_HOVER:
            sCappy.vel.x *= 0.85f;
            sCappy.vel.y *= 0.6f;
            sCappy.vel.z *= 0.85f;
            sCappy.pos.x += sCappy.vel.x;
            sCappy.pos.y += sCappy.vel.y;
            sCappy.pos.z += sCappy.vel.z;
            if (Sm64Cappy_TryBounce(play, &mpos, player))
                break;
            if (--sCappy.timer <= 0) {
                sCappy.phase = CAPPY_RETURN;
            }
            break;
        case CAPPY_ORBIT:
            sCappy.orbitAng += 0x1500;
            sCappy.pos.x = mpos.x + Math_SinS(sCappy.orbitAng) * CAPPY_ORBIT_RADIUS;
            sCappy.pos.y = mpos.y + 26.0f;
            sCappy.pos.z = mpos.z + Math_CosS(sCappy.orbitAng) * CAPPY_ORBIT_RADIUS;
            if (Sm64Cappy_TryBounce(play, &mpos, player))
                break;
            if (--sCappy.timer <= 0) {
                sCappy.phase = CAPPY_RETURN;
            }
            break;
        case CAPPY_RETURN:
            dx = mpos.x - sCappy.pos.x;
            dy = (mpos.y + 24.0f) - sCappy.pos.y;
            dz = mpos.z - sCappy.pos.z;
            dist = sqrtf(dx * dx + dy * dy + dz * dz);
            if (dist < CAPPY_CATCH_DIST) {
                Sm64Cappy_Kill();
                return;
            }
            sCappy.pos.x += (dx / dist) * CAPPY_RETURN_SPEED;
            sCappy.pos.y += (dy / dist) * CAPPY_RETURN_SPEED;
            sCappy.pos.z += (dz / dist) * CAPPY_RETURN_SPEED;
            break;
    }

    // World floor collision (OUT/HOVER): keep the cap riding ABOVE the terrain so
    // it follows up-slopes instead of phasing through them. Ray from above the cap
    // so a fast/steep climb still catches the floor.
    if (sCappy.phase == CAPPY_OUT || sCappy.phase == CAPPY_HOVER) {
        CollisionPoly* fpoly = NULL;
        Vec3f fq;
        f32 fy;
        fq.x = sCappy.pos.x;
        fq.y = sCappy.pos.y + 60.0f;
        fq.z = sCappy.pos.z;
        fy = BgCheck_EntityRaycastFloor1(&play->colCtx, &fpoly, &fq);
        if (fpoly != NULL && fy > BGCHECK_Y_MIN && sCappy.pos.y < fy + 10.0f) {
            sCappy.pos.y = fy + 10.0f;       // hug the slope
            if (sCappy.phase == CAPPY_OUT) { // climbing into terrain -> settle/hover
                sCappy.vel.y = 0.0f;
            }
        }
    }

    // Arm the stun collider at the cap each frame.
    if (sCappy.colInited) {
        sCappy.col.dim.pos.x = (s16)sCappy.pos.x;
        sCappy.col.dim.pos.y = (s16)sCappy.pos.y;
        sCappy.col.dim.pos.z = (s16)sCappy.pos.z;
        sCappy.col.base.atFlags |= AT_ON;
        CollisionCheck_SetAT(play, &play->colChkCtx, &sCappy.col.base);
    }
}

// Billboard placeholder for the cap (camera-facing white/gold disc). The real
// tiara mesh (converted from omm_tiara_geo.bin) replaces this once integrated.
void Sm64Cappy_Draw(PlayState* play) {
    GraphicsContext* gfxCtx;
    f32 spin;

    if (play == NULL || !sCappy.active)
        return;
    gfxCtx = play->state.gfxCtx;
    spin = sCappy.fxScroll * 0x800; // spins about its own axis as it flies

    // Mario's cap base is ~302 units; ~0.09 → a ~27-unit cap. Centered in X; Y
    // spans 0..144 and Z ~12, so recenter (0,-72,-12) to spin about its middle.
    OPEN_DISPS(gfxCtx);
    Matrix_Translate(sCappy.pos.x, sCappy.pos.y + 4.0f, sCappy.pos.z, MTXMODE_NEW);
    Matrix_RotateYF(spin * (3.14159265358979323846f / 0x8000), MTXMODE_APPLY);
    Matrix_Scale(0.09f, 0.09f, 0.09f, MTXMODE_APPLY);      // tune to taste
    Matrix_Translate(0.0f, -72.0f, -12.0f, MTXMODE_APPLY); // recenter
    gSPMatrix(POLY_OPA_DISP++, Matrix_Finalize(gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    Gfx_SetupDL25_Opa(gfxCtx);
    gDPSetCombineMode(POLY_OPA_DISP++, G_CC_SHADE, G_CC_SHADE); // lit vertex shade, no texture
    gSPSetGeometryMode(POLY_OPA_DISP++, G_LIGHTING);
    gSPSetLights1(POLY_OPA_DISP++, mario_red_lights_group);    // red for the cap dome
    gSPDisplayList(POLY_OPA_DISP++, mario_cap_unused_base_dl); // top (red) + brim (brown)
    CLOSE_DISPS(gfxCtx);
}

// =============================================================================
// Dispatcher — verbatim copy of z_en_partner.c:511-578 switch statement.
// =============================================================================

void Sm64Mario_HandleItems(PlayState* play, Player* player) {
    if (play == NULL || player == NULL || !Sm64Mario_IsReady()) {
        return;
    }

    Sm64MarioCaps_Tick();
    Sm64Mario_HandleCapDpad(play);
}

void Sm64Mario_DrawHeldStick(PlayState* play) {
    (void)play;
}

void Sm64Mario_ItemsReset(void) {
    sSm64CapPress = 0;
    Sm64Cappy_Kill();
    Sm64Mario_KillAllFireballs();
}

void Sm64Mario_ItemsResetWithPlayer(Player* player) {
    (void)player;
    Sm64Mario_ItemsReset();
}

u8 Sm64Mario_LensActive(void) {
    return (gPlayState != NULL && gPlayState->actorCtx.lensActive) ? 1 : 0;
}