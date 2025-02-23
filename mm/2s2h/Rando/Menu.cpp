#include "Rando/Rando.h"
#include "Rando/Spoiler/Spoiler.h"
#include <libultraship/libultraship.h>
#include "2s2h/BenGui/UIWidgets.hpp"
#include "Rando/CheckTracker/CheckTracker.h"
#include "BenPort.h"
#include "build.h"
#include "2s2h/BenGui/BenMenu.h"

#include <bitset>

// TODO: This block should come from elsewhere, tied to data in Rando::StaticData::Options
std::unordered_map<int32_t, const char*> logicOptions = {
    { RO_LOGIC_GLITCHLESS, "Glitchless" },
    { RO_LOGIC_NO_LOGIC, "No Logic" },
    { RO_LOGIC_NEARLY_NO_LOGIC, "Nearly No Logic" },
    { RO_LOGIC_FRENCH_VANILLA, "French Vanilla" },
    { RO_LOGIC_VANILLA, "Vanilla" },
};

std::unordered_map<int32_t, const char*> accessDungeonOptions = {
    { RO_ACCESS_DUNGEONS_FORM_AND_SONG, "Requires Transformation & Song" },
    { RO_ACCESS_DUNGEONS_FORM_ONLY, "Requires Transformation" },
};

std::unordered_map<int32_t, const char*> accessTrialsOptions = {
    { RO_ACCESS_TRIALS_20_MASKS, "2-6-12-20 Masks" },
    { RO_ACCESS_TRIALS_REMAINS, "Requires Associated Remains" },
    { RO_ACCESS_TRIALS_FORMS, "Requires Assocaited Transformation" },
    { RO_ACCESS_TRIALS_OPEN, "Open" },
};

std::unordered_map<StartingItemCategory, std::pair<const char*, int8_t>> startingItemCategoryLabels = {
    { STARTING_ITEMS_INVENTORY, { "Inventory", 5 } },
    { STARTING_ITEMS_MASK, { "Masks", 6 } },
    { STARTING_ITEMS_QUEST, { "Quest Items", 5 } },
    { STARTING_ITEMS_TRADE, { "Trade Items", 5 } },
};

namespace BenGui {
extern std::shared_ptr<Rando::CheckTracker::CheckTrackerWindow> mRandoCheckTrackerWindow;
extern std::shared_ptr<Rando::CheckTracker::SettingsWindow> mRandoCheckTrackerSettingsWindow;
extern std::shared_ptr<BenMenu> mBenMenu;
} // namespace BenGui

using namespace BenGui;
using namespace UIWidgets;

extern "C" {
#include "archives/icon_item_24_static/icon_item_24_static_yar.h"
}

std::bitset<32> LoadBitset(RandoOptionId optionId) {
    std::bitset<32> selectedItemsBitset = CVarGetInteger(Rando::StaticData::Options[optionId].cvar, 0);
    return selectedItemsBitset;
}

std::bitset<32> GetBitset(RandoItemId randoItemId) {
    std::bitset<32> selectedItemsBitset;
    if (randoItemId <= RI_DEKU_NUTS_5) {
        selectedItemsBitset = LoadBitset(RO_STARTING_ITEMS_1);
    } else if (randoItemId <= RI_MASK_CAPTAIN) {
        selectedItemsBitset = LoadBitset(RO_STARTING_ITEMS_2);
    } else if (randoItemId <= RI_OWL_SOUTHERN_SWAMP) {
        selectedItemsBitset = LoadBitset(RO_STARTING_ITEMS_3);
    } else if (randoItemId <= RI_SNOWHEAD_BOSS_KEY) {
        selectedItemsBitset = LoadBitset(RO_STARTING_ITEMS_4);
    } else if (randoItemId <= RI_TINGLE_MAP_CLOCK_TOWN) {
        selectedItemsBitset = LoadBitset(RO_STARTING_ITEMS_5);
    } else {
        selectedItemsBitset = LoadBitset(RO_STARTING_ITEMS_6);
    }
    return selectedItemsBitset;
}

static void SaveStartingItemsBitset(std::bitset<32> bitset, RandoItemId randoItemId) {
    RandoOptionId optionId;
    if (randoItemId <= RI_DEKU_NUTS_5) {
        optionId = RO_STARTING_ITEMS_1;
    } else if (randoItemId <= RI_MASK_CAPTAIN) {
        optionId = RO_STARTING_ITEMS_2;
    } else if (randoItemId <= RI_OWL_SOUTHERN_SWAMP) {
        optionId = RO_STARTING_ITEMS_3;
    } else if (randoItemId <= RI_SNOWHEAD_BOSS_KEY) {
        optionId = RO_STARTING_ITEMS_4;
    } else if (randoItemId <= RI_TINGLE_MAP_CLOCK_TOWN) {
        optionId = RO_STARTING_ITEMS_5;
    } else {
        optionId = RO_STARTING_ITEMS_6;
    }
    CVarSetInteger(Rando::StaticData::Options[optionId].cvar, bitset.to_ulong());
}

