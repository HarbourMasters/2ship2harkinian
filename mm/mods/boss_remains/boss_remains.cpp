/**
 * boss_remains.cpp - The four boss remains as custom wearable "masks" (Skijer's NEI).
 *
 * See boss_remains.h. Summary of the three phases wired here:
 *
 *  Phase 1 (equip): BossRemains_TryEquipAtCursor is called from the MM kaleido
 *  quest page (z_kaleido_collect.c) when the cursor is on a remains. The remains
 *  are plain u8 items (0x5D..0x60), so they go straight into buttonItems /
 *  dpadItems — the same direct idiom the medallion equip block uses. The HUD icon
 *  needs no special-casing (gItemIcons[] already has the remains textures and the
 *  ids are < ITEM_F0, so ITEM_IS_DRAWABLE_ON_BUTTON already allows them).
 *
 *  Phase 2 (wear): BossRemains_TickInput (from Player_UpdateCommon) watches for a
 *  press on whichever C/D-pad button holds a remains and toggles it on/off Link's
 *  face. BossRemains_DrawWornMask (from Player_PostLimbDrawGameplay at the head
 *  limb) draws the Moon Child's face-fitted mask DL (gMoonChild*MaskDL, object_ob)
 *  — the exact model the moon children wear — using the moon child's own per-mask
 *  scale/rotate/translate (z_en_js.c). In 2ship an OTR path string can be passed
 *  straight to gSPDisplayList, so no object needs to be spawned/bound.
 *
 *  Phase 3 (action, TEST): while a remains is worn, A does the remains' custom
 *  action instead of the roll. For now BossRemains_TryActionA is a single audible
 *  placeholder that proves the hook fires; the real per-remains actions + friendly
 *  ally summons come next.
 */

#include "boss_remains.h"

#include <libultraship/bridge.h> // CVarGetInteger

// OPEN_DISPS / CLOSE_DISPS redeclare these two symbols inline at each call site; in a C++ TU that
// takes C++ linkage unless a C declaration exists at file scope. Force the C symbols (same trick as
// spiritual_stones.cpp / PropHunt.cpp) so the macro's redeclaration matches and links.
extern "C" {
void FrameInterpolation_RecordOpenChild(const void* a, int b);
void FrameInterpolation_RecordCloseChild(void);
}

extern "C" {
#include "z64.h"
#include "functions.h"
#include "macros.h"
#include "variables.h"
extern SaveContext gSaveContext;
// Not in functions.h (z_kaleido_collect.c forward-declares them locally too): the C-button /
// D-pad HUD icon refreshers used after an equip.
void Interface_LoadItemIconImpl(PlayState* play, u8 btn);
void Interface_Dpad_LoadItemIcon(PlayState* play, u8 btn);
// Majora's chant streak textures (Goht bull-charge cone).
#include "objects/object_stk2/object_stk2.h"
// EnWaterEffect types — Gyorg's REAL water funnel (whirlpool visual, 1:1).
#include "overlays/actors/ovl_En_Water_Effect/z_en_water_effect.h"

// ── Phase 3 summon allies ───────────────────────────────────────────────────
// Unity-#include the friendly-ally source files INSIDE this extern "C" block (like
// spiritual_stones.cpp #includes ../actors/spiritual_stone_statue.c) so their ActorProfile /
// RemainsAlly*_Spawn symbols get C linkage and match the DEFINE_ACTOR rows in actor_table.h
// (0x2BE bug / 0x2BF chu / 0x2C0 fish). remains_ally_common.c MUST precede the three actors:
// they call RemainsAlly_FindNearestEnemy / _HomeTowardPos / _FollowPlayer, defined there. None
// of these are in CMake/vcxproj — they compile as part of this boss_remains.cpp TU.
#include "actors/remains_ally_common.h"
#include "actors/remains_ally_common.c"
#include "actors/remains_ally_bug.c"
#include "actors/remains_ally_chu.c"
#include "actors/remains_ally_fish.c"
#include "actors/remains_ally_link.c"
}

// ============================================================================
// State + config
// ============================================================================

