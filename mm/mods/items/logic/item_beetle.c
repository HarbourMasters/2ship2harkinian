/**
 * item_beetle.c - Beetle from Skyward Sword
 *
 * Controls:
 *   C Button:    Launch beetle in aimed direction
 *   Analog:      Steer beetle flight path
 *   C Button:    Recall beetle early
 *   B Button:    Boost speed temporarily
 *
 * Features:
 *   - Remote-controlled flying beetle with camera follow
 *   - Can grab and carry items back to Link
 *   - Damages enemies on impact
 *   - Limited flight time before returning
 */

#include "z64.h"
#include "item_beetle.h"
#include "../custom_items.h"
#include "../helpers/camera_helper.h"
#include "../helpers/equip_helper.h"
#include "../helpers/combat_helper.h"
#include "../helpers/item_voice.h"
#include "macros.h"
#include "functions.h"
#include "variables.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"

// z_player.c internal: drives MM's real bow/arrow aim camera (sets unk_AA5 = PLAYER_UNKAA5_3 each
// frame so func_80847190 updates actor.focus.rot). FirstPerson_Init only sets flags and never aimed;
// its PLAYER_STATE1_FIRST_PERSON flag also fights MM's aim state (whip oscillation), so the beetle
// uses func_80831010 directly + unk_AA5 management, like the Switch Hook. Skijer's NEI
extern bool func_80831010(Player* this, PlayState* play);

static u8 sBeetleColInitialized = 0;
static s8 sBeetlePrevInvinc = 0;
static Actor* sBeetleTarget = NULL;    // Z-locked enemy (NULL = no lock). Skijer's NEI
static Actor* sBeetleCandidate = NULL; // nearest lockable actor each frame (drives the "offer" reticle)
static u8 sBeetleKamikaze = 0;         // B-released: home hard at the target to hit it, then return
static u8 sBeetleAutonomous = 0;       // B pressed: camera+control back to Link; beetle flies on its own

static void Beetle_DropGrabbedActor(Player* p);

static ColliderCylinderInit sBeetleColliderInit = {
    { COL_MATERIAL_NONE, AT_ON | AT_TYPE_PLAYER, AC_NONE, OC1_ON | OC1_TYPE_ALL, OC2_TYPE_PLAYER, COLSHAPE_CYLINDER },
    { ELEM_MATERIAL_UNK2,
      { BEETLE_DMG_FLAGS, 0x00, 0x01 },
      { 0xFFCFFFFF, 0x00, 0x00 },
      ATELEM_ON | ATELEM_NEAREST | ATELEM_SFX_NORMAL,
      ACELEM_NONE,
      OCELEM_ON },
    { (s16)BEETLE_DAMAGE_RADIUS, (s16)BEETLE_DAMAGE_HEIGHT, 0, { 0, 0, 0 } }
};

static void Beetle_InitCollider(PlayState* play, Player* p) {
    if (sBeetleColInitialized)
        return;
    Collider_InitCylinder(play, &beetleCollider);
    Collider_SetCylinder(play, &beetleCollider, &p->actor, &sBeetleColliderInit);
    sBeetleColInitialized = 1;
}

static void Beetle_UpdateCollider(PlayState* play, Vec3f* pos) {
    beetleCollider.dim.pos.x = (s16)pos->x;
    beetleCollider.dim.pos.y = (s16)pos->y;
    beetleCollider.dim.pos.z = (s16)pos->z;
    beetleCollider.base.atFlags |= AT_ON | AT_TYPE_PLAYER;
    CollisionCheck_SetAT(play, &play->colChkCtx, &beetleCollider.base);
    CollisionCheck_SetOC(play, &play->colChkCtx, &beetleCollider.base);
}

