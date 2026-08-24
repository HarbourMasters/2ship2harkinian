/**
 * item_gustjar.c - Gust Jar (Minish Cap style)
 *
 * Two modes:
 *   ABSORB (hold C): Pull enemies/props toward nozzle, AT collider damages them.
 *                     Props break via their own AC_HIT handlers (proper drops).
 *                     Heat builds over 10s (blue→yellow→red ring).
 *   BLOW (auto after 10s absorb): Elemental cone pushes all non-boss enemies.
 *                     Element effects based on medallion selection.
 *
 * Long-press C (20 frames, while idle): Radial element picker overlay.
 * Tap C: Use with last selected element.
 */

#include "z64.h"
#include "item_gustjar.h"
#include "../custom_items.h"
#include "../helpers/camera_helper.h"
#include "../helpers/equip_helper.h"
#include "../helpers/fx_helper.h"
#include "../objects/object_tornado.h" // shared wind cone + spiral ribbons
#include "mods/nei_save.h"             // ootQuestItems — MM's stand-in for OoT's medallion quest bits
#include "macros.h"
#include "functions.h"
#include "variables.h"
#include "objects/gameplay_keep/gameplay_keep.h"

void Player_InitGustJarIA(PlayState* play, Player* this) {
    Collider_InitCylinder(play, &gCustomItemState.gustJarCollider);
    Collider_SetCylinder(play, &gCustomItemState.gustJarCollider, &this->actor, &sGustJarColliderInit);
}

// Returns the medallion ITEM_* matching the gust jar's current element, or -1
// if the element is WIND (default — no medallion overlay needed).
s32 GustJar_GetActiveMedallionItem(void) {
    switch (gjElement) {
        case GUST_ELEMENT_FIRE:
            return ITEM_MEDALLION_FIRE;
        case GUST_ELEMENT_ICE:
            return ITEM_MEDALLION_WATER;
        case GUST_ELEMENT_SHADOW:
            return ITEM_MEDALLION_SHADOW;
        case GUST_ELEMENT_SPIRIT:
            return ITEM_MEDALLION_SPIRIT;
        case GUST_ELEMENT_LIGHT:
            return ITEM_MEDALLION_LIGHT;
        default:
            return -1; // WIND or unknown
    }
}

// =============================================================================
// Element ownership — driven by the OoT medallion mirror, not MM quest bits
// =============================================================================
//
// MM has no medallions. nei_oot_compat.h stubs every QUEST_MEDALLION_* to 0, so the ported
// CHECK_QUEST_ITEM(QUEST_MEDALLION_x) checks all read the SAME (wrong) quest bit — the blow
// ignored which medallion the player actually owned. 2ship keeps OoT's collect_register in a
// parallel store (NeiSaveData::ootQuestItems, OOT_QUEST_* bit indices), which is what the OoT
// quest page, the SW97 arrow wheel and the Sage's tunic already read. Use the same source.

static const u8 sGustElementQuestBit[GUST_ELEMENT_COUNT] = {
    OOT_QUEST_MEDALLION_FOREST, // WIND (always available, listed for the icon lookup)
    OOT_QUEST_MEDALLION_FIRE,   // FIRE
    OOT_QUEST_MEDALLION_WATER,  // ICE
    OOT_QUEST_MEDALLION_SHADOW, // SHADOW
    OOT_QUEST_MEDALLION_SPIRIT, // SPIRIT
    OOT_QUEST_MEDALLION_LIGHT,  // LIGHT
};

static const u16 sGustElementIcon[GUST_ELEMENT_COUNT] = {
    ITEM_MEDALLION_FOREST, ITEM_MEDALLION_FIRE,   ITEM_MEDALLION_WATER,
    ITEM_MEDALLION_SHADOW, ITEM_MEDALLION_SPIRIT, ITEM_MEDALLION_LIGHT,
};

u16 GustJar_ElementIcon(u8 element) {
    return (element < GUST_ELEMENT_COUNT) ? sGustElementIcon[element] : ITEM_MEDALLION_FOREST;
}

// AT damage for the current element. Bare WIND is pure knockback — it deals NO HP damage to
// anything (user decision). The collider stays armed with its damage bit though, because
// breakables key on dmgFlags + AC_HIT and ignore the damage value entirely (Obj_Tsubo mask
// 0x058BFFBC, En_Ishi 0x508, Obj_Kibako2 (1<<8|1<<10)), so pots, crates, grass and rocks still
// shatter in the gust while enemies only get blown away.
static u8 GustJar_ElementDamage(u8 element, u8 baseDamage) {
    return (element == GUST_ELEMENT_WIND) ? 0 : baseDamage;
}

// Blow cone half-angle, derived from the cone's own dimensions so the test can never disagree
// with the cone that gets drawn. Math_Atan2S(y, x) with (radius, length) is the apex angle.
static s16 GustJar_BlowHalfAngle(void) {
    return Math_Atan2S(GUST_CONE_BLOW_RADIUS, GUST_CONE_BLOW_LENGTH);
}

// WIND needs no medallion (it's the bare gust jar); the other five need theirs.
static u8 GustJar_ElementOwned(u8 element) {
    if (element == GUST_ELEMENT_WIND) {
        return 1;
    }
    if (element >= GUST_ELEMENT_COUNT) {
        return 0;
    }
    return (Nei_Save()->ootQuestItems & (1u << sGustElementQuestBit[element])) != 0;
}

u8 GustJar_ElementCount(void) {
    u8 count = 0;
    for (u8 e = 0; e < GUST_ELEMENT_COUNT; e++) {
        if (GustJar_ElementOwned(e)) {
            count++;
        }
    }
    return count;
}

u8 GustJar_ElementAt(u8 index) {
    u8 seen = 0;
    for (u8 e = 0; e < GUST_ELEMENT_COUNT; e++) {
        if (GustJar_ElementOwned(e)) {
            if (seen == index) {
                return e;
            }
            seen++;
        }
    }
    return GUST_ELEMENT_WIND;
}

u8 GustJar_GetElement(void) {
    // A medallion lost/never-owned (save editor, new file) must not leave a dead element primed.
    if (!GustJar_ElementOwned(gjElement)) {
        gjElement = GUST_ELEMENT_WIND;
    }
    return gjElement;
}

void GustJar_SetElement(u8 element) {
    if (GustJar_ElementOwned(element)) {
        gjElement = element;
    }
}

u8 GustJar_ElementNeighbor(u8 element, s32 dir) {
    u8 count = GustJar_ElementCount();
    if (count <= 1) {
        return GUST_ELEMENT_WIND;
    }
    for (u8 i = 0; i < count; i++) {
        if (GustJar_ElementAt(i) == element) {
            return GustJar_ElementAt((u8)((i + dir + count) % count));
        }
    }
    return GustJar_ElementAt(0);
}

