#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include <unordered_map>

extern "C" {
#include "variables.h"
#include "functions.h"
}

#define CVAR_NAME "gEnhancements.DifficultyOptions.BossHealthMultiplier"
#define CVAR CVarGetInteger(CVAR_NAME, 1)

static std::unordered_map<Actor*, int> sLastBossHealth;
static uint32_t sLastFrames = 0;

void RegisterBossHealthMultiplier() {
    COND_HOOK(OnActorUpdate, CVAR > 1, [](Actor* actor) {
        if (actor->id == ACTOR_BOSS_01 || actor->id == ACTOR_BOSS_HAKUGIN || actor->id == ACTOR_BOSS_03 ||
            actor->id == ACTOR_BOSS_02 || actor->id == ACTOR_BOSS_07) {

            if (gPlayState != NULL && Player_InBlockingCsMode(gPlayState, GET_PLAYER(gPlayState))) {
                return;
            }

            if (gPlayState != NULL) {
                if (gPlayState->gameplayFrames < sLastFrames || gPlayState->gameplayFrames == 0) {
                    sLastBossHealth.clear();
                }
                sLastFrames = gPlayState->gameplayFrames;
            }

            int currentHealth = actor->colChkInfo.health;

            if (currentHealth > 0) {
                // Every time a boss phases or unfreezes (e.g. Goth, Majora's transformations), the engine overwrites
                // its health with hardcoded values, so we just watch for health surges and override it again.
                if (sLastBossHealth.count(actor) == 0 || currentHealth > sLastBossHealth[actor]) {
                    int option = CVarGetInteger(CVAR_NAME, 1);
                    float multiplier = 1.0f;

                    if (option == 2)
                        multiplier = 1.5f;
                    if (option == 3)
                        multiplier = 2.0f;

                    if (multiplier > 1.0f) {
                        currentHealth = (s8)(currentHealth * multiplier);
                        actor->colChkInfo.health = currentHealth;
                    }
                }

                sLastBossHealth[actor] = currentHealth;
            }
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterBossHealthMultiplier, { CVAR_NAME });