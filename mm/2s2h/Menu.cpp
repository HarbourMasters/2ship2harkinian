#include "Menu.h"
#include "BenPort.h"
#include "2s2h/BenGui/UIWidgets.hpp"
#include "graphic/Fast3D/gfx_rendering_api.h"
#include "2s2h/DeveloperTools/DeveloperTools.h"
#include "2s2h/Enhancements/Enhancements.h"

extern "C" {
#include "z64.h"
#include "functions.h"
}

std::vector<ImVec2> windowTypeSizes = { {} };

static const std::unordered_map<int32_t, const char*> textureFilteringMap = {
    { FILTER_THREE_POINT, "Three-Point" },
    { FILTER_LINEAR, "Linear" },
    { FILTER_NONE, "None" },
};

static const std::unordered_map<int32_t, const char*> debugSaveOptions = {
    { DEBUG_SAVE_INFO_COMPLETE, "100\% save" },
    { DEBUG_SAVE_INFO_VANILLA_DEBUG, "Vanilla debug save" },
    { DEBUG_SAVE_INFO_NONE, "Empty save" },
};

static const std::unordered_map<Ship::AudioBackend, const char*> audioBackendsMap = {
    { Ship::AudioBackend::WASAPI, "Windows Audio Session API" },
    { Ship::AudioBackend::SDL, "SDL" },
};

static std::unordered_map<Ship::WindowBackend, const char*> windowBackendsMap = {
    { Ship::WindowBackend::DX11, "DirectX" },
    { Ship::WindowBackend::SDL_OPENGL, "OpenGL" },
    { Ship::WindowBackend::SDL_METAL, "Metal" },
    { Ship::WindowBackend::GX2, "GX2" }
};

static const std::unordered_map<int32_t, const char*> clockTypeOptions = {
    { CLOCK_TYPE_ORIGINAL, "Original" },
    { CLOCK_TYPE_TEXT_BASED, "Text only" },
};

static const std::unordered_map<int32_t, const char*> alwaysWinDoggyraceOptions = {
    { ALWAYS_WIN_DOGGY_RACE_OFF, "Off" },
    { ALWAYS_WIN_DOGGY_RACE_MASKOFTRUTH, "When owning Mask of Truth" },
    { ALWAYS_WIN_DOGGY_RACE_ALWAYS, "Always" },
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
    ImGui::PushFont(OTRGlobals::Instance->fontStandardLargest);
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
    ImGui::PopFont();

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

    ImGui::PushFont(OTRGlobals::Instance->fontMonoLarger);
    pos = ImVec2{ sectionCenterX + 70, topY } + style.ItemSpacing;
    window->DrawList->AddRectFilled(pos, pos + ImVec2{ 4, 600 - style.FramePadding.y * 2 }, ImGui::GetColorU32({ 255, 255, 255, 255 }), true, style.WindowRounding);
    pos.x += 4 + style.ItemSpacing.x;
    ImGui::SetNextWindowPos(pos + style.ItemSpacing);
    float windowWidth = 1280 - 140 - 4 - style.ItemSpacing.x * 6;
    switch (sectionIndex) {
        case 0:
        default:
            DrawAudioSettings((windowWidth - style.ItemSpacing.x) / 3);
            break;
        case 1:
            DrawGraphicsSettings((windowWidth - style.ItemSpacing.x) / 3);
            break;
        case 2:
            DrawControllerSettings((windowWidth - style.ItemSpacing.x) / 3);
    }
    ImGui::PopFont();

    ImGui::EndChild();
}

