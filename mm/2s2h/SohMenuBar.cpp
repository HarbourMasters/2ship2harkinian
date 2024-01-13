#include "SohMenuBar.h"
#include "ImGui/imgui.h"
#include "public/bridge/consolevariablebridge.h"
#include <libultraship/libultraship.h>
#include "UIWidgets.hpp"
//#include "include/z64audio.h"
//#include "OTRGlobals.h"
#include "z64.h"
//#include "Enhancements/game-interactor/GameInteractor.h"
//#include "soh/Enhancements/presets.h"
//#include "soh/Enhancements/mods.h"
//#include "Enhancements/cosmetics/authenticGfxPatches.h"
#ifdef ENABLE_CROWD_CONTROL
#include "Enhancements/crowd-control/CrowdControl.h"
#endif


//#include "Enhancements/audio/AudioEditor.h"
//#include "Enhancements/controls/GameControlEditor.h"
//#include "Enhancements/cosmetics/CosmeticsEditor.h"
//#include "Enhancements/debugger/actorViewer.h"
//#include "Enhancements/debugger/colViewer.h"
//#include "Enhancements/debugger/debugSaveEditor.h"
//#include "Enhancements/debugger/dlViewer.h"
//#include "Enhancements/gameplaystatswindow.h"
//#include "Enhancements/randomizer/randomizer_check_tracker.h"
//#include "Enhancements/randomizer/randomizer_entrance_tracker.h"
//#include "Enhancements/randomizer/randomizer_item_tracker.h"
//#include "Enhancements/randomizer/randomizer_settings_window.h"

extern bool ShouldClearTextureCacheAtEndOfFrame;
extern bool isBetaQuestEnabled;

extern "C" PlayState* gPlayState;

enum SeqPlayers {
    /* 0 */ SEQ_BGM_MAIN,
    /* 1 */ SEQ_FANFARE,
    /* 2 */ SEQ_SFX,
    /* 3 */ SEQ_BGM_SUB,
    /* 4 */ SEQ_MAX
};

std::string GetWindowButtonText(const char* text, bool menuOpen) {
    char buttonText[100] = "";
    if (menuOpen) {
        strcat(buttonText, ICON_FA_CHEVRON_RIGHT " ");
    }
    strcat(buttonText, text);
    if (!menuOpen) { strcat(buttonText, "  "); }
    return buttonText;
}

    static const char* filters[3] = {
#ifdef __WIIU__
            "",
#else
            "Three-Point",
#endif
            "Linear", "None"
    };

    static const char* chestStyleMatchesContentsOptions[4] = { "Disabled", "Both", "Texture Only", "Size Only" };
    static const char* bunnyHoodOptions[3] = { "Disabled", "Faster Run & Longer Jump", "Faster Run" };
    static const char* mirroredWorldModes[9] = {
        "Disabled",           "Always",        "Random",          "Random (Seeded)",          "Dungeons",
        "Dungeons (Vanilla)", "Dungeons (MQ)", "Dungeons Random", "Dungeons Random (Seeded)",
    };
    static const char* enemyRandomizerModes[3] = { "Disabled", "Random", "Random (Seeded)" };
    static const char* allPowers[9] = {
                        "Vanilla (1x)",
                        "Double (2x)",
                        "Quadruple (4x)",
                        "Octuple (8x)",
                        "Foolish (16x)",
                        "Ridiculous (32x)",
                        "Merciless (64x)",
                        "Pure Torture (128x)",
                        "OHKO (256x)" };
    static const char* subPowers[8] = { allPowers[0], allPowers[1], allPowers[2], allPowers[3], allPowers[4], allPowers[5], allPowers[6], allPowers[7] };
    static const char* subSubPowers[7] = { allPowers[0], allPowers[1], allPowers[2], allPowers[3], allPowers[4], allPowers[5], allPowers[6] };
    static const char* zFightingOptions[3] = { "Disabled", "Consistent Vanish", "No Vanish" };
    static const char* autosaveLabels[6] = { "Off", "New Location + Major Item", "New Location + Any Item", "New Location", "Major Item", "Any Item" };
    static const char* DebugSaveFileModes[3] = { "Off", "Vanilla", "Maxed" };
    static const char* FastFileSelect[5] = { "File N.1", "File N.2", "File N.3", "Zelda Map Select (require OoT Debug Mode)", "File select" };
    static const char* DekuStickCheat[3] = { "Normal", "Unbreakable", "Unbreakable + Always on Fire" };
    static const char* bonkDamageValues[8] = {
        "No Damage",
        "0.25 Heart",
        "0.5 Heart",
        "1 Heart",
        "2 Hearts",
        "4 Hearts",
        "8 Hearts",
        "OHKO"
    };
    static const char* timeTravelOptions[3] = { "Disabled", "Ocarina of Time", "Any Ocarina" };