bool CheckStartingItemsBitset(RandoItemId randoItemId) {
    uint32_t item = randoItemId % 32;
    std::bitset<32> selectedItemsBitset = GetBitset(randoItemId);

    return selectedItemsBitset.test(item);
}

static void SetStartingItemsBit(RandoItemId randoItemId) {
    uint32_t item = randoItemId % 32;
    std::bitset<32> selectedItemsBitset = GetBitset(randoItemId);
    selectedItemsBitset.set(item);
    SaveStartingItemsBitset(selectedItemsBitset, randoItemId);
}

static void ResetStartingItemsBit(RandoItemId randoItemId) {
    uint32_t item = randoItemId % 32;
    std::bitset<32> selectedItemsBitset = GetBitset(randoItemId);
    selectedItemsBitset.reset(item);
    SaveStartingItemsBitset(selectedItemsBitset, randoItemId);
}

static void FlipStartingItemsBitset(RandoItemId randoItemId) {
    uint32_t item = randoItemId % 32;
    std::bitset<32> selectedItemsBitset;

    switch (randoItemId) {
        case RI_PROGRESSIVE_BOW:
            if (CheckStartingItemsBitset(RI_QUIVER_50)) {
                ResetStartingItemsBit(RI_QUIVER_50);
                ResetStartingItemsBit(RI_QUIVER_40);
                ResetStartingItemsBit(RI_BOW);
                ResetStartingItemsBit(RI_PROGRESSIVE_BOW);
            } else if (CheckStartingItemsBitset(RI_QUIVER_40)) {
                SetStartingItemsBit(RI_QUIVER_50);
            } else if (CheckStartingItemsBitset(RI_BOW)) {
                SetStartingItemsBit(RI_QUIVER_40);
            } else {
                SetStartingItemsBit(RI_BOW);
                SetStartingItemsBit(RI_PROGRESSIVE_BOW);
            }
            break;
        case RI_PROGRESSIVE_BOMB_BAG:
            if (CheckStartingItemsBitset(RI_BOMB_BAG_40)) {
                ResetStartingItemsBit(RI_BOMB_BAG_40);
                ResetStartingItemsBit(RI_BOMB_BAG_30);
                ResetStartingItemsBit(RI_BOMB_BAG_20);
                ResetStartingItemsBit(RI_BOMBCHU);
                ResetStartingItemsBit(RI_PROGRESSIVE_BOMB_BAG);
            } else if (CheckStartingItemsBitset(RI_BOMB_BAG_30)) {
                SetStartingItemsBit(RI_BOMB_BAG_40);
            } else if (CheckStartingItemsBitset(RI_BOMB_BAG_20)) {
                SetStartingItemsBit(RI_BOMB_BAG_30);
            } else {
                SetStartingItemsBit(RI_BOMB_BAG_20);
                SetStartingItemsBit(RI_BOMBCHU);
                SetStartingItemsBit(RI_PROGRESSIVE_BOMB_BAG);
            }
            break;
        case RI_PROGRESSIVE_MAGIC:
            if (CheckStartingItemsBitset(RI_DOUBLE_MAGIC)) {
                ResetStartingItemsBit(RI_DOUBLE_MAGIC);
                ResetStartingItemsBit(RI_SINGLE_MAGIC);
                ResetStartingItemsBit(RI_PROGRESSIVE_MAGIC);
            } else if (CheckStartingItemsBitset(RI_SINGLE_MAGIC)) {
                SetStartingItemsBit(RI_DOUBLE_MAGIC);
            } else {
                SetStartingItemsBit(RI_SINGLE_MAGIC);
                SetStartingItemsBit(RI_PROGRESSIVE_MAGIC);
            }
            break;
        case RI_PROGRESSIVE_SWORD:
            if (CheckStartingItemsBitset(RI_SWORD_GILDED)) {
                ResetStartingItemsBit(RI_SWORD_GILDED);
                ResetStartingItemsBit(RI_SWORD_RAZOR);
                ResetStartingItemsBit(RI_SWORD_KOKIRI);
                ResetStartingItemsBit(RI_PROGRESSIVE_SWORD);
            } else if (CheckStartingItemsBitset(RI_SWORD_RAZOR)) {
                SetStartingItemsBit(RI_SWORD_GILDED);
            } else if (CheckStartingItemsBitset(RI_SWORD_KOKIRI)) {
                SetStartingItemsBit(RI_SWORD_RAZOR);
            } else {
                SetStartingItemsBit(RI_SWORD_KOKIRI);
                SetStartingItemsBit(RI_PROGRESSIVE_SWORD);
            }
            break;
        case RI_PROGRESSIVE_WALLET:
            if (CheckStartingItemsBitset(RI_WALLET_GIANT)) {
                ResetStartingItemsBit(RI_WALLET_GIANT);
                ResetStartingItemsBit(RI_WALLET_ADULT);
                ResetStartingItemsBit(RI_PROGRESSIVE_WALLET);
            } else if (CheckStartingItemsBitset(RI_WALLET_ADULT)) {
                SetStartingItemsBit(RI_WALLET_GIANT);
            } else {
                SetStartingItemsBit(RI_WALLET_ADULT);
                SetStartingItemsBit(RI_PROGRESSIVE_WALLET);
            }
            break;
        case RI_PROGRESSIVE_LULLABY:
            if (CheckStartingItemsBitset(RI_SONG_LULLABY)) {
                ResetStartingItemsBit(RI_SONG_LULLABY);
                ResetStartingItemsBit(RI_SONG_LULLABY_INTRO);
                ResetStartingItemsBit(RI_PROGRESSIVE_LULLABY);
            } else if (CheckStartingItemsBitset(RI_SONG_LULLABY_INTRO)) {
                SetStartingItemsBit(RI_SONG_LULLABY);
            } else {
                SetStartingItemsBit(RI_SONG_LULLABY_INTRO);
                SetStartingItemsBit(RI_PROGRESSIVE_LULLABY);
            }
            break;
        case RI_SHIELD_HERO:
            if (CheckStartingItemsBitset(RI_SHIELD_MIRROR)) {
                ResetStartingItemsBit(RI_SHIELD_MIRROR);
                ResetStartingItemsBit(RI_SHIELD_HERO);
            } else if (CheckStartingItemsBitset(RI_SHIELD_HERO)) {
                SetStartingItemsBit(RI_SHIELD_MIRROR);
            } else {
                SetStartingItemsBit(RI_SHIELD_HERO);
            }
            break;
        default:
            selectedItemsBitset = GetBitset(randoItemId);
            selectedItemsBitset.flip(item);
            SaveStartingItemsBitset(selectedItemsBitset, randoItemId);
            break;
    }
}