static void GustJar_ClearScaleCache(void);        // Forward declaration
static void GustJar_TornadoStop(PlayState* play); // Forward declaration

// =============================================================================
// Equip / Unequip
// =============================================================================

static void GustJar_Equip(PlayState* play, Player* player) {
    if (gjEquipped)
        return;
    gjEquipped = 1;
    gjMode = GUST_MODE_IDLE;
    gjHeatTimer = 0;
    gjBlowActive = 0;
    gjBlowTimer = 0;
    gjCooldownTimer = 0;
    GustJar_GetElement(); // drop a primed element whose medallion is no longer owned

    // Equip does NOT enter first-person. Link starts in "holding the gust jar"
    // free-roam state with the carry pose applied each frame. C-Up explicitly
    // toggles first-person aim. Z-target works independently (rotates Link
    // toward focusActor like the bow, without forcing first-person).
    gjFirstPerson = 0;
    gjAimMode = 0;
    ItemEquip_PlayEquipSFX(play, player);
}

static void GustJar_Unequip(PlayState* play, Player* player) {
    if (!gjEquipped)
        return;
    if (gjFirstPerson) {
        FirstPerson_Exit(player, play);
        gjFirstPerson = 0;
    }
    GustJar_ClearScaleCache();
    GustJar_TornadoStop(play); // Handle_GustJar stops running now, so release the blures here
    gjEquipped = 0;
    gjMode = GUST_MODE_OFF;
    gjBlowActive = 0;
    gjHeatTimer = 0;
    gjBlowTimer = 0;
    gjCooldownTimer = 0;
    gjButtonMask = 0;
    Audio_StopSfxById(NA_SE_EV_WIND_TRAP);
    ItemEquip_PlayUnequipSFX(play, player);
}

// =============================================================================
// Aiming
// =============================================================================

static s16 GustJar_GetAimYaw(PlayState* play, Player* player) {
    // Priority: Z-target (lock-on) → first-person → Link's facing yaw.
    if (Player_IsZTargeting(player) && player->focusActor != NULL) {
        return Math_Vec3f_Yaw(&player->actor.world.pos, &player->focusActor->focus.pos);
    }
    if (gjFirstPerson) {
        return FirstPerson_GetAimYaw(player);
    }
    return player->actor.shape.rot.y;
}

// =============================================================================
// Absorb Mode — pull actors + AT collider damage
// =============================================================================

// Both lists are checked for every candidate. The caller only ever walks the ENEMY and PROP
// actor lists, and MM's categories don't always match the OoT intuition the lists were written
// with (En_Tubo_Trap, En_Kusa2, ...), so gating each list on a category just made those actors
// silently unsuckable.
static s32 GustJar_IsSuckable(Actor* actor) {
    for (s32 i = 0; i < GUST_SUCKABLE_PROP_COUNT; i++) {
        if (actor->id == sGustSuckableProps[i])
            return 1;
    }
    for (s32 i = 0; i < GUST_SUCKABLE_ENEMY_COUNT; i++) {
        if (actor->id == sGustSuckableEnemies[i])
            return 1;
    }
    return 0;
}

// =============================================================================
// Scale Cache (shrink actors as they get sucked in)
// =============================================================================

#define GUST_MAX_SCALED 16

static struct {
    Actor* actor;
    Vec3f originalScale;
} sScaleCache[GUST_MAX_SCALED];
static u8 sScaleCacheCount = 0;

static void GustJar_SaveScale(Actor* actor) {
    for (u8 i = 0; i < sScaleCacheCount; i++) {
        if (sScaleCache[i].actor == actor)
            return;
    }
    if (sScaleCacheCount < GUST_MAX_SCALED) {
        sScaleCache[sScaleCacheCount].actor = actor;
        sScaleCache[sScaleCacheCount].originalScale = actor->scale;
        sScaleCacheCount++;
    }
}

static void GustJar_ShrinkActor(Actor* actor, f32 factor) {
    GustJar_SaveScale(actor);
    for (u8 i = 0; i < sScaleCacheCount; i++) {
        if (sScaleCache[i].actor == actor) {
            actor->scale.x = sScaleCache[i].originalScale.x * factor;
            actor->scale.y = sScaleCache[i].originalScale.y * factor;
            actor->scale.z = sScaleCache[i].originalScale.z * factor;
            return;
        }
    }
}

static void GustJar_ClearScaleCache(void) {
    for (u8 i = 0; i < sScaleCacheCount; i++) {
        if (sScaleCache[i].actor != NULL && sScaleCache[i].actor->update != NULL) {
            sScaleCache[i].actor->scale = sScaleCache[i].originalScale;
        }
    }
    sScaleCacheCount = 0;
}

// =============================================================================
// Freezard-style cone VFX (smoke particles in cone shape)
// =============================================================================

// Spawn smoke balls flowing TOWARD nozzle (suction cone)
void GustJar_SpawnSuckVFX(PlayState* play, Vec3f* nozzle, s16 aimYaw) {
    // Supporting dust only — the tornado mesh is the effect now.
    if ((play->gameplayFrames % GUST_VFX_SPAWN_EVERY) != 0) {
        return;
    }
    for (s32 i = 0; i < GUST_VFX_PARTICLES; i++) {
        f32 dist = 80.0f + Rand_ZeroFloat(140.0f);
        s16 spreadAngle = aimYaw + (s16)Rand_CenteredFloat(0x2000); // ~45° spread
        f32 spreadY = Rand_CenteredFloat(30.0f);

        Vec3f pos = {
            nozzle->x + Math_SinS(spreadAngle) * dist,
            nozzle->y + spreadY,
            nozzle->z + Math_CosS(spreadAngle) * dist,
        };
        // Velocity: toward nozzle (negative of outward direction)
        f32 speed = 8.0f + Rand_ZeroFloat(4.0f);
        Vec3f vel = {
            -Math_SinS(spreadAngle) * speed,
            Rand_CenteredFloat(1.0f),
            -Math_CosS(spreadAngle) * speed,
        };
        Vec3f accel = { 0, 0.6f, 0 }; // Slight upward buoyancy (like Freezard)

        Color_RGBA8 prim = { 195, 225, 235, 150 }; // Pale cyan (Freezard style)
        Color_RGBA8 env = { 150, 200, 220, 100 };
        func_8002836C(play, &pos, &vel, &accel, &prim, &env, 200, 30, 12);
    }
}