namespace {

// Master toggle. Default ON so the feature works out of the box; a menu entry can
// gate it later. When OFF the remains behave as inert quest items.
inline bool RemainsEnabled() {
    return CVarGetInteger("gMods.BossRemains.Enabled", 1) != 0;
}

// The remains currently worn on Link's face (ITEM_REMAINS_*), or ITEM_NONE.
// Transient per-session state (like the native currentMask "on" state).
s16 sWornRemains = ITEM_NONE;

// Odolwa A-action: while the Odolwa remains is worn, Link's roll is suppressed (func_80836B3C) and he
// just RUNS — human locomotion is always 1.5x (BossRemains_RunSpeedMul, read in Player_Action_13), the
// red trail draws while moving, and Odolwa footstep SFX play on a cadence. No toggle needed.
s16 sOdolwaRunSfxTimer = 0; // footstep-SFX cadence while running
s16 sOdolwaShieldFireTimer = 0; // cooldown between shield-deflect fire bursts
bool sOdolwaRunBoost = false; // true while HOLDING A (worn) → 2x run + purple trail

// Goht A-action state:
//   A held        → BULL CHARGE: cl_nigeru anim + Majora-red cone + 3x forward speed. Drains 1 magic per
//                   15 frames (Pegasus-dash cadence); with NO magic it still charges, it just loses the
//                   contact damage + the cone. Goron-roll terrain: it ignores ledges/slopes and ramps
//                   launch Link. Crashing into a wall detonates a POWDER KEG ("bonk to break stuff").
//   B held        → CHARGED GOHT THUNDER: hold to charge (light-orb VFX grows), release to fire at the
//                   enemy most in front. Charge scales reach + damage. Costs magic. Pierces walls.
//   R + A         → GORON QUAKE POUND: jumpAT anim + hop; on landing a small earthquake (camera quake +
//                   radial DMG_GORON_POUND burst) then jumpATend.
//   R + B         → bombchu toggle: throw ONE friendly bombchu (one at a time); press R+B AGAIN while it's
//                   out to detonate it manually. It runs its own Real-Bombchu AI (hunts enemies, climbs any
//                   wall/ceiling, wanders when there's no foe). Never targets Link.
bool sGohtCharging = false;   // A held → bull charge
s16 sGohtChargeCooldown = 0;  // lockout after a crash
s16 sGohtSfxTimer = 0;        // hoof-stomp cadence while charging
s16 sGohtQuakeState = 0;      // 0=idle, 1=airborne (jumpAT), 2=landed (jumpATend + damage burst)
s16 sGohtQuakeDmgFrames = 0;  // frames the radial quake AT stays live
s16 sGohtQuakeAnimDelay = 0;  // frames to wait after leaving the ground before playing jumpAT (rocscape-style)
s16 sGohtJumpPlayTimer = 0;   // frames jumpAT is allowed to play before we FREEZE it on its last frame
s16 sGohtRecoverFrames = 0;   // after the pound: force a human idle so the Zora jumpAT pose doesn't linger
f32 sGohtChargeAnimFrame = 0.0f; // hand-driven cl_nigeru cycle frame (we OWN the pose while charging)
ColliderCylinder sGohtQuakeCollider;
bool sGohtQuakeColReady = false; // lazy Collider_InitCylinder done
ColliderCylinder sGohtChargeCollider; // light goron-punch hit box while charging
bool sGohtChargeColReady = false;
bool sGohtSwordStashed = false; // true while the B-button sword is unequipped (stashed) for Goht
bool sGohtNoSnapOwned = false;  // true while WE own BGCHECKFLAG_PLAYER_800 (bull-charge ledge/slope ignore)
s16 sGohtChargeMagicTick = 0;   // bull-charge magic drain counter (Pegasus-style dedicated tick)
s16 sGohtThunderCharge = 0;     // B-hold charge level (frames) → scales the bolt's distance + damage
bool sGohtThunderCharging = false; // true while B (no R) is held (drives the charging light-orb VFX)

constexpr f32 kGohtChargeSpeed = 18.0f;  // ~3x a normal run
// Bull charge drains magic OVER TIME, 1:1 with the Pegasus Anklet dash (equip_pegasus.c): 1 magic every
// 15 frames off a DEDICATED tick counter (not gameplayFrames%, so the cadence is identical no matter when
// the charge starts). Like the Pegasus, running dry does NOT stop the charge — it just loses the
// magic-powered parts (the contact-damage collider and the cone).
constexpr s16 kGohtChargeMagicInterval = 15;
constexpr f32 kGohtChargeAnimSpeed = 1.4f; // cl_nigeru advance per frame (8-frame loop)
constexpr s16 kGohtTurnStep = 0x2AA;     // ~3.7°/frame — stiff bull steering, harder to turn than a Goron roll
constexpr f32 kGohtJumpVel = 15.5f;      // high, snappy pound hop
constexpr f32 kGohtJumpGravityBoost = 5.0f; // extra downward accel past the apex → fast, snappy descent
constexpr s16 kGohtJumpAnimDelay = 3;    // rocscape-style: play jumpAT a few frames AFTER leaving the ground
constexpr f32 kGohtBounceSpeed = -16.0f; // wall-crash recoil (backwards)
constexpr f32 kGohtBounceHop = 6.0f;     // wall-crash recoil little hop
constexpr s16 kGohtThunderMagicCost = 4;   // magic per thunder shot (flat, deducted directly like Odolwa)
constexpr s16 kGohtThunderChargeMax = 45;  // frames of R+A hold for a full charge
constexpr s16 kGohtThunderChargeMin = 5;   // minimum charge before a shot will fire (anti-spam)
constexpr s16 kGohtThunderTtlMin = 12;     // bolt lifetime → distance at min charge
constexpr s16 kGohtThunderTtlMax = 64;     // ... at full charge
constexpr s16 kGohtThunderDmgMin = 2;      // bolt damage at min charge
constexpr s16 kGohtThunderDmgMax = 10;     // ... at full charge

inline bool IsRemainsItem(s16 item) {
    return (item >= ITEM_REMAINS_ODOLWA) && (item <= ITEM_REMAINS_TWINMOLD);
}

// Item id → 0..3 index (Odolwa/Goht/Gyorg/Twinmold).
inline s32 RemainsIndex(s16 item) {
    return item - ITEM_REMAINS_ODOLWA;
}

inline bool DpadEquipsEnabled() {
    return CVarGetInteger("gEnhancements.Dpad.DpadEquips", 0) != 0;
}

// ── Worn-mask draw data ────────────────────────────────────────────────────
// The face-fitted geometry is the Moon Child's masks. In 2ship a resource path
// string can be handed straight to gSPDisplayList (Ship resolves + patches the
// DL's internal Vtx/texture refs), so we reference object_ob by OTR path with no
// object spawn/segment bind. Index: 0=Odolwa 1=Goht 2=Gyorg 3=Twinmold.
const char* const kMaskDLPath[4] = {
    "__OTR__objects/object_ob/gMoonChildOdolwasMaskDL",
    "__OTR__objects/object_ob/gMoonChildGohtsMaskDL",
    "__OTR__objects/object_ob/gMoonChildGyorgsMaskDL",
    "__OTR__objects/object_ob/gMoonChildTwinmoldsMaskDL",
};

// Per-mask transform, copied verbatim from z_en_js.c (D_8096ABE0/ABF4/AC08/AC1C,
// entries 1..4 — index 0 there is Majora's, which we skip). These are authored
// for the Moon Child's head node; Link's head node has a different base scale, so
// kExtraScale / the offsets below are the knobs to tune visually if the mask sits
// off the face. Rotation is the moon child's fixed ZYX(0, -0x4000, -0x36B0).
const f32 kMaskScale[4] = { 0.5f, 0.5f, 0.48f, 0.45f };
const f32 kMaskTransX[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
const f32 kMaskTransY[4] = { 1400.0f, 1470.0f, 1670.0f, 1470.0f };
const f32 kMaskTransZ[4] = { 700.0f, 900.0f, 900.0f, 900.0f };

// Base mask orientation = the Moon Child's fixed ZYX(0, -0x4000, -0x36B0). Link's head node is
// rolled ~90° in Z relative to the moon child's, so we add a global rotation correction on top.
// 0x4000 == 90° in binang. If the mask ends up rolled the OTHER way, flip kExtraRotZ to -0x4000.
// Worn-mask placement on Link's head — final values tuned in-game via the (now-removed) menu sliders.
// Rotation is in degrees; the offset is ADDED to each mask's per-mask base translate; the scale
// MULTIPLIES each mask's per-mask base scale. Shared by all four remains: the per-mask base arrays
// above already carry the moon child's per-remain deltas, so one global tweak keeps them consistent.
constexpr s32 kRotXDeg = 180;
constexpr s32 kRotYDeg = -90;
constexpr s32 kRotZDeg = 15;
constexpr f32 kOffX = 0.0f;
constexpr f32 kOffY = -510.0f;
constexpr f32 kOffZ = 383.0f;
constexpr f32 kScaleMul = 1.04f;

inline s16 DegToBinang(s32 deg) {
    return (s16)((deg * 0x10000) / 360);
}

} // namespace

// ============================================================================
// Phase 1 — equip from the kaleido quest page
// ============================================================================

extern "C" s32 BossRemains_TryEquipAtCursor(PlayState* play, Input* input) {
    if (!RemainsEnabled() || play == nullptr || input == nullptr) {
        return false;
    }

    s16 item = play->pauseCtx.cursorItem[PAUSE_QUEST];
    if (!IsRemainsItem(item)) {
        return false;
    }
    // Must actually own it (the quest bit), so an empty slot can't be equipped.
    if (!CHECK_QUEST_ITEM(QUEST_REMAINS_ODOLWA + RemainsIndex(item))) {
        return false;
    }

    // C-button equip (remains are u8, so the direct BUTTON_ITEM_EQUIP idiom — same as medallions).
    s16 cBtn = -1;
    if (CHECK_BTN_ALL(input->press.button, BTN_CLEFT)) {
        cBtn = EQUIP_SLOT_C_LEFT;
    } else if (CHECK_BTN_ALL(input->press.button, BTN_CDOWN)) {
        cBtn = EQUIP_SLOT_C_DOWN;
    } else if (CHECK_BTN_ALL(input->press.button, BTN_CRIGHT)) {
        cBtn = EQUIP_SLOT_C_RIGHT;
    }
    if (cBtn != -1) {
        BUTTON_ITEM_EQUIP(0, cBtn) = (u8)item;
        C_SLOT_EQUIP(0, cBtn) = 0xFF; // not from an inventory slot
        Interface_LoadItemIconImpl(play, (u8)cBtn);
        Audio_PlaySfx(NA_SE_SY_DECIDE);
        return true;
    }

    // D-pad equip (only when the DpadEquips enhancement is on, matching the item/quest pages).
    if (DpadEquipsEnabled()) {
        s16 dBtn = -1;
        if (CHECK_BTN_ALL(input->press.button, BTN_DRIGHT)) {
            dBtn = EQUIP_SLOT_D_RIGHT;
        } else if (CHECK_BTN_ALL(input->press.button, BTN_DLEFT)) {
            dBtn = EQUIP_SLOT_D_LEFT;
        } else if (CHECK_BTN_ALL(input->press.button, BTN_DDOWN)) {
            dBtn = EQUIP_SLOT_D_DOWN;
        } else if (CHECK_BTN_ALL(input->press.button, BTN_DUP)) {
            dBtn = EQUIP_SLOT_D_UP;
        }
        if (dBtn != -1) {
            DPAD_BUTTON_ITEM_EQUIP(0, dBtn) = (u8)item;
            Interface_Dpad_LoadItemIcon(play, (u8)dBtn);
            Audio_PlaySfx(NA_SE_SY_DECIDE);
            return true;
        }
    }

    return false;
}

// ============================================================================
// Phase 2 — wear toggle + face draw
// ============================================================================

extern "C" s16 BossRemains_GetWorn(void) {
    return sWornRemains;
}

extern "C" void BossRemains_ClearWorn(void) {
    sWornRemains = ITEM_NONE;
}

// ============================================================================
// Phase 3 — per-remains friendly ally summon driver
// ============================================================================
//   Odolwa (idx 0): SHIELD (R) + B  → spawn a small swarm of friendly beetles near Link (magic).
//   Goht   (idx 1): SHIELD (R) + B  → friendly bombchu;  SHIELD (R) + A → thunder bolt (magic).
//   Gyorg  (idx 2): SHIELD (R) + B on DRY LAND → a few Desbreko-look fish that flop, bite close, and die.
// The RemainsAlly*_Spawn functions are unity-included above, so they are directly callable.

// Defined in the Goht block further down; TickSummons (Goht thunder aiming) needs it earlier.
static Actor* BossRemains_FindFrontEnemy(PlayState* play, Player* player, f32 maxDist, s16 cone);

// z_player.c internal (no header) — the intended movement yaw/speed from the stick+camera. We reuse it
// to steer the bull charge stiffly. `player` here is just the (arbitrary) param name in this declaration.
extern "C" s32 Player_GetMovementSpeedAndYaw(Player* player, f32* outSpeedTarget, s16* outYawTarget, f32 speedMode,
                                             PlayState* play);

// Spawn Gyorg's little fish school near Link (En_Tanron3). Shared by the Shield+B summon and the
// in-water "dive button" summon in z_player.c (func_8083B3B4) — pressing the dive input while swimming
// with Gyorg makes fish instead of diving. Fish self-manage (swim/dart in water, flop/bite/die on land).
extern "C" void BossRemains_GyorgSummonFish(PlayState* play, Player* player) {
    if (play == nullptr || player == nullptr || !BossRemains_IsGyorgWorn()) {
        return;
    }
    s16 yaw = player->actor.shape.rot.y;
    for (s32 i = 0; i < 3; i++) {
        Vec3f p = player->actor.world.pos;
        p.x += Rand_CenteredFloat(90.0f);
        p.z += Rand_CenteredFloat(90.0f);
        p.y += 12.0f;
        Actor_Spawn(&play->actorCtx, play, ACTOR_REMAINS_ALLY_FISH, p.x, p.y, p.z, 0, yaw, 0, (s16)i);
    }
    Actor_PlaySfx(&player->actor, NA_SE_EN_PIRANHA_ATTACK);
}

// ============================================================================
// GYORG — WHIRLPOOL (B while swimming): a stationary vortex that PULLS IN and DAMAGES nearby enemies
// for a few seconds (Gyorg's water-current trap). Damage via a friendly AT cylinder centered on the
// cast point (same friend/foe idea as the Goht quake / the fish); the pull nudges each enemy's world
// pos toward the center each frame. Visual = a spinning translucent-blue funnel.
// ============================================================================
static bool sGyorgWhirlpoolActive = false; // true while B is HELD (swimming) with magic to spend
static s16 sGyorgWhirlpoolDrain = 0;       // Deku-leaf-style magic-drain frame counter
static Vec3f sGyorgWhirlpoolPos;           // aim point — from the mask mouth along Link's facing + pitch
static ColliderCylinder sGyorgWhirlpoolCollider;
static bool sGyorgWhirlpoolColReady = false;

static constexpr f32 kGyorgWhirlpoolRadius = 220.0f; // pull + damage XZ radius
static constexpr f32 kGyorgWhirlpoolPull = 14.0f;    // pull speed toward the aim point per frame
static constexpr s16 kGyorgWhirlpoolDrainRate = 10;  // 1 magic every 10 frames — the Mogma Mitts pace
                                                     // (MITTS_MP_COST 1 / MITTS_DRAIN_INTERVAL 10)

// ── Funnel placement/sizing — tuned in-game, then baked (temp NEI sliders removed) ───────────────────
// Scale: EnWaterEffect derives its scale from rot.z (Actor_SetScale(rot.z * 0.0002)); Gyorg spawns his
// own at 0x78 (120). His body scale is 0.2 vs human Link's 0.01, so 10 reads correct on Link.
static constexpr s16 kGyorgFunnelScale = 10;
// The vanilla splash life decays the cone's LENGTH (unk_DC8[i].z) to 0 while growing its RADIUS
// (unk_DC8[i].y) to 20 — it deliberately flattens into a wide disc. We floor the length and cap the
// radius every frame to hold an actual CONE: long and narrow, like a spout.
static constexpr f32 kGyorgFunnelLength = 12.0f;
static constexpr f32 kGyorgFunnelRadius = 3.07f;
// Reach 0 = the spout forms AT the mask's mouth rather than out in front. Note this also puts the pull
// centre on Link himself, so the vortex drags enemies onto him — straight into the damage sphere.
static constexpr f32 kGyorgFunnelReach = 0.0f;
static constexpr f32 kGyorgFunnelOffsetY = -5.25f;
// The DL points straight UP, so 90° (0x4000) lays it along the aim direction.
static constexpr s16 kGyorgFunnelPitch = 0x4000;
static constexpr s16 kGyorgAuraRadius = 95;

static ColliderCylinderInit sGyorgWhirlpoolColliderInit = {
    {
        COL_MATERIAL_NONE,
        AT_ON | AT_TYPE_PLAYER, // friendly: hits enemies (AC_TYPE_PLAYER bumpers), never Link
        AC_NONE,
        OC1_NONE,
        OC2_NONE,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        { DMG_DEKU_STICK, 0x00, 0x02 }, // toucher: deku-stick class so enemy bumpers accept it; small dmg
        { 0xF7CFFFFF, 0x00, 0x00 },     // bumper mask inert (AC off)
        ATELEM_ON | ATELEM_SFX_NORMAL,
        ACELEM_NONE,
        OCELEM_NONE,
    },
    { 200, 120, -40, { 0, 0, 0 } },
};

// The protective "damage sphere" around LINK himself while the whirlpool is up: anything the vortex
// drags close keeps taking hits and never gets to touch him. Same friendly AT setup, centered on Link.
static ColliderCylinder sGyorgAuraCollider;
static bool sGyorgAuraColReady = false;

static ColliderCylinderInit sGyorgAuraColliderInit = {
    {
        COL_MATERIAL_NONE,
        AT_ON | AT_TYPE_PLAYER,
        AC_NONE,
        OC1_NONE,
        OC2_NONE,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        { DMG_DEKU_STICK, 0x00, 0x04 }, // hits harder than the vortex body — this is the "keep off" ring
        { 0xF7CFFFFF, 0x00, 0x00 },
        ATELEM_ON | ATELEM_SFX_NORMAL,
        ACELEM_NONE,
        OCELEM_NONE,
    },
    { kGyorgAuraRadius, 110, -45, { 0, 0, 0 } },
};

// Per-frame whirlpool driver: aim + magic drain + pull + damage. Runs while HELD (sGyorgWhirlpoolActive,
// set in TickSummons from B-held + magic). The vortex forms AHEAD of the mask's mouth along Link's facing
// + swim pitch, so steering Link (360° in the water) aims it; it drains magic Deku-leaf style and drags
// every nearby enemy into it.
static void BossRemains_GyorgWhirlpoolTick(PlayState* play, Player* player) {
    if (!BossRemains_IsGyorgWorn()) {
        sGyorgWhirlpoolActive = false; // doffed / not Gyorg → force off (TickSummons won't run its case)
    }
    if (!sGyorgWhirlpoolActive) {
        return;
    }

    // Aim: from Link's head (the worn mask's mouth) along his facing YAW + swim PITCH.
    // The pitch is unk_AAA, NOT shape.rot.x: the player's shape.rot.x is pinned to 0 everywhere
    // (z_player.c:662/4701/11878/20146 — it's even reused as a spawn param) and the swim tilt is applied
    // to the model from unk_AAA (z_player_lib.c:2603, Matrix_RotateXS(player->unk_AAA)). Reading
    // shape.rot.x gave a constant 0, so the spout never aimed up/down at all.
    s16 yaw = player->actor.shape.rot.y;
    s16 pitch = player->unk_AAA;
    f32 cosP = Math_CosS(pitch);
    sGyorgWhirlpoolPos.x = player->actor.focus.pos.x + (Math_SinS(yaw) * cosP * kGyorgFunnelReach);
    sGyorgWhirlpoolPos.y = player->actor.focus.pos.y - (Math_SinS(pitch) * kGyorgFunnelReach) + kGyorgFunnelOffsetY;
    sGyorgWhirlpoolPos.z = player->actor.focus.pos.z + (Math_CosS(yaw) * cosP * kGyorgFunnelReach);

    // Tip the (vertically-authored) funnel along the aim — a spout out of the mask's mouth — plus Link's
    // swim pitch. Never 0: that's the "vanilla, randomize yaw" sentinel in EnWaterEffect_Init.
    s16 funnelPitch = kGyorgFunnelPitch + pitch;
    if (funnelPitch == 0) {
        funnelPitch = 1;
    }

    // Glue every LIVE funnel to the current aim so the spout TURNS WITH LINK. Without this the effects
    // keep their spawn-time orientation (they live ~50 frames but respawn every 14), leaving a fan of
    // stale cones — which is exactly the "doesn't rotate with Link" symptom. EnWaterEffect is
    // ACTORCAT_BOSS; OURS are the ones with rot.x != 0 (vanilla Gyorg's are always 0).
    for (Actor* fx = play->actorCtx.actorLists[ACTORCAT_BOSS].first; fx != nullptr; fx = fx->next) {
        if ((fx->id == ACTOR_EN_WATER_EFFECT) && (fx->shape.rot.x != 0)) {
            fx->world.pos = sGyorgWhirlpoolPos;
            fx->shape.rot.x = funnelPitch;
            fx->shape.rot.y = yaw;

            // Hold the CONE. The draw does Matrix_Scale(unk_DC8[i].y, unk_DC8[i].z, unk_DC8[i].y), so
            // .z is the cone's height and .y its radius — and the vanilla splash update drives height
            // to 0 while pushing radius to 20, which is precisely "flat with a huge centre". Clamp
            // both back every frame so it stays a spout.
            EnWaterEffect* wfx = (EnWaterEffect*)fx;
            for (s32 i = 1; i < 4; i++) {
                if (wfx->unk_DC8[i].z < kGyorgFunnelLength) {
                    wfx->unk_DC8[i].z = kGyorgFunnelLength;
                }
                if (wfx->unk_DC8[i].y > kGyorgFunnelRadius) {
                    wfx->unk_DC8[i].y = kGyorgFunnelRadius;
                }
            }
        }
    }

    // Deku-leaf-style drain: 1 magic every kGyorgWhirlpoolDrainRate frames (running-out is handled in
    // TickSummons, which clears sGyorgWhirlpoolActive when magic hits 0).
    if (++sGyorgWhirlpoolDrain >= kGyorgWhirlpoolDrainRate) {
        sGyorgWhirlpoolDrain = 0;
        if (gSaveContext.save.saveInfo.playerData.magic > 0) {
            gSaveContext.save.saveInfo.playerData.magic--;
        }
    }

    if (!sGyorgWhirlpoolColReady) {
        Collider_InitCylinder(play, &sGyorgWhirlpoolCollider);
        Collider_SetCylinder(play, &sGyorgWhirlpoolCollider, &player->actor, &sGyorgWhirlpoolColliderInit);
        sGyorgWhirlpoolColReady = true;
    }
    // Keep the AT cylinder live at the aim point. RE-ARM it every frame (set AT_ON, clear the sticky
    // AT_HIT): once an AT lands a hit the engine latches AT_HIT and it stops connecting, so without
    // this the vortex only ever damaged once instead of ticking — the Gust Jar does the same.
    sGyorgWhirlpoolCollider.dim.pos.x = (s16)sGyorgWhirlpoolPos.x;
    sGyorgWhirlpoolCollider.dim.pos.y = (s16)sGyorgWhirlpoolPos.y;
    sGyorgWhirlpoolCollider.dim.pos.z = (s16)sGyorgWhirlpoolPos.z;
    sGyorgWhirlpoolCollider.base.atFlags |= AT_ON | AT_TYPE_PLAYER;
    sGyorgWhirlpoolCollider.base.atFlags &= ~AT_HIT;
    CollisionCheck_SetAT(play, &play->colChkCtx, &sGyorgWhirlpoolCollider.base);

    // The damage sphere on LINK: whatever the vortex drags in keeps getting hit and can't reach him.
    // Live every frame, so contact damage repeats as fast as each enemy's own i-frames allow.
    if (!sGyorgAuraColReady) {
        Collider_InitCylinder(play, &sGyorgAuraCollider);
        Collider_SetCylinder(play, &sGyorgAuraCollider, &player->actor, &sGyorgAuraColliderInit);
        sGyorgAuraColReady = true;
    }
    Collider_UpdateCylinder(&player->actor, &sGyorgAuraCollider);
    sGyorgAuraCollider.base.atFlags |= AT_ON | AT_TYPE_PLAYER;
    sGyorgAuraCollider.base.atFlags &= ~AT_HIT; // re-arm so the ring keeps ticking, not one-and-done
    CollisionCheck_SetAT(play, &play->colChkCtx, &sGyorgAuraCollider.base);

    if ((play->gameplayFrames & 7) == 0) {
        Actor_PlaySfx(&player->actor, NA_SE_EV_DIVE_INTO_WATER);
    }

    // The visual: Gyorg's REAL water funnel, 1:1 DL + texture. We spawn PRIMARY_SPRAY — the boss's
    // "large funnel of water" — DIRECTLY rather than RIPPLES: RIPPLES is the flat surface-ripple ring
    // and it auto-spawns its own spray children we'd have no handle on, so this gives one clean cone we
    // fully own. Scale is Link-proportioned (kGyorgFunnelScale), not Gyorg's 0x78. It's a burst effect
    // that fades on its own, so refresh one every 14 frames while held for a sustained, steerable spout.
    // Outside Gyorg's arena OBJECT_WATER_EFFECT isn't resident and Actor_Spawn NULLs; load it with
    // Object_SpawnPersistent and the next refresh sticks (the FleetWarpDoor idiom).
    if ((play->gameplayFrames % 14) == 0) {
        // rot.x = funnelPitch marks it as OURS (aimed) so Init keeps the yaw instead of randomizing it,
        // the draw tips it, and the follow-loop above re-aims + reshapes it every frame.
        if (Actor_Spawn(&play->actorCtx, play, ACTOR_EN_WATER_EFFECT, sGyorgWhirlpoolPos.x, sGyorgWhirlpoolPos.y,
                        sGyorgWhirlpoolPos.z, funnelPitch, yaw, kGyorgFunnelScale,
                        ENWATEREFFECT_TYPE_GYORG_PRIMARY_SPRAY) == NULL) {
            Object_SpawnPersistent(&play->objectCtx, OBJECT_WATER_EFFECT);
        }
    }

    // Gust-jar-style trap on every enemy in range: PULL toward the vortex + PARALYZE + (via the AT
    // cylinder above) remote damage.
    //  - Paralyze = PULSED freezeTimer (frozen 7 of every 10 frames): a frozen actor skips its update
    //    (z_actor.c:2977), which is exactly what lets the pull win against enemies that pin/reset their
    //    own position every frame (e.g. patrol Guays) — but a frozen actor also never re-registers its
    //    AC collider, so a permanent freeze would make it UNDAMAGEABLE. The 3 free frames let it
    //    register colliders (vortex AT hits land) without giving it real control back.
    //  - The pull is a position nudge (the Gust Jar idiom): "try if possible" — anything the engine
    //    hard-anchors simply doesn't move, and the remote AT damage still applies.
    for (s32 cat = 0; cat < 2; cat++) {
        Actor* a = play->actorCtx.actorLists[(cat == 0) ? ACTORCAT_ENEMY : ACTORCAT_BOSS].first;
        for (; a != nullptr; a = a->next) {
            if (a->update == nullptr) {
                continue;
            }
            // Skip our own water funnels: EnWaterEffect is ACTORCAT_BOSS, so this loop was grabbing the
            // whirlpool's OWN visual as if it were an enemy — freezing it (killing its animation),
            // tinting it blue and dragging it around. That alone made the effect look broken.
            if (a->id == ACTOR_EN_WATER_EFFECT) {
                continue;
            }
            f32 d = Math_Vec3f_DistXZ(&a->world.pos, &sGyorgWhirlpoolPos);
            if ((d < kGyorgWhirlpoolRadius) && (d > 1.0f)) {
                s16 pullYaw = Math_Vec3f_Yaw(&a->world.pos, &sGyorgWhirlpoolPos); // toward the vortex
                a->world.pos.x += Math_SinS(pullYaw) * kGyorgWhirlpoolPull;
                a->world.pos.z += Math_CosS(pullYaw) * kGyorgWhirlpoolPull;
                // Y too, so fliers get dragged down/up INTO the vortex, not just sideways.
                Math_ApproachF(&a->world.pos.y, sGyorgWhirlpoolPos.y, 0.3f, kGyorgWhirlpoolPull * 0.6f);
                // Paralyze (near-continuous: frozen 14 of every 15 frames) + blue tint. The old 7/10
                // pulse left 3 free frames per cycle — enough for position-pinning AIs (patrol fliers)
                // to snap all the way back, which read as "no attraction". One free frame per 15 still
                // lets the actor register its AC collider so the vortex AT damage lands, but its AI can
                // no longer undo the drag. freezeTimer=2 → DECR leaves 1 → frozen this frame.
                if ((play->gameplayFrames % 15) != 0) {
                    a->freezeTimer = 2;
                }
                Actor_SetColorFilter(a, COLORFILTER_COLORFLAG_BLUE, 180, COLORFILTER_BUFFLAG_OPA, 10);
            }
        }
    }
}

// The whirlpool's look is now Gyorg's REAL water funnel (EnWaterEffect actors respawned each ~14
// frames in the tick above), so the old hand-made blue funnel is gone. Kept as a no-op so the
// z_player.c draw hook / header ABI stays stable.
extern "C" void BossRemains_DrawGyorgWhirlpool(Player* player, PlayState* play) {
    (void)player;
    (void)play;
}

static void BossRemains_TickSummons(PlayState* play, Player* player) {
    if (!IsRemainsItem(sWornRemains)) {
        return;
    }

    s32 idx = RemainsIndex(sWornRemains);
    Input* in = &play->state.input[0];
    s16 yaw = player->actor.shape.rot.y;

    switch (idx) {
        case 0: // Odolwa — friendly beetles on SHIELD (R) + B. Costs magic (like the sword-moth beam).
            if (CHECK_BTN_ALL(in->cur.button, BTN_R) && CHECK_BTN_ALL(in->press.button, BTN_B) &&
                gSaveContext.save.saveInfo.playerData.isMagicAcquired &&
                (gSaveContext.save.saveInfo.playerData.magic >= 6)) {
                // Deduct magic DIRECTLY, not via Magic_Consume: its magicState machine collides with
                // the spell system (blocks casting) and only consumes when magicState is idle (so it
                // charged inconsistently). We already gated on magic >= 6 above, so this can't go negative.
                gSaveContext.save.saveInfo.playerData.magic -= 6;
                for (s32 i = 0; i < 6; i++) {
                    Vec3f p = player->actor.world.pos;
                    p.x += Rand_CenteredFloat(70.0f);
                    p.y += 15.0f + Rand_ZeroFloat(30.0f); // chest height so they visibly pop out
                    p.z += Rand_CenteredFloat(70.0f);
                    RemainsAllyBug_Spawn(play, &p, (s16)(s32)Rand_CenteredFloat(60000.0f));
                }
                Actor_PlaySfx(&player->actor, NA_SE_EN_MIBOSS_VOICE2_OLD);
            }
            break;
        case 1: // Goht — SHIELD(R)+B = friendly bombchu; B (no R) held = charged thunder bolt (magic, pierces walls)
            // R+B is the bombchu toggle: throw ONE (only if none of ours is out), then press R+B AGAIN while
            // it's alive to detonate it manually right where it is.
            if (CHECK_BTN_ALL(in->cur.button, BTN_R) && CHECK_BTN_ALL(in->press.button, BTN_B)) {
                if (RemainsAllyChu_IsAlive()) {
                    RemainsAllyChu_DetonateActive(play);
                } else {
                    Vec3f p = player->actor.world.pos;
                    p.x += Math_SinS(yaw) * 30.0f;
                    p.z += Math_CosS(yaw) * 30.0f;
                    RemainsAllyChu_Spawn(play, &p, yaw);
                    Actor_PlaySfx(&player->actor, NA_SE_EN_BOMCHU_AIM);
                }
            }
            // B (no R) = CHARGED thunder (like the real Goht). HOLD B to charge — a Goht light-orb grows in
            // front of Link; the longer you charge the FARTHER and HARDER the bolt (anti-spam). Release to fire.
            {
                bool canCast = gSaveContext.save.saveInfo.playerData.isMagicAcquired &&
                               (gSaveContext.save.saveInfo.playerData.magic >= kGohtThunderMagicCost);
                bool holding = CHECK_BTN_ALL(in->cur.button, BTN_B) && !CHECK_BTN_ALL(in->cur.button, BTN_R);

                if (holding && canCast) {
                    if (sGohtThunderCharge < kGohtThunderChargeMax) {
                        sGohtThunderCharge++;
                    }
                    sGohtThunderCharging = true;
                    // Goht's charging thunder loop (crackle while it winds up).
                    Actor_PlaySfx(&player->actor, NA_SE_EN_COMMON_THUNDER - SFX_FLAG);
                } else {
                    // Released (or ran out of magic) → fire if we charged enough and can still pay for it.
                    if (sGohtThunderCharging && (sGohtThunderCharge >= kGohtThunderChargeMin) && canCast) {
                        // Deduct magic DIRECTLY (not Magic_Consume — its magicState machine blocks the ocarina/
                        // spell system; same reason Odolwa's beetle deducts directly). Gated on magic >= cost.
                        gSaveContext.save.saveInfo.playerData.magic -= kGohtThunderMagicCost;

                        f32 t = (f32)sGohtThunderCharge / (f32)kGohtThunderChargeMax; // 0..1
                        s16 ttl = kGohtThunderTtlMin + (s16)(t * (kGohtThunderTtlMax - kGohtThunderTtlMin));
                        s16 dmg = kGohtThunderDmgMin + (s16)(t * (kGohtThunderDmgMax - kGohtThunderDmgMin));

                        // Aim at the enemy most in front of Link (if any); else straight ahead.
                        s16 boltYaw = yaw;
                        Actor* front = BossRemains_FindFrontEnemy(play, player, 900.0f, 0x5000);
                        if (front != NULL) {
                            boltYaw = Actor_WorldYawTowardActor(&player->actor, front);
                        }
                        // Fire from the raised shield (just ahead of Link, chest height).
                        Vec3f p = player->actor.world.pos;
                        p.x += Math_SinS(yaw) * 22.0f;
                        p.z += Math_CosS(yaw) * 22.0f;
                        p.y += 40.0f;
                        RemainsAllyBug_SpawnThunderCharged(play, &p, boltYaw, ttl, dmg);
                        Actor_PlaySfx(&player->actor, NA_SE_EN_COMMON_THUNDER_THR); // Goht's real thunder-shoot sfx
                    }
                    sGohtThunderCharge = 0;
                    sGohtThunderCharging = false;
                }
            }
            break;

        case 2: // Gyorg — Zora-tunic swim (Nei_IsZoraSwim). The Zora water barrier is repurposed: while
                // SWIMMING, R alone summons Gyorg's fish (barrier disabled in func_8082F164); ON LAND,
                // R + B summons them. Fish swim+dart in water, flop+bite+die on land (remains_ally_fish.c).
            if (player->actor.depthInWater > 0.0f) {
                // Swimming: R (repurposed barrier) = fish; B HELD = the whirlpool (drains magic while up).
                if (CHECK_BTN_ALL(in->press.button, BTN_R)) {
                    BossRemains_GyorgSummonFish(play, player);
                }
                sGyorgWhirlpoolActive =
                    CHECK_BTN_ALL(in->cur.button, BTN_B) && (gSaveContext.save.saveInfo.playerData.magic > 0);
            } else {
                sGyorgWhirlpoolActive = false;
                // On land: R + B = fish.
                if (CHECK_BTN_ALL(in->cur.button, BTN_R) && CHECK_BTN_ALL(in->press.button, BTN_B)) {
                    BossRemains_GyorgSummonFish(play, player);
                }
            }
            break;

        case 3: // Twinmold — a DARK LINK companion walks with you for as long as the remains is worn. It
                // self-maintains: if it isn't out (first frame worn, or it got culled), respawn it beside
                // Link. It fights on its own using whatever weapons the save says Link owns.
            if (!RemainsAllyLink_IsAlive()) {
                Vec3f p = player->actor.world.pos;
                p.x += Math_SinS(yaw) * -60.0f; // step in just behind Link
                p.z += Math_CosS(yaw) * -60.0f;
                RemainsAllyLink_Spawn(play, &p, yaw);
            }
            break;

        default:
            break;
    }
}

// Magic drained per sword-swing moth projectile.
static constexpr s16 kOdolwaMothMagicCost = 2;

// Sword swing with magic → a moth projectile flies forward (toward the lock-on target, else Link's
// facing) like FD's sword beam. Called from z_player.c func_80833864 (every melee-attack setup).
extern "C" void BossRemains_OdolwaSwordMoth(PlayState* play, Player* player) {
    if (!BossRemains_IsOdolwaWorn() || play == nullptr || player == nullptr) {
        return;
    }
    // SHIELD (R) held = the R+B bug summon, not a normal swing — no beam / no magic drain then.
    if (CHECK_BTN_ALL(play->state.input[0].cur.button, BTN_R)) {
        return;
    }
    if (!gSaveContext.save.saveInfo.playerData.isMagicAcquired ||
        (gSaveContext.save.saveInfo.playerData.magic < kOdolwaMothMagicCost)) {
        return;
    }
    // Direct deduction (see the R+B beetle note): Magic_Consume's state machine fights the spell
    // system and only fires when idle. We gated on magic >= cost above, so this stays >= 0.
    gSaveContext.save.saveInfo.playerData.magic -= kOdolwaMothMagicCost;

    Vec3f pos = player->bodyPartsPos[PLAYER_BODYPART_WAIST];
    s16 yaw = player->actor.shape.rot.y;
    if (player->focusActor != NULL) {
        yaw = Actor_WorldYawTowardActor(&player->actor, player->focusActor);
    }
    RemainsAllyBug_SpawnProjectile(play, &pos, yaw);
    Actor_PlaySfx(&player->actor, NA_SE_EN_MB_MOTH_FLY);
}

// Odolwa's shield deflects an attack → a defensive fire burst at Link (small Din's-fire actor), with a
// cooldown so a sustained block doesn't spam it.
static void BossRemains_OdolwaShieldFire(PlayState* play, Player* player) {
    if (sOdolwaShieldFireTimer > 0) {
        return;
    }
    sOdolwaShieldFireTimer = 30;
    // Summon the Fire Medallion effect (our SW97 ground-fire wave) at Link. ACTOR_SW97_MAGIC_FIRE is
    // GAMEPLAY_KEEP (spawns anywhere) and branches to the Fire-Medallion visual by its actor id.
    Vec3f p = player->actor.world.pos;
    Actor_Spawn(&play->actorCtx, play, ACTOR_SW97_MAGIC_FIRE, p.x, p.y, p.z, 0, player->actor.shape.rot.y, 0, 0);
}

// ============================================================================
// GOHT — bull charge (A held), quake pound (B), thunder (R+A), one friendly bombchu (R+B)
// ============================================================================

extern "C" s32 BossRemains_IsGohtWorn(void) {
    return (RemainsEnabled() && (sWornRemains == ITEM_REMAINS_GOHT)) ? 1 : 0;
}

// True while the bull charge is running. Read by z_player.c's walk-off handler (func_8083827C), which
// bails out for a charging Goht exactly like it already does for the Goron roll (Player_Action_96) — that
// early-out is what stops the fall action / auto-hop / ledge-grab from eating the launch, so leaving a
// ledge or ramp at charge speed sends Link flying instead of just dropping him back into a normal run.
extern "C" s32 BossRemains_IsGohtCharging(void) {
    return sGohtCharging ? 1 : 0;
}

// True whenever Gyorg's remains is worn — drives human Link's Zora-style free 3D dive/swim, current
// immunity, "can't walk in water", and damage resilience (all gated in z_player.c on THIS + being in
// water). The in-water test lives at each z_player.c call site (depthInWater / BGCHECKFLAG_WATER).
extern "C" s32 BossRemains_IsGyorgWorn(void) {
    return (RemainsEnabled() && (sWornRemains == ITEM_REMAINS_GYORG)) ? 1 : 0;
}

// Roll suppression: both Odolwa (runs instead) and Goht (A = bull charge) take over A.
extern "C" s32 BossRemains_SuppressRoll(void) {
    return BossRemains_IsOdolwaWorn() || BossRemains_IsGohtWorn();
}

// Radial quake-pound damage: a fat AT cylinder centered on Link for a few frames after landing.
// DMG_GORON_POUND so it reads as the Goron pound hit class.
static ColliderCylinderInit sGohtQuakeColliderInit = {
    {
        COL_MATERIAL_NONE,
        AT_ON | AT_TYPE_PLAYER,
        AC_NONE,
        OC1_NONE,
        OC2_NONE,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        { DMG_GORON_POUND, 0x00, 0x04 },
        { 0xF7CFFFFF, 0x00, 0x00 },
        ATELEM_ON | ATELEM_SFX_NORMAL,
        ACELEM_NONE,
        OCELEM_NONE,
    },
    { 110, 60, -20, { 0, 0, 0 } },
};

// Light bull-charge contact damage: a small AT cylinder on Link while charging. Goron-PUNCH hit class but
// only 1 damage ("daño tipo goron punch pero leve de quantity 1").
static ColliderCylinderInit sGohtChargeColliderInit = {
    {
        COL_MATERIAL_NONE,
        AT_ON | AT_TYPE_PLAYER,
        AC_NONE,
        OC1_NONE,
        OC2_NONE,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        { DMG_GORON_PUNCH, 0x00, 0x01 },
        { 0xF7CFFFFF, 0x00, 0x00 },
        ATELEM_ON | ATELEM_SFX_NORMAL,
        ACELEM_NONE,
        OCELEM_NONE,
    },
    { 45, 60, 0, { 0, 0, 0 } },
};

// The enemy most IN FRONT of Link: smallest |yaw difference| to Link's facing within range/cone.
// Returns NULL if none qualifies.
static Actor* BossRemains_FindFrontEnemy(PlayState* play, Player* player, f32 maxDist, s16 cone) {
    Actor* best = NULL;
    s16 bestAbs = cone;
    for (s32 cat = 0; cat < 2; cat++) {
        Actor* a = play->actorCtx.actorLists[(cat == 0) ? ACTORCAT_ENEMY : ACTORCAT_BOSS].first;
        for (; a != NULL; a = a->next) {
            if ((a->update == NULL) || (Actor_WorldDistXZToActor(&player->actor, a) > maxDist)) {
                continue;
            }
            s16 diff = Actor_WorldYawTowardActor(&player->actor, a) - player->actor.shape.rot.y;
            s16 absDiff = (diff < 0) ? -diff : diff;
            if (absDiff < bestAbs) {
                bestAbs = absDiff;
                best = a;
            }
        }
    }
    return best;
}

// B (sword) replaced with the bull QUAKE POUND: jumpAT + hop; landing handled in GohtPostAction.
// Returns true if it took over this swing (caller = func_80833864 right after the melee anim starts).
extern "C" s32 BossRemains_GohtQuakeStart(PlayState* play, Player* player) {
    if (!BossRemains_IsGohtWorn() || (play == nullptr) || (player == nullptr)) {
        return false;
    }
    if (sGohtQuakeState != 0) {
        return true; // already pounding — don't restack
    }
    // High, snappy hop. Like item_rocscape.c, we DON'T play the anim now — native jump physics kick in for
    // a few frames, then jumpAT plays a bit after leaving the ground (sGohtQuakeAnimDelay, handled below).
    player->actor.velocity.y = kGohtJumpVel;
    player->actor.bgCheckFlags &= ~BGCHECKFLAG_GROUND;
    sGohtQuakeState = 1;
    sGohtQuakeAnimDelay = kGohtJumpAnimDelay;
    Actor_PlaySfx(&player->actor, NA_SE_EN_MIBOSS_JUMP1);
    return true;
}

// Per-frame Goht driver, called from z_player.c AFTER the player's action func so our speed/anim
// overrides win over locomotion. Owns: bull charge, quake landing.
extern "C" void BossRemains_GohtPostAction(PlayState* play, Player* player) {
    if (play == nullptr || player == nullptr) {
        return;
    }

    if (!BossRemains_IsGohtWorn() || (player->transformation != PLAYER_FORM_HUMAN)) {
        sGohtCharging = false;
        sGohtQuakeState = 0;
        // Never leave our no-snap flag behind (it would make Link float over every ledge). Only clear the
        // copy WE set — a real Goron roll owns its own and must keep it.
        if (sGohtNoSnapOwned) {
            player->actor.bgCheckFlags &= ~BGCHECKFLAG_PLAYER_800;
            sGohtNoSnapOwned = false;
        }
        return;
    }

    if (sGohtChargeCooldown > 0) {
        sGohtChargeCooldown--;
    }

    Input* in = &play->state.input[0];

    // ── GORON QUAKE POUND on SHIELD(R)+A ──────────────────────────────────────
    // (Swapped with the thunder charge: B now charges the bolt, R+A pounds.) The sword is UNEQUIPPED while
    // Goht is worn, so none of this goes through the melee path.
    if (CHECK_BTN_ALL(in->press.button, BTN_A) && CHECK_BTN_ALL(in->cur.button, BTN_R) &&
        (sGohtQuakeState == 0) && (player->actor.bgCheckFlags & BGCHECKFLAG_GROUND) &&
        !(player->stateFlags1 & PLAYER_STATE1_CARRYING_ACTOR) && (play->msgCtx.msgMode == MSGMODE_NONE)) {
        BossRemains_GohtQuakeStart(play, player);
    }

    // ── BULL CHARGE: while HOLDING A (no R), not mid-pound ────────────────────
    // Like the Goron roll, a charge can only START on the ground but KEEPS GOING through the air, so a
    // ledge/ramp launch stays a charge (bull anim + cone + full speed) and Link sails a long way.
    {
        bool wasCharging = sGohtCharging;
        // NOTE: no magic gate here — like the Pegasus dash, the charge itself always runs; magic only
        // powers the contact damage + the cone (see the drain below).
        bool wantCharge = CHECK_BTN_ALL(in->cur.button, BTN_A) && !CHECK_BTN_ALL(in->cur.button, BTN_R) &&
                          (sGohtQuakeState == 0) && (sGohtChargeCooldown == 0) &&
                          !(player->stateFlags1 & PLAYER_STATE1_CARRYING_ACTOR) &&
                          (play->msgCtx.msgMode == MSGMODE_NONE);
        bool grounded = (player->actor.bgCheckFlags & BGCHECKFLAG_GROUND) != 0;
        sGohtCharging = wantCharge && (grounded || wasCharging);
        if (!sGohtCharging) {
            sGohtChargeMagicTick = 0; // Pegasus_Stop resets its tick the same way
        }
    }

    // GORON-ROLL TERRAIN HANDLING: while charging, stop Link from snapping down onto the floor. This is the
    // exact vanilla flag the Goron roll sets (func_80857A44) — z_actor.c's floor check only hugs small drops
    // when PLAYER_800 is clear, so with it set ledges and slopes are IGNORED and a ramp launches Link into a
    // long flight instead of gluing him to the terrain. Cleared the moment the charge ends (Player_Action_96
    // clears it the same way on exit) so normal ground-hugging comes right back. We only ever clear the flag
    // when WE set it (sGohtNoSnapOwned) so a real Goron roll's copy of this flag is never stomped.
    if (sGohtCharging) {
        player->actor.bgCheckFlags |= BGCHECKFLAG_PLAYER_800;
        sGohtNoSnapOwned = true;
    } else if (sGohtNoSnapOwned) {
        player->actor.bgCheckFlags &= ~BGCHECKFLAG_PLAYER_800;
        sGohtNoSnapOwned = false;
    }

    if (sGohtCharging) {
        // 3x forward speed — override whatever the action set. On the GROUND, split that speed along the
        // floor pitch exactly like the Goron roll (z_player.c:21039): speedXZ = speed·cos(pitch),
        // velocity.y = speed·sin(pitch). Running UP a ramp therefore BANKS real upward velocity.y, and the
        // instant Link leaves the ledge that Y momentum carries him into a long arc. Airborne we keep the
        // horizontal speed and leave velocity.y alone so gravity + the banked launch play out naturally.
        if (player->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
            player->speedXZ = kGohtChargeSpeed * Math_CosS(player->floorPitch);
            player->actor.velocity.y = kGohtChargeSpeed * Math_SinS(player->floorPitch);
        } else {
            player->speedXZ = kGohtChargeSpeed;
        }

        // STIFF steering, like a Goron roll but harder to turn: the locomotion func snapped the yaw to the
        // stick this frame; we overwrite it and instead crawl toward the stick's intended direction at a
        // small fixed rate. Facing barely moves per frame, so hard turns become wide arcs.
        f32 speedTarget;
        s16 yawTarget;
        // SPEED_MODE_CURVED (0.018f) — the macro is local to z_player.c, so inline its value here.
        if (Player_GetMovementSpeedAndYaw(player, &speedTarget, &yawTarget, 0.018f, play)) {
            Math_ScaledStepToS(&player->actor.world.rot.y, yawTarget, kGohtTurnStep);
        }
        player->actor.shape.rot.y = player->actor.world.rot.y;
        player->yaw = player->actor.world.rot.y;

        // Bull run anim (cl_nigeru): we OWN the pose so the locomotion func can't steal it on turns. Set the
        // exact cycle frame ourselves each frame (morph 0) and advance our own accumulator (8-frame loop).
        sGohtChargeAnimFrame += kGohtChargeAnimSpeed;
        if (sGohtChargeAnimFrame >= 8.0f) {
            sGohtChargeAnimFrame -= 8.0f;
        }
        PlayerAnimation_Change(play, &player->skelAnime, (PlayerAnimationHeader*)gPlayerAnim_cl_nigeru, 1.0f,
                               sGohtChargeAnimFrame, 8.0f, ANIMMODE_ONCE, 0.0f);

        // Hoof stomps — Goht's own heavy footstep.
        if (--sGohtSfxTimer <= 0) {
            Actor_PlaySfx(&player->actor, NA_SE_EN_ICEB_FOOTSTEP_OLD);
            sGohtSfxTimer = 6;
        }

        // MAGIC-POWERED PART (1:1 with Pegasus_StateRunning): only WITH magic does the charge get its
        // light goron-punch contact damage, and only then does it drain. Out of magic the charge keeps
        // running at full speed — it just stops hurting things (and the cone stops drawing).
        if (gSaveContext.save.saveInfo.playerData.magic > 0) {
            if (!sGohtChargeColReady) {
                Collider_InitCylinder(play, &sGohtChargeCollider);
                Collider_SetCylinder(play, &sGohtChargeCollider, &player->actor, &sGohtChargeColliderInit);
                sGohtChargeColReady = true;
            }
            Collider_UpdateCylinder(&player->actor, &sGohtChargeCollider);
            CollisionCheck_SetAT(play, &play->colChkCtx, &sGohtChargeCollider.base);

            sGohtChargeMagicTick++;
            if (sGohtChargeMagicTick >= kGohtChargeMagicInterval) {
                sGohtChargeMagicTick = 0;
                gSaveContext.save.saveInfo.playerData.magic--;
                if (gSaveContext.save.saveInfo.playerData.magic < 0) {
                    gSaveContext.save.saveInfo.playerData.magic = 0;
                }
            }
        } else {
            sGohtChargeMagicTick = 0;
        }

        // CRASH: hitting a wall detonates a POWDER KEG at the impact point — the real keg blast, so it
        // breaks every bombable/keg-breakable thing (that's the "bonk to break stuff"). Link takes NO damage
        // from his own blast (brief intangibility) and BOUNCES back off the wall like a real bonk recoil.
        if (player->actor.bgCheckFlags & BGCHECKFLAG_WALL) {
            Vec3f p = player->actor.world.pos;
            p.x += Math_SinS(player->actor.shape.rot.y) * 30.0f;
            p.z += Math_CosS(player->actor.shape.rot.y) * 30.0f;
            p.y += 20.0f;
            EnBom* keg = (EnBom*)Actor_Spawn(&play->actorCtx, play, ACTOR_EN_BOM, p.x, p.y, p.z,
                                             BOMB_EXPLOSIVE_TYPE_POWDER_KEG, player->actor.shape.rot.y, 0,
                                             BOMB_TYPE_BODY);
            if (keg != NULL) {
                keg->timer = 0; // blow NOW
            }
            // Intangible through the blast (invincibilityTimer != 0 → damage is skipped; negative = no flashing).
            player->invincibilityTimer = -40;
            // Bonk recoil: shoot backwards off the wall with a little hop.
            player->speedXZ = kGohtBounceSpeed;
            player->actor.velocity.y = kGohtBounceHop;
            player->actor.bgCheckFlags &= ~BGCHECKFLAG_GROUND;
            Actor_PlaySfx(&player->actor, NA_SE_PL_BODY_BOUND); // bonk thud
            sGohtCharging = false;
            sGohtChargeCooldown = 25;
        }
    }

    // ── QUAKE POUND: airborne → landing after the high hop ────────────────────
    if (sGohtQuakeState == 1) {
        // rocscape-style: play jumpAT a few frames AFTER leaving the ground (native jump physics show first).
        if (sGohtQuakeAnimDelay > 0) {
            sGohtQuakeAnimDelay--;
            if (sGohtQuakeAnimDelay == 0) {
                // pz_jumpAT = 13 frames (indices 0..12). Played to the LAST real frame (12) at 3x speed;
                // going to 13 would land on the invalid bind/rest pose.
                PlayerAnimation_Change(play, &player->skelAnime, (PlayerAnimationHeader*)gPlayerAnim_pz_jumpAT, 3.0f,
                                       0.0f, 12.0f, ANIMMODE_ONCE, -3.0f);
                sGohtJumpPlayTimer = 5; // 12 frames at 3x ≈ 4 → let it play, then hold on the last frame
            }
        } else if (sGohtJumpPlayTimer > 0) {
            sGohtJumpPlayTimer--; // let jumpAT play through
        } else {
            // jumpAT finished: LOOP on its LAST real frame (12) for the rest of the airtime, so the player's
            // action func can't drop us into the rest/bind pose. Re-asserted every frame (morph 0) to hold.
            PlayerAnimation_Change(play, &player->skelAnime, (PlayerAnimationHeader*)gPlayerAnim_pz_jumpAT, 1.0f,
                                   12.0f, 12.0f, ANIMMODE_LOOP, 0.0f);
        }
        // Fast, snappy descent: pile on extra downward accel once past the apex.
        if (player->actor.velocity.y <= 0.0f) {
            player->actor.velocity.y -= kGohtJumpGravityBoost;
        }
        if ((player->actor.bgCheckFlags & BGCHECKFLAG_GROUND) && (player->actor.velocity.y <= 0.0f)) {
            Actor_RequestQuake(play, 10, 16);
            Actor_PlaySfx(&player->actor, NA_SE_EN_ICEB_FOOTSTEP_OLD); // Goht's heavy stomp on landing
            // pz_jumpATend = 13 frames (indices 0..12). End on frame 12, not 13 (13 = rest/bind pose). 3x speed.
            PlayerAnimation_Change(play, &player->skelAnime, (PlayerAnimationHeader*)gPlayerAnim_pz_jumpATend, 3.0f,
                                   0.0f, 12.0f, ANIMMODE_ONCE, -2.0f);
            sGohtQuakeState = 2;
            sGohtQuakeDmgFrames = 8;
        }
    } else if (sGohtQuakeState == 2) {
        if (sGohtQuakeDmgFrames > 0) {
            sGohtQuakeDmgFrames--;
            // Lazy one-time collider init (needs a live PlayState).
            if (!sGohtQuakeColReady) {
                Collider_InitCylinder(play, &sGohtQuakeCollider);
                Collider_SetCylinder(play, &sGohtQuakeCollider, &player->actor, &sGohtQuakeColliderInit);
                sGohtQuakeColReady = true;
            }
            Collider_UpdateCylinder(&player->actor, &sGohtQuakeCollider);
            CollisionCheck_SetAT(play, &play->colChkCtx, &sGohtQuakeCollider.base);
        } else {
            sGohtQuakeState = 0;
            sGohtRecoverFrames = 8; // kill the lingering Zora jumpAT pose next
        }
    }

    // RECOVERY: pz_jumpAT/pz_jumpATend are ZORA-form anims (see the melee-anim table), so on a human Link they
    // finish in a swim-like pose. Force the normal human idle for a few frames so the pose resets cleanly.
    if ((sGohtRecoverFrames > 0) && !sGohtCharging && (sGohtQuakeState == 0)) {
        sGohtRecoverFrames--;
        // In this TU the anim symbols are OTR path strings, so Animation_GetLastFrame can't read a header —
        // waitR_free is 29 frames (indices 0..28); loop to 28, not 29 (29 = rest/bind pose).
        PlayerAnimation_Change(play, &player->skelAnime, (PlayerAnimationHeader*)gPlayerAnim_link_normal_waitR_free,
                               1.0f, 0.0f, 28.0f, ANIMMODE_LOOP, -4.0f);
    }
}

// ── Majora-red bull-charge cone (pegasus cone geometry, retinted + Majora chant texture) ──────
static Vtx sGohtConeVtx[] = {
    { { { 0, 0, 0 }, 0, { 512, 2048 }, { 0xFF, 0xFF, 0xFF, 0xFF } } },      // tip (front)
    { { { 4000, 8000, 0 }, 0, { 0, 0 }, { 0xFF, 0xFF, 0xFF, 0x00 } } },     // base ring
    { { { 2828, 8000, 2828 }, 0, { 256, 0 }, { 0xFF, 0xFF, 0xFF, 0x00 } } },
    { { { 0, 8000, 4000 }, 0, { 512, 0 }, { 0xFF, 0xFF, 0xFF, 0x00 } } },
    { { { -2828, 8000, 2828 }, 0, { 768, 0 }, { 0xFF, 0xFF, 0xFF, 0x00 } } },
    { { { -4000, 8000, 0 }, 0, { 1024, 0 }, { 0xFF, 0xFF, 0xFF, 0x00 } } },
    { { { -2828, 8000, -2828 }, 0, { 1280, 0 }, { 0xFF, 0xFF, 0xFF, 0x00 } } },
    { { { 0, 8000, -4000 }, 0, { 1536, 0 }, { 0xFF, 0xFF, 0xFF, 0x00 } } },
    { { { 2828, 8000, -2828 }, 0, { 1792, 0 }, { 0xFF, 0xFF, 0xFF, 0x00 } } },
};

// Same combiner/geometry as the pegasus cone but with MAJORA's reddish chant colors baked in.
static Gfx sGohtConeDL[] = {
    gsDPSetCombineLERP(TEXEL1, PRIMITIVE, PRIM_LOD_FRAC, TEXEL0, TEXEL1, TEXEL0, PRIM_LOD_FRAC, TEXEL0, PRIMITIVE,
                       ENVIRONMENT, COMBINED, ENVIRONMENT, COMBINED, 0, SHADE, 0),
    gsDPSetRenderMode(G_RM_PASS, G_RM_AA_ZB_XLU_SURF2),
    gsSPClearGeometryMode(G_CULL_BACK | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR),
    gsDPSetPrimColor(0, 0x80, 255, 90, 60, 255), // Majora chant red-orange
    gsDPSetEnvColor(130, 10, 10, 0),             // deep red
    gsSPDisplayList(0x08000001),                 // segment 0x08: animated tex scroll (set per frame)
    gsSPVertex(sGohtConeVtx, 9, 0),
    gsSP2Triangles(0, 1, 2, 0, 0, 2, 3, 0),
    gsSP2Triangles(0, 3, 4, 0, 0, 4, 5, 0),
    gsSP2Triangles(0, 5, 6, 0, 0, 6, 7, 0),
    gsSP2Triangles(0, 7, 8, 0, 0, 8, 1, 0),
    gsSPEndDisplayList(),
};

// Draw the charge cone around Link (called from the player draw, next to the Odolwa trail).
extern "C" void BossRemains_DrawGohtCone(Player* player, PlayState* play) {
    if (!sGohtCharging || (play == nullptr) || (player == nullptr)) {
        return;
    }
    // Magic powers the cone (Pegasus_Draw bails the same way at magic <= 0): out of magic the bull still
    // charges, it just loses the aura.
    if (gSaveContext.save.saveInfo.playerData.magic <= 0) {
        return;
    }

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);

