#include "ShipUtils.h"
#include <libultraship/libultraship.h>
#include "assets/2s2h_assets.h"
#include <string>
#include <random>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost_custom/container_hash/hash_32.hpp>

// Image Icons
#include "assets/interface/parameter_static/parameter_static.h"
#include "assets/archives/icon_item_24_static/icon_item_24_static_yar.h"
#include "assets/archives/icon_item_static/icon_item_static_yar.h"
#include "assets/archives/schedule_dma_static/schedule_dma_static_yar.h"
#include "assets/interface/icon_item_dungeon_static/icon_item_dungeon_static.h"
#include "assets/interface/icon_item_field_static/icon_item_field_static.h"
#include "assets/archives/icon_item_static/icon_item_static_yar.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"
#include "assets/objects/object_mag/object_mag.h"

extern "C" {
#include "z64.h"
#include "functions.h"
#include "macros.h"

extern float OTRGetAspectRatio();

extern f32 sNESFontWidths[160];
extern const char* fontTbl[156];
extern TexturePtr gItemIcons[131];
extern TexturePtr gQuestIcons[14];
extern TexturePtr gBombersNotebookPhotos[24];
}

// 2S2H Added columns to scene table: entranceSceneId, betterMapSelectIndex, humanName
#define DEFINE_SCENE(_name, enumValue, _textId, _drawConfig, _restrictionFlags, _persistentCycleFlags, \
                     _entranceSceneId, _betterMapSelectIndex, humanName)                               \
    { enumValue, humanName },
#define DEFINE_SCENE_UNSET(_enumValue)

std::unordered_map<s16, const char*> sceneNames = {
#include "tables/scene_table.h"
};

#undef DEFINE_SCENE
#undef DEFINE_SCENE_UNSET

// These textures are not in existing lists that we iterate over.
std::vector<const char*> miscellaneousTextures = {
    gArcheryScoreIconTex,
    gBarrelTrackerIcon,
    gChestTrackerIcon,
    gCrateTrackerIcon,
    gDungeonStrayFairyGreatBayIconTex,
    gDungeonStrayFairySnowheadIconTex,
    gDungeonStrayFairyStoneTowerIconTex,
    gDungeonStrayFairyWoodfallIconTex,
    gItemIconEmptyBottleTex,
    gPotTrackerIcon,
    gQuestIconGoldSkulltulaTex,
    gMagicArrowEquipEffectTex,
    gRupeeCounterIconTex,
    gStrayFairyGreatBayIconTex,
    gStrayFairySnowheadIconTex,
    gStrayFairyStoneTowerIconTex,
    gStrayFairyWoodfallIconTex,
    gTimerClockIconTex,
    gWorldMapOwlFaceTex,
    gTitleScreenMajorasMaskTex,
};

std::vector<const char*> digitList = { gCounterDigit0Tex, gCounterDigit1Tex, gCounterDigit2Tex, gCounterDigit3Tex,
                                       gCounterDigit4Tex, gCounterDigit5Tex, gCounterDigit6Tex, gCounterDigit7Tex,
                                       gCounterDigit8Tex, gCounterDigit9Tex, gCounterColonTex };

