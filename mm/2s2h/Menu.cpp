#include "Menu.h"
#include "BenPort.h"
#include "2s2h/BenGui/BenGui.hpp"
#include "2s2h/BenGui/UIWidgets.hpp"
#include "graphic/Fast3D/gfx_rendering_api.h"
#include "2s2h/Enhancements/Enhancements.h"
#include "2s2h/Enhancements/Graphics/MotionBlur.h"
#include "2s2h/Enhancements/Graphics/PlayAsKafei.h"
#include "2s2h/DeveloperTools/DeveloperTools.h"
#include "2s2h/DeveloperTools/WarpPoint.h"
#include "window/gui/GuiMenuBar.h"
#include "window/gui/GuiElement.h"
#include "DeveloperTools/SaveEditor.h"
#include "DeveloperTools/ActorViewer.h"
#include "DeveloperTools/CollisionViewer.h"
#include "DeveloperTools/EventLog.h"
#include "2s2h/BenGui/HudEditor.h"
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

namespace BenGui {

extern std::shared_ptr<HudEditorWindow> mHudEditorWindow;
extern std::shared_ptr<Ship::GuiWindow> mStatsWindow;
extern std::shared_ptr<Ship::GuiWindow> mConsoleWindow;
extern std::shared_ptr<Ship::GuiWindow> mGfxDebuggerWindow;
extern std::shared_ptr<SaveEditorWindow> mSaveEditorWindow;
extern std::shared_ptr<ActorViewerWindow> mActorViewerWindow;
extern std::shared_ptr<CollisionViewerWindow> mCollisionViewerWindow;
extern std::shared_ptr<EventLogWindow> mEventLogWindow;

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

// Camera
void DrawCameraFixes() {
    ImGui::SeparatorText("Fixes");
    UIWidgets::CVarCheckbox(
        "Fix Targetting Camera Snap", "gEnhancements.Camera.FixTargettingCameraSnap",
        { .tooltip =
                "Fixes the camera snap that occurs when you are moving and press the targetting button." });
}

void DrawFreeLook() {
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

        UIWidgets::CVarSliderInt("Camera Distance: %d", "gEnhancements.Camera.FreeLook.MaxCameraDistance", 100,
            900, 185);
        UIWidgets::CVarSliderInt("Camera Transition Speed: %d", "gEnhancements.Camera.FreeLook.TransitionSpeed",
            1, 900, 25);
        UIWidgets::CVarSliderFloat("Max Camera Height Angle: %.0f°", "gEnhancements.Camera.FreeLook.MaxPitch",
            -89.0f, 89.0f, 72.0f);
        UIWidgets::CVarSliderFloat("Min Camera Height Angle: %.0f°", "gEnhancements.Camera.FreeLook.MinPitch",
            -89.0f, 89.0f, -49.0f);
        f32 maxY = CVarGetFloat("gEnhancements.Camera.FreeLook.MaxPitch", 72.0f);
        f32 minY = CVarGetFloat("gEnhancements.Camera.FreeLook.MinPitch", -49.0f);
        CVarSetFloat("gEnhancements.Camera.FreeLook.MaxPitch", std::max(maxY, minY));
        CVarSetFloat("gEnhancements.Camera.FreeLook.MinPitch", std::min(maxY, minY));
    }
}

