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

void HandleBossHealthMultiplier(Actor* actor) {
    if (gPlayState != NULL && Player_InBlockingCsMode(gPlayState, GET_PLAYER(gPlayState))) {
        return;
    }

    // If the gameplay frames reset or go backwards, it means the player reloaded the scene,
    // died, or started a new fight. We wipe the map clean to avoid stale health data carrying over.
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
            int option = CVAR;
            float multiplier = 1.0f;

            switch (option) {
                case 2:
                    multiplier = 1.25f;
                    break;
                case 3:
                    multiplier = 1.50f;
                    break;
                case 4:
                    multiplier = 1.75f;
                    break;
                case 5:
                    multiplier = 2.0f;
                    break;
                default:
                    multiplier = 1.0f;
                    break;
            }

            currentHealth = (s8)(currentHealth * multiplier);
            actor->colChkInfo.health = currentHealth;
        }

        sLastBossHealth[actor] = currentHealth;
    }
}

void RegisterBossHealthMultiplier() {
    // Specific COND_ID_HOOK hooks are used instead of a global OnActorUpdate to optimize performance.
    // This prevents the engine from performing checks on thousands of regular actors in every frame,
    // ensuring that this code connects ONLY to these 5 specific boss IDs.
    COND_ID_HOOK(OnActorUpdate, ACTOR_BOSS_01, CVAR > 1, HandleBossHealthMultiplier);
    COND_ID_HOOK(OnActorUpdate, ACTOR_BOSS_HAKUGIN, CVAR > 1, HandleBossHealthMultiplier);
    COND_ID_HOOK(OnActorUpdate, ACTOR_BOSS_03, CVAR > 1, HandleBossHealthMultiplier);
    COND_ID_HOOK(OnActorUpdate, ACTOR_BOSS_02, CVAR > 1, HandleBossHealthMultiplier);
    COND_ID_HOOK(OnActorUpdate, ACTOR_BOSS_07, CVAR > 1, HandleBossHealthMultiplier);
}

static RegisterShipInitFunc initFunc(RegisterBossHealthMultiplier, { CVAR_NAME });