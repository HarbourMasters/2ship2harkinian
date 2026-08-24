/**
 * item_ballchain.c - Ball and Chain from Twilight Princess
 *
 * Controls (TP behavior — Skijer's NEI rework):
 *   Hold C Button:  Link spins the ball around himself, winding up over ~1.5s.
 *                   He can shuffle slowly and TURNS SLOWLY (capped yaw step —
 *                   heavy inertia). C-up toggles first-person aim.
 *   Release C:      The ball launches in a heavy ballistic ARC (real gravity).
 *   Flight:         Floor hits BOUNCE the ball (thud + quake + rumble, up to 2
 *                   bounces), wall hits make it RICOCHET (reflect off the wall
 *                   normal + clank); it rests a beat and then retracts along the
 *                   chain back to Link's hand. Link is braced until it returns.
 *
 * Damage: hammer-tier (4) with DMG_GORON_PUNCH | DMG_GORON_POUND — NO fire (a
 * fire flag would light torches). Ice obstacles are destroyed explicitly with an
 * ice-shatter effect + the goron-punch collider (see BALLCHAIN_DMG_FLAGS and
 * BallChain_CheckDestructibles).
 */

#include "z64.h"
#include "item_ballchain.h"
#include "../custom_items.h"
#include "../helpers/camera_helper.h"
#include "../helpers/equip_helper.h"
#include "../helpers/combat_helper.h"
#include "../helpers/item_voice.h"
#include "../anim/ballchain/ballchain_anim_data.h"
#include "macros.h"
#include "functions.h"
#include "variables.h"
#include "overlays/actors/ovl_Obj_Ice_Poly/z_obj_ice_poly.h"
#include "overlays/actors/ovl_Bg_Icicle/z_bg_icicle.h"
#include "overlays/actors/ovl_Obj_Bigicicle/z_obj_bigicicle.h"
#include "overlays/actors/ovl_Obj_Iceblock/z_obj_iceblock.h"
#include "overlays/actors/ovl_En_Fz/z_en_fz.h"
#include "overlays/actors/ovl_En_Snowman/z_en_snowman.h"
// EnSnowman_SetupMelt is non-static in z_en_snowman.c but not declared in its header. Skijer's NEI
extern void EnSnowman_SetupMelt(EnSnowman* this);
// OoT breakables (ice shelter, gerudo iron, goron basket, Shadow Temple pot) have no
// MM analog — the ball smashes MM breakables (pots/crates/grass/rocks) instead.
#include "objects/gameplay_keep/gameplay_keep.h"

// =============================================================================
// Static Data
// =============================================================================

// Goron-punch damage collider (NO fire — see BALLCHAIN_DMG_FLAGS), active both while spinning
// (orbit hits) and while thrown. Ice obstacles are handled separately. Skijer's NEI
static ColliderCylinderInit sBallChainColInit = { { COL_MATERIAL_NONE, AT_ON | AT_TYPE_PLAYER | AT_TYPE_OTHER, AC_NONE,
                                                    OC1_NONE, OC2_NONE, COLSHAPE_CYLINDER },
                                                  { ELEM_MATERIAL_UNK2,
                                                    { BALLCHAIN_DMG_FLAGS, 0, BALLCHAIN_DAMAGE },
                                                    { 0, 0, 0 },
                                                    ATELEM_ON | ATELEM_SFX_NORMAL,
                                                    ACELEM_NONE,
                                                    OCELEM_NONE },
                                                  { BALLCHAIN_COL_RADIUS, BALLCHAIN_COL_HEIGHT, 0, { 0, 0, 0 } } };

static u8 sBallChainColInitialized = 0;
static s8 sBallChainPrevInvinc = 0;

// =============================================================================
// Pose Functions
// =============================================================================

static void BallChain_ResetPose(Player* p) {
    p->upperLimbRot.x = 0;
    p->upperLimbRot.y = 0;
    p->upperLimbRot.z = 0;
}

static void BallChain_SetEquipPose(Player* p) {
    p->skelAnime.jointTable[PLAYER_LIMB_L_SHOULDER].x = BC_EQUIP_L_SHOULDER_X;
    p->skelAnime.jointTable[PLAYER_LIMB_L_SHOULDER].y = BC_EQUIP_L_SHOULDER_Y;
    p->skelAnime.jointTable[PLAYER_LIMB_L_SHOULDER].z = BC_EQUIP_L_SHOULDER_Z;
    p->skelAnime.jointTable[PLAYER_LIMB_L_FOREARM].x = BC_EQUIP_L_FOREARM_X;
    p->skelAnime.jointTable[PLAYER_LIMB_L_FOREARM].y = BC_EQUIP_L_FOREARM_Y;
    p->skelAnime.jointTable[PLAYER_LIMB_L_FOREARM].z = BC_EQUIP_L_FOREARM_Z;
    p->skelAnime.jointTable[PLAYER_LIMB_L_HAND].x = BC_EQUIP_L_HAND_X;
    p->skelAnime.jointTable[PLAYER_LIMB_L_HAND].y = BC_EQUIP_L_HAND_Y;
    p->skelAnime.jointTable[PLAYER_LIMB_L_HAND].z = BC_EQUIP_L_HAND_Z;
    p->skelAnime.jointTable[PLAYER_LIMB_R_SHOULDER].x = BC_EQUIP_R_SHOULDER_X;
    p->skelAnime.jointTable[PLAYER_LIMB_R_SHOULDER].y = BC_EQUIP_R_SHOULDER_Y;
    p->skelAnime.jointTable[PLAYER_LIMB_R_SHOULDER].z = BC_EQUIP_R_SHOULDER_Z;
    p->skelAnime.jointTable[PLAYER_LIMB_R_FOREARM].x = BC_EQUIP_R_FOREARM_X;
    p->skelAnime.jointTable[PLAYER_LIMB_R_FOREARM].y = BC_EQUIP_R_FOREARM_Y;
    p->skelAnime.jointTable[PLAYER_LIMB_R_FOREARM].z = BC_EQUIP_R_FOREARM_Z;
    p->skelAnime.jointTable[PLAYER_LIMB_R_HAND].x = BC_EQUIP_R_HAND_X;
    p->skelAnime.jointTable[PLAYER_LIMB_R_HAND].y = BC_EQUIP_R_HAND_Y;
    p->skelAnime.jointTable[PLAYER_LIMB_R_HAND].z = BC_EQUIP_R_HAND_Z;
    p->upperLimbRot.x = 0;
    p->upperLimbRot.y = 0;
    p->upperLimbRot.z = 0;
}

