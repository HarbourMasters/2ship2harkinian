#include "Menu.h"
#include "BenPort.h"
#include "2s2h/BenGui/UIWidgets.hpp"

extern "C" {
#include "z64.h"
#include "functions.h"
}

std::vector<ImVec2> windowTypeSizes = { {} };

static const std::unordered_map<Ship::AudioBackend, const char*> audioBackendsMap = {
    { Ship::AudioBackend::WASAPI, "Windows Audio Session API" },
    { Ship::AudioBackend::SDL, "SDL" },
};

void BenMenu::InitElement() {
}

void BenMenu::UpdateElement() {
    menuHeight = ImGui::GetMainViewport()->WorkSize.y;
    menuWidth = ImGui::GetMainViewport()->WorkSize.x;
}

void BenMenu::DrawElement() {
    ImGui::SetNextWindowSize({ static_cast<float>(menuWidth), static_cast<float>(menuHeight) });
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), 0, { 0.5f, 0.5f });
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    if (!ImGui::Begin("Main Menu", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::PopStyleVar();
        ImGui::End();
        return;
    }
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiStyle& style = ImGui::GetStyle();
    std::vector<const char*> entries = { "Settings##Menu", "Enhancements##Menu", "Cheats##Menu" };
    /*std::vector<std::vector<std::string>> sidebarEntries = {
        { "Audio", "Graphics", "Controls" },
        { "Camera", "Cutscenes", "Cycle/Saving", "Dialogues", "Dpad", "Fixes", "Graphics", "Masks", "Minigames",
          "Modes", "Player Movement", "Restorations", "Songs/Playback" },
        {}
    };*/
    
    float centerX = ImGui::GetMainViewport()->GetCenter().x;
    uint8_t selectedHeader = CVarGetInteger("gSettings.SelectedHeader", 0);
    std::vector<ImVec2> headerSizes = { ImGui::CalcTextSize(entries.at(0), NULL, true),  ImGui::CalcTextSize(entries.at(1), NULL, true), ImGui::CalcTextSize(entries.at(2), NULL, true)};
    ImVec2 pos = window->DC.CursorPos;
    pos.x = centerX - (headerSizes.at(0).x + headerSizes.at(1).x / 2 + (style.ItemSpacing.x * 2));
    pos.y += 100;

    const ImGuiID settingsId = window->GetID(entries.at(0));
    const ImGuiID enhId = window->GetID(entries.at(1));
    const ImGuiID cheatsId = window->GetID(entries.at(2));
    ImRect bb = { pos - style.FramePadding, pos + headerSizes.at(0) + style.FramePadding };
    ImGui::ItemSize(bb, style.FramePadding.y);
    ImGui::ItemAdd(bb, settingsId);
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, settingsId, &hovered, &held);
    if (pressed) {
        ImGui::MarkItemEdited(settingsId);
        CVarSetInteger("gSettings.SelectedHeader", 0);
        selectedHeader = 0;
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

    bb = { pos - style.FramePadding, pos + headerSizes.at(1) + style.FramePadding };
    ImGui::ItemSize(bb, style.FramePadding.y);
    ImGui::ItemAdd(bb, enhId);
    pressed = ImGui::ButtonBehavior(bb, enhId, &hovered, &held);
    if (pressed) {
        ImGui::MarkItemEdited(enhId);
        CVarSetInteger("gSettings.SelectedHeader", 1);
        selectedHeader = 1;
    }
    if (selectedHeader != 1) {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0, 0, 0, 0 });
    }
    window->DrawList->AddRectFilled(pos - style.FramePadding, pos + headerSizes.at(1) + style.FramePadding,
        ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive
            : hovered ? ImGuiCol_FrameBgHovered
            : ImGuiCol_FrameBg),
        true, style.FrameRounding);
    if (selectedHeader != 1) {
        ImGui::PopStyleColor();
    }

    UIWidgets::RenderText(pos, entries.at(1), ImGui::FindRenderedTextEnd(entries.at(1)), true);
    pos.x += headerSizes.at(1).x + style.ItemSpacing.x * 2;

    bb = { pos - style.FramePadding, pos + headerSizes.at(2) + style.FramePadding };
    ImGui::ItemSize(bb, style.FramePadding.y);
    ImGui::ItemAdd(bb, cheatsId);
    pressed = ImGui::ButtonBehavior(bb, cheatsId, &hovered, &held);
    if (pressed) {
        ImGui::MarkItemEdited(cheatsId);
        CVarSetInteger("gSettings.SelectedHeader", 2);
        selectedHeader = 2;
    }
    if (selectedHeader != 2) {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0, 0, 0, 0 });
    }
    window->DrawList->AddRectFilled(pos - style.FramePadding, pos + headerSizes.at(2) + style.FramePadding,
        ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive
            : hovered ? ImGuiCol_FrameBgHovered
            : ImGuiCol_FrameBg),
        true, style.FrameRounding);
    if (selectedHeader != 2) {
        ImGui::PopStyleColor();
    }

    UIWidgets::RenderText(pos, entries.at(2), ImGui::FindRenderedTextEnd(entries.at(2)), true);
    pos.y += bb.GetHeight() + style.ItemSpacing.y;
    pos.x = centerX - 640;
    window->DrawList->AddRectFilled(pos, pos + ImVec2{ 1280, 4 }, ImGui::GetColorU32({255, 255, 255, 255}), true, style.WindowRounding);
    pos.y += style.ItemSpacing.y;
    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize({ 1280, 600 });

    switch (selectedHeader) {
    case 1:
        DrawEnhancementsMenu();
        break;
    case 2:
        DrawCheatsMenu();
    case 0:
    default:
        DrawSettingsMenu();
        break;
    }

    // style.Colors[ImGuiCol_Button] = prevButtonCol;
    ImGui::PopStyleVar();
    ImGui::End();
}