ImVec4 GetBorderColor(RandoItemId randoItemId, bool isChecked) {
    switch (randoItemId) {
        case RI_PROGRESSIVE_BOW:
            if (CheckStartingItemsBitset(RI_QUIVER_50)) {
                return UIWidgets::ColorValues.at(UIWidgets::Colors::Orange);
            } else if (CheckStartingItemsBitset(RI_QUIVER_40)) {
                return UIWidgets::ColorValues.at(UIWidgets::Colors::Purple);
            } else if (CheckStartingItemsBitset(RI_BOW)) {
                return UIWidgets::ColorValues.at(UIWidgets::Colors::White);
            } else {
                return ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
            }
            break;
        case RI_PROGRESSIVE_BOMB_BAG:
            if (CheckStartingItemsBitset(RI_BOMB_BAG_40)) {
                return UIWidgets::ColorValues.at(UIWidgets::Colors::Orange);
            } else if (CheckStartingItemsBitset(RI_BOMB_BAG_30)) {
                return UIWidgets::ColorValues.at(UIWidgets::Colors::Purple);
            } else if (CheckStartingItemsBitset(RI_BOMB_BAG_20)) {
                return UIWidgets::ColorValues.at(UIWidgets::Colors::White);
            } else {
                return ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
            }
            break;
        case RI_PROGRESSIVE_MAGIC:
            if (CheckStartingItemsBitset(RI_DOUBLE_MAGIC)) {
                return UIWidgets::ColorValues.at(UIWidgets::Colors::Purple);
            } else if (CheckStartingItemsBitset(RI_SINGLE_MAGIC)) {
                return UIWidgets::ColorValues.at(UIWidgets::Colors::White);
            } else {
                return ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
            }
            break;
        case RI_PROGRESSIVE_SWORD:
            if (CheckStartingItemsBitset(RI_SWORD_GILDED)) {
                return UIWidgets::ColorValues.at(UIWidgets::Colors::Orange);
            } else if (CheckStartingItemsBitset(RI_SWORD_RAZOR)) {
                return UIWidgets::ColorValues.at(UIWidgets::Colors::Purple);
            } else if (CheckStartingItemsBitset(RI_SWORD_KOKIRI)) {
                return UIWidgets::ColorValues.at(UIWidgets::Colors::White);
            } else {
                return ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
            }
            break;
        case RI_PROGRESSIVE_WALLET:
            if (CheckStartingItemsBitset(RI_WALLET_GIANT)) {
                return UIWidgets::ColorValues.at(UIWidgets::Colors::Purple);
            } else if (CheckStartingItemsBitset(RI_WALLET_ADULT)) {
                return UIWidgets::ColorValues.at(UIWidgets::Colors::White);
            } else {
                return ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
            }
            break;
        case RI_PROGRESSIVE_LULLABY:
            if (CheckStartingItemsBitset(RI_SONG_LULLABY)) {
                return UIWidgets::ColorValues.at(UIWidgets::Colors::Purple);
            } else if (CheckStartingItemsBitset(RI_SONG_LULLABY_INTRO)) {
                return UIWidgets::ColorValues.at(UIWidgets::Colors::White);
            } else {
                return ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
            }
            break;
        case RI_SHIELD_HERO:
            if (CheckStartingItemsBitset(RI_SHIELD_MIRROR)) {
                return UIWidgets::ColorValues.at(UIWidgets::Colors::Purple);
            } else if (CheckStartingItemsBitset(RI_SHIELD_HERO)) {
                return UIWidgets::ColorValues.at(UIWidgets::Colors::White);
            } else {
                return ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
            }
            break;
        default:
            if (isChecked) {
                return UIWidgets::ColorValues.at(UIWidgets::Colors::White);
            } else {
                return ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
            }
            break;
    }
}