// Spawn smoke balls flowing AWAY from nozzle (blow cone), colored by element
void GustJar_SpawnBlowVFX(PlayState* play, Vec3f* nozzle, s16 aimYaw, u8 element) {
    const GustElementColor* col = &sGustElementColors[element];

    if ((play->gameplayFrames % GUST_VFX_SPAWN_EVERY) != 0) {
        return;
    }
    for (s32 i = 0; i < GUST_VFX_PARTICLES; i++) {
        s16 spreadAngle = aimYaw + (s16)Rand_CenteredFloat(0x2000);
        f32 startDist = 10.0f + Rand_ZeroFloat(20.0f);

        Vec3f pos = {
            nozzle->x + Math_SinS(spreadAngle) * startDist,
            nozzle->y + Rand_CenteredFloat(10.0f),
            nozzle->z + Math_CosS(spreadAngle) * startDist,
        };
        // Velocity: away from nozzle (Freezard blow direction)
        f32 speed = 15.0f + Rand_ZeroFloat(5.0f);
        Vec3f vel = {
            Math_SinS(spreadAngle) * speed,
            Rand_CenteredFloat(2.0f) - 1.0f,
            Math_CosS(spreadAngle) * speed,
        };
        Vec3f accel = { 0, 0.6f, 0 };

        func_8002836C(play, &pos, &vel, &accel, (Color_RGBA8*)&col->prim, (Color_RGBA8*)&col->env, 200, 25, 15);
    }
}

// =============================================================================
// Tornado — the wind cone the jar summons at its mouth
// =============================================================================
//
// The cone mesh (object_nei_tornado) is intensity-only, so it comes out in whatever colour we
// hand it. Only the BLOW is tinted, by the primed element's colour, which is what makes the
// damage type readable at a glance; the SUCK is always plain white, because sucking has no
// damage type to communicate. The blow cone is also the larger of the two — it's the one that
// hurts.
//
// Update side fills sGustTornado and feeds the ribbons; the draw hook emits the geometry.
// Drawn at the SAME dimensions the gameplay cone uses (item_gustjar.h), so what you see is what
// grabs and what hits.
#define GUST_TORNADO_RIBBONS 6
// Texture scroll per frame, in quarter-texels. The texture is 64 tall = 256 quarter-texels, so
// the blow traverses the whole cone in ~13 frames and the suck (inward, hence negative) in ~21.
#define GUST_TORNADO_SCROLL_BLOW 20
#define GUST_TORNADO_SCROLL_SUCK (-12)

static TornadoParams sGustTornado;
static TornadoRibbons sGustRibbons;
static u8 sGustTornadoOn = 0;

static void GustJar_TornadoUpdate(PlayState* play, Vec3f* nozzle, s16 aimYaw, s16 aimPitch, u8 isBlow) {
    u8 element = (gjElement < GUST_ELEMENT_COUNT) ? gjElement : GUST_ELEMENT_WIND;
    const GustElementColor* col = &sGustElementColors[element];

    sGustTornado.origin = *nozzle;
    sGustTornado.yaw = aimYaw;
    sGustTornado.pitch = aimPitch;
    sGustTornado.length = isBlow ? GUST_CONE_BLOW_LENGTH : GUST_CYL_SUCK_LENGTH;
    sGustTornado.radius = isBlow ? GUST_CONE_BLOW_RADIUS : GUST_CYL_SUCK_RADIUS;
    // Element tint on the blow only; the suck is always white.
    sGustTornado.color.r = isBlow ? col->prim.r : 255;
    sGustTornado.color.g = isBlow ? col->prim.g : 255;
    sGustTornado.color.b = isBlow ? col->prim.b : 255;
    sGustTornado.color.a = isBlow ? 220 : 170;
    // Roll about the cone axis. The suck spins the other way from the blow so the two modes
    // read differently even when the element (and therefore the colour) is the same.
    sGustTornado.spin += isBlow ? 0x1200 : -0x0C00;
    // ...and slide the streaks ALONG the cone on top of that roll: outward while blowing,
    // back into the mouth while sucking. That axial motion is what sells the direction of the
    // wind — the roll alone reads the same either way.
    Tornado_AdvanceScroll(&sGustTornado, 0, isBlow ? GUST_TORNADO_SCROLL_BLOW : GUST_TORNADO_SCROLL_SUCK);
    sGustTornadoOn = 1;

    Tornado_RibbonsUpdate(play, &sGustRibbons, &sGustTornado, GUST_TORNADO_RIBBONS);
}

static void GustJar_TornadoStop(PlayState* play) {
    sGustTornadoOn = 0;
    Tornado_RibbonsStop(play, &sGustRibbons);
}

// =============================================================================
// Absorb Mode — pull actors + AT collider damage + shrink + Freezard suction VFX
// =============================================================================