void DrawCameraDebug() {
    ImGui::SeparatorText("'Debug' Camera");
    if (UIWidgets::CVarCheckbox(
        "Debug Camera", "gEnhancements.Camera.DebugCam.Enable",
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
        UIWidgets::CVarSliderFloat("Camera Speed: %.0f", "gEnhancements.Camera.DebugCam.CameraSpeed", 0.1f,
            3.0f, 0.5f);
    }
}

// Cheats
void DrawCheatEnhancements() {
    UIWidgets::CVarCheckbox("Infinite Health", "gCheats.InfiniteHealth");
    UIWidgets::CVarCheckbox("Infinite Magic", "gCheats.InfiniteMagic");
    UIWidgets::CVarCheckbox("Infinite Rupees", "gCheats.InfiniteRupees");
    UIWidgets::CVarCheckbox("Infinite Consumables", "gCheats.InfiniteConsumables");
    UIWidgets::CVarCheckbox("Unbreakable Razor Sword", "gCheats.UnbreakableRazorSword");
    UIWidgets::CVarCheckbox("Unrestricted Items", "gCheats.UnrestrictedItems");
    if (UIWidgets::CVarCheckbox("Moon Jump on L", "gCheats.MoonJumpOnL",
                                { .tooltip = "Holding L makes you float into the air" })) {
        RegisterMoonJumpOnL();
    }
    UIWidgets::CVarCheckbox("No Clip", "gCheats.NoClip");
}

// Gameplay
void DrawGameplayEnhancements() {
    ImGui::SeparatorText("Ocarina");
    UIWidgets::CVarCheckbox("Dpad Ocarina", "gEnhancements.Playback.DpadOcarina",
                            { .tooltip = "Enables using the Dpad for Ocarina playback." });
    UIWidgets::CVarCheckbox("Prevent Dropped Ocarina Inputs", "gEnhancements.Playback.NoDropOcarinaInput",
                            { .tooltip = "Prevent dropping inputs when playing the ocarina quickly" });
    UIWidgets::CVarCheckbox("Dpad Equips", "gEnhancements.Dpad.DpadEquips",
                            { .tooltip = "Allows you to equip items to your d-pad" });
    UIWidgets::CVarCombobox("Always Win Doggy Race", "gEnhancements.Minigames.AlwaysWinDoggyRace",
                            alwaysWinDoggyraceOptions);
    UIWidgets::CVarSliderInt("Climb speed", "gEnhancements.PlayerMovement.ClimbSpeed", 1, 5, 1,
                            { .tooltip = "Increases the speed at which Link climbs vines and ladders." });
}

void DrawGameModesEnhancements() {
    UIWidgets::CVarCheckbox("Play As Kafei", "gModes.PlayAsKafei",
        { .tooltip = "Requires scene reload to take effect." });
}

void DrawSaveTimeEnhancements() {
    ImGui::SeparatorText("Saving");
    UIWidgets::CVarCheckbox("Persistent Owl Saves", "gEnhancements.Saving.PersistentOwlSaves",
                            { .tooltip = "Continuing a save will not remove the owl save. Playing Song of "
                                            "Time, allowing the moon to crash or finishing the "
                                            "game will remove the owl save and become the new last save." });
    UIWidgets::CVarCheckbox("Pause Menu Save", "gEnhancements.Saving.PauseSave",
        { .tooltip = "Re-introduce the pause menu save system. Pressing B in the pause menu will give you the "
                        "option to create a persistent Owl Save from your current location.\n\nWhen loading back "
                        "into the game, you will be placed either at the entrance of the dungeon you saved in, or "
                        "in South Clock Town." });
    if (UIWidgets::CVarCheckbox("Autosave", "gEnhancements.Saving.Autosave",
            { .tooltip = "Automatically create a persistent Owl Save on the chosen interval.\n\nWhen loading "
                            "back into the game, you will be placed either at the entrance of the dungeon you "
                            "saved in, or in South Clock Town." })) {
        RegisterAutosave();
    }
    UIWidgets::CVarSliderInt("Autosave Interval (minutes): %d", "gEnhancements.Saving.AutosaveInterval", 1, 60,
                                5, { .disabled = !CVarGetInteger("gEnhancements.Saving.Autosave", 0) });

    ImGui::SeparatorText("Time Cycle");
    UIWidgets::CVarCheckbox("Do not reset Bottle content", "gEnhancements.Cycle.DoNotResetBottleContent",
                            { .tooltip = "Playing the Song Of Time will not reset the bottles' content." });
    UIWidgets::CVarCheckbox("Do not reset Consumables", "gEnhancements.Cycle.DoNotResetConsumables",
                            { .tooltip = "Playing the Song Of Time will not reset the consumables." });
    UIWidgets::CVarCheckbox(
        "Do not reset Razor Sword", "gEnhancements.Cycle.DoNotResetRazorSword",
        { .tooltip = "Playing the Song Of Time will not reset the Sword back to Kokiri Sword." });
    UIWidgets::CVarCheckbox("Do not reset Rupees", "gEnhancements.Cycle.DoNotResetRupees",
                            { .tooltip = "Playing the Song Of Time will not reset the your rupees." });

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255, 255, 0, 255));
    ImGui::SeparatorText("Unstable");
    ImGui::PopStyleColor();
    UIWidgets::CVarCheckbox("Disable Save Delay", "gEnhancements.Saving.DisableSaveDelay",
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
};