static void BallChain_SetSpinPose(Player* p, f32 stickX, f32 stickY) {
    p->skelAnime.jointTable[PLAYER_LIMB_L_SHOULDER].x = BC_SPIN_L_SHOULDER_X;
    p->skelAnime.jointTable[PLAYER_LIMB_L_SHOULDER].y = BC_SPIN_L_SHOULDER_Y;
    p->skelAnime.jointTable[PLAYER_LIMB_L_SHOULDER].z = BC_SPIN_L_SHOULDER_Z;
    p->skelAnime.jointTable[PLAYER_LIMB_L_FOREARM].x = BC_SPIN_L_FOREARM_X;
    p->skelAnime.jointTable[PLAYER_LIMB_L_FOREARM].y = BC_SPIN_L_FOREARM_Y;
    p->skelAnime.jointTable[PLAYER_LIMB_L_FOREARM].z = BC_SPIN_L_FOREARM_Z;
    p->skelAnime.jointTable[PLAYER_LIMB_L_HAND].x = BC_SPIN_L_HAND_X;
    p->skelAnime.jointTable[PLAYER_LIMB_L_HAND].y = BC_SPIN_L_HAND_Y;
    p->skelAnime.jointTable[PLAYER_LIMB_L_HAND].z = BC_SPIN_L_HAND_Z;
    p->skelAnime.jointTable[PLAYER_LIMB_R_SHOULDER].x = BC_SPIN_R_SHOULDER_X;
    p->skelAnime.jointTable[PLAYER_LIMB_R_SHOULDER].y = BC_SPIN_R_SHOULDER_Y;
    p->skelAnime.jointTable[PLAYER_LIMB_R_SHOULDER].z = BC_SPIN_R_SHOULDER_Z;
    p->skelAnime.jointTable[PLAYER_LIMB_R_FOREARM].x = BC_SPIN_R_FOREARM_X;
    p->skelAnime.jointTable[PLAYER_LIMB_R_FOREARM].y = BC_SPIN_R_FOREARM_Y;
    p->skelAnime.jointTable[PLAYER_LIMB_R_FOREARM].z = BC_SPIN_R_FOREARM_Z;
    p->skelAnime.jointTable[PLAYER_LIMB_R_HAND].x = BC_SPIN_R_HAND_X;
    p->skelAnime.jointTable[PLAYER_LIMB_R_HAND].y = BC_SPIN_R_HAND_Y;
    p->skelAnime.jointTable[PLAYER_LIMB_R_HAND].z = BC_SPIN_R_HAND_Z;
    p->upperLimbRot.x = (s16)(-stickY * BALLCHAIN_LEAN_MULT);
    p->upperLimbRot.y = 0;
    p->upperLimbRot.z = (s16)(stickX * BALLCHAIN_LEAN_MULT);
}

// =============================================================================
// Collider Functions
// =============================================================================

static void BallChain_InitCollider(PlayState* play, Player* p) {
    if (sBallChainColInitialized)
        return;
    Collider_InitCylinder(play, &bcCollider);
    Collider_SetCylinder(play, &bcCollider, &p->actor, &sBallChainColInit);
    sBallChainColInitialized = 1;
}

static void BallChain_UpdateCollider(PlayState* play, Player* p, Vec3f* pos) {
    // Goron-punch/pound flags only (see BALLCHAIN_DMG_FLAGS) — smashes hammer switches/pillars and
    // triggers Obj_Bigicicle's break; other ice is destroyed in BallChain_CheckDestructibles. Skijer's NEI
    bcCollider.elem.atDmgInfo.dmgFlags = BALLCHAIN_DMG_FLAGS;
    bcCollider.elem.atDmgInfo.damage = BALLCHAIN_DAMAGE;
    bcCollider.elem.atDmgInfo.effect = 0;
    bcCollider.elem.atElemFlags = ATELEM_ON | ATELEM_SFX_NORMAL;

    bcCollider.dim.pos.x = (s16)pos->x;
    bcCollider.dim.pos.y = (s16)(pos->y - (BALLCHAIN_COL_HEIGHT / 2));
    bcCollider.dim.pos.z = (s16)pos->z;
    bcCollider.base.atFlags |= AT_ON | AT_TYPE_PLAYER | AT_TYPE_OTHER;
    CollisionCheck_SetAT(play, &play->colChkCtx, &bcCollider.base);
    CollisionCheck_SetOC(play, &play->colChkCtx, &bcCollider.base);
}

// =============================================================================
// Hit Detection
// =============================================================================

static void BallChain_CheckHit(Vec3f* pos) {
    if (bcCollider.base.atFlags & AT_HIT) {
        Audio_PlaySoundGeneral(BALLCHAIN_SFX_HIT, pos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale,
                               &gSfxDefaultReverb);
        bcCollider.base.atFlags &= ~AT_HIT;
    }
}

