#include "Logic.h"

extern "C" {
#include "variables.h"
#include "ShipUtils.h"
}

namespace Rando {

namespace Logic {

void ApplyNearlyNoLogicToSaveContext(std::vector<RandoCheckId>& checkPool, std::vector<RandoItemId>& itemPool) {
    for (size_t i = 0; i < itemPool.size(); i++) {
        std::swap(itemPool[i], itemPool[Ship_Random(0, itemPool.size() - 1)]);
    }

    std::map<RandoItemId, RandoCheckId> importantItems;
    std::vector<RandoCheckId> safeChecks;

    std::map<RandoItemId, std::vector<SceneId>> itemToSceneBlacklist = {
        { RI_MASK_DEKU,
          { SCENE_MITURIN, SCENE_MITURIN_BS, SCENE_LAST_DEKU, SCENE_LAST_GORON, SCENE_LAST_ZORA, SCENE_LAST_LINK,
            SCENE_SOUGEN, SCENE_LAST_BS } },
        { RI_SONG_SONATA,
          { SCENE_MITURIN, SCENE_MITURIN_BS, SCENE_LAST_DEKU, SCENE_LAST_GORON, SCENE_LAST_ZORA, SCENE_LAST_LINK,
            SCENE_SOUGEN, SCENE_LAST_BS } },
        { RI_MASK_ZORA,
          { SCENE_SEA, SCENE_SEA_BS, SCENE_LAST_DEKU, SCENE_LAST_GORON, SCENE_LAST_ZORA, SCENE_LAST_LINK, SCENE_SOUGEN,
            SCENE_LAST_BS } },
        { RI_SONG_NOVA,
          { SCENE_SEA, SCENE_SEA_BS, SCENE_LAST_DEKU, SCENE_LAST_GORON, SCENE_LAST_ZORA, SCENE_LAST_LINK, SCENE_SOUGEN,
            SCENE_LAST_BS } },
        { RI_REMAINS_ODOLWA,
          { SCENE_LAST_DEKU, SCENE_LAST_GORON, SCENE_LAST_ZORA, SCENE_LAST_LINK, SCENE_SOUGEN, SCENE_LAST_BS } },
        { RI_REMAINS_GOHT,
          { SCENE_LAST_DEKU, SCENE_LAST_GORON, SCENE_LAST_ZORA, SCENE_LAST_LINK, SCENE_SOUGEN, SCENE_LAST_BS } },
        { RI_REMAINS_GYORG,
          { SCENE_LAST_DEKU, SCENE_LAST_GORON, SCENE_LAST_ZORA, SCENE_LAST_LINK, SCENE_SOUGEN, SCENE_LAST_BS } },
        { RI_REMAINS_TWINMOLD,
          { SCENE_LAST_DEKU, SCENE_LAST_GORON, SCENE_LAST_ZORA, SCENE_LAST_LINK, SCENE_SOUGEN, SCENE_LAST_BS } },
        { RI_SONG_OATH,
          { SCENE_LAST_DEKU, SCENE_LAST_GORON, SCENE_LAST_ZORA, SCENE_LAST_LINK, SCENE_SOUGEN, SCENE_LAST_BS } },
    };

    std::map<RandoItemId, std::vector<RandoCheckId>> itemToCheckBlacklist = {
        { RI_SONG_EPONA,
          { RC_BENEATH_THE_WELL_COW, RC_ROMANI_RANCH_BARN_COW_LEFT, RC_ROMANI_RANCH_BARN_COW_MIDDLE,
            RC_ROMANI_RANCH_BARN_COW_RIGHT, RC_ROMANI_RANCH_FIELD_COW_ENTRANCE,
            RC_ROMANI_RANCH_FIELD_COW_NEAR_HOUSE_BACK, RC_ROMANI_RANCH_FIELD_COW_NEAR_HOUSE_FRONT,
            RC_TERMINA_FIELD_COW_BACK, RC_TERMINA_FIELD_COW_FRONT, RC_GREAT_BAY_COAST_COW_BACK,
            RC_GREAT_BAY_COAST_COW_FRONT } }
    };

    for (auto& randoCheckId : checkPool) {
        if (randoCheckId == RC_UNKNOWN) {
            continue;
        }

        RandoItemId randoItemId = itemPool.back();
        itemPool.pop_back();

        RANDO_SAVE_CHECKS[randoCheckId].shuffled = true;
        RANDO_SAVE_CHECKS[randoCheckId].randoItemId = randoItemId;

        auto randoStaticCheck = Rando::StaticData::Checks[randoCheckId];

        if (itemToSceneBlacklist.find(randoItemId) != itemToSceneBlacklist.end() ||
            itemToCheckBlacklist.find(randoItemId) != itemToCheckBlacklist.end()) {
            importantItems[randoItemId] = randoCheckId;
        } else if (randoStaticCheck.sceneId != SCENE_MITURIN &&    // Woodfall Temple
                   randoStaticCheck.sceneId != SCENE_MITURIN_BS && // Woodfall Temple Boss
                   randoStaticCheck.sceneId != SCENE_SEA &&        // Great Bay Temple
                   randoStaticCheck.sceneId != SCENE_SEA_BS &&     // Great Bay Temple Boss
                   randoStaticCheck.sceneId != SCENE_LAST_DEKU &&  // Moon Deku
                   randoStaticCheck.sceneId != SCENE_LAST_GORON && // Moon Goron
                   randoStaticCheck.sceneId != SCENE_LAST_ZORA &&  // Moon Goron
                   randoStaticCheck.sceneId != SCENE_LAST_LINK &&  // Moon Human
                   randoStaticCheck.sceneId != SCENE_SOUGEN &&     // Moon
                   randoStaticCheck.sceneId != SCENE_LAST_BS       // Moon Boss
        ) {
            safeChecks.push_back(randoCheckId);
        }
    }

    auto PerformBlacklistReplacement = [&](RandoItemId randoItemId) {
        auto otherCheckIndex = Ship_Random(0, safeChecks.size() - 1);
        RANDO_SAVE_CHECKS[importantItems[randoItemId]].randoItemId =
            RANDO_SAVE_CHECKS[safeChecks[otherCheckIndex]].randoItemId;
        RANDO_SAVE_CHECKS[safeChecks[otherCheckIndex]].randoItemId = randoItemId;
        safeChecks.erase(safeChecks.begin() + otherCheckIndex);
    };

    for (auto& [randoItemId, blacklistedScenes] : itemToSceneBlacklist) {
        if (importantItems.find(randoItemId) != importantItems.end()) {
            auto randoStaticCheck = Rando::StaticData::Checks[importantItems.at(randoItemId)];
            const auto& blacklist = itemToSceneBlacklist.at(randoItemId);
            // This item is blacklisted from this scene, so replace it
            if (std::find(blacklist.begin(), blacklist.end(), randoStaticCheck.sceneId) != blacklist.end()) {
                PerformBlacklistReplacement(randoItemId);
            }
        }
    }

    for (auto& [randoItemId, blackListedChecks] : itemToCheckBlacklist) {
        if (importantItems.find(randoItemId) != importantItems.end()) {
            auto randoStaticCheck = Rando::StaticData::Checks[importantItems.at(randoItemId)];
            const auto& blacklist = itemToCheckBlacklist.at(randoItemId);
            // This item is blacklisted from this check, so replace it
            if (std::find(blacklist.begin(), blacklist.end(), randoStaticCheck.randoCheckId) != blacklist.end()) {
                PerformBlacklistReplacement(randoItemId);
            }
        }
    }
}

} // namespace Logic

} // namespace Rando
