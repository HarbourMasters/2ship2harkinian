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

#include "2s2h/search/SearchMenu.h"

extern "C" {
#include "z64.h"
#include "functions.h"
extern PlayState* gPlayState;
}
std::vector<ImVec2> windowTypeSizes = { {} };

static const std::unordered_map<Ship::AudioBackend, const char*> audioBackendsMap = {
    { Ship::AudioBackend::WASAPI, "Windows Audio Session API" },
    { Ship::AudioBackend::SDL, "SDL" },
};

static std::unordered_map<Ship::WindowBackend, const char*> windowBackendsMap = {
    { Ship::WindowBackend::FAST3D_DXGI_DX11, "DirectX" },
    { Ship::WindowBackend::FAST3D_SDL_OPENGL, "OpenGL" },
    { Ship::WindowBackend::FAST3D_SDL_METAL, "Metal" },
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

char searchText[30] = "";
uint32_t enhSize = sizeof(enhancementList) / sizeof(enhancementList[0]);

void DrawSearchSettings() {
    ImGui::Text("Search: ");
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_FrameBg, menuTheme[menuThemeIndex]);
    ImGui::InputText("##search", searchText, sizeof(searchText));
    ImGui::PopStyleColor(1);
    std::string str(searchText);

    if (str == "") {
        ImGui::Text("Start typing to see results.");
        return;
    }
    ImGui::BeginChild("Search Results");
    for (int i = 0; i < enhSize; i++) {
        std::string ctr(enhancementList[i].widgetName);
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
        std::transform(ctr.begin(), ctr.end(), ctr.begin(), ::tolower);
        ctr.erase(std::remove(ctr.begin(), ctr.end(), ' '), ctr.end());
        if (ctr.find(str) != std::string::npos) {
            SearchMenuGetItem(i);
        }
    }
    ImGui::EndChild();
}

void DrawGeneralSettings() {
    SearchMenuGetItem(MENU_ITEM_MENU_THEME);

#if not defined(__SWITCH__) and not defined(__WIIU__)
    SearchMenuGetItem(MENU_ITEM_MENUBAR_CONTROLLER_NAV);
    SearchMenuGetItem(MENU_ITEM_CURSOR_VISIBILITY);
    // bool cursor = Ship::Context::GetInstance()->GetWindow()->ShouldForceCursorVisibility();
    // if (UIWidgets::Checkbox("Cursor Always Visible", &cursor,
    //                         { .tooltip = "Makes the cursor always visible, even in full screen." })) {
    //     Ship::Context::GetInstance()->GetWindow()->SetForceCursorVisibility(cursor);
    // }
#endif
    SearchMenuGetItem(MENU_ITEM_HOTKEY_TEXT);
}

void DrawAudioSettings() {
    SearchMenuGetItem(MENU_ITEM_MASTER_VOLUME);
    SearchMenuGetItem(MENU_ITEM_MAIN_MUSIC_VOLUME);
    SearchMenuGetItem(MENU_ITEM_SUB_MUSIC_VOLUME);
    SearchMenuGetItem(MENU_ITEM_SOUND_EFFECT_VOLUME);
    SearchMenuGetItem(MENU_ITEM_FANFARE_VOLUME);
    SearchMenuGetItem(MENU_ITEM_AMBIENT_VOLUME);

    auto currentAudioBackend = Ship::Context::GetInstance()->GetAudio()->GetAudioBackend();
    if (UIWidgets::Combobox(
            "Audio API", &currentAudioBackend, audioBackendsMap,
            { .color = menuTheme[menuThemeIndex],
              .tooltip = "Sets the audio API used by the game. Requires a relaunch to take effect.",
              .disabled = Ship::Context::GetInstance()->GetAudio()->GetAvailableAudioBackends()->size() <= 1,
              .disabledTooltip = "Only one audio API is available on this platform." })) {
        Ship::Context::GetInstance()->GetAudio()->SetAudioBackend(currentAudioBackend);
    }
}