    // Placement baked from the editor tuning: 59 forward, 20 up (head height), tip pitched -90° forward.
    Matrix_Push();
    f32 sinY = Math_SinS(player->actor.shape.rot.y);
    f32 cosY = Math_CosS(player->actor.shape.rot.y);
    Matrix_Translate(player->actor.world.pos.x + sinY * 59.0f, player->actor.world.pos.y + 20.0f,
                     player->actor.world.pos.z + cosY * 59.0f, MTXMODE_NEW);
    Matrix_RotateYF(BINANG_TO_RAD(player->actor.shape.rot.y), MTXMODE_APPLY);
    Matrix_RotateXFApply(BINANG_TO_RAD((s16)-0x4000)); // tip forward
    Matrix_Scale(0.015f, 0.015f, 0.015f, MTXMODE_APPLY);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);

    // Majora's chant streak texture (i8 32x64, object_stk2), loaded at runtime like the pegasus cone.
    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetTextureLUT(POLY_XLU_DISP++, G_TT_NONE);
    gSPTexture(POLY_XLU_DISP++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    gDPLoadTextureBlock(POLY_XLU_DISP++, object_stk2_Tex_008B50, G_IM_FMT_I, G_IM_SIZ_8b, 32, 64, 0,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, 5, 6, G_TX_NOLOD, G_TX_NOLOD);
    gDPLoadMultiBlock(POLY_XLU_DISP++, object_stk2_Tex_008B50, 0x0100, 1, G_IM_FMT_I, G_IM_SIZ_8b, 32, 64, 0,
                      G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, 5, 6, 14, 14);

    u32 frames = play->gameplayFrames;
    // (uintptr_t) cast: Gfx_TwoTexScroll returns Gfx*, and this C++ TU has no implicit pointer→integer
    // conversion for gSPSegment's uintptr_t arg (same cast the Odolwa seg-0xC noop uses above).
    gSPSegment(POLY_XLU_DISP++, 0x08,
               (uintptr_t)Gfx_TwoTexScroll(play->state.gfxCtx, 0, -(s32)(frames * 1), (s32)(frames * 20), 0x20, 0x40,
                                           1, -(s32)(frames * 2), (s32)(frames * 10), 0x20, 0x40));

    gSPDisplayList(POLY_XLU_DISP++, sGohtConeDL);

    Matrix_Pop();

    CLOSE_DISPS(play->state.gfxCtx);
}