std::vector<int16_t> orderedInventoryItemList = {
    ITEM_OCARINA_OF_TIME,
    ITEM_BOW,
    ITEM_ARROW_FIRE,
    ITEM_ARROW_ICE,
    ITEM_ARROW_LIGHT,
    ITEM_MOONS_TEAR,
    ITEM_BOMB,
    ITEM_BOMBCHU,
    ITEM_DEKU_STICK,
    ITEM_DEKU_NUT,
    ITEM_MAGIC_BEANS,
    ITEM_ROOM_KEY,
    ITEM_POWDER_KEG,
    ITEM_PICTOGRAPH_BOX,
    ITEM_LENS_OF_TRUTH,
    ITEM_HOOKSHOT,
    ITEM_SWORD_GREAT_FAIRY,
    ITEM_LETTER_TO_KAFEI,
    ITEM_BOTTLE,
    ITEM_BOTTLE,
    ITEM_BOTTLE,
    ITEM_BOTTLE,
    ITEM_BOTTLE,
    ITEM_BOTTLE,
    ITEM_MASK_POSTMAN,
    ITEM_MASK_ALL_NIGHT,
    ITEM_MASK_BLAST,
    ITEM_MASK_STONE,
    ITEM_MASK_GREAT_FAIRY,
    ITEM_MASK_DEKU,
    ITEM_MASK_KEATON,
    ITEM_MASK_BREMEN,
    ITEM_MASK_BUNNY,
    ITEM_MASK_DON_GERO,
    ITEM_MASK_SCENTS,
    ITEM_MASK_GORON,
    ITEM_MASK_ROMANI,
    ITEM_MASK_CIRCUS_LEADER,
    ITEM_MASK_KAFEIS_MASK,
    ITEM_MASK_COUPLE,
    ITEM_MASK_TRUTH,
    ITEM_MASK_ZORA,
    ITEM_MASK_KAMARO,
    ITEM_MASK_GIBDO,
    ITEM_MASK_GARO,
    ITEM_MASK_CAPTAIN,
    ITEM_MASK_GIANT,
    ITEM_MASK_FIERCE_DEITY,
};

std::map<int16_t, int16_t> questToItemMap = {
    { QUEST_REMAINS_ODOLWA, ITEM_REMAINS_ODOLWA },
    { QUEST_REMAINS_GOHT, ITEM_REMAINS_GOHT },
    { QUEST_REMAINS_GYORG, ITEM_REMAINS_GYORG },
    { QUEST_REMAINS_TWINMOLD, ITEM_REMAINS_TWINMOLD },
    { QUEST_SONG_SONATA, ITEM_SONG_SONATA },
    { QUEST_SONG_LULLABY, ITEM_SONG_LULLABY },
    { QUEST_SONG_BOSSA_NOVA, ITEM_SONG_NOVA },
    { QUEST_SONG_ELEGY, ITEM_SONG_ELEGY },
    { QUEST_SONG_OATH, ITEM_SONG_OATH },
    { QUEST_SONG_SARIA, ITEM_SONG_SARIA },
    { QUEST_SONG_TIME, ITEM_SONG_TIME },
    { QUEST_SONG_HEALING, ITEM_SONG_HEALING },
    { QUEST_SONG_EPONA, ITEM_SONG_EPONA },
    { QUEST_SONG_SOARING, ITEM_SONG_SOARING },
    { QUEST_SONG_STORMS, ITEM_SONG_STORMS },
    { QUEST_SONG_SUN, ITEM_SONG_SUN },
    { QUEST_BOMBERS_NOTEBOOK, ITEM_BOMBERS_NOTEBOOK },
};

ImVec4 Ship_SongColors(int16_t itemID) {
    switch (itemID) {
        case ITEM_SONG_SONATA:
            return ImVec4(0.588f, 1, 0.392f, 1);
        case ITEM_SONG_LULLABY:
        case ITEM_SONG_LULLABY_INTRO:
            return ImVec4(1, 0.313f, 0.156f, 1);
        case ITEM_SONG_NOVA:
            return ImVec4(0.392f, 0.588f, 1, 1);
        case ITEM_SONG_ELEGY:
            return ImVec4(1, 0.627f, 0, 1);
        case ITEM_SONG_OATH:
            return ImVec4(1, 0.392f, 1, 1);
        default:
            return ImVec4(1, 1, 1, 1);
    }
};

ImVec4 Ship_DungeonKeyColors(int16_t dungeonIndex) {
    switch (dungeonIndex) {
        case DUNGEON_INDEX_WOODFALL_TEMPLE:
            return ImVec4(0.92f, 0.47f, 0.73f, 1.0f);
        case DUNGEON_INDEX_SNOWHEAD_TEMPLE:
            return ImVec4(0.5f, 0.68f, 0.27f, 1.0f);
        case DUNGEON_INDEX_GREAT_BAY_TEMPLE:
            return ImVec4(0.38f, 0.35f, 0.72f, 1.0f);
        case DUNGEON_INDEX_STONE_TOWER_TEMPLE:
            return ImVec4(0.69f, 0.65f, 0.32f, 1.0f);
        default:
            break;
    }
}

