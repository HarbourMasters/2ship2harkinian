#include "BenMenuBar.h"
#include "BenPort.h"
#include "ImGui/imgui.h"
#include "public/bridge/consolevariablebridge.h"
#include <libultraship/libultraship.h>
#include "UIWidgets.hpp"
#include <unordered_map>
#include <string>
#include "2s2h/Enhancements/Enhancements.h"
#include "2s2h/DeveloperTools/DeveloperTools.h"
#include "HudEditor.h"

extern bool ShouldClearTextureCacheAtEndOfFrame;

extern "C" {
#include "z64.h"
#include "functions.h"
extern PlayState* gPlayState;
}

static const std::unordered_map<int32_t, const char*> textureFilteringMap = {
    { FILTER_THREE_POINT, "Three-Point" },
    { FILTER_LINEAR, "Linear" },
    { FILTER_NONE, "None" },
};

static const std::unordered_map<LUS::AudioBackend, const char*> audioBackendsMap = {
    { LUS::AudioBackend::WASAPI, "Windows Audio Session API" },
    { LUS::AudioBackend::SDL, "SDL" },
};

static std::unordered_map<LUS::WindowBackend, const char*> windowBackendsMap = {
    { LUS::WindowBackend::DX11, "DirectX" },
    { LUS::WindowBackend::SDL_OPENGL, "OpenGL" },
    { LUS::WindowBackend::SDL_METAL, "Metal" },
    { LUS::WindowBackend::GX2, "GX2" }
};

