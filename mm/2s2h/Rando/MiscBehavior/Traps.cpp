#include "Traps.h"
#include "public/bridge/consolevariablebridge.h"
#include "MiscBehavior.h"
#include "2s2h/DeveloperTools/SaveEditor.h"

extern "C" {
#include "variables.h"
#include "functions.h"
void func_80833B18(PlayState* play, Player* thisx, s32 arg2, f32 speed, f32 velocityY, s16 arg5,
                   s32 invincibilityTimer);
}

extern void UpdateGameTime(u16 gameTime);

int roll = TRAP_FREEZE;
const u16 timeSkipInterval = 4000;

// Delays
int trapDelay = -1;
TrapTypes currentTrap = TRAP_MAX;

std::map<TrapTypes, const char*> trapToCvarMap = {
    { TRAP_FREEZE, "gRando.Traps.Freeze" }, { TRAP_BLAST, "gRando.Traps.Blast" },
    { TRAP_SHOCK, "gRando.Traps.Shock" },   { TRAP_JINX, "gRando.Traps.Jinx" },
    { TRAP_WALLET, "gRando.Traps.Wallet" }, { TRAP_ENEMY, "gRando.Traps.Enemy" },
    { TRAP_TIME, "gRando.Traps.Time" },
};

std::vector<TrapTypes> getEnabledTrapTypes() {
    std::vector<TrapTypes> enabledTrapTypes;
    for (auto& trap : trapToCvarMap) {
        if (CVarGetInteger(trap.second, 0)) {
            enabledTrapTypes.push_back(trap.first);
        }
    }
    if (enabledTrapTypes.size() == 0) {
        enabledTrapTypes.push_back(TRAP_FREEZE);
    }
    return enabledTrapTypes;
};

int RollTrapType() {
    auto enabledTraps = getEnabledTrapTypes();
    roll = enabledTraps[rand() % enabledTraps.size()];
    return roll;
}

// clang-format off
std::vector<std::string> defaultTrapMessages = { 
    "This item is available in the %bRando DLC%w.",
    "This is what happens when %gCaladius%w is left unsupervised.",
    "Oh no!",
    "Uh oh!",
    "KEKW",
    "We've been trying to reach you about your Horses %gextended warranty%w.",
    "Admit it, you wish this was %gGreg%w.",
    "Error 404: Item Not Found",
};

std::vector<std::string> freezeTrapMessages = {
    "%rOcarina of Time%w called, they want their %bIce Trap%w back.",
    "Greetings from %bSnowhead%w! Wish you were here.",
    "This item was too %bcool%w for you anyway.",
    "Let me see your best %bKing Zora%w impersonation.",
    "There's no business like %bSnow%w business!",
    "How much does a polar bear weigh? Enough to break the %bice%w.",
    "You found the %yTrifo%w... Wait, nevermind...",
    "Quick Time Event! Don't die.",
};

std::vector<std::string> blastTrapMessages = {
    "Coming to you live from the %yThunderdome%w!",
    "There was supposed to be an Earth shattering %yKaboom%w!",
    "Pardon me while I %yburst%w.",
    "This is not the greatest %yblast%w in the world, this is just a tribute.",
    "Hey look, this %yitem%w is ticking!",

};

std::vector<std::string> shockTrapMessages = {
    "We're losing him!\n%gCLEAR%w",
    "It's got what %gplants%w need.",
    "On todays episode of Grey's Anatomy...",
    "I'm giving it all I've got, captain!",
    "Now simulating a CD in a Microwave.",
};

std::vector<std::string> timeTrapMessages = {
    "%yTime%w flashes before your eyes!",
    "You have played the %ySun's Song%w!",
    "The %rGoddess of Time%w smites you!",
    "Spent an hour and a half admiring this %bfake item%w.",
    "You found a great place to take a nap!",
    "Break time! Not like the world is ending right?",
    "I just need you to tell me how to get to the %ytime machine%w.",
};
// clang-format on

std::map<TrapTypes, std::vector<std::string>> trapMessageList = {
    { TRAP_FREEZE, freezeTrapMessages },
    { TRAP_BLAST, blastTrapMessages },
    { TRAP_SHOCK, shockTrapMessages },
    { TRAP_TIME, timeTrapMessages },
};

std::string GetTrapMessage() {
    RollTrapType();
    auto findIt = trapMessageList.find((TrapTypes)roll);
    if (findIt == trapMessageList.end()) {
        return defaultTrapMessages[rand() % defaultTrapMessages.size()];
    } else {
        return findIt->second[rand() % findIt->second.size()];
    }
}