const char* fairyIcons[] = { gDungeonStrayFairyWoodfallIconTex, gDungeonStrayFairySnowheadIconTex,
                             gDungeonStrayFairyGreatBayIconTex, gDungeonStrayFairyStoneTowerIconTex };

extern "C" int16_t findQuestByItem(int16_t itemId) {
    for (auto& [quest, itemVal] : questToItemMap) {
        if (itemVal == itemId) {
            return quest;
        }
    }
    return -1;
}

extern "C" const char* Ship_GetSceneName(s16 sceneId) {
    if (sceneNames.contains(sceneId)) {
        return sceneNames[sceneId];
    }

    return "Unknown";
}

constexpr f32 fourByThree = 4.0f / 3.0f;

// Gets the additional ratio of the screen compared to the original 4:3 ratio, clamping to 1 if smaller
extern "C" f32 Ship_GetExtendedAspectRatioMultiplier() {
    f32 currentRatio = OTRGetAspectRatio();
    return MAX(currentRatio / fourByThree, 1.0f);
}

// Enables Extended Culling options on specific actors by applying an inverse ratio of the draw distance slider
// to the projected Z value of the actor. This tricks distance checks without having to replace hardcoded values.
// Requires that Ship_ExtendedCullingActorRestoreProjectedPos is called within the same function scope.
extern "C" void Ship_ExtendedCullingActorAdjustProjectedZ(Actor* actor) {
    s32 multiplier = CVarGetInteger("gEnhancements.Graphics.IncreaseActorDrawDistance", 1);
    multiplier = MAX(multiplier, 1);
    if (multiplier > 1) {
        actor->projectedPos.z /= multiplier;
    }
}

// Enables Extended Culling options on specific actors by applying an inverse ratio of the widescreen aspect ratio
// to the projected X value of the actor. This tricks distance checks without having to replace hardcoded values.
// Requires that Ship_ExtendedCullingActorRestoreProjectedPos is called within the same function scope.
extern "C" void Ship_ExtendedCullingActorAdjustProjectedX(Actor* actor) {
    if (CVarGetInteger("gEnhancements.Graphics.ActorCullingAccountsForWidescreen", 0)) {
        f32 ratioAdjusted = Ship_GetExtendedAspectRatioMultiplier();
        actor->projectedPos.x /= ratioAdjusted;
    }
}

// Restores the projectedPos values on the actor after modifications from the Extended Culling hacks
extern "C" void Ship_ExtendedCullingActorRestoreProjectedPos(PlayState* play, Actor* actor) {
    f32 invW = 0.0f;
    Actor_GetProjectedPos(play, &actor->world.pos, &actor->projectedPos, &invW);
}

extern "C" bool Ship_IsCStringEmpty(const char* str) {
    return str == NULL || str[0] == '\0';
}

// Build vertex coordinates for a quad command
// In order of top left, top right, bottom left, then bottom right
// Supports flipping the texture horizontally
extern "C" void Ship_CreateQuadVertexGroup(Vtx* vtxList, s32 xStart, s32 yStart, s32 width, s32 height, u8 flippedH) {
    vtxList[0].v.ob[0] = xStart;
    vtxList[0].v.ob[1] = yStart;
    vtxList[0].v.tc[0] = (flippedH ? width : 0) << 5;
    vtxList[0].v.tc[1] = 0 << 5;

    vtxList[1].v.ob[0] = xStart + width;
    vtxList[1].v.ob[1] = yStart;
    vtxList[1].v.tc[0] = (flippedH ? width * 2 : width) << 5;
    vtxList[1].v.tc[1] = 0 << 5;

    vtxList[2].v.ob[0] = xStart;
    vtxList[2].v.ob[1] = yStart + height;
    vtxList[2].v.tc[0] = (flippedH ? width : 0) << 5;
    vtxList[2].v.tc[1] = height << 5;

    vtxList[3].v.ob[0] = xStart + width;
    vtxList[3].v.ob[1] = yStart + height;
    vtxList[3].v.tc[0] = (flippedH ? width * 2 : width) << 5;
    vtxList[3].v.tc[1] = height << 5;
}