// Charging-thunder VFX at Link's shield (Goht's real light-orb + crossed lightning bolts, from
// BossHakugin_DrawChargingLightning). Grows as the R+A charge builds. Call from the player draw.
extern "C" void BossRemains_DrawGohtChargingThunder(Player* player, PlayState* play) {
    if (!sGohtThunderCharging || (sGohtThunderCharge <= 0) || (play == nullptr) || (player == nullptr)) {
        return;
    }

    f32 t = (f32)sGohtThunderCharge / (f32)kGohtThunderChargeMax; // 0..1
    s16 yaw = player->actor.shape.rot.y;
    Vec3f pos;
    pos.x = player->actor.world.pos.x + Math_SinS(yaw) * 22.0f;
    pos.y = player->actor.world.pos.y + 40.0f;
    pos.z = player->actor.world.pos.z + Math_CosS(yaw) * 22.0f;
    s16 spin = (s16)(play->gameplayFrames * 0x1000);

    OPEN_DISPS(play->state.gfxCtx);
    Gfx_SetupDL25_Xlu(play->state.gfxCtx);

    // Two crossed lightning models rotating around the center (mirrors the real charging effect).
    gDPSetEnvColor(POLY_XLU_DISP++, 0, 255, 255, 0); // sLightningColor
    gSPDisplayList(POLY_XLU_DISP++, (Gfx*)gGohtLightningMaterialDL);
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 255, 255, 255);
    for (s32 i = -1; i <= 1; i += 2) {
        Vec3s rot = { 0, yaw, 0 };
        Matrix_SetTranslateRotateYXZ(pos.x, pos.y, pos.z, &rot);
        Matrix_RotateYS((s16)(0x1400 * i), MTXMODE_APPLY);
        Matrix_RotateXS((s16)(0xC00 * i), MTXMODE_APPLY);
        Matrix_RotateZS(spin, MTXMODE_APPLY);
        Matrix_Scale(0.15f, 0.15f, 0.45f * t, MTXMODE_APPLY);
        MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
        gSPDisplayList(POLY_XLU_DISP++, (Gfx*)gGohtLightningModelDL);
        Matrix_RotateZS(0x4000, MTXMODE_APPLY);
        MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
        gSPDisplayList(POLY_XLU_DISP++, (Gfx*)gGohtLightningModelDL);
    }

    // Growing light orb at the center (billboarded).
    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 180, 255, 255, 0);
    f32 orb = 0.015f + (0.05f * t);
    Matrix_Translate(pos.x, pos.y, pos.z, MTXMODE_NEW);
    Matrix_ReplaceRotation(&play->billboardMtxF);
    Matrix_Scale(orb, orb, orb, MTXMODE_APPLY);
    Matrix_RotateZS(spin, MTXMODE_APPLY);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, (Gfx*)gGohtLightOrbMaterialDL);
    gSPDisplayList(POLY_XLU_DISP++, (Gfx*)gGohtLightOrbModelDL);

    CLOSE_DISPS(play->state.gfxCtx);
}

