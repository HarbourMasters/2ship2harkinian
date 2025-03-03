#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "variables.h"
#include "functions.h"
#include "overlays/actors/ovl_Boss_06/z_boss_06.h"
#include "overlays/actors/ovl_En_Bigpo/z_en_bigpo.h"
#include "overlays/actors/ovl_En_Bigslime/z_en_bigslime.h"
#include "overlays/actors/ovl_En_Death/z_en_death.h"
#include "overlays/actors/ovl_En_Jso2/z_en_jso2.h"
#include "overlays/actors/ovl_En_Kaizoku/z_en_kaizoku.h"
#include "overlays/actors/ovl_En_Knight/z_en_knight.h"
#include "overlays/actors/ovl_En_Wiz/z_en_wiz.h"
void EnDeath_DeathCutscenePart1(EnDeath* enDeath, PlayState* play);
void EnDeath_SetupDeathCutscenePart2(EnDeath* enDeath, PlayState* play);
void func_809F2EE8(Boss06* boss06, PlayState* play);
void func_809B3E9C(EnKnight* enKnight, PlayState* play);
void EnWiz_SetupDisappear(EnWiz* enWiz);
void EnBigpo_SetupIdleFlying(EnBigpo* enBigpo);
void EnBigpo_DrawMainBigpo(Actor* actor, PlayState* play);
void EnBigslime_CallMinislime(EnBigslime* enBigslime, PlayState* play);
void EnBigslime_GekkoSfxOutsideBigslime(EnBigslime* enBigslime, u16 sfxId);
void func_80B872A4(EnKaizoku* enKaizoku);
void EnKaizoku_Draw(Actor* actor, PlayState* play);
}

