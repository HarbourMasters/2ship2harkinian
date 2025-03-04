#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "variables.h"
#include "functions.h"
}

#define CVAR_NAME "gEnhancements.Cutscenes.SkipStoryCutscenes"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterSkipWakingAndRidingTurtle() {
    COND_VB_SHOULD(VB_START_CUTSCENE, CVAR, {
        s16* csId = va_arg(args, s16*);
        if (gPlayState->sceneId == SCENE_31MISAKI) {
            // 12 is first time waking turtle, 20 is subsequent times
            if (*csId == 12 || *csId == 20) {
                *should = false;

                // Need to reload scene. Position is where Link ends up after CS
                // Differences from no skip: Camera ends up behind Link, Lulu is gone
                // Based on WarpPoint.cpp
                // Player* player = GET_PLAYER(gPlayState);

                // gPlayState->nextEntrance = ENTRANCE(ZORA_CAPE, 2);
                // gPlayState->transitionTrigger = TRANS_TRIGGER_START;
                // gPlayState->transitionType = TRANS_TYPE_INSTANT;

                // gSaveContext.respawn[RESPAWN_MODE_DOWN].entrance = ENTRANCE(ZORA_CAPE, 2);
                // gSaveContext.respawn[RESPAWN_MODE_DOWN].roomIndex = 0;
                // gSaveContext.respawn[RESPAWN_MODE_DOWN].pos.x = -5525.0f;
                // gSaveContext.respawn[RESPAWN_MODE_DOWN].pos.y = 14.0f;
                // gSaveContext.respawn[RESPAWN_MODE_DOWN].pos.z = 1548.0f;
                // gSaveContext.respawn[RESPAWN_MODE_DOWN].yaw = -16384;
                // gSaveContext.respawn[RESPAWN_MODE_DOWN].playerParams = PLAYER_PARAMS(0xFF, PLAYER_INITMODE_D);
                // gSaveContext.nextTransitionType = TRANS_TYPE_FADE_BLACK_FAST;
                // gSaveContext.respawnFlag = -8;

                Actor* turtle;
                Actor* currentActor = gPlayState->actorCtx.actorLists[ACTORCAT_BG].first;
                if (currentActor != nullptr) {
                    while (currentActor != nullptr) {
                        if (currentActor->id == ACTOR_DM_CHAR08) {
                            turtle = currentActor;
                        }
                        currentActor = currentActor->next;
                    }
                }

                // This gets sets a bit later after this hook executes, but needs to be set before respawning turtle
                SET_WEEKEVENTREG(WEEKEVENTREG_53_20);
                // probably need to kill its palm trees too
                Actor_Kill(turtle);

                // posX: -6238.000000, posY: -320.000000, posZ: 1749.000000, rotX: 0, rotY: 0, rotZ: 0, params: 0, csId: c, half: 3ff, parent: 0
                Actor_SpawnAsChildAndCutscene(&gPlayState->actorCtx, gPlayState, ACTOR_DM_CHAR08, -6238.0, -320.0, 1749.0, 0, 0, 0, 0, 0xc, 0x3ff, NULL);

                //Can't seem to get this to play after respawing

                Audio_PlayFanfare(NA_BGM_DUNGEON_APPEAR);
            }
            // 13 is turtle leaving zora cape first time, 15 is subsequent times
            if (*csId == 13 || *csId == 15) {
                *should = false;
                GameInteractor::Instance->events.emplace_back(GIEventTransition{
                    .entrance = ENTRANCE(GREAT_BAY_TEMPLE, 0),
                    .cutsceneIndex = 0,
                    .transitionTrigger = TRANS_TRIGGER_START,
                    .transitionType = TRANS_TYPE_FADE_BLACK_FAST,
                });
            }
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterSkipWakingAndRidingTurtle, { CVAR_NAME });