void DrawGraphicsSettings() {
    SearchMenuGetItem(MENU_ITEM_TOGGLE_FULLSCREEN);
#ifndef __APPLE__
    SearchMenuGetItem(MENU_ITEM_INTERNAL_RESOLUTION);
#endif
#ifndef __WIIU__
    SearchMenuGetItem(MENU_ITEM_MSAA);
#endif
    SearchMenuGetItem(MENU_ITEM_FRAME_RATE);
    SearchMenuGetItem(MENU_ITEM_MATCH_REFRESH_RATE);
    if (Ship::Context::GetInstance()->GetWindow()->GetWindowBackend() == Ship::WindowBackend::FAST3D_DXGI_DX11) {
        SearchMenuGetItem(MENU_ITEM_JITTER_FIX);
    }
    //  #endregion */
    if (UIWidgets::Combobox("Renderer API (Needs reload)", &configWindowBackend, availableWindowBackendsMap,
                            { .color = menuTheme[menuThemeIndex],
                              .tooltip = "Sets the renderer API used by the game. Requires a relaunch to take effect.",
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
        SearchMenuGetItem(MENU_ITEM_ENABLE_VSYNC);
    }

    if (Ship::Context::GetInstance()->GetWindow()->SupportsWindowedFullscreen()) {
        SearchMenuGetItem(MENU_ITEM_ENABLE_WINDOWED_FULLSCREEN);
    }

    if (Ship::Context::GetInstance()->GetWindow()->GetGui()->SupportsViewports()) {
        SearchMenuGetItem(MENU_ITEM_ENABLE_MULTI_VIEWPORT);
    }

    SearchMenuGetItem(MENU_ITEM_TEXTURE_FILTER);
}

void DrawControllerSettings() {
    // SearchMenuGetItem(MENU_ITEM_INPUT_EDITOR);
    UIWidgets::WindowButton("Popout Input Editor", "gWindows.BenInputEditor", mBenInputEditorWindow,
                            { .tooltip = "Enables the separate Input Editor window." });
    if (!CVarGetInteger("gWindows.BenInputEditor", 0)) {
        mBenInputEditorWindow->DrawPortTabContents(0);
    }
};

// Camera
void DrawCameraEnhancements1() {
    ImGui::SeparatorText("Fixes");
    SearchMenuGetItem(MENU_ITEM_FIX_TARGET_CAMERA_SNAP);
}

void DrawCameraEnhancements2() {
    ImGui::SeparatorText("Free Look");
    SearchMenuGetItem(MENU_ITEM_ENABLE_FREE_LOOK);
    if (CVarGetInteger("gEnhancements.Camera.FreeLook.Enable", 0)) {
        SearchMenuGetItem(MENU_ITEM_INVERT_CAMERA_X_AXIS);
        SearchMenuGetItem(MENU_ITEM_INVERT_CAMERA_Y_AXIS);
        SearchMenuGetItem(MENU_ITEM_THIRD_PERSON_CAMERA_X_SENSITIVITY);
        SearchMenuGetItem(MENU_ITEM_THIRD_PERSON_CAMERA_Y_SENSITIVITY);
        SearchMenuGetItem(MENU_ITEM_FREE_LOOK_CAMERA_DISTANCE);
        SearchMenuGetItem(MENU_ITEM_FREE_LOOK_TRANSITION_SPEED);
        SearchMenuGetItem(MENU_ITEM_FREE_LOOK_MAX_PITCH);
        SearchMenuGetItem(MENU_ITEM_FREE_LOOK_MIN_PITCH);
        f32 maxY = CVarGetFloat("gEnhancements.Camera.FreeLook.MaxPitch", 72.0f);
        f32 minY = CVarGetFloat("gEnhancements.Camera.FreeLook.MinPitch", -49.0f);
        CVarSetFloat("gEnhancements.Camera.FreeLook.MaxPitch", std::max(maxY, minY));
        CVarSetFloat("gEnhancements.Camera.FreeLook.MinPitch", std::min(maxY, minY));
    }
}

void DrawCameraEnhancements3() {
    ImGui::SeparatorText("'Debug' Camera");
    SearchMenuGetItem(MENU_ITEM_ENABLE_DEBUG_CAMERA);
    if (CVarGetInteger("gEnhancements.Camera.DebugCam.Enable", 0)) {
        SearchMenuGetItem(MENU_ITEM_INVERT_CAMERA_X_AXIS);
        SearchMenuGetItem(MENU_ITEM_INVERT_CAMERA_Y_AXIS);
        SearchMenuGetItem(MENU_ITEM_THIRD_PERSON_CAMERA_X_SENSITIVITY);
        SearchMenuGetItem(MENU_ITEM_THIRD_PERSON_CAMERA_Y_SENSITIVITY);
        SearchMenuGetItem(MENU_ITEM_ENABLE_CAMERA_ROLL);
        SearchMenuGetItem(MENU_ITEM_CAMERA_SPEED);
    }
}

// Cheats
void DrawCheatEnhancements() {
    SearchMenuGetItem(MENU_ITEM_CHEATS_INFINITE_HEALTH);
    SearchMenuGetItem(MENU_ITEM_CHEATS_INFINITE_MAGIC);
    SearchMenuGetItem(MENU_ITEM_CHEATS_INFINITE_RUPEES);
    SearchMenuGetItem(MENU_ITEM_CHEATS_INFINITE_CONSUMABLES);
    SearchMenuGetItem(MENU_ITEM_CHEATS_LONG_FLOWER_GLIDE);
    SearchMenuGetItem(MENU_ITEM_CHEATS_NO_CLIP);
    SearchMenuGetItem(MENU_ITEM_CHEATS_INFINITE_RAZOR_SWORD);
    SearchMenuGetItem(MENU_ITEM_CHEATS_UNRESTRICTED_ITEMS);
    SearchMenuGetItem(MENU_ITEM_CHEATS_MOON_JUMP_ON_L);
}

// Gameplay
void DrawGameplayEnhancements() {
    ImGui::SeparatorText("Player");
    SearchMenuGetItem(MENU_ITEM_FAST_FLOWER_LAUNCH);
    SearchMenuGetItem(MENU_ITEM_INSTANT_PUTAWAY);
    SearchMenuGetItem(MENU_ITEM_CLIMB_SPEED);
    SearchMenuGetItem(MENU_ITEM_DPAD_EQUIPS);
    SearchMenuGetItem(MENU_ITEM_ALWAYS_WIN_DOGGY_RACE);
}

void DrawGameModesEnhancements() {
    ImGui::SeparatorText("Modes");
    SearchMenuGetItem(MENU_ITEM_PLAY_AS_KAFEI);
    SearchMenuGetItem(MENU_ITEM_TIME_MOVES_WHEN_YOU_MOVE);
}

void DrawSaveTimeEnhancements() {
    ImGui::SeparatorText("Saving");
    SearchMenuGetItem(MENU_ITEM_PERSIST_OWL_SAVES);
    SearchMenuGetItem(MENU_ITEM_PAUSE_MENU_SAVE);
    SearchMenuGetItem(MENU_ITEM_AUTOSAVE);
    SearchMenuGetItem(MENU_ITEM_AUTOSAVE_INTERVAL);

    ImGui::SeparatorText("Time Cycle");
    SearchMenuGetItem(MENU_ITEM_DISABLE_BOTTLE_RESET);
    SearchMenuGetItem(MENU_ITEM_DISABLE_CONSUMABLE_RESET);
    SearchMenuGetItem(MENU_ITEM_DISABLE_RAZOR_SWORD_RESET);
    SearchMenuGetItem(MENU_ITEM_DISABLE_RUPEE_RESET);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255, 255, 0, 255));
    ImGui::SeparatorText("Unstable");
    ImGui::PopStyleColor();
    SearchMenuGetItem(MENU_ITEM_DISABLE_SAVE_DELAY);
}