extern "C" f32 Ship_GetCharFontWidthNES(u8 character) {
    u8 adjustedChar = character - ' ';

    if (adjustedChar >= ARRAY_COUNTU(sNESFontWidths)) {
        return 0.0f;
    }

    return sNESFontWidths[adjustedChar];
}

extern "C" TexturePtr Ship_GetCharFontTextureNES(u8 character) {
    u8 adjustedChar = character - ' ';

    if (adjustedChar >= ARRAY_COUNTU(sNESFontWidths)) {
        return (TexturePtr)gEmptyTexture;
    }

    return (TexturePtr)fontTbl[adjustedChar];
}

static bool seeded = false;
static boost::random::mt19937 generator;

extern "C" void Ship_Random_Seed(u32 seed) {
    seeded = true;
    generator = boost::random::mt19937{ seed };
}

extern "C" s32 Ship_Random(s32 min, s32 max) {
    if (!seeded) {
        const auto seed = static_cast<uint32_t>(std::random_device{}());
        Ship_Random_Seed(seed);
    }
    boost::random::uniform_int_distribution<uint32_t> distribution(min, max - 1);
    return distribution(generator);
}

void LoadGuiTextures() {
    for (TexturePtr entry : gItemIcons) {
        const char* path = static_cast<const char*>(entry);
        Ship::Context::GetInstance()->GetWindow()->GetGui()->LoadGuiTexture(path, path, ImVec4(1, 1, 1, 1));
    }
    for (TexturePtr entry : gQuestIcons) {
        const char* path = static_cast<const char*>(entry);
        Ship::Context::GetInstance()->GetWindow()->GetGui()->LoadGuiTexture(path, path, ImVec4(1, 1, 1, 1));
    }
    for (TexturePtr entry : gBombersNotebookPhotos) {
        const char* path = static_cast<const char*>(entry);
        Ship::Context::GetInstance()->GetWindow()->GetGui()->LoadGuiTexture(path, path, ImVec4(1, 1, 1, 1));
    }
    for (auto& entry : miscellaneousTextures) {
        const char* path = static_cast<const char*>(entry);
        Ship::Context::GetInstance()->GetWindow()->GetGui()->LoadGuiTexture(path, path, ImVec4(1, 1, 1, 1));
    }
    for (auto& entry : digitList) {
        const char* path = static_cast<const char*>(entry);
        Ship::Context::GetInstance()->GetWindow()->GetGui()->LoadGuiTexture(path, path, ImVec4(1, 1, 1, 1));
    }
}

std::string convertEnumToReadableName(const std::string& input) {
    std::string result;
    std::string content = input;

    // Step 1: Remove "RC_" prefix if present
    const std::string prefix = "RC_";
    if (content.rfind(prefix, 0) == 0) {
        content = content.substr(prefix.size());
    }

    // Step 2: Split the string by '_'
    std::vector<std::string> words;
    std::string word;
    std::istringstream stream(content);
    while (std::getline(stream, word, '_')) {
        words.push_back(word);
    }

    // Step 3: Capitalize the first letter of each word
    for (auto& w : words) {
        std::transform(w.begin(), w.end(), w.begin(), [](unsigned char c) { return std::tolower(c); });
        if (!w.empty()) {
            if (w == "hp") {
                w = "HP";
            } else {
                w[0] = std::toupper(w[0]);
            }
        }
    }

    // Step 4: Join the words with spaces
    for (size_t i = 0; i < words.size(); ++i) {
        result += words[i];
        if (i < words.size() - 1) {
            result += " ";
        }
    }

    return result;
}