static void DrawStartingItemSlot(RandoItemId item, f32 columnWidth, int8_t columnCount) {
    ImGui::PushID(item);
    std::string itemName = Rando::StaticData::Items[item].name;
    ImTextureID textureId = 0;
    const char* texturePath = Rando::StaticData::GetIconTexturePath(item);

    bool checked =
        item == RI_BOMBCHU && CheckStartingItemsBitset(RI_PROGRESSIVE_BOMB_BAG) ? true : CheckStartingItemsBitset(item);
    bool isSong = item >= RI_SONG_ELEGY && item <= RI_SONG_TIME || item == RI_PROGRESSIVE_LULLABY;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
                        isSong ? ImVec2(columnWidth / columnCount * 0.115f, 1.0f) : ImVec2(1.0f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.1f));
    ImGui::PushStyleColor(ImGuiCol_Border, GetBorderColor(item, checked));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 3.0f);

    ImVec2 buttonSize =
        ImVec2((columnWidth / columnCount) * (isSong ? 0.55f : 0.75f), (columnWidth / columnCount) * 0.75f);

    for (auto& progressive : Rando::StaticData::ProgressiveItemsMap) {
        if (progressive.first != item) {
            continue;
        }
        for (auto& upgItem : progressive.second) {
            if (CheckStartingItemsBitset(upgItem)) {
                texturePath = Rando::StaticData::GetIconTexturePath(upgItem);
                break;
            }
        }
    }

    textureId = Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(texturePath);

    if (ImGui::ImageButton(std::to_string(item).c_str(), textureId, buttonSize, ImVec2(0, 0), ImVec2(1, 1),
                           ImVec4(0, 0, 0, 0), iconColor((int16_t)item))) {
        if (item != RI_BOMBCHU) {
            FlipStartingItemsBitset(item);
        }

        Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
    }

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(3);
    UIWidgets::Tooltip(itemName.c_str());
    ImGui::PopID();
}

