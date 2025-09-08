
#include "Timesplits.h"
#include <spdlog/fmt/fmt.h>
#include "public/bridge/consolevariablebridge.h"
#include "Context.h"
#include "Window.h"

extern "C" {
#include "variables.h"
uint64_t GetUnixTimestamp();
}

#include "ShipUtils.h"
#include "interface/parameter_static/parameter_static.h"
#include "GameInteractor/GameInteractor.h"

// ImVec4 Colors
#define COLOR_WHITE ImVec4(1.00f, 1.00f, 1.00f, 1.00f)
#define COLOR_GREY ImVec4(0.78f, 0.78f, 0.78f, 1.00f)
#define COLOR_GREEN ImVec4(0.10f, 1.00f, 0.10f, 1.00f)
#define COLOR_RED ImVec4(1.00f, 0.00f, 0.00f, 1.00f)

std::vector<TimesplitObject> splitList;
ImGuiTableFlags tableColumnFlags = ImGuiTableColumnFlags_None;
ImVec4 splitOpacity = { 0, 0, 0, 0.5f };

std::string formatTimesplitTime(uint32_t value) {
    uint32_t sec = value / 10;
    uint32_t hh = sec / 3600;
    uint32_t mm = (sec - hh * 3600) / 60;
    uint32_t ss = sec - hh * 3600 - mm * 60;
    uint32_t ds = value % 10;
    return fmt::format("{}:{:0>2}:{:0>2}.{}", hh, mm, ss, ds);
}

SplitTextObject GetCurrentTimeTextDisplay(TimesplitObject split) {
    uint32_t totalTime = ((GetUnixTimestamp() - gSaveContext.save.shipSaveInfo.fileCreatedAt) / 100);
    SplitTextObject textDisplay;

    switch (split.splitStatus) {
        case SPLIT_INACTIVE:
        case SPLIT_SKIPPED:
            textDisplay.timeDisplay = 0;
            textDisplay.colorDisplay = COLOR_GREY;
            return textDisplay;
        case SPLIT_ACTIVE:
            textDisplay.timeDisplay = totalTime;
            textDisplay.colorDisplay = COLOR_WHITE;
            return textDisplay;
        case SPLIT_COMPLETE:
            textDisplay.timeDisplay = split.splitCurrentTime;
            if (split.splitCurrentTime > split.splitPreviousBest) {
                textDisplay.colorDisplay = COLOR_RED;
            } else if (split.splitCurrentTime == split.splitPreviousBest) {
                textDisplay.colorDisplay = COLOR_WHITE;
            } else if (split.splitCurrentTime < split.splitPreviousBest) {
                textDisplay.colorDisplay = COLOR_GREEN;
            }
            return textDisplay;
        default:
            break;
    }
}

SplitTextObject GetTimeDiffTextDisplay(TimesplitObject split) {
    uint32_t totalTime = ((GetUnixTimestamp() - gSaveContext.save.shipSaveInfo.fileCreatedAt) / 100);
    SplitTextObject textDisplay;

    switch (split.splitStatus) {
        case SPLIT_INACTIVE:
        case SPLIT_SKIPPED:
            textDisplay.timeDisplay = split.splitPreviousBest;
            textDisplay.colorDisplay = COLOR_GREY;
            return textDisplay;
        case SPLIT_ACTIVE:
            if (totalTime > split.splitPreviousBest) {
                textDisplay.timeDisplay = totalTime - split.splitPreviousBest;
                textDisplay.colorDisplay = COLOR_RED;
            } else if (totalTime == split.splitPreviousBest) {
                textDisplay.timeDisplay = totalTime;
                textDisplay.colorDisplay = COLOR_WHITE;
            } else if (totalTime < split.splitPreviousBest) {
                textDisplay.timeDisplay = split.splitPreviousBest - totalTime;
                textDisplay.colorDisplay = COLOR_GREEN;
            }
            return textDisplay;
        case SPLIT_COMPLETE:
            if (split.splitCurrentTime > split.splitPreviousBest) {
                textDisplay.timeDisplay = split.splitCurrentTime - split.splitPreviousBest;
                textDisplay.colorDisplay = COLOR_RED;
            } else if (split.splitCurrentTime == split.splitPreviousBest) {
                textDisplay.timeDisplay = split.splitCurrentTime;
                textDisplay.colorDisplay = COLOR_WHITE;
            } else if (split.splitCurrentTime < split.splitPreviousBest) {
                textDisplay.timeDisplay = split.splitPreviousBest - split.splitCurrentTime;
                textDisplay.colorDisplay = COLOR_GREEN;
            }
            return textDisplay;
        default:
            break;
    }
}