// Motion trail (EffectBlure) for the spinning/flying ball — a subtle cool-white streak, fed SPARSER
// than the sword (every 2nd frame + shorter duration) so it reads as heavy metal, not a blade. The
// sword trail system is EFFECT_BLURE2 fed via EffectBlure_AddVertex (see mm_player_form Gerudo trail
// in SoH); MM's EffectBlureInit2 has no trailType field. Skijer's NEI
static void BallChain_FeedTrail(PlayState* play, Vec3f* ballPos) {
    Vec3f tip, base;

    if (!bcTrailActive) {
        EffectBlureInit2 init = {
            0,                      // calcMode
            8,                      // flags
            0,                      // addAngleChange
            { 255, 255, 255, 255 }, // p1StartColor (white — clearly visible)
            { 200, 220, 255, 128 }, // p2StartColor (cool tint, softer edge)
            { 255, 255, 255, 0 },   // p1EndColor
            { 200, 220, 255, 0 },   // p2EndColor
            4,                      // elemDuration
            0,                      // unkFlag
            EFF_BLURE_DRAW_MODE_SMOOTH,
            0,                      // mode4Param
            { 235, 235, 245, 200 }, // altPrimColor
            { 180, 190, 210, 96 },  // altEnvColor
        };
        Effect_Add(play, &bcTrailIndex, EFFECT_BLURE2, 0, 0, &init);
        bcTrailActive = 1;
        bcTrailTick = 0;
    }

    // Feed a segment EVERY frame (like the sword) so consecutive vertices form a continuous strip.
    // Feeding sparser left fewer than 2 live elements at a time, so the blure drew nothing — the
    // subtler-than-sword look comes from the short elemDuration + softer alpha instead. Skijer's NEI
    tip = *ballPos;
    tip.y += 14.0f;
    base = *ballPos;
    base.y -= 14.0f;
    EffectBlure_AddVertex((EffectBlure*)Effect_GetByIndex(bcTrailIndex), &tip, &base);
}

static void BallChain_KillTrail(PlayState* play) {
    if (bcTrailActive) {
        Effect_Delete(play, bcTrailIndex);
        bcTrailActive = 0;
    }
    bcTrailIndex = -1;
}

// Helper: smash a nearby MM breakable (pot/crate/grass/rock) — puff + SFX + small drop.
static void BallChain_SmashBreakable(PlayState* play, Actor* actor) {
    static Vec3f sZeroVec = { 0.0f, 0.0f, 0.0f };
    Vec3f pos;
    EnItem00* drop;

    pos.x = actor->world.pos.x;
    pos.y = actor->world.pos.y + 20.0f;
    pos.z = actor->world.pos.z;

    EffectSsBomb2_SpawnLayered(play, &pos, &sZeroVec, &sZeroVec, 60, 25);
    Audio_PlaySoundGeneral(NA_SE_EV_POT_BROKEN, &actor->world.pos, 4, &gSfxDefaultFreqAndVolScale,
                           &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);

    drop = Item_DropCollectible(play, &pos, ITEM00_RUPEE_GREEN);
    if (drop != NULL) {
        drop->actor.velocity.y = 12.0f;
    }

    if (actor->child != NULL) {
        actor->child->parent = NULL;
        Actor_Kill(actor->child);
    }
    Actor_Kill(actor);
}

// (OoT Shadow Temple / Goron basket destroy helpers removed — MM uses the generic
//  BallChain_SmashBreakable above for pots/crates/grass/rocks.)

// Generic ice-shatter feedback: ice-piece burst + break sfx (no fire). Skijer's NEI
static void BallChain_IceBurst(PlayState* play, Vec3f* pos) {
    EffectSsIcePiece_SpawnBurst(play, pos, 1.5f);
    Audio_PlaySoundGeneral(NA_SE_EV_ICE_BROKEN, pos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale,
                           &gSfxDefaultReverb);
}

// Obj_Ice_Poly (red meltable ice) melts ONLY on an exact dmgFlags == 0x800 fire hit OR when its
// switch flag is set (z_obj_ice_poly.c func_80931A38). We can't use fire (it lights torches), so we
// drive its OWN native shatter by setting the switch flag: next frame it queues its shatter cutscene,
// bursts and dies AND opens the path it was gating (Flags_SetSwitch inside its melt). For decorative
// ice with no switch flag (0xFF) there's nothing to gate, so just burst + kill it ourselves.
// Skijer's NEI
static void BallChain_ShatterIce(PlayState* play, Actor* actor) {
    ObjIcePoly* ice = (ObjIcePoly*)actor;

    if (ice->switchFlag != OBJICEPOLY_SWITCH_FLAG_NONE) {
        Flags_SetSwitch(play, ice->switchFlag); // native melt + path-open, no fire
    } else {
        BallChain_IceBurst(play, &actor->world.pos);
        Actor_Kill(actor);
    }
}