static void Beetle_PlaySound(Vec3f* pos, u16 sfxId) {
    Audio_PlaySoundGeneral(sfxId, pos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
}

static void Beetle_PlayLoopSound(Actor* actor, u16 sfxId) {
    func_8002F974(actor, sfxId - SFX_FLAG);
}

u8 Beetle_IsFlying(void) {
    return beetleActive && (beetleState == BEETLE_STATE_FLYING || beetleState == BEETLE_STATE_RETURNING);
}

static void Beetle_DestroySubCam(PlayState* play) {
    if (beetleSubCamId != SUBCAM_FREE) {
        // Force MAIN_CAM out of CAM_MODE_FOLLOWBOOMERANG before reactivating it.
        // While the beetle flew we set PLAYER_STATE1_BOOMERANG_THROWN, which
        // makes z_player.c put MAIN_CAM into FOLLOWBOOMERANG mode pointed at
        // a stale Player.boomerangActor. Reactivating in that mode can deref
        // freed memory (Camera_KeepOn1) and crash — common during Barinade
        // phase 4 where actor churn fills the freed En_Boom slot with valid-
        // looking data, defeating the camera->target->update == NULL guard.
        Camera_ChangeMode(Play_GetCamera(play, MAIN_CAM), CAM_MODE_NORMAL);
        Play_ChangeCameraStatus(play, MAIN_CAM, CAM_STAT_ACTIVE);
        Play_ClearCamera(play, beetleSubCamId);
        beetleSubCamId = SUBCAM_FREE;
    }
}

static void Beetle_CreateSubCam(PlayState* play) {
    if (beetleSubCamId == SUBCAM_FREE) {
        beetleSubCamId = Play_CreateSubCamera(play);
        Play_ChangeCameraStatus(play, MAIN_CAM, CAM_STAT_WAIT);
        Play_ChangeCameraStatus(play, beetleSubCamId, CAM_STAT_ACTIVE);
    }
}

static void Beetle_UpdateSubCam(PlayState* play) {
    if (beetleSubCamId == SUBCAM_FREE)
        return;

    f32 sinY = Math_SinS(beetleRot.y);
    f32 cosY = Math_CosS(beetleRot.y);
    f32 sinP = Math_SinS(beetleRot.x);
    f32 cosP = Math_CosS(beetleRot.x);

    Vec3f eye;
    eye.x = beetlePos.x - sinY * cosP * BEETLE_CAM_DISTANCE;
    eye.y = beetlePos.y - BEETLE_CAM_HEIGHT + sinP * BEETLE_CAM_DISTANCE;
    eye.z = beetlePos.z - cosY * cosP * BEETLE_CAM_DISTANCE;

    Vec3f at = beetlePos;

    // MM's real function (was OoT-named Play_CameraSetAtEye → routed to a no-op stub, so the beetle
    // subcam never moved: it froze at its init pose and never tracked the flying beetle). Skijer's NEI
    Play_SetCameraAtEye(play, beetleSubCamId, &at, &eye);
}

static void Beetle_Stop(Player* p, PlayState* play) {
    if (beetleFirstPerson) {
        p->unk_AA5 = PLAYER_UNKAA5_0; // exit MM's aim camera
        p->stateFlags1 &= ~PLAYER_STATE1_8; // clear ITEM_IN_HAND
        beetleFirstPerson = 0;
    }
    Beetle_DestroySubCam(play);
    p->stateFlags1 &= ~PLAYER_STATE1_BOOMERANG_THROWN;
    p->stateFlags2 &= ~PLAYER_STATE2_DISABLE_ROTATION_Z_TARGET;
    beetleCollider.base.atFlags &= ~(AT_ON | AT_HIT);
    beetleActive = 0;
    beetleState = BEETLE_STATE_IDLE;
    beetleGrabbed = NULL;
    sBeetleTarget = NULL; // drop lock + reticle
    sBeetleKamikaze = 0;
    sBeetleAutonomous = 0;
    p->focusActor = NULL;
    // Stop looping fly sound
    Audio_StopSfxById(BEETLE_SFX_FLY);
    ItemEquip_PlayUnequipSFX(play, p);
}

static void Beetle_Start(Player* p, PlayState* play) {
    if (beetleActive)
        return;
    beetleActive = 1;
    beetleState = BEETLE_STATE_AIMING;
    beetleFirstPerson = 1;
    beetleGrabbed = NULL;
    beetleWingScale = BEETLE_WING_SCALE_MAX;
    beetleWingDir = -1;
    beetleTimer = BEETLE_MAX_TIME;

    LinkAnimation_PlayLoop(play, &p->skelAnimeUpper, &gPlayerAnim_link_boom_throw_waitR);

    p->stateFlags1 |= PLAYER_STATE1_8; // ITEM_IN_HAND + unk_ACC → real aim cam on entry (see StateAiming)
    p->unk_ACC = 0xA;
    func_80831010(p, play);
    ItemEquip_PlayEquipSFX(play, p);
}

static void Beetle_Launch(Player* p, PlayState* play) {
    beetleState = BEETLE_STATE_FLYING;
    beetleTimer = BEETLE_MAX_TIME;
    beetleStartPos = p->actor.world.pos;
    sBeetleTarget = NULL; // fresh flight: no lock yet
    sBeetleKamikaze = 0;
    sBeetleAutonomous = 0;

    s16 launchYaw = FirstPerson_GetAimYaw(p);
    s16 launchPitch = FirstPerson_GetAimPitch(p);

    beetlePos.x = p->actor.world.pos.x + Math_SinS(launchYaw) * BEETLE_LAUNCH_OFFSET_XZ;
    beetlePos.y = p->actor.world.pos.y + BEETLE_LAUNCH_OFFSET_Y;
    beetlePos.z = p->actor.world.pos.z + Math_CosS(launchYaw) * BEETLE_LAUNCH_OFFSET_XZ;

    beetleRot.x = launchPitch;
    beetleRot.y = launchYaw;
    beetleRot.z = 0;

    LinkAnimation_PlayOnce(play, &p->skelAnimeUpper, &gPlayerAnim_link_boom_throwR);

    p->unk_AA5 = PLAYER_UNKAA5_0; // exit MM's aim camera
    p->stateFlags1 &= ~PLAYER_STATE1_8; // clear ITEM_IN_HAND — beetle is now flying, not aiming
    beetleFirstPerson = 0;
    Beetle_CreateSubCam(play);

    Beetle_PlaySound(&p->actor.world.pos, BEETLE_SFX_LAUNCH);
    ItemVoice_Play(p, NA_SE_VO_LI_SWORD_N, NA_SE_VO_LI_SWORD_N_KID);
}

static void Beetle_StartReturn(Player* p, PlayState* play) {
    beetleState = BEETLE_STATE_RETURNING;
    sBeetleTarget = NULL; // clear the Z-lock / kamikaze so the next flight starts fresh
    sBeetleKamikaze = 0;
    sBeetleAutonomous = 0;
    p->focusActor = NULL; // drop the lock-on reticle
    Beetle_DestroySubCam(play);
    p->stateFlags1 &= ~PLAYER_STATE1_BOOMERANG_THROWN;
    p->stateFlags2 &= ~PLAYER_STATE2_DISABLE_ROTATION_Z_TARGET;
    Beetle_PlaySound(&beetlePos, BEETLE_SFX_RETURN);
}

static void Beetle_Catch(Player* p, PlayState* play) {
    LinkAnimation_PlayOnce(play, &p->skelAnimeUpper, &gPlayerAnim_link_boom_catch);

    Beetle_PlaySound(&p->actor.world.pos, BEETLE_SFX_CATCH);
    ItemVoice_Play(p, NA_SE_VO_LI_SWORD_N, NA_SE_VO_LI_SWORD_N_KID);

    Beetle_DropGrabbedActor(p);

    beetleActive = 0;
    beetleState = BEETLE_STATE_IDLE;
}

static void Beetle_Move(f32 speed) {
    f32 cosP = Math_CosS(beetleRot.x);
    f32 sinP = Math_SinS(beetleRot.x);
    f32 sinY = Math_SinS(beetleRot.y);
    f32 cosY = Math_CosS(beetleRot.y);

    beetlePos.x += sinY * cosP * speed;
    beetlePos.y -= sinP * speed;
    beetlePos.z += cosY * cosP * speed;
}

static u8 Beetle_CheckActorCollision(Player* p, PlayState* play) {
    u8 shouldReturn = 0;

    // AT hits — attackable colliders (enemies, En_Item00, the fairy BUBBLE which has an AC that the
    // beetle's AT pops, etc.).
    if (beetleCollider.base.atFlags & AT_HIT) {
        Actor* hitActor = beetleCollider.base.at;
        if (hitActor != NULL) {
            if (hitActor->id == ACTOR_EN_ITEM00 || hitActor->id == ACTOR_EN_SI ||
                hitActor->id == ACTOR_EN_ELFORG || // stray fairy — grab it, carry to Link
                (hitActor->id == ACTOR_EN_G_SWITCH && (hitActor->params & 0x0F) == ENGSWITCH_SILVER_RUPEE)) {
                beetleGrabbed = hitActor;
                if (hitActor->id == ACTOR_EN_SI || hitActor->id == ACTOR_EN_G_SWITCH ||
                    hitActor->id == ACTOR_EN_ELFORG) {
                    hitActor->flags |= ACTOR_FLAG_HOOKSHOT_ATTACHED; // stop its own homing while carried
                }
            } else if (hitActor->id == ACTOR_EN_ELFBUB) {
                // Fairy bubble: the beetle's AT already popped it (its AC took an AC_HIT →
                // EnElfbub_Pop releases the En_Elforg inside). DON'T return — the beetle flies on and
                // grabs the freed fairy on the next contact. Skijer's NEI
                Beetle_PlaySound(&beetlePos, BEETLE_SFX_HIT);
            } else {
                Beetle_PlaySound(&beetlePos, BEETLE_SFX_HIT);
                shouldReturn = 1;
            }
        }
        beetleCollider.base.atFlags &= ~AT_HIT;
    }

    // OC hits — the stray fairy En_Elforg has ONLY an OC collider (no AC), so the AT pass above misses
    // it. Grab it via the beetle's OC contact. Skijer's NEI
    if ((beetleCollider.base.ocFlags1 & OC1_HIT) && (beetleGrabbed == NULL) &&
        (beetleCollider.base.oc != NULL) && (beetleCollider.base.oc->id == ACTOR_EN_ELFORG)) {
        beetleGrabbed = beetleCollider.base.oc;
        beetleGrabbed->flags |= ACTOR_FLAG_HOOKSHOT_ATTACHED;
    }
    beetleCollider.base.ocFlags1 &= ~OC1_HIT;

    return shouldReturn;
}

static u8 Beetle_CheckGeometryCollision(PlayState* play) {
    Vec3f hitPoint;
    CollisionPoly* hitPoly = NULL;
    s32 hitDynaId = 0;

    f32 cosP = Math_CosS(beetleRot.x);
    f32 sinP = Math_SinS(beetleRot.x);
    f32 sinY = Math_SinS(beetleRot.y);
    f32 cosY = Math_CosS(beetleRot.y);

    Vec3f prevPos = beetlePos;
    prevPos.x -= sinY * cosP * BEETLE_SPEED;
    prevPos.y += sinP * BEETLE_SPEED;
    prevPos.z -= cosY * cosP * BEETLE_SPEED;

    if (BgCheck_EntityLineTest1(&play->colCtx, &prevPos, &beetlePos, &hitPoint, &hitPoly, true, true, true, true,
                                &hitDynaId)) {
        beetlePos = hitPoint;
        Beetle_PlaySound(&beetlePos, BEETLE_SFX_HIT);
        return 1;
    }
    return 0;
}

static void Beetle_UpdateGrabbedActor(void) {
    if (beetleGrabbed == NULL)
        return;
    if (beetleGrabbed->update == NULL) {
        beetleGrabbed = NULL;
        return;
    }
    Math_Vec3f_Copy(&beetleGrabbed->world.pos, &beetlePos);
}

static void Beetle_DropGrabbedActor(Player* p) {
    if (beetleGrabbed == NULL)
        return;

    Math_Vec3f_Copy(&beetleGrabbed->world.pos, &p->actor.world.pos);
    if (beetleGrabbed->id == ACTOR_EN_ITEM00) {
        beetleGrabbed->gravity = -0.9f;
        beetleGrabbed->bgCheckFlags &= ~0x03;
    } else if (beetleGrabbed->id == ACTOR_EN_SI) {
        beetleGrabbed->flags &= ~ACTOR_FLAG_HOOKSHOT_ATTACHED;
    } else if (beetleGrabbed->id == ACTOR_EN_ELFORG) {
        // Stray fairy dropped at Link: clear the attach flag so it resumes homing/collection. NEI
        beetleGrabbed->flags &= ~ACTOR_FLAG_HOOKSHOT_ATTACHED;
    } else if (beetleGrabbed->id == ACTOR_EN_G_SWITCH) {
        // Silver rupee - drop near Link so it can be collected
        beetleGrabbed->flags &= ~ACTOR_FLAG_HOOKSHOT_ATTACHED;
        beetleGrabbed->gravity = -2.0f;
        beetleGrabbed->bgCheckFlags &= ~0x03;
    }
    beetleGrabbed = NULL;
}

static void Beetle_StateAiming(Player* p, PlayState* play, ItemInputState* in) {
    // Animation update is handled by Player_UpperAction_Beetle

    // The real vanilla-bow aim state needs THREE things each frame (the missing 3rd — unk_ACC — is
    // why it flipped in/out): PLAYER_STATE1_8 (ITEM_IN_HAND) so func_8083868C picks the aim cam;
    // unk_ACC != 0 so func_800B7128 stays true and Player_Action_43's exit test (z_player.c:16916)
    // doesn't eject the aim (the beetle's no-op upper action never keeps unk_ACC alive like the bow's
    // does); and func_80831010 for unk_AA5 = 3. NOT PLAYER_STATE1_FIRST_PERSON (that oscillated). NEI
    if (beetleFirstPerson) {
        p->stateFlags1 |= PLAYER_STATE1_8;
        p->unk_ACC = 0xA;
        func_80831010(p, play);
    } else {
        p->stateFlags1 &= ~PLAYER_STATE1_8;
    }

    if (CHECK_BTN_ALL(play->state.input[0].press.button, BTN_CUP)) {
        if (beetleFirstPerson) {
            p->unk_AA5 = PLAYER_UNKAA5_0; // exit MM's aim camera
            beetleFirstPerson = 0;
        } else {
            func_80831010(p, play); // enter/hold MM's real aim camera (was FirstPerson_Init, which never aimed)
            beetleFirstPerson = 1;
        }
        ItemEquip_PlayEquipSFX(play, p);
        return;
    }

    u8 isZTargeting = Player_IsZTargeting(p);
    if (beetleFirstPerson && isZTargeting) {
        p->unk_AA5 = PLAYER_UNKAA5_0; // exit MM's aim camera
        beetleFirstPerson = 0;
    } else if (!beetleFirstPerson && !isZTargeting) {
        func_80831010(p, play); // enter/hold MM's real aim camera (was FirstPerson_Init, which never aimed)
        beetleFirstPerson = 1;
    }

    if (!in->isHeld && !in->isPressed) {
        Beetle_Launch(p, play);
    }
}

// Only actors the beetle can actually DAMAGE (attention-enabled enemies/bosses) or COLLECT/interact
// with (stray fairy + its bubble, rupees/items, silver-rupee switch) are valid targets — no random
// props. Skijer's NEI
static u8 Beetle_IsTargetable(Actor* a) {
    if ((a->category == ACTORCAT_ENEMY) || (a->category == ACTORCAT_BOSS)) {
        return (a->flags & ACTOR_FLAG_ATTENTION_ENABLED) ? 1 : 0;
    }
    return (a->id == ACTOR_EN_ELFORG || a->id == ACTOR_EN_ELFBUB || a->id == ACTOR_EN_ITEM00 ||
            a->id == ACTOR_EN_SI ||
            (a->id == ACTOR_EN_G_SWITCH && (a->params & 0x0F) == ENGSWITCH_SILVER_RUPEE))
               ? 1
               : 0;
}

// Nearest targetable actor within range AND roughly in front of the beetle — the Z-target candidate.
static Actor* Beetle_FindTarget(PlayState* play) {
    static const u8 sCats[] = { ACTORCAT_ENEMY, ACTORCAT_BOSS, ACTORCAT_ITEMACTION, ACTORCAT_PROP };
    Actor* best = NULL;
    f32 bestDist = BEETLE_TARGET_RANGE;
    s32 c;

    for (c = 0; c < (s32)ARRAY_COUNT(sCats); c++) {
        Actor* actor = play->actorCtx.actorLists[sCats[c]].first;
        while (actor != NULL) {
            if ((actor->update != NULL) && Beetle_IsTargetable(actor)) {
                f32 dist = Math_Vec3f_DistXYZ(&beetlePos, &actor->world.pos);
                if (dist < bestDist) {
                    // Prefer what's in front of the beetle (~78° cone).
                    s16 angleDiff = Math_Vec3f_Yaw(&beetlePos, &actor->world.pos) - beetleRot.y;
                    if (ABS_ALT(angleDiff) < 0x3800) {
                        bestDist = dist;
                        best = actor;
                    }
                }
            }
            actor = actor->next;
        }
    }
    return best;
}

// Reliable proximity grab for the stray fairy (En_Elforg) — its OC collider is finicky against the
// beetle's, so just grab any freed fairy within reach and carry it to Link. Skijer's NEI
static void Beetle_ProximityGrabFairy(PlayState* play) {
    Actor* actor;

    if (beetleGrabbed != NULL) {
        return;
    }
    actor = play->actorCtx.actorLists[ACTORCAT_ITEMACTION].first;
    while (actor != NULL) {
        if ((actor->update != NULL) && (actor->id == ACTOR_EN_ELFORG) &&
            (Math_Vec3f_DistXYZ(&beetlePos, &actor->world.pos) < BEETLE_GRAB_RADIUS * 2.0f)) {
            beetleGrabbed = actor;
            actor->flags |= ACTOR_FLAG_HOOKSHOT_ATTACHED;
            return;
        }
        actor = actor->next;
    }
}

// Turn beetleRot toward the locked target by up to `step` binang/frame (homing). Skijer's NEI
static void Beetle_HomeToTarget(s16 step) {
    Vec3f tgt = sBeetleTarget->world.pos;
    s16 wantYaw = Math_Vec3f_Yaw(&beetlePos, &tgt);
    s16 wantPitch = Math_Vec3f_Pitch(&beetlePos, &tgt);
    Math_SmoothStepToS(&beetleRot.y, wantYaw, 4, step, 0x10);
    Math_SmoothStepToS(&beetleRot.x, wantPitch, 4, step, 0x10);
}

static void Beetle_StateFlying(Player* p, PlayState* play) {
    Input* input = &play->state.input[0];
    u8 aHeld;

    // Drop a lock whose enemy died/despawned.
    if ((sBeetleTarget != NULL) && (sBeetleTarget->update == NULL)) {
        sBeetleTarget = NULL;
        sBeetleKamikaze = 0;
    }

    // ── AUTONOMOUS mode (B was pressed) ───────────────────────────────────────────────────────────
    // Camera + control are back with Link (subcam gone, no player freeze). The beetle flies on its
    // OWN — no stick input. If it has a locked enemy it homes in to hit it, otherwise it heads home.
    // A hit / max distance / timeout returns it to Link. Skijer's NEI
    if (sBeetleAutonomous) {
        u8 hit = Beetle_CheckActorCollision(p, play);
        Beetle_ProximityGrabFairy(play);

        if ((sBeetleTarget != NULL) && (sBeetleTarget->update != NULL)) {
            Beetle_HomeToTarget(BEETLE_KAMIKAZE_STEP);
        } else {
            Beetle_StartReturn(p, play); // nothing to chase → fly back to Link
            return;
        }

        Beetle_Move(BEETLE_SPEED * BEETLE_BOOST_MULT);
        Beetle_UpdateCollider(play, &beetlePos);
        Beetle_UpdateWingAnimation(&beetleWingScale, &beetleWingDir);
        Beetle_UpdateGrabbedActor(); // NOTE: no Beetle_UpdateSubCam — the camera is Link's now

        if (hit || Beetle_CheckGeometryCollision(play)) {
            Beetle_StartReturn(p, play);
            return;
        }
        if ((Math_Vec3f_DistXYZ(&beetlePos, &beetleStartPos) > BEETLE_MAX_DISTANCE) || (DECR(beetleTimer) == 0)) {
            Beetle_StartReturn(p, play);
            return;
        }
        Beetle_PlayLoopSound(&p->actor, BEETLE_SFX_FLY);
        return;
    }

    // ── PILOTED mode (beetle subcam, Link frozen, you steer) ──────────────────────────────────────
    aHeld = CHECK_BTN_ALL(input->cur.button, BTN_A);

    // Animation update is handled by Player_UpperAction_Beetle
    Player_ZeroSpeedXZ(p);
    p->stateFlags1 |= PLAYER_STATE1_BOOMERANG_THROWN;
    p->stateFlags2 |= PLAYER_STATE2_DISABLE_ROTATION_Z_TARGET;
    // Point boomerangActor at Link himself so the FOLLOWBOOMERANG camera path
    // (z_player.c:12350) never propagates a stale En_Boom pointer through
    // Camera_SetParam — Link's actor is always valid.
    p->zoraBoomerangActor = &p->actor;

    // Z: toggle the enemy lock (locks the nearest attention-enabled enemy, like Link's Z-target).
    if (CHECK_BTN_ALL(input->press.button, BTN_Z)) {
        if (sBeetleTarget != NULL) {
            sBeetleTarget = NULL; // Z again = untarget
            sBeetleKamikaze = 0;
        } else {
            sBeetleTarget = Beetle_FindTarget(play);
        }
    }

    // B: hand the camera + control back to Link and let the beetle fly on its OWN. If a target is
    // locked it kamikaze-homes to hit it (then returns); otherwise it flies straight back to Link.
    if (CHECK_BTN_ALL(input->press.button, BTN_B)) {
        sBeetleAutonomous = 1;
        Beetle_DestroySubCam(play);                           // camera back to Link
        p->stateFlags1 &= ~PLAYER_STATE1_BOOMERANG_THROWN;    // control back to Link
        p->stateFlags2 &= ~PLAYER_STATE2_DISABLE_ROTATION_Z_TARGET;
        p->focusActor = NULL;                                 // drop the lock reticle
        if (sBeetleTarget != NULL) {
            sBeetleKamikaze = 1;
        }
        return; // the autonomous branch takes over next frame
    }

    // Track the nearest lockable actor each frame so the reticle can show as an "offer" even before
    // you press Z. The actual player->focusActor is set LATE (Beetle_LateReticle via CustomItems_
    // LatePose) — setting it here gets clobbered by Player_UpdateCommon's own Z-target logic. NEI
    sBeetleCandidate = Beetle_FindTarget(play);

    u8 hitActor = Beetle_CheckActorCollision(p, play);
    Beetle_ProximityGrabFairy(play); // grab any freed stray fairy in reach → carried back to Link

    if (sBeetleKamikaze && (sBeetleTarget != NULL)) {
        // Locked-in: home hard at the target, ignore the stick.
        Beetle_HomeToTarget(BEETLE_KAMIKAZE_STEP);
    } else {
        // Normal stick steering; while locked, a gentle homing pull keeps the enemy in your path but
        // you stay in control. Holding A while locked redirects harder toward it (the "Z+A" redirect).
        Projectile_UpdateRotationFromStick(&beetleRot.y, &beetleRot.x, play, BEETLE_TURN_SPEED, BEETLE_PITCH_MAX);
        if (sBeetleTarget != NULL) {
            Beetle_HomeToTarget(aHeld ? BEETLE_TARGET_HOMING_STEP_A : BEETLE_TARGET_HOMING_STEP);
        }
    }

    // A = fly faster; kamikaze always boosts to close the gap.
    Beetle_Move((aHeld || sBeetleKamikaze) ? (BEETLE_SPEED * BEETLE_BOOST_MULT) : BEETLE_SPEED);
    Beetle_UpdateCollider(play, &beetlePos);
    Beetle_UpdateWingAnimation(&beetleWingScale, &beetleWingDir);
    Beetle_UpdateSubCam(play);
    Beetle_UpdateGrabbedActor();

    if (hitActor || Beetle_CheckGeometryCollision(play)) {
        Beetle_StartReturn(p, play);
        return;
    }

    f32 distFromStart = Math_Vec3f_DistXYZ(&beetlePos, &beetleStartPos);
    if (distFromStart > BEETLE_MAX_DISTANCE || DECR(beetleTimer) == 0) {
        Beetle_StartReturn(p, play);
        return;
    }

    Beetle_PlayLoopSound(&p->actor, BEETLE_SFX_FLY);
}

static void Beetle_StateReturning(Player* p, PlayState* play) {
    Vec3f targetPos = p->actor.world.pos;
    targetPos.y += BEETLE_LAUNCH_OFFSET_Y;

    f32 distToLink = Math_Vec3f_DistXYZ(&beetlePos, &targetPos);

    Beetle_UpdateWingAnimation(&beetleWingScale, &beetleWingDir);
    Beetle_UpdateGrabbedActor();

    if (distToLink > BEETLE_CATCH_DISTANCE) {
        f32 dx = targetPos.x - beetlePos.x;
        f32 dy = targetPos.y - beetlePos.y;
        f32 dz = targetPos.z - beetlePos.z;

        if (distToLink > 0.1f) {
            f32 invNorm = BEETLE_RETURN_SPEED / distToLink;
            beetlePos.x += dx * invNorm;
            beetlePos.y += dy * invNorm;
            beetlePos.z += dz * invNorm;
        }

        beetleRot.y = Math_Vec3f_Yaw(&beetlePos, &targetPos);
        beetleRot.x = Math_Vec3f_Pitch(&beetlePos, &targetPos);
        Beetle_PlayLoopSound(&p->actor, BEETLE_SFX_FLY);
    } else {
        Beetle_Catch(p, play);
    }
}

void Handle_Beetle(Player* p, PlayState* play) {
    if (!sBeetleColInitialized)
        Beetle_InitCollider(play, p);

    ItemInputState in;
    ItemInput_Update(&in, ITEM_BEETLE, p, play);

    if (!in.wasEquipped) {
        if (beetleActive)
            Beetle_Stop(p, play);
        return;
    }

    if (beetleState != BEETLE_STATE_FLYING && beetleState != BEETLE_STATE_RETURNING) {
        if (ItemInput_IsBlocked(p, play)) {
            if (beetleActive)
                Beetle_Stop(p, play);
            return;
        }
    }

    if (ItemInput_CheckDamage(p, &sBeetlePrevInvinc)) {
        if (beetleState == BEETLE_STATE_FLYING) {
            Beetle_StartReturn(p, play);
        } else if (beetleActive) {
            Beetle_Stop(p, play);
        }
        return;
    }

    if (!beetleActive) {
        if (in.isPressed)
            Beetle_Start(p, play);
        return;
    }

    if (beetleState == BEETLE_STATE_AIMING && CHECK_BTN_ALL(play->state.input[0].press.button, BTN_B)) {
        Beetle_Stop(p, play);
        return;
    }

    switch (beetleState) {
        case BEETLE_STATE_AIMING:
            Beetle_StateAiming(p, play, &in);
            break;
        case BEETLE_STATE_FLYING:
            Beetle_StateFlying(p, play);
            break;
        case BEETLE_STATE_RETURNING:
            Beetle_StateReturning(p, play);
            break;
        default:
            beetleState = BEETLE_STATE_IDLE;
            beetleActive = 0;
            break;
    }
}

// Runs LATE — from CustomItems_LatePose, AFTER Player_UpdateCommon — so the focusActor we set for
// MM's lock-on reticle survives the player's own Z-target logic (which would otherwise clobber it).
// Attention_Update (z_actor.c:3117) then reads player->focusActor (gated by zTargetActiveTimer >= 5)
// and draws the spinning reticle. This is the MANUAL lock: only shown when you've pressed Z
// (sBeetleTarget set). The automatic "offer" arrow is separate (Beetle_DrawOffer, draw phase). NEI
void Beetle_LateReticle(Player* p, PlayState* play) {
    if (beetleActive && (beetleState == BEETLE_STATE_FLYING) && !sBeetleAutonomous && (sBeetleTarget != NULL) &&
        (sBeetleTarget->update != NULL)) {
        p->focusActor = sBeetleTarget;
        p->zTargetActiveTimer = 15;
    }
}

// Runs in the DRAW phase (from CustomItems_OverrideDraw, which draws before Attention_Draw in
// z_parameter.c). Points MM's automatic "offer" arrow (the single, non-circular gLockOnArrowDL, NOT
// the spinning lock reticle) at the nearest candidate while flying and NOT locked — so you see which
// actor Z would grab, without an auto-lock. Attention_Update recomputes arrowHoverActor each update
// from Link's position, so we override it here right before it's drawn. Skijer's NEI
void Beetle_DrawOffer(Player* p, PlayState* play) {
    if (beetleActive && (beetleState == BEETLE_STATE_FLYING) && !sBeetleAutonomous && (sBeetleTarget == NULL) &&
        (sBeetleCandidate != NULL) && (sBeetleCandidate->update != NULL)) {
        play->actorCtx.attention.arrowHoverActor = sBeetleCandidate;
    }
}

s32 Player_UpperAction_Beetle(Player* this, PlayState* play) {
    // Return busy if beetle is active - this makes the upper body use skelAnimeUpper
    if (beetleActive) {
        LinkAnimation_Update(play, &this->skelAnimeUpper);
        return 1;
    }
    return 0;
}

void Player_InitBeetleIA(PlayState* play, Player* this) {
    Beetle_InitCollider(play, this);
    beetleActive = 0;
    beetleState = BEETLE_STATE_IDLE;
    beetleFirstPerson = 0;
    beetleGrabbed = NULL;
    beetleWingScale = BEETLE_WING_SCALE_MAX;
    beetleWingDir = -1;
    beetleTimer = 0;
    beetleSubCamId = SUBCAM_FREE;
    this->stateFlags1 |= PLAYER_STATE1_ITEM_IN_HAND;
}