void BenMenu::DrawSettingsMenu() {
    ImGui::BeginChild("Settings Menu");

    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImVec2 pos = window->DC.CursorPos;
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::SetNextWindowPos(pos + style.ItemSpacing);
    ImGui::SetNextWindowSize({ 140, 600 });
    ImGui::BeginChild("Settings Section");
    uint8_t sectionIndex = CVarGetInteger("gSettings.SettingsMenuIndex", 0);
    if (sectionIndex > 2) sectionIndex = 2;
    if (sectionIndex < 0) sectionIndex = 0;
    std::vector<const char*> entries = { "Audio##Settings", "Graphics##Settings", "Controls##Settings" };
    std::vector<ImVec2> headerSizes = { ImGui::CalcTextSize(entries.at(0), NULL, true),  ImGui::CalcTextSize(entries.at(1), NULL, true), ImGui::CalcTextSize(entries.at(2), NULL, true) };
    const ImGuiID audioId = window->GetID(entries.at(0));
    const ImGuiID graphicsId = window->GetID(entries.at(1));
    const ImGuiID controlsId = window->GetID(entries.at(2));
    float sectionCenterX = pos.x + 70;
    float topY = pos.y;
    pos.y += style.ItemSpacing.y + style.FramePadding.y;
    pos.x = sectionCenterX - headerSizes.at(0).x / 2;
    ImVec2 offset = { headerSizes.at(0).x / 2, 0 };
    ImRect bb = { pos - style.FramePadding - offset, pos + headerSizes.at(0) + style.FramePadding };
    ImGui::ItemSize(bb, style.FramePadding.y);
    ImGui::ItemAdd(bb, audioId);
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, audioId, &hovered, &held);
    if (pressed) {
        ImGui::MarkItemEdited(audioId);
        CVarSetInteger("gSettings.SettingsMenuIndex", 0);
        sectionIndex = 0;
    }
    if (sectionIndex != 0) {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0, 0, 0, 0 });
    }
    window->DrawList->AddRectFilled(pos - style.FramePadding, pos + headerSizes.at(0) + style.FramePadding,
        ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive
            : hovered ? ImGuiCol_FrameBgHovered
            : ImGuiCol_FrameBg),
        true, style.FrameRounding);
    if (sectionIndex != 0) {
        ImGui::PopStyleColor();
    }
    UIWidgets::RenderText(pos, entries.at(0), ImGui::FindRenderedTextEnd(entries.at(0)), true);

    offset = { headerSizes.at(1).x / 2, 0 };
    pos.y += style.ItemSpacing.y + headerSizes.at(1).y + style.FramePadding.y * 2;
    pos.x = sectionCenterX - headerSizes.at(1).x / 2;
    bb = { pos - style.FramePadding, pos + headerSizes.at(1) + style.FramePadding };
    ImGui::ItemSize(bb, style.FramePadding.y);
    ImGui::ItemAdd(bb, graphicsId);
    pressed = ImGui::ButtonBehavior(bb, graphicsId, &hovered, &held);
    if (pressed) {
        ImGui::MarkItemEdited(graphicsId);
        CVarSetInteger("gSettings.SettingsMenuIndex", 1);
        sectionIndex = 1;
    }
    if (sectionIndex != 1) {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0, 0, 0, 0 });
    }
    window->DrawList->AddRectFilled(pos - style.FramePadding, pos + headerSizes.at(1) + style.FramePadding,
        ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive
            : hovered ? ImGuiCol_FrameBgHovered
            : ImGuiCol_FrameBg),
        true, style.FrameRounding);
    if (sectionIndex != 1) {
        ImGui::PopStyleColor();
    }
    UIWidgets::RenderText(pos, entries.at(1), ImGui::FindRenderedTextEnd(entries.at(1)), true);

    offset = { headerSizes.at(2).x / 2, 0 };
    pos.y += style.ItemSpacing.y + headerSizes.at(2).y + style.FramePadding.y * 2;
    pos.x = sectionCenterX - headerSizes.at(2).x / 2;
    bb = { pos - style.FramePadding, pos + headerSizes.at(2) + style.FramePadding };
    ImGui::ItemSize(bb, style.FramePadding.y);
    ImGui::ItemAdd(bb, controlsId);
    pressed = ImGui::ButtonBehavior(bb, controlsId, &hovered, &held);
    if (pressed) {
        ImGui::MarkItemEdited(controlsId);
        CVarSetInteger("gSettings.SettingsMenuIndex", 2);
        sectionIndex = 2;
    }
    if (sectionIndex != 2) {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0, 0, 0, 0 });
    }
    window->DrawList->AddRectFilled(pos - style.FramePadding, pos + headerSizes.at(2) + style.FramePadding,
        ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive
            : hovered ? ImGuiCol_FrameBgHovered
            : ImGuiCol_FrameBg),
        true, style.FrameRounding);
    if (sectionIndex != 2) {
        ImGui::PopStyleColor();
    }
    UIWidgets::RenderText(pos, entries.at(2), ImGui::FindRenderedTextEnd(entries.at(2)), true);
    ImGui::EndChild();

    pos = ImVec2{ sectionCenterX + 70, topY } + style.ItemSpacing;
    window->DrawList->AddRectFilled(pos, pos + ImVec2{ 4, 600 - style.FramePadding.y * 2 }, ImGui::GetColorU32({ 255, 255, 255, 255 }), true, style.WindowRounding);
    pos.x += 4 + style.ItemSpacing.x;

    if (sectionIndex == 0) {
        ImGui::SetNextWindowPos(pos + style.ItemSpacing);
        float sectionWidth = 1280 - 140 - 4 - style.ItemSpacing.x * 6;
        ImGui::BeginChild("Audio Settings", { sectionWidth, 600 });
        UIWidgets::CVarSliderFloat("Master Volume: %.0f %%", "gSettings.Audio.MasterVolume", 0.0f, 1.0f, 1.0f,
            { .showButtons = false, .format = "", .isPercentage = true });

        if (UIWidgets::CVarSliderFloat("Main Music Volume: %.0f %%", "gSettings.Audio.MainMusicVolume", 0.0f, 1.0f,
            1.0f, { .showButtons = false, .format = "", .isPercentage = true })) {
            AudioSeq_SetPortVolumeScale(SEQ_PLAYER_BGM_MAIN, CVarGetFloat("gSettings.Audio.MainMusicVolume", 1.0f));
        }
        if (UIWidgets::CVarSliderFloat("Sub Music Volume: %.0f %%", "gSettings.Audio.SubMusicVolume", 0.0f, 1.0f,
            1.0f, { .showButtons = false, .format = "", .isPercentage = true })) {
            AudioSeq_SetPortVolumeScale(SEQ_PLAYER_BGM_SUB, CVarGetFloat("gSettings.Audio.SubMusicVolume", 1.0f));
        }
        if (UIWidgets::CVarSliderFloat("Sound Effects Volume: %.0f %%", "gSettings.Audio.SoundEffectsVolume", 0.0f,
            1.0f, 1.0f, { .showButtons = false, .format = "", .isPercentage = true })) {
            AudioSeq_SetPortVolumeScale(SEQ_PLAYER_SFX, CVarGetFloat("gSettings.Audio.SoundEffectsVolume", 1.0f));
        }
        if (UIWidgets::CVarSliderFloat("Fanfare Volume: %.0f %%", "gSettings.Audio.FanfareVolume", 0.0f, 1.0f, 1.0f,
            { .showButtons = false, .format = "", .isPercentage = true })) {
            AudioSeq_SetPortVolumeScale(SEQ_PLAYER_FANFARE, CVarGetFloat("gSettings.Audio.FanfareVolume", 1.0f));
        }
        if (UIWidgets::CVarSliderFloat("Ambience Volume: %.0f %%", "gSettings.Audio.AmbienceVolume", 0.0f, 1.0f,
            1.0f, { .showButtons = false, .format = "", .isPercentage = true })) {
            AudioSeq_SetPortVolumeScale(SEQ_PLAYER_AMBIENCE, CVarGetFloat("gSettings.Audio.AmbienceVolume", 1.0f));
        }

        auto currentAudioBackend = Ship::Context::GetInstance()->GetAudio()->GetAudioBackend();
        if (UIWidgets::Combobox(
            "Audio API", &currentAudioBackend, audioBackendsMap,
            { .tooltip = "Sets the audio API used by the game. Requires a relaunch to take effect.",
              .disabled = Ship::Context::GetInstance()->GetAudio()->GetAvailableAudioBackends()->size() <= 1,
              .disabledTooltip = "Only one audio API is available on this platform." })) {
            Ship::Context::GetInstance()->GetAudio()->SetAudioBackend(currentAudioBackend);
        }
        ImGui::EndChild();
    }

    ImGui::EndChild();
}

void BenMenu::DrawEnhancementsMenu() {
    ImGui::BeginChild("Enhancements Menu");



    ImGui::EndChild();
}

void BenMenu::DrawCheatsMenu() {
    ImGui::BeginChild("Cheats Menu");



    ImGui::EndChild();
}
