#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "overlays/actors/ovl_En_Pametfrog/z_en_pametfrog.h"
void EnPametfrog_PlaceSnapper(EnPametfrog* enPametfrog, PlayState* play);
void EnPametfrog_SnapperSpawn(EnPametfrog* enPametfrog, PlayState* play);
void EnPametfrog_PlayCutscene(EnPametfrog* enPametfrog, PlayState* play);
void EnPametfrog_SetupFallInAir(EnPametfrog* enPametfrog, PlayState* play);
void EnPametfrog_FallOnGround(EnPametfrog* enPametfrog, PlayState* play);
void EnPametfrog_SetupSpawnFrog(EnPametfrog* enPametfrog, PlayState* play);
void EnPametfrog_SetupTransitionGekkoSnapper(EnPametfrog* enPametfrog, PlayState* play);
void EnPametfrog_SetupRunToSnapper(EnPametfrog* enPametfrog);
void EnPametfrog_SetupJumpToWall(EnPametfrog* enPametfrog);
void EnPametfrog_SpawnFrog(EnPametfrog* enPametfrog, PlayState* play);
}

#define CVAR_NAME "gEnhancements.Cutscenes.SkipEnemyCutscenes"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void EnPametfrog_DefeatSnapper_WithoutCamera(EnPametfrog* enPametfrog, PlayState* play) {
    enPametfrog->timer--;
    Actor_SetScale(&enPametfrog->actor, enPametfrog->timer * 0.00035000002f);
    enPametfrog->actor.colorFilterTimer = 16;
    if (enPametfrog->timer == 0) {
        EnPametfrog_SetupSpawnFrog(enPametfrog, play);
    }
}

void EnPametfrog_DefeatGekko_WithoutCamera(EnPametfrog* enPametfrog, PlayState* play) {
    enPametfrog->actor.colorFilterTimer = 16;
    if (enPametfrog->timer > 0) {
        enPametfrog->timer--;
        if (enPametfrog->timer == 0) {
            // Behaves like EnPametFrog_SetupDefeatSnapper but without camera
            enPametfrog->timer = 20;
            enPametfrog->actionFunc = EnPametfrog_DefeatSnapper_WithoutCamera;
        }
    }
}

void EnPametfrog_SnapperSpawn_WithoutCamera(EnPametfrog* enPametfrog, PlayState* play) {
    enPametfrog->timer--;
    if (enPametfrog->timer != 0) {
        Rumble_Request(enPametfrog->actor.xyzDistToPlayerSq, 120, 20, 10);
    } else {
        EnPametfrog_SetupTransitionGekkoSnapper(enPametfrog, play);
    }
}

void EnPametfrog_CallSnapper_WithoutCamera(EnPametfrog* enPametfrog, PlayState* play) {
    if (SkelAnime_Update(&enPametfrog->skelAnime)) {
        EnPametfrog_PlaceSnapper(enPametfrog, gPlayState);
        enPametfrog->timer = 10; // Normally 40, but sped up
        enPametfrog->actionFunc = EnPametfrog_SnapperSpawn_WithoutCamera;
    }
}

void EnPametfrog_FallOnGround_WithoutCamera(EnPametfrog* enPametfrog, PlayState* play) {
    if (SkelAnime_Update(&enPametfrog->skelAnime)) {
        /*
         * The actor normally checks for equivalence to &gGekkoFallOnGroundAnim, but this never seemed to be true here.
         * I don't know why. Instead, check for not playing the recovery animation that comes after.
         */
        if (enPametfrog->skelAnime.animation != &gGekkoRecoverAnim) {
            if (enPametfrog->actor.colChkInfo.health == 0) {
                enPametfrog->timer--;
                if (enPametfrog->timer == 0) {
                    // Behaves like EnPametfrog_SetupDefeatGekko, but without camera
                    enPametfrog->actor.params = GEKKO_DEFEAT;
                    enPametfrog->timer = 38;
                    enPametfrog->actionFunc = EnPametfrog_DefeatGekko_WithoutCamera;
                }
            } else {
                Animation_PlayOnce(&enPametfrog->skelAnime, (AnimationHeader*)&gGekkoRecoverAnim);
            }
        } else {
            EnPametfrog_SetupRunToSnapper(enPametfrog);
        }
    }
}