static void BallChain_CheckDestructibles(PlayState* play, Vec3f* ballPos) {
    // Ice actors span several categories: Bg_Icicle/Obj_Bigicicle in PROP, Obj_Iceblock in BG,
    // Obj_Ice_Poly in ITEMACTION, En_Fz/En_Snowman in ENEMY. Scan all so every kind of ice is
    // reachable. Everything below is triggered by PROXIMITY (the fast throw/retract tunnels past
    // thin ice and small enemies), each via its OWN native break/melt so drops, path-flags and
    // puzzles all fire — with NO fire damage. Skijer's NEI
    static const s32 sCats[4] = { ACTORCAT_PROP, ACTORCAT_BG, ACTORCAT_ITEMACTION, ACTORCAT_ENEMY };
    s32 c;

    for (c = 0; c < 4; c++) {
        Actor* actor = play->actorCtx.actorLists[sCats[c]].first;
        Actor* next;
        for (; actor != NULL; actor = next) {
            f32 dist = Math_Vec3f_DistXYZ(ballPos, &actor->world.pos);
            next = actor->next;
            switch (actor->id) {
                case ACTOR_OBJ_TSUBO:   // pot
                case ACTOR_OBJ_KIBAKO:  // small crate
                case ACTOR_OBJ_KIBAKO2: // large crate
                case ACTOR_EN_KUSA:     // grass
                case ACTOR_EN_ISHI:     // liftable rock
                    if (dist < BALLCHAIN_BREAKABLE_REACH) {
                        BallChain_SmashBreakable(play, actor);
                    }
                    break;

                // The Termina "stalactite cutscene" puzzle IS this pair: Obj_Bigicicle is the falling
                // stalactite, Obj_Ice_Poly is the large ice it lands on. Big actors → bigger reach. Skijer's NEI
                case ACTOR_OBJ_ICE_POLY: // red meltable ice — set its switch flag (native melt + path)
                    if (dist < BALLCHAIN_BIGICE_REACH) {
                        BallChain_ShatterIce(play, actor);
                    }
                    break;
                case ACTOR_BG_ICICLE: // hanging/ground icicles — break on ANY player AC hit
                    if (dist < BALLCHAIN_ICE_REACH) {
                        ((BgIcicle*)actor)->collider.base.acFlags |= AC_HIT;
                    }
                    break;
                case ACTOR_OBJ_BIGICICLE: // large stalactite — collider1 breaks on any AC hit → fall + cutscene
                    if (dist < BALLCHAIN_BIGICE_REACH) {
                        ((ObjBigicicle*)actor)->collider1.base.acFlags |= AC_HIT;
                    }
                    break;
                case ACTOR_OBJ_ICEBLOCK: { // frozen-enemy cube — force its native melt (fire OR 22s timer)
                    ObjIceblock* ice = (ObjIceblock*)actor;
                    // Skip the ICEBERG variant: it's a floating platform you stand on. Only trip a
                    // block that hasn't already started melting (meltTimer > 0). Skijer's NEI
                    if ((dist < BALLCHAIN_BIGICE_REACH) && !ICEBLOCK_GET_ICEBERG(actor) && (ice->meltTimer > 0)) {
                        ice->meltTimer = 0;
                    }
                    break;
                }
                case ACTOR_EN_FZ: { // Freezard — drive its own fire-melt reaction (melt + drop), no fire
                    EnFz* fz = (EnFz*)actor;
                    if ((dist < BALLCHAIN_ICE_REACH) && (fz->unk_BD8 == 0)) { // unk_BD8 != 0 = already dying
                        fz->actor.colChkInfo.damageEffect = 2;                // fire-melt effect
                        fz->collider1.base.acFlags |= AC_HIT;
                    }
                    break;
                }
                case ACTOR_EN_SNOWMAN: // Eeno — native melt (handles split/large parts + drop)
                    // AC_ON is cleared by SetupMelt, so this only fires once per Eeno. Skijer's NEI
                    if ((dist < BALLCHAIN_ICE_REACH) && (((EnSnowman*)actor)->collider.base.acFlags & AC_ON)) {
                        EnSnowman_SetupMelt((EnSnowman*)actor);
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

// (Ice enemies like En_Fz/En_St take the collider's normal goron-punch damage through their own
//  tables — no fire, no hardcoded kills. Static ice OBSTACLES are handled above. Skijer's NEI)

// =============================================================================
// Core Functions
// =============================================================================

static void BallChain_ApplySpeedPenalty(Player* p) {
    p->actor.speed *= BALLCHAIN_SPEED_MULT;
    p->linearVelocity *= BALLCHAIN_SPEED_MULT;
}

// HARD INTERRUPTS — StateSpinning/StateThrown pin Link's speed to 0 and re-stamp his yaw EVERY
// frame, so anything that takes control away from him MUST drop the item or he stays frozen and
// softlocks. ItemInput_CheckDamage only fires on a POSITIVE invincibilityTimer edge, but a real hit
// drives the timer NEGATIVE for the whole damage/knockback reaction (z64player.h: "negative are
// invulnerability") and it may never go positive — so a big knockback slipped past it entirely.
// Skijer's NEI
static u8 BallChain_ShouldInterrupt(Player* p, PlayState* play) {
    if (p->invincibilityTimer < 0) { // damage / knockback reaction in progress
        return 1;
    }
    // Fell in water / swimming — let go. PLAYER_STATE1_8000000 (bit 27) is MM's native name for the
    // flag OoT calls PLAYER_STATE1_IN_WATER; that alias only exists in nei_oot_compat.h, which this
    // file doesn't include, so use the native one and stay independent of it. Skijer's NEI
    if (p->stateFlags1 & PLAYER_STATE1_8000000) {
        return 1;
    }
    if (Player_InBlockingCsMode(play, p)) { // cutscene / forced state
        return 1;
    }
    return 0;
}

static void BallChain_Stop(Player* p, PlayState* play) {
    if (bcFirstPerson) {
        FirstPerson_Exit(p, play);
        bcFirstPerson = 0;
    }
    bcCollider.base.atFlags &= ~(AT_ON | AT_HIT);
    bcActive = 0;
    bcState = BALLCHAIN_STATE_INACTIVE;
    bcCharge = 0;
    bcSpinAngle = 0;
    // TP rework — clear thrown-ball ballistic state. Skijer's NEI
    bcPhase = BALLCHAIN_PHASE_FLY;
    bcBounces = 0;
    bcRestTimer = 0;
    bcBallVel.x = bcBallVel.y = bcBallVel.z = 0.0f;
    BallChain_KillTrail(play); // drop the motion streak — Skijer's NEI
    BallChain_ResetPose(p);
    // The states above pin playSpeed at 0 every frame; if we let go mid-freeze (knockback, water)
    // Link's animation would stay stuck. Hand it back so he can move again. Skijer's NEI
    p->skelAnime.playSpeed = 1.0f;
    // (All sfx are one-shots now — no flagged/looping sounds to stop. Skijer's NEI)
    ItemEquip_PlayUnequipSFX(play, p);
}

static void BallChain_Start(Player* p, PlayState* play) {
    if (bcActive)
        return;
    bcActive = 1;
    bcCharge = 0;
    bcSpinAngle = 0;
    bcFirstPerson = 0;
    bcState = BALLCHAIN_STATE_EQUIP;
    ItemEquip_PlayEquipSFX(play, p);
}

// =============================================================================
// State: Equip
// =============================================================================

static void StateEquip(Player* p, PlayState* play, ItemInputState* in) {
    s16 yaw = p->actor.shape.rot.y;
    Vec3f* leftHand = &p->bodyPartsPos[PLAYER_BODYPART_L_HAND];
    Vec3f* rightHand = &p->bodyPartsPos[PLAYER_BODYPART_R_HAND];

    // Clear the motion streak once the ball is back in the hand (covers throw-return + timeout). Skijer's NEI
    if (bcTrailActive) {
        BallChain_KillTrail(play);
    }

    BallChain_ApplySpeedPenalty(p);
    p->skelAnime.playSpeed = 0.0f;
    BallChain_SetEquipPose(p);

    // TWO-HANDED grip: the ball sits at the MIDPOINT of BOTH hands — the exact same point the chain
    // is drawn from (CustomItems_DrawBallChain uses midHand). Anchoring the ball there keeps it
    // centered between Link's hands for every facing (both hands rotate with him), so it never drifts
    // to one side, and the ball stays visually joined to the chain origin. Skijer's NEI
    bcBallPos.x = (leftHand->x + rightHand->x) * 0.5f;
    bcBallPos.y = (leftHand->y + rightHand->y) * 0.5f + BALLCHAIN_EQUIP_Y_OFFSET;
    bcBallPos.z = (leftHand->z + rightHand->z) * 0.5f;

    // Collider stays live even while just holding the ball (TP style — it's always a weapon), so it
    // keeps hitting whatever it contacts through the whole cycle, including right after it returns to
    // the hand. Contact-only here: NO proximity sweep while idle (standing next to ice shouldn't nuke
    // it — you spin/throw for that); spinning/thrown do the ranged ice sweep below. Skijer's NEI
    BallChain_UpdateCollider(play, p, &bcBallPos);
    BallChain_CheckHit(&bcBallPos);

    if (in->isPressed) {
        bcState = BALLCHAIN_STATE_SPINNING;
        bcCharge = 0;
        bcThrowYaw = yaw;
        Audio_PlaySoundGeneral(BALLCHAIN_SFX_SWING, &p->actor.world.pos, 4, &gSfxDefaultFreqAndVolScale,
                               &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
    }
}

// =============================================================================
// State: Spinning
// =============================================================================

// TP spin-up: the ball orbits Link, ramping up over ~1.5s; Link shuffles slowly and
// turns with heavy inertia (hard-capped yaw step per frame). Skijer's NEI
static void StateSpinning(Player* p, PlayState* play, ItemInputState* in) {
    s8 rawStickX = play->state.input[0].cur.stick_x;
    s8 rawStickY = play->state.input[0].cur.stick_y;
    u8 isZTarget = Player_IsZTargeting(p);
    f32 stickX = 0.0f;
    f32 stickY = 0.0f;
    f32 stickMag, chargeRatio, spinHeight;
    f32 orbitY, heightMod, sideMod;
    s16 stickAngle, desiredYaw, spinSpeed;
    u16 prevSpinAngle;

    // Heavy movement: Link can always walk SLOWLY while spinning (TP), except while
    // aiming in first person. The multiplier caps whatever speed the normal player
    // movement code built up this frame. Skijer's NEI
    if (bcFirstPerson) {
        p->actor.speed = 0.0f;
        p->linearVelocity = 0.0f;
        p->skelAnime.playSpeed = 0.0f;
    } else {
        p->actor.speed *= BALLCHAIN_SPIN_WALK_MULT;
        p->linearVelocity *= BALLCHAIN_SPIN_WALK_MULT;
        p->skelAnime.playSpeed = (fabsf(p->linearVelocity) > 0.3f) ? 0.5f : 0.0f;
    }

    if (CHECK_BTN_ALL(play->state.input[0].press.button, BTN_CUP)) {
        if (bcFirstPerson) {
            FirstPerson_Exit(p, play);
            bcFirstPerson = 0;
        } else {
            FirstPerson_Init(p, play);
            bcFirstPerson = 1;
        }
    }

    if (bcFirstPerson) {
        FirstPerson_Update(p, play);
    }

    // TP heavy inertia: Link's yaw follows the stick (camera-relative) with a REDUCED
    // angular step instead of turning instantly. Skijer's NEI
    Lib_GetControlStickData(&stickMag, &stickAngle, &play->state.input[0]);
    if (isZTarget && p->focusActor != NULL) {
        desiredYaw = Math_Vec3f_Yaw(&p->actor.world.pos, &p->focusActor->focus.pos);
    } else if (bcFirstPerson) {
        desiredYaw = FirstPerson_GetAimYaw(p);
    } else if (stickMag > BALLCHAIN_STICK_DEADZONE) {
        desiredYaw = Camera_GetInputDirYaw(GET_ACTIVE_CAM(play)) + stickAngle;
    } else {
        desiredYaw = bcThrowYaw; // no input: hold current facing
    }
    Math_ScaledStepToS(&bcThrowYaw, desiredYaw, BALLCHAIN_TURN_RATE);
    // Stamped here AND re-stamped late in BallChain_RefreshPose (after Player_UpdateCommon)
    // so the player's own fast turning code can't override the slow turn. Skijer's NEI
    p->actor.shape.rot.y = bcThrowYaw;
    p->actor.world.rot.y = bcThrowYaw;
    p->yaw = bcThrowYaw;

    if (!isZTarget && stickMag > BALLCHAIN_STICK_DEADZONE) {
        stickX = (f32)rawStickX / 127.0f;
        stickY = (f32)rawStickY / 127.0f;
    }

    if (bcCharge < BALLCHAIN_CHARGE_MAX) {
        bcCharge++;
    }

    // Wind-up ramp: orbit speed and height rise to max over BALLCHAIN_CHARGE_MAX frames.
    chargeRatio = (f32)bcCharge / (f32)BALLCHAIN_CHARGE_MAX;
    spinSpeed = (s16)(BALLCHAIN_SPIN_SPEED_MIN + (BALLCHAIN_SPIN_SPEED_MAX - BALLCHAIN_SPIN_SPEED_MIN) * chargeRatio);
    prevSpinAngle = (u16)bcSpinAngle;
    bcSpinAngle += spinSpeed;
    // Heavy whoosh once per full revolution — frequency rises with the wind-up. Skijer's NEI
    if ((u16)bcSpinAngle < prevSpinAngle) {
        Audio_PlaySoundGeneral(BALLCHAIN_SFX_WHOOSH, &bcBallPos, 4, &gSfxDefaultFreqAndVolScale,
                               &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
    }
    spinHeight = BALLCHAIN_SPIN_HEIGHT_MIN + (BALLCHAIN_SPIN_HEIGHT_MAX - BALLCHAIN_SPIN_HEIGHT_MIN) * chargeRatio;

    // Ball orbits Link in world space (radius BALLCHAIN_SPIN_RADIUS ~ TP's wide orbit),
    // with a little stick-driven wobble on the orbit height. Skijer's NEI
    orbitY = spinHeight;
    heightMod = -Math_CosS(bcSpinAngle) * stickY * BALLCHAIN_LEAN_TILT;
    sideMod = -Math_SinS(bcSpinAngle) * stickX * BALLCHAIN_LEAN_TILT;
    orbitY += heightMod + sideMod;

    bcBallPos.x = p->actor.world.pos.x + Math_SinS(bcSpinAngle) * BALLCHAIN_SPIN_RADIUS;
    bcBallPos.y = p->actor.world.pos.y + orbitY;
    bcBallPos.z = p->actor.world.pos.z + Math_CosS(bcSpinAngle) * BALLCHAIN_SPIN_RADIUS;

    BallChain_SetSpinPose(p, stickX, stickY);

    BallChain_UpdateCollider(play, p, &bcBallPos);
    BallChain_CheckDestructibles(play, &bcBallPos);
    BallChain_CheckHit(&bcBallPos);
    BallChain_FeedTrail(play, &bcBallPos); // spin streak — Skijer's NEI

    if (!in->isHeld) {
        // RELEASE: violent ballistic launch in the aimed direction. Skijer's NEI
        f32 launchSpeed =
            BALLCHAIN_LAUNCH_SPEED_MIN + (BALLCHAIN_LAUNCH_SPEED_MAX - BALLCHAIN_LAUNCH_SPEED_MIN) * chargeRatio;
        s16 throwYaw = bcThrowYaw;
        s16 throwPitch = 0;

        if (bcFirstPerson) {
            throwYaw = FirstPerson_GetAimYaw(p);
            throwPitch = FirstPerson_GetAimPitch(p);
            FirstPerson_Exit(p, play);
            bcFirstPerson = 0;
        }

        // Launch origin: over Link's shoulder, slightly forward.
        bcBallPos.x = p->actor.world.pos.x + Math_SinS(throwYaw) * 20.0f;
        bcBallPos.y = p->actor.world.pos.y + 45.0f;
        bcBallPos.z = p->actor.world.pos.z + Math_CosS(throwYaw) * 20.0f;

        if (isZTarget && p->focusActor != NULL && p->focusActor->update != NULL) {
            // Lock-on: ballistic lead — aim the arc so gravity drops the ball ON the target.
            Vec3f* tPos = &p->focusActor->focus.pos;
            f32 dx = tPos->x - bcBallPos.x;
            f32 dy = tPos->y - bcBallPos.y;
            f32 dz = tPos->z - bcBallPos.z;
            f32 flightT = sqrtf(SQ(dx) + SQ(dz)) / launchSpeed;

            if (flightT < 1.0f) {
                flightT = 1.0f;
            }
            bcBallVel.x = dx / flightT;
            bcBallVel.z = dz / flightT;
            bcBallVel.y = (dy / flightT) - (0.5f * BALLCHAIN_GRAVITY * flightT); // gravity compensation
            throwYaw = Math_Atan2S(dx, dz);                                      // MM arg order: yaw of (dx, dz)
        } else {
            bcBallVel.x = Math_SinS(throwYaw) * Math_CosS(throwPitch) * launchSpeed;
            bcBallVel.z = Math_CosS(throwYaw) * Math_CosS(throwPitch) * launchSpeed;
            // Positive pitch aims down (same convention as FirstPerson_GetAimPitch).
            bcBallVel.y = BALLCHAIN_LAUNCH_VY - Math_SinS(throwPitch) * launchSpeed;
        }

        bcThrowYaw = throwYaw;
        bcState = BALLCHAIN_STATE_THROWN;
        bcPhase = BALLCHAIN_PHASE_FLY;
        bcBounces = 0;
        bcRestTimer = 0;
        bcCharge = 0; // reused as the thrown-safety frame counter

        ItemVoice_Play(p, BALLCHAIN_SFX_VOICE_ADULT, BALLCHAIN_SFX_VOICE_CHILD);
        Audio_PlaySoundGeneral(BALLCHAIN_SFX_SWING, &p->actor.world.pos, 4, &gSfxDefaultFreqAndVolScale,
                               &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
        Rumble_Request(0.0f, 120, 8, 20); // violent launch kick — Skijer's NEI
    }
}

// =============================================================================
// State: Thrown
// =============================================================================

// TP thrown ball: real gravity arc -> hard floor bounces (thud + quake + rumble) ->
// wall clank + drop -> rest a beat -> retract along the chain. Link is braced in
// place the whole time. Skijer's NEI
static void StateThrown(Player* p, PlayState* play) {
    f32 dist, dx, dy, dz, norm;
    CollisionPoly* poly = NULL;
    Vec3f prevPos, resultPos;

    p->actor.speed = 0.0f;
    p->linearVelocity = 0.0f;
    p->skelAnime.playSpeed = 0.0f;

    p->actor.shape.rot.y = bcThrowYaw;
    p->actor.world.rot.y = bcThrowYaw;
    p->yaw = bcThrowYaw;

    BallChain_SetSpinPose(p, 0.0f, 0.0f);
    p->upperLimbRot.x = BALLCHAIN_THROW_LEAN;

    // Hard safety: never leave Link braced forever (bcCharge = thrown frame counter).
    bcCharge++;
    if (bcCharge > BALLCHAIN_THROWN_TIMEOUT) {
        bcState = BALLCHAIN_STATE_EQUIP;
        return;
    }

    if (bcPhase == BALLCHAIN_PHASE_FLY) {
        CollisionPoly* floorPoly = NULL;
        s32 bgId;
        Vec3f probe;
        f32 floorY, xzDist;

        prevPos = bcBallPos;

        // Heavy ballistic integration (20fps logic frames).
        bcBallVel.y += BALLCHAIN_GRAVITY;
        if (bcBallVel.y < -BALLCHAIN_TERMINAL_VY) {
            bcBallVel.y = -BALLCHAIN_TERMINAL_VY;
        }
        bcBallPos.x += bcBallVel.x;
        bcBallPos.y += bcBallVel.y;
        bcBallPos.z += bcBallVel.z;

        // Chain taut: the ball can never fly past the chain length — the outward
        // momentum dies and the ball drops at the end of the chain. Skijer's NEI
        dx = bcBallPos.x - p->actor.world.pos.x;
        dz = bcBallPos.z - p->actor.world.pos.z;
        xzDist = sqrtf(SQ(dx) + SQ(dz));
        if (xzDist > BALLCHAIN_CHAIN_MAX) {
            f32 clamp = BALLCHAIN_CHAIN_MAX / xzDist;

            bcBallPos.x = p->actor.world.pos.x + dx * clamp;
            bcBallPos.z = p->actor.world.pos.z + dz * clamp;
            bcBallVel.x = 0.0f;
            bcBallVel.z = 0.0f;
        }

        // Wall hit: RICOCHET — reflect the horizontal velocity off the wall normal so the ball
        // bounces off walls (TP feel) instead of dropping dead against them. Metal clank + rumble
        // on a solid hit; the floor logic below still settles + retracts it once it lands. Skijer's NEI
        resultPos = bcBallPos;
        if (BgCheck_EntitySphVsWall1(&play->colCtx, &resultPos, &bcBallPos, &prevPos, BALLCHAIN_WALL_RADIUS, &poly,
                                     BALLCHAIN_WALL_HEIGHT)) {
            bcBallPos = resultPos;
            if (fabsf(bcBallVel.x) + fabsf(bcBallVel.z) > 1.0f) {
                Audio_PlaySoundGeneral(BALLCHAIN_SFX_WALL_BOUNCE, &bcBallPos, 4, &gSfxDefaultFreqAndVolScale,
                                       &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
                Rumble_Request(0.0f, 120, 6, 20);
            }
            if (poly != NULL) {
                // v' = v - 2(v·n)n, then damp — reflect XZ across the wall's horizontal normal.
                f32 nx = COLPOLY_GET_NORMAL(poly->normal.x);
                f32 nz = COLPOLY_GET_NORMAL(poly->normal.z);
                f32 dot = (bcBallVel.x * nx) + (bcBallVel.z * nz);

                bcBallVel.x = (bcBallVel.x - (2.0f * dot * nx)) * BALLCHAIN_WALL_BOUNCE_FACTOR;
                bcBallVel.z = (bcBallVel.z - (2.0f * dot * nz)) * BALLCHAIN_WALL_BOUNCE_FACTOR;
            } else {
                bcBallVel.x = 0.0f;
                bcBallVel.z = 0.0f;
            }
        }

        // Ground bounce: heavy thud + quake, invert velocity.y, up to MAX_BOUNCES,
        // then rest a beat before retracting. Skijer's NEI
        probe = bcBallPos;
        probe.y += 20.0f;
        floorY = BgCheck_EntityRaycastFloor5(&play->colCtx, &floorPoly, &bgId, &p->actor, &probe);
        if ((floorY > BGCHECK_Y_MIN) && (bcBallPos.y - BALLCHAIN_BALL_RADIUS <= floorY) && (bcBallVel.y <= 0.0f)) {
            bcBallPos.y = floorY + BALLCHAIN_BALL_RADIUS;
            bcBounces++;

            Audio_PlaySoundGeneral(BALLCHAIN_SFX_HIT, &bcBallPos, 4, &gSfxDefaultFreqAndVolScale,
                                   &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
            Actor_RequestQuake(play, 2, 8);
            Rumble_Request(0.0f, 160, 10, 20);

            if ((bcBounces >= BALLCHAIN_MAX_BOUNCES) || (fabsf(bcBallVel.y) < 3.0f)) {
                bcBallVel.x = bcBallVel.y = bcBallVel.z = 0.0f;
                bcPhase = BALLCHAIN_PHASE_REST;
                bcRestTimer = BALLCHAIN_REST_FRAMES;
            } else {
                bcBallVel.y = -bcBallVel.y * BALLCHAIN_BOUNCE_FACTOR;
                bcBallVel.x *= BALLCHAIN_BOUNCE_XZ_KEEP;
                bcBallVel.z *= BALLCHAIN_BOUNCE_XZ_KEEP;
            }
        } else if (bcBallPos.y < p->actor.world.pos.y - 500.0f) {
            // Thrown into the void — just reel it back in.
            bcPhase = BALLCHAIN_PHASE_RETRACT;
            bcRestTimer = 0;
        }
    } else if (bcPhase == BALLCHAIN_PHASE_REST) {
        bcRestTimer--;
        if (bcRestTimer <= 0) {
            bcPhase = BALLCHAIN_PHASE_RETRACT;
            bcRestTimer = 0; // reused below as the retract clink counter
        }
    } else { // BALLCHAIN_PHASE_RETRACT
        dist = Math_Vec3f_DistXYZ(&bcBallPos, &p->actor.world.pos);

        if (dist > BALLCHAIN_RETURN_DIST) {
            dx = p->actor.world.pos.x - bcBallPos.x;
            dy = (p->actor.world.pos.y + 45.0f) - bcBallPos.y;
            dz = p->actor.world.pos.z - bcBallPos.z;
            norm = sqrtf(SQ(dx) + SQ(dy) + SQ(dz));

            if (norm > 0.1f) {
                norm = BALLCHAIN_RETRACT_SPEED / norm;
                bcBallPos.x += dx * norm;
                bcBallPos.y += dy * norm;
                bcBallPos.z += dz * norm;
            }
            // Chain clinks while reeling in — one-shots, never a flagged loop. Skijer's NEI
            bcRestTimer++;
            if ((bcRestTimer & 3) == 0) {
                Audio_PlaySoundGeneral(BALLCHAIN_SFX_CHAIN, &bcBallPos, 4, &gSfxDefaultFreqAndVolScale,
                                       &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
            }
        } else {
            bcState = BALLCHAIN_STATE_EQUIP;
            return;
        }
    }

    BallChain_UpdateCollider(play, p, &bcBallPos);
    BallChain_CheckDestructibles(play, &bcBallPos);
    BallChain_CheckHit(&bcBallPos);
    if (bcPhase != BALLCHAIN_PHASE_REST) {
        BallChain_FeedTrail(play,
                            &bcBallPos); // streak while airborne (fly + retract), not while resting — Skijer's NEI
    }
}

// =============================================================================
// Public API
// =============================================================================

// Re-applies the ball & chain arm pose AFTER the frame's animation has been sampled into jointTable.
// 2ship runs CustomItems_Update (Handle_BallAndChain) at the TOP of Player_Update, BEFORE
// Player_UpdateCommon → PlayerAnimation_Update re-copies the idle anim into jointTable and WIPES the
// manually-written arm pose (SoH ran CustomItems_Update after, so its pose stuck). This runs late
// (via CustomItems_LatePose) to re-stamp the jointTable. Pose-only: the jointTable writes are
// constant, and the stick-driven upperLimbRot lean set by Handle_BallAndChain survives the anim
// update, so we save/restore it around the pose call (the pose helpers overwrite it). Skijer's NEI
void BallChain_RefreshPose(Player* p) {
    Vec3s savedRot;

    if (!bcActive) {
        return;
    }

    savedRot = p->upperLimbRot; // preserve the lean already set this frame by Handle_BallAndChain
    p->skelAnime.playSpeed = 0.0f;

    if (bcState == BALLCHAIN_STATE_EQUIP) {
        BallChain_SetEquipPose(p);
    } else if (bcState == BALLCHAIN_STATE_SPINNING || bcState == BALLCHAIN_STATE_THROWN) {
        BallChain_SetSpinPose(p, 0.0f, 0.0f); // jointTable writes are stick-independent
        // TP heavy inertia: re-stamp the rate-capped yaw AFTER Player_UpdateCommon so the
        // player's own (fast) turning code can never override the slow turn. Skijer's NEI
        p->actor.shape.rot.y = bcThrowYaw;
        p->actor.world.rot.y = bcThrowYaw;
        p->yaw = bcThrowYaw;
    } else {
        return;
    }

    p->upperLimbRot = savedRot; // the pose helpers zeroed it; restore the real lean
}

void Handle_BallAndChain(Player* p, PlayState* play) {
    ItemInputState in;

    if (!sBallChainColInitialized) {
        BallChain_InitCollider(play, p);
    }

    ItemInput_Update(&in, ITEM_BALL_AND_CHAIN, p, play);

    if (!in.wasEquipped || ItemInput_IsBlocked(p, play) || ItemInput_CheckDamage(p, &sBallChainPrevInvinc) ||
        BallChain_ShouldInterrupt(p, play)) {
        if (bcActive)
            BallChain_Stop(p, play);
        return;
    }
    if (in.otherButtonPressed) {
        BallChain_Stop(p, play);
        return;
    }

    if (!bcActive) {
        if (in.isPressed || in.isHeld) {
            BallChain_Start(p, play);
        }
        return;
    }

    switch (bcState) {
        case BALLCHAIN_STATE_EQUIP:
            StateEquip(p, play, &in);
            break;
        case BALLCHAIN_STATE_SPINNING:
            StateSpinning(p, play, &in);
            break;
        case BALLCHAIN_STATE_THROWN:
            StateThrown(p, play);
            break;
        default:
            bcState = BALLCHAIN_STATE_EQUIP;
            break;
    }
}

void Player_InitBallAndChainIA(PlayState* play, Player* p) {
    BallChain_InitCollider(play, p);
    bcActive = 0;
    bcCharge = 0;
    bcSpinAngle = 0;
    bcFirstPerson = 0;
    bcState = BALLCHAIN_STATE_INACTIVE;
    // TP rework — ballistic thrown-ball state. Skijer's NEI
    bcPhase = BALLCHAIN_PHASE_FLY;
    bcBounces = 0;
    bcRestTimer = 0;
    bcBallVel.x = bcBallVel.y = bcBallVel.z = 0.0f;
    // Motion trail starts inactive. Skijer's NEI
    bcTrailActive = 0;
    bcTrailIndex = -1;
}