#define CVAR_NAME "gEnhancements.Cutscenes.SkipEnemyCutscenes"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterSkipEnemyIntros() {
    // Odolwa
    COND_ID_HOOK(ShouldActorInit, ACTOR_BOSS_01, CVAR, [](Actor* actor, bool* should) { SET_EVENTINF(EVENTINF_54); });

    // Goht
    COND_ID_HOOK(ShouldActorInit, ACTOR_BOSS_HAKUGIN, CVAR, [](Actor* actor, bool* should) {
        /*
         * EVENTINF_62 gets set after seeing the boss lair intro cutscene.
         * EVENTINF_53 gets set after melting the ice (not skipped here, as that would skip the ice melting
         * requirement).
         */
        SET_EVENTINF(EVENTINF_62);
    });

    // Gyorg
    COND_ID_HOOK(ShouldActorInit, ACTOR_BOSS_03, CVAR, [](Actor* actor, bool* should) { SET_EVENTINF(EVENTINF_56); });

    // Twinmold
    COND_ID_HOOK(ShouldActorInit, ACTOR_BOSS_02, CVAR, [](Actor* actor, bool* should) { SET_EVENTINF(EVENTINF_55); });

    // Majora
    COND_ID_HOOK(ShouldActorInit, ACTOR_BOSS_07, CVAR,
                 [](Actor* actor, bool* should) { gSaveContext.eventInf[6] |= 2; });

    // Igos du Ikana (and lackeys)
    COND_ID_HOOK(ShouldActorInit, ACTOR_EN_KNIGHT, CVAR, [](Actor* actor, bool* should) {
        // In the credits, this sceneLayer will be 1. Do not set this flag in that case, or things will break.
        if (gSaveContext.sceneLayer == 0) {
            SET_EVENTINF(EVENTINF_57);
        }
    });

    // Gomess
    COND_ID_HOOK(ShouldActorInit, ACTOR_EN_DEATH, CVAR,
                 [](Actor* actor, bool* should) { gSaveContext.eventInf[6] |= 8; });

    // Stone Tower Temple Garo Master
    COND_ID_HOOK(ShouldActorInit, ACTOR_EN_JSO2, CVAR, [](Actor* actor, bool* should) {
        /*
         * Two of the three Garo Masters have no cutscenes, but the one in Stone Tower Temple is special. Its intro and
         * death cutscenes can be skipped by simply changing its type to that of a normal Garo Master.
         */
        actor->params = EN_JSO2_TYPE_NORMAL;
    });

    // Dinolfos
    COND_ID_HOOK(ShouldActorInit, ACTOR_EN_DINOFOS, CVAR, [](Actor* actor, bool* should) {
        /*
         * The Woodfall Temple and Secret Shrine Dinolfos have intro and death cutscenes, while the two Snowhead Temple
         * Dinolfos do not. Simply setting the csId to CS_ID_NONE circumvents the cutscenes entirely, putting the
         * Dinolfos into an attack-ready state.
         */
        if (actor->csId != CS_ID_NONE) {
            actor->csId = CS_ID_NONE;
            Audio_PlayBgm_StorePrevBgm(NA_BGM_MINI_BOSS);
        }
    });

    // Wart
    COND_ID_HOOK(ShouldActorInit, ACTOR_BOSS_04, CVAR, [](Actor* actor, bool* should) { SET_EVENTINF(EVENTINF_60); });

    // Big Poe (Beneath the Well)
    COND_ID_HOOK(OnActorInit, ACTOR_EN_BIGPO, CVAR, [](Actor* actor) {
        // Only do for the well Big Poe. The Dampe one has a brief cutscene where Dampe flees.
        if (actor->params == BIG_POE_TYPE_REGULAR) {
            EnBigpo* enBigpo = (EnBigpo*)actor;
            actor->draw = EnBigpo_DrawMainBigpo;
            enBigpo->mainColor.a = 255;
            EnBigpo_SetupIdleFlying(enBigpo);
        }
    });

    // Gekko + Mad Jelly
    COND_ID_HOOK(OnActorInit, ACTOR_EN_BIGSLIME, CVAR, [](Actor* actor) {
        EnBigslime* enBigslime = (EnBigslime*)actor;
        Animation_MorphToPlayOnce(&enBigslime->skelAnime, (AnimationHeader*)&gGekkoCallAnim, 5.0f);
        EnBigslime_GekkoSfxOutsideBigslime(enBigslime, NA_SE_EN_FROG_GREET);
        enBigslime->callTimer = 0;
        enBigslime->actionFunc = EnBigslime_CallMinislime;
    });

    // Gerudo Pirate
    COND_ID_HOOK(OnActorInit, ACTOR_EN_KAIZOKU, CVAR, [](Actor* actor) {
        EnKaizoku* enKaizoku = (EnKaizoku*)actor;
        enKaizoku->unk_59C = 0;
        enKaizoku->picto.actor.flags &= ~ACTOR_FLAG_100000;
        enKaizoku->picto.actor.flags &= ~ACTOR_FLAG_CANT_LOCK_ON;
        enKaizoku->picto.actor.flags |= ACTOR_FLAG_TARGETABLE;
        enKaizoku->picto.actor.draw = EnKaizoku_Draw;
        enKaizoku->picto.actor.gravity = -2.0f;
        // Swords
        enKaizoku->unk_2F8.y = 1.0f;
        enKaizoku->unk_2F8.z = 1.0f;
        enKaizoku->unk_304.x = 1.0f;
        enKaizoku->unk_304.y = 1.0f;
        enKaizoku->unk_304.z = 1.0f;
        enKaizoku->unk_2D8 = 0; // Flag for updating animation
        Audio_SetMainBgmVolume(0x7F, 0);
        Audio_PlayBgm_StorePrevBgm(NA_BGM_MINI_BOSS);
        func_80B872A4(enKaizoku);
    });

    // Wizrobe
    COND_ID_HOOK(OnActorInit, ACTOR_EN_WIZ, CVAR, [](Actor* actor) {
        EnWiz* enWiz = (EnWiz*)actor;
        enWiz->introCutsceneState = 6; // EN_WIZ_INTRO_CS_DISAPPEAR
        EnWiz_SetupDisappear(enWiz);
        if (enWiz->type != EN_WIZ_TYPE_FIRE_NO_BGM) {
            Audio_PlayBgm_StorePrevBgm(NA_BGM_MINI_BOSS);
        }
    });
}

void RegisterSkipBossWarpCutscenes() {
    COND_VB_SHOULD(VB_START_CUTSCENE, CVAR, {
        s16* csId = va_arg(args, s16*);
        switch (gPlayState->sceneId) {
            case SCENE_MITURIN_BS: // Odolwa's Lair
                if (*csId == 9) {  // Warping out
                    *should = false;
                }
                break;
            case SCENE_MITURIN:    // Woodfall Temple
                if (*csId == 34) { // Cleared vines to room Deku Princess is in
                    *should = false;
                }
                break;
            case SCENE_HAKUGIN_BS: // Goht's Lair
                if (*csId == 10) { // Warping out
                    *should = false;
                }
                break;
            case SCENE_10YUKIYAMANOMURA2: // Mountain Village (Spring)
                if (*csId == 13) {        // Warping from Goht's Lair
                    *should = false;
                }
                break;
            case SCENE_SEA_BS:    // Gyorg's Lair
                if (*csId == 9) { // Warping out
                    *should = false;
                }
                break;
            case SCENE_31MISAKI:   // Zora Cape
                if (*csId == 18) { // Warping from Gyorg's Lair
                    *should = false;
                }
                break;
            case SCENE_INISIE_BS: // Twinmold's Lair
                if (*csId == 9) { // Warping out
                    *should = false;
                }
                break;
            case SCENE_IKANA:      // Ikana Canyon
                if (*csId == 31) { // Warping from Twinmold's Lair
                    *should = false;
                }
                break;
            default:
                break;
        }
    });
}