// Items/Songs
void DrawItemEnhancements() {
    ImGui::SeparatorText("Masks");
    UIWidgets::CVarCheckbox("Fast Transformation", "gEnhancements.Masks.FastTransformation");
    UIWidgets::CVarCheckbox("Fierce Deity's Mask Anywhere", "gEnhancements.Masks.FierceDeitysAnywhere",
                            { .tooltip = "Allow using Fierce Deity's mask outside of boss rooms." });
    UIWidgets::CVarCheckbox("No Blast Mask Cooldown", "gEnhancements.Masks.NoBlastMaskCooldown", {});
}

void DrawSongEnhancements() {
    UIWidgets::CVarCheckbox("Enable Sun's Song", "gEnhancements.Songs.EnableSunsSong",
        { .tooltip = "Enables the partially implemented Sun's Song. RIGHT-DOWN-UP-RIGHT-DOWN-UP to play it. "
                        "This song will make time move very fast until either Link moves to a different scene, "
                        "or when the time switches to a new time period." });
}

void DrawTimeSaverEnhancements() {
    ImGui::SeparatorText("Cutscenes");
    UIWidgets::CVarCheckbox("Hide Title Cards", "gEnhancements.Cutscenes.HideTitleCards");
    UIWidgets::CVarCheckbox("Skip Entrance Cutscenes", "gEnhancements.Cutscenes.SkipEntranceCutscenes");
    UIWidgets::CVarCheckbox("Skip to File Select", "gEnhancements.Cutscenes.SkipToFileSelect",
        { .tooltip = "Skip the opening title sequence and go straight to the file select menu after boot" });
    UIWidgets::CVarCheckbox("Skip Intro Sequence", "gEnhancements.Cutscenes.SkipIntroSequence",
        { .tooltip = "When starting a game you will be taken straight to South Clock Town as Deku Link." });
    UIWidgets::CVarCheckbox("Skip Story Cutscenes", "gEnhancements.Cutscenes.SkipStoryCutscenes",
        { .tooltip =
                "Disclaimer: This doesn't do much yet, we will be progressively adding more skips over time" });
    UIWidgets::CVarCheckbox("Skip Misc Interactions", "gEnhancements.Cutscenes.SkipMiscInteractions",
        { .tooltip =
                "Disclaimer: This doesn't do much yet, we will be progressively adding more skips over time" });
    UIWidgets::CVarCheckbox("Fast Text", "gEnhancements.Dialogue.FastText",
        { .tooltip = "Speeds up text rendering, and enables holding of B progress to next message" });
}

void DrawFixEnhancements() {
    UIWidgets::CVarCheckbox("Fix Ammo Count Color", "gFixes.FixAmmoCountEnvColor",
        { .tooltip = "Fixes a missing gDPSetEnvColor, which causes the ammo count to be "
                     "the wrong color prior to obtaining magic or other conditions." });

    UIWidgets::CVarCheckbox("Fix Hess and Weirdshot Crash", "gEnhancements.Fixes.HessCrash",
        { .tooltip = "Fixes a crash that can occur when performing a HESS or Weirdshot.",
            .defaultValue = true });

    UIWidgets::CVarCheckbox("Fix Text Control Characters", "gEnhancements.Fixes.ControlCharacters",
        { .tooltip = "Fixes certain control characters not functioning properly "
                        "depending on their position within the text." });
}

void DrawRestorationEnhancements() {
    UIWidgets::CVarCheckbox("Constant Distance Backflips and Sidehops", "gEnhancements.Restorations.ConstantFlipsHops",
        { .tooltip = "Backflips and Sidehops travel a constant distance as they did in OOT." });
    UIWidgets::CVarCheckbox("Power Crouch Stab", "gEnhancements.Restorations.PowerCrouchStab",
        { .tooltip =
                "Crouch stabs will use the power of Link's previous melee attack, as is in MM JP 1.0 and OOT." });
    UIWidgets::CVarCheckbox("Side Rolls", "gEnhancements.Restorations.SideRoll",
                            { .tooltip = "Restores side rolling from OOT." });
    UIWidgets::CVarCheckbox("Tatl ISG", "gEnhancements.Restorations.TatlISG",
                            { .tooltip = "Restores Navi ISG from OOT, but now with Tatl." });
}

