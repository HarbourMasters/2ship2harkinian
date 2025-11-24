#include "QuickStart.h"
#include "UIWidgets.hpp"
#include "BenMenu.h"
#include "ship/config/Config.h"
#include "PresetManager/PresetManager.h"
#include "SaveManager/SaveManager.h"

extern "C" {
#include "functions.h"
#include "variables.h"
#include "overlays/gamestates/ovl_file_choose/z_file_select.h"
void FileSelect_LoadGame(GameState* thisx);
}

/*
This file is dedicated to making the process simpler for folks to boot up a fresh or existing download.
The goal is to provide a small set of questions and have the player select what they want to do, in the
end our backend code will handle setting their options appropriately before creating and throwing them
right into a file.

                            Workflow is currently as follows:
                                - Ask if Vanilla or Rando
                         Vanilla                         Rando
                     - Ask for Preset choice         - Ask for Preset choice
                                                     - Ask for Logic choice
                                                     - Prompt for Shuffle Options
                                - Goto Start sidebar
                                - Enter Name for file
                                - Click Let's Go to Start

Please keep the flow simple and easy for the user. If you plan to add more to this, keep simplicity in mind.
*/

// ImVec4 Colors
#define COLOR_WHITE UIWidgets::Colors::White
#define COLOR_GREY UIWidgets::Colors::Gray
#define COLOR_GREEN UIWidgets::Colors::Green
#define COLOR_ORANGE UIWidgets::Colors::Orange
#define COLOR_RED UIWidgets::Colors::Red

#define TEXT_COLOR(color) UIWidgets::ColorValues.at(color)

QuickStartSelection quickStartOptions = { .questID = QUICK_START_NONE,
                                          .presetOption = QUICK_START_NONE,
                                          .rando = { .shuffleSet = QUICK_START_NONE,
                                                     .logicOption = QUICK_START_NONE } };

namespace BenGui {
extern std::shared_ptr<BenMenu> mBenMenu;
extern std::shared_ptr<Ship::QuickStart> mQuickStartMenu;
} // namespace BenGui

