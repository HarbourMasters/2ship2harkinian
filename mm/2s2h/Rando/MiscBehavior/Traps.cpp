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
    { TRAP_FREEZE, "gRando.Traps.Freeze" }, { TRAP_BLAST, "gRando.Traps.Blast" }, { TRAP_SHOCK, "gRando.Traps.Shock" },
    { TRAP_JINX, "gRando.Traps.Jinx" },     { TRAP_ENEMY, "gRando.Traps.Enemy" }, { TRAP_TIME, "gRando.Traps.Time" },
};

std::unordered_map<SceneId, std::pair<int, std::pair<s32, s32>>> kickOutMap{
    { SCENE_8ITEMSHOP, { 0x1883, { CLOCK_TIME(21, 0), CLOCK_TIME(22, 0) } } },
    { SCENE_TAKARAKUJI, { 0x1887, { CLOCK_TIME(23, 0), CLOCK_TIME(6, 0) } } },
    { SCENE_DOUJOU, { 0x1807, { CLOCK_TIME(23, 0), CLOCK_TIME(0, 30) } } },
    { SCENE_MILK_BAR, { 0x1889, { CLOCK_TIME(22, 0), CLOCK_TIME(5, 0) } } },
    { SCENE_BOWLING, { 0x1886, { CLOCK_TIME(22, 0), CLOCK_TIME(6, 0) } } },
    { SCENE_TAKARAYA, { 0x1892, { CLOCK_TIME(22, 0), CLOCK_TIME(6, 0) } } },
    { SCENE_SYATEKI_MIZU, { 0x188f, { CLOCK_TIME(22, 0), CLOCK_TIME(6, 0) } } },
    { SCENE_SONCHONOIE, { 0x1889, { CLOCK_TIME(20, 0), CLOCK_TIME(10, 0) } } },
    { SCENE_AYASHIISHOP, { 0x1889, { CLOCK_TIME(5, 0), CLOCK_TIME(22, 0) } } },
    { SCENE_SYATEKI_MORI, { 0x1884, { CLOCK_TIME(22, 0), CLOCK_TIME(6, 0) } } },
    { SCENE_POSTHOUSE, { 0x1889, { CLOCK_TIME(23, 59), CLOCK_TIME(9, 0) } } },
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

std::vector<std::string> defaultTrapMessages = { "This item is available in the %bRando DLC%w.",
                                                 "This is what happens when %gCaladius%w is left unsupervised." };

std::vector<std::string> freezeTrapMessages = {
    "%rOcarina of Time%w called, they want their %bIce Trap%w back.",
};

std::vector<std::string> blastTrapMessages = {
    "Coming to you live from the %yThunderdome%w!",
    "There was supposed to be an Earth shattering %yKaboom%w!",
};

std::vector<std::string> shockTrapMessages = {
    "We're losing him!\n%gCLEAR%w",
};

std::vector<std::string> timeTrapMessages = {
    "Time flashes before your eyes!",         "You have played the Sun's Song!",
    "The Goddess of Time smites you!",        "Spent an hour and a half admiring this fake item.",
    "You found a great place to take a nap!", "Break time! Not like the world is ending right?",
};

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

                for (auto& kick : kickOutMap) {
                    SceneId checked_scene = kick.first;
                    s32 close_time = kick.second.second.first;
                    s32 reopen_time = kick.second.second.second;
                    int msg_id = kick.second.first;
                    bool past_midnight = previous_time > new_time;
                    bool triggered = false;
                    if (gPlayState->sceneId == checked_scene) {
                        if (checked_scene == SCENE_POSTHOUSE && CURRENT_DAY == 3) {
                            // Special case for Postoffice, it does not close on day 3
                            continue;
                        } else {
                            // Handles midnight crossing edgecases
                            if (reopen_time < close_time) {
                                if (gSaveContext.save.time >= close_time) {
                                    // For cases where it is triggered before midnight, but closes before midnight, but
                                    // reopens after midnight.
                                    triggered = true;
                                } else if (previous_time <= close_time && gSaveContext.save.time <= reopen_time &&
                                           past_midnight) {
                                    // For cases where it is triggered through midnight, closes before midnight, but
                                    // reopens after midnight.
                                    triggered = true;
                                }
                            }
                            if (gSaveContext.save.time >= close_time && gSaveContext.save.time <= reopen_time) {
                                // For cases where it does not trigger through midnight, closes before midnight, and
                                // reopens before midnight.
                                triggered = true;
                            }
                        }
                        if (triggered) {
                            Message_StartTextbox(gPlayState, msg_id, NULL);
                            currentTrap = (TrapTypes)roll;
                            trapDelay = 3;
                            break;
                        }
                    }
                }
            } });
            break;
        default:
            break;
    }
}

void Rando::MiscBehavior::InitTrapBehavior() {
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
}