static void DrawGeneralTab() {
    ImGui::BeginChild("randoSettings", ImVec2(ImGui::GetContentRegionAvail().x / 2, 0));
    ImGui::SeparatorText("Seed Generation");
    UIWidgets::CVarCheckbox("Enable Rando (Randomizes new files upon creation)", "gRando.Enabled");

    if (UIWidgets::CVarCombobox("Seed", "gRando.SpoilerFileIndex", Rando::Spoiler::spoilerOptions)) {
        if (CVarGetInteger("gRando.SpoilerFileIndex", 0) == 0) {
            CVarSetString("gRando.SpoilerFile", "");
        } else {
            CVarSetString("gRando.SpoilerFile",
                          Rando::Spoiler::spoilerOptions[CVarGetInteger("gRando.SpoilerFileIndex", 0)].c_str());
        }
    }

    if (CVarGetInteger("gRando.SpoilerFileIndex", 0) == 0) {
        UIWidgets::PushStyleSlider();
        static char seed[256];
        std::string stringSeed = CVarGetString("gRando.InputSeed", "");
        strcpy(seed, stringSeed.c_str());
        ImGui::InputText("##Seed", seed, sizeof(seed), ImGuiInputTextFlags_CallbackAlways,
                         [](ImGuiInputTextCallbackData* data) {
                             CVarSetString("gRando.InputSeed", data->Buf);
                             return 0;
                         });
        if (stringSeed.length() < 1) {
            ImGui::SameLine(17.0f);
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.4f), "Leave blank for random seed");
        }
        UIWidgets::PopStyleSlider();

        UIWidgets::CVarCheckbox("Generate Spoiler File", "gRando.GenerateSpoiler");
    }
    ImGui::SeparatorText("Enhancements");
    UIWidgets::CVarCheckbox("Container Style Matches Contents", "gRando.CSMC");
    UIWidgets::Tooltip("This will make the contents of a container match the container itself. This currently only "
                       "applies to chests and pots");
    UIWidgets::WindowButton("Check Tracker", "gWindows.CheckTracker", BenGui::mRandoCheckTrackerWindow,
                            { .size = ImVec2((ImGui::GetContentRegionAvail().x - 48.0f), 40.0f) });
    ImGui::SameLine();
    if (UIWidgets::Button(ICON_FA_COG, { .size = ImVec2(40.0f, 40.0f) })) {
        BenGui::mRandoCheckTrackerSettingsWindow->ToggleVisibility();
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("randoDisclaimer");
    ImGui::PushStyleColor(ImGuiCol_Text, ColorValues.at(Colors::Gray));
    if (gGitCommitTag[0] == 0) {
        ImGui::Text("%s | %s", (char*)gGitBranch, (char*)gGitCommitHash);
    } else {
        ImGui::Text("%s", (char*)gBuildVersion);
    }
    ImGui::PopStyleColor();
    ImGui::PushStyleColor(ImGuiCol_Text, ColorValues.at(Colors::Orange));
    ImGui::SeparatorText("Disclaimer");
    ImGui::PopStyleColor();
    ImGui::TextWrapped(
        "This is an Alpha. Please make note of any odd or unexpected behavior while you are playing. While we are in "
        "the earlier phases of this project, some things you may encounter are:\n- X Check is not shuffled\n- X "
        "Cutscene is not skipped\n- Soft lock when interacting with X\n- Unbeatable seed (glitchless logic)\n\nWe are "
        "aware of some of these, but likely not all. Please compare your findings to our list of known issues, which "
        "is available in the pins of the Rando Alpha Discord thread, or on the Github Issue #211, and let us know if "
        "you encounter any new issues.\n\nExplore the menus for various enhancements and time savers, they are not "
        "enabled by default in Rando.\n\n");
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.5f, 0.5f, 1.0f));
    ImGui::SeparatorText("Thank You");
    ImGui::PopStyleColor();
    ImGui::TextWrapped("Special thanks to BalloonDude, Eblo, Caladius, Sitton, Dana, our playtesters, everyone who "
                       "contributed to the SoH randomizer, and the creators of the various other randomizer "
                       "implementations that inspired this project. I hope you enjoy it.\n\n");
    ImTextureID swordTextureId = Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
        (const char*)gQuestIconHeartContainer2Tex);
    ImGui::Image(swordTextureId, ImVec2(25.0f, 25.0f));
    ImGui::SameLine();
    ImGui::Text("ProxySaw");
    ImGui::EndChild();
}

static void DrawLogicConditionsTab() {
    f32 columnWidth = ImGui::GetContentRegionAvail().x / 3 - (ImGui::GetStyle().ItemSpacing.x * 2);
    f32 halfHeight = ImGui::GetContentRegionAvail().y / 2 - (ImGui::GetStyle().ItemSpacing.y * 2);
    ImGui::BeginChild("randoLogicColumn1", ImVec2(columnWidth, halfHeight));
    UIWidgets::CVarCombobox("Logic", Rando::StaticData::Options[RO_LOGIC].cvar, logicOptions);
    UIWidgets::Tooltip(
        "Glitchless - The items are shuffled in a way that guarantees the seed is beatable without "
        "glitches\n\n"
        "No Logic - The items are shuffled completely randomly, this can result in unbeatable seeds, and "
        "will require heavy use of glitches\n\n"
        "Nearly No Logic - The items are shuffled completely randomly, with the following exceptions:\n"
        "- Oath to Order and Remains cannot be placed on the Moon\n"
        "- Deku Mask, Zora Mask, Sonata, and Bossa Nova cannot be placed in their respective Temples or on "
        "the Moon\n\n"
        "French Vanilla - This is an alternative variant to Glitchless, but the items are biased to be "
        "closer to their vanilla locations. Tends to be an more beginner friendly experience.\n\n"
        "Vanilla - The items are not shuffled.");
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("randoLogicColumn2", ImVec2(columnWidth, halfHeight));
    UIWidgets::CVarCombobox("Dungeon Access", Rando::StaticData::Options[RO_ACCESS_DUNGEONS].cvar,
                            accessDungeonOptions);
    UIWidgets::CVarSliderInt("Majora Access Remains Required",
                             Rando::StaticData::Options[RO_ACCESS_MAJORA_REMAINS_COUNT].cvar,
                             IntSliderOptions().Min(0).Max(4).DefaultValue(0));
    UIWidgets::CVarSliderInt("Moon Access Remains Required",
                             Rando::StaticData::Options[RO_ACCESS_MOON_REMAINS_COUNT].cvar,
                             IntSliderOptions().Min(0).Max(4).DefaultValue(4));
    UIWidgets::CVarCombobox("Trials Access", Rando::StaticData::Options[RO_ACCESS_TRIALS].cvar, accessTrialsOptions);
    ImGui::EndChild();
    ImGui::BeginChild("randoLogicTricks", ImVec2(0, 0));
    ImGui::SeparatorText("Tricks & Glitches");
    ImGui::EndChild();
}