static void GustJar_Absorb(Player* player, PlayState* play, Vec3f* nozzle, s16 aimYaw, s16 aimPitch) {
    gjHeatTimer++;

    GustJar_TornadoUpdate(play, nozzle, aimYaw, aimPitch, /*isBlow=*/0);

    // Looping wind sound
    func_8002F974(&player->actor, NA_SE_EV_WIND_TRAP - SFX_FLAG);

    // Freezard-style suction VFX (cone of particles toward nozzle)
    GustJar_SpawnSuckVFX(play, nozzle, aimYaw);

    // Position AT collider at nozzle for damage
    ColliderCylinder* col = &gjCollider;
    col->dim.pos.x = (s16)nozzle->x;
    col->dim.pos.y = (s16)nozzle->y;
    col->dim.pos.z = (s16)nozzle->z;
    // SUCK always deals the physical bit regardless of selected element — only the BLOW cone
    // delivers elemental damage. Reset every frame because GustJar_Blow rewrites this with
    // elemDmgFlags on the next mode switch.
    //
    // DMG_GORON_POUND, not OoT's 0x40 (DMG_HAMMER_SWING). In MM bit 6 is DMG_UNK_0x06 and every
    // damage table scores it DMG_ENTRY(0, ...) — the ported constant was the reason absorb did no
    // damage at all. Also restore the collider's damage/effect, which GustJar_Blow zeroes.
    col->elem.atDmgInfo.dmgFlags = DMG_GORON_POUND;
    col->elem.atDmgInfo.damage = GustJar_ElementDamage(gjElement, GUST_DAMAGE_SUCK);
    col->elem.atDmgInfo.effect = 0x00;
    col->dim.radius = GUST_COL_SUCK_RADIUS; // small nub at the mouth; the blow sizes its own
    col->dim.height = GUST_COL_SUCK_HEIGHT;
    col->dim.yShift = -(GUST_COL_SUCK_HEIGHT / 2); // centre it on the nozzle, not stack it above
    col->base.atFlags |= AT_ON | AT_TYPE_PLAYER;
    CollisionCheck_SetAT(play, &play->colChkCtx, &col->base);
    if (col->base.atFlags & AT_HIT) {
        col->base.atFlags &= ~AT_HIT;
    }

    // Pull + shrink actors toward the nozzle.
    //
    // The volume is a CYLINDER along the aim axis: within the suck length ahead of the nozzle and
    // within the suck radius of the axis. A cone would pinch to nothing right at the mouth, which is
    // where the pull should be strongest, so suction keeps its full width the whole way out.
    // (It used to be an unaimed sphere, which grabbed things behind Link.)
    Vec3f axis;
    f32 suckRadiusSq = SQ(GUST_CYL_SUCK_RADIUS);
    f32 shrinkStart = GUST_CYL_SUCK_LENGTH * 0.75f; // Start shrinking at this distance
    ActorCategory categories[] = { ACTORCAT_ENEMY, ACTORCAT_PROP };

    Tornado_GetAxis(aimYaw, aimPitch, &axis);

    for (s32 c = 0; c < 2; c++) {
        Actor* actor = play->actorCtx.actorLists[categories[c]].first;
        while (actor != NULL) {
            Actor* next = actor->next;
            if (actor->update != NULL && GustJar_IsSuckable(actor)) {
                // Nozzle -> actor, then split it into "along the axis" and "off the axis".
                f32 ax = actor->world.pos.x - nozzle->x;
                f32 ay = actor->world.pos.y - nozzle->y;
                f32 az = actor->world.pos.z - nozzle->z;
                f32 along = (ax * axis.x) + (ay * axis.y) + (az * axis.z);

                if ((along >= 0.0f) && (along <= GUST_CYL_SUCK_LENGTH)) {
                    f32 px = ax - (axis.x * along);
                    f32 py = ay - (axis.y * along);
                    f32 pz = az - (axis.z * along);

                    if (((px * px) + (py * py) + (pz * pz)) < suckRadiusSq) {
                        // Pull straight back down the axis toward the nozzle.
                        f32 dx = -ax;
                        f32 dz = -az;
                        f32 norm = sqrtf(SQ(dx) + SQ(dz));

                        if (norm > 1.0f) {
                            f32 strength = 8.0f;
                            f32 invNorm = strength / norm;

                            actor->world.pos.x += dx * invNorm;
                            actor->world.pos.z += dz * invNorm;
                            if (actor->bgCheckFlags & BGCHECKFLAG_GROUND)
                                actor->world.pos.y += 2.0f;

                            // Shrink as they get closer
                            if (norm < shrinkStart) {
                                f32 factor = norm / shrinkStart;
                                if (factor < 0.15f)
                                    factor = 0.15f;
                                GustJar_ShrinkActor(actor, factor);
                            }
                        }
                    }
                }
            }
            actor = next;
        }
    }

    // Cap heat at max — DO NOT auto-transition to blow. The blow is now
    // manual: SUCK direction fires it on C-release (proportional to charge);
    // BLOW direction fires it directly on C-hold. Per user request: blow
    // never releases automatically.
    if (gjHeatTimer > GUST_HEAT_MAX) {
        gjHeatTimer = GUST_HEAT_MAX;
    }
}

// =============================================================================
// Blow Mode — elemental cone VFX + strong push + collider sweep for env effects
// =============================================================================

// Element → AT collider dmgFlags mapping for the BLOW cone. Mirrors what THIS build's SW97
// medallion arrows hit with (sArrowDmgFlags, z_actor.c:4566) so each gustjar element delivers the
// same damage type as its bow arrow; SUCK uses the physical bit and is set in GustJar_Absorb.
//
// Every entry is deliberately a SINGLE bit. CollisionCheck_GetDamageAndEffectOnElementAC shifts
// dmgFlags until it equals 1 to pick the damage-table row, so only the highest set bit is ever
// read: the ported OoT masks (0x00020800, 0x00041000, ...) resolved to DMG_DEKU_LAUNCH /
// DMG_UNK_0x12 / DMG_DEKU_BUBBLE instead of the elemental rows, i.e. the blow "damaged wrong".
//
// Passing the real elemental bit is also what makes the enemy run its OWN reaction (En_Bb's
// ICE_ARROW freeze, LIGHT_ARROW stun, ...) rather than us faking one with freezeTimer.
static u32 GustJar_GetElementDmgFlags(PlayState* play, u8 element) {
    switch (element) {
        case GUST_ELEMENT_FIRE:
            return DMG_FIRE_ARROW;
        case GUST_ELEMENT_ICE:
            return DMG_ICE_ARROW;
        case GUST_ELEMENT_LIGHT:
            // Alternate with DMG_LIGHT_RAY on odd frames, exactly like MagicLight_Update
            // (z_magic_light.inc.c:292), so mirror-shield light receivers fire too.
            return (play->gameplayFrames & 1) ? DMG_LIGHT_RAY : DMG_LIGHT_ARROW;
        case GUST_ELEMENT_SHADOW:
            // MM has no dark/soul damage bit; ARROW_TYPE_SW97_DARK/SOUL hit as normal arrows.
            // The shadow flavour comes from Sw97_TagBlinded in GustJar_ApplyElementEffect.
            return DMG_NORMAL_ARROW;
        case GUST_ELEMENT_SPIRIT:
            return DMG_NORMAL_ARROW;
        case GUST_ELEMENT_WIND:
        default:
            // Bare wind is the heavy physical blast — same bit the suck uses, so the cone also
            // shatters pots/crates/rocks it sweeps over.
            return DMG_GORON_POUND;
    }
}

