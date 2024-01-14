#include "BenMenuBar.h"
#include "ImGui/imgui.h"
#include "public/bridge/consolevariablebridge.h"
#include <libultraship/libultraship.h>
#include "UIWidgets.hpp"
#include <unordered_map>
#include <string>
#include "z64.h"

extern bool ShouldClearTextureCacheAtEndOfFrame;

extern "C" PlayState* gPlayState;

enum SeqPlayers {
    /* 0 */ SEQ_BGM_MAIN,
    /* 1 */ SEQ_FANFARE,
    /* 2 */ SEQ_SFX,
    /* 3 */ SEQ_BGM_SUB,
    /* 4 */ SEQ_MAX
};

static const std::unordered_map<int32_t, const char*> textureFilteringMap = {
    { FILTER_THREE_POINT, "Three-Point" },
    { FILTER_LINEAR, "Linear" },
    { FILTER_NONE, "None" },
};

static const std::unordered_map<LUS::AudioBackend, const char*> audioBackendsMap = {
    { LUS::AudioBackend::WASAPI, "Windows Audio Session API" },
    { LUS::AudioBackend::PULSE, "PulseAudio" },
    { LUS::AudioBackend::SDL, "SDL" },
};

static std::unordered_map<LUS::WindowBackend, const char*> windowBackendsMap = {
    { LUS::WindowBackend::DX11, "DirectX" },
    { LUS::WindowBackend::SDL_OPENGL, "OpenGL" },
    { LUS::WindowBackend::SDL_METAL, "Metal" },
    { LUS::WindowBackend::GX2, "GX2" }
};