void BenMenu::DrawAudioSettings(float width) {
    ImGui::BeginChild("Audio Settings", { width, 600 });
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

void BenMenu::DrawGraphicsSettings(float width) {
    ImGui::BeginChild("Audio Settings", { width, 600 });
#ifndef __APPLE__
    if (UIWidgets::CVarSliderFloat("Internal Resolution: %f %%", CVAR_INTERNAL_RESOLUTION, 0.5f, 2.0f, 1.0f)) {
        Ship::Context::GetInstance()->GetWindow()->SetResolutionMultiplier(
            CVarGetFloat(CVAR_INTERNAL_RESOLUTION, 1));
    };
    UIWidgets::Tooltip(
        "Multiplies your output resolution by the value inputted, as a more intensive but effective "
        "form of anti-aliasing");
#endif
#ifndef __WIIU__
    if (UIWidgets::CVarSliderInt((CVarGetInteger(CVAR_MSAA_VALUE, 1) == 1) ? "Anti-aliasing (MSAA): Off"
        : "Anti-aliasing (MSAA): %d",
        CVAR_MSAA_VALUE, 1, 8, 1)) {
        Ship::Context::GetInstance()->GetWindow()->SetMsaaLevel(CVarGetInteger(CVAR_MSAA_VALUE, 1));
    };
    UIWidgets::Tooltip(
        "Activates MSAA (multi-sample anti-aliasing) from 2x up to 8x, to smooth the edges of rendered "
        "geometry.\n"
        "Higher sample count will result in smoother edges on models, but may reduce performance.");
#endif

    { // FPS Slider
        constexpr unsigned int minFps = 20;
        static unsigned int maxFps;
        if (Ship::Context::GetInstance()->GetWindow()->GetWindowBackend() == Ship::WindowBackend::DX11) {
            maxFps = 360;
        }
        else {
            maxFps = Ship::Context::GetInstance()->GetWindow()->GetCurrentRefreshRate();
        }
        unsigned int currentFps =
            std::max(std::min(OTRGlobals::Instance->GetInterpolationFPS(), maxFps), minFps);
        bool matchingRefreshRate =
            CVarGetInteger("gMatchRefreshRate", 0) &&
            Ship::Context::GetInstance()->GetWindow()->GetWindowBackend() != Ship::WindowBackend::DX11;
        UIWidgets::CVarSliderInt((currentFps == 20) ? "FPS: Original (20)" : "FPS: %d", "gInterpolationFPS",
            minFps, maxFps, 20, { .disabled = matchingRefreshRate });
        if (Ship::Context::GetInstance()->GetWindow()->GetWindowBackend() == Ship::WindowBackend::DX11) {
            UIWidgets::Tooltip(
                "Uses Matrix Interpolation to create extra frames, resulting in smoother graphics. This is "
                "purely visual and does not impact game logic, execution of glitches etc.\n\n A higher target "
                "FPS than your monitor's refresh rate will waste resources, and might give a worse result.");
        }
        else {
            UIWidgets::Tooltip(
                "Uses Matrix Interpolation to create extra frames, resulting in smoother graphics. This is "
                "purely visual and does not impact game logic, execution of glitches etc.");
        }
    } // END FPS Slider

    if (Ship::Context::GetInstance()->GetWindow()->GetWindowBackend() == Ship::WindowBackend::DX11) {
        // UIWidgets::Spacer(0);
        if (ImGui::Button("Match Refresh Rate")) {
            int hz = Ship::Context::GetInstance()->GetWindow()->GetCurrentRefreshRate();
            if (hz >= 20 && hz <= 360) {
                CVarSetInteger("gInterpolationFPS", hz);
                Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
            }
        }
    }
    else {
        UIWidgets::CVarCheckbox("Match Refresh Rate", "gMatchRefreshRate");
    }
    UIWidgets::Tooltip("Matches interpolation value to the current game's window refresh rate");

    if (Ship::Context::GetInstance()->GetWindow()->GetWindowBackend() == Ship::WindowBackend::DX11) {
        UIWidgets::CVarSliderInt(CVarGetInteger("gExtraLatencyThreshold", 80) == 0 ? "Jitter fix: Off"
            : "Jitter fix: >= %d FPS",
            "gExtraLatencyThreshold", 0, 360, 80);
        UIWidgets::Tooltip("When Interpolation FPS setting is at least this threshold, add one frame of input "
            "lag (e.g. 16.6 ms for 60 FPS) in order to avoid jitter. This setting allows the "
            "CPU to work on one frame while GPU works on the previous frame.\nThis setting "
            "should be used when your computer is too slow to do CPU + GPU work in time.");
    }

    // UIWidgets::PaddedSeparator(true, true, 3.0f, 3.0f);
    //  #endregion */

    Ship::WindowBackend runningWindowBackend = Ship::Context::GetInstance()->GetWindow()->GetWindowBackend();
    Ship::WindowBackend configWindowBackend;
    int32_t configWindowBackendId = Ship::Context::GetInstance()->GetConfig()->GetInt("Window.Backend.Id", -1);
    if (configWindowBackendId != -1 &&
        configWindowBackendId < static_cast<int>(Ship::WindowBackend::BACKEND_COUNT)) {
        configWindowBackend = static_cast<Ship::WindowBackend>(configWindowBackendId);
    }
    else {
        configWindowBackend = runningWindowBackend;
    }

    auto availableWindowBackends = Ship::Context::GetInstance()->GetWindow()->GetAvailableWindowBackends();
    std::unordered_map<Ship::WindowBackend, const char*> availableWindowBackendsMap;
    for (auto& backend : *availableWindowBackends) {
        availableWindowBackendsMap[backend] = windowBackendsMap[backend];
    }

    if (UIWidgets::Combobox(
        "Renderer API (Needs reload)", &configWindowBackend, availableWindowBackendsMap,
        { .tooltip = "Sets the renderer API used by the game. Requires a relaunch to take effect.",
          .disabled = Ship::Context::GetInstance()->GetWindow()->GetAvailableWindowBackends()->size() <= 1,
          .disabledTooltip = "Only one renderer API is available on this platform." })) {
        Ship::Context::GetInstance()->GetConfig()->SetInt("Window.Backend.Id",
            static_cast<int32_t>(configWindowBackend));
        Ship::Context::GetInstance()->GetConfig()->SetString("Window.Backend.Name",
            windowBackendsMap.at(configWindowBackend));
        Ship::Context::GetInstance()->GetConfig()->Save();
    }

    if (Ship::Context::GetInstance()->GetWindow()->CanDisableVerticalSync()) {
        UIWidgets::CVarCheckbox("Enable Vsync", CVAR_VSYNC_ENABLED);
    }

    if (Ship::Context::GetInstance()->GetWindow()->SupportsWindowedFullscreen()) {
        UIWidgets::CVarCheckbox("Windowed fullscreen", CVAR_SDL_WINDOWED_FULLSCREEN);
    }

    if (Ship::Context::GetInstance()->GetWindow()->GetGui()->SupportsViewports()) {
        UIWidgets::CVarCheckbox(
            "Allow multi-windows", CVAR_ENABLE_MULTI_VIEWPORTS,
            { .tooltip = "Allows multiple windows to be opened at once. Requires a reload to take effect." });
    }

    UIWidgets::CVarCombobox("Texture Filter (Needs reload)", CVAR_TEXTURE_FILTER, textureFilteringMap);
    ImGui::EndChild();
}

void BenMenu::DrawControllerSettings(float width) {
    ImGui::BeginChild("Controller Settings", { width, 600 });
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