// Graphics
void DrawGraphicsEnhancements() {
    ImGui::SeparatorText("Clock");
    SearchMenuGetItem(MENU_ITEM_CLOCK_OPTIONS);
    SearchMenuGetItem(MENU_ITEM_MILITARY_CLOCK);

    ImGui::SeparatorText("Motion Blur");
    SearchMenuGetItem(MENU_ITEM_MOTION_BLUR_MODE);
    SearchMenuGetItem(MENU_ITEM_MOTION_BLUE_INTERPOLATE);
    if (CVarGetInteger("gEnhancements.Graphics.MotionBlur.Mode", 0) == 0) {
        SearchMenuGetItem(MENU_ITEM_MOTION_BLUR_ENABLE);
    } else if (CVarGetInteger("gEnhancements.Graphics.MotionBlur.Mode", 0) == 1) {
        CVarSetInteger("gEnhancements.Graphics.MotionBlur.Toggle", 0);
    }
    if (CVarGetInteger("gEnhancements.Graphics.MotionBlur.Mode", 0) == 2 ||
        CVarGetInteger("gEnhancements.Graphics.MotionBlur.Toggle", 0) == 1) {
        SearchMenuGetItem(MENU_ITEM_MOTION_BLUR_STRENGTH);
    }

    ImGui::SeparatorText("Other");
    SearchMenuGetItem(MENU_ITEM_AUTHENTIC_LOGO);
    SearchMenuGetItem(MENU_ITEM_BOW_RETICLE);
    SearchMenuGetItem(MENU_ITEM_DISABLE_BLACK_BARS);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255, 255, 0, 255));
    ImGui::SeparatorText("Unstable");
    ImGui::PopStyleColor();
    SearchMenuGetItem(MENU_ITEM_GEOMETRY_DISTANCE_CHECK);
    // BENTODO: Not implemented yet
    // UIWidgets::CVarCheckbox("Widescreen Actor Culling",
    //                         "gEnhancements.Graphics.ActorCullingAccountsForWidescreen",
    //                         { .tooltip = "Adjusts the culling planes to account for widescreen resolutions. "
    //                                      "This may have unintended side effects." });

    SearchMenuGetItem(MENU_ITEM_ACTOR_DRAW_DISTANCE);
    SearchMenuGetItem(MENU_ITEM_ACTOR_UPDATE_DISTANCE);
}

