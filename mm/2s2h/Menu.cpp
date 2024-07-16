#include "Menu.h"
#include "BenPort.h"
#include "2s2h/BenGui/BenGui.hpp"
#include "2s2h/BenGui/UIWidgets.hpp"
#include "graphic/Fast3D/gfx_rendering_api.h"
#include "2s2h/Enhancements/Enhancements.h"
#include "2s2h/Enhancements/Graphics/MotionBlur.h"
#include "2s2h/Enhancements/Graphics/PlayAsKafei.h"
#include "2s2h/DeveloperTools/DeveloperTools.h"
#include "window/gui/GuiMenuBar.h"
#include "window/gui/GuiElement.h"
#include "DeveloperTools/SaveEditor.h"
#include "DeveloperTools/ActorViewer.h"
#include "DeveloperTools/CollisionViewer.h"
#include "DeveloperTools/EventLog.h"
#include "2s2h/BenGui/HudEditor.h"

extern "C" {
#include "z64.h"
#include "functions.h"
extern PlayState* gPlayState;
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
    { Ship::WindowBackend::FAST3D_DXGI_DX11, "DirectX" },
    { Ship::WindowBackend::FAST3D_SDL_OPENGL, "OpenGL" },
    { Ship::WindowBackend::FAST3D_SDL_METAL, "Metal" },
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

const char* logLevels[] = {
    "trace", "debug", "info", "warn", "error", "critical", "off",
};

namespace BenGui {

extern std::shared_ptr<HudEditorWindow> mHudEditorWindow;
extern std::shared_ptr<Ship::GuiWindow> mStatsWindow;
extern std::shared_ptr<Ship::GuiWindow> mConsoleWindow;
extern std::shared_ptr<Ship::GuiWindow> mGfxDebuggerWindow;
extern std::shared_ptr<SaveEditorWindow> mSaveEditorWindow;
extern std::shared_ptr<ActorViewerWindow> mActorViewerWindow;
extern std::shared_ptr<CollisionViewerWindow> mCollisionViewerWindow;
extern std::shared_ptr<EventLogWindow> mEventLogWindow;
extern std::shared_ptr<BenInputEditorWindow> mBenInputEditorWindow;
extern std::shared_ptr<std::vector<Ship::WindowBackend>> availableWindowBackends;
extern std::unordered_map<Ship::WindowBackend, const char*> availableWindowBackendsMap;
extern Ship::WindowBackend configWindowBackend;

extern void UpdateWindowBackendObjects();

void DrawGeneralSettings() {
#if not defined(__SWITCH__) and not defined(__WIIU__)
    UIWidgets::CVarCheckbox(
        "Menubar Controller Navigation", CVAR_IMGUI_CONTROLLER_NAV,
        { .tooltip = "Allows controller navigation of the SOH menu bar (Settings, Enhancements,...)\nCAUTION: "
                     "This will disable game inputs while the menubar is visible.\n\nD-pad to move between "
                     "items, A to select, and X to grab focus on the menu bar" });
    bool cursor = Ship::Context::GetInstance()->GetWindow()->ShouldForceCursorVisibility();
    if (UIWidgets::Checkbox("Cursor Always Visible", &cursor,
                            { .tooltip = "Makes the cursor always visible, even in full screen." })) {
        Ship::Context::GetInstance()->GetWindow()->SetForceCursorVisibility(cursor);
    }
#endif
    UIWidgets::CVarCheckbox("Hide Menu Hotkey Text", "gSettings.DisableMenuShortcutNotify",
                            { .tooltip = "Prevents showing the text telling you the shortcut to open the menu\n"
                                         "when the menu isn't open." });
}

void DrawAudioSettings() {
    UIWidgets::CVarSliderFloat("Master Volume: %.0f %%", "gSettings.Audio.MasterVolume", 0.0f, 1.0f, 1.0f,
                               { .showButtons = false, .format = "", .isPercentage = true });

    if (UIWidgets::CVarSliderFloat("Main Music Volume: %.0f %%", "gSettings.Audio.MainMusicVolume", 0.0f, 1.0f, 1.0f,
                                   { .showButtons = false, .format = "", .isPercentage = true })) {
        AudioSeq_SetPortVolumeScale(SEQ_PLAYER_BGM_MAIN, CVarGetFloat("gSettings.Audio.MainMusicVolume", 1.0f));
    }
    if (UIWidgets::CVarSliderFloat("Sub Music Volume: %.0f %%", "gSettings.Audio.SubMusicVolume", 0.0f, 1.0f, 1.0f,
                                   { .showButtons = false, .format = "", .isPercentage = true })) {
        AudioSeq_SetPortVolumeScale(SEQ_PLAYER_BGM_SUB, CVarGetFloat("gSettings.Audio.SubMusicVolume", 1.0f));
    }
    if (UIWidgets::CVarSliderFloat("Sound Effects Volume: %.0f %%", "gSettings.Audio.SoundEffectsVolume", 0.0f, 1.0f,
                                   1.0f, { .showButtons = false, .format = "", .isPercentage = true })) {
        AudioSeq_SetPortVolumeScale(SEQ_PLAYER_SFX, CVarGetFloat("gSettings.Audio.SoundEffectsVolume", 1.0f));
    }
    if (UIWidgets::CVarSliderFloat("Fanfare Volume: %.0f %%", "gSettings.Audio.FanfareVolume", 0.0f, 1.0f, 1.0f,
                                   { .showButtons = false, .format = "", .isPercentage = true })) {
        AudioSeq_SetPortVolumeScale(SEQ_PLAYER_FANFARE, CVarGetFloat("gSettings.Audio.FanfareVolume", 1.0f));
    }
    if (UIWidgets::CVarSliderFloat("Ambience Volume: %.0f %%", "gSettings.Audio.AmbienceVolume", 0.0f, 1.0f, 1.0f,
                                   { .showButtons = false, .format = "", .isPercentage = true })) {
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
    bool fs = Ship::Context::GetInstance()->GetWindow()->IsFullscreen();
    if (UIWidgets::Checkbox("Toggle Fullscreen", &fs)) {
        Ship::Context::GetInstance()->GetWindow()->ToggleFullscreen();
    }
#ifndef __APPLE__
    if (UIWidgets::CVarSliderFloat("Internal Resolution: %f %%", CVAR_INTERNAL_RESOLUTION, 0.5f, 2.0f, 1.0f)) {
        Ship::Context::GetInstance()->GetWindow()->SetResolutionMultiplier(CVarGetFloat(CVAR_INTERNAL_RESOLUTION, 1));
    };
    UIWidgets::Tooltip("Multiplies your output resolution by the value inputted, as a more intensive but effective "
                       "form of anti-aliasing");
#endif
#ifndef __WIIU__
    if (UIWidgets::CVarSliderInt((CVarGetInteger(CVAR_MSAA_VALUE, 1) == 1) ? "Anti-aliasing (MSAA): Off"
                                                                           : "Anti-aliasing (MSAA): %d",
                                 CVAR_MSAA_VALUE, 1, 8, 1)) {
        Ship::Context::GetInstance()->GetWindow()->SetMsaaLevel(CVarGetInteger(CVAR_MSAA_VALUE, 1));
    };
    UIWidgets::Tooltip("Activates MSAA (multi-sample anti-aliasing) from 2x up to 8x, to smooth the edges of rendered "
                       "geometry.\n"
                       "Higher sample count will result in smoother edges on models, but may reduce performance.");
#endif

    { // FPS Slider
        constexpr unsigned int minFps = 20;
        static unsigned int maxFps;
        if (Ship::Context::GetInstance()->GetWindow()->GetWindowBackend() == Ship::WindowBackend::FAST3D_DXGI_DX11) {
            maxFps = 360;
        } else {
            maxFps = Ship::Context::GetInstance()->GetWindow()->GetCurrentRefreshRate();
        }
        unsigned int currentFps = std::max(std::min(OTRGlobals::Instance->GetInterpolationFPS(), maxFps), minFps);
        bool matchingRefreshRate =
            CVarGetInteger("gMatchRefreshRate", 0) &&
            Ship::Context::GetInstance()->GetWindow()->GetWindowBackend() != Ship::WindowBackend::FAST3D_DXGI_DX11;
        UIWidgets::CVarSliderInt((currentFps == 20) ? "FPS: Original (20)" : "FPS: %d", "gInterpolationFPS", minFps,
                                 maxFps, 20, { .disabled = matchingRefreshRate });
        if (Ship::Context::GetInstance()->GetWindow()->GetWindowBackend() == Ship::WindowBackend::FAST3D_DXGI_DX11) {
            UIWidgets::Tooltip(
                "Uses Matrix Interpolation to create extra frames, resulting in smoother graphics. This is "
                "purely visual and does not impact game logic, execution of glitches etc.\n\n A higher target "
                "FPS than your monitor's refresh rate will waste resources, and might give a worse result.");
        } else {
            UIWidgets::Tooltip(
                "Uses Matrix Interpolation to create extra frames, resulting in smoother graphics. This is "
                "purely visual and does not impact game logic, execution of glitches etc.");
        }
    } // END FPS Slider

    if (Ship::Context::GetInstance()->GetWindow()->GetWindowBackend() == Ship::WindowBackend::FAST3D_DXGI_DX11) {
        // UIWidgets::Spacer(0);
        if (ImGui::Button("Match Refresh Rate")) {
            int hz = Ship::Context::GetInstance()->GetWindow()->GetCurrentRefreshRate();
            if (hz >= 20 && hz <= 360) {
                CVarSetInteger("gInterpolationFPS", hz);
                Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
            }
        }
    } else {
        UIWidgets::CVarCheckbox("Match Refresh Rate", "gMatchRefreshRate");
    }
    UIWidgets::Tooltip("Matches interpolation value to the current game's window refresh rate");

    if (Ship::Context::GetInstance()->GetWindow()->GetWindowBackend() == Ship::WindowBackend::FAST3D_DXGI_DX11) {
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

    if (UIWidgets::Combobox("Renderer API (Needs reload)", &configWindowBackend, availableWindowBackendsMap,
                            { .tooltip = "Sets the renderer API used by the game. Requires a relaunch to take effect.",
                              .disabled = availableWindowBackends->size() <= 1,
                              .disabledTooltip = "Only one renderer API is available on this platform." })) {
        Ship::Context::GetInstance()->GetConfig()->SetInt("Window.Backend.Id",
                                                          static_cast<int32_t>(configWindowBackend));
        Ship::Context::GetInstance()->GetConfig()->SetString("Window.Backend.Name",
                                                             windowBackendsMap.at(configWindowBackend));
        Ship::Context::GetInstance()->GetConfig()->Save();
        UpdateWindowBackendObjects();
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

void DrawControllerSettings() {
    UIWidgets::WindowButton("Popout Input Editor", "gWindows.BenInputEditor", mBenInputEditorWindow,
                            { .tooltip = "Enables the separate Input Editor window." });
    if (!CVarGetInteger("gWindows.BenInputEditor", 0)) {
        mBenInputEditorWindow->DrawPortTabContents(0);
    }
};

// Camera
void DrawCameraEnhancements1() {
    ImGui::SeparatorText("Fixes");
    UIWidgets::CVarCheckbox(
        "Fix Targetting Camera Snap", "gEnhancements.Camera.FixTargettingCameraSnap",
        { .tooltip = "Fixes the camera snap that occurs when you are moving and press the targetting button." });
}

void DrawCameraEnhancements2() {
    ImGui::SeparatorText("Free Look");
    if (UIWidgets::CVarCheckbox(
            "Free Look", "gEnhancements.Camera.FreeLook.Enable",
            { .tooltip = "Enables free look camera control\nNote: You must remap C buttons off of the right "
                         "stick in the controller config menu, and map the camera stick to the right stick.",
              .disabled = CVarGetInteger("gEnhancements.Camera.DebugCam.Enable", 0) != 0 })) {
        RegisterCameraFreeLook();
    }

    if (CVarGetInteger("gEnhancements.Camera.FreeLook.Enable", 0)) {
        UIWidgets::CVarCheckbox("Invert Camera X Axis", "gEnhancements.Camera.RightStick.InvertXAxis",
                                { .tooltip = "Inverts the Camera X Axis" });
        UIWidgets::CVarCheckbox("Invert Camera Y Axis", "gEnhancements.Camera.RightStick.InvertYAxis",
                                { .tooltip = "Inverts the Camera Y Axis", .defaultValue = true });
        UIWidgets::CVarSliderFloat("Third-Person Horizontal Sensitivity: %.0f",
                                   "gEnhancements.Camera.RightStick.CameraSensitivity.X", 0.01f, 5.0f, 1.0f);
        UIWidgets::CVarSliderFloat("Third-Person Vertical Sensitivity: %.0f",
                                   "gEnhancements.Camera.RightStick.CameraSensitivity.Y", 0.01f, 5.0f, 1.0f);

        UIWidgets::CVarSliderInt("Camera Distance: %d", "gEnhancements.Camera.FreeLook.MaxCameraDistance", 100, 900,
                                 185);
        UIWidgets::CVarSliderInt("Camera Transition Speed: %d", "gEnhancements.Camera.FreeLook.TransitionSpeed", 1, 900,
                                 25);
        UIWidgets::CVarSliderFloat("Max Camera Height Angle: %.0f°", "gEnhancements.Camera.FreeLook.MaxPitch", -89.0f,
                                   89.0f, 72.0f);
        UIWidgets::CVarSliderFloat("Min Camera Height Angle: %.0f°", "gEnhancements.Camera.FreeLook.MinPitch", -89.0f,
                                   89.0f, -49.0f);
        f32 maxY = CVarGetFloat("gEnhancements.Camera.FreeLook.MaxPitch", 72.0f);
        f32 minY = CVarGetFloat("gEnhancements.Camera.FreeLook.MinPitch", -49.0f);
        CVarSetFloat("gEnhancements.Camera.FreeLook.MaxPitch", std::max(maxY, minY));
        CVarSetFloat("gEnhancements.Camera.FreeLook.MinPitch", std::min(maxY, minY));
    }
}

void DrawCameraEnhancements3() {
    ImGui::SeparatorText("'Debug' Camera");
    if (UIWidgets::CVarCheckbox("Debug Camera", "gEnhancements.Camera.DebugCam.Enable",
                                { .tooltip = "Enables free camera control.",
                                  .disabled = CVarGetInteger("gEnhancements.Camera.FreeLook.Enable", 0) != 0 })) {
        RegisterDebugCam();
    }

    if (CVarGetInteger("gEnhancements.Camera.DebugCam.Enable", 0)) {
        UIWidgets::CVarCheckbox("Invert Camera X Axis", "gEnhancements.Camera.RightStick.InvertXAxis",
                                { .tooltip = "Inverts the Camera X Axis" });
        UIWidgets::CVarCheckbox("Invert Camera Y Axis", "gEnhancements.Camera.RightStick.InvertYAxis",
                                { .tooltip = "Inverts the Camera Y Axis", .defaultValue = true });
        UIWidgets::CVarSliderFloat("Third-Person Horizontal Sensitivity: %.0f",
                                   "gEnhancements.Camera.RightStick.CameraSensitivity.X", 0.01f, 5.0f, 1.0f);
        UIWidgets::CVarSliderFloat("Third-Person Vertical Sensitivity: %.0f",
                                   "gEnhancements.Camera.RightStick.CameraSensitivity.Y", 0.01f, 5.0f, 1.0f);

        UIWidgets::CVarCheckbox(
            "Enable Roll (Six Degrees Of Freedom)", "gEnhancements.Camera.DebugCam.6DOF",
            { .tooltip = "This allows for all six degrees of movement with the camera, NOTE: Yaw will work "
                         "differently in "
                         "this system, instead rotating around the focal point, rather than a polar axis." });
        UIWidgets::CVarSliderFloat("Camera Speed: %.0f", "gEnhancements.Camera.DebugCam.CameraSpeed", 0.1f, 3.0f, 0.5f);
    }
}

// Cheats
void DrawCheatEnhancements() {
    UIWidgets::CVarCheckbox("Infinite Health", "gCheats.InfiniteHealth");
    UIWidgets::CVarCheckbox("Infinite Magic", "gCheats.InfiniteMagic");
    UIWidgets::CVarCheckbox("Infinite Rupees", "gCheats.InfiniteRupees");
    UIWidgets::CVarCheckbox("Infinite Consumables", "gCheats.InfiniteConsumables");
    if (UIWidgets::CVarCheckbox(
            "Longer Deku Flower Glide", "gCheats.LongerFlowerGlide",
            { .tooltip = "Allows Deku Link to glide longer, no longer dropping after a certain distance" })) {
        RegisterLongerFlowerGlide();
    }
    UIWidgets::CVarCheckbox("No Clip", "gCheats.NoClip");
    UIWidgets::CVarCheckbox("Unbreakable Razor Sword", "gCheats.UnbreakableRazorSword");
    UIWidgets::CVarCheckbox("Unrestricted Items", "gCheats.UnrestrictedItems");
    if (UIWidgets::CVarCheckbox("Moon Jump on L", "gCheats.MoonJumpOnL",
                                { .tooltip = "Holding L makes you float into the air" })) {
        RegisterMoonJumpOnL();
    }
}

// Gameplay
void DrawGameplayEnhancements() {
    ImGui::SeparatorText("Player");
    if (UIWidgets::CVarCheckbox("Fast Deku Flower Launch", "gEnhancements.Player.FastFlowerLaunch",
                                { .tooltip = "Speeds up the time it takes to be able to get maximum height from "
                                             "launching out of a deku flower" })) {
        RegisterFastFlowerLaunch();
    }
    UIWidgets::CVarCheckbox("Instant Putaway", "gEnhancements.Player.InstantPutaway",
                            { .tooltip = "Allows Link to instantly puts away held item without waiting." });
    UIWidgets::CVarSliderInt("Climb speed", "gEnhancements.PlayerMovement.ClimbSpeed", 1, 5, 1,
                             { .tooltip = "Increases the speed at which Link climbs vines and ladders." });
    UIWidgets::CVarCheckbox("Dpad Equips", "gEnhancements.Dpad.DpadEquips",
                            { .tooltip = "Allows you to equip items to your d-pad" });
    UIWidgets::CVarCombobox("Always Win Doggy Race", "gEnhancements.Minigames.AlwaysWinDoggyRace",
                            alwaysWinDoggyraceOptions);
}

void DrawGameModesEnhancements() {
    ImGui::SeparatorText("Modes");
    UIWidgets::CVarCheckbox("Play As Kafei", "gModes.PlayAsKafei",
                            { .tooltip = "Requires scene reload to take effect." });
    if (UIWidgets::CVarCheckbox("Time Moves When You Move", "gModes.TimeMovesWhenYouMove")) {
        RegisterTimeMovesWhenYouMove();
    }
}

void DrawSaveTimeEnhancements() {
    ImGui::SeparatorText("Saving");
    UIWidgets::CVarCheckbox("Persistent Owl Saves", "gEnhancements.Saving.PersistentOwlSaves",
                            { .tooltip = "Continuing a save will not remove the owl save. Playing Song of "
                                         "Time, allowing the moon to crash or finishing the "
                                         "game will remove the owl save and become the new last save." });
    UIWidgets::CVarCheckbox(
        "Pause Menu Save", "gEnhancements.Saving.PauseSave",
        { .tooltip = "Re-introduce the pause menu save system. Pressing B in the pause menu will give you the "
                     "option to create a persistent Owl Save from your current location.\n\nWhen loading back "
                     "into the game, you will be placed either at the entrance of the dungeon you saved in, or "
                     "in South Clock Town." });
    if (UIWidgets::CVarCheckbox(
            "Autosave", "gEnhancements.Saving.Autosave",
            { .tooltip = "Automatically create a persistent Owl Save on the chosen interval.\n\nWhen loading "
                         "back into the game, you will be placed either at the entrance of the dungeon you "
                         "saved in, or in South Clock Town." })) {
        RegisterAutosave();
    }
    UIWidgets::CVarSliderInt("Autosave Interval (minutes): %d", "gEnhancements.Saving.AutosaveInterval", 1, 60, 5,
                             { .disabled = !CVarGetInteger("gEnhancements.Saving.Autosave", 0) });

    ImGui::SeparatorText("Time Cycle");
    UIWidgets::CVarCheckbox("Do not reset Bottle content", "gEnhancements.Cycle.DoNotResetBottleContent",
                            { .tooltip = "Playing the Song Of Time will not reset the bottles' content." });
    UIWidgets::CVarCheckbox("Do not reset Consumables", "gEnhancements.Cycle.DoNotResetConsumables",
                            { .tooltip = "Playing the Song Of Time will not reset the consumables." });
    UIWidgets::CVarCheckbox("Do not reset Razor Sword", "gEnhancements.Cycle.DoNotResetRazorSword",
                            { .tooltip = "Playing the Song Of Time will not reset the Sword back to Kokiri Sword." });
    UIWidgets::CVarCheckbox("Do not reset Rupees", "gEnhancements.Cycle.DoNotResetRupees",
                            { .tooltip = "Playing the Song Of Time will not reset the your rupees." });

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255, 255, 0, 255));
    ImGui::SeparatorText("Unstable");
    ImGui::PopStyleColor();
    UIWidgets::CVarCheckbox(
        "Disable Save Delay", "gEnhancements.Saving.DisableSaveDelay",
        { .tooltip = "Removes the arbitrary 2 second timer for saving from the original game. This is known to "
                     "cause issues when attempting the 0th Day Glitch" });
}

// Graphics
void DrawGraphicsEnhancements() {
    ImGui::SeparatorText("Clock");
    UIWidgets::CVarCombobox("Clock Type", "gEnhancements.Graphics.ClockType", clockTypeOptions);
    UIWidgets::CVarCheckbox("24 Hours Clock", "gEnhancements.Graphics.24HoursClock");
    MotionBlur_RenderMenuOptions();
    ImGui::SeparatorText("Other");
    UIWidgets::CVarCheckbox("Authentic logo", "gEnhancements.Graphics.AuthenticLogo",
                            { .tooltip = "Hide the game version and build details and display the authentic "
                                         "model and texture on the boot logo start screen" });
    UIWidgets::CVarCheckbox("Bow Reticle", "gEnhancements.Graphics.BowReticle",
                            { .tooltip = "Gives the bow a reticle when you draw an arrow" });
    UIWidgets::CVarCheckbox("Disable Black Bar Letterboxes", "gEnhancements.Graphics.DisableBlackBars",
                            { .tooltip =
                                  "Disables Black Bar Letterboxes during cutscenes and Z-targeting\nNote: there may be "
                                  "minor visual glitches that were covered up by the black bars\nPlease disable this "
                                  "setting before reporting a bug" });

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255, 255, 0, 255));
    ImGui::SeparatorText("Unstable");
    ImGui::PopStyleColor();
    UIWidgets::CVarCheckbox(
        "Disable Scene Geometry Distance Check", "gEnhancements.Graphics.DisableSceneGeometryDistanceCheck",
        { .tooltip = "Disables the distance check for scene geometry, allowing it to be drawn no matter how far "
                     "away it is from the player. This may have unintended side effects." });
    // BENTODO: Not implemented yet
    // UIWidgets::CVarCheckbox("Widescreen Actor Culling",
    //                         "gEnhancements.Graphics.ActorCullingAccountsForWidescreen",
    //                         { .tooltip = "Adjusts the culling planes to account for widescreen resolutions. "
    //                                      "This may have unintended side effects." });
    if (UIWidgets::CVarSliderInt(
            "Increase Actor Draw Distance: %dx", "gEnhancements.Graphics.IncreaseActorDrawDistance", 1, 5, 1,
            { .tooltip = "Increase the range in which Actors are drawn. This may have unintended side effects." })) {
        CVarSetInteger("gEnhancements.Graphics.IncreaseActorUpdateDistance",
                       MIN(CVarGetInteger("gEnhancements.Graphics.IncreaseActorDrawDistance", 1),
                           CVarGetInteger("gEnhancements.Graphics.IncreaseActorUpdateDistance", 1)));
    }
    if (UIWidgets::CVarSliderInt(
            "Increase Actor Update Distance: %dx", "gEnhancements.Graphics.IncreaseActorUpdateDistance", 1, 5, 1,
            { .tooltip = "Increase the range in which Actors are updated. This may have unintended side effects." })) {
        CVarSetInteger("gEnhancements.Graphics.IncreaseActorDrawDistance",
                       MAX(CVarGetInteger("gEnhancements.Graphics.IncreaseActorDrawDistance", 1),
                           CVarGetInteger("gEnhancements.Graphics.IncreaseActorUpdateDistance", 1)));
    }
};

// Items/Songs
void DrawItemEnhancements() {
    ImGui::SeparatorText("Masks");
    UIWidgets::CVarCheckbox("Blast Mask has Powder Keg Force", "gEnhancements.Masks.BlastMaskKeg");
    UIWidgets::CVarCheckbox("Fast Transformation", "gEnhancements.Masks.FastTransformation");
    UIWidgets::CVarCheckbox("Fierce Deity's Mask Anywhere", "gEnhancements.Masks.FierceDeitysAnywhere",
                            { .tooltip = "Allow using Fierce Deity's mask outside of boss rooms." });
    UIWidgets::CVarCheckbox("No Blast Mask Cooldown", "gEnhancements.Masks.NoBlastMaskCooldown", {});
}

void DrawSongEnhancements() {
    UIWidgets::CVarCheckbox(
        "Enable Sun's Song", "gEnhancements.Songs.EnableSunsSong",
        { .tooltip = "Enables the partially implemented Sun's Song. RIGHT-DOWN-UP-RIGHT-DOWN-UP to play it. "
                     "This song will make time move very fast until either Link moves to a different scene, "
                     "or when the time switches to a new time period." });
    UIWidgets::CVarCheckbox("Dpad Ocarina", "gEnhancements.Playback.DpadOcarina",
                            { .tooltip = "Enables using the Dpad for Ocarina playback." });
    UIWidgets::CVarCheckbox("Prevent Dropped Ocarina Inputs", "gEnhancements.Playback.NoDropOcarinaInput",
                            { .tooltip = "Prevent dropping inputs when playing the ocarina quickly" });
}

void DrawTimeSaverEnhancements1() {
    ImGui::SeparatorText("Cutscenes");
    UIWidgets::CVarCheckbox("Hide Title Cards", "gEnhancements.Cutscenes.HideTitleCards");
    UIWidgets::CVarCheckbox("Skip Entrance Cutscenes", "gEnhancements.Cutscenes.SkipEntranceCutscenes");
    UIWidgets::CVarCheckbox(
        "Skip to File Select", "gEnhancements.Cutscenes.SkipToFileSelect",
        { .tooltip = "Skip the opening title sequence and go straight to the file select menu after boot" });
    UIWidgets::CVarCheckbox(
        "Skip Intro Sequence", "gEnhancements.Cutscenes.SkipIntroSequence",
        { .tooltip = "When starting a game you will be taken straight to South Clock Town as Deku Link." });
    UIWidgets::CVarCheckbox(
        "Skip Story Cutscenes", "gEnhancements.Cutscenes.SkipStoryCutscenes",
        { .tooltip = "Disclaimer: This doesn't do much yet, we will be progressively adding more skips over time" });
    UIWidgets::CVarCheckbox(
        "Skip Misc Interactions", "gEnhancements.Cutscenes.SkipMiscInteractions",
        { .tooltip = "Disclaimer: This doesn't do much yet, we will be progressively adding more skips over time" });
}

void DrawTimeSaverEnhancements2() {
    UIWidgets::CVarCheckbox(
        "Fast Bank Selection", "gEnhancements.Dialogue.FastBankSelection",
        { .tooltip = "Pressing the Z or R buttons while the Deposit/Withdrawl Rupees dialogue is open will set "
                     "the Rupees to Links current Rupees or 0 respectively." });
    UIWidgets::CVarCheckbox(
        "Fast Text", "gEnhancements.Dialogue.FastText",
        { .tooltip = "Speeds up text rendering, and enables holding of B progress to next message" });
    UIWidgets::CVarCheckbox("Fast Magic Arrow Equip Animation", "gEnhancements.Equipment.MagicArrowEquipSpeed",
                            { .tooltip = "Removes the animation for equipping Magic Arrows." });
}

void DrawFixEnhancements() {
    UIWidgets::CVarCheckbox("Fix Ammo Count Color", "gFixes.FixAmmoCountEnvColor",
                            { .tooltip = "Fixes a missing gDPSetEnvColor, which causes the ammo count to be "
                                         "the wrong color prior to obtaining magic or other conditions." });

    UIWidgets::CVarCheckbox(
        "Fix Hess and Weirdshot Crash", "gEnhancements.Fixes.HessCrash",
        { .tooltip = "Fixes a crash that can occur when performing a HESS or Weirdshot.", .defaultValue = true });

    UIWidgets::CVarCheckbox("Fix Text Control Characters", "gEnhancements.Fixes.ControlCharacters",
                            { .tooltip = "Fixes certain control characters not functioning properly "
                                         "depending on their position within the text." });
}

void DrawRestorationEnhancements() {
    UIWidgets::CVarCheckbox("Constant Distance Backflips and Sidehops", "gEnhancements.Restorations.ConstantFlipsHops",
                            { .tooltip = "Backflips and Sidehops travel a constant distance as they did in OOT." });
    UIWidgets::CVarCheckbox(
        "Power Crouch Stab", "gEnhancements.Restorations.PowerCrouchStab",
        { .tooltip = "Crouch stabs will use the power of Link's previous melee attack, as is in MM JP 1.0 and OOT." });
    UIWidgets::CVarCheckbox("Side Rolls", "gEnhancements.Restorations.SideRoll",
                            { .tooltip = "Restores side rolling from OOT." });
    UIWidgets::CVarCheckbox("Tatl ISG", "gEnhancements.Restorations.TatlISG",
                            { .tooltip = "Restores Navi ISG from OOT, but now with Tatl." });
}

void DrawHudEditorContents() {
    UIWidgets::WindowButton("Popout Hud Editor", "gWindows.HudEditor", mHudEditorWindow,
                            { .tooltip = "Enables the Hud Editor window, allowing you to edit your hud" });
    if (!CVarGetInteger("gWindows.HudEditor", 0)) {
        mHudEditorWindow->DrawElement();
    }
}

void DrawGeneralDevTools() {
    // PortNote: This should be hidden for ports on systems that are single-screen, and/or smaller than 1280x800.
    // Popout will assume size of 1280x800, and will break on those systems.
    UIWidgets::CVarCheckbox("Popout Menu", "gSettings.Menu.Popout",
                            { .tooltip = "Changes the menu display from overlay to windowed." });
    if (UIWidgets::Button("Open App Files Folder",
                          { .tooltip = "Opens the folder that contains the save and mods folders, etc." })) {
        std::string filesPath = Ship::Context::GetInstance()->GetAppDirectoryPath();
        SDL_OpenURL(std::string("file:///" + std::filesystem::absolute(filesPath).string()).c_str());
    }
    if (UIWidgets::CVarCheckbox("Debug Mode", "gDeveloperTools.DebugEnabled",
                                { .tooltip = "Enables Debug Mode, allowing you to select maps with L + R + Z." })) {
        // If disabling debug mode, disable all debug features
        if (!CVarGetInteger("gDeveloperTools.DebugEnabled", 0)) {
            CVarClear("gDeveloperTools.DebugSaveFileMode");
            CVarClear("gDeveloperTools.PreventActorUpdate");
            CVarClear("gDeveloperTools.PreventActorDraw");
            CVarClear("gDeveloperTools.PreventActorInit");
            CVarClear("gDeveloperTools.DisableObjectDependency");
            if (gPlayState != NULL) {
                gPlayState->frameAdvCtx.enabled = false;
            }
            RegisterDebugSaveCreate();
            RegisterPreventActorUpdateHooks();
            RegisterPreventActorDrawHooks();
            RegisterPreventActorInitHooks();
        }
    };

    if (CVarGetInteger("gDeveloperTools.DebugEnabled", 0)) {
        UIWidgets::CVarCheckbox(
            "Better Map Select", "gDeveloperTools.BetterMapSelect.Enabled",
            { .tooltip = "Overrides the original map select with a translated, more user-friendly version." });

        if (UIWidgets::CVarCombobox(
                "Debug Save File Mode", "gDeveloperTools.DebugSaveFileMode", debugSaveOptions,
                { .tooltip =
                      "Change the behavior of creating saves while debug mode is enabled:\n\n"
                      "- Empty Save: The default 3 heart save file in first cycle\n"
                      "- Vanilla Debug Save: Uses the title screen save info (8 hearts, all items and masks)\n"
                      "- 100\% Save: All items, equipment, mask, quast status and bombers notebook complete" })) {
            RegisterDebugSaveCreate();
        }
    }

    if (UIWidgets::CVarCheckbox("Prevent Actor Update", "gDeveloperTools.PreventActorUpdate")) {
        RegisterPreventActorUpdateHooks();
    }
    if (UIWidgets::CVarCheckbox("Prevent Actor Draw", "gDeveloperTools.PreventActorDraw")) {
        RegisterPreventActorDrawHooks();
    }
    if (UIWidgets::CVarCheckbox("Prevent Actor Init", "gDeveloperTools.PreventActorInit")) {
        RegisterPreventActorInitHooks();
    }
    UIWidgets::CVarCheckbox("Disable Object Dependency", "gDeveloperTools.DisableObjectDependency");
    if (UIWidgets::CVarCombobox("Log Level", "gDeveloperTools.LogLevel", logLevels,
                                {
                                    .tooltip = "The log level determines which messages are printed to the "
                                               "console. This does not affect the log file output",
                                    .defaultIndex = 1,
                                })) {
        Ship::Context::GetInstance()->GetLogger()->set_level(
            (spdlog::level::level_enum)CVarGetInteger("gDeveloperTools.LogLevel", 1));
    }

    if (gPlayState != NULL) {
        ImGui::Separator();
        UIWidgets::Checkbox(
            "Frame Advance", (bool*)&gPlayState->frameAdvCtx.enabled,
            { .tooltip = "This allows you to advance through the game one frame at a time on command. "
                         "To advance a frame, hold Z and tap R on the second controller. Holding Z "
                         "and R will advance a frame every half second. You can also use the buttons below." });
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
    RenderWarpPointSection();
}

void DrawCollisionViewerContents() {
    UIWidgets::WindowButton("Popout Collision Viewer", "gWindows.CollisionViewer", mCollisionViewerWindow,
                            { .tooltip = "Draws collision to the screen" });
    if (!CVarGetInteger("gWindows.CollisionViewer", 0)) {
        mCollisionViewerWindow->DrawElement();
    }
}

void DrawStatsContents() {
    UIWidgets::WindowButton(
        "Popout Stats", "gOpenWindows.Stats", mStatsWindow,
        { .tooltip = "Shows the stats window, with your FPS and frametimes, and the OS you're playing on" });
    if (!CVarGetInteger("gOpenWindows.Stats", 0)) {
        mStatsWindow->DrawElement();
    }
}

void DrawConsoleContents() {
    UIWidgets::WindowButton(
        "Popout Console", "gOpenWindows.Console", mConsoleWindow,
        { .tooltip = "Enables the console window, allowing you to input commands, type help for some examples" });
    if (!CVarGetInteger("gOpenWindows.Console", 0)) {
        mConsoleWindow->DrawElement();
    }
}

void DrawGfxDebuggerContents() {
    UIWidgets::WindowButton(
        "Popout Gfx Debugger", "gOpenWindows.GfxDebugger", mGfxDebuggerWindow,
        { .tooltip = "Enables the Gfx Debugger window, allowing you to input commands, type help for some examples" });
    if (!CVarGetInteger("gOpenWindows.GfxDebugger", 0)) {
        mGfxDebuggerWindow->DrawElement();
    }
}

void DrawSaveEditorContents() {
    UIWidgets::WindowButton("Popout Save Editor", "gWindows.SaveEditor", mSaveEditorWindow,
                            { .tooltip = "Enables the Save Editor window, allowing you to edit your save file" });
    if (!CVarGetInteger("gWindows.SaveEditor", 0)) {
        mSaveEditorWindow->DrawElement();
    }
}

void DrawActorViewerContents() {
    UIWidgets::WindowButton(
        "Popout Actor Viewer", "gWindows.ActorViewer", mActorViewerWindow,
        { .tooltip = "Enables the Actor Viewer window, allowing you to view actors in the world." });
    if (!CVarGetInteger("gWindows.ActorViewer", 0)) {
        mActorViewerWindow->DrawElement();
    }
}

void DrawEventLogContents() {
    UIWidgets::WindowButton("Popout Event Log", "gWindows.EventLog", mEventLogWindow);
    if (!CVarGetInteger("gWindows.EventLog", 0)) {
        mActorViewerWindow->DrawElement();
    }
}

std::vector<UIWidgets::MainMenuEntry> menuEntries;

BenMenu::BenMenu(const std::string& consoleVariable, const std::string& name) : GuiWindow(consoleVariable, name) {
}

void BenMenu::InitElement() {
    popped = CVarGetInteger("gSettings.Menu.Popout", 0);
    poppedSize.x = CVarGetInteger("gSettings.Menu.PoppedWidth", 1280);
    poppedSize.y = CVarGetInteger("gSettings.Menu.PoppedHeight", 800);
    poppedPos.x = CVarGetInteger("gSettings.Menu.PoppedPos.x", 0);
    poppedPos.y = CVarGetInteger("gSettings.Menu.PoppedPos.y", 0);
    std::vector<UIWidgets::SidebarEntry> settingsSidebar = { { "General", { DrawGeneralSettings, nullptr, nullptr } },
                                                             { "Audio", { DrawAudioSettings, nullptr, nullptr } },
                                                             { "Graphics", { DrawGraphicsSettings, nullptr, nullptr } },
                                                             { "Controls", { DrawControllerSettings } } };

    std::vector<UIWidgets::SidebarEntry> enhancementsSidebar = {
        { "Camera", { DrawCameraEnhancements1, DrawCameraEnhancements2, DrawCameraEnhancements3 } },
        { "Cheats", { DrawCheatEnhancements, nullptr, nullptr } },
        { "Gameplay", { DrawGameplayEnhancements, DrawGameModesEnhancements, DrawSaveTimeEnhancements } },
        { "Graphics", { DrawGraphicsEnhancements, nullptr, nullptr } },
        { "Items/Songs", { DrawItemEnhancements, DrawSongEnhancements, nullptr } },
        { "Time Savers", { DrawTimeSaverEnhancements1, DrawTimeSaverEnhancements2, nullptr } },
        { "Fixes", { DrawFixEnhancements, nullptr, nullptr } },
        { "Restorations", { DrawRestorationEnhancements, nullptr, nullptr } },
        { "Hud Editor", { DrawHudEditorContents, nullptr } }
    };

    std::vector<UIWidgets::SidebarEntry> devToolsSidebar = { { "General", { DrawGeneralDevTools, nullptr, nullptr } },
                                                             { "Collision Viewer",
                                                               { DrawCollisionViewerContents, nullptr } },
                                                             { "Stats", { DrawStatsContents, nullptr } },
                                                             { "Console", { DrawConsoleContents, nullptr } },
                                                             { "Gfx Debugger", { DrawGfxDebuggerContents, nullptr } },
                                                             { "Save Editor", { DrawSaveEditorContents, nullptr } },
                                                             { "Actor Viewer", { DrawActorViewerContents, nullptr } },
                                                             { "Event Log", { DrawEventLogContents, nullptr } } };

    menuEntries = { { "Settings", settingsSidebar, "gSettings.Menu.SettingsSidebarIndex" },
                    { "Enhancements", enhancementsSidebar, "gSettings.Menu.EnhancementsSidebarIndex" },
                    { "Developer Tools", devToolsSidebar, "gSettings.Menu.DevToolsSidebarIndex" } };

    UpdateWindowBackendObjects();
}

void BenMenu::UpdateElement() {
}

bool ModernMenuSidebarEntry(std::string label) {
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec2 pos = window->DC.CursorPos;
    const ImGuiID sidebarId = window->GetID(std::string(label + "##Sidebar").c_str());
    ImVec2 labelSize = ImGui::CalcTextSize(label.c_str(), ImGui::FindRenderedTextEnd(label.c_str()), true);
    pos.y += style.FramePadding.y;
    pos.x = window->WorkRect.GetCenter().x - labelSize.x / 2;
    ImRect bb = { pos - style.FramePadding, pos + labelSize + style.FramePadding };
    ImGui::ItemSize(bb, style.FramePadding.y);
    ImGui::ItemAdd(bb, sidebarId);
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, sidebarId, &hovered, &held);
    if (pressed) {
        ImGui::MarkItemEdited(sidebarId);
    }
    window->DrawList->AddRectFilled(pos - style.FramePadding, pos + labelSize + style.FramePadding,
                                    ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive
                                                       : hovered         ? ImGuiCol_FrameBgHovered
                                                                         : ImGuiCol_FrameBg),
                                    true, style.FrameRounding);
    UIWidgets::RenderText(pos, label.c_str(), ImGui::FindRenderedTextEnd(label.c_str()), true);
    return pressed;
}

bool ModernMenuHeaderEntry(std::string label) {
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec2 pos = window->DC.CursorPos;
    const ImGuiID headerId = window->GetID(std::string(label + "##Header").c_str());
    ImVec2 labelSize = ImGui::CalcTextSize(label.c_str(), ImGui::FindRenderedTextEnd(label.c_str()), true);
    ImRect bb = { pos, pos + labelSize + style.FramePadding * 2 };
    ImGui::ItemSize(bb, style.FramePadding.y);
    ImGui::ItemAdd(bb, headerId);
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, headerId, &hovered, &held);
    window->DrawList->AddRectFilled(bb.Min, bb.Max,
                                    ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive
                                                       : hovered         ? ImGuiCol_FrameBgHovered
                                                                         : ImGuiCol_FrameBg),
                                    true, style.FrameRounding);
    pos += style.FramePadding;
    UIWidgets::RenderText(pos, label.c_str(), ImGui::FindRenderedTextEnd(label.c_str()), true);
    return pressed;
}