namespace BenGui {

void DrawMenuBarIcon() {
    static bool gameIconLoaded = false;
    if (!gameIconLoaded) {
        LUS::Context::GetInstance()->GetWindow()->GetGui()->LoadTextureFromRawImage("Game_Icon", "textures/icons/g2ShipIcon.png");
        gameIconLoaded = true;
    }

    if (LUS::Context::GetInstance()->GetWindow()->GetGui()->HasTextureByName("Game_Icon")) {
#ifdef __SWITCH__
        ImVec2 iconSize = ImVec2(20.0f, 20.0f);
        float posScale = 1.0f;
#elif defined(__WIIU__)
        ImVec2 iconSize = ImVec2(16.0f * 2, 16.0f * 2);
        float posScale = 2.0f;
#else
        ImVec2 iconSize = ImVec2(16.0f, 16.0f);
        float posScale = 1.0f;
#endif
        ImGui::SetCursorPos(ImVec2(5, 5) * posScale);
        ImGui::Image(LUS::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName("Game_Icon"), iconSize);
        ImGui::SameLine();
        ImGui::SetCursorPos(ImVec2(25, 0) * posScale);
    }
}

void DrawBenMenu() {
    if (UIWidgets::BeginMenu("2Ship")) {
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
            UIWidgets::CVarSliderFloat("Master Volume: %.0f %%", "gSettings.Audio.MasterVolume", 0.0f, 1.0f, 1.0f, { .showButtons = false, .format = "", .isPercentage = true });

            if (UIWidgets::CVarSliderFloat("Main Music Volume: %.0f %%", "gSettings.Audio.MainMusicVolume", 0.0f, 1.0f, 1.0f, { .showButtons = false, .format = "", .isPercentage = true })) {
                AudioSeq_SetPortVolumeScale(SEQ_PLAYER_BGM_MAIN, CVarGetFloat("gSettings.Audio.MainMusicVolume", 1.0f));
            }
            if (UIWidgets::CVarSliderFloat("Sub Music Volume: %.0f %%", "gSettings.Audio.SubMusicVolume", 0.0f, 1.0f, 1.0f, { .showButtons = false, .format = "", .isPercentage = true })) {
                AudioSeq_SetPortVolumeScale(SEQ_PLAYER_BGM_SUB, CVarGetFloat("gSettings.Audio.SubMusicVolume", 1.0f));
            }
            if (UIWidgets::CVarSliderFloat("Sound Effects Volume: %.0f %%", "gSettings.Audio.SoundEffectsVolume", 0.0f, 1.0f, 1.0f, { .showButtons = false, .format = "", .isPercentage = true })) {
                AudioSeq_SetPortVolumeScale(SEQ_PLAYER_SFX, CVarGetFloat("gSettings.Audio.SoundEffectsVolume", 1.0f));
            }
            if (UIWidgets::CVarSliderFloat("Fanfare Volume: %.0f %%", "gSettings.Audio.FanfareVolume", 0.0f, 1.0f, 1.0f, { .showButtons = false, .format = "", .isPercentage = true })) {
                AudioSeq_SetPortVolumeScale(SEQ_PLAYER_FANFARE, CVarGetFloat("gSettings.Audio.FanfareVolume", 1.0f));
            }
            if (UIWidgets::CVarSliderFloat("Ambience Volume: %.0f %%", "gSettings.Audio.AmbienceVolume", 0.0f, 1.0f, 1.0f, { .showButtons = false, .format = "", .isPercentage = true })) {
                AudioSeq_SetPortVolumeScale(SEQ_PLAYER_AMBIENCE, CVarGetFloat("gSettings.Audio.AmbienceVolume", 1.0f));
            }

            auto currentAudioBackend = LUS::Context::GetInstance()->GetAudio()->GetAudioBackend();
            if (UIWidgets::Combobox("Audio API", &currentAudioBackend, audioBackendsMap, {
                .tooltip = "Sets the audio API used by the game. Requires a relaunch to take effect.",
                .disabled = LUS::Context::GetInstance()->GetAudio()->GetAvailableAudioBackends()->size() <= 1,
                .disabledTooltip = "Only one audio API is available on this platform."
            })) {
                LUS::Context::GetInstance()->GetAudio()->SetAudioBackend(currentAudioBackend);
            }

            ImGui::EndMenu();
        }

        if (UIWidgets::BeginMenu("Graphics")) {
            
    #ifndef __APPLE__
            if (UIWidgets::CVarSliderFloat("Internal Resolution: %d %%", "gInternalResolution", 0.5f, 2.0f, 1.0f)) {
                LUS::Context::GetInstance()->GetWindow()->SetResolutionMultiplier(CVarGetFloat("gInternalResolution", 1));
            };
            UIWidgets::Tooltip("Multiplies your output resolution by the value inputted, as a more intensive but effective " "form of anti-aliasing");
#endif 
#ifndef __WIIU__ 
            if (UIWidgets::CVarSliderInt("MSAA: %d","gMSAAValue", 1, 8, 1)) {
                LUS::Context::GetInstance()->GetWindow()->SetMsaaLevel(CVarGetInteger("gMSAAValue", 1));
            };
            UIWidgets::Tooltip("Activates multi-sample anti-aliasing when above 1x up to 8x for 8 samples for every pixel");
#endif

            { // FPS Slider
                constexpr unsigned int minFps = 20;
                static unsigned int maxFps;
                if (LUS::Context::GetInstance()->GetWindow()->GetWindowBackend() == LUS::WindowBackend::DX11) {
                    maxFps = 360;
                } else {
                    maxFps = LUS::Context::GetInstance()->GetWindow()->GetCurrentRefreshRate();
                }
                unsigned int currentFps = std::max(std::min(OTRGlobals::Instance->GetInterpolationFPS(), maxFps), minFps);
                bool matchingRefreshRate =
                    CVarGetInteger("gMatchRefreshRate", 0) &&
                    LUS::Context::GetInstance()->GetWindow()->GetWindowBackend() != LUS::WindowBackend::DX11;
                UIWidgets::CVarSliderInt((currentFps == 20) ? "FPS: Original (20)" : "FPS: %d", "gInterpolationFPS",
                                         minFps, maxFps, 20, {.disabled = matchingRefreshRate} );
                if (LUS::Context::GetInstance()->GetWindow()->GetWindowBackend() == LUS::WindowBackend::DX11) {
                    UIWidgets::Tooltip(
                        "Uses Matrix Interpolation to create extra frames, resulting in smoother graphics. This is purely visual and does not impact game logic, execution of glitches etc.\n\n A higher target FPS than your monitor's refresh rate will waste resources, and might give a worse result."); 
                } else { 
                    UIWidgets::Tooltip( "Uses Matrix Interpolation to create extra frames, resulting in smoother graphics. This is purely visual and does not impact game logic, execution of glitches etc.");
                }
            } // END FPS Slider

            if (LUS::Context::GetInstance()->GetWindow()->GetWindowBackend() == LUS::WindowBackend::DX11) {
                //UIWidgets::Spacer(0);
                if (ImGui::Button("Match Refresh Rate")) {
                    int hz = LUS::Context::GetInstance()->GetWindow()->GetCurrentRefreshRate();
                    if (hz >= 20 && hz <= 360) {
                        CVarSetInteger("gInterpolationFPS", hz);
                        LUS::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
                    }
                }
            } else {
                UIWidgets::CVarCheckbox("Match Refresh Rate", "gMatchRefreshRate");
            }
            UIWidgets::Tooltip("Matches interpolation value to the current game's window refresh rate");

            if (LUS::Context::GetInstance()->GetWindow()->GetWindowBackend() == LUS::WindowBackend::DX11) {
                UIWidgets::CVarSliderInt(CVarGetInteger("gExtraLatencyThreshold", 80) == 0 ? "Jitter fix: Off" : "Jitter fix: >= %d FPS",
                    "gExtraLatencyThreshold", 0, 360, 80);
                UIWidgets::Tooltip("When Interpolation FPS setting is at least this threshold, add one frame of input lag (e.g. 16.6 ms for 60 FPS) in order to avoid jitter. This setting allows the CPU to work on one frame while GPU works on the previous frame.\nThis setting should be used when your computer is too slow to do CPU + GPU work in time.");
            }

            //UIWidgets::PaddedSeparator(true, true, 3.0f, 3.0f);
            // #endregion */

            LUS::WindowBackend runningWindowBackend = LUS::Context::GetInstance()->GetWindow()->GetWindowBackend();
            LUS::WindowBackend configWindowBackend;
            int32_t configWindowBackendId = LUS::Context::GetInstance()->GetConfig()->GetInt("Window.Backend.Id", -1);
            if (configWindowBackendId != -1 &&
                configWindowBackendId < static_cast<int>(LUS::WindowBackend::BACKEND_COUNT)) {
                configWindowBackend = static_cast<LUS::WindowBackend>(configWindowBackendId);
            } else {
                configWindowBackend = runningWindowBackend;
            }

            auto availableWindowBackends = LUS::Context::GetInstance()->GetWindow()->GetAvailableWindowBackends();
            std::unordered_map<LUS::WindowBackend, const char*> availableWindowBackendsMap;
            for (auto& backend : *availableWindowBackends) {
                availableWindowBackendsMap[backend] = windowBackendsMap[backend];
            }

            if (UIWidgets::Combobox(
                    "Renderer API (Needs reload)", &configWindowBackend, availableWindowBackendsMap,
                    { .tooltip = "Sets the renderer API used by the game. Requires a relaunch to take effect.",
                      .disabled = LUS::Context::GetInstance()->GetWindow()->GetAvailableWindowBackends()->size() <= 1,
                      .disabledTooltip = "Only one renderer API is available on this platform." })) {
                LUS::Context::GetInstance()->GetConfig()->SetInt("Window.Backend.Id",
                                                                 static_cast<int32_t>(configWindowBackend));
                LUS::Context::GetInstance()->GetConfig()->SetString("Window.Backend.Name",
                                                                    windowBackendsMap.at(configWindowBackend));
                LUS::Context::GetInstance()->GetConfig()->Save();
            }

            if (LUS::Context::GetInstance()->GetWindow()->CanDisableVerticalSync()) {
                UIWidgets::CVarCheckbox("Enable Vsync", "gVsyncEnabled");
            }

            if (LUS::Context::GetInstance()->GetWindow()->SupportsWindowedFullscreen()) {
                UIWidgets::CVarCheckbox("Windowed fullscreen", "gSdlWindowedFullscreen");
            }

            if (LUS::Context::GetInstance()->GetWindow()->GetGui()->SupportsViewports()) {
                UIWidgets::CVarCheckbox(
                    "Allow multi-windows", "gEnableMultiViewports",
                    { .tooltip = "Allows multiple windows to be opened at once. Requires a reload to take effect." });
            }

            UIWidgets::CVarCombobox("Texture Filter (Needs reload)", "gTextureFilter", textureFilteringMap);

            // Currently this only has "Overlays Text Font", it doesn't use our new UIWidgets so it stands out
            // LUS::Context::GetInstance()->GetWindow()->GetGui()->GetGameOverlay()->DrawSettings();

            ImGui::EndMenu();
        }
    // #region 2S2H [Todo] None of this works yet
        /*
        if (UIWidgets::BeginMenu("Controller")) { */
            if (mInputEditorWindow) {
                UIWidgets::WindowButton("Controller Mapping", "gWindows.InputEditor", mInputEditorWindow);
            }
        /*
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
                .tooltip = "Sets the on screen size of the displayed inputs from the Show Inputs setting",
                .showButtons = false, 
                .format = ""
            });
            UIWidgets::CVarSliderInt("Simulated Input Lag: %d frames", "gSimulatedInputLag", 0, 6, 0, {
                .tooltip = "Buffers your inputs to be executed a specified amount of frames later"
            });

            ImGui::EndMenu();
        }
        ImGui::EndMenu();
        */
        ImGui::EndMenu();
    }
}

extern std::shared_ptr<HudEditorWindow> mHudEditorWindow;

void DrawEnhancementsMenu() {
    if (UIWidgets::BeginMenu("Enhancements")) {
        
        if (UIWidgets::BeginMenu("Masks")) {
            UIWidgets::CVarCheckbox("Fierce Deity's Mask Anywhere", "gEnhancements.Masks.FierceDeitysAnywhere", {
                .tooltip = "Allow using Fierce Deity's mask outside of boss rooms."
            });

            ImGui::EndMenu();
        }
        
        UIWidgets::CVarCheckbox("Fast Text", "gEnhancements.TimeSavers.FastText", {
            .tooltip = "Speeds up text rendering, and enables holding of B progress to next message"
        });
        UIWidgets::CVarCheckbox("Authentic logo", "gEnhancements.General.AuthenticLogo", {
            .tooltip = "Hide the game version and build details and display the authentic model and texture on the boot logo start screen"
        });
        UIWidgets::CVarCheckbox("Skip Entrance Cutscenes", "gEnhancements.TimeSavers.SkipEntranceCutscenes");
        UIWidgets::CVarCheckbox("Hide Title Cards", "gEnhancements.TimeSavers.HideTitleCards");
        UIWidgets::CVarCheckbox("24 Hours Clock", "gEnhancements.General.24HoursClock");

        if (mHudEditorWindow) {
            UIWidgets::WindowButton("Hud Editor", "gWindows.HudEditor", mHudEditorWindow, {
                .tooltip = "Enables the Hud Editor window, allowing you to edit your hud"
            });
        }

        ImGui::EndMenu();
    }
    
}

void DrawCheatsMenu() {
    if (UIWidgets::BeginMenu("Cheats")) {
        UIWidgets::CVarCheckbox("Infinite Health", "gCheats.InfiniteHealth");
        UIWidgets::CVarCheckbox("Infinite Magic", "gCheats.InfiniteMagic");
        UIWidgets::CVarCheckbox("Infinite Rupees", "gCheats.InfiniteRupees");
        UIWidgets::CVarCheckbox("Infinite Consumables", "gCheats.InfiniteConsumables");
        if (UIWidgets::CVarCheckbox("Moon Jump on L", "gCheats.MoonJumpOnL", {
            .tooltip = "Holding L makes you float into the air"
        })) {
            RegisterMoonJumpOnL();
        }
        UIWidgets::CVarCheckbox("No Clip", "gCheats.NoClip");

        ImGui::EndMenu();
    }
}

extern std::shared_ptr<LUS::GuiWindow> mStatsWindow;
extern std::shared_ptr<LUS::GuiWindow> mConsoleWindow;
extern std::shared_ptr<LUS::GuiWindow> mGfxDebuggerWindow;
extern std::shared_ptr<SaveEditorWindow> mSaveEditorWindow;
extern std::shared_ptr<ActorViewerWindow> mActorViewerWindow;
extern std::shared_ptr<CollisionViewerWindow> mCollisionViewerWindow;

void DrawDeveloperToolsMenu() {
    if (UIWidgets::BeginMenu("Developer Tools", UIWidgets::Colors::Yellow)) {
        UIWidgets::CVarCheckbox("Debug Mode", "gDeveloperTools.DebugEnabled", {
            .tooltip = "Enables Debug Mode, allowing you to select maps with L + R + Z."
        });
        
        UIWidgets::CVarCheckbox("Better Map Select", "gDeveloperTools.BetterMapSelect.Enabled");
        if (UIWidgets::CVarCheckbox("Prevent Actor Update", "gDeveloperTools.PreventActorUpdate")) {
            RegisterPreventActorUpdateHooks();
        }
        if (UIWidgets::CVarCheckbox("Prevent Actor Draw", "gDeveloperTools.PreventActorDraw")) {
            RegisterPreventActorDrawHooks();
        }
        if (UIWidgets::CVarCheckbox("Prevent Actor Init", "gDeveloperTools.PreventActorInit")) {
            RegisterPreventActorInitHooks();
        }
        
        if (gPlayState != NULL) {
            ImGui::Separator();
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
        ImGui::Separator();
        if (mCollisionViewerWindow) {
            UIWidgets::WindowButton("Collision Viewer", "gWindows.CollisionViewer", mCollisionViewerWindow,
                { .tooltip = "Draws collision to the screen" });
        }
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
        if (mActorViewerWindow) {
            UIWidgets::WindowButton("Actor Viewer", "gWindows.ActorViewer", mActorViewerWindow, {
                .tooltip = "Enables the Actor Viewer window, allowing you to view actors in the world."
            });
        }
        ImGui::EndMenu();
    }
}

void BenMenuBar::DrawElement() {
    if (ImGui::BeginMenuBar()) {
        DrawMenuBarIcon();

        static ImVec2 sWindowPadding(8.0f, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, sWindowPadding);

        ImGui::SetCursorPosY(0.0f);

        DrawBenMenu();

        ImGui::SetCursorPosY(0.0f);

        DrawSettingsMenu();

        ImGui::SetCursorPosY(0.0f);

        DrawEnhancementsMenu();

        ImGui::SetCursorPosY(0.0f);

        DrawCheatsMenu();

        ImGui::SetCursorPosY(0.0f);

        DrawDeveloperToolsMenu();

        ImGui::SetCursorPosY(0.0f);

        ImGui::PopStyleVar(1);
        ImGui::EndMenuBar();
    }
}
}
// namespace BenGui