static void DrawLocationsTab() {
    f32 columnWidth = ImGui::GetContentRegionAvail().x / 3 - (ImGui::GetStyle().ItemSpacing.x * 2);
    f32 halfHeight = ImGui::GetContentRegionAvail().y / 2 - (ImGui::GetStyle().ItemSpacing.y * 2);
    ImGui::SeparatorText("Shuffle Options");
    ImGui::BeginChild("randoLocationsColumn1", ImVec2(columnWidth, halfHeight));
    CVarCheckbox("Shuffle Songs", "gPlaceholderBool",
                 CheckboxOptions({ { .disabled = true, .disabledTooltip = "Coming Soon" } }).DefaultValue(true));
    CVarCheckbox("Shuffle Owl Statues", Rando::StaticData::Options[RO_SHUFFLE_OWL_STATUES].cvar);
    CVarCheckbox("Shuffle Shops", Rando::StaticData::Options[RO_SHUFFLE_SHOPS].cvar);
    CVarCheckbox("Shuffle Tingle Maps", Rando::StaticData::Options[RO_SHUFFLE_TINGLE_SHOPS].cvar);
    CVarCheckbox("Shuffle Boss Remains", Rando::StaticData::Options[RO_SHUFFLE_BOSS_REMAINS].cvar);
    CVarCheckbox("Shuffle Cows", Rando::StaticData::Options[RO_SHUFFLE_COWS].cvar);
    CVarCheckbox("Shuffle Gold Skulltula Tokens", Rando::StaticData::Options[RO_SHUFFLE_GOLD_SKULLTULAS].cvar);
    CVarSliderInt(
        "Shuffle Gold Skulltula Tokens", "gPlaceholderInt",
        IntSliderOptions({ { .disabled = true, .disabledTooltip = "Coming Soon" } }).Min(1).Max(30).DefaultValue(30));
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("randoLocationsColumn2", ImVec2(columnWidth, halfHeight));
    CVarCheckbox("Shuffle Pot Drops", Rando::StaticData::Options[RO_SHUFFLE_POT_DROPS].cvar);
    CVarCheckbox("Shuffle Crate Drops", Rando::StaticData::Options[RO_SHUFFLE_CRATE_DROPS].cvar);
    CVarCheckbox("Shuffle Barrel Drops", Rando::StaticData::Options[RO_SHUFFLE_BARREL_DROPS].cvar);
    CVarCheckbox("Shuffle Hive Drops", "gPlaceholderBool",
                 CheckboxOptions({ { .disabled = true, .disabledTooltip = "Coming Soon" } }));
    CVarCheckbox("Shuffle Freestanding Items", Rando::StaticData::Options[RO_SHUFFLE_FREESTANDING_ITEMS].cvar);
    CVarCheckbox("Shuffle Wonder Items", "gPlaceholderBool",
                 CheckboxOptions({ { .disabled = true, .disabledTooltip = "Coming Soon" } }));
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("randoLocationsColumn3", ImVec2(columnWidth, halfHeight));
    ImGui::EndChild();
    ImGui::BeginChild("randoLocationsExclusions", ImVec2(0, 0));
    ImGui::SeparatorText("Exclusions");
    ImGui::TextWrapped("These checks will be gauranteed junk items, and marked as skipped in the check tracker.");
    ImGui::EndChild();
}