void BenMenu::Draw() {
    if (!IsVisible()) {
        return;
    }
    DrawElement();
    // Sync up the IsVisible flag if it was changed by ImGui
    SyncVisibilityConsoleVariable();
}

void BenMenu::DrawElement() {
    windowHeight = ImGui::GetMainViewport()->WorkSize.y;
    windowWidth = ImGui::GetMainViewport()->WorkSize.x;
    auto windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
    bool popout = CVarGetInteger("gSettings.Menu.Popout", 0) && allowPopout;
    if (popout) {
        windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoDocking;
    }
    if (popout != popped) {
        if (popout) {
            windowHeight = poppedSize.y;
            windowWidth = poppedSize.x;
            ImGui::SetNextWindowSize({ static_cast<float>(windowWidth), static_cast<float>(windowHeight) },
                                     ImGuiCond_Always);
            ImGui::SetNextWindowPos(poppedPos, ImGuiCond_Always);
        } else if (popped) {
            CVarSetFloat("gSettings.Menu.PoppedWidth", poppedSize.x);
            CVarSetFloat("gSettings.Menu.PoppedHeight", poppedSize.y);
            CVarSave();
        }
    }
    popped = popout;
    auto windowCond = ImGuiCond_Always;
    if (!popout) {
        ImGui::SetNextWindowSize({ static_cast<float>(windowWidth), static_cast<float>(windowHeight) }, windowCond);
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), windowCond, { 0.5f, 0.5f });
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    }
    if (!ImGui::Begin("Main Menu", NULL, windowFlags | ImGuiWindowFlags_NoBringToFrontOnFocus)) {
        if (!popout) {
            ImGui::PopStyleVar();
        }
        ImGui::End();
        return;
    }
    if (popped != popout) {
        if (!popout) {
            ImGui::PopStyleVar();
        }
        CVarSetInteger("gSettings.Menu.Popout", popped);
        CVarSetFloat("gSettings.Menu.PoppedWidth", poppedSize.x);
        CVarSetFloat("gSettings.Menu.PoppedHeight", poppedSize.y);
        CVarSetFloat("gSettings.Menu.PoppedPos.x", poppedSize.x);
        CVarSetFloat("gSettings.Menu.PoppedPos.y", poppedSize.y);
        CVarSave();
        ImGui::End();
        return;
    }
    ImGui::PushFont(OTRGlobals::Instance->fontStandardLargest);
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiStyle& style = ImGui::GetStyle();
    windowHeight = window->WorkRect.GetHeight();
    windowWidth = window->WorkRect.GetWidth();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 8.0f));
    auto sectionCount = menuEntries.size();
    const char* headerCvar = "gSettings.Menu.SelectedHeader";
    uint8_t headerIndex = CVarGetInteger(headerCvar, 0);
    ImVec2 pos = window->DC.CursorPos;
    float centerX = pos.x + windowWidth / 2 - (style.ItemSpacing.x * (sectionCount + 1));
    std::vector<ImVec2> headerSizes;
    float headerWidth = 0;
    for (int i = 0; i < sectionCount; i++) {
        ImVec2 size = ImGui::CalcTextSize(menuEntries.at(i).label.c_str());
        headerSizes.push_back(size);
        headerWidth += size.x + style.FramePadding.x * 2;
        if (i + 1 < sectionCount) {
            headerWidth += style.ItemSpacing.x;
        }
    }
    ImVec2 menuSize = { std::fminf(1280, windowWidth), std::fminf(800, windowHeight) };
    pos += window->WorkRect.GetSize() / 2 - menuSize / 2;
    ImGui::SetNextWindowPos(pos);
    ImGui::BeginChild("Menu Block", menuSize,
                      ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize,
                      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);

    std::vector<UIWidgets::SidebarEntry> sidebar;
    float headerHeight = headerSizes.at(0).y + style.FramePadding.y * 2;
    ImVec2 buttonSize = ImGui::CalcTextSize(ICON_FA_TIMES_CIRCLE) + style.FramePadding * 2;
    bool scrollbar = false;
    if (headerWidth > menuSize.x - buttonSize.x * 3 - style.ItemSpacing.x * 3) {
        headerHeight += style.ScrollbarSize;
        scrollbar = true;
    }
    if (UIWidgets::Button(ICON_FA_TIMES_CIRCLE, { .size = UIWidgets::Sizes::Inline, .tooltip = "Close Menu (Esc)" })) {
        ToggleVisibility();
    }
    ImGui::SameLine();
    ImGui::SetNextWindowSizeConstraints({ 0, headerHeight }, { headerWidth, headerHeight });
    ImVec2 headerSelSize = { menuSize.x - buttonSize.x * 3 - style.ItemSpacing.x * 3, headerHeight };
    if (scrollbar) {
        headerSelSize.y += style.ScrollbarSize;
    }
    ImGui::BeginChild("Header Selection", headerSelSize,
                      ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize,
                      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_HorizontalScrollbar);
    for (int i = 0; i < sectionCount; i++) {
        auto entry = menuEntries.at(i);
        uint8_t nextIndex = i;
        if (headerIndex != i) {
            ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0, 0, 0, 0 });
        }
        if (ModernMenuHeaderEntry(entry.label)) {
            CVarSetInteger(headerCvar, i);
            CVarSave();
            nextIndex = i;
        }
        if (headerIndex != i) {
            ImGui::PopStyleColor();
        }
        if (headerIndex == i) {
            sidebar = entry.sidebarEntries;
        }
        if (i + 1 < sectionCount) {
            ImGui::SameLine();
        }
        if (nextIndex != i) {
            headerIndex = nextIndex;
        }
    }
    ImGui::EndChild();
    ImGui::SameLine(menuSize.x - (buttonSize.x * 2) - style.ItemSpacing.x);
    if (UIWidgets::Button(ICON_FA_UNDO, { .color = UIWidgets::Colors::Red,
                                          .size = UIWidgets::Sizes::Inline,
                                          .tooltip = "Reset"
#ifdef __APPLE__
                                                     " (Command-R)"
#elif !defined(__SWITCH__) && !defined(__WIIU__)
                                                     " (Ctrl+R)"
#else
                                                     ""
#endif
                                        })) {
        std::reinterpret_pointer_cast<Ship::ConsoleWindow>(
            Ship::Context::GetInstance()->GetWindow()->GetGui()->GetGuiWindow("Console"))
            ->Dispatch("reset");
    }
    ImGui::SameLine();
    if (UIWidgets::Button(
            ICON_FA_POWER_OFF,
            { .color = UIWidgets::Colors::Red, .size = UIWidgets::Sizes::Inline, .tooltip = "Quit 2S2H" })) {
        Ship::Context::GetInstance()->GetWindow()->Close();
    }
    ImGui::PopStyleVar();

    pos.y += headerHeight + style.ItemSpacing.y;
    pos.x = centerX - menuSize.x / 2 + (style.ItemSpacing.x * (sectionCount + 1));
    window->DrawList->AddRectFilled(pos, pos + ImVec2{ menuSize.x, 4 }, ImGui::GetColorU32({ 255, 255, 255, 255 }),
                                    true, style.WindowRounding);
    pos.y += style.ItemSpacing.y;
    float sectionHeight = menuSize.y - headerHeight - 4 - style.ItemSpacing.y * 2;
    float columnHeight = sectionHeight - style.ItemSpacing.y * 4;
    ImGui::SetNextWindowPos(pos + style.ItemSpacing * 2);
    float sidebarWidth = 200 - style.ItemSpacing.x;

    const char* sidebarCvar = menuEntries.at(headerIndex).sidebarCvar;

    uint8_t sectionIndex = CVarGetInteger(sidebarCvar, 0);
    if (sectionIndex > sidebar.size())
        sectionIndex = sidebar.size();
    if (sectionIndex < 0)
        sectionIndex = 0;
    float sectionCenterX = pos.x + (sidebarWidth / 2);
    float topY = pos.y;
    ImGui::SetNextWindowSizeConstraints({ sidebarWidth, 0 }, { sidebarWidth, columnHeight });
    ImGui::BeginChild((menuEntries.at(headerIndex).label + " Section").c_str(), { sidebarWidth, columnHeight * 3 },
                      ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize, ImGuiWindowFlags_NoTitleBar);
    for (int i = 0; i < sidebar.size(); i++) {
        auto sidebarEntry = sidebar.at(i);
        uint8_t nextIndex = i;
        if (sectionIndex != i) {
            ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0, 0, 0, 0 });
        }
        if (ModernMenuSidebarEntry(sidebarEntry.label)) {
            CVarSetInteger(sidebarCvar, i);
            CVarSave();
            nextIndex = i;
        }
        if (sectionIndex != i) {
            ImGui::PopStyleColor();
        }
        if (nextIndex != i) {
            sectionIndex = i;
        }
    }
    ImGui::EndChild();

    ImGui::PushFont(OTRGlobals::Instance->fontMonoLarger);
    pos = ImVec2{ sectionCenterX + (sidebarWidth / 2), topY } + style.ItemSpacing * 2;
    window->DrawList->AddRectFilled(pos, pos + ImVec2{ 4, sectionHeight - style.FramePadding.y * 2 },
                                    ImGui::GetColorU32({ 255, 255, 255, 255 }), true, style.WindowRounding);
    pos.x += 4 + style.ItemSpacing.x;
    ImGui::SetNextWindowPos(pos + style.ItemSpacing);
    float sectionWidth = menuSize.x - sidebarWidth - 4 - style.ItemSpacing.x * 4;
    std::string sectionMenuId = sidebar.at(sectionIndex).label + " Settings";
    int columns = sidebar.at(sectionIndex).columnFuncs.size();
    int columnFuncs = 0;
    for (auto func : sidebar.at(sectionIndex).columnFuncs) {
        if (func != nullptr) {
            columnFuncs++;
        }
    }
    if (windowWidth < 800) {
        columns = 1;
    }
    float columnWidth = (sectionWidth - style.ItemSpacing.x * columns) / columns;
    bool useColumns = columns > 1;
    if (!useColumns) {
        ImGui::SameLine();
        ImGui::SetNextWindowSizeConstraints({ sectionWidth, 0 }, { sectionWidth, columnHeight });
        ImGui::BeginChild(sectionMenuId.c_str(), { sectionWidth, windowHeight * 4 }, ImGuiChildFlags_AutoResizeY,
                          ImGuiWindowFlags_NoTitleBar);
    }
    for (int i = 0; i < columnFuncs; i++) {
        std::string sectionId = fmt::format("{} Column {}", sectionMenuId, i);
        if (useColumns) {
            ImGui::SetNextWindowSizeConstraints({ columnWidth, 0 }, { columnWidth, columnHeight });
            ImGui::BeginChild(sectionId.c_str(), { columnWidth, windowHeight * 4 }, ImGuiChildFlags_AutoResizeY,
                              ImGuiWindowFlags_NoTitleBar);
        }
        if (sidebar.at(sectionIndex).columnFuncs.at(i) != nullptr) {
            sidebar.at(sectionIndex).columnFuncs.at(i)();
        }
        if (useColumns) {
            ImGui::EndChild();
        }
        if (i < columns - 1) {
            ImGui::SameLine();
        }
    }
    if (!useColumns) {
        ImGui::EndChild();
    }
    ImGui::PopFont();
    ImGui::PopFont();

    if (!popout) {
        ImGui::PopStyleVar();
    }
    ImGui::EndChild();
    if (popout) {
        poppedSize = ImGui::GetWindowSize();
        poppedPos = ImGui::GetWindowPos();
    }
    ImGui::End();
}
} // namespace BenGui
