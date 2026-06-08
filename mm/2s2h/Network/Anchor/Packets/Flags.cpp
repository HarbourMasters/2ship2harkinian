#include "2s2h/Network/Anchor/Anchor.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

extern "C" {
#include "variables.h"
#include "functions.h"
extern PlayState* gPlayState;
}

/**
 * SET_FLAG / UNSET_FLAG
 *
 * Sent when the local player sets/unsets a save flag (event, scene switch/chest/etc). Applied on
 * other clients to keep progression in sync. sceneId == SCENE_MAX denotes a non-scene (global)
 * flag such as week event reg or event inf.
 *
 * Scene flags are only applied when the sender is in our current scene; cross-scene scene flags
 * are reconciled via team save-state sync.
 */

void Anchor::SendPacket_SetFlag(s16 sceneId, s16 flagType, s32 flag) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = SET_FLAG;
    payload["targetTeamId"] = CVarGetString("gNetwork.Anchor.TeamId", "default");
    payload["addToQueue"] = true;
    payload["sceneId"] = sceneId;
    payload["flagType"] = flagType;
    payload["flag"] = flag;

    SendPacket(payload);
}

void Anchor::SendPacket_UnsetFlag(s16 sceneId, s16 flagType, s32 flag) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = UNSET_FLAG;
    payload["targetTeamId"] = CVarGetString("gNetwork.Anchor.TeamId", "default");
    payload["addToQueue"] = true;
    payload["sceneId"] = sceneId;
    payload["flagType"] = flagType;
    payload["flag"] = flag;

    SendPacket(payload);
}

void Anchor::HandlePacket_SetFlag(nlohmann::json payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    s16 sceneId = payload.at("sceneId").get<s16>();
    s16 flagType = payload.at("flagType").get<s16>();
    s32 flag = payload.at("flag").get<s32>();

    isApplyingRemotePacket = true;
    switch (flagType) {
        case FLAG_WEEK_EVENT_REG:
            Flags_SetWeekEventReg(flag);
            break;
        case FLAG_WEEK_EVENT_REG_HORSE_RACE:
            Flags_SetWeekEventRegHorseRace((u8)flag);
            break;
        case FLAG_EVENT_INF:
            Flags_SetEventInf(flag);
            break;
        case FLAG_RANDO_INF:
            Flags_SetRandoInf(flag);
            break;
        case FLAG_CYCL_SCENE_SWITCH:
            if (sceneId == gPlayState->sceneId) {
                Flags_SetSwitch(gPlayState, flag);
            }
            break;
        case FLAG_CYCL_SCENE_CHEST:
            if (sceneId == gPlayState->sceneId) {
                Flags_SetTreasure(gPlayState, flag);
            }
            break;
        case FLAG_CYCL_SCENE_CLEARED_ROOM:
            if (sceneId == gPlayState->sceneId) {
                Flags_SetClear(gPlayState, flag);
            }
            break;
        case FLAG_CYCL_SCENE_COLLECTIBLE:
            if (sceneId == gPlayState->sceneId) {
                Flags_SetCollectible(gPlayState, flag);
            }
            break;
        default:
            break;
    }
    isApplyingRemotePacket = false;
}

void Anchor::HandlePacket_UnsetFlag(nlohmann::json payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    s16 sceneId = payload.at("sceneId").get<s16>();
    s16 flagType = payload.at("flagType").get<s16>();
    s32 flag = payload.at("flag").get<s32>();

    isApplyingRemotePacket = true;
    switch (flagType) {
        case FLAG_WEEK_EVENT_REG:
            Flags_ClearWeekEventReg(flag);
            break;
        case FLAG_EVENT_INF:
            Flags_ClearEventInf(flag);
            break;
        case FLAG_RANDO_INF:
            Flags_ClearRandoInf(flag);
            break;
        case FLAG_CYCL_SCENE_SWITCH:
            if (sceneId == gPlayState->sceneId) {
                Flags_UnsetSwitch(gPlayState, flag);
            }
            break;
        case FLAG_CYCL_SCENE_CLEARED_ROOM:
            if (sceneId == gPlayState->sceneId) {
                Flags_UnsetClear(gPlayState, flag);
            }
            break;
        default:
            break;
    }
    isApplyingRemotePacket = false;
}