// Per-frame upkeep while the Odolwa remains is worn (footsteps + shield-deflect fire). The 1.5x run,
// the roll suppression, and the R+A dash all live in z_player.c; this only owns the mask-side effects.
static void BossRemains_TickOdolwa(PlayState* play, Player* player) {
    // The 2x run + purple trail are active only while HOLDING A.
    sOdolwaRunBoost = CHECK_BTN_ALL(play->state.input[0].cur.button, BTN_A);

    // Heavy Odolwa footsteps only during the boosted run.
    if (sOdolwaRunBoost && (player->actor.speed > 1.0f)) {
        if (--sOdolwaRunSfxTimer <= 0) {
            Actor_PlaySfx(&player->actor, NA_SE_EN_MIBOSS_GND1_OLD);
            sOdolwaRunSfxTimer = 8;
        }
    }
    if (sOdolwaShieldFireTimer > 0) {
        sOdolwaShieldFireTimer--;
    }
    if (player->shieldQuad.base.acFlags & AC_BOUNCED) {
        BossRemains_OdolwaShieldFire(play, player);
    }
}

// Kill every lingering Odolwa SFX the instant the mask is doffed (toggled off, transformed away, or
// covered by a native mask). The run footsteps play on a fast cadence and the moth/beetle cues ring
// out, so without this they keep sounding after the mask is gone. AudioSfx_StopById is global (no
// position) — the real MM name for what the switch hook calls (via the Audio_StopSfxById OoT-compat
// macro) to kill its chain loop on stop; this C++ TU doesn't pull that macro header, so use it direct.
static void BossRemains_StopOdolwaSfx(void) {
    AudioSfx_StopById(NA_SE_EN_MIBOSS_GND1_OLD);   // running footsteps cadence
    AudioSfx_StopById(NA_SE_EN_MB_MOTH_FLY);       // sword-swing moth beam
    AudioSfx_StopById(NA_SE_EN_MIBOSS_VOICE2_OLD); // R+B beetle summon voice
}