void EnPametfrog_FallOffSnapper_WithoutCamera(EnPametfrog* enPametfrog, PlayState* play) {
    SkelAnime_Update(&enPametfrog->skelAnime);
    enPametfrog->actor.shape.rot.x += 0x800;
    enPametfrog->actor.shape.rot.z += 0x1000;
    if (enPametfrog->timer != 0) {
        enPametfrog->timer--;
    }

    if (enPametfrog->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
        EnPametfrog_SetupJumpToWall(enPametfrog);
    }
}
void EnPametfrog_PlayCutscene_WithoutCamera(EnPametfrog* enPametfrog, PlayState* play) {
    if (enPametfrog->actor.colChkInfo.health == 0) {          // Death cutscene
        if (enPametfrog->actor.params == GEKKO_PRE_SNAPPER) { // First phase
            // Acts like EnPametfrog_SetupCallSnapper, but without camera
            s16 yawDiff;
            Animation_MorphToPlayOnce(&enPametfrog->skelAnime, (AnimationHeader*)&gGekkoCallAnim, 3.0f);
            Actor_PlaySfx(&enPametfrog->actor, NA_SE_EN_FROG_GREET);
            enPametfrog->actor.flags &= ~ACTOR_FLAG_TARGETABLE;
            enPametfrog->actor.colChkInfo.health = 6;
            enPametfrog->actor.world.rot.y =
                Actor_WorldYawTowardPoint(&enPametfrog->actor, &enPametfrog->actor.home.pos);
            yawDiff = enPametfrog->actor.yawTowardsPlayer - enPametfrog->actor.world.rot.y;
            if (yawDiff > 0) {
                enPametfrog->actor.world.rot.y -= 0x2000;
            } else {
                enPametfrog->actor.world.rot.y += 0x2000;
            }
            enPametfrog->actor.shape.rot.y = enPametfrog->actor.world.rot.y;
            enPametfrog->timer = 0;
            enPametfrog->actor.hintId = TATL_HINT_ID_GEKKO_GIANT_SLIME;
            enPametfrog->actionFunc = EnPametfrog_CallSnapper_WithoutCamera;
        } else { // Second phase
            EnPametfrog_SetupFallInAir(enPametfrog, gPlayState);
        }
    } else { // Knocked off of Snapper
        // Acts like EnPametfrog_SetupFallOffSnapper, but without camera
        enPametfrog->subCamId = SUB_CAM_ID_DONE;
        Animation_PlayOnce(&enPametfrog->skelAnime, (AnimationHeader*)&gGekkoFallInAirAnim);
        enPametfrog->actor.params = GEKKO_FALL_OFF_SNAPPER;
        enPametfrog->actor.speed = 7.0f;
        enPametfrog->actor.velocity.y = 15.0f;
        enPametfrog->actor.world.rot.y = BINANG_ROT180(enPametfrog->actor.child->world.rot.y);
        enPametfrog->actor.shape.rot.y = enPametfrog->actor.world.rot.y;
        enPametfrog->actor.flags |= ACTOR_FLAG_TARGETABLE;
        enPametfrog->timer = 30;
        enPametfrog->collider.base.ocFlags1 |= OC1_ON;
        Actor_PlaySfx(&enPametfrog->actor, NA_SE_EN_FROG_DAMAGE);
        enPametfrog->actionFunc = EnPametfrog_FallOffSnapper_WithoutCamera;
    }
}

void EnPametfrog_SpawnFrog_WithoutCamera(EnPametfrog* enPametfrog, PlayState* play) {
    enPametfrog->timer--;
    if (enPametfrog->timer == 0) {
        Actor_Kill(&enPametfrog->actor);
    }
}

void RegisterSkipGekkoSnapperCutscenes() {
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_EN_PAMETFROG, CVAR, [](Actor* actor, bool* should) {
        EnPametfrog* enPametfrog = (EnPametfrog*)actor;
        if (actor->params == GEKKO_CUTSCENE) {
            /*
             * Gekko has been knocked off of Snapper. EnPametfrog_Update will always try to enforce the cutscene lock if
             * params is GEKKO_CUTSCENE, so we need to change the params to some other value to prevent that. The params
             * will be overwritten to the correct value by EnPametfrog_PlayCutscene_WithoutCamera.
             */
            actor->params = GEKKO_FALL_OFF_SNAPPER;
            enPametfrog->actionFunc = EnPametfrog_PlayCutscene_WithoutCamera;
        }

        // Replace camera-controlling action funcs with custom variants
        if (enPametfrog->actionFunc == EnPametfrog_PlayCutscene) {
            enPametfrog->actionFunc = EnPametfrog_PlayCutscene_WithoutCamera;
        } else if (enPametfrog->actionFunc == EnPametfrog_FallOnGround) {
            enPametfrog->actionFunc = EnPametfrog_FallOnGround_WithoutCamera;
        } else if (enPametfrog->actionFunc == EnPametfrog_SpawnFrog) {
            enPametfrog->actionFunc = EnPametfrog_SpawnFrog_WithoutCamera;
        }
    });
}

static RegisterShipInitFunc enemyCsInitFunc(RegisterSkipGekkoSnapperCutscenes, { CVAR_NAME });