// Element flavour on regular enemies, on TOP of whatever the elemental dmgFlags already made the
// enemy do to itself.
//
// The OoT version parked freezeTimer on the target every frame of the blow. In MM freezeTimer
// halts the actor's update entirely (z64actor.h:174), which means it stops re-registering its AC
// collider and stops moving: the target took no damage and never got knocked back for the whole
// blow. So: tint only, applied ONCE per target (guarded on colorFilterTimer) and never long enough
// to swallow the cone. The actual freeze/burn/stun is the enemy's own damage-table reaction to
// DMG_ICE_ARROW / DMG_FIRE_ARROW / DMG_LIGHT_ARROW.
extern void Sw97_TagBlinded(Actor* actor, int16_t frames);
static void GustJar_ApplyElementEffect(Actor* actor, PlayState* play, u8 element) {
    (void)play;
    // Only affect regular enemies, not bosses
    if (actor->category != ACTORCAT_ENEMY)
        return;
    // Already flagged by this blow (or by something else) — don't re-stamp every frame.
    if (actor->colorFilterTimer != 0)
        return;

    switch (element) {
        case GUST_ELEMENT_SHADOW:
            Actor_SetColorFilter(actor, COLORFILTER_COLORFLAG_GRAY, 255, COLORFILTER_BUFFLAG_XLU, 40);
            // Shadow gustjar BLOW blinds the target — same effect as the SW97 Shadow Arrow.
            // Distance-to-player is spoofed for ~10 sec so the enemy stops tracking Link.
            Sw97_TagBlinded(actor, 300);
            break;
        case GUST_ELEMENT_FIRE:
            Actor_SetColorFilter(actor, COLORFILTER_COLORFLAG_RED, 255, COLORFILTER_BUFFLAG_XLU, 30);
            break;
        case GUST_ELEMENT_ICE:
            Actor_SetColorFilter(actor, COLORFILTER_COLORFLAG_BLUE, 255, COLORFILTER_BUFFLAG_XLU, 40);
            break;
        case GUST_ELEMENT_LIGHT:
            Actor_SetColorFilter(actor, COLORFILTER_COLORFLAG_BLUE, 255, COLORFILTER_BUFFLAG_XLU, 30);
            break;
        case GUST_ELEMENT_SPIRIT:
            Actor_SetColorFilter(actor, COLORFILTER_COLORFLAG_RED, 200, COLORFILTER_BUFFLAG_XLU, 40);
            break;
        default:
            break;
    }
}

// Knockback for one actor in the blow cone.
//
// The OoT version teleported world.pos by up to 40 units/frame and then bumped `velocity`. Neither
// survives contact with MM: the teleport tunnels enemies through walls and floors (nothing
// re-runs a bg check on that write), and MM movement helpers rebuild velocity.x/z from
// `actor->speed` and `actor->world.rot.y` every frame, so the velocity bump was gone the same
// frame it was applied. Drive MM's own movement fields instead, and line-test the (much smaller)
// positional nudge against the collision so nothing is shoved into geometry.
static void GustJar_PushActor(Actor* actor, PlayState* play, s16 pushYaw, f32 force) {
    Vec3f from = actor->world.pos;
    Vec3f to;
    Vec3f hit;
    CollisionPoly* poly;
    s32 bgId;

    // Aim the actor's own locomotion away from the nozzle. MM enemies that keep steering will
    // fight this, which is the intent — the blast shoves, it doesn't mind-control.
    actor->world.rot.y = pushYaw;
    actor->shape.rot.y = pushYaw;
    actor->speed = force;
    actor->velocity.x = Math_SinS(pushYaw) * force;
    actor->velocity.z = Math_CosS(pushYaw) * force;
    if (actor->velocity.y < GUST_BLOW_LIFT) {
        actor->velocity.y = GUST_BLOW_LIFT;
    }
    actor->bgCheckFlags &= ~BGCHECKFLAG_GROUND; // let them leave the floor

    // Immediate nudge so the shove reads instantly, capped and wall-tested.
    to.x = from.x + Math_SinS(pushYaw) * GUST_BLOW_NUDGE;
    to.y = from.y + 5.0f;
    to.z = from.z + Math_CosS(pushYaw) * GUST_BLOW_NUDGE;
    if (!BgCheck_EntityLineTest1(&play->colCtx, &from, &to, &hit, &poly, true, false, false, true, &bgId)) {
        actor->world.pos.x = to.x;
        actor->world.pos.z = to.z;
    }
}

static void GustJar_Blow(Player* player, PlayState* play, Vec3f* nozzle, s16 aimYaw, s16 aimPitch) {
    gjBlowTimer--;

    GustJar_TornadoUpdate(play, nozzle, aimYaw, aimPitch, /*isBlow=*/1);

    // Freezard-style blow VFX
    GustJar_SpawnBlowVFX(play, nozzle, aimYaw, gjElement);

    // Wind sound
    func_8002F974(&player->actor, NA_SE_EV_WIND_TRAP - SFX_FLAG);

    // === AT COLLIDER SWEEP along cone for environmental effects ===
    // Place collider at 3 positions along the cone so torches/sun switches get hit
    ColliderCylinder* col = &gjCollider;
    u32 elemDmgFlags = GustJar_GetElementDmgFlags(play, gjElement);
    col->elem.atDmgInfo.dmgFlags = elemDmgFlags;
    // The OoT version zeroed damage here ("VFX only"). In MM that also zeroes the base value the
    // damage table multiplies (CollisionCheck_GetDamageAndEffectOnElementAC scales the AT damage
    // by the table's multiplier), so the elemental blow landed for exactly 0 — the "daña mal".
    // Bare WIND is the one element that IS meant to land for 0: it only knocks things away.
    col->elem.atDmgInfo.damage = GustJar_ElementDamage(gjElement, GUST_DAMAGE_BLOW);
    col->elem.atDmgInfo.effect = 0x00;
    col->base.atFlags |= AT_ON | AT_TYPE_PLAYER;

    // Sweep the collider along the cone at near/mid/far. These are FRACTIONS of the cone's
    // length, so the damage coverage follows the cone — hardcoded distances would leave the far
    // half of a longer cone doing nothing but pushing.
    static const f32 sweepFracs[] = { GUST_BLOW_SWEEP_NEAR, GUST_BLOW_SWEEP_MID, GUST_BLOW_SWEEP_FAR };
    Vec3f sweepAxis;

    Tornado_GetAxis(aimYaw, aimPitch, &sweepAxis);
    for (s32 s = 0; s < 3; s++) {
        f32 dist = GUST_CONE_BLOW_LENGTH * sweepFracs[s];
        // The cone's own radius at this distance — see GUST_COL_BLOW_RADIUS_AT. Each sample is
        // sized to swallow the cone's cross-section right there, so the three of them together
        // cover the whole drawn cone instead of one fixed size being too fat at the mouth and
        // too thin at the far end.
        s16 coneR = GUST_COL_BLOW_RADIUS_AT(sweepFracs[s]);

        col->dim.radius = coneR;
        col->dim.height = coneR * 2;
        col->dim.yShift = -coneR; // yShift is the BOTTOM — centre the cylinder on the aim line

        // Follow the full 3D aim axis, so aiming up or down still lands the hits on the cone
        // instead of on the ground track underneath it.
        col->dim.pos.x = (s16)(nozzle->x + sweepAxis.x * dist);
        col->dim.pos.y = (s16)(nozzle->y + sweepAxis.y * dist);
        col->dim.pos.z = (s16)(nozzle->z + sweepAxis.z * dist);
        CollisionCheck_SetAT(play, &play->colChkCtx, &col->base);
    }
    if (col->base.atFlags & AT_HIT) {
        col->base.atFlags &= ~AT_HIT;
    }

    // === STRONG PUSH on all ACTORCAT_ENEMY in cone (NOT bosses) ===
    f32 pushForce = (gjElement == GUST_ELEMENT_WIND) ? GUST_BLOW_PUSH_WIND : GUST_BLOW_PUSH_ELEM;
    f32 blowLength = GUST_CONE_BLOW_LENGTH;
    f32 rangeSq = SQ(blowLength);
    s16 halfAngle = GustJar_BlowHalfAngle();

    Actor* actor = play->actorCtx.actorLists[ACTORCAT_ENEMY].first;
    while (actor != NULL) {
        Actor* next = actor->next;
        if (actor->update != NULL) {
            f32 dx = actor->world.pos.x - nozzle->x;
            f32 dz = actor->world.pos.z - nozzle->z;
            f32 distSq = SQ(dx) + SQ(dz);

            if (distSq < rangeSq && distSq > 1.0f) {
                s16 angleToActor = Math_Atan2S(dx, dz);
                s16 angleDiff = angleToActor - aimYaw;
                if (angleDiff < 0)
                    angleDiff = -angleDiff;
                if (angleDiff > 0x7FFF)
                    angleDiff = (s16)(0xFFFF - angleDiff);

                if (angleDiff < halfAngle) {
                    // Vertical gate: the cone is a cone, not an infinite column. Without this the
                    // blow yanked enemies on balconies/below ledges that were nowhere near the aim.
                    if (fabsf(actor->world.pos.y - nozzle->y) < LINK_HEIGHT_HITBOX) {
                        f32 dist = sqrtf(distSq);
                        f32 force = pushForce * (1.0f - dist / blowLength);
                        if (force < 2.0f)
                            force = 2.0f;

                        GustJar_PushActor(actor, play, angleToActor, force);
                        GustJar_ApplyElementEffect(actor, play, gjElement);
                    }
                }
            }
        }
        actor = next;
    }

    // Blow timer expired
    if (gjBlowTimer <= 0) {
        gjMode = GUST_MODE_IDLE;
        gjBlowActive = 0;
        gjHeatTimer = 0;
        // No cooldown — the old GUST_COOLDOWN (120 frames) was for the legacy
        // auto-overheat behaviour. With manual SUCK/BLOW direction control,
        // the user should be able to immediately press C again to suck or
        // blow without a 2-second lockout.
        Audio_StopSfxById(NA_SE_EV_WIND_TRAP);
    }
}