// Goht UNEQUIPS the sword (like Goron): while worn we stash the B-button item and blank it, so B is free
// for the pound and no sword can ever be drawn/swung. Restored on doff / transform / mask-swap. The stashed
// id is also mirrored to a CVar so a save+reload that lands mid-stash can recover it (buttonItems persist
// in the save file). Runs every frame; only acts on the stash/restore transitions.
static void BossRemains_SyncGohtSword(PlayState* play, Player* player) {
    bool gohtNow = (sWornRemains == ITEM_REMAINS_GOHT) && (player->transformation == PLAYER_FORM_HUMAN);

    if (gohtNow && !sGohtSwordStashed) {
        s16 cur = GET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_B);
        CVarSetInteger("gBossRemains.GohtStashedSword", cur);
        SET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_B, ITEM_NONE);
        Interface_LoadItemIconImpl(play, EQUIP_SLOT_B);
        sGohtSwordStashed = true;
    } else if (!gohtNow) {
        s16 stashed = (s16)CVarGetInteger("gBossRemains.GohtStashedSword", ITEM_NONE);
        if (sGohtSwordStashed) {
            SET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_B, stashed);
            CVarSetInteger("gBossRemains.GohtStashedSword", ITEM_NONE);
            Interface_LoadItemIconImpl(play, EQUIP_SLOT_B);
            sGohtSwordStashed = false;
        } else if ((stashed != ITEM_NONE) && (GET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_B) == ITEM_NONE)) {
            // Recover a sword lost to a save/reload that happened while Goht was worn.
            SET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_B, stashed);
            CVarSetInteger("gBossRemains.GohtStashedSword", ITEM_NONE);
            Interface_LoadItemIconImpl(play, EQUIP_SLOT_B);
        }
    }
}

extern "C" void BossRemains_TickInput(PlayState* play, Player* player) {
    if (!RemainsEnabled() || play == nullptr || player == nullptr) {
        return;
    }
    // Keep the sword unequip in sync EVERY frame, before any early return below can skip it (so doffing via
    // transform / textbox / mask-swap still restores the blade).
    BossRemains_SyncGohtSword(play, player);

    // Twinmold's Dark Link companion only exists while that remains is worn — retire it the moment it comes
    // off (toggled, swapped for another mask, transformed away). Also before the early returns below.
    if (((sWornRemains != ITEM_REMAINS_TWINMOLD) || (player->transformation != PLAYER_FORM_HUMAN)) &&
        RemainsAllyLink_IsAlive()) {
        RemainsAllyLink_Despawn(play);
    }
    // No wearing mid-textbox.
    if (play->msgCtx.msgMode != MSGMODE_NONE) {
        return;
    }
    // Only Human Link wears these; drop it on any transformation.
    if (player->transformation != PLAYER_FORM_HUMAN) {
        if (sWornRemains == ITEM_REMAINS_ODOLWA) {
            BossRemains_StopOdolwaSfx();
        }
        sWornRemains = ITEM_NONE;
        return;
    }
    // Mutual exclusion with native masks — if one got donned, our remains comes off.
    if ((player->currentMask != PLAYER_MASK_NONE) && (sWornRemains != ITEM_NONE)) {
        if (sWornRemains == ITEM_REMAINS_ODOLWA) {
            BossRemains_StopOdolwaSfx();
        }
        sWornRemains = ITEM_NONE;
    }

    // Phase 3 — drive this frame's per-remains ally summons. Runs unconditionally so Gyorg's fish
    // school can self-maintain; Odolwa (R+B) / Goht (R+A) gate internally.
    BossRemains_TickSummons(play, player);

    // Gyorg's whirlpool keeps pulling + damaging until its timer runs out (self-gates when inactive).
    BossRemains_GyorgWhirlpoolTick(play, player);

    // Odolwa mask-side effects (footsteps + shield-deflect fire).
    if (sWornRemains == ITEM_REMAINS_ODOLWA) {
        BossRemains_TickOdolwa(play, player);
    }

    u16 press = play->state.input[0].press.button;
    if (press == 0) {
        return;
    }

    // (button bit, equipped item) for each remains-capable slot: 3 C buttons + 4 D-pad (form 0).
    struct SlotBind {
        u16 btn;
        s16 item;
    };
    SlotBind slots[7] = {
        { BTN_CLEFT, BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) },
        { BTN_CDOWN, BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) },
        { BTN_CRIGHT, BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) },
        { BTN_DRIGHT, DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) },
        { BTN_DLEFT, DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) },
        { BTN_DDOWN, DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) },
        { BTN_DUP, DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) },
    };
    bool dpadOn = DpadEquipsEnabled();

    for (s32 i = 0; i < 7; i++) {
        bool isDpad = (i >= 3);
        if (isDpad && !dpadOn) {
            continue;
        }
        if (!(press & slots[i].btn)) {
            continue;
        }
        if (!IsRemainsItem(slots[i].item)) {
            continue;
        }
        // Toggle: same remains → doff, different/none → don.
        if (sWornRemains == slots[i].item) {
            if (sWornRemains == ITEM_REMAINS_ODOLWA) {
                BossRemains_StopOdolwaSfx();
            }
            sWornRemains = ITEM_NONE;
            Actor_PlaySfx(&player->actor, NA_SE_PL_TAKE_OUT_SHIELD);
        } else {
            sWornRemains = slots[i].item;
            player->currentMask = PLAYER_MASK_NONE; // hide any native mask under it
            Actor_PlaySfx(&player->actor, NA_SE_PL_CHANGE_ARMS);
        }
        break;
    }
}

extern "C" void BossRemains_DrawWornMask(PlayState* play, Player* player) {
    if (!RemainsEnabled() || play == nullptr || player == nullptr) {
        return;
    }
    if (!IsRemainsItem(sWornRemains) || (player->transformation != PLAYER_FORM_HUMAN)) {
        return;
    }

    s32 idx = RemainsIndex(sWornRemains);
    f32 scale = kMaskScale[idx] * kScaleMul;

    OPEN_DISPS(play->state.gfxCtx);

    // Current matrix here is Link's head node (we are called from the PLAYER_LIMB_HEAD limb draw).
    // Mirror the Moon Child's order: Scale -> RotateZYX -> Translate, then load + draw. Push/Pop so we
    // don't perturb the hat/next limbs.
    Matrix_Push();
    Matrix_Scale(scale, scale, scale, MTXMODE_APPLY);
    Matrix_RotateZYX(DegToBinang(kRotXDeg), DegToBinang(kRotYDeg), DegToBinang(kRotZDeg), MTXMODE_APPLY);
    Matrix_Translate(kMaskTransX[idx] + kOffX, kMaskTransY[idx] + kOffY, kMaskTransZ[idx] + kOffZ, MTXMODE_APPLY);

    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, (Gfx*)kMaskDLPath[idx]);
    Matrix_Pop();

    CLOSE_DISPS(play->state.gfxCtx);
}