void Rando::MiscBehavior::OfferTrapItem() {
    if (!gPlayState) {
        return;
    }

    switch (roll) {
        case TRAP_FREEZE:
            GameInteractor::Instance->events.emplace_back(
                GIEventTrap{ .action = []() { func_80833B18(gPlayState, GET_PLAYER(gPlayState), 3, 0, 0, 0, 0); } });
            break;
        case TRAP_BLAST:
            GameInteractor::Instance->events.emplace_back(GIEventTrap{ .action = []() {
                Actor_Spawn(&gPlayState->actorCtx, gPlayState, ACTOR_EN_BOM, GET_PLAYER(gPlayState)->actor.world.pos.x,
                            GET_PLAYER(gPlayState)->actor.world.pos.y, GET_PLAYER(gPlayState)->actor.world.pos.z, 1, 0,
                            0, 0);
            } });
            break;
        case TRAP_SHOCK:
            GameInteractor::Instance->events.emplace_back(
                GIEventTrap{ .action = []() { func_80833B18(gPlayState, GET_PLAYER(gPlayState), 4, 0, 0, 0, 0); } });
            break;
        case TRAP_JINX:
            GameInteractor::Instance->events.emplace_back(GIEventTrap{ .action = []() {
                Actor_PlaySfx(&GET_PLAYER(gPlayState)->actor, NA_SE_EN_BUBLE_BITE);
                gSaveContext.jinxTimer = 1200;
            } });
            break;
        case TRAP_WALLET:
            GameInteractor::Instance->events.emplace_back(GIEventTrap{ .action = []() {
                int16_t currentRupees = gSaveContext.save.saveInfo.playerData.rupees;
                if (currentRupees != 0) {
                    Vec3f positional = GET_PLAYER(gPlayState)->actor.world.pos;
                    positional.y = GET_PLAYER(gPlayState)->actor.world.pos.y + 100.0f;
                    Item00Type rupee = ITEM00_RUPEE_GREEN;
                    int16_t spawnedRupees = 0;
                    int16_t remainingRupees = currentRupees;
                    for (int i = spawnedRupees; spawnedRupees < remainingRupees;) {
                        if (currentRupees >= 20) {
                            rupee = ITEM00_RUPEE_RED;
                            spawnedRupees += 20;
                            Rupees_ChangeBy(-20);
                            currentRupees -= 20;
                        } else if (currentRupees >= 5) {
                            rupee = ITEM00_RUPEE_BLUE;
                            spawnedRupees += 5;
                            Rupees_ChangeBy(-5);
                            currentRupees -= 5;
                        } else if (currentRupees >= 1) {
                            rupee = ITEM00_RUPEE_GREEN;
                            spawnedRupees += 1;
                            Rupees_ChangeBy(-1);
                            currentRupees -= 1;
                        }
                        EnItem00* rupeeActor = (EnItem00*)Item_DropCollectible(gPlayState, &positional, rupee);
                        rupeeActor->actor.speed = Rand_CenteredFloat(5.0f);
                        rupeeActor->unk152 = 600; // Extending Time before Despawning
                    }
                }
            } });
            break;
        case TRAP_ENEMY:
            GameInteractor::Instance->events.emplace_back(GIEventTrap{ .action = []() {
                int currentSetting = CVarGetInteger("gDeveloperTools.DisableObjectDependency", 0);
                CVarSetInteger("gDeveloperTools.DisableObjectDependency", 1);
                Actor_Spawn(&gPlayState->actorCtx, gPlayState, ACTOR_EN_RR, GET_PLAYER(gPlayState)->actor.world.pos.x,
                            GET_PLAYER(gPlayState)->actor.world.pos.y, GET_PLAYER(gPlayState)->actor.world.pos.z, 0, 0,
                            0, 1);
                CVarSetInteger("gDeveloperTools.DisableObjectDependency", currentSetting);
            } });
            break;
        case TRAP_TIME:
            GameInteractor::Instance->events.emplace_back(GIEventTrap{ .action = []() {
                u16 previous_time = gSaveContext.save.time;
                u16 new_time = gSaveContext.save.time + timeSkipInterval;
                u16 morning_time = 16429;
                if (previous_time < morning_time && new_time >= morning_time) {
                    // Handles case where Night -> Day
                    if (gSaveContext.save.day != 3) {
                        gSaveContext.save.day++;
                        gSaveContext.save.eventDayCount++;
                        UpdateGameTime(new_time);
                        Interface_NewDay(gPlayState, CURRENT_DAY);
                        // Load environment values for new day
                        func_800FEAF4(&gPlayState->envCtx);
                        // Clear weather from day 2
                        gWeatherMode = WEATHER_MODE_CLEAR;
                        gPlayState->envCtx.lightningState = LIGHTNING_OFF;
                    } else {
                        // Handles Moonfall case, prevents skipping past it by setting time right before Moonfall.
                        UpdateGameTime(morning_time - (timeSkipInterval / 10));
                    }
                } else {
                    // Every other case
                    UpdateGameTime(new_time);
                }
                TransitionFade_SetColor(&gPlayState->unk_18E48, 0x000000);
                R_TRANS_FADE_FLASH_ALPHA_STEP = -1;
                Player_PlaySfx(GET_PLAYER(gPlayState), NA_SE_SY_TRANSFORM_MASK_FLASH);
            } });
            break;
        default:
            break;
    }
}

void Rando::MiscBehavior::InitTrapBehavior() {
    /* TODO: Handle being kicked out of a place in a cleaner way
        COND_ID_HOOK(OnActorUpdate, ACTOR_PLAYER, RANDO_SAVE_OPTIONS[RO_SHUFFLE_TRAPS] == 1, [](Actor* actor) {
            if (trapDelay == 0) {
                switch (currentTrap) {
                    case TRAP_TIME:
                        gPlayState->nextEntrance = gPlayState->setupExitList[256 & 0x1F];
                        gPlayState->transitionTrigger = TRANS_TRIGGER_START;
                        Actor_PlaySfx(&GET_PLAYER(gPlayState)->actor, NA_SE_OC_DOOR_OPEN);
                        break;
                    default:
                        break;
                }
                currentTrap = TRAP_MAX;
                trapDelay--;
            }
            if (trapDelay > 0) {
                trapDelay--;
            }
        })
    */
}