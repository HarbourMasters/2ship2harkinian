#ifndef RANDO_STATIC_DATA_H
#define RANDO_STATIC_DATA_H

#include <map>
#include "Rando/Types.h"
#include "2s2h/GameInteractor/GameInteractor.h"

extern "C" {
#include "z64item.h"
#include "z64scene.h"
}

namespace Rando {

namespace StaticData {

struct RandoStaticCheck {
    RandoCheckId randoCheckId;
    const char* name;
    RandoCheckType randoCheckType;
    SceneId sceneId;
    FlagType flagType;
    s32 flag;
    RandoItemId randoItemId;
};

extern std::map<RandoCheckId, RandoStaticCheck> Checks;

RandoStaticCheck GetCheckFromFlag(FlagType flagType, s32 flag, s16 sceneId = SCENE_MAX);
RandoCheckId GetCheckIdFromName(const char* name);

struct RandoStaticItem {
    RandoItemId randoItemId;
    const char* spoilerName;
    const char* articleEng;
    const char* nameEng;
    const char* articleFre;
    const char* nameFre;
    const char* articleGer;
    const char* articleGer2; // German use article variation depending on the context
    const char* nameGer;
    const char* articleJpn;
    const char* nameJpn;
    const char* articleSpa;
    const char* nameSpa;
    RandoItemType randoItemType;
    ItemId itemId;
    GetItemId getItemId;
    GetItemDrawId drawId;
};

extern std::map<RandoItemId, RandoStaticItem> Items;
extern std::vector<RandoItemId> StartingItemsMap;

RandoItemId GetItemIdFromName(const char* name);
u8 GetIconForZMessage(RandoItemId itemId);
const char* GetIconTexturePath(RandoItemId itemId);
bool ShouldShowGetItemCutscene(RandoItemId itemId);
std::string GetItemName(RandoItemId randoItemId, bool includeArticle = true);

struct RandoStaticOption {
    RandoOptionId randoOptionId;
    const char* name;
    const char* cvar;
    u32 defaultValue;
};

extern std::map<RandoOptionId, RandoStaticOption> Options;

RandoOptionId GetOptionIdFromName(const char* name);

struct RandoStaticRegion {
    RandoRegionId randoRegionId;
    const char* name;
    SceneId sceneId;
    std::unordered_map<RandoCheckId, std::function<bool()>> checks;
    std::unordered_map<RandoRegionId, std::function<bool()>> regions;
};

extern std::map<RandoRegionId, RandoStaticRegion> Regions;

} // namespace StaticData

} // namespace Rando

#endif // RANDO_STATIC_DATA_H