// Items/Songs
void DrawItemEnhancements() {
    ImGui::SeparatorText("Masks");
    SearchMenuGetItem(MENU_ITEM_BLAST_MASK_KEG_FORCE);
    SearchMenuGetItem(MENU_ITEM_FAST_TRANSFORMATION);
    SearchMenuGetItem(MENU_ITEM_FIERCE_DEITY_ANYWHERE);
    SearchMenuGetItem(MENU_ITEM_NO_BLAST_MASK_COOLDOWN);
}

void DrawSongEnhancements() {
    ImGui::SeparatorText("Items/Songs");
    SearchMenuGetItem(MENU_ITEM_ENABLE_SUNS_SONG);
    SearchMenuGetItem(MENU_ITEM_DPAD_OCARINA);
    SearchMenuGetItem(MENU_ITEM_PREVENT_DROPPED_OCARINA_INPUTS);
}

void DrawTimeSaverEnhancements1() {
    ImGui::SeparatorText("Cutscenes");
    SearchMenuGetItem(MENU_ITEM_HIDE_TITLE_CARDS);
    SearchMenuGetItem(MENU_ITEM_SKIP_ENTRANCE_CUTSCENES);
    SearchMenuGetItem(MENU_ITEM_SKIP_TO_FILE_SELECT);
    SearchMenuGetItem(MENU_ITEM_SKIP_INTRO_SEQUENCE);
    SearchMenuGetItem(MENU_ITEM_SKIP_STORY_CUTSCENES);
    SearchMenuGetItem(MENU_ITEM_SKIP_MISC_INTERACTIONS);
}