void RegisterSkipEnemyCutscenes() {
    // Enemy actor cutscene starts
    COND_VB_SHOULD(VB_START_CUTSCENE, CVAR, {
        s16* csId = va_arg(args, s16*);
        Actor* actor = va_arg(args, Actor*);

        if (actor == NULL) {
            return;
        }

        switch (actor->id) {
            case ACTOR_EN_BIGSLIME:
            case ACTOR_EN_DEATH:
            case ACTOR_EN_EGOL:
            case ACTOR_EN_GB2:
            case ACTOR_EN_IK:
            case ACTOR_EN_KAIZOKU:
            case ACTOR_EN_PAMETFROG:
                *should = false;
                break;
            default:
                break;
        }
    });

    // Enemy cutscene actions (camera control, phase changes, etc.)
    COND_VB_SHOULD(VB_ENEMY_CUTSCENE_ACTION, CVAR, {
        Actor* actor = va_arg(args, Actor*);
        switch (actor->id) {
            case ACTOR_BOSS_06: {
                // Igos du Ikana curtain burning. Just instantly snap to the post-burned state
                Boss06* boss06 = (Boss06*)actor;
                boss06->unk_1C9 = 2;
                boss06->unk_1CA = 0;
                boss06->unk_1B4 = 0.0f;
                boss06->unk_1B0 = 0.0f;
                boss06->unk_144 = 2;
                boss06->unk_1A4 = 0.0f;
                boss06->unk_1A0 = 0.0f;
                boss06->unk_1DC = 18.0f;
                boss06->unk_1E0 = 255.0f;
                boss06->unk_19C = 1.0f;
                Actor_SpawnAsChild(&gPlayState->actorCtx, actor, gPlayState, ACTOR_MIR_RAY2, actor->world.pos.x,
                                   actor->world.pos.y - 200.0f, actor->world.pos.z - 170.0f, 15, 0, 0, 1);
                boss06->actionFunc = func_809F2EE8;
                *should = false;
            } break;
            case ACTOR_EN_KNIGHT: {
                // Igos du Ikana standing up. Skip the camera locking and just immediately ready him for battle
                EnKnight* enKnight = (EnKnight*)actor;
                enKnight->unk46C = enKnight->unk470 = 1.0f; // Show sword and shield
                enKnight->unk688 = 0;
                func_809B3E9C(enKnight, gPlayState);
                enKnight->unk14A[2] = 0x12C;
                enKnight->unk152 = 1;
                enKnight->actor.flags |= 1;
                enKnight->actor.gravity = -1.5f;
                *should = false;
            } break;
            case ACTOR_EN_DEATH: {
                EnDeath* enDeath = (EnDeath*)actor;
                if (enDeath->actionFunc == EnDeath_DeathCutscenePart1) {
                    // Gomess death. Manually advance the timer, as camera movement is skipped.
                    if (--enDeath->actionTimer == 0) {
                        EnDeath_SetupDeathCutscenePart2(enDeath, gPlayState);
                    }
                }
                *should = false;
            } break;
            case ACTOR_EN_WIZ: {
                // Start of second phase
                EnWiz* enWiz = (EnWiz*)actor;
                enWiz->action = 2; // EN_WIZ_ACTION_RUN_BETWEEN_PLATFORMS
                enWiz->nextPlatformIndex = 0;
                enWiz->platformCount = 0;
                enWiz->fightState = 2; // EN_WIZ_FIGHT_STATE_SECOND_PHASE_GHOSTS_COPY_WIZROBE
                enWiz->timer = 0;
                EnWiz_SetupDisappear(enWiz);
                *should = false;
            } break;
            case ACTOR_EN_BIGSLIME:
            case ACTOR_EN_EGOL:
            case ACTOR_EN_IK:
            case ACTOR_EN_KAIZOKU:
            case ACTOR_EN_PAMETFROG:
                *should = false;
                break;
            default:
                break;
        }
    });
}

static RegisterShipInitFunc introsInitFunc(RegisterSkipEnemyIntros, { CVAR_NAME });
static RegisterShipInitFunc bossWarpInitFunc(RegisterSkipBossWarpCutscenes, { CVAR_NAME });
static RegisterShipInitFunc enemyCsInitFunc(RegisterSkipEnemyCutscenes, { CVAR_NAME });