void DrawHudEditorContents() {
    UIWidgets::WindowButton("Popout Hud Editor", "gWindows.HudEditor", mHudEditorWindow,
        { .tooltip = "Enables the Hud Editor window, allowing you to edit your hud" });
    if (!CVarGetInteger("gWindows.HudEditor", 0)) {
        mHudEditorWindow->DrawContents();
    }
}

void DrawGeneralDevTools() {
    // PortNote: This should be hidden for ports on systems that are single-screen, and/or smaller than 1280x800.
    // Popout will assume size of 1280x800, and will break on those systems.
    UIWidgets::CVarCheckbox("Popout Menu", "gSettings.Menu.Popout",
        { .tooltip = "Changes the menu display from overlay to windowed." });
}

std::vector<UIWidgets::MainMenuEntry> menuEntries;


void BenMenu::InitElement() {
    popped = CVarGetInteger("gSettings.Menu.Popout", 0);
    poppedSize.x = CVarGetInteger("gSettings.Menu.PoppedWidth", 1280);
    poppedSize.y = CVarGetInteger("gSettings.Menu.PoppedHeight", 800);
    poppedPos.x = CVarGetInteger("gSettings.Menu.PoppedPos.x", 0);
    poppedPos.y = CVarGetInteger("gSettings.Menu.PoppedPos.y", 0);
    std::vector<UIWidgets::SidebarEntry> settingsSidebar = {
        { "Audio", { DrawAudioSettings, nullptr, nullptr } },
        { "Graphics", { DrawGraphicsSettings } },
        { "Controls", { DrawControllerSettings } }
    };

    std::vector<UIWidgets::SidebarEntry> enhancementsSidebar = {
        { "Camera",       { DrawCameraFixes, DrawFreeLook, DrawCameraDebug } },
        { "Cheats",       { DrawCheatEnhancements } },
        { "Gameplay",     { DrawGameplayEnhancements, DrawGameModesEnhancements, DrawSaveTimeEnhancements } },
        { "Graphics",     { DrawGraphicsEnhancements } },
        { "Items/Songs",  { DrawItemEnhancements, DrawSongEnhancements } },
        { "Time Savers",  { DrawTimeSaverEnhancements } },
        { "Fixes",        { DrawFixEnhancements } },
        { "Restorations", { DrawRestorationEnhancements } },
        { "Hud Editor",   { DrawHudEditorContents, nullptr } }
    };

    std::vector<UIWidgets::SidebarEntry> devToolsSidebar = {
        { "General", { DrawGeneralDevTools } }
    };

    menuEntries = {
        { "Settings", settingsSidebar, "gSettings.Menu.SettingsSidebarIndex" },
        { "Enhancements", enhancementsSidebar, "gSettings.Menu.EnhancementsSidebarIndex"},
        { "Developer Tools", devToolsSidebar, "gSettings.Menu.DevToolsSidebarIndex"}
    };
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
            : hovered ? ImGuiCol_FrameBgHovered
            : ImGuiCol_FrameBg),
        true, style.FrameRounding);
    UIWidgets::RenderText(pos, label.c_str(), ImGui::FindRenderedTextEnd(label.c_str()), true);
    return pressed;
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
            ImGui::SetNextWindowSize({ static_cast<float>(windowWidth), static_cast<float>(windowHeight) }, ImGuiCond_Always);
            ImGui::SetNextWindowPos(poppedPos, ImGuiCond_Always);
        }
        else if (popped) {
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
    if (!ImGui::Begin("Main Menu", &popped, windowFlags)) {
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

    auto sectionCount = menuEntries.size();
    const char* headerCvar = "gSettings.Menu.SelectedHeader";
    uint8_t selectedHeader = CVarGetInteger(headerCvar, 0);
    ImVec2 pos = window->DC.CursorPos;
    float centerX = pos.x + windowWidth / 2;
    std::vector<ImVec2> headerSizes;
    for (int i = 0; i < sectionCount; i++) {
        headerSizes.push_back(ImGui::CalcTextSize(menuEntries.at(i).label.c_str()));
    }
    float menuHeight = std::fminf(800, windowHeight);

    pos.x = centerX - (headerSizes.at(0).x + headerSizes.at(1).x / 2 + (style.ItemSpacing.x * 2));
    pos.y += std::fminf(windowHeight - menuHeight, std::fminf(100, (windowHeight - menuHeight)/2));
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
            CVarSave();
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
    float menuWidth = std::fminf(1280, windowWidth);
    pos.y += headerHeight + style.ItemSpacing.y;
    pos.x = centerX - menuWidth / 2;
    window->DrawList->AddRectFilled(pos, pos + ImVec2{menuWidth, 4}, ImGui::GetColorU32({255, 255, 255, 255}), true, style.WindowRounding);
    pos.y += style.ItemSpacing.y;
    float sectionHeight = menuHeight - headerHeight - 4 - style.ItemSpacing.y * 2;
    float columnHeight = sectionHeight - style.ItemSpacing.y * 4;
    ImGui::SetNextWindowPos(pos);

    ImGui::BeginChild((menuEntries.at(selectedHeader).label + " Menu").c_str(), { menuWidth, sectionHeight }, ImGuiChildFlags_None, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
    //ImGui::BeginTable((menuEntries.at(selectedHeader).label + " Menu").c_str(), 3, ImGuiTableFlags_ScrollY  | ImGuiTableFlags_NoBordersInBody, { menuWidth, sectionHeight });
    //auto sidebarFlags = ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_NoReorder | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHeaderLabel | ImGuiTableColumnFlags_NoHide;
    /*ImGui::TableSetupColumn("Sidebar Column", sidebarFlags, 200);
    ImGui::TableSetupColumn("Separator Column", sidebarFlags);
    ImGui::TableSetupColumn("Section Column", sidebarFlags & ~ImGuiTableColumnFlags_WidthFixed);*/
    ImGui::SetNextWindowPos(pos + style.ItemSpacing * 2);
    float sidebarWidth = 200 - style.ItemSpacing.x;

    const char* sidebarCvar = menuEntries.at(selectedHeader).sidebarCvar;

    uint8_t sectionIndex = CVarGetInteger(sidebarCvar, 0);
    if (sectionIndex > sidebar.size()) sectionIndex = sidebar.size();
    if (sectionIndex < 0) sectionIndex = 0;
    float sectionCenterX = pos.x + (sidebarWidth / 2);
    float topY = pos.y;
    ImGui::SetNextWindowSizeConstraints({ sidebarWidth, 0 }, { sidebarWidth, columnHeight });
    ImGui::BeginChild((menuEntries.at(selectedHeader).label + " Section").c_str(), { sidebarWidth, columnHeight * 3 }, ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize,
        ImGuiWindowFlags_NoTitleBar);
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
    pos = ImVec2{ sectionCenterX + (sidebarWidth / 2), topY} + style.ItemSpacing * 2;
    window->DrawList->AddRectFilled(pos, pos + ImVec2{ 4, sectionHeight - style.FramePadding.y * 2 }, ImGui::GetColorU32({ 255, 255, 255, 255 }), true, style.WindowRounding);
    pos.x += 4 + style.ItemSpacing.x;
    ImGui::SetNextWindowPos(pos + style.ItemSpacing);
    float sectionWidth = menuWidth - sidebarWidth - 4 - style.ItemSpacing.x * 4;
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
        ImGui::SetNextWindowSizeConstraints({ sectionWidth, 0 }, { sectionWidth, columnHeight });
    }
    ImGui::BeginChild(sectionMenuId.c_str(), { sectionWidth, windowHeight * 4 }, ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_NoTitleBar);
    for (int i = 0; i < columnFuncs; i++) {
        std::string sectionId = std::format("{} Column {}", sectionMenuId, i);
        if (useColumns) {
            ImGui::SetNextWindowSizeConstraints({ columnWidth, 0 }, { columnWidth, columnHeight });
            ImGui::BeginChild(sectionId.c_str(), { columnWidth, windowHeight * 4 }, ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_NoTitleBar);
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
    ImGui::EndChild();
    ImGui::PopFont();
    ImGui::EndChild();
    ImGui::PopFont();

    // style.Colors[ImGuiCol_Button] = prevButtonCol;
    if (!popout) {
        ImGui::PopStyleVar();
    }
    if (popout) {
        poppedSize = ImGui::GetWindowSize();
        poppedPos = ImGui::GetWindowPos();
    }
    ImGui::End();
}
}