static void DrawItemsTab() {
    f32 columnWidth = ImGui::GetContentRegionAvail().x / 3 - (ImGui::GetStyle().ItemSpacing.x * 2);
    f32 halfHeight = ImGui::GetContentRegionAvail().y / 3 - (ImGui::GetStyle().ItemSpacing.y * 2);
    ImGui::BeginChild("randoItemsColumn1", ImVec2(columnWidth, halfHeight));
    CVarCheckbox("Bronze Scale", "gPlaceholderBool",
                 CheckboxOptions({ { .disabled = true, .disabledTooltip = "Coming Soon" } }));
    CVarCheckbox("Deku Stick Bag", "gPlaceholderBool",
                 CheckboxOptions({ { .disabled = true, .disabledTooltip = "Coming Soon" } }));
    CVarCheckbox("Deku Nut Bag", "gPlaceholderBool",
                 CheckboxOptions({ { .disabled = true, .disabledTooltip = "Coming Soon" } }));
    CVarCheckbox("Skeleton Key", "gPlaceholderBool",
                 CheckboxOptions({ { .disabled = true, .disabledTooltip = "Coming Soon" } }));
    CVarCheckbox("Child Wallet", "gPlaceholderBool",
                 CheckboxOptions({ { .disabled = true, .disabledTooltip = "Coming Soon" } }));
    CVarCheckbox("Infinite Upgrades", "gPlaceholderBool",
                 CheckboxOptions({ { .disabled = true, .disabledTooltip = "Coming Soon" } }));
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("randoItemsColumn2", ImVec2(columnWidth, halfHeight));
    CVarCheckbox("Plentiful Items", Rando::StaticData::Options[RO_PLENTIFUL_ITEMS].cvar);
    CVarCheckbox("Boss Souls", Rando::StaticData::Options[RO_SHUFFLE_BOSS_SOULS].cvar);
    CVarCheckbox("Enemy Souls", "gPlaceholderBool",
                 CheckboxOptions({ { .disabled = true, .disabledTooltip = "Coming Soon" } }));
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("randoItemsColumn3", ImVec2(columnWidth, halfHeight));
    CVarCheckbox("Song of Double Time", "gPlaceholderBool",
                 CheckboxOptions({ { .disabled = true, .disabledTooltip = "Coming Soon" } }));
    CVarCheckbox("Inverted Song of Time", "gPlaceholderBool",
                 CheckboxOptions({ { .disabled = true, .disabledTooltip = "Coming Soon" } }));
    CVarCheckbox("Saria's Song", "gPlaceholderBool",
                 CheckboxOptions({ { .disabled = true, .disabledTooltip = "Coming Soon" } }));
    CVarCheckbox("Sun's Song", "gPlaceholderBool",
                 CheckboxOptions({ { .disabled = true, .disabledTooltip = "Coming Soon" } }));
    ImGui::EndChild();
    ImGui::BeginChild("randoItemsStarting", ImVec2(0, 0));
    ImGui::BeginChild("randoStartingItemsColumn1", ImVec2(columnWidth, 0));
    ImGui::SeparatorText("Starting Options");
    CVarCheckbox("Wallet Full", Rando::StaticData::Options[RO_STARTING_RUPEES].cvar);
    CVarCheckbox("Consumables Full", Rando::StaticData::Options[RO_STARTING_CONSUMABLES].cvar);
    CVarCheckbox("Maps and Compasses", Rando::StaticData::Options[RO_STARTING_MAPS_AND_COMPASSES].cvar);
    CVarSliderInt("Health", Rando::StaticData::Options[RO_STARTING_HEALTH].cvar,
                  IntSliderOptions()
                      .Min(1)
                      .Max(20)
                      .DefaultValue(3)
                      .LabelPosition(LabelPosition::None)
                      .Format("%d Hearts")
                      .Color(Colors::Red));
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("randoStartingItemsColumn2",
                      ImVec2(ImGui::GetContentRegionAvail().x - (ImGui::GetStyle().ItemSpacing.x * 2), 0));
    ImGui::SeparatorText("Available Items");
    if (ImGui::BeginTable("Starting Items Table", 2, 0,
                          ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y))) {
        ImGui::TableNextColumn();
        for (auto& category : Rando::StaticData::StartingItemsMap) {
            ImGui::BeginChild(std::to_string(category.first).c_str(), ImVec2(0, 0),
                              ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AutoResizeX);
            int uncheckedItemIndex = 0;
            int8_t columnCount = startingItemCategoryLabels[category.first].second;

            for (auto& item : Rando::StaticData::StartingItemsMap[category.first]) {
                if (std::find(Rando::StaticData::StartingItemsExclusions.begin(),
                              Rando::StaticData::StartingItemsExclusions.end(),
                              item) != Rando::StaticData::StartingItemsExclusions.end()) {
                    continue;
                }
                DrawStartingItemSlot(item, columnWidth, columnCount);
                uncheckedItemIndex++;

                if (uncheckedItemIndex % columnCount != 0) {
                    ImGui::SameLine(0, 7.0f);
                } else {
                    ImGui::Dummy(ImVec2(0, 0));
                }
            }
            ImGui::EndChild();
            ImGui::TableNextColumn();
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();
    ImGui::EndChild();
}

static void DrawHintsTab() {
    f32 columnWidth = ImGui::GetContentRegionAvail().x / 3 - (ImGui::GetStyle().ItemSpacing.x * 2);
    f32 halfHeight = ImGui::GetContentRegionAvail().y / 2 - (ImGui::GetStyle().ItemSpacing.y * 2);
    ImGui::BeginChild("randoHintsColumn1", ImVec2(columnWidth, halfHeight));
    CVarCheckbox(
        "Spider House", Rando::StaticData::Options[RO_HINTS_SPIDER_HOUSES].cvar,
        CheckboxOptions(
            { { .tooltip =
                    "Swamp Spider House: Hinted at his normal location within the Swamp Spider House\n\nOcean Spider "
                    "House: Hinted in South Clock Town day 1, by the main standing on the scaffolding." } }));
    CVarCheckbox(
        "Gossip Stone Static Hint", Rando::StaticData::Options[RO_HINTS_GOSSIP_STONES].cvar,
        CheckboxOptions(
            { { .tooltip = "Each gossip stone will give a static hint about the contents of a random location." } }));
    CVarCheckbox(
        "Gossip Stone Purchaseable", Rando::StaticData::Options[RO_HINTS_PURCHASEABLE].cvar,
        CheckboxOptions({ { .tooltip = "Gossip stones will offer a hint for a scaling rupee cost. This cost ranges "
                                       "from 10-250 rupees depending on how many checks are remaining in your seed. "
                                       "The hint will gauranteed be a check you have not obtained yet." } }));
    CVarCheckbox(
        "Boss Remains", Rando::StaticData::Options[RO_HINTS_BOSS_REMAINS].cvar,
        CheckboxOptions(
            { { .tooltip =
                    "Lists the location of the Boss remains on the guard recruitment posters around Clock Town" } }));
    CVarCheckbox("Oath to Order", Rando::StaticData::Options[RO_HINTS_OATH_TO_ORDER].cvar,
                 CheckboxOptions({ { .tooltip = "Once you have the Moon Access Requirments, talking to Skull Kid on "
                                                "the Clock Tower Rooftop will hint the location of Oath to Order" } }));
    CVarCheckbox(
        "General Actor Hints", "gPlaceholderBool",
        CheckboxOptions({ { .disabled = true,
                            .disabledTooltip = "Soon you will be able to disable these. Currently hinted:\n- Bomb Shop "
                                               "4th Item\n- Lottery\n- Great Fairy Fountains\n- Mountain Smithy" } })
            .DefaultValue(true));
    CVarCheckbox("Saria's Song", "gPlaceholderBool",
                 CheckboxOptions({ { .disabled = true, .disabledTooltip = "Coming Soon" } }));
    CVarCheckbox("Song of Soaring", "gPlaceholderBool",
                 CheckboxOptions({ { .disabled = true, .disabledTooltip = "Coming Soon" } }));
    CVarCheckbox(
        "Hookshot Location", Rando::StaticData::Options[RO_HINTS_HOOKSHOT].cvar,
        CheckboxOptions(
            { { .tooltip =
                    "The Zora in Great Bay Coast, near Pirates Fortress, will hint the location of the Hookshot." } }));
    ImGui::EndChild();
}

void Rando::RegisterMenu() {
    mBenMenu->AddSidebarEntry("Rando", "General", 1);
    WidgetPath path = { "Rando", "General", 1 };
    mBenMenu->AddWidget(path, "General", WIDGET_CUSTOM).CustomFunction([](WidgetInfo& info) { DrawGeneralTab(); });
    mBenMenu->AddSidebarEntry("Rando", "Logic/Conditions", 1);
    path.sidebarName = "Logic/Conditions";
    mBenMenu->AddWidget(path, "Logic/Conditions", WIDGET_CUSTOM).CustomFunction([](WidgetInfo& info) {
        DrawLogicConditionsTab();
    });
    mBenMenu->AddSidebarEntry("Rando", "Locations", 1);
    path.sidebarName = "Locations";
    mBenMenu->AddWidget(path, "Locations", WIDGET_CUSTOM).CustomFunction([](WidgetInfo& info) { DrawLocationsTab(); });
    mBenMenu->AddSidebarEntry("Rando", "Items", 1);
    path.sidebarName = "Items";
    mBenMenu->AddWidget(path, "Items", WIDGET_CUSTOM).CustomFunction([](WidgetInfo& info) { DrawItemsTab(); });
    mBenMenu->AddSidebarEntry("Rando", "Hints", 1);
    path.sidebarName = "Hints";
    mBenMenu->AddWidget(path, "Hints", WIDGET_CUSTOM).CustomFunction([](WidgetInfo& info) { DrawHintsTab(); });
}