void DrawTimeSaverEnhancements2() {
    ImGui::SeparatorText("Dialogue");
    SearchMenuGetItem(MENU_ITEM_FAST_BANK_SELECTION);
    SearchMenuGetItem(MENU_ITEM_FAST_TEXT);
    SearchMenuGetItem(MENU_ITEM_FAST_MAGIC_ARROW_ANIM);
}

void DrawFixEnhancements() {
    ImGui::SeparatorText("Fixes");
    SearchMenuGetItem(MENU_ITEM_FIX_AMMO_COUNT_COLOR);
    SearchMenuGetItem(MENU_ITEM_FIX_HESS_WEIRDSHOT);
    SearchMenuGetItem(MENU_ITEM_FIX_TEXT_CONTROL_CHAR);
}

void DrawRestorationEnhancements() {
    ImGui::SeparatorText("Restorations");
    SearchMenuGetItem(MENU_ITEM_RESTORE_DISTANCE_FLIPS_HOPS);
    SearchMenuGetItem(MENU_ITEM_RESTORE_POWER_CROUCH_STAB);
    SearchMenuGetItem(MENU_ITEM_RESTORE_SIDEROLLS);
    SearchMenuGetItem(MENU_ITEM_RESTORE_TATL_ISG);
}

void DrawHudEditorContents() {
    // SearchMenuGetItem(MENU_ITEM_HUD_EDITOR);
    UIWidgets::WindowButton("Popout Hud Editor", "gWindows.HudEditor", mHudEditorWindow,
                            { .tooltip = "Enables the Hud Editor window, allowing you to edit your hud" });
    if (!CVarGetInteger("gWindows.HudEditor", 0)) {
        mHudEditorWindow->DrawElement();
    }
}

void DrawGeneralDevTools() {
    // PortNote: This should be hidden for ports on systems that are single-screen, and/or smaller than 1280x800.
    // Popout will assume size of 1280x800, and will break on those systems.
    SearchMenuGetItem(MENU_ITEM_MODERN_MENU_POPOUT);
    SearchMenuGetItem(MENU_ITEM_OPEN_APP_FILES);
    SearchMenuGetItem(MENU_ITEM_DEBUG_MODE_ENABLE);

    if (CVarGetInteger("gDeveloperTools.DebugEnabled", 0)) {
        SearchMenuGetItem(MENU_ITEM_DEBUG_BETTER_MAP_SELECT);
        SearchMenuGetItem(MENU_ITEM_DEBUG_SAVE_FILE_MODE);
    }

    SearchMenuGetItem(MENU_ITEM_PREVENT_ACTOR_UPDATE);
    SearchMenuGetItem(MENU_ITEM_PREVENT_ACTOR_DRAW);
    SearchMenuGetItem(MENU_ITEM_PREVENT_ACTOR_INIT);
    SearchMenuGetItem(MENU_ITEM_DISABLE_OBJECT_DEPENDECY);
    SearchMenuGetItem(MENU_ITEM_DEBUG_LOG_LEVEL);

    if (gPlayState != NULL) {
        ImGui::Separator();
        SearchMenuGetItem(MENU_ITEM_FRAME_ADVANCE_ENABLE);
        if (gPlayState->frameAdvCtx.enabled) {
            SearchMenuGetItem(MENU_ITEM_FRAME_ADVANCE_SINGLE);
            SearchMenuGetItem(MENU_ITEM_FRAME_ADVANCE_HOLD);
            if (ImGui::IsItemActive()) {
                CVarSetInteger("gDeveloperTools.FrameAdvanceTick", 1);
            }
        }
    }
    ImGui::PushStyleColor(ImGuiCol_Button, menuTheme[menuThemeIndex]);
    RenderWarpPointSection();
    ImGui::PopStyleColor(1);
}

void DrawCollisionViewerContents() {
    UIWidgets::WindowButton("Popout Collision Viewer", "gWindows.CollisionViewer", mCollisionViewerWindow,
                            { .color = menuTheme[menuThemeIndex], .tooltip = "Draws collision to the screen" });
    if (!CVarGetInteger("gWindows.CollisionViewer", 0)) {
        mCollisionViewerWindow->DrawElement();
    }
}

