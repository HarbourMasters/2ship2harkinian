#include "Menu.h"
#include "BenPort.h"
#include "2s2h/BenGui/UIWidgets.hpp"

std::vector<ImVec2> windowTypeSizes = { {} };

void BenMenu::InitElement() {
    //auto dpi = ImGui::GetMainViewport()->DpiScale;
    menuHeight = ImGui::GetMainViewport()->WorkSize.y;
    menuWidth = ImGui::GetMainViewport()->WorkSize.x;
}

void BenMenu::UpdateElement() {
    menuHeight = ImGui::GetMainViewport()->WorkSize.y;
    menuWidth = ImGui::GetMainViewport()->WorkSize.x;
}

void BenMenu::DrawElement() {
    ImGui::SetNextWindowSize({ static_cast<float>(menuWidth), static_cast<float>(menuHeight) });
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), 0, { 0.5f, 0.5f });
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    if (!ImGui::Begin("Settings Menu", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::PopStyleVar();
        ImGui::End();
        return;
    }
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    ImGuiStyle& style = ImGui::GetStyle();
    std::vector<const char*> entries = { "Settings##Menu", "Enhancements##Menu", "Cheats##Menu" };
    /*std::vector<std::vector<std::string>> sidebarEntries = {
        { "Audio", "Graphics", "Controls" },
        { "Camera", "Cutscenes", "Cycle/Saving", "Dialogues", "Dpad", "Fixes", "Graphics", "Masks", "Minigames",
          "Modes", "Player Movement", "Restorations", "Songs/Playback" },
        {}
    };*/
    float centerX = menuWidth / 2.0f;
    uint8_t selectedHeader = CVarGetInteger("gSettings.SelectedHeader", 0);
    std::vector<ImVec2> headerSizes = { ImGui::CalcTextSize(entries.at(0), NULL, true),  ImGui::CalcTextSize(entries.at(1), NULL, true), ImGui::CalcTextSize(entries.at(2), NULL, true)};
    float headerWidth = headerSizes.at(0).x + headerSizes.at(1).x + headerSizes.at(2).x +
                         (style.ItemSpacing.x * 4);
    ImVec2 pos = window->DC.CursorPos;
    pos.x = (centerX - headerWidth / 2);
    pos.y += 100;
    ImRect bb = { pos, pos + headerSizes.at(0) };

    const ImGuiID settingsId = window->GetID(entries.at(0));
    const ImGuiID enhId = window->GetID(entries.at(1));
    const ImGuiID cheatsId = window->GetID(entries.at(2));
    ImGui::ItemSize(bb, style.FramePadding.y);
    ImGui::ItemAdd(bb, settingsId);
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, settingsId, &hovered, &held);
    if (pressed) {
        ImGui::MarkItemEdited(settingsId);
        CVarSetInteger("gSettings.SelectedHeader", 0);
    }
    if (selectedHeader != 0) {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0, 0, 0, 0 });
    }
    window->DrawList->AddRectFilled(pos - style.FramePadding, pos + headerSizes.at(0) + style.FramePadding, 
                                        ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive
                                                           : hovered         ? ImGuiCol_FrameBgHovered
                                                                             : ImGuiCol_FrameBg),
                                        true, style.FrameRounding);
    if (selectedHeader != 0) {
        ImGui::PopStyleColor();
    }

    UIWidgets::RenderText(pos, entries.at(0), ImGui::FindRenderedTextEnd(entries.at(0)), true);
    pos.x += headerSizes.at(0).x + style.ItemSpacing.x * 2;

    ImGui::ItemSize(bb, style.FramePadding.y);
    ImGui::ItemAdd(bb, enhId);
    pressed = ImGui::ButtonBehavior(bb, enhId, &hovered, &held);
    if (pressed) {
        ImGui::MarkItemEdited(enhId);
        CVarSetInteger("gSettings.SelectedHeader", 1);
    }
    if (selectedHeader != 1) {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0, 0, 0, 0 });
    }
    window->DrawList->AddRectFilled(pos - style.FramePadding, pos + headerSizes.at(1) + style.FramePadding, 
                                        ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive
                                                           : hovered         ? ImGuiCol_FrameBgHovered
                                                                             : ImGuiCol_FrameBg),
                                        true, style.FrameRounding);
    if (selectedHeader != 1) {
        ImGui::PopStyleColor();
    }
    UIWidgets::RenderText(pos, entries.at(1), ImGui::FindRenderedTextEnd(entries.at(1)), true);

    // style.Colors[ImGuiCol_Button] = prevButtonCol;
    ImGui::PopStyleVar();
    ImGui::End();
}