extern "C" SaveContext gSaveContext;

namespace SohGui {

void DrawMenuBarIcon() {
    static bool gameIconLoaded = false;
    if (!gameIconLoaded) {
        LUS::Context::GetInstance()->GetWindow()->GetGui()->LoadTexture("Game_Icon", "textures/icons/gIcon.png");
        gameIconLoaded = true;
    }

    if (LUS::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName("Game_Icon")) {
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
        ImGui::SetCursorPos(ImVec2(5, 2.5f) * posScale);
        ImGui::Image(LUS::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName("Game_Icon"), iconSize);
        ImGui::SameLine();
        ImGui::SetCursorPos(ImVec2(25, 0) * posScale);
    }
}

void DrawShipMenu() {
    if (ImGui::BeginMenu("Ship")) {
        if (ImGui::MenuItem("Hide Menu Bar",
#if !defined(__SWITCH__) && !defined(__WIIU__)
                            "F1"
#else
                            "[-]"
#endif
                            )) {
            LUS::Context::GetInstance()->GetWindow()->GetGui()->GetMenuBar()->ToggleVisibility();
        }
        UIWidgets::Spacer(0);
#if !defined(__SWITCH__) && !defined(__WIIU__)
        if (ImGui::MenuItem("Toggle Fullscreen", "F11")) {
            LUS::Context::GetInstance()->GetWindow()->ToggleFullscreen();
        }
        UIWidgets::Spacer(0);
#endif
        if (ImGui::MenuItem("Reset",
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
        UIWidgets::Spacer(0);
        if (ImGui::MenuItem("Open App Files Folder")) {
            std::string filesPath = LUS::Context::GetInstance()->GetAppDirectoryPath();
            SDL_OpenURL(std::string("file:///" + std::filesystem::absolute(filesPath).string()).c_str());
        }
        UIWidgets::Spacer(0);

        if (ImGui::MenuItem("Quit")) {
            LUS::Context::GetInstance()->GetWindow()->Close();
        }
#endif
        ImGui::EndMenu();
    }
}

extern std::shared_ptr<LUS::GuiWindow> mInputEditorWindow;
// extern std::shared_ptr<GameControlEditor::GameControlEditorWindow> mGameControlEditorWindow;

void DrawSettingsMenu() {
    if (ImGui::BeginMenu("Settings")) {
        if (ImGui::BeginMenu("Audio")) {
            UIWidgets::PaddedEnhancementSliderFloat("Master Volume: %d %%", "##Master_Vol", "gGameMasterVolume", 0.0f,
                                                    1.0f, "", 1.0f, true, true, false, true);
            if (UIWidgets::PaddedEnhancementSliderFloat("Main Music Volume: %d %%", "##Main_Music_Vol",
                                                        "gMainMusicVolume", 0.0f, 1.0f, "", 1.0f, true, true, false,
                                                        true)) {
                // Audio_SetGameVolume(SEQ_BGM_MAIN, CVarGetFloat("gMainMusicVolume", 1.0f));
            }
            if (UIWidgets::PaddedEnhancementSliderFloat("Sub Music Volume: %d %%", "##Sub_Music_Vol", "gSubMusicVolume",
                                                        0.0f, 1.0f, "", 1.0f, true, true, false, true)) {
                // Audio_SetGameVolume(SEQ_BGM_SUB, CVarGetFloat("gSubMusicVolume", 1.0f));
            }
            if (UIWidgets::PaddedEnhancementSliderFloat("Sound Effects Volume: %d %%", "##Sound_Effect_Vol",
                                                        "gSFXMusicVolume", 0.0f, 1.0f, "", 1.0f, true, true, false,
                                                        true)) {
                // Audio_SetGameVolume(SEQ_SFX, CVarGetFloat("gSFXMusicVolume", 1.0f));
            }
            if (UIWidgets::PaddedEnhancementSliderFloat("Fanfare Volume: %d %%", "##Fanfare_Vol", "gFanfareVolume",
                                                        0.0f, 1.0f, "", 1.0f, true, true, false, true)) {
                // Audio_SetGameVolume(SEQ_FANFARE, CVarGetFloat("gFanfareVolume", 1.0f));
            }

            static std::unordered_map<LUS::AudioBackend, const char*> audioBackendNames = {
                { LUS::AudioBackend::WASAPI, "Windows Audio Session API" },
                { LUS::AudioBackend::PULSE, "PulseAudio" },
                { LUS::AudioBackend::SDL, "SDL" },
            };

            ImGui::Text("Audio API (Needs reload)");
            auto currentAudioBackend = LUS::Context::GetInstance()->GetAudio()->GetAudioBackend();

            if (LUS::Context::GetInstance()->GetAudio()->GetAvailableAudioBackends()->size() <= 1) {
                UIWidgets::DisableComponent(ImGui::GetStyle().Alpha * 0.5f);
            }
            if (ImGui::BeginCombo("##AApi", audioBackendNames[currentAudioBackend])) {
                for (uint8_t i = 0; i < LUS::Context::GetInstance()->GetAudio()->GetAvailableAudioBackends()->size();
                     i++) {
                    auto backend = LUS::Context::GetInstance()->GetAudio()->GetAvailableAudioBackends()->data()[i];
                    if (ImGui::Selectable(audioBackendNames[backend], backend == currentAudioBackend)) {
                        LUS::Context::GetInstance()->GetAudio()->SetAudioBackend(backend);
                    }
                }
                ImGui::EndCombo();
            }
            if (LUS::Context::GetInstance()->GetAudio()->GetAvailableAudioBackends()->size() <= 1) {
                UIWidgets::ReEnableComponent("");
            }

            ImGui::EndMenu();
        }

        UIWidgets::Spacer(0);

        if (ImGui::BeginMenu("Controller")) {
            //ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 6.0f));
            //ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.0f));
            //ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            //ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.22f, 0.38f, 0.56f, 1.0f));
            if (mInputEditorWindow) {
                if (ImGui::Button(
                        GetWindowButtonText("Controller Mapping", CVarGetInteger("gControllerConfigurationEnabled", 0))
                            .c_str(),
                        ImVec2(-1.0f, 0.0f))) {
                    mInputEditorWindow->ToggleVisibility();
                }
            }
            // if (mGameControlEditorWindow) {
            // if (ImGui::Button(GetWindowButtonText("Additional Controller Options",
            // CVarGetInteger("gGameControlEditorEnabled", 0)).c_str(), ImVec2(-1.0f, 0.0f))) {
            // mGameControlEditorWindow->ToggleVisibility();
            //}

        ImGui::EndMenu();

        }
        UIWidgets::PaddedSeparator();
        //ImGui::PopStyleColor(1);
        //ImGui::PopStyleVar(3);
#ifndef __SWITCH__
        UIWidgets::EnhancementCheckbox("Menubar Controller Navigation", "gControlNav");
        UIWidgets::Tooltip("Allows controller navigation of the SOH menu bar (Settings, Enhancements,...)\nCAUTION: "
                           "This will disable game inputs while the menubar is visible.\n\nD-pad to move between "
                           "items, A to select, and X to grab focus on the menu bar");
#endif
        UIWidgets::PaddedEnhancementCheckbox("Show Inputs", "gInputEnabled", true, false);
        UIWidgets::Tooltip("Shows currently pressed inputs on the bottom right of the screen");
        UIWidgets::PaddedEnhancementSliderFloat("Input Scale: %.1f", "##Input", "gInputScale", 1.0f, 3.0f, "", 1.0f,
                                                false, true, true, false);
        UIWidgets::Tooltip("Sets the on screen size of the displayed inputs from the Show Inputs setting");
        UIWidgets::PaddedEnhancementSliderInt("Simulated Input Lag: %d frames", "##SimulatedInputLag",
                                              "gSimulatedInputLag", 0, 6, "", 0, true, true, false);
        UIWidgets::Tooltip("Buffers your inputs to be executed a specified amount of frames later");

        ImGui::EndMenu();
    }

    UIWidgets::Spacer(0);

    if (ImGui::BeginMenu("Graphics")) {
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

        static std::unordered_map<LUS::WindowBackend, const char*> windowBackendNames = {
            { LUS::WindowBackend::DX11, "DirectX" },
            { LUS::WindowBackend::SDL_OPENGL, "OpenGL" },
            { LUS::WindowBackend::SDL_METAL, "Metal" },
            { LUS::WindowBackend::GX2, "GX2" }
        };

        ImGui::Text("Renderer API (Needs reload)");
        LUS::WindowBackend runningWindowBackend = LUS::Context::GetInstance()->GetWindow()->GetWindowBackend();
        LUS::WindowBackend configWindowBackend;
        int configWindowBackendId = LUS::Context::GetInstance()->GetConfig()->GetInt("Window.Backend.Id", -1);
        if (configWindowBackendId != -1 &&
            configWindowBackendId < static_cast<int>(LUS::WindowBackend::BACKEND_COUNT)) {
            configWindowBackend = static_cast<LUS::WindowBackend>(configWindowBackendId);
        } else {
            configWindowBackend = runningWindowBackend;
        }

        if (LUS::Context::GetInstance()->GetWindow()->GetAvailableWindowBackends()->size() <= 1) {
            UIWidgets::DisableComponent(ImGui::GetStyle().Alpha * 0.5f);
        }
        if (ImGui::BeginCombo("##RApi", windowBackendNames[configWindowBackend])) {
            for (size_t i = 0; i < LUS::Context::GetInstance()->GetWindow()->GetAvailableWindowBackends()->size();
                 i++) {
                auto backend = LUS::Context::GetInstance()->GetWindow()->GetAvailableWindowBackends()->data()[i];
                if (ImGui::Selectable(windowBackendNames[backend], backend == configWindowBackend)) {
                    LUS::Context::GetInstance()->GetConfig()->SetInt("Window.Backend.Id", static_cast<int>(backend));
                    LUS::Context::GetInstance()->GetConfig()->SetString("Window.Backend.Name",
                                                                        windowBackendNames[backend]);
                    LUS::Context::GetInstance()->GetConfig()->Save();
                }
            }
            ImGui::EndCombo();
        }
        if (LUS::Context::GetInstance()->GetWindow()->GetAvailableWindowBackends()->size() <= 1) {
            UIWidgets::ReEnableComponent("");
        }

        if (LUS::Context::GetInstance()->GetWindow()->CanDisableVerticalSync()) {
            UIWidgets::PaddedEnhancementCheckbox("Enable Vsync", "gVsyncEnabled", true, false);
        }

        if (LUS::Context::GetInstance()->GetWindow()->SupportsWindowedFullscreen()) {
            UIWidgets::PaddedEnhancementCheckbox("Windowed fullscreen", "gSdlWindowedFullscreen", true, false);
        }

        if (LUS::Context::GetInstance()->GetWindow()->GetGui()->SupportsViewports()) {
            UIWidgets::PaddedEnhancementCheckbox("Allow multi-windows", "gEnableMultiViewports", true, false, false, "",
                                                 UIWidgets::CheckboxGraphics::Cross, true);
            UIWidgets::Tooltip("Allows windows to be able to be dragged off of the main game window. Requires a reload "
                               "to take effect.");
        }

        // If more filters are added to LUS, make sure to add them to the filters list here
        ImGui::Text("Texture Filter (Needs reload)");

        UIWidgets::EnhancementCombobox("gTextureFilter", filters, FILTER_THREE_POINT);

        UIWidgets::Spacer(0);

        LUS::Context::GetInstance()->GetWindow()->GetGui()->GetGameOverlay()->DrawSettings();

        ImGui::EndMenu();
    }

    UIWidgets::Spacer(0);

    if (ImGui::BeginMenu("Languages")) {
        UIWidgets::PaddedEnhancementCheckbox("Translate Title Screen", "gTitleScreenTranslation");
        if (UIWidgets::EnhancementRadioButton("English", "gLanguages", LANGUAGE_ENG)) {
            // GameInteractor::Instance->ExecuteHooks<GameInteractor::OnSetGameLanguage>();
        }
        if (UIWidgets::EnhancementRadioButton("German", "gLanguages", LANGUAGE_GER)) {
            // GameInteractor::Instance->ExecuteHooks<GameInteractor::OnSetGameLanguage>();
        }
        // if (UIWidgets::EnhancementRadioButton("French", "gLanguages", LANGUAGE_FRA)) {
        // GameInteractor::Instance->ExecuteHooks<GameInteractor::OnSetGameLanguage>();
        //}
        ImGui::EndMenu();
    }

    UIWidgets::Spacer(0);

    if (ImGui::BeginMenu("Accessibility")) {
#if defined(_WIN32) || defined(__APPLE__)
        UIWidgets::PaddedEnhancementCheckbox("Text to Speech", "gA11yTTS");
        UIWidgets::Tooltip("Enables text to speech for in game dialog");
#endif
        UIWidgets::PaddedEnhancementCheckbox("Disable Idle Camera Re-Centering", "gA11yDisableIdleCam");
        UIWidgets::Tooltip("Disables the automatic re-centering of the camera when idle.");

        ImGui::EndMenu();
    }

    //ImGui::EndMenu();
}

extern std::shared_ptr<LUS::GuiWindow> mStatsWindow;
extern std::shared_ptr<LUS::GuiWindow> mConsoleWindow;
extern std::shared_ptr<LUS::GuiWindow> mGfxDebuggerWindow;
extern std::shared_ptr<SaveEditorWindow> mSaveEditorWindow;
// extern std::shared_ptr<ColViewerWindow> mColViewerWindow;
// extern std::shared_ptr<ActorViewerWindow> mActorViewerWindow;
// extern std::shared_ptr<DLViewerWindow> mDLViewerWindow;

void DrawDeveloperToolsMenu() {
    if (ImGui::BeginMenu("Developer Tools")) {
        UIWidgets::EnhancementCheckbox("OoT Debug Mode", "gDebugEnabled");
        UIWidgets::Tooltip("Enables Debug Mode, allowing you to select maps with L + R + Z, noclip with L + D-pad "
                           "Right, and open the debug menu with L on the pause screen");
        if (CVarGetInteger("gDebugEnabled", 0)) {
            ImGui::Text("Debug Save File Mode:");
            UIWidgets::EnhancementCombobox("gDebugSaveFileMode", DebugSaveFileModes, 1);
            UIWidgets::Tooltip("Changes the behaviour of debug file select creation (creating a save file on slot 1 "
                               "with debug mode on)\n"
                               "- Off: The debug save file will be a normal savefile\n"
                               "- Vanilla: The debug save file will be the debug save file from the original game\n"
                               "- Maxed: The debug save file will be a save file with all of the items & upgrades");
        }
        UIWidgets::PaddedEnhancementCheckbox("OoT Skulltula Debug", "gSkulltulaDebugEnabled", true, false);
        UIWidgets::Tooltip("Enables Skulltula Debug, when moving the cursor in the menu above various map icons (boss "
                           "key, compass, map screen locations, etc) will set the GS bits in that area.\nUSE WITH "
                           "CAUTION AS IT DOES NOT UPDATE THE GS COUNT.");
        UIWidgets::PaddedEnhancementCheckbox("Fast File Select", "gSkipLogoTitle", true, false);
        UIWidgets::Tooltip(
            "Load the game to the selected menu or file\n\"Zelda Map Select\" require debug mode else you will "
            "fallback to File choose menu\nUsing a file number that don't have save will create a save file only if "
            "you toggle on \"Create a new save if none ?\" else it will bring you to the File choose menu");
        if (CVarGetInteger("gSkipLogoTitle", 0)) {
            ImGui::Text("Loading:");
            UIWidgets::EnhancementCombobox("gSaveFileID", FastFileSelect, 0);
        };
        UIWidgets::PaddedEnhancementCheckbox("Better Debug Warp Screen", "gBetterDebugWarpScreen", true, false);
        UIWidgets::Tooltip("Optimized debug warp screen, with the added ability to chose entrances and time of day");
        UIWidgets::PaddedEnhancementCheckbox("Debug Warp Screen Translation", "gDebugWarpScreenTranslation", true,
                                             false, false, "", UIWidgets::CheckboxGraphics::Cross, true);
        UIWidgets::Tooltip("Translate the Debug Warp Screen based on the game language");
        if (gPlayState != NULL) {
            UIWidgets::PaddedSeparator();
            ImGui::Checkbox("Frame Advance##frameAdvance", (bool*)&gPlayState->frameAdvCtx.enabled);
            UIWidgets::Tooltip("This allows you to advance through the game one frame at a time on command. "
                               "To advance a frame, hold Z and tap R on the second controller. Holding Z "
                               "and R will advance a frame every half second. You can also use the buttons below.");
            if (gPlayState->frameAdvCtx.enabled) {
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 6.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0,0));
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.22f, 0.38f, 0.56f, 1.0f));
                if (ImGui::Button("Advance 1", ImVec2(90.0f, 0.0f))) {
                    CVarSetInteger("gDeveloperTools.FrameAdvanceTick", 1);
                }
                ImGui::SameLine();
                ImGui::Button("Advance (Hold)");
                if (ImGui::IsItemActive()) {
                    CVarSetInteger("gDeveloperTools.FrameAdvanceTick", 1);
                }
                ImGui::PopStyleVar(3);
                ImGui::PopStyleColor(1);
            }
        }
        UIWidgets::PaddedEnhancementCheckbox("Moon Jump on L", "gDeveloperTools.MoonJumpOnL", true, false);
        UIWidgets::Tooltip("Holding L makes you float into the air");
        UIWidgets::PaddedSeparator();
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 6.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.22f, 0.38f, 0.56f, 1.0f));
        if (mStatsWindow) {
            if (ImGui::Button(GetWindowButtonText("Stats", CVarGetInteger("gStatsEnabled", 0)).c_str(),
                              ImVec2(-1.0f, 0.0f))) {
                mStatsWindow->ToggleVisibility();
            }
            UIWidgets::Tooltip("Shows the stats window, with your FPS and frametimes, and the OS you're playing on");
        }
        UIWidgets::Spacer(0);
        if (mConsoleWindow) {
            if (ImGui::Button(GetWindowButtonText("Console", CVarGetInteger("gConsoleEnabled", 0)).c_str(),
                              ImVec2(-1.0f, 0.0f))) {
                mConsoleWindow->ToggleVisibility();
            }
            UIWidgets::Tooltip(
                "Enables the console window, allowing you to input commands, type help for some examples");
        }
        UIWidgets::Spacer(0);
        if (mGfxDebuggerWindow) {
            if (ImGui::Button(GetWindowButtonText("Gfx Debugger", CVarGetInteger("gGfxDebuggerEnabled", 0)).c_str(),
                              ImVec2(-1.0f, 0.0f))) {
                mGfxDebuggerWindow->ToggleVisibility();
            }
            UIWidgets::Tooltip(
                "Enables the Gfx Debugger window, allowing you to input commands, type help for some examples");
        }
        UIWidgets::Spacer(0);

        if (mSaveEditorWindow) {
            if (ImGui::Button(GetWindowButtonText("Save Editor", CVarGetInteger("gSaveEditorEnabled", 0)).c_str(), ImVec2(-1.0f, 0.0f))) {
                mSaveEditorWindow->ToggleVisibility();
            }
        }
        // UIWidgets::Spacer(0);
        // if (mColViewerWindow) {
        //     if (ImGui::Button(GetWindowButtonText("Collision Viewer", CVarGetInteger("gCollisionViewerEnabled",
        //     0)).c_str(), ImVec2(-1.0f, 0.0f))) {
        //         //mColViewerWindow->ToggleVisibility();
        //     }
        // }
        // UIWidgets::Spacer(0);
        // if (mActorViewerWindow) {
        //     if (ImGui::Button(GetWindowButtonText("Actor Viewer", CVarGetInteger("gActorViewerEnabled", 0)).c_str(),
        //     ImVec2(-1.0f, 0.0f))) {
        //         //mActorViewerWindow->ToggleVisibility();
        //     }
        // }
        // UIWidgets::Spacer(0);
        // if (mDLViewerWindow) {
        //     if (ImGui::Button(GetWindowButtonText("Display List Viewer", CVarGetInteger("gDLViewerEnabled",
        //     0)).c_str(), ImVec2(-1.0f, 0.0f))) {
        //         //mDLViewerWindow->ToggleVisibility();
        //     }
        // }

        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(1);

        ImGui::EndMenu();
    }
}

void SohMenuBar::DrawElement() {
    if (ImGui::BeginMenuBar()) {
        static ImVec2 sWindowPadding(8.0f, 8.0f);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, sWindowPadding);

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
 // namespace SohGui 