// ============================================================================
// (deprecated hook — Odolwa's roll is now suppressed directly at func_80836B3C,
// and R+A/R+B are handled per-frame in BossRemains_TickInput. Kept as a no-op so
// the header/ABI stays stable.)
// ============================================================================

extern "C" s32 BossRemains_TryActionA(PlayState* play, Player* player) {
    (void)play;
    (void)player;
    return false;
}

// ============================================================================
// A-action accessors read by the player (z_player.c) — kept trivial + gated.
// ============================================================================

// True whenever the Odolwa remains is worn (drives run 1.5x, sword/shield swap, faster attacks, trail).
extern "C" s32 BossRemains_IsOdolwaWorn(void) {
    return (RemainsEnabled() && (sWornRemains == ITEM_REMAINS_ODOLWA)) ? 1 : 0;
}

// Run-speed multiplier applied at Player_Action_13 (human locomotion): 2x while wearing Odolwa AND
// HOLDING A; 1.0x otherwise. (sOdolwaRunBoost is refreshed each frame in BossRemains_TickOdolwa.)
extern "C" f32 BossRemains_RunSpeedMul(void) {
    return (BossRemains_IsOdolwaWorn() && sOdolwaRunBoost) ? 2.0f : 1.0f;
}

// True while the 2x run boost (A held) is active — drives the purple trail (trail func also gates on
// actual movement), so the trail only shows during the boosted run.
extern "C" s32 BossRemains_IsOdolwaRunning(void) {
    return (BossRemains_IsOdolwaWorn() && sOdolwaRunBoost) ? 1 : 0;
}

// ============================================================================
// Odolwa red running afterimage (trail)
// ============================================================================
// Same idea as z_boss_01.c's afterimage (frozen-pose ghosts of the SAME skeleton, tinted red) but
// for Link, via the equip_foursword.c ghost technique: keep a ring buffer of past {pos, yaw, pose}
// captured each frame while running, then redraw Link's skeleton at those past poses with Odolwa's
// dark-red POSITIONAL FOG (Gfx_SetFog 50,0,40,255,900,1099 — the exact boss tint, z_boss_01.c:2934),
// restored afterward. POLY_OPA, no per-copy alpha — the fade is purely temporal (older samples get
// overwritten). Drawn from Player_Draw (z_player.c), next to CustomItems_OverrideDraw.

extern "C" {
// SkelAnime override that selects Link's per-limb equipment DLs (sword/shield/sheath/tunic) — so the
// ghosts wear exactly what Link wears. Declared with the OverrideLimbDrawOpa signature (last param
// Actor*) so it slots straight into SkelAnime_DrawFlexOpa in this C++ TU. Gfx_SetFog / Play_SetFog /
// SkelAnime_DrawFlexOpa themselves come from z64.h (gfx.h / z64play.h / z64animation.h).
s32 Player_OverrideLimbDrawGameplayDefault(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot,
                                           Actor* actor);
}

namespace {
constexpr s32 kOdolwaTrailMax = 6; // ghost copies trailing at once
struct OdolwaTrailSample {
    Vec3f pos;
    s16 yaw;
    u8 valid;
    Vec3s joints[PLAYER_LIMB_MAX];
};
OdolwaTrailSample sOdolwaTrail[kOdolwaTrailMax];
s32 sOdolwaTrailHead = 0;

inline void OdolwaTrailClear() {
    for (s32 i = 0; i < kOdolwaTrailMax; i++) {
        sOdolwaTrail[i].valid = 0;
    }
}
} // namespace

extern "C" void BossRemains_DrawOdolwaTrail(Player* player, PlayState* play) {
    if (play == nullptr || player == nullptr) {
        return;
    }
    // Only trail while actively running; otherwise clear so stale ghosts don't linger.
    if (!BossRemains_IsOdolwaRunning() || (player->actor.speed < 1.0f)) {
        OdolwaTrailClear();
        return;
    }

    s32 lc = player->skelAnime.limbCount;
    if (lc >= PLAYER_LIMB_MAX) {
        lc = PLAYER_LIMB_MAX - 1;
    }

    // Capture this frame's finalized pose into the ring buffer.
    OdolwaTrailSample* cur = &sOdolwaTrail[sOdolwaTrailHead];
    cur->pos = player->actor.world.pos;
    cur->yaw = player->actor.shape.rot.y;
    cur->valid = 1;
    for (s32 j = 0; j <= lc; j++) {
        cur->joints[j] = player->skelAnime.jointTable[j];
    }
    sOdolwaTrailHead = (sOdolwaTrailHead + 1) % kOdolwaTrailMax;

    OPEN_DISPS(play->state.gfxCtx);

    // Keep each ghost's REAL model — textures, tunic, sword, everything — and lay a DARK PURPLE tint
    // OVER it, exactly like the switch hook keeps the hookshot model and just tints it blue. The switch
    // hook can use a plain gDPSetEnvColor because ITS single DL's combiner reads env; the player skeleton
    // mixes many per-limb combiners (several ignore env entirely — that is why the env tint showed
    // nothing on Link). So we tint at the FOG stage, which runs AFTER every limb's combiner and so is
    // combiner-independent (SETUPDL_25 keeps G_FOG on — z_rcp.c — and scene fog already tints Link this
    // way). Instead of Gfx_SetFog's full-fog band we set the fog by hand: gDPSetFogColor picks the dark
    // purple, and gSPFogFactor with multiplier 0 makes the blend a CONSTANT fraction at EVERY depth
    // (depth-independent, so it reaches Link right up against the lens). Offset 120/255 ≈ a 47% overlay,
    // so the textured model clearly shows through — a tint, not a flat silhouette. Play_SetFog restores
    // the scene's own fog afterward.
    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    gDPSetFogColor(POLY_OPA_DISP++, 55, 12, 90, 255);
    gSPFogFactor(POLY_OPA_DISP++, 0, 120);

    Vec3s blended[PLAYER_LIMB_MAX];
    for (s32 k = 0; k < kOdolwaTrailMax; k++) {
        OdolwaTrailSample* smp = &sOdolwaTrail[k];
        if (!smp->valid || (smp == cur)) { // skip the just-captured (live) body
            continue;
        }
        for (s32 j = 0; j <= lc; j++) {
            blended[j] = smp->joints[j];
        }

        Matrix_Translate(smp->pos.x, smp->pos.y, smp->pos.z, MTXMODE_NEW);
        Matrix_RotateYF((f32)smp->yaw * ((f32)M_PI / 32768.0f), MTXMODE_APPLY);
        Matrix_Scale(player->actor.scale.x, player->actor.scale.y, player->actor.scale.z, MTXMODE_APPLY);

        // Pass &player->actor (Actor*) — the override casts it back to Player* (Actor is Player's
        // first member, so the address is identical). C++ won't implicitly convert Player*→Actor*.
        SkelAnime_DrawFlexOpa(play, player->skelAnime.skeleton, blended, player->skelAnime.dListCount,
                              Player_OverrideLimbDrawGameplayDefault, NULL, &player->actor);
    }
    // Restore the scene's fog so the real Link + later actors aren't left purple.
    POLY_OPA_DISP = Play_SetFog(play, POLY_OPA_DISP);

    CLOSE_DISPS(play->state.gfxCtx);
}

// ============================================================================
// Odolwa sword + shield swap
// ============================================================================
// While the Odolwa remains is worn, Link's native sword/shield are hidden in the limb override
// (z_player_lib.c) and Odolwa's own DLs (object_boss01) are drawn here in their place, following the
// hand-limb matrix (called from Player_PostLimbDrawGameplay at LEFT_HAND / RIGHT_HAND). Odolwa's
// models are authored at boss scale, so the fit is tuned LIVE via menu sliders — same idea as the
// worn mask. Passing an "__OTR__..." path straight to gSPDisplayList resolves the DL + its internal
// refs (the moon-child mask proves this works for a native MM object).
static const char* const kOdolwaSwordDLPath = "__OTR__objects/object_boss01/gOdolwaSwordDL";
static const char* const kOdolwaShieldDLPath = "__OTR__objects/object_boss01/gOdolwaShieldDL";

// In-hand placement, tuned in-game and baked. Sword on Link's LEFT hand, shield on the RIGHT.
constexpr f32 kOdolwaSwordScale = 0.4f;
constexpr s32 kOdolwaSwordRotX = 0, kOdolwaSwordRotY = 7, kOdolwaSwordRotZ = 75;
constexpr f32 kOdolwaSwordOffX = -21.0f, kOdolwaSwordOffY = 393.0f, kOdolwaSwordOffZ = -157.0f;
constexpr f32 kOdolwaShieldScale = 0.7f;
constexpr s32 kOdolwaShieldRotX = -90, kOdolwaShieldRotY = 90, kOdolwaShieldRotZ = 0;
constexpr f32 kOdolwaShieldOffX = 0.0f, kOdolwaShieldOffY = 0.0f, kOdolwaShieldOffZ = 0.0f;

static Gfx sOdolwaEquipSeg0xC_Noop[] = {
    gsSPEndDisplayList(),
    gsSPEndDisplayList(),
    gsSPEndDisplayList(),
    gsSPEndDisplayList(),
};

// Draw one boss DL at the current (hand-limb) matrix with a CVar-tuned scale/rot/offset.
static void DrawOdolwaEquipDL(PlayState* play, const char* dlPath, f32 scale, s16 rx, s16 ry, s16 rz, f32 ox,
                              f32 oy, f32 oz) {
    OPEN_DISPS(play->state.gfxCtx);
    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    gSPSegment(POLY_OPA_DISP++, 0x0C, (uintptr_t)sOdolwaEquipSeg0xC_Noop);

    Matrix_Push();
    Matrix_Translate(ox, oy, oz, MTXMODE_APPLY);
    Matrix_RotateZYX(rx, ry, rz, MTXMODE_APPLY);
    Matrix_Scale(scale, scale, scale, MTXMODE_APPLY);
    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, (Gfx*)dlPath);
    Matrix_Pop();

    CLOSE_DISPS(play->state.gfxCtx);
}

extern "C" void BossRemains_DrawOdolwaSword(PlayState* play, Player* player) {
    if (play == nullptr || player == nullptr || !BossRemains_IsOdolwaWorn()) {
        return;
    }
    if ((player->transformation != PLAYER_FORM_HUMAN) ||
        (player->leftHandType != PLAYER_MODELTYPE_LH_ONE_HAND_SWORD)) {
        return; // only while Link actually has a one-handed sword in hand
    }
    DrawOdolwaEquipDL(play, kOdolwaSwordDLPath, kOdolwaSwordScale, DegToBinang(kOdolwaSwordRotX),
                      DegToBinang(kOdolwaSwordRotY), DegToBinang(kOdolwaSwordRotZ), kOdolwaSwordOffX,
                      kOdolwaSwordOffY, kOdolwaSwordOffZ);
}

extern "C" void BossRemains_DrawOdolwaShield(PlayState* play, Player* player) {
    if (play == nullptr || player == nullptr || !BossRemains_IsOdolwaWorn()) {
        return;
    }
    if (player->transformation != PLAYER_FORM_HUMAN) {
        return;
    }
    DrawOdolwaEquipDL(play, kOdolwaShieldDLPath, kOdolwaShieldScale, DegToBinang(kOdolwaShieldRotX),
                      DegToBinang(kOdolwaShieldRotY), DegToBinang(kOdolwaShieldRotZ), kOdolwaShieldOffX,
                      kOdolwaShieldOffY, kOdolwaShieldOffZ);
}