void DrawStatsContents() {
    UIWidgets::WindowButton(
        "Popout Stats", "gOpenWindows.Stats", mStatsWindow,
        { .color = menuTheme[menuThemeIndex],
          .tooltip = "Shows the stats window, with your FPS and frametimes, and the OS you're playing on" });
    if (!CVarGetInteger("gOpenWindows.Stats", 0)) {
        mStatsWindow->DrawElement();
    }
}

void DrawConsoleContents() {
    UIWidgets::WindowButton(
        "Popout Console", "gOpenWindows.Console", mConsoleWindow,
        { .color = menuTheme[menuThemeIndex],
          .tooltip = "Enables the console window, allowing you to input commands, type help for some examples" });
    if (!CVarGetInteger("gOpenWindows.Console", 0)) {
        mConsoleWindow->DrawElement();
    }
}

void DrawGfxDebuggerContents() {
    UIWidgets::WindowButton(
        "Popout Gfx Debugger", "gOpenWindows.GfxDebugger", mGfxDebuggerWindow,
        { .color = menuTheme[menuThemeIndex],
          .tooltip = "Enables the Gfx Debugger window, allowing you to input commands, type help for some examples" });
    if (!CVarGetInteger("gOpenWindows.GfxDebugger", 0)) {
        mGfxDebuggerWindow->DrawElement();
    }
}

void DrawSaveEditorContents() {
    UIWidgets::WindowButton("Popout Save Editor", "gWindows.SaveEditor", mSaveEditorWindow,
                            { .color = menuTheme[menuThemeIndex],
                              .tooltip = "Enables the Save Editor window, allowing you to edit your save file" });
    if (!CVarGetInteger("gWindows.SaveEditor", 0)) {
        mSaveEditorWindow->DrawElement();
    }
}

void DrawActorViewerContents() {
    UIWidgets::WindowButton(
        "Popout Actor Viewer", "gWindows.ActorViewer", mActorViewerWindow,
        { .color = menuTheme[menuThemeIndex],
          .tooltip = "Enables the Actor Viewer window, allowing you to view actors in the world." });
    if (!CVarGetInteger("gWindows.ActorViewer", 0)) {
        mActorViewerWindow->DrawElement();
    }
}

void DrawEventLogContents() {
    UIWidgets::WindowButton("Popout Event Log", "gWindows.EventLog", mEventLogWindow,
                            {
                                .color = menuTheme[menuThemeIndex],
                            });
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
    std::vector<UIWidgets::SidebarEntry> settingsSidebar = { { "Search", { DrawSearchSettings, nullptr, nullptr } },
                                                             { "General", { DrawGeneralSettings, nullptr, nullptr } },
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

CVarVariant GetCVarVariant(std::shared_ptr<Ship::CVar> cVar, CVarVariant cVarDefault) {
    if (cVar == nullptr) {
        return cVarDefault;
    }
    switch (cVar->Type) {
        default:
        case Ship::ConsoleVariableType::Integer:
            return cVar->Integer;
        case Ship::ConsoleVariableType::String:
            return cVar->String.c_str();
        case Ship::ConsoleVariableType::Float:
            return cVar->Float;
        //case Ship::ConsoleVariableType::Color:
        //    return cVar->Color;
        //case Ship::ConsoleVariableType::Color24:
        //    return cVar->Color24;
    }
}

void BenMenu::DrawElement() {
    for (auto& [reason, info] : disabledMap) {
        auto cVar = CVarGet(info.cVar);
        CVarVariant state = GetCVarVariant(cVar, info.cVarDefault);
        info.active = conditionFuncs.at(info.condition)(info.conditionVal, state);
    }
    menuThemeIndex = static_cast<ColorOption>(CVarGetInteger("gSettings.MenuTheme", 3));

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