namespace BenGui {
void DrawBenMenu() {
    if (UIWidgets::BeginMenu("Game")) {
        if (UIWidgets::MenuItem("Hide Menu Bar",
#if !defined(__SWITCH__) && !defined(__WIIU__)
            "F1"
#else
            "[-]"
#endif
        )) {
            LUS::Context::GetInstance()->GetWindow()->GetGui()->GetMenuBar()->ToggleVisibility();
        }
#if !defined(__SWITCH__) && !defined(__WIIU__)
        if (UIWidgets::MenuItem("Toggle Fullscreen", "F11")) {
            LUS::Context::GetInstance()->GetWindow()->ToggleFullscreen();
        }
#endif
        if (UIWidgets::MenuItem("Reset",
#ifdef __APPLE__
            "Command-R"
#elif !defined(__SWITCH__) && !defined(__WIIU__)
            "Ctrl+R"
#else
            ""
#endif
        )) {
            std::reinterpret_pointer_cast<LUS::ConsoleWindow>(
                LUS::Context::GetInstance()->GetWindow()->GetGui()->GetGuiWindow("Console"))
                ->Dispatch("reset");
        }
#if !defined(__SWITCH__) && !defined(__WIIU__)
        if (UIWidgets::MenuItem("Open App Files Folder")) {
            std::string filesPath = LUS::Context::GetInstance()->GetAppDirectoryPath();
            SDL_OpenURL(std::string("file:///" + std::filesystem::absolute(filesPath).string()).c_str());
        }

        if (UIWidgets::MenuItem("Quit")) {
            LUS::Context::GetInstance()->GetWindow()->Close();
        }
#endif
        ImGui::EndMenu();
    }
}

extern std::shared_ptr<LUS::GuiWindow> mInputEditorWindow;

void DrawSettingsMenu() {
    if (UIWidgets::BeginMenu("Settings")) {
        if (UIWidgets::BeginMenu("Audio")) {
            UIWidgets::CVarSliderFloat("Master Volume: %.0f %%", "gSettings.Audio.MasterVolume", 0.0f, 1.0f, 1.0f, { .isPercentage = true, .showButtons = false, .format = "" });

            if (UIWidgets::CVarSliderFloat("Main Music Volume: %.0f %%", "gSettings.Audio.MainMusicVolume", 0.0f, 1.0f, 1.0f, { .isPercentage = true, .showButtons = false, .format = "" })) {
                // Audio_SetGameVolume(SEQ_BGM_MAIN, CVarGetFloat("gSettings.Audio.MainMusicVolume", 1.0f));
            }
            if (UIWidgets::CVarSliderFloat("Sub Music Volume: %.0f %%", "gSettings.Audio.SubMusicVolume", 0.0f, 1.0f, 1.0f, { .isPercentage = true, .showButtons = false, .format = "" })) {
                // Audio_SetGameVolume(SEQ_BGM_SUB, CVarGetFloat("gSettings.Audio.SubMusicVolume", 1.0f));
            }
            if (UIWidgets::CVarSliderFloat("Sound Effects Volume: %.0f %%", "gSettings.Audio.SoundEffectsVolume", 0.0f, 1.0f, 1.0f, { .isPercentage = true, .showButtons = false, .format = "" })) {
                // Audio_SetGameVolume(SEQ_SFX, CVarGetFloat("gSettings.Audio.SoundEffectsVolume", 1.0f));
            }
            if (UIWidgets::CVarSliderFloat("Fanfare Volume: %.0f %%", "gSettings.Audio.FanfareVolume", 0.0f, 1.0f, 1.0f, { .isPercentage = true, .showButtons = false, .format = "" })) {
                // Audio_SetGameVolume(SEQ_FANFARE, CVarGetFloat("gSettings.Audio.FanfareVolume", 1.0f));
            }

            auto currentAudioBackend = LUS::Context::GetInstance()->GetAudio()->GetAudioBackend();
            if (UIWidgets::Combobox("Audio API", &currentAudioBackend, audioBackendsMap, {
                .disabled = LUS::Context::GetInstance()->GetAudio()->GetAvailableAudioBackends()->size() <= 1,
                .tooltip = "Sets the audio API used by the game. Requires a relaunch to take effect.",
                .disabledTooltip = "Only one audio API is available on this platform."
            })) {
                LUS::Context::GetInstance()->GetAudio()->SetAudioBackend(currentAudioBackend);
            }

            ImGui::EndMenu();
        }

        if (UIWidgets::BeginMenu("Controller")) {
            if (mInputEditorWindow) {
                UIWidgets::WindowButton("Controller Mapping", "gWindows.InputEditor", mInputEditorWindow);
            }
            ImGui::EndMenu();
        }
#ifndef __SWITCH__
        UIWidgets::CVarCheckbox("Menubar Controller Navigation", "gControlNav", {
            .tooltip = "Allows controller navigation of the SOH menu bar (Settings, Enhancements,...)\nCAUTION: "
            "This will disable game inputs while the menubar is visible.\n\nD-pad to move between "
            "items, A to select, and X to grab focus on the menu bar"
        });
#endif
        UIWidgets::CVarCheckbox("Show Inputs", "gInputEnabled", {
            .tooltip = "Shows currently pressed inputs on the bottom right of the screen"
        });
        UIWidgets::CVarSliderFloat("Input Scale: %.1f", "gInputScale", 1.0f, 3.0f, 1.0f, { 
            .showButtons = false, 
            .format = "",
            .tooltip = "Sets the on screen size of the displayed inputs from the Show Inputs setting"
        });
        UIWidgets::CVarSliderInt("Simulated Input Lag: %d frames", "gSimulatedInputLag", 0, 6, 0, {
            .tooltip = "Buffers your inputs to be executed a specified amount of frames later"
        });

        ImGui::EndMenu();
    }

    if (UIWidgets::BeginMenu("Graphics")) {
        /* // #region 2S2H [Todo] None of this works yet
#ifndef __APPLE__
        if (UIWidgets::EnhancementSliderFloat("Internal Resolution: %d %%", "##IMul", "gInternalResolution", 0.5f, 2.0f,
                                              "", 1.0f, true)) {
            LUS::Context::GetInstance()->GetWindow()->SetResolutionMultiplier(CVarGetFloat("gInternalResolution", 1));
        };
        UIWidgets::Tooltip("Multiplies your output resolution by the value inputted, as a more intensive but effective "
                           "form of anti-aliasing");
#endif
#ifndef __WIIU__
        if (UIWidgets::PaddedEnhancementSliderInt("MSAA: %d", "##IMSAA", "gMSAAValue", 1, 8, "", 1, true, true,
                                                  false)) {
            LUS::Context::GetInstance()->GetWindow()->SetMsaaLevel(CVarGetInteger("gMSAAValue", 1));
        };
        UIWidgets::Tooltip("Activates multi-sample anti-aliasing when above 1x up to 8x for 8 samples for every pixel");
#endif

        { // FPS Slider
            const int minFps = 20;
            static int maxFps;
            if (LUS::Context::GetInstance()->GetWindow()->GetWindowBackend() == LUS::WindowBackend::DX11) {
                maxFps = 360;
            } else {
                maxFps = LUS::Context::GetInstance()->GetWindow()->GetCurrentRefreshRate();
            }
            // int currentFps = fmax(fmin(OTRGlobals::Instance->GetInterpolationFPS(), maxFps), minFps);
            int currentFps = 20;
            bool matchingRefreshRate =
                CVarGetInteger("gMatchRefreshRate", 0) &&
                LUS::Context::GetInstance()->GetWindow()->GetWindowBackend() != LUS::WindowBackend::DX11;
            UIWidgets::PaddedEnhancementSliderInt((currentFps == 20) ? "FPS: Original (20)" : "FPS: %d",
                                                  "##FPSInterpolation", "gInterpolationFPS", minFps, maxFps, "", 20,
                                                  true, true, false, matchingRefreshRate);
            if (LUS::Context::GetInstance()->GetWindow()->GetWindowBackend() == LUS::WindowBackend::DX11) {
                UIWidgets::Tooltip(
                    "Uses Matrix Interpolation to create extra frames, resulting in smoother graphics. This is purely "
                    "visual and does not impact game logic, execution of glitches etc.\n\n"
                    "A higher target FPS than your monitor's refresh rate will waste resources, and might give a worse "
                    "result.");
            } else {
                UIWidgets::Tooltip(
                    "Uses Matrix Interpolation to create extra frames, resulting in smoother graphics. This is purely "
                    "visual and does not impact game logic, execution of glitches etc.");
            }
        } // END FPS Slider

        if (LUS::Context::GetInstance()->GetWindow()->GetWindowBackend() == LUS::WindowBackend::DX11) {
            UIWidgets::Spacer(0);
            if (ImGui::Button("Match Refresh Rate")) {
                int hz = LUS::Context::GetInstance()->GetWindow()->GetCurrentRefreshRate();
                if (hz >= 20 && hz <= 360) {
                    CVarSetInteger("gInterpolationFPS", hz);
                    LUS::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
                }
            }
        } else {
            UIWidgets::PaddedEnhancementCheckbox("Match Refresh Rate", "gMatchRefreshRate", true, false);
        }
        UIWidgets::Tooltip("Matches interpolation value to the current game's window refresh rate");

        if (LUS::Context::GetInstance()->GetWindow()->GetWindowBackend() == LUS::WindowBackend::DX11) {
            UIWidgets::PaddedEnhancementSliderInt(
                CVarGetInteger("gExtraLatencyThreshold", 80) == 0 ? "Jitter fix: Off" : "Jitter fix: >= %d FPS",
                "##ExtraLatencyThreshold", "gExtraLatencyThreshold", 0, 360, "", 80, true, true, false);
            UIWidgets::Tooltip("When Interpolation FPS setting is at least this threshold, add one frame of input lag "
                               "(e.g. 16.6 ms for 60 FPS) in order to avoid jitter. This setting allows the CPU to "
                               "work on one frame while GPU works on the previous frame.\nThis setting should be used "
                               "when your computer is too slow to do CPU + GPU work in time.");
        }

        UIWidgets::PaddedSeparator(true, true, 3.0f, 3.0f);
        // #endregion */

        LUS::WindowBackend runningWindowBackend = LUS::Context::GetInstance()->GetWindow()->GetWindowBackend();
        LUS::WindowBackend configWindowBackend;
        int32_t configWindowBackendId = LUS::Context::GetInstance()->GetConfig()->GetInt("Window.Backend.Id", -1);
        if (configWindowBackendId != -1 && configWindowBackendId < static_cast<int>(LUS::WindowBackend::BACKEND_COUNT)) {
            configWindowBackend = static_cast<LUS::WindowBackend>(configWindowBackendId);
        } else {
            configWindowBackend = runningWindowBackend;
        }

        auto availableWindowBackends = LUS::Context::GetInstance()->GetWindow()->GetAvailableWindowBackends();
        std::unordered_map<LUS::WindowBackend, const char*> availableWindowBackendsMap;
        for (auto& backend : *availableWindowBackends) {
            availableWindowBackendsMap[backend] = windowBackendsMap[backend];
        }

        if (UIWidgets::Combobox("Renderer API (Needs reload)", &configWindowBackend, availableWindowBackendsMap, {
            .disabled = LUS::Context::GetInstance()->GetWindow()->GetAvailableWindowBackends()->size() <= 1,
            .tooltip = "Sets the renderer API used by the game. Requires a relaunch to take effect.",
            .disabledTooltip = "Only one renderer API is available on this platform."
        })) {
            LUS::Context::GetInstance()->GetConfig()->SetInt("Window.Backend.Id", static_cast<int32_t>(configWindowBackend));
            LUS::Context::GetInstance()->GetConfig()->SetString("Window.Backend.Name", windowBackendsMap.at(configWindowBackend));
            LUS::Context::GetInstance()->GetConfig()->Save();
        }

        if (LUS::Context::GetInstance()->GetWindow()->CanDisableVerticalSync()) {
            UIWidgets::CVarCheckbox("Enable Vsync", "gVsyncEnabled");
        }

        if (LUS::Context::GetInstance()->GetWindow()->SupportsWindowedFullscreen()) {
            UIWidgets::CVarCheckbox("Windowed fullscreen", "gSdlWindowedFullscreen");
        }

        if (LUS::Context::GetInstance()->GetWindow()->GetGui()->SupportsViewports()) {
            UIWidgets::CVarCheckbox("Allow multi-windows", "gEnableMultiViewports", {
                .tooltip = "Allows multiple windows to be opened at once. Requires a reload to take effect."
            });
        }

        UIWidgets::CVarCombobox("Texture Filter (Needs reload)", "gTextureFilter", textureFilteringMap);

        // Currently this only has "Overlays Text Font", it doesn't use our new UIWidgets so it stands out
        // LUS::Context::GetInstance()->GetWindow()->GetGui()->GetGameOverlay()->DrawSettings();

        ImGui::EndMenu();
    }
}

extern std::shared_ptr<LUS::GuiWindow> mStatsWindow;
extern std::shared_ptr<LUS::GuiWindow> mConsoleWindow;
extern std::shared_ptr<LUS::GuiWindow> mGfxDebuggerWindow;
extern std::shared_ptr<SaveEditorWindow> mSaveEditorWindow;

void DrawDeveloperToolsMenu() {
    if (UIWidgets::BeginMenu("Developer Tools", UIWidgets::Colors::Yellow)) {
        UIWidgets::CVarCheckbox("OoT Debug Mode", "gDebugEnabled", {
            .tooltip = "Enables Debug Mode, allowing you to select maps with L + R + Z, noclip with L + D-pad "
            "Right, and open the debug menu with L on the pause screen"
        });
        UIWidgets::CVarCheckbox("No Clip", "gDeveloperTools.NoClip");
        UIWidgets::CVarCheckbox("Moon Jump on L", "gDeveloperTools.MoonJumpOnL", {
            .tooltip = "Holding L makes you float into the air"
        });
        if (gPlayState != NULL) {
            UIWidgets::PaddedSeparator();
            UIWidgets::Checkbox("Frame Advance", (bool*)&gPlayState->frameAdvCtx.enabled, {
                .tooltip = "This allows you to advance through the game one frame at a time on command. "
                "To advance a frame, hold Z and tap R on the second controller. Holding Z "
                "and R will advance a frame every half second. You can also use the buttons below."
            });
            if (gPlayState->frameAdvCtx.enabled) {
                if (UIWidgets::Button("Advance 1", { .size = UIWidgets::Sizes::Inline })) {
                    CVarSetInteger("gDeveloperTools.FrameAdvanceTick", 1);
                }
                ImGui::SameLine();
                UIWidgets::Button("Advance (Hold)", { .size = UIWidgets::Sizes::Inline });
                if (ImGui::IsItemActive()) {
                    CVarSetInteger("gDeveloperTools.FrameAdvanceTick", 1);
                }
            }
        }
        UIWidgets::PaddedSeparator();
        if (mStatsWindow) {
            UIWidgets::WindowButton("Stats", "gWindows.Stats", mStatsWindow, {
                .tooltip = "Shows the stats window, with your FPS and frametimes, and the OS you're playing on"
            });
        }
        if (mConsoleWindow) {
            UIWidgets::WindowButton("Console", "gWindows.Console", mConsoleWindow, {
                .tooltip = "Enables the console window, allowing you to input commands, type help for some examples"
            });
        }
        if (mGfxDebuggerWindow) {
            UIWidgets::WindowButton("Gfx Debugger", "gWindows.GfxDebugger", mGfxDebuggerWindow, {
                .tooltip = "Enables the Gfx Debugger window, allowing you to input commands, type help for some examples"
            });
        }
        if (mSaveEditorWindow) {
            UIWidgets::WindowButton("Save Editor", "gWindows.SaveEditor", mSaveEditorWindow, {
                .tooltip = "Enables the Save Editor window, allowing you to edit your save file"
            });
        }
        ImGui::EndMenu();
    }
}

void BenMenuBar::DrawElement() {
    if (ImGui::BeginMenuBar()) {
        static ImVec2 sWindowPadding(8.0f, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, sWindowPadding);

        ImGui::SetCursorPosY(0.0f);

        DrawBenMenu();

        ImGui::SetCursorPosY(0.0f);

        DrawSettingsMenu();

        ImGui::SetCursorPosY(0.0f);

        DrawDeveloperToolsMenu();

        ImGui::SetCursorPosY(0.0f);

        ImGui::PopStyleVar(1);
        ImGui::EndMenuBar();
    }
}
}
 // namespace BenGui 