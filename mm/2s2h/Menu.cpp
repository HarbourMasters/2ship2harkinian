#include "Menu.h"
#include "BenPort.h"
#include "2s2h/BenGui/UIWidgets.hpp"
#include "graphic/Fast3D/gfx_rendering_api.h"
#include "2s2h/DeveloperTools/DeveloperTools.h"
#include "2s2h/Enhancements/Enhancements.h"
#include <format>

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
    windowHeight = ImGui::GetMainViewport()->WorkSize.y;
    windowWidth = ImGui::GetMainViewport()->WorkSize.x;
}

void BenMenu::DrawElement() {
    ImGui::SetNextWindowSize({ static_cast<float>(windowWidth), static_cast<float>(windowHeight) });
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), 0, { 0.5f, 0.5f });
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    if (!ImGui::Begin("Main Menu", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::PopStyleVar();
        ImGui::End();
        return;
    }
    ImGui::PushFont(OTRGlobals::Instance->fontStandardLargest);
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiStyle& style = ImGui::GetStyle();

    std::vector<UIWidgets::SidebarEntry> settingsSidebar = {
        { "Audio",    { DrawAudioSettings } },
        { "Graphics", { DrawGraphicsSettings } },
        { "Controls", { DrawControllerSettings } }
    };
    std::vector<UIWidgets::SidebarEntry> enhancementsSidebar = {
        { "Gameplay", { DrawControllerSettings } }
    };
    std::vector<UIWidgets::MainMenuEntry> menuEntries = {
        { "Settings", settingsSidebar, "gSettings.Menu.SettingsSidebarIndex" },
        { "Enhancements", enhancementsSidebar, "gSettings.Menu.EnhancementsSidebarIndex"},
        { "Cheats", {}, "gSettings.Menu.CheatsSidebarIndex"}
    };

    auto sectionCount = menuEntries.size();
    const char* headerCvar = "gSettings.Menu.SelectedHeader";
    uint8_t selectedHeader = CVarGetInteger(headerCvar, 0);
    float centerX = ImGui::GetMainViewport()->GetCenter().x;
    std::vector<ImVec2> headerSizes;
    for (int i = 0; i < sectionCount; i++) {
        headerSizes.push_back(ImGui::CalcTextSize(menuEntries.at(i).label.c_str()));
    }
    float menuHeight = std::fminf(800, window->Viewport->Size.y);

    float sectionHeight = std::fminf(window->Viewport->Size.y, menuHeight - 200);
    ImVec2 pos = window->DC.CursorPos;
    pos.x = centerX - (headerSizes.at(0).x + headerSizes.at(1).x / 2 + (style.ItemSpacing.x * 2));
    pos.y += std::fminf(window->Viewport->Size.y - menuHeight, 100);
    std::vector<UIWidgets::SidebarEntry> sidebar;
    float headerHeight = 0;

    for (int i = 0; i < sectionCount; i++) {
        auto entry = menuEntries.at(i);
        const ImGuiID headerId = window->GetID(std::string(entry.label + "##Header").c_str());
        ImRect bb = { pos - style.FramePadding, pos + headerSizes.at(i) + style.FramePadding };
        ImGui::ItemSize(bb, style.FramePadding.y);
        ImGui::ItemAdd(bb, headerId);
        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb, headerId, &hovered, &held);
        if (pressed) {
            ImGui::MarkItemEdited(headerId);
            CVarSetInteger(headerCvar, i);
            selectedHeader = i;
        }
        if (selectedHeader != i) {
            ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0, 0, 0, 0 });
        }
        if (selectedHeader == i) {
            sidebar = entry.sidebarEntries;
        }
        window->DrawList->AddRectFilled(pos - style.FramePadding, pos + headerSizes.at(i) + style.FramePadding,
            ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive
                : hovered ? ImGuiCol_FrameBgHovered
                : ImGuiCol_FrameBg),
            true, style.FrameRounding);
        if (selectedHeader != i) {
            ImGui::PopStyleColor();
        }

        UIWidgets::RenderText(pos, entry.label.c_str(), ImGui::FindRenderedTextEnd(entry.label.c_str()), true);
        if (i + 1 < sectionCount) {
            pos.x += headerSizes.at(i).x + style.ItemSpacing.x * 2;
        } else {
            headerHeight = bb.GetHeight();
        }
    }
    pos.y += headerHeight + style.ItemSpacing.y;
    pos.x = centerX - 640;
    window->DrawList->AddRectFilled(pos, pos + ImVec2{ 1280, 4 }, ImGui::GetColorU32({255, 255, 255, 255}), true, style.WindowRounding);
    pos.y += style.ItemSpacing.y;
    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize({ 1280, sectionHeight });

    ImGui::BeginChild((menuEntries.at(selectedHeader).label + " Menu").c_str());
    ImGui::SetNextWindowPos(pos + style.ItemSpacing);
    float sidebarWidth = 200;
    ImGui::SetNextWindowSize({ sidebarWidth, sectionHeight });

    const char* sidebarCvar = menuEntries.at(selectedHeader).sidebarCvar;

    uint8_t sectionIndex = CVarGetInteger(sidebarCvar, 0);
    if (sectionIndex > 2) sectionIndex = 2;
    if (sectionIndex < 0) sectionIndex = 0;
    float sectionCenterX = pos.x + (sidebarWidth / 2);
    float topY = pos.y;
    ImGui::BeginChild((menuEntries.at(selectedHeader).label + " Section").c_str());
    for (int i = 0; i < sidebar.size(); i++) {
        auto sidebarEntry = sidebar.at(i);
        const char* label = sidebarEntry.label.c_str();
        const ImGuiID sidebarId = window->GetID(std::string(sidebarEntry.label + "##Sidebar").c_str());
        ImVec2 labelSize = ImGui::CalcTextSize(label, ImGui::FindRenderedTextEnd(label), true);
        pos.y += style.ItemSpacing.y + style.FramePadding.y;
        pos.x = sectionCenterX - labelSize.x / 2;
        ImRect bb = { pos - style.FramePadding, pos + labelSize + style.FramePadding };
        ImGui::ItemSize(bb, style.FramePadding.y);
        ImGui::ItemAdd(bb, sidebarId);
        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb, sidebarId, &hovered, &held);
        if (pressed) {
            ImGui::MarkItemEdited(sidebarId);
            CVarSetInteger(sidebarCvar, i);
            sectionIndex = i;
        }
        if (sectionIndex != i) {
            ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0, 0, 0, 0 });
        }
        if (sectionIndex == i) {

        }
        window->DrawList->AddRectFilled(pos - style.FramePadding, pos + labelSize + style.FramePadding,
            ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive
                : hovered ? ImGuiCol_FrameBgHovered
                : ImGuiCol_FrameBg),
            true, style.FrameRounding);
        if (sectionIndex != i) {
            ImGui::PopStyleColor();
        }
        UIWidgets::RenderText(pos, label, ImGui::FindRenderedTextEnd(label), true);
        pos.y += bb.GetHeight();
    }
    ImGui::EndChild();

    ImGui::PushFont(OTRGlobals::Instance->fontMonoLarger);
    pos = ImVec2{ sectionCenterX + (sidebarWidth / 2), topY} + style.ItemSpacing;
    window->DrawList->AddRectFilled(pos, pos + ImVec2{ 4, sectionHeight - style.FramePadding.y * 2 }, ImGui::GetColorU32({ 255, 255, 255, 255 }), true, style.WindowRounding);
    pos.x += 4 + style.ItemSpacing.x;
    ImGui::SetNextWindowPos(pos + style.ItemSpacing);
    float sectionWidth = 1280 - sidebarWidth - 4 - style.ItemSpacing.x * 6;
    std::string sectionMenuId = sidebar.at(sectionIndex).label + " Settings";
    ImGui::BeginChild(sectionMenuId.c_str(), { sectionWidth, sectionHeight });
    for (int i = 0; i < sidebar.at(sectionIndex).columnFuncs.size(); i++) {
        std::string sectionId = std::format("{} Column {}", sectionMenuId, i);
        ImGui::BeginChild(sectionId.c_str(), { (sectionWidth - style.ItemSpacing.x) / 3, 600 });
        sidebar.at(sectionIndex).columnFuncs.at(i)();
        ImGui::EndChild();
    }
    ImGui::EndChild();
    ImGui::PopFont();
    ImGui::EndChild();
    ImGui::PopFont();

    // style.Colors[ImGuiCol_Button] = prevButtonCol;
    ImGui::PopStyleVar();
    ImGui::End();
}

void DrawAudioSettings() {
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
}

void DrawGraphicsSettings() {
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
}

void DrawControllerSettings() {};