// Element selection is handled in Kaleido (z_kaleido_item.c), not during gameplay.

// =============================================================================
// Draw
// =============================================================================

void CustomItems_DrawGustJar(Player* this, PlayState* play) {
    GustJarPot_Draw(this, play);
    // The wind cone. sGustTornadoOn is re-armed every frame by GustJar_Absorb/GustJar_Blow and
    // cleared at the top of Handle_GustJar, so it can only be set when the jar is actually
    // sucking or blowing this frame.
    if (sGustTornadoOn) {
        Tornado_Draw(play, &sGustTornado);
    }
}

// =============================================================================
// Hold Pose — override Link's arm joints with frame 8 of carryB_free
// =============================================================================
//
// While the gust jar is equipped, Link visually holds it in front of his chest
// using the vanilla "carry pot with both hands" pose (gPlayerAnim_link_normal_
// carryB_free at frame 8). We extract that exact frame's joint rotations and
// copy ONLY the 6 arm bones (shoulder/forearm/hand × L/R) on top of whatever
// animation Link is currently playing — walk, run, idle, jump, etc. all
// continue normally; only the arms snap to the carry pose. Same pattern as
// item_ballchain.c (BallChain_SetEquipPose), but data-driven from the anim
// asset instead of hardcoded magic numbers.

static void GustJar_ApplyCarryPose(Player* player, PlayState* play) {
    Vec3s frameBuf[PLAYER_LIMB_MAX];

    // AnimationContext_SetLoadFrame does an immediate synchronous memcpy of
    // the frame's joint table into frameBuf (z_skelanime.c:909) — values are
    // available right away.
    AnimationContext_SetLoadFrame(play, (LinkAnimationHeader*)&gPlayerAnim_link_normal_carryB_free, 8, PLAYER_LIMB_MAX,
                                  frameBuf);

    player->skelAnime.jointTable[PLAYER_LIMB_L_SHOULDER] = frameBuf[PLAYER_LIMB_L_SHOULDER];
    player->skelAnime.jointTable[PLAYER_LIMB_L_FOREARM] = frameBuf[PLAYER_LIMB_L_FOREARM];
    player->skelAnime.jointTable[PLAYER_LIMB_L_HAND] = frameBuf[PLAYER_LIMB_L_HAND];
    player->skelAnime.jointTable[PLAYER_LIMB_R_SHOULDER] = frameBuf[PLAYER_LIMB_R_SHOULDER];
    player->skelAnime.jointTable[PLAYER_LIMB_R_FOREARM] = frameBuf[PLAYER_LIMB_R_FOREARM];
    player->skelAnime.jointTable[PLAYER_LIMB_R_HAND] = frameBuf[PLAYER_LIMB_R_HAND];
}

// Re-stamp the hold pose from CustomItems_LatePose.
//
// Handle_GustJar runs out of CustomItems_Update, which is BEFORE Player_UpdateCommon re-samples
// the current animation into jointTable. So the arms written during the update were overwritten
// every single frame and Link never actually held the jar up in MM. Ball & chain already solves
// this the same way (BallChain_RefreshPose); the gust jar just was never registered.
void GustJar_RefreshPose(Player* player, PlayState* play) {
    if (!gjEquipped) {
        return;
    }
    GustJar_ApplyCarryPose(player, play);
}

// =============================================================================
// Main Handler
// =============================================================================

