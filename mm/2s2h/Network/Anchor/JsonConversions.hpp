#ifndef NETWORK_ANCHOR_JSON_CONVERSIONS_H
#define NETWORK_ANCHOR_JSON_CONVERSIONS_H
#ifdef __cplusplus

#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>
#include "Anchor.h"

extern "C" {
#include "z64.h"
}

using json = nlohmann::json;

inline void to_json(json& j, const Color_RGB8& color) {
    j = json{ { "r", color.r }, { "g", color.g }, { "b", color.b } };
}

inline void from_json(const json& j, Color_RGB8& color) {
    j.at("r").get_to(color.r);
    j.at("g").get_to(color.g);
    j.at("b").get_to(color.b);
}

inline void from_json(const json& j, AnchorClient& client) {
    client.clientId = j.value("clientId", (u32)0);
    client.name = j.value("name", "???");
    client.color = j.value("color", Color_RGB8{ 255, 255, 255 });
    client.clientVersion = j.value("clientVersion", "???");
    client.teamId = j.value("teamId", "default");
    client.online = j.value("online", false);
    client.self = j.value("self", false);
    client.seed = j.value("seed", (u32)0);
    client.isSaveLoaded = j.value("isSaveLoaded", false);
    client.isGameComplete = j.value("isGameComplete", false);
    client.sceneId = j.value("sceneId", (s16)SCENE_MAX);
    client.curRoomNum = j.value("curRoomNum", (s8)-1);
    client.entranceIndex = j.value("entranceIndex", (s32)0);
}

#endif // __cplusplus
#endif // NETWORK_ANCHOR_JSON_CONVERSIONS_H