inline void TableCellCenteredText(ImVec4 color, const char* text) {
    float textHeight = ImGui::GetTextLineHeight();
    float offsetY = (32.0f - textHeight + 10.0f) * 0.5f;
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY);
    ImGui::TextColored(color, text);
}

void SplitsPushImageButtonStyle() {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.1f));
}

void SplitsPopImageButtonStyle() {
    ImGui::PopStyleColor(3);
}

void DrawSplitsList() {
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 7.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(10, 0));

    ImGui::Begin("Timesplits", nullptr,
                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing |
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);

    if (ImGui::BeginTable("Splits", 5, ImGuiTableFlags_Hideable | ImGuiTableFlags_Reorderable)) {
        ImGui::TableSetupColumn("Item Image", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHeaderLabel,
                                28.0f);
        ImGui::TableSetupColumn("Item Name");
        ImGui::TableSetupColumn("Current Time");
        ImGui::TableSetupColumn("+/-");
        ImGui::TableSetupColumn("Prev. Best  ");
        if (tableColumnFlags != ImGuiTableColumnFlags_None) {
            ImGui::TableHeadersRow();
        }

        for (size_t i = 0; i < splitList.size(); i++) {
            ImGui::PushID(splitList[i].splitId);

            // Item Image Column
            ImGui::TableNextColumn();

            if (CVarGetInteger("gSettings.TimeSplits.Highlight", 0) && splitList[i].splitStatus == SPLIT_ACTIVE) {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(47, 79, 90, 255));
            }

            SplitsPushImageButtonStyle();
            if (ImGui::ImageButton(std::to_string(splitList[i].splitId).c_str(),
                                   Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
                                       GetItemImageById(splitList[i].splitId)),
                    ImVec2(32.0f, 32.0f), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0),
                                   GetColorTint(splitList[i].splitId))) {
                SkipSplitEntry(i);
            };
            SplitsPopImageButtonStyle();

            // Item Name Column
            ImGui::TableNextColumn();
            TableCellCenteredText(COLOR_WHITE, splitList[i].splitName.c_str());

            // Current Time Column
            ImGui::TableNextColumn();
            TableCellCenteredText(GetCurrentTimeTextDisplay(splitList[i]).colorDisplay,
                                  !gPlayState
                                      ? "--:--:--.-"
                            : formatTimesplitTime(GetCurrentTimeTextDisplay(splitList[i]).timeDisplay).c_str());

            // +/- Column
            ImGui::TableNextColumn();
            TableCellCenteredText(GetTimeDiffTextDisplay(splitList[i]).colorDisplay,
                                  !gPlayState
                                      ? "--:--:--.-"
                                      : formatTimesplitTime(GetTimeDiffTextDisplay(splitList[i]).timeDisplay).c_str());

            // Previous Best Column
            ImGui::TableNextColumn();
            TableCellCenteredText(COLOR_WHITE, !gPlayState ? "--:--:--.-" : formatTimesplitTime(splitList[i].splitPreviousBest).c_str());

            ImGui::PopID();
        }

        ImGui::EndTable();
    }

    ImGui::End();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(1);
}

void UpdateSplitSettings(uint32_t settingName) {
    switch (settingName) {
        case SPLIT_HEADERS:
            tableColumnFlags = CVarGetInteger("gSettings.TimeSplits.ShowHeaders", 0)
                                   ? ImGuiTableColumnFlags_NoHeaderLabel
                                   : ImGuiTableColumnFlags_None;
            break;
        case SPLIT_OPACITY:
            splitOpacity.w = CVarGetInteger("gSettings.TimeSplits.Opacity", 0) ? 0 : 0.5f;
            break;
        default:
            break;
    }
}

void TimesplitsWindow::Draw() {
    if (!CVarGetInteger("gSettings.TimeSplits.Enable", 0)) {
        return;
    }
    ImGui::PushStyleColor(ImGuiCol_WindowBg, splitOpacity);
    DrawSplitsList();
    ImGui::PopStyleColor();
}

void TimesplitsWindow::InitElement() {
    UpdateSplitSettings(SPLIT_HEADERS);
    UpdateSplitSettings(SPLIT_OPACITY);
}

// void TimesplitsWindow::DrawElement() {
// }