using namespace BenGui;
namespace Ship {

static int LimitFileNameLength(ImGuiInputTextCallbackData* data) {
    const int maxChars = 8;
    if (data->BufTextLen > maxChars) {
        data->Buf[maxChars] = '\0';
        data->BufDirty = true;
        data->CursorPos = maxChars;
    }
    return 0;
}

std::vector<uint8_t> ConvertNameSet(const char* input) {
    std::vector<uint8_t> result;
    if (!input) {
        return result;
    }

    while (*input) {
        char c = *input++;
        if (c >= 'A' && c <= 'Z') {
            result.push_back(D_808141F0[c - 'A']);
            continue;
        }
        if (c >= 'a' && c <= 'z') {
            result.push_back(D_808141F0[26 + (c - 'a')]);
            continue;
        }
        if (c >= '1' && c <= '9') {
            result.push_back(D_808141F0[52 + (c - '1')]);
            continue;
        }
        if (c == '0') {
            result.push_back(D_808141F0[52 + 9]);
            continue;
        }
        switch (c) {
            case '.':
                result.push_back(D_808141F0[62]);
                continue;
            case '-':
                result.push_back(D_808141F0[63]);
                continue;
            case ' ':
                result.push_back(D_808141F0[64]);
                continue;
        }
        result.push_back(0xFF);
    }

    return result;
}

void CenterText(const char* text) {
    ImVec2 textSize = ImGui::CalcTextSize(text);

    float windowWidth = ImGui::GetWindowSize().x;
    float textX = (windowWidth - textSize.x) * 0.5f;

    ImGui::SetCursorPosX(textX);
    ImGui::TextColored(TEXT_COLOR(COLOR_WHITE), "%s", text);
}

void CenterFullWidthSeparatorText(const char* text, std::string type) {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 textSize = ImGui::CalcTextSize(text);
    ImVec2 region;
    ImVec2 textPos;

    if (type == "Full") {
        region = ImVec2(viewport->WorkPos.x, viewport->WorkPos.x + viewport->WorkSize.x);
        textPos = ImVec2(region.x + (viewport->WorkSize.x - textSize.x) * 0.5f, ImGui::GetCursorScreenPos().y);
    } else {
        ImVec2 cursor = ImGui::GetCursorScreenPos();
        float regionWidth = ImGui::GetContentRegionAvail().x;
        region = ImVec2(cursor.x, cursor.x + regionWidth);
        textPos = ImVec2(region.x + (regionWidth - textSize.x) * 0.5f, cursor.y);
    }

    float spacing = ImGui::GetStyle().ItemSpacing.y;
    float lineY = textPos.y + textSize.y * 0.5f;
    ImU32 lineColor = IM_COL32(255, 255, 255, 255);

    drawList->AddLine(ImVec2(region.x, lineY), ImVec2(textPos.x - spacing, lineY), lineColor, 2.0f);
    drawList->AddLine(ImVec2(textPos.x + textSize.x + spacing, lineY), ImVec2(region.y, lineY), lineColor, 2.0f);

    ImGui::SetCursorScreenPos(ImVec2(textPos.x, textPos.y));
    ImGui::TextColored(TEXT_COLOR(COLOR_WHITE), text);
}

void SetPresetOption(QuickStartSettings option) {
    if (option != QUICK_START_NONE) {
        PresetManager_ApplyPreset(option == QUICK_START_PRESET_DEFAULT ? defaultsPresetJ : vanillaEnhancedPresetJ);
        Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
    }
}

void SetRandoQuickStartOptions() {
    Ship::Context::GetInstance()->GetConsoleVariables()->ClearBlock("gRando.Options");

    // Logic
    CVarSetInteger(Rando::StaticData::Options[RO_LOGIC].cvar,
                   quickStartOptions.rando.logicOption == QUICK_START_RANDO_GLITCHLESS ? RO_LOGIC_GLITCHLESS
                                                                                       : RO_LOGIC_NO_LOGIC);

    // Hints
    CVarSetInteger(Rando::StaticData::Options[RO_HINTS_GOSSIP_STONES].cvar, 1);
    CVarSetInteger(Rando::StaticData::Options[RO_HINTS_BOSS_REMAINS].cvar, 1);
    CVarSetInteger(Rando::StaticData::Options[RO_HINTS_OATH_TO_ORDER].cvar, 1);
    CVarSetInteger(Rando::StaticData::Options[RO_HINTS_HOOKSHOT].cvar, 1);

    // Base Game Shuffles
    CVarSetInteger(Rando::StaticData::Options[RO_SHUFFLE_OWL_STATUES].cvar, 1);
    CVarSetInteger(Rando::StaticData::Options[RO_SHUFFLE_SHOPS].cvar, 1);
    CVarSetInteger(Rando::StaticData::Options[RO_SHUFFLE_TINGLE_SHOPS].cvar, 1);
    CVarSetInteger(Rando::StaticData::Options[RO_SHUFFLE_GOLD_SKULLTULAS].cvar, 1);
    CVarSetInteger(Rando::StaticData::Options[RO_MINIMUM_STRAY_FAIRIES].cvar, 15);
    CVarSetInteger(Rando::StaticData::Options[RO_SHUFFLE_BOSS_REMAINS].cvar, 1);

    if (quickStartOptions.rando.shuffleSet == QUICK_START_RANDO_ALL) {
        CVarSetInteger(Rando::StaticData::Options[RO_SHUFFLE_POT_DROPS].cvar, 1);
        CVarSetInteger(Rando::StaticData::Options[RO_SHUFFLE_CRATE_DROPS].cvar, 1);
        CVarSetInteger(Rando::StaticData::Options[RO_SHUFFLE_BARREL_DROPS].cvar, 1);
        CVarSetInteger(Rando::StaticData::Options[RO_SHUFFLE_SNOWBALL_DROPS].cvar, 1);
        CVarSetInteger(Rando::StaticData::Options[RO_SHUFFLE_GRASS_DROPS].cvar, 1);
        CVarSetInteger(Rando::StaticData::Options[RO_SHUFFLE_FREESTANDING_ITEMS].cvar, 1);
        CVarSetInteger(Rando::StaticData::Options[RO_SHUFFLE_BOSS_SOULS].cvar, 1);
    }

    Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
}

void QuickStartQuestInit(const char* fileNameEntry) {
    static std::string cleanFileName = Ship_RemoveSpecialCharacters(fileNameEntry);
    std::vector<uint8_t> playerName = ConvertNameSet(cleanFileName.c_str());
    FileSelectState* fileSelect = (FileSelectState*)gGameState;
    SramContext* sramCtx = &fileSelect->sramCtx;

    fileSelect->buttonIndex = (SelectMenuButtonIndex)SaveManager_GetOpenFileSlot() - 1;
    if (fileSelect->buttonIndex < FS_BTN_SELECT_FILE_1) {
        Audio_PlaySfx(NA_SE_SY_QUIZ_INCORRECT);
        return;
    }

    Audio_PlaySfx(NA_SE_SY_QUIZ_CORRECT);
    gSaveContext.fileNum = fileSelect->buttonIndex;

    s32 nameIndex;
    for (nameIndex = 0; nameIndex < ARRAY_COUNT(gSaveContext.save.saveInfo.playerData.playerName); nameIndex++) {
        if (nameIndex < playerName.size()) {
            fileSelect->fileNames[fileSelect->buttonIndex][nameIndex] = playerName[nameIndex];
        } else {
            fileSelect->fileNames[fileSelect->buttonIndex][nameIndex] = 0x3E;
        }
    }

    Sram_InitSave(fileSelect, sramCtx);
    gSaveContext.save.time = CURRENT_TIME;

    if (!gSaveContext.flashSaveAvailable) {
        fileSelect->configMode = CM_NAME_ENTRY_TO_MAIN;
    } else {
        Sram_SetFlashPagesDefault(sramCtx, gFlashSaveStartPages[fileSelect->buttonIndex * FLASH_SAVE_MAIN_MULTIPLIER],
                                  gFlashSpecialSaveNumPages[fileSelect->buttonIndex * FLASH_SAVE_MAIN_MULTIPLIER]);
        Sram_StartWriteToFlashDefault(sramCtx);
        fileSelect->configMode = CM_NAME_ENTRY_WAIT_FOR_FLASH_SAVE;
    }

    fileSelect->nameBoxAlpha[fileSelect->buttonIndex] = fileSelect->nameAlpha[fileSelect->buttonIndex] = 200;
    fileSelect->connectorAlpha[fileSelect->buttonIndex] = 255;

    FileSelect_LoadGame(gGameState);
    mQuickStartMenu->Hide();
}

void DrawQuickStartHeader() {
    CenterFullWidthSeparatorText("Welcome to the Quick Start Section", "Full");
    CenterText("This section will ask you a series of questions about what you are looking to do.\n"
               "Once complete you will be brought straight into the game with settings that match\n"
               "the options you selected.");
}

void DrawQuickStartQuestSelect() {
    CenterFullWidthSeparatorText("Which Quest are you feeling today?", "Half");
    if (ImGui::BeginTable("QuestSelect", 2, ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableNextColumn();
        if (UIWidgets::Button("Vanilla",
                              { .color = quickStartOptions.questID == SAVETYPE_VANILLA ? COLOR_GREEN : COLOR_GREY })) {
            quickStartOptions.questID = SAVETYPE_VANILLA;
        }
        ImGui::TableNextColumn();
        if (UIWidgets::Button("Randomizer",
                              { .color = quickStartOptions.questID == SAVETYPE_RANDO ? COLOR_GREEN : COLOR_GREY })) {
            quickStartOptions.questID = SAVETYPE_RANDO;
        }
        ImGui::EndTable();
    }
}

void DrawQuickStartQuestOptions() {
    CenterFullWidthSeparatorText("Choose your preferred experience", "Half");
    if (ImGui::BeginTable("ExperienceTable", 3, ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableNextColumn();
        if (UIWidgets::Button(
                "Default N64",
                { .color = quickStartOptions.presetOption == QUICK_START_PRESET_DEFAULT ? COLOR_GREEN : COLOR_GREY })) {
            quickStartOptions.presetOption = QUICK_START_PRESET_DEFAULT;
        }
        ImGui::TableNextColumn();
        if (UIWidgets::Button(
                "Current Settings",
                { .color = quickStartOptions.presetOption == QUICK_START_NONE ? COLOR_GREEN : COLOR_GREY })) {
            quickStartOptions.presetOption = QUICK_START_NONE;
        }
        ImGui::TableNextColumn();
        if (UIWidgets::Button("Enhanced", { .color = quickStartOptions.presetOption == QUICK_START_PRESET_ENHANCED
                                                         ? COLOR_GREEN
                                                         : COLOR_GREY })) {
            quickStartOptions.presetOption = QUICK_START_PRESET_ENHANCED;
        }
        ImGui::EndTable();
    }
    if (UIWidgets::Button("Apply", { .size = ImVec2(ImGui::GetContentRegionAvail().x, 0), .color = COLOR_GREEN })) {
        if (quickStartOptions.presetOption != QUICK_START_NONE) {
            SetPresetOption(quickStartOptions.presetOption);
        }
    }
    UIWidgets::Separator();
    CenterFullWidthSeparatorText("What playstyle for the Quest?", "Half");
    switch (quickStartOptions.questID) {
        case SAVETYPE_VANILLA:
            ImGui::Text("Vanilla Selected");
            ImGui::Text("No additional selections required.");
            break;
        case SAVETYPE_RANDO:
            if (ImGui::BeginTable("RandoOptions", 2, ImGuiTableFlags_SizingStretchSame)) {
                ImGui::TableNextColumn();
                if (UIWidgets::Button("Shuffle only the Base Game",
                                      { .color = quickStartOptions.rando.shuffleSet == QUICK_START_RANDO_BASE
                                                     ? COLOR_GREEN
                                                     : COLOR_GREY })) {
                    quickStartOptions.rando.shuffleSet = QUICK_START_RANDO_BASE;
                }
                ImGui::TableNextColumn();
                if (UIWidgets::Button("Shuffle Advanced Options",
                                      { .color = quickStartOptions.rando.shuffleSet == QUICK_START_RANDO_ALL
                                                     ? COLOR_GREEN
                                                     : COLOR_GREY })) {
                    quickStartOptions.rando.shuffleSet = QUICK_START_RANDO_ALL;
                }
                ImGui::EndTable();
            }
            UIWidgets::Separator();
            CenterFullWidthSeparatorText("Glitchless or No Logic?", "Half");
            if (ImGui::BeginTable("RandoOptions2", 2, ImGuiTableFlags_SizingStretchSame)) {
                ImGui::TableNextColumn();
                if (UIWidgets::Button("Glitchless",
                                      { .color = quickStartOptions.rando.logicOption == QUICK_START_RANDO_GLITCHLESS
                                                     ? COLOR_GREEN
                                                     : COLOR_GREY })) {
                    quickStartOptions.rando.logicOption = QUICK_START_RANDO_GLITCHLESS;
                }
                ImGui::TableNextColumn();
                if (UIWidgets::Button("No Logic",
                                      { .color = quickStartOptions.rando.logicOption == QUICK_START_RANDO_NO_LOGIC
                                                     ? COLOR_GREEN
                                                     : COLOR_GREY })) {
                    quickStartOptions.rando.logicOption = QUICK_START_RANDO_NO_LOGIC;
                }
                ImGui::EndTable();
            }
            break;
        default:
            ImGui::TextColored(TEXT_COLOR(COLOR_ORANGE), "No Quest Selected...");
            break;
    }
}

void DrawQuickStartQuestReview() {
    std::string reviewText = "";
    if (quickStartOptions.questID == QUICK_START_NONE ||
        (quickStartOptions.questID == SAVETYPE_RANDO && (quickStartOptions.rando.shuffleSet == QUICK_START_NONE ||
                                                         quickStartOptions.rando.logicOption == QUICK_START_NONE))) {
        ImGui::TextColored(TEXT_COLOR(COLOR_ORANGE), "Select the Options on the left to see what changes.");
        return;
    }
    SelectMenuButtonIndex fileCheck = (SelectMenuButtonIndex)SaveManager_GetOpenFileSlot();
    if (fileCheck - 1 < 0) {
        ImGui::TextColored(TEXT_COLOR(COLOR_ORANGE), "No File slot available, please delete a File first.");
        if (ImGui::BeginTable("EraseFileButtons", 3, ImGuiTableFlags_SizingStretchSame)) {
            for (int i = 0; i <= FS_BTN_SELECT_FILE_3; i++) {
                ImGui::TableNextColumn();
                std::string buttonLabel = "Delete File ";
                buttonLabel += std::to_string(i).c_str();
                if (UIWidgets::Button(buttonLabel.c_str(), { .color = COLOR_RED })) {
                    FileSelectState* fileSelectState = (FileSelectState*)gGameState;
                    Sram_EraseSave(fileSelectState, &fileSelectState->sramCtx, i);
                    Sram_SetFlashPagesDefault(&fileSelectState->sramCtx,
                                              gFlashSaveStartPages[i * FLASH_SAVE_MAIN_MULTIPLIER],
                                              gFlashSpecialSaveNumPages[i * FLASH_SAVE_MAIN_MULTIPLIER]);
                    Sram_StartWriteToFlashDefault(&fileSelectState->sramCtx);
                    fileSelectState->configMode = CM_ERASE_WAIT_FOR_FLASH_SAVE;
                }
            }
            ImGui::EndTable();
        }
        return;
    }
    CenterFullWidthSeparatorText("Here's what to expect", "Half");
    switch (quickStartOptions.questID) {
        case SAVETYPE_VANILLA:
            reviewText = "The Vanilla experience:\n"
                         "- Save Termina or get lost in the side quests.";
            ImGui::Text(reviewText.c_str());
            break;
        case SAVETYPE_RANDO:
            ImGui::Text(quickStartOptions.rando.logicOption == QUICK_START_RANDO_GLITCHLESS
                            ? "Glitchless Logic Selected"
                            : "No Logic Selected");
            UIWidgets::Separator();
            ImGui::TextColored(TEXT_COLOR(COLOR_GREEN), "The following are shuffled into your seed:");
            reviewText = "- Chests\n"
                         "- Piece of Heart & Heart Containers\n"
                         "- NPC Rewards\n"
                         "- Shops & Tingle Maps\n"
                         "- Gold Skulltula Tokens & Stray Fairies";
            if (quickStartOptions.rando.shuffleSet == QUICK_START_RANDO_ALL) {
                reviewText += "\n"
                              "- Pots, Crates, Barrels, & Grass\n"
                              "- Snowballs\n"
                              "- Freestanding Items\n"
                              "- Boss Souls";
            }
            if (ImGui::BeginChild("SeedDetails")) {
                ImGui::Text(reviewText.c_str());
                ImGui::TextColored(TEXT_COLOR(COLOR_GREEN), "Plentiful Items is also enabled, this will shuffle\n"
                                                            "additional copies of Progression Items into the pool.");
                UIWidgets::Separator();
                ImGui::Text("There are plenty of other options within the Menus.\n"
                            "Use the Search at the top if you know what you are\n"
                            "looking for. Otherwise, browse through to see what\n"
                            "all we have to offer!");
                ImGui::EndChild();
            }
            break;
        default:
            break;
    }
}

void DrawQuickStartSelectioncheck() {
    static char fileNameBuf[9];
    if (quickStartOptions.questID == QUICK_START_NONE ||
        (quickStartOptions.questID == SAVETYPE_RANDO && (quickStartOptions.rando.shuffleSet == QUICK_START_NONE ||
                                                         quickStartOptions.rando.logicOption == QUICK_START_NONE))) {
        ImGui::TextColored(TEXT_COLOR(COLOR_ORANGE), "Not Ready");
        return;
    }
    CenterFullWidthSeparatorText("Ready to start?", "Half");
    if (ImGui::BeginTable("NameEntry", 2, ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableNextColumn();
        UIWidgets::PushStyleInput(BenGui::mBenMenu->GetMenuThemeColor());
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputTextWithHint("##name", "Enter your Name", fileNameBuf, sizeof(fileNameBuf), ImGuiInputTextFlags_CallbackEdit, LimitFileNameLength);
        UIWidgets::PopStyleInput();
        ImGui::TableNextColumn();
        if (UIWidgets::Button("Let's Go!", { .size = ImVec2(ImGui::GetContentRegionAvail().x, 0),
                                             .color = strlen(fileNameBuf) > 0 ? COLOR_RED : COLOR_GREEN })) {
            if (strlen(fileNameBuf) > 0) {
                if (quickStartOptions.questID == SAVETYPE_RANDO) {
                    SetRandoQuickStartOptions();
                }
                CVarSetInteger("gRando.Enabled", quickStartOptions.questID == SAVETYPE_VANILLA ? 0 : 1);
                QuickStartQuestInit(fileNameBuf);
            }
        }
        ImGui::EndTable();
    }
}

void QuickStart::Draw() {
    if (!IsVisible()) {
        return;
    }

    auto* viewport = ImGui::GetMainViewport();
    auto windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

    FileSelectState* fileSelectMenu = (FileSelectState*)gGameState;
    ImGui::SetNextWindowPos(viewport->WorkPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(viewport->WorkSize, ImGuiCond_Always);
    if (ImGui::Begin("QuickStartMain", nullptr, windowFlags)) {
        ImGui::SetWindowFontScale(1.3f);
        DrawQuickStartHeader();
        UIWidgets::Separator();
        if (fileSelectMenu->configMode == CM_MAIN_MENU) {
            if (ImGui::BeginTable("QuickStartMenu", 2, ImGuiTableFlags_SizingStretchSame)) {
                ImGui::TableNextColumn();
                DrawQuickStartQuestSelect();
                UIWidgets::Separator();
                DrawQuickStartQuestOptions();
                UIWidgets::Separator();
                DrawQuickStartSelectioncheck();

                ImGui::TableNextColumn();
                DrawQuickStartQuestReview();
                ImGui::EndTable();
            }
        } else {
            ImGui::TextColored(TEXT_COLOR(COLOR_ORANGE), "Waiting for File Select Screen...");
        }
        ImGui::End();
    }
}
} // namespace Ship