void Handle_GustJar(Player* this, PlayState* play) {
    // Ensure collider is initialized
    if (gjCollider.base.shape != COLSHAPE_CYLINDER) {
        Player_InitGustJarIA(play, this);
    }

    // Tornado is opt-in per frame: only GustJar_Absorb / GustJar_Blow re-arm it, so every early
    // return below (unequipped, blocked, damaged, idle) leaves the cone hidden.
    //
    // sGustTornadoOn still holds LAST frame's value here. If nothing armed it, the effect is
    // over and the ribbons must give their blure slots back — the engine only has 25, so
    // leaking six per suck would starve every other trail in the scene within a few uses.
    if (!sGustTornadoOn) {
        GustJar_TornadoStop(play);
    }
    sGustTornadoOn = 0;

    ItemInputState input;
    static s8 prevInvincibility = 0;
    ItemInput_Update(&input, ITEM_GUST_JAR, this, play);

    if (!input.wasEquipped) {
        if (gjEquipped)
            GustJar_Unequip(play, this);
        return;
    }
    if (ItemInput_IsBlocked(this, play)) {
        if (gjEquipped)
            GustJar_Unequip(play, this);
        return;
    }

    gjButtonMask = input.equippedButton;

    if (ItemInput_CheckDamage(this, &prevInvincibility)) {
        GustJar_Unequip(play, this);
        return;
    }

    u8 btnPressed = input.isPressed;
    u8 btnHeld = input.isHeld;

    if (!gjEquipped) {
        if (btnPressed) {
            // Equip and fall through to the rest of Handle_GustJar so the
            // pose, Z-target rotation, and C-button absorb/blow handler all
            // run on the SAME frame as the equip. The old early-return only
            // made sense when equip auto-entered first-person and needed a
            // settle frame — the new flow never does that, so returning here
            // just delays the first suck/blow by one frame and occasionally
            // dropped fast C taps.
            GustJar_Equip(play, this);
        } else {
            return;
        }
    }

    // C-Up: explicit toggle of first-person aim. No other state changes here —
    // Z-target rotation, pose override, and cone aim are handled separately
    // and work in BOTH first-person and free-roam states.
    if (CHECK_BTN_ALL(play->state.input[0].press.button, BTN_CUP)) {
        if (gjFirstPerson) {
            FirstPerson_Exit(this, play);
            gjFirstPerson = 0;
            gjAimMode = 0;
        } else {
            FirstPerson_Init(this, play);
            gjFirstPerson = 1;
            gjAimMode = 0;
            // (FirstPerson_Init already primes the aim-ready timer; MM Player has no unk_834)
        }
        ItemEquip_PlayEquipSFX(play, this);
        return;
    }

    // Other button pressed → unequip. R is excluded here (locally for the
    // gust jar): R is used by the in-game element cycle / shield-suppress
    // logic and must NOT trigger an unequip. We re-derive the check inline
    // instead of trusting ItemInput_CheckOtherButtons, which lists R as an
    // "action button" globally for all other items.
    {
        static const u16 sGjUnequipButtons = BTN_A | BTN_B | BTN_START | BTN_CLEFT | BTN_CDOWN | BTN_CRIGHT | BTN_DUP |
                                             BTN_DDOWN | BTN_DLEFT | BTN_DRIGHT;
        u16 mask = sGjUnequipButtons & ~input.equippedButton;
        if (play->state.input[0].press.button & mask) {
            GustJar_Unequip(play, this);
            return;
        }
    }

    // Z-target rotation (bow-style): smooth-rotate Link toward the focusActor
    // when locked on. Works regardless of first-person state. This matches the
    // bow's Z-target behaviour where Player_GetMovementSpeedAndYaw aligns the
    // player yaw to focusActor every frame without forcing first-person.
    u8 isZTargeting = Player_IsZTargeting(this);
    if (isZTargeting && this->focusActor != NULL) {
        s16 targetYaw = Math_Vec3f_Yaw(&this->actor.world.pos, &this->focusActor->focus.pos);
        Math_ScaledStepToS(&this->actor.shape.rot.y, targetYaw, 0x800);
        this->actor.world.rot.y = this->actor.shape.rot.y;
        this->yaw = this->actor.shape.rot.y;
    }

    // First-person update — only when explicitly toggled on via C-Up.
    if (gjFirstPerson) {
        FirstPerson_Update(this, play);
    }

    // Hold pose — override Link's arm joints every frame so he visually holds
    // the gust jar in front of him with both hands, regardless of what
    // animation his lower body is playing (walk, run, idle, jump, ...).
    GustJar_ApplyCarryPose(this, play);

    // Calculate nozzle position
    s16 aimYaw = GustJar_GetAimYaw(play, this);
    s16 aimPitch = gjFirstPerson ? FirstPerson_GetAimPitch(this) : 0;
    Vec3f nozzle = this->actor.world.pos;
    nozzle.y += 25.0f;
    f32 hDist = 35.0f * Math_CosS(aimPitch);
    nozzle.x += Math_SinS(aimYaw) * hDist;
    nozzle.z += Math_CosS(aimYaw) * hDist;
    nozzle.y -= Math_SinS(aimPitch) * 35.0f;

    // Cooldown tick
    if (gjCooldownTimer > 0) {
        gjCooldownTimer--;
    }

    // Heat decay when idle (not absorbing)
    if (gjMode == GUST_MODE_IDLE && gjHeatTimer > 0) {
        gjHeatTimer -= 2;
        if (gjHeatTimer < 0)
            gjHeatTimer = 0;
    }

    // ===== BLOW MODE dispatch =====
    // Two flavors share GUST_MODE_BLOW:
    //   - SUCK direction: timed blow set on C-release. Runs until gjBlowTimer
    //     decays to 0 (handled inside GustJar_Blow).
    //   - BLOW direction (manual): hold C blows continuously. Release C stops
    //     immediately (no timer involvement). Heat is not consumed here.
    if (gjMode == GUST_MODE_BLOW) {
        if (gjBlowDir == GUST_DIR_BLOW && !btnHeld) {
            // Manual blow ended — C released. Reset to IDLE.
            GustJar_ClearScaleCache();
            gjMode = GUST_MODE_IDLE;
            gjBlowActive = 0;
            gjBlowTimer = 0;
            Audio_StopSfxById(NA_SE_EV_WIND_TRAP);
            return;
        }
        GustJar_Blow(this, play, &nozzle, aimYaw, aimPitch);
        return;
    }

    // ===== L+R COMBO: toggle SUCK/BLOW direction =====
    // L+R simultaneously toggles gjBlowDir between SUCK (default — hold C
    // absorbs, release C blows proportional to charge) and BLOW (manual —
    // hold C directly blows with current element, no charge mechanic).
    // Works in any mode while the gust jar is equipped.
    u8 lrCurr =
        CHECK_BTN_ALL(play->state.input[0].cur.button, BTN_L) && CHECK_BTN_ALL(play->state.input[0].cur.button, BTN_R);
    u8 lrPress = CHECK_BTN_ALL(play->state.input[0].press.button, BTN_L) ||
                 CHECK_BTN_ALL(play->state.input[0].press.button, BTN_R);
    u8 lrToggled = 0;
    if (lrCurr && lrPress) {
        gjBlowDir = (gjBlowDir == GUST_DIR_SUCK) ? GUST_DIR_BLOW : GUST_DIR_SUCK;
        Audio_PlaySoundGeneral(NA_SE_SY_DECIDE, &this->actor.world.pos, 4, &gSfxDefaultFreqAndVolScale,
                               &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
        lrToggled = 1;

        // Reset to a clean IDLE so the new direction takes effect on the
        // next C action — otherwise toggling mid-ABSORB or mid-BLOW leaves
        // gjMode set in the OLD direction and the next C-press silently
        // does nothing (user-reported bug after SUCK→BLOW toggle).
        if (gjMode == GUST_MODE_ABSORB || gjMode == GUST_MODE_BLOW) {
            GustJar_ClearScaleCache();
            Audio_StopSfxById(NA_SE_EV_WIND_TRAP);
            gjMode = GUST_MODE_IDLE;
            gjBlowActive = 0;
            gjBlowTimer = 0;
            gjHeatTimer = 0;
        }

        // Consume L/R so neither the cycle nor the shield handler also fires.
        play->state.input[0].cur.button &= ~(BTN_L | BTN_R);
        play->state.input[0].press.button &= ~(BTN_L | BTN_R);
    }

    // ===== SINGLE R/L: cycle element =====
    // Works ANY time the gust jar is equipped (IDLE, ABSORB, BLOW — any
    // state, any aim mode). Skipped if the L+R combo just toggled the
    // direction this frame.
    if (!lrToggled) {
        s8 cycleDir = 0;
        if (CHECK_BTN_ALL(play->state.input[0].press.button, BTN_R)) {
            cycleDir = 1;
        } else if (CHECK_BTN_ALL(play->state.input[0].press.button, BTN_L)) {
            cycleDir = -1;
        }

        // Ownership comes from the OoT medallion mirror (Nei_Save()->ootQuestItems). The old
        // CHECK_QUEST_ITEM(QUEST_MEDALLION_*) path read MM quest bits that don't exist — every
        // constant is stubbed to 0 in nei_oot_compat.h, so all five checks tested the same bit.
        if (cycleDir != 0 && GustJar_ElementCount() > 1) {
            GustJar_SetElement(GustJar_ElementNeighbor(GustJar_GetElement(), cycleDir));
            Audio_PlaySoundGeneral(NA_SE_SY_CURSOR, &this->actor.world.pos, 4, &gSfxDefaultFreqAndVolScale,
                                   &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
        }
    }

    // Defensive R/L swallow on play->state.input[0]: the sp44 copy is filtered
    // earlier by GustJar_FilterPlayerInput (z_player.c, before Player_UpdateCommon),
    // which prevents shield. This pass also clears the raw input so downstream
    // consumers in the same frame (other custom items, ExtEquip, etc.) don't
    // re-trigger off the same R/L press. Fires whenever the gust jar is the
    // active C-button item.
    if (gjEquipped) {
        play->state.input[0].cur.button &= ~(BTN_L | BTN_R);
        play->state.input[0].press.button &= ~(BTN_L | BTN_R);
    }

    // ===== C-BUTTON HANDLING (direction-aware) =====
    static u8 wasHeld = 0;
    u8 isHeld = btnHeld && !btnPressed;

    if (wasHeld && !btnHeld) {
        // C released. Two cases by direction:
        if (gjBlowDir == GUST_DIR_SUCK && gjMode == GUST_MODE_ABSORB) {
            // SUCK direction: discharge the accumulated charge as a timed blow.
            // Blow duration = gjHeatTimer / 2 (half the suck time). If no
            // charge was built, no blow — just return to IDLE.
            s16 blowDuration = gjHeatTimer / 2;
            GustJar_ClearScaleCache();
            if (blowDuration > 0) {
                gjMode = GUST_MODE_BLOW;
                gjBlowActive = 1;
                gjBlowTimer = blowDuration;
                Audio_StopSfxById(NA_SE_EV_WIND_TRAP);
                Player_PlaySfx(this, NA_SE_PL_MAGIC_WIND_NORMAL);
            } else {
                gjMode = GUST_MODE_IDLE;
            }
            gjHeatTimer = 0;
        } else if (gjMode == GUST_MODE_ABSORB) {
            // BLOW direction with C released from absorb (shouldn't normally
            // happen since BLOW direction starts blow directly, but safe path).
            GustJar_ClearScaleCache();
            gjMode = GUST_MODE_IDLE;
            gjHeatTimer = 0;
        }
        wasHeld = 0;
        return;
    }

    if (isHeld && gjCooldownTimer <= 0) {
        if (gjMode == GUST_MODE_IDLE) {
            // Start suck or blow based on direction.
            if (gjBlowDir == GUST_DIR_BLOW) {
                gjMode = GUST_MODE_BLOW;
                gjBlowActive = 1;
                gjBlowTimer = 0x7FFF; // sentinel — manual mode reads btnHeld, not timer
                Player_PlaySfx(this, NA_SE_PL_MAGIC_WIND_NORMAL);
            } else {
                gjMode = GUST_MODE_ABSORB;
            }
        }
        if (gjMode == GUST_MODE_ABSORB) {
            GustJar_Absorb(this, play, &nozzle, aimYaw, aimPitch);
        }
        wasHeld = 1;
    }
}

// Called from Player_Update BEFORE Player_UpdateCommon runs — strips L/R from
// the local sp44 copy so the vanilla shield action handler (which reads
// sControlInput->cur.button, the COPY) never sees the press. Handle_GustJar
// runs much later in the frame and was too late to prevent shield latching.
//
// The actual in-game cycle logic in Handle_GustJar reads play->state.input[0]
// (NOT this filtered copy), so cycling/L+R combo still see the raw bits.
//
// Gate: gjEquipped. Whenever the gust jar is the live C-button item, R must
// never raise the shield — R/L are reserved for element cycling (IDLE+aim) or
// the L+R combo (ABSORB). This covers ALL aim modes: first-person (gjAimMode
// 0 / gjFirstPerson), Z-target (gjAimMode 1), and static (gjAimMode 2).
// Independent of the NeiAimCycle CVar — the user explicitly asked: while
// gust jar is in play, R never shields. Period.
void GustJar_FilterPlayerInput(Input* input) {
    if (input == NULL) {
        return;
    }
    if (!gjEquipped) {
        return;
    }
    input->cur.button &= ~(BTN_L | BTN_R);
    input->press.button &= ~(BTN_L | BTN_R);
    input->rel.button &= ~(BTN_L | BTN_R);
}
