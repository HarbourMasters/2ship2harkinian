#include "Traps.h"
#include "MiscBehavior.h"

extern "C" {
#include "variables.h"
#include "functions.h"
void func_80833B18(PlayState* play, Player* thisx, s32 arg2, f32 speed, f32 velocityY, s16 arg5,
                   s32 invincibilityTimer);
}

int roll = 0;

std::vector<std::string> blastTrapMessages = {
    "Coming to you live from the %yThunderdome%w!",
    "There was supposed to be an Earth shattering %yKaboom%w!",
};

std::vector<std::string> freezeTrapMessages = {
    "This item is available in the %bRando DLC%w.",
};

std::vector<std::string> shockTrapMessages = {
    "We're losing him!\n%gCLEAR%w",
};

std::map<TrapTypes, std::vector<std::string>> trapMessageList = {
    { TRAP_BLAST, blastTrapMessages },
    { TRAP_FREEZE, freezeTrapMessages },
    { TRAP_SHOCK, shockTrapMessages },
};

std::string GetTrapMessage() {
    roll = rand() % TRAP_MAX;
    std::vector<std::string> trapMessages = trapMessageList.at((TrapTypes)roll);
    return trapMessages[rand() % trapMessages.size()];
}

void Rando::MiscBehavior::OfferTrapItem() {
    if (!gPlayState) {
        return;
    }

    switch (roll) {
        case TRAP_BLAST:
            GameInteractor::Instance->events.emplace_back(GIEventTrap{ .action = []() {
                Actor_Spawn(&gPlayState->actorCtx, gPlayState, ACTOR_EN_BOM, GET_PLAYER(gPlayState)->actor.world.pos.x,
                            GET_PLAYER(gPlayState)->actor.world.pos.y, GET_PLAYER(gPlayState)->actor.world.pos.z, 1, 0,
                            0, 0);
            } });
            break;
        case TRAP_FREEZE:
            GameInteractor::Instance->events.emplace_back(
                GIEventTrap{ .action = []() { func_80833B18(gPlayState, GET_PLAYER(gPlayState), 3, 0, 0, 0, 0); } });
            break;
        case TRAP_SHOCK:
            GameInteractor::Instance->events.emplace_back(
                GIEventTrap{ .action = []() { func_80833B18(gPlayState, GET_PLAYER(gPlayState), 4, 0, 0, 0, 0); } });
            break;
        default:
            break;
    }
}
