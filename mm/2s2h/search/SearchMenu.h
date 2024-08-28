#include "2s2h/Enhancements/Enhancements.h"
#include "2s2h/BenGui/UIWidgets.hpp"
#include "2s2h/BenGui/BenMenuBar.h"
#include <variant>
#include <tuple>

extern "C" {
#include "functions.h"
extern PlayState* gPlayState;
}

typedef enum {
    COLOR_WHITE,
    COLOR_GRAY,
    COLOR_DARK_GRAY,
    COLOR_INDIGO,
    COLOR_RED,
    COLOR_DARK_RED,
    COLOR_LIGHT_GREEN,
    COLOR_GREEN,
    COLOR_DARK_GREEN,
    COLOR_YELLOW,
} ColorOption;

typedef enum {
    DISABLE_FOR_CAMERAS_OFF,
    DISABLE_FOR_DEBUG_CAM_ON,
    DISABLE_FOR_DEBUG_CAM_OFF,
    DISABLE_FOR_FREE_LOOK_ON,
    DISABLE_FOR_FREE_LOOK_OFF,
    DISABLE_FOR_AUTO_SAVE_OFF,
    DISABLE_FOR_SAVE_NOT_LOADED,
    DISABLE_FOR_DEBUG_MODE_OFF,
    DISABLE_FOR_NO_VSYNC,
    DISABLE_FOR_NO_WINDOWED_FULLSCREEN,
    DISABLE_FOR_NO_MULTI_VIEWPORT,
    DISABLE_FOR_NOT_DIRECTX,
    DISABLE_FOR_MATCH_REFRESH_RATE_ON,
    DISABLE_FOR_MOTION_BLUR_MODE,
    DISABLE_FOR_MOTION_BLUR_OFF
} DisableOption;

struct widgetInfo;
using VoidFunc = void (*)();
using ConditionFunc = bool (*)();
using DisableVec = std::vector<DisableOption>;
using WidgetFunc = void (*)(widgetInfo&);
std::string disabledTempTooltip;
const char* disabledTooltip;
bool disabledValue = false;
ColorOption menuThemeIndex = COLOR_INDIGO;
const ImVec4 COLOR_NONE = { 0, 0, 0, 0 };

typedef enum {
    WIDGET_CHECKBOX,
    WIDGET_COMBOBOX,
    WIDGET_SLIDER_INT,
    WIDGET_SLIDER_FLOAT,
    WIDGET_BUTTON,
    WIDGET_COLOR_24,
    WIDGET_COLOR_32,
    WIDGET_SEARCH,
    WIDGET_SEPARATOR,
    WIDGET_SEPARATOR_TEXT,
    WIDGET_WINDOW_BUTTON,
    WIDGET_AUDIO_BACKEND,
    WIDGET_VIDEO_BACKEND
} WidgetType;

typedef enum {
    MOTION_BLUR_DYNAMIC,
    MOTION_BLUR_ALWAYS_OFF,
    MOTION_BLUR_ALWAYS_ON,
};

typedef enum {
    DEBUG_LOG_TRACE,
    DEBUG_LOG_DEBUG,
    DEBUG_LOG_INFO,
    DEBUG_LOG_WARN,
    DEBUG_LOG_ERROR,
    DEBUG_LOG_CRITICAL,
    DEBUG_LOG_OFF,
};

using CVarVariant = std::variant<int32_t, const char*, float, Color_RGBA8, Color_RGB8>;

struct WidgetOptions {
    CVarVariant min;
    CVarVariant max;
    CVarVariant defaultVariant;
    std::unordered_map<int32_t, const char*> comboBoxOptions;
    const ImVec4 color = COLOR_NONE;
    const char* windowName = "";
};

bool operator==(Color_RGB8 const& l, Color_RGB8 const& r) noexcept {
    return l.r == r.r && l.g == r.g && l.b == r.b;
}

bool operator==(Color_RGBA8 const& l, Color_RGBA8 const& r) noexcept {
    return l.r == r.r && l.g == r.g && l.b == r.b && l.a == r.a;
}

bool operator<(Color_RGB8 const& l, Color_RGB8 const& r) noexcept {
    return (l.r < r.r && l.g <= r.g && l.b <= r.b) || (l.r <= r.r && l.g < r.g && l.b <= r.b) ||
           (l.r <= r.r && l.g <= r.g && l.b < r.b);
}

bool operator<(Color_RGBA8 const& l, Color_RGBA8 const& r) noexcept {
    return (l.r < r.r && l.g <= r.g && l.b <= r.b && l.a <= r.a) ||
           (l.r <= r.r && l.g < r.g && l.b <= r.b && l.a <= r.a) ||
           (l.r <= r.r && l.g <= r.g && l.b < r.b && l.a <= r.a) ||
           (l.r <= r.r && l.g <= r.g && l.b <= r.b && l.a < r.a);
}

bool operator>(Color_RGB8 const& l, Color_RGB8 const& r) noexcept {
    return (l.r > r.r && l.g >= r.g && l.b >= r.b) || (l.r >= r.r && l.g > r.g && l.b >= r.b) ||
           (l.r >= r.r && l.g >= r.g && l.b > r.b);
}

bool operator>(Color_RGBA8 const& l, Color_RGBA8 const& r) noexcept {
    return (l.r > r.r && l.g >= r.g && l.b >= r.b && l.a >= r.a) ||
           (l.r >= r.r && l.g > r.g && l.b >= r.b && l.a >= r.a) ||
           (l.r >= r.r && l.g >= r.g && l.b > r.b && l.a >= r.a) ||
           (l.r >= r.r && l.g >= r.g && l.b >= r.b && l.a > r.a);
}

std::unordered_map<ColorOption, ImVec4> menuTheme = { { COLOR_WHITE, UIWidgets::Colors::White },
                                                      { COLOR_GRAY, UIWidgets::Colors::Gray },
                                                      { COLOR_DARK_GRAY, UIWidgets::Colors::DarkGray },
                                                      { COLOR_INDIGO, UIWidgets::Colors::Indigo },
                                                      { COLOR_RED, UIWidgets::Colors::Red },
                                                      { COLOR_DARK_RED, UIWidgets::Colors::DarkRed },
                                                      { COLOR_LIGHT_GREEN, UIWidgets::Colors::LightGreen },
                                                      { COLOR_GREEN, UIWidgets::Colors::Green },
                                                      { COLOR_DARK_GREEN, UIWidgets::Colors::DarkGreen },
                                                      { COLOR_YELLOW, UIWidgets::Colors::Yellow } };

typedef enum {
    SETTINGS_INDEX_SEARCH,
    SETTINGS_INDEX_GENERAL,
    SETTINGS_INDEX_AUDIO,
    SETTINGS_INDEX_GRAPHICS,
    SETTINGS_INDEX_CONTROLS,
    ENHANCEMENTS_INDEX_CAMERA,
    ENHANCEMENTS_INDEX_CHEATS,
    ENHANCEMENTS_INDEX_GAMEPLAY,
    ENHANCEMENTS_INDEX_GRAPHICS,
    ENHANCEMENTS_INDEX_ITEMS_SONGS,
    ENHANCEMENTS_INDEX_TIME_SAVERS,
    ENHANCEMENTS_INDEX_FIXES,
    ENHANCEMENTS_INDEX_RESTORATIONS,
    ENHANCEMENTS_INDEX_HUD_EDITOR,
    DEV_TOOLS_INDEX_GENERAL,
    DEV_TOOLS_INDEX_COLLISION,
    DEV_TOOLS_INDEX_STATS,
    DEV_TOOLS_INDEX_CONSOLE,
    DEV_TOOLS_INDEX_GFX_DEBUGGER,
    DEV_TOOLS_INDEX_SAVE_EDITOR,
    DEV_TOOLS_INDEX_ACTOR_VIEWER,
    DEV_TOOLS_INDEX_EVENT_LOG
} SidebarEntryIndex;

struct widgetInfo {
    const char* widgetName;
    const char* widgetCVar;
    const char* widgetTooltip;
    WidgetType widgetType;
    WidgetOptions widgetOptions;
    WidgetFunc widgetCallback = nullptr;
    WidgetFunc modifierFunc = nullptr;
    DisableVec activeDisables = {};
    bool isHidden = false;
};

struct disabledInfo {
    ConditionFunc evaluation;
    const char* reason;
    bool active = false;
    int32_t state = 0;
};

struct SidebarEntry {
    std::string label;
    std::vector<std::vector<widgetInfo>> columnWidgets;
    uint32_t columnCount = 1;
};

struct MainMenuEntry {
    std::string label;
    std::vector<SidebarEntryIndex> sidebarEntries;
    const char* sidebarCvar;
};

namespace BenGui {
extern std::shared_ptr<std::vector<Ship::WindowBackend>> availableWindowBackends;
extern std::unordered_map<Ship::WindowBackend, const char*> availableWindowBackendsMap;
extern Ship::WindowBackend configWindowBackend;
extern void UpdateWindowBackendObjects();

std::vector<MainMenuEntry> menuEntries;
static ImGuiTextFilter menuSearch;

static std::pair<DisableOption, disabledInfo> disabledMap[] = {
    { DISABLE_FOR_CAMERAS_OFF,
      { []() -> bool {
           return !CVarGetInteger("gEnhancements.Camera.DebugCam.Enable", 0) &&
                  !CVarGetInteger("gEnhancements.Camera.FreeLook.Enable", 0);
       },
        "Both Debug Camera and Free Look are Disabled" } },
    { DISABLE_FOR_DEBUG_CAM_ON,
      { []() -> bool { return CVarGetInteger("gEnhancements.Camera.DebugCam.Enable", 0); },
        "Debug Camera is Enabled" } },
    { DISABLE_FOR_DEBUG_CAM_OFF,
      { []() -> bool { return !CVarGetInteger("gEnhancements.Camera.DebugCam.Enable", 0); },
        "Debug Camera is Disabled" } },
    { DISABLE_FOR_FREE_LOOK_ON,
      { []() -> bool { return CVarGetInteger("gEnhancements.Camera.FreeLook.Enable", 0); }, "Free Look is Enabled" } },
    { DISABLE_FOR_FREE_LOOK_OFF,
      { []() -> bool { return !CVarGetInteger("gEnhancements.Camera.FreeLook.Enable", 0); },
        "Free Look is Disabled" } },
    { DISABLE_FOR_AUTO_SAVE_OFF,
      { []() -> bool { return !CVarGetInteger("gEnhancements.Saving.Autosave", 0); }, "AutoSave is Disabled" } },
    { DISABLE_FOR_SAVE_NOT_LOADED, { []() -> bool { return gPlayState == NULL; }, "Save Not Loaded" } },
    { DISABLE_FOR_DEBUG_MODE_OFF,
      { []() -> bool { return !CVarGetInteger("gDeveloperTools.DebugEnabled", 0); }, "Save Not Loaded" } },
    { DISABLE_FOR_NO_VSYNC,
      { []() -> bool { return !Ship::Context::GetInstance()->GetWindow()->CanDisableVerticalSync(); },
        "Disabling VSync not supported" } },
    { DISABLE_FOR_NO_WINDOWED_FULLSCREEN,
      { []() -> bool { return !Ship::Context::GetInstance()->GetWindow()->SupportsWindowedFullscreen(); },
        "Windowed Fullscreen not supported" } },
    { DISABLE_FOR_NO_MULTI_VIEWPORT,
      { []() -> bool { return !Ship::Context::GetInstance()->GetWindow()->GetGui()->SupportsViewports(); },
        "Multi-viewports not supported" } },
    { DISABLE_FOR_MATCH_REFRESH_RATE_ON,
      { []() -> bool { return !CVarGetInteger("gEnhancements.Camera.FreeLook.Enable", 0); },
        "Free Look is Disabled" } },
    { DISABLE_FOR_MOTION_BLUR_MODE,
      { []() -> bool { return !CVarGetInteger("gEnhancements.Graphics.MotionBlur.Mode", 0); },
        "Free Look is Disabled" } },
    { DISABLE_FOR_MOTION_BLUR_OFF,
      { []() -> bool { return !CVarGetInteger("gEnhancements.Graphics.MotionBlur.Toggle", 0); },
        "Free Look is Disabled" } }
};

std::unordered_map<int32_t, const char*> menuThemeOptions = {
    { COLOR_WHITE, "White" },
    { COLOR_GRAY, "Gray" },
    { COLOR_DARK_GRAY, "Dark Gray" },
    { COLOR_INDIGO, "Indigo" },
    { COLOR_RED, "Red" },
    { COLOR_DARK_RED, "Dark Red" },
    { COLOR_LIGHT_GREEN, "Light Green" },
    { COLOR_GREEN, "Green" },
    { COLOR_DARK_GREEN, "Dark Green" },
    { COLOR_YELLOW, "Yellow" },
};

static const std::unordered_map<int32_t, const char*> alwaysWinDoggyraceOptions = {
    { ALWAYS_WIN_DOGGY_RACE_OFF, "Off" },
    { ALWAYS_WIN_DOGGY_RACE_MASKOFTRUTH, "When owning Mask of Truth" },
    { ALWAYS_WIN_DOGGY_RACE_ALWAYS, "Always" },
};

static const std::unordered_map<int32_t, const char*> clockTypeOptions = {
    { CLOCK_TYPE_ORIGINAL, "Original" },
    { CLOCK_TYPE_TEXT_BASED, "Text only" },
};

static const std::unordered_map<int32_t, const char*> textureFilteringMap = {
    { FILTER_THREE_POINT, "Three-Point" },
    { FILTER_LINEAR, "Linear" },
    { FILTER_NONE, "None" },
};

static const std::unordered_map<int32_t, const char*> motionBlurOptions = {
    { MOTION_BLUR_DYNAMIC, "Dynamic (default)" },
    { MOTION_BLUR_ALWAYS_OFF, "Always Off" },
    { MOTION_BLUR_ALWAYS_ON, "Always On" },
};

static const std::unordered_map<int32_t, const char*> debugSaveOptions = {
    { DEBUG_SAVE_INFO_COMPLETE, "100\% save" },
    { DEBUG_SAVE_INFO_VANILLA_DEBUG, "Vanilla debug save" },
    { DEBUG_SAVE_INFO_NONE, "Empty save" },
};

static const std::unordered_map<int32_t, const char*> logLevels = {
    { DEBUG_LOG_TRACE, "Trace" }, { DEBUG_LOG_DEBUG, "Debug" }, { DEBUG_LOG_INFO, "Info" },
    { DEBUG_LOG_WARN, "Warn" },   { DEBUG_LOG_ERROR, "Error" }, { DEBUG_LOG_CRITICAL, "Critical" },
    { DEBUG_LOG_OFF, "Off" },
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

void FreeLookPitchMinMax() {
    f32 maxY = CVarGetFloat("gEnhancements.Camera.FreeLook.MaxPitch", 72.0f);
    f32 minY = CVarGetFloat("gEnhancements.Camera.FreeLook.MinPitch", -49.0f);
    CVarSetFloat("gEnhancements.Camera.FreeLook.MaxPitch", std::max(maxY, minY));
    CVarSetFloat("gEnhancements.Camera.FreeLook.MinPitch", std::min(maxY, minY));
}

std::map<SidebarEntryIndex, SidebarEntry> sidebarEntries = {
    { SETTINGS_INDEX_SEARCH, { "Search", { {} } } },
    { SETTINGS_INDEX_GENERAL, { "General", { {} }, 3 } },
    { SETTINGS_INDEX_AUDIO, { "Audio", { {} }, 3 } },
    { SETTINGS_INDEX_GRAPHICS, { "Graphics", { {} }, 3 } },
    { SETTINGS_INDEX_CONTROLS, { "Controls", { {} } } },
    { ENHANCEMENTS_INDEX_CAMERA, { "Camera", { {}, {} }, 3 } },
    { ENHANCEMENTS_INDEX_CHEATS, { "Cheats", { {} }, 3 } },
    { ENHANCEMENTS_INDEX_GAMEPLAY, { "Gameplay", { {}, {}, {} }, 3 } },
    { ENHANCEMENTS_INDEX_GRAPHICS, { "Graphics", { {} }, 3 } },
    { ENHANCEMENTS_INDEX_ITEMS_SONGS, { "Items/Songs", { {}, {} }, 3 } },
    { ENHANCEMENTS_INDEX_TIME_SAVERS, { "Time Savers", { {}, {} }, 3 } },
    { ENHANCEMENTS_INDEX_FIXES, { "Fixes", { {} }, 3 } },
    { ENHANCEMENTS_INDEX_RESTORATIONS, { "Restorations", { {} }, 3 } },
    { ENHANCEMENTS_INDEX_HUD_EDITOR, { "Hud Editor", { {} }, 2 } },
    { DEV_TOOLS_INDEX_GENERAL, { "General", { {} }, 3 } },
    { DEV_TOOLS_INDEX_COLLISION, { "Collision Viewer", { {} }, 2 } },
    { DEV_TOOLS_INDEX_STATS, { "Stats", { {} }, 2 } },
    { DEV_TOOLS_INDEX_CONSOLE, { "Console", { {} }, 2 } },
    { DEV_TOOLS_INDEX_GFX_DEBUGGER, { "Gfx Debugger", { {} }, 2 } },
    { DEV_TOOLS_INDEX_SAVE_EDITOR, { "Save Editor", { {} }, 2 } },
    { DEV_TOOLS_INDEX_ACTOR_VIEWER, { "Actor Viewer", { {} }, 2 } },
    { DEV_TOOLS_INDEX_EVENT_LOG, { "Event Log", { {} }, 2 } }
};

void AddSettings() {
    // General
    sidebarEntries[SETTINGS_INDEX_SEARCH].columnWidgets[0].push_back(
        { "Menu Theme", "", "Searches all menus for the given text, including tooltips.", WIDGET_SEARCH, {} });
    std::vector<widgetInfo>* widgets = &sidebarEntries[SETTINGS_INDEX_GENERAL].columnWidgets[0];
    // Menu Theme
    widgets->push_back({ "Menu Theme",
                         "gSettings.MenuTheme",
                         "Changes the Theme of the Menu Widgets.",
                         WIDGET_COMBOBOX,
                         { 0, 0, 0, menuThemeOptions } });
    // General Settings
#if not defined(__SWITCH__) and not defined(__WIIU__)
    widgets->push_back({ "Menubar Controller Navigation",
                         CVAR_IMGUI_CONTROLLER_NAV,
                         "Allows controller navigation of the SOH menu bar (Settings, Enhancements,...)\nCAUTION: "
                         "This will disable game inputs while the menubar is visible.\n\nD-pad to move between "
                         "items, A to select, and X to grab focus on the menu bar",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Cursor Always Visible",
                         "gSettings.CursorVisibility",
                         "Makes the cursor always visible, even in full screen.",
                         WIDGET_CHECKBOX,
                         {},
                         [](widgetInfo& info) {
                             Ship::Context::GetInstance()->GetWindow()->SetForceCursorVisibility(
                                 CVarGetInteger("gSettings.CursorVisibility", 0));
                         } });
#endif
    widgets = &sidebarEntries[SETTINGS_INDEX_AUDIO].columnWidgets[0];
    widgets->push_back({ "Hide Menu Hotkey Text",
                         "gSettings.DisableMenuShortcutNotify",
                         "Prevents showing the text telling you the shortcut to open the menu\n"
                         "when the menu isn't open.",
                         WIDGET_CHECKBOX,
                         {} });
    // Audio Settings
    widgets->push_back({ "Master Volume: %.2f%%",
                         "gSettings.Audio.MasterVolume",
                         "Adjust overall sound volume.",
                         WIDGET_SLIDER_FLOAT,
                         { 0.0f, 100.0f, 100.0f } });
    widgets->push_back({ "Main Music Volume: %.2f%%",
                         "gSettings.Audio.MainMusicVolume",
                         "Adjust the Background Music volume.",
                         WIDGET_SLIDER_FLOAT,
                         { 0.0f, 100.0f, 100.0f },
                         [](widgetInfo& info) {
                             AudioSeq_SetPortVolumeScale(SEQ_PLAYER_BGM_MAIN,
                                                         CVarGetFloat("gSettings.Audio.MainMusicVolume", 1.0f));
                         } });
    widgets->push_back({ "Sub Music Volume: %.2f%%",
                         "gSettings.Audio.SubMusicVolume",
                         "Adjust the Sub Music volume.",
                         WIDGET_SLIDER_FLOAT,
                         { 0.0f, 100.0f, 100.0f },
                         [](widgetInfo& info) {
                             AudioSeq_SetPortVolumeScale(SEQ_PLAYER_BGM_SUB,
                                                         CVarGetFloat("gSettings.Audio.SubMusicVolume", 1.0f));
                         } });
    widgets->push_back({ "Sound Effects Volume: %.2f%%",
                         "gSettings.Audio.SoundEffectsVolume",
                         "Adjust the Sound Effects volume.",
                         WIDGET_SLIDER_FLOAT,
                         { 0.0f, 100.0f, 100.0f },
                         [](widgetInfo& info) {
                             AudioSeq_SetPortVolumeScale(SEQ_PLAYER_SFX,
                                                         CVarGetFloat("gSettings.Audio.SoundEffectsVolume", 1.0f));
                         } });
    widgets->push_back({ "Fanfare Volume: %.2f%%",
                         "gSettings.Audio.FanfareVolume",
                         "Adjust the Fanfare volume.",
                         WIDGET_SLIDER_FLOAT,
                         { 0.0f, 100.0f, 100.0f },
                         [](widgetInfo& info) {
                             AudioSeq_SetPortVolumeScale(SEQ_PLAYER_FANFARE,
                                                         CVarGetFloat("gSettings.Audio.FanfareVolume", 1.0f));
                         } });
    widgets->push_back({ "Ambience Volume: %.2f%%",
                         "gSettings.Audio.AmbienceVolume",
                         "Adjust the Ambient Sound volume.",
                         WIDGET_SLIDER_FLOAT,
                         { 0.0f, 100.0f, 100.0f },
                         [](widgetInfo& info) {
                             AudioSeq_SetPortVolumeScale(SEQ_PLAYER_AMBIENCE,
                                                         CVarGetFloat("gSettings.Audio.AmbienceVolume", 1.0f));
                         } });
    widgets->push_back({ "Audio API", NULL, "Sets the audio API used by the game. Requires a relaunch to take effect.",
                         WIDGET_AUDIO_BACKEND });
    // Graphics Settings
    widgets = &sidebarEntries[SETTINGS_INDEX_GRAPHICS].columnWidgets[0];
    widgets->push_back({ "Toggle Fullscreen",
                         "gSettings.Fullscreen",
                         "Toggles Fullscreen On/Off.",
                         WIDGET_CHECKBOX,
                         {},
                         [](widgetInfo& info) { Ship::Context::GetInstance()->GetWindow()->ToggleFullscreen(); } });
#ifndef __APPLE__
    widgets->push_back({ "Internal Resolution: %f%%",
                         CVAR_INTERNAL_RESOLUTION,
                         "Multiplies your output resolution by the value inputted, as a more intensive but effective "
                         "form of anti-aliasing.",
                         WIDGET_SLIDER_FLOAT,
                         { 50.0f, 200.0f, 100.0f },
                         [](widgetInfo& info) {
                             Ship::Context::GetInstance()->GetWindow()->SetResolutionMultiplier(
                                 CVarGetFloat(CVAR_INTERNAL_RESOLUTION, 1));
                         } });
#endif
#ifndef __WIIU__
    widgets->push_back(
        { "Anti-aliasing (MSAA): %d",
          CVAR_MSAA_VALUE,
          "Activates MSAA (multi-sample anti-aliasing) from 2x up to 8x, to smooth the edges of rendered "
          "geometry.\n"
          "Higher sample count will result in smoother edges on models, but may reduce performance.",
          WIDGET_SLIDER_INT,
          { 1, 8, 1 },
          [](widgetInfo& info) {
              Ship::Context::GetInstance()->GetWindow()->SetMsaaLevel(CVarGetInteger(CVAR_MSAA_VALUE, 1));
          } });
#endif
    static int32_t maxFps;
    std::string tooltip = "";
    if (Ship::Context::GetInstance()->GetWindow()->GetWindowBackend() == Ship::WindowBackend::FAST3D_DXGI_DX11) {
        maxFps = 360;
        tooltip = "Uses Matrix Interpolation to create extra frames, resulting in smoother graphics. This is "
                  "purely visual and does not impact game logic, execution of glitches etc.\n\n A higher target "
                  "FPS than your monitor's refresh rate will waste resources, and might give a worse result.";
    } else {
        maxFps = Ship::Context::GetInstance()->GetWindow()->GetCurrentRefreshRate();
        tooltip = "Uses Matrix Interpolation to create extra frames, resulting in smoother graphics. This is "
                  "purely visual and does not impact game logic, execution of glitches etc.";
    }
    widgets->push_back({ "Current FPS: %d",
                         "gInterpolationFPS",
                         tooltip.c_str(),
                         WIDGET_SLIDER_INT,
                         { 20, maxFps, 20 },
                         [](widgetInfo& info) {
                             int32_t defaultVariant = std::get<int32_t>(info.widgetOptions.defaultVariant);
                             if (CVarGetInteger(info.widgetCVar, defaultVariant) == defaultVariant) {
                                 info.widgetName = "Current FPS: Original (%d)";
                             } else {
                                 info.widgetName = "Current FPS: %d";
                             }
                         },
                         [](widgetInfo& info) {
                             if (disabledMap[DISABLE_FOR_MATCH_REFRESH_RATE_ON].second.active)
                                 info.activeDisables.push_back(DISABLE_FOR_MATCH_REFRESH_RATE_ON);
                         } });
    if (Ship::Context::GetInstance()->GetWindow()->GetWindowBackend() == Ship::WindowBackend::FAST3D_DXGI_DX11) {
        widgets->push_back(
            { "Match Refresh Rate",
              "",
              "Matches interpolation value to the current game's window refresh rate.",
              WIDGET_BUTTON,
              {},
              [](widgetInfo& info) {
                  int hz = Ship::Context::GetInstance()->GetWindow()->GetCurrentRefreshRate();
                  if (hz >= 20 && hz <= 360) {
                      CVarSetInteger("gInterpolationFPS", hz);
                      Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
                  }
              },
              [](widgetInfo& info) {
                  if (disabledMap[DISABLE_FOR_NOT_DIRECTX].second.active)
                      info.isHidden = true;
              } });
    } else {
        widgets->push_back({ "Match Refresh Rate",
                             "gMatchRefreshRate",
                             "Matches interpolation value to the current game's window refresh rate.",
                             WIDGET_CHECKBOX,
                             {},
                             nullptr,
                             [](widgetInfo& info) {
                                 if (!disabledMap[DISABLE_FOR_NOT_DIRECTX].second.active)
                                     info.isHidden = true;
                             } });
    }
    widgets->push_back(
        { "Jitter fix : >= % d FPS",
          "gExtraLatencyThreshold",
          "When Interpolation FPS setting is at least this threshold, add one frame of input "
          "lag (e.g. 16.6 ms for 60 FPS) in order to avoid jitter. This setting allows the "
          "CPU to work on one frame while GPU works on the previous frame.\nThis setting "
          "should be used when your computer is too slow to do CPU + GPU work in time.",
          WIDGET_SLIDER_INT,
          { 0, 360, 80 },
          nullptr,
          [](widgetInfo& info) { info.isHidden = disabledMap[DISABLE_FOR_NOT_DIRECTX].second.active; } });
    widgets->push_back({ "Renderer API (Needs reload)", NULL,
                         "Sets the audio API used by the game. Requires a relaunch to take effect.",
                         WIDGET_AUDIO_BACKEND });
    widgets->push_back(
        { "Enable Vsync", CVAR_VSYNC_ENABLED, "Enables Vsync.", WIDGET_CHECKBOX, {}, nullptr, [](widgetInfo& info) {
             info.isHidden = disabledMap[DISABLE_FOR_NO_VSYNC].second.active;
         } });
    widgets->push_back(
        { "Windowed Fullscreen",
          CVAR_SDL_WINDOWED_FULLSCREEN,
          "Enables Windowed Fullscreen Mode.",
          WIDGET_CHECKBOX,
          {},
          nullptr,
          [](widgetInfo& info) { info.isHidden = disabledMap[DISABLE_FOR_NO_WINDOWED_FULLSCREEN].second.active; } });
    widgets->push_back(
        { "Allow multi-windows",
          CVAR_ENABLE_MULTI_VIEWPORTS,
          "Allows multiple windows to be opened at once. Requires a reload to take effect.",
          WIDGET_CHECKBOX,
          {},
          nullptr,
          [](widgetInfo& info) { info.isHidden = disabledMap[DISABLE_FOR_NO_MULTI_VIEWPORT].second.active; } });
    widgets->push_back({ "Texture Filter (Needs reload)",
                         CVAR_TEXTURE_FILTER,
                         "Sets the applied Texture Filtering.",
                         WIDGET_COMBOBOX,
                         { 0, 0, 0, textureFilteringMap } });
    // Input Editor
    sidebarEntries[SETTINGS_INDEX_CONTROLS].columnWidgets[0].push_back({ "Popout Input Editor",
                                                                         "gWindows.BenInputEditor",
                                                                         "Enables the separate Input Editor window.",
                                                                         WIDGET_WINDOW_BUTTON,
                                                                         { .windowName = "2S2H Input Editor" } });

    //    UIWidgets::WindowButton("Popout Input Editor", "gWindows.BenInputEditor", mBenInputEditorWindow,
    //        { .tooltip = "Enables the separate Input Editor window." });
    //    if (!CVarGetInteger("gWindows.BenInputEditor", 0)) {
    //        mBenInputEditorWindow->DrawPortTabContents(0);
    //    }
    //};
}

void AddEnhancements() {
    // Camera Snap Fix
    std::vector<widgetInfo>* widgets = &sidebarEntries[ENHANCEMENTS_INDEX_CAMERA].columnWidgets[0];
    widgets->push_back({ "Fixes", NULL, NULL, WIDGET_SEPARATOR_TEXT });
    widgets->push_back({ "Fix Targetting Camera Snap",
                         "gEnhancements.Camera.FixTargettingCameraSnap",
                         "Fixes the camera snap that occurs when you are moving and press the targetting button.",
                         WIDGET_CHECKBOX,
                         {} });
    // Camera Enhancements
    widgets = &sidebarEntries[ENHANCEMENTS_INDEX_CAMERA].columnWidgets[1];
    widgets->push_back({ "Cameras", NULL, NULL, WIDGET_SEPARATOR_TEXT });
    widgets->push_back({ "Free Look",
                         "gEnhancements.Camera.FreeLook.Enable",
                         "Enables free look camera control\nNote: You must remap C buttons off of the right "
                         "stick in the controller config menu, and map the camera stick to the right stick.",
                         WIDGET_CHECKBOX,
                         {},
                         [](widgetInfo& info) { RegisterCameraFreeLook(); },
                         [](widgetInfo& info) {
                             if (disabledMap[DISABLE_FOR_DEBUG_CAM_ON].second.active)
                                 info.activeDisables.push_back(DISABLE_FOR_DEBUG_CAM_ON);
                         } });
    widgets->push_back(
        { "Camera Distance: %d",
          "gEnhancements.Camera.FreeLook.MaxCameraDistance",
          "Maximum Camera Distance for Free Look.",
          WIDGET_SLIDER_INT,
          { 100, 900, 185 },
          nullptr,
          [](widgetInfo& info) { info.isHidden = disabledMap[DISABLE_FOR_FREE_LOOK_OFF].second.active; } });
    widgets->push_back(
        { "Camera Transition Speed: %d",
          "gEnhancements.Camera.FreeLook.TransitionSpeed",
          "Can someone help me?",
          WIDGET_SLIDER_INT,
          { 1, 900, 25 },
          nullptr,
          [](widgetInfo& info) { info.isHidden = disabledMap[DISABLE_FOR_FREE_LOOK_OFF].second.active; } });
    widgets->push_back(
        { "Max Camera Height Angle: %.0f\xC2\xB0",
          "gEnhancements.Camera.FreeLook.MaxPitch",
          "Maximum Height of the Camera.",
          WIDGET_SLIDER_FLOAT,
          { -8900.0f, 8900.0f, 7200.0f },
          [](widgetInfo& info) { FreeLookPitchMinMax(); },
          [](widgetInfo& info) { info.isHidden = disabledMap[DISABLE_FOR_FREE_LOOK_OFF].second.active; } });
    widgets->push_back(
        { "Min Camera Height Angle: %.0f\xC2\xB0",
          "gEnhancements.Camera.FreeLook.MinPitch",
          "Minimum Height of the Camera.",
          WIDGET_SLIDER_FLOAT,
          { -8900.0f, 8900.0f, -4900.0f },
          [](widgetInfo& info) { FreeLookPitchMinMax(); },
          [](widgetInfo& info) { info.isHidden = disabledMap[DISABLE_FOR_FREE_LOOK_OFF].second.active; } });
    widgets->push_back({ "Debug Camera",
                         "gEnhancements.Camera.DebugCam.Enable",
                         "Enables free camera control.",
                         WIDGET_CHECKBOX,
                         {},
                         ([](widgetInfo& info) { RegisterDebugCam(); }),
                         [](widgetInfo& info) {
                             if (disabledMap[DISABLE_FOR_FREE_LOOK_ON].second.active) {
                                 info.activeDisables.push_back(DISABLE_FOR_FREE_LOOK_ON);
                             }
                         } });
    widgets->push_back({ "Invert Camera X Axis",
                         "gEnhancements.Camera.RightStick.InvertXAxis",
                         "Inverts the Camera X Axis",
                         WIDGET_CHECKBOX,
                         {},
                         nullptr,
                         [](widgetInfo& info) {
                             if (disabledMap[DISABLE_FOR_CAMERAS_OFF].second.active) {
                                 info.activeDisables.push_back(DISABLE_FOR_CAMERAS_OFF);
                             }
                         } });
    widgets->push_back({ "Invert Camera Y Axis",
                         "gEnhancements.Camera.RightStick.InvertYAxis",
                         "Inverts the Camera Y Axis",
                         WIDGET_CHECKBOX,
                         {},
                         nullptr,
                         [](widgetInfo& info) {
                             if (disabledMap[DISABLE_FOR_CAMERAS_OFF].second.active) {
                                 info.activeDisables.push_back(DISABLE_FOR_CAMERAS_OFF);
                             }
                         } });
    widgets->push_back({ "Third-Person Horizontal Sensitivity: %.0f",
                         "gEnhancements.Camera.RightStick.CameraSensitivity.X",
                         "Adjust the Sensitivity of the x axis when in Third Person.",
                         WIDGET_SLIDER_FLOAT,
                         { 1.0f, 500.0f, 100.0f },
                         nullptr,
                         [](widgetInfo& info) {
                             if (disabledMap[DISABLE_FOR_CAMERAS_OFF].second.active) {
                                 info.activeDisables.push_back(DISABLE_FOR_CAMERAS_OFF);
                             }
                         } });
    widgets->push_back({ "Third-Person Vertical Sensitivity: %.0f",
                         "gEnhancements.Camera.RightStick.CameraSensitivity.Y",
                         "Adjust the Sensitivity of the x axis when in Third Person.",
                         WIDGET_SLIDER_FLOAT,
                         { 1.0f, 500.0f, 100.0f },
                         nullptr,
                         [](widgetInfo& info) {
                             if (disabledMap[DISABLE_FOR_CAMERAS_OFF].second.active) {
                                 info.activeDisables.push_back(DISABLE_FOR_CAMERAS_OFF);
                             }
                         } });
    widgets->push_back(
        { "Enable Roll (6\xC2\xB0 of Freedom)",
          "gEnhancements.Camera.DebugCam.6DOF",
          "This allows for all six degrees of movement with the camera, NOTE: Yaw will work "
          "differently in this system, instead rotating around the focal point"
          ", rather than a polar axis.",
          WIDGET_CHECKBOX,
          {},
          nullptr,
          [](widgetInfo& info) { info.isHidden = disabledMap[DISABLE_FOR_DEBUG_CAM_OFF].second.active; } });
    widgets->push_back(
        { "Camera Speed: %.0f",
          "gEnhancements.Camera.DebugCam.CameraSpeed",
          "Adjusts the speed of the Camera.",
          WIDGET_SLIDER_FLOAT,
          { 10.0f, 300.0f, 50.0f },
          nullptr,
          [](widgetInfo& info) { info.isHidden = disabledMap[DISABLE_FOR_DEBUG_CAM_OFF].second.active; } });
    // Cheats
    widgets = &sidebarEntries[ENHANCEMENTS_INDEX_CHEATS].columnWidgets[0];
    widgets->push_back(
        { "Infinite Health", "gCheats.InfiniteHealth", "Always have full Hearts.", WIDGET_CHECKBOX, {} });
    widgets->push_back({ "Infinite Magic", "gCheats.InfiniteMagic", "Always have full Magic.", WIDGET_CHECKBOX, {} });
    widgets->push_back(
        { "Infinite Rupees", "gCheats.InfiniteRupees", "Always have a full Wallet.", WIDGET_CHECKBOX, {} });
    widgets->push_back({ "Infinite Consumables",
                         "gCheats.InfiniteConsumables",
                         "Always have max Consumables, you must have collected the consumables first.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Longer Deku Flower Glide",
                         "gCheats.LongerFlowerGlide",
                         "Allows Deku Link to glide longer, no longer dropping after a certain distance.",
                         WIDGET_CHECKBOX,
                         {},
                         ([](widgetInfo& info) { RegisterLongerFlowerGlide(); }) });
    widgets->push_back({ "No Clip", "gCheats.NoClip", "Allows Link to phase through collision.", WIDGET_CHECKBOX, {} });
    widgets->push_back({ "Unbreakable Razor Sword",
                         "gCheats.UnbreakableRazorSword",
                         "Allows to Razor Sword to be used indefinitely without dulling its blade.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Unrestricted Items",
                         "gCheats.UnrestrictedItems",
                         "Allows all Forms to use all Items.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Moon Jump on L",
                         "gCheats.MoonJumpOnL",
                         "Holding L makes you float into the air.",
                         WIDGET_CHECKBOX,
                         {},
                         ([](widgetInfo& info) { RegisterMoonJumpOnL(); }) });
    // Gameplay Enhancements
    widgets = &sidebarEntries[ENHANCEMENTS_INDEX_GAMEPLAY].columnWidgets[0];
    widgets->push_back({ "Player", NULL, NULL, WIDGET_SEPARATOR_TEXT });
    widgets->push_back(
        { "Fast Deku Flower Launch",
          "gEnhancements.Player.FastFlowerLaunch",
          "Speeds up the time it takes to be able to get maximum height from launching out of a deku flower",
          WIDGET_CHECKBOX,
          {},
          ([](widgetInfo& info) { RegisterFastFlowerLaunch(); }) });
    widgets->push_back({ "Instant Putaway",
                         "gEnhancements.Player.InstantPutaway",
                         "Allows Link to instantly puts away held item without waiting.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Climb speed",
                         "gEnhancements.PlayerMovement.ClimbSpeed",
                         "Increases the speed at which Link climbs vines and ladders.",
                         WIDGET_SLIDER_INT,
                         { 1, 5, 1 } });
    widgets->push_back({ "Dpad Equips",
                         "gEnhancements.Dpad.DpadEquips",
                         "Allows you to equip items to your d-pad",
                         WIDGET_CHECKBOX,
                         {},
                         nullptr });
    widgets->push_back({ "Always Win Doggy Race",
                         "gEnhancements.Minigames.AlwaysWinDoggyRace",
                         "Makes the Doggy Race easier to win.",
                         WIDGET_COMBOBOX,
                         { 0, 0, 0, alwaysWinDoggyraceOptions } });
    // Game Modes
    widgets = &sidebarEntries[ENHANCEMENTS_INDEX_GAMEPLAY].columnWidgets[1];
    widgets->push_back({ "Modes", NULL, NULL, WIDGET_SEPARATOR_TEXT });
    widgets->push_back(
        { "Play as Kafei", "gModes.PlayAsKafei", "Requires scene reload to take effect.", WIDGET_CHECKBOX, {} });
    widgets->push_back({ "Time Moves when you Move",
                         "gModes.TimeMovesWhenYouMove",
                         "Time only moves when Link is not standing still.",
                         WIDGET_CHECKBOX,
                         {},
                         ([](widgetInfo& info) { RegisterTimeMovesWhenYouMove(); }) });
    // Saving Enhancements
    widgets = &sidebarEntries[ENHANCEMENTS_INDEX_GAMEPLAY].columnWidgets[2];
    widgets->push_back({ "Saving", NULL, NULL, WIDGET_SEPARATOR_TEXT });
    widgets->push_back({ "Persistent Owl Saves",
                         "gEnhancements.Saving.PersistentOwlSaves",
                         "Continuing a save will not remove the owl save. Playing Song of "
                         "Time, allowing the moon to crash or finishing the "
                         "game will remove the owl save and become the new last save.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Pause Menu Save",
                         "gEnhancements.Saving.PauseSave",
                         "Re-introduce the pause menu save system. Pressing B in the pause menu will give you the "
                         "option to create a persistent Owl Save from your current location.\n\nWhen loading back "
                         "into the game, you will be placed either at the entrance of the dungeon you saved in, or "
                         "in South Clock Town.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Autosave",
                         "gEnhancements.Saving.Autosave",
                         "Automatically create a persistent Owl Save on the chosen interval.\n\nWhen loading "
                         "back into the game, you will be placed either at the entrance of the dungeon you "
                         "saved in, or in South Clock Town.",
                         WIDGET_CHECKBOX,
                         {},
                         ([](widgetInfo& info) { RegisterAutosave(); }) });
    widgets->push_back({ "Autosave Interval: %d minutes",
                         "gEnhancements.Saving.AutosaveInterval",
                         "Sets the interval between Autosaves.",
                         WIDGET_SLIDER_INT,
                         { 1, 60, 5 },
                         nullptr,
                         [](widgetInfo& info) {
                             if (disabledMap[DISABLE_FOR_AUTO_SAVE_OFF].second.active) {
                                 info.activeDisables.push_back(DISABLE_FOR_AUTO_SAVE_OFF);
                             }
                         } });
    widgets->push_back({ "Time Cycle", NULL, NULL, WIDGET_SEPARATOR_TEXT });
    widgets->push_back({ "Do not reset Bottle content",
                         "gEnhancements.Cycle.DoNotResetBottleContent",
                         "Playing the Song Of Time will not reset the bottles' content.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Do not reset Consumables",
                         "gEnhancements.Cycle.DoNotResetConsumables",
                         "Playing the Song Of Time will not reset the consumables.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Do not reset Razor Sword",
                         "gEnhancements.Cycle.DoNotResetRazorSword",
                         "Playing the Song Of Time will not reset the Sword back to Kokiri Sword.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Do not reset Rupees",
                         "gEnhancements.Cycle.DoNotResetRupees",
                         "Playing the Song Of Time will not reset the your rupees.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Unstable", NULL, NULL, WIDGET_SEPARATOR_TEXT, { 0, 0, 0, {}, UIWidgets::Colors::Yellow } });
    widgets->push_back({ "Disable Save Delay",
                         "gEnhancements.Saving.DisableSaveDelay",
                         "Removes the arbitrary 2 second timer for saving from the original game. This is known to "
                         "cause issues when attempting the 0th Day Glitch",
                         WIDGET_CHECKBOX,
                         {} });
    // Graphics Enhancements
    widgets = &sidebarEntries[ENHANCEMENTS_INDEX_GRAPHICS].columnWidgets[0];
    widgets->push_back({ "Clock", NULL, NULL, WIDGET_SEPARATOR_TEXT });
    widgets->push_back({ "Clock Type",
                         "gEnhancements.Graphics.ClockType",
                         "Swaps between Graphical and Text only Clock types.",
                         WIDGET_COMBOBOX,
                         { 0, 0, 0, clockTypeOptions } });
    widgets->push_back({ "24 Hours Clock",
                         "gEnhancements.Graphics.24HoursClock",
                         "Changes from a 12 Hour to a 24 Hour Clock",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Motion Blur", NULL, NULL, WIDGET_SEPARATOR_TEXT });
    widgets->push_back({ "Motion Blur Mode",
                         "gEnhancements.Graphics.MotionBlur.Mode",
                         "Selects the Mode for Motion Blur.",
                         WIDGET_COMBOBOX,
                         { 0, 0, 0, motionBlurOptions },
                         [](widgetInfo& info) {
                             if (CVarGetInteger("gEnhancements.Graphics.MotionBlur.Mode", 0) == 1) {
                                 CVarSetInteger("gEnhancements.Graphics.MotionBlur.Toggle", 0);
                             }
                         } });
    widgets->push_back(
        { "Interpolate",
          "gEnhancements.Graphics.MotionBlur.Interpolate",
          "Change motion blur capture to also happen on interpolated frames instead of only on game frames.\n"
          "This notably reduces the overall motion blur strength but smooths out the trails.",
          WIDGET_CHECKBOX,
          {} });
    widgets->push_back({ "On/Off",
                         "gEnhancements.Graphics.MotionBlur.Toggle",
                         "Enables Motion Blur.",
                         WIDGET_CHECKBOX,
                         {},
                         [](widgetInfo& info) {
                             if (CVarGetInteger("gEnhancements.Graphics.MotionBlur.Toggle", 0) == 0) {
                                 R_MOTION_BLUR_ENABLED;
                                 R_MOTION_BLUR_ALPHA = CVarGetInteger("gEnhancements.Graphics.MotionBlur.Strength", 0);
                             } else {
                                 !R_MOTION_BLUR_ENABLED;
                             }
                         } });
    widgets->push_back({ "Strength",
                         "gEnhancements.Graphics.MotionBlur.Strength",
                         "Motion Blur strength.",
                         WIDGET_SLIDER_INT,
                         { 0, 255, 180 } });
    widgets->push_back({ "Other", NULL, NULL, WIDGET_SEPARATOR_TEXT });
    widgets->push_back({ "Authentic Logo",
                         "gEnhancements.Graphics.AuthenticLogo",
                         "Hide the game version and build details and display the authentic "
                         "model and texture on the boot logo start screen",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Bow Reticle",
                         "gEnhancements.Graphics.BowReticle",
                         "Gives the bow a reticle when you draw an arrow.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Disable Black Bar Letterboxes",
                         "gEnhancements.Graphics.DisableBlackBars",
                         "Disables Black Bar Letterboxes during cutscenes and Z-targeting\nNote: there may be "
                         "minor visual glitches that were covered up by the black bars\nPlease disable this "
                         "setting before reporting a bug.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Unstable", NULL, NULL, WIDGET_SEPARATOR_TEXT, { 0, 0, 0, {}, UIWidgets::Colors::Yellow } });
    widgets->push_back({ "Disable Scene Geometry Distance Check",
                         "gEnhancements.Graphics.DisableSceneGeometryDistanceCheck",
                         "Disables the distance check for scene geometry, allowing it to be drawn no matter how far "
                         "away it is from the player. This may have unintended side effects.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Increase Actor Draw Distance: %dx",
                         "gEnhancements.Graphics.IncreaseActorDrawDistance",
                         "Increase the range in which Actors are drawn. This may have unintended side effects.",
                         WIDGET_SLIDER_INT,
                         { 1, 5, 1 },
                         ([](widgetInfo& info) {
                             CVarSetInteger(
                                 "gEnhancements.Graphics.IncreaseActorUpdateDistance",
                                 MIN(CVarGetInteger("gEnhancements.Graphics.IncreaseActorDrawDistance", 1),
                                     CVarGetInteger("gEnhancements.Graphics.IncreaseActorUpdateDistance", 1)));
                         }) });
    widgets->push_back({ "Increase Actor Update Distance: %dx",
                         "gEnhancements.Graphics.IncreaseActorUpdateDistance",
                         "Increase the range in which Actors are updated. This may have unintended side effects.",
                         WIDGET_SLIDER_INT,
                         { 1, 5, 1 },
                         [](widgetInfo& info) {
                             CVarSetInteger(
                                 "gEnhancements.Graphics.IncreaseActorDrawDistance",
                                 MAX(CVarGetInteger("gEnhancements.Graphics.IncreaseActorDrawDistance", 1),
                                     CVarGetInteger("gEnhancements.Graphics.IncreaseActorUpdateDistance", 1)));
                         } });
    // Mask Enhancements
    widgets = &sidebarEntries[ENHANCEMENTS_INDEX_ITEMS_SONGS].columnWidgets[0];
    widgets->push_back({ "Masks", NULL, NULL, WIDGET_SEPARATOR_TEXT });
    widgets->push_back({ "Blast Mask has Powder Keg Force",
                         "gEnhancements.Masks.BlastMaskKeg",
                         "Blast Mask can also destroy objects only the Powder Keg can.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Fast Transformation",
                         "gEnhancements.Masks.FastTransformation",
                         "Removes the delay when using transormation masks.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Fierce Deity's Mask Anywhere",
                         "gEnhancements.Masks.FierceDeitysAnywhere",
                         "Allow using Fierce Deity's mask outside of boss rooms.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "No Blast Mask Cooldown",
                         "gEnhancements.Masks.NoBlastMaskCooldown",
                         "Eliminates the Cooldown between Blast Mask usage.",
                         WIDGET_CHECKBOX,
                         {} });
    // Song Enhancements
    widgets = &sidebarEntries[ENHANCEMENTS_INDEX_ITEMS_SONGS].columnWidgets[1];
    widgets->push_back({ "Ocarina", NULL, NULL, WIDGET_SEPARATOR_TEXT });
    widgets->push_back({ "Enable Sun's Song",
                         "gEnhancements.Songs.EnableSunsSong",
                         "Enables the partially implemented Sun's Song. RIGHT-DOWN-UP-RIGHT-DOWN-UP to play it. "
                         "This song will make time move very fast until either Link moves to a different scene, "
                         "or when the time switches to a new time period.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Dpad Ocarina",
                         "gEnhancements.Playback.DpadOcarina",
                         "Enables using the Dpad for Ocarina playback.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Prevent Dropped Ocarina Inputs",
                         "gEnhancements.Playback.NoDropOcarinaInput",
                         "Prevent dropping inputs when playing the ocarina quickly.",
                         WIDGET_CHECKBOX,
                         {} });
    // Cutscene Skips
    widgets = &sidebarEntries[ENHANCEMENTS_INDEX_TIME_SAVERS].columnWidgets[0];
    widgets->push_back({ "Cutscenes", NULL, NULL, WIDGET_SEPARATOR_TEXT });
    widgets->push_back({ "Hide Title Cards",
                         "gEnhancements.Cutscenes.HideTitleCards",
                         "Hides Title Cards when entering areas.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Skip Entrance Cutscenes",
                         "gEnhancements.Cutscenes.SkipEntranceCutscenes",
                         "Skip cutscenes that occur when first entering a new area.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Skip to File Select",
                         "gEnhancements.Cutscenes.SkipToFileSelect",
                         "Skip the opening title sequence and go straight to the file select menu after boot.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Skip Intro Sequence",
                         "gEnhancements.Cutscenes.SkipIntroSequence",
                         "When starting a game you will be taken straight to South Clock Town as Deku Link.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Skip Story Cutscenes",
                         "gEnhancements.Cutscenes.SkipStoryCutscenes",
                         "Disclaimer: This doesn't do much yet, we will be progressively adding more skips over time.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Skip Misc Interactions",
                         "gEnhancements.Cutscenes.SkipMiscInteractions",
                         "Disclaimer: This doesn't do much yet, we will be progressively adding more skips over time.",
                         WIDGET_CHECKBOX,
                         {} });
    // Dialogue Enhancements
    widgets = &sidebarEntries[ENHANCEMENTS_INDEX_TIME_SAVERS].columnWidgets[1];
    widgets->push_back({ "Dialogue", NULL, NULL, WIDGET_SEPARATOR_TEXT });
    widgets->push_back({ "Fast Bank Selection",
                         "gEnhancements.Dialogue.FastBankSelection",
                         "Pressing the Z or R buttons while the Deposit/Withdrawl Rupees dialogue is open will set "
                         "the Rupees to Links current Rupees or 0 respectively.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Fast Text",
                         "gEnhancements.Dialogue.FastText",
                         "Speeds up text rendering, and enables holding of B progress to next message.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Fast Magic Arrow Equip Animation",
                         "gEnhancements.Equipment.MagicArrowEquipSpeed",
                         "Removes the animation for equipping Magic Arrows.",
                         WIDGET_CHECKBOX,
                         {} });
    // Fixes
    widgets = &sidebarEntries[ENHANCEMENTS_INDEX_FIXES].columnWidgets[0];
    widgets->push_back({ "Fixes", NULL, NULL, WIDGET_SEPARATOR_TEXT });
    widgets->push_back({ "Fix Ammo Count Color",
                         "gFixes.FixAmmoCountEnvColor",
                         "Fixes a missing gDPSetEnvColor, which causes the ammo count to be "
                         "the wrong color prior to obtaining magic or other conditions.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Fix Hess and Weirdshot Crash",
                         "gEnhancements.Fixes.HessCrash",
                         "Fixes a crash that can occur when performing a HESS or Weirdshot.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Fix Text Control Characters",
                         "gEnhancements.Fixes.ControlCharacters",
                         "Fixes certain control characters not functioning properly "
                         "depending on their position within the text.",
                         WIDGET_CHECKBOX,
                         {} });
    // Restorations
    widgets = &sidebarEntries[ENHANCEMENTS_INDEX_RESTORATIONS].columnWidgets[0];
    widgets->push_back({ "Restorations", NULL, NULL, WIDGET_SEPARATOR_TEXT });
    widgets->push_back({ "Constant Distance Backflips and Sidehops",
                         "gEnhancements.Restorations.ConstantFlipsHops",
                         "Backflips and Sidehops travel a constant distance as they did in OoT.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Power Crouch Stab",
                         "gEnhancements.Restorations.PowerCrouchStab",
                         "Crouch stabs will use the power of Link's previous melee attack, as is in MM JP 1.0 and OoT.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Side Rolls",
                         "gEnhancements.Restorations.SideRoll",
                         "Restores side rolling from OoT.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Tatl ISG",
                         "gEnhancements.Restorations.TatlISG",
                         "Restores Navi ISG from OoT, but now with Tatl.",
                         WIDGET_CHECKBOX,
                         {} });
    // HUD Editor
    sidebarEntries[ENHANCEMENTS_INDEX_HUD_EDITOR].columnWidgets[0].push_back(
        { "Popout HUD Editor",
          "gWindows.HudEditor",
          "Enables the HUD Editor window, allowing you to modify your HUD",
          WIDGET_WINDOW_BUTTON,
          { .windowName = "HUD Editor" } });
}

void AddDevTools() {
    std::vector<widgetInfo>* widgets = &sidebarEntries[DEV_TOOLS_INDEX_GENERAL].columnWidgets[0];
    widgets->push_back({ "Popout Menu",
                         "gSettings.Menu.Popout",
                         "Changes the menu display from overlay to windowed.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Open App Files Folder",
                         "",
                         "Opens the folder that contains the save and mods folders, etc,",
                         WIDGET_BUTTON,
                         {},
                         [](widgetInfo& info) {
                             std::string filesPath = Ship::Context::GetInstance()->GetAppDirectoryPath();
                             SDL_OpenURL(
                                 std::string("file:///" + std::filesystem::absolute(filesPath).string()).c_str());
                         } });
    widgets->push_back({ "Debug Mode",
                         "gDeveloperTools.DebugEnabled",
                         "Enables Debug Mode, allowing you to select maps with L + R + Z.",
                         WIDGET_CHECKBOX,
                         {},
                         [](widgetInfo& info) {
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
                         } });
    widgets->push_back({ "Better Map Select",
                         "gDeveloperTools.BetterMapSelect.Enabled",
                         "Overrides the original map select with a translated, more user-friendly version.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Debug Save File Mode",
                         "gDeveloperTools.DebugSaveFileMode",
                         "Change the behavior of creating saves while debug mode is enabled:\n\n"
                         "- Empty Save: The default 3 heart save file in first cycle\n"
                         "- Vanilla Debug Save: Uses the title screen save info (8 hearts, all items and masks)\n"
                         "- 100\% Save: All items, equipment, mask, quast status and bombers notebook complete",
                         WIDGET_COMBOBOX,
                         { 0, 0, 0, debugSaveOptions },
                         [](widgetInfo& info) { RegisterDebugSaveCreate(); } });
    widgets->push_back({ "Prevent Actor Update",
                         "gDeveloperTools.PreventActorUpdate",
                         "Prevents Actors from updating.",
                         WIDGET_CHECKBOX,
                         {},
                         [](widgetInfo& info) { RegisterPreventActorUpdateHooks(); } });
    widgets->push_back({ "Prevent Actor Draw",
                         "gDeveloperTools.PreventActorDraw",
                         "Prevents Actors from drawing.",
                         WIDGET_CHECKBOX,
                         {},
                         [](widgetInfo& info) { RegisterPreventActorDrawHooks(); } });
    widgets->push_back({ "Prevent Actor Init",
                         "gDeveloperTools.PreventActorInit",
                         "Prevents Actors from initializing.",
                         WIDGET_CHECKBOX,
                         {},
                         [](widgetInfo& info) { RegisterPreventActorInitHooks(); } });
    widgets->push_back({ "Disable Object Dependency",
                         "gDeveloperTools.DisableObjectDependency",
                         "Disables dependencies when loading objects.",
                         WIDGET_CHECKBOX,
                         {} });
    widgets->push_back({ "Log Level",
                         "gDeveloperTools.LogLevel",
                         "The log level determines which messages are printed to the "
                         "console. This does not affect the log file output",
                         WIDGET_COMBOBOX,
                         { 0, 0, 0, logLevels },
                         [](widgetInfo& info) {
                             Ship::Context::GetInstance()->GetLogger()->set_level(
                                 (spdlog::level::level_enum)CVarGetInteger("gDeveloperTools.LogLevel", 1));
                         } });
    widgets->push_back({ "Frame Advance",
                         "gDeveloperTools.FrameAdvance",
                         "This allows you to advance through the game one frame at a time on command. "
                         "To advance a frame, hold Z and tap R on the second controller. Holding Z "
                         "and R will advance a frame every half second. You can also use the buttons below.",
                         WIDGET_CHECKBOX,
                         {},
                         [](widgetInfo& info) {
                             if (CVarGetInteger("gDeveloperTools.FrameAdvance", 0) == 1) {
                                 gPlayState->frameAdvCtx.enabled = true;
                             } else {
                                 gPlayState->frameAdvCtx.enabled = false;
                             };
                         } });
    widgets->push_back({ "Advance 1", "", "Advance 1 frame.", WIDGET_BUTTON, {}, [](widgetInfo& info) {
                            CVarSetInteger("gDeveloperTools.FrameAdvanceTick", 1);
                        } });
    widgets->push_back({ "Advance (Hold)", "", "Advance frames while the button is held.", WIDGET_BUTTON });
    // dev tools windows
    sidebarEntries[DEV_TOOLS_INDEX_COLLISION].columnWidgets[0].push_back({ "Popout Collision Viewer",
                                                                           "gWindows.CollisionViewer",
                                                                           "Makes collision visible on screen",
                                                                           WIDGET_WINDOW_BUTTON,
                                                                           { .windowName = "Collision Viewer" } });
    sidebarEntries[DEV_TOOLS_INDEX_STATS].columnWidgets[0].push_back(
        { "Popout Stats",
          "gOpenWindows.Stats",
          "Shows the stats window, with your FPS and frametimes, and the OS you're playing on",
          WIDGET_WINDOW_BUTTON,
          { .windowName = "Stats" } });
    sidebarEntries[DEV_TOOLS_INDEX_CONSOLE].columnWidgets[0].push_back(
        { "Popout Console",
          "gOpenWindows.Console",
          "Enables the console window, allowing you to input commands. Type help for some examples",
          WIDGET_WINDOW_BUTTON,
          { .windowName = "Console" } });
    sidebarEntries[DEV_TOOLS_INDEX_GFX_DEBUGGER].columnWidgets[0].push_back(
        { "Popout Gfx Debugger",
          "gOpenWindows.GfxDebugger",
          "Enables the Gfx Debugger window, allowing you to input commands, type help for some examples",
          WIDGET_WINDOW_BUTTON,
          { .windowName = "GfxDebuggerWindow" } });
    sidebarEntries[DEV_TOOLS_INDEX_SAVE_EDITOR].columnWidgets[0].push_back(
        { "Popout Save Editor",
          "gWindows.SaveEditor",
          "Enables the Save Editor window, allowing you to edit your save file",
          WIDGET_WINDOW_BUTTON,
          { .windowName = "Save Editor" } });
    sidebarEntries[DEV_TOOLS_INDEX_ACTOR_VIEWER].columnWidgets[0].push_back(
        { "Popout Actor Viewer",
          "gWindows.ActorViewer",
          "Enables the Actor Viewer window, allowing you to view actors in the world.",
          WIDGET_WINDOW_BUTTON,
          { .windowName = "Actor Viewer" } });
    sidebarEntries[DEV_TOOLS_INDEX_EVENT_LOG].columnWidgets[0].push_back({ "Popout Event Log",
                                                                           "gWindows.EventLog",
                                                                           "Enables the event log window",
                                                                           WIDGET_WINDOW_BUTTON,
                                                                           { .windowName = "Event Log" } });
}

void SearchMenuGetItem(widgetInfo& widget) {
    disabledTempTooltip = "This setting is disabled because: \n\n";
    disabledValue = false;
    disabledTooltip = " ";

    if (widget.modifierFunc != nullptr) {
        widget.activeDisables.clear();
        widget.isHidden = false;
        widget.modifierFunc(widget);
        if (widget.isHidden) {
            return;
        }
        if (!widget.activeDisables.empty()) {
            disabledValue = true;
            for (auto option : widget.activeDisables) {
                disabledTempTooltip += std::string("- ") + disabledMap[option].second.reason + std::string("\n");
            }
            disabledTooltip = disabledTempTooltip.c_str();
        }
    }

    switch (widget.widgetType) {
        case WIDGET_CHECKBOX:
            if (UIWidgets::CVarCheckbox(
                    widget.widgetName, widget.widgetCVar,
                    {
                        .color = menuTheme[menuThemeIndex],
                        .tooltip = widget.widgetTooltip,
                        .disabled = disabledValue,
                        .disabledTooltip = disabledTooltip,
                        .defaultValue = static_cast<bool>(std::get<int32_t>(widget.widgetOptions.defaultVariant)),
                    })) {
                if (widget.widgetCallback != nullptr) {
                    widget.widgetCallback(widget);
                }
            };
            break;
        case WIDGET_AUDIO_BACKEND: {
            auto currentAudioBackend = Ship::Context::GetInstance()->GetAudio()->GetAudioBackend();
            if (UIWidgets::Combobox(
                    "Audio API", &currentAudioBackend, audioBackendsMap,
                    { .color = menuTheme[menuThemeIndex],
                      .tooltip = widget.widgetTooltip,
                      .disabled = Ship::Context::GetInstance()->GetAudio()->GetAvailableAudioBackends()->size() <= 1,
                      .disabledTooltip = "Only one audio API is available on this platform." })) {
                Ship::Context::GetInstance()->GetAudio()->SetAudioBackend(currentAudioBackend);
            }
        } break;
        case WIDGET_VIDEO_BACKEND: {
            if (UIWidgets::Combobox("Renderer API (Needs reload)", &configWindowBackend, availableWindowBackendsMap,
                                    { .color = menuTheme[menuThemeIndex],
                                      .tooltip = widget.widgetTooltip,
                                      .disabled = availableWindowBackends->size() <= 1,
                                      .disabledTooltip = "Only one renderer API is available on this platform." })) {
                Ship::Context::GetInstance()->GetConfig()->SetInt("Window.Backend.Id",
                                                                  static_cast<int32_t>(configWindowBackend));
                Ship::Context::GetInstance()->GetConfig()->SetString("Window.Backend.Name",
                                                                     windowBackendsMap.at(configWindowBackend));
                Ship::Context::GetInstance()->GetConfig()->Save();
                UpdateWindowBackendObjects();
            }
        } break;
        case WIDGET_SEPARATOR:
            ImGui::Separator();
            break;
        case WIDGET_SEPARATOR_TEXT:
            if (widget.widgetOptions.color != COLOR_NONE) {
                ImGui::PushStyleColor(ImGuiCol_Text, widget.widgetOptions.color);
            }
            ImGui::SeparatorText(widget.widgetName);
            if (widget.widgetOptions.color != COLOR_NONE) {
                ImGui::PopStyleColor();
            }
            break;
        case WIDGET_COMBOBOX:
            if (UIWidgets::CVarCombobox(
                    widget.widgetName, widget.widgetCVar, widget.widgetOptions.comboBoxOptions,
                    {
                        .color = menuTheme[menuThemeIndex],
                        .tooltip = widget.widgetTooltip,
                        .disabled = disabledValue,
                        .disabledTooltip = disabledTooltip,
                        .defaultIndex = static_cast<uint32_t>(std::get<int32_t>(widget.widgetOptions.defaultVariant)),
                    })) {
                if (widget.widgetCallback != nullptr) {
                    widget.widgetCallback(widget);
                }
            }
            break;
        case WIDGET_SLIDER_INT:
            if (UIWidgets::CVarSliderInt(
                    widget.widgetName, widget.widgetCVar, std::get<int32_t>(widget.widgetOptions.min),
                    std::get<int32_t>(widget.widgetOptions.max), std::get<int32_t>(widget.widgetOptions.defaultVariant),
                    {
                        .color = menuTheme[menuThemeIndex],
                        .tooltip = widget.widgetTooltip,
                        .disabled = disabledValue,
                        .disabledTooltip = disabledTooltip,
                    })) {
                if (widget.widgetCallback != nullptr) {
                    widget.widgetCallback(widget);
                }
            };
            break;
        case WIDGET_SLIDER_FLOAT: {
            float floatMin = (std::get<float>(widget.widgetOptions.min) / 100);
            float floatMax = (std::get<float>(widget.widgetOptions.max) / 100);
            float floatDefault = (std::get<float>(widget.widgetOptions.defaultVariant) / 100);
            if (UIWidgets::CVarSliderFloat(widget.widgetName, widget.widgetCVar, floatMin, floatMax, floatDefault,
                                           {
                                               .color = menuTheme[menuThemeIndex],
                                               .tooltip = widget.widgetTooltip,
                                               .disabled = disabledValue,
                                               .disabledTooltip = disabledTooltip,
                                           })) {
                if (widget.widgetCallback != nullptr) {
                    widget.widgetCallback(widget);
                }
            }
        } break;
        case WIDGET_BUTTON:
            if (UIWidgets::Button(widget.widgetName, {
                                                         .color = menuTheme[menuThemeIndex],
                                                         .tooltip = widget.widgetTooltip,
                                                         .disabled = disabledValue,
                                                         .disabledTooltip = disabledTooltip,
                                                     })) {
                if (widget.widgetCallback != nullptr) {
                    widget.widgetCallback(widget);
                }
            }
            break;
        case WIDGET_WINDOW_BUTTON: {
            if (widget.widgetOptions.windowName == "") {
                std::string msg =
                    fmt::format("Error drawing window contents for {}: windowName not defined", widget.widgetName);
                SPDLOG_ERROR(msg.c_str());
                break;
            }
            auto window =
                Ship::Context::GetInstance()->GetWindow()->GetGui()->GetGuiWindow(widget.widgetOptions.windowName);
            if (!window) {
                std::string msg = fmt::format("Error drawing window contents: windowName {} does not exist",
                                              widget.widgetOptions.windowName);
                SPDLOG_ERROR(msg.c_str());
                break;
            }
            UIWidgets::WindowButton(widget.widgetName, widget.widgetCVar, window, { .tooltip = widget.widgetTooltip });
            if (!window->IsVisible()) {
                window->DrawElement();
            }
        } break;
        case WIDGET_SEARCH: {
            if (ImGui::Button("Clear")) {
                menuSearch.Clear();
            }
            ImGui::SameLine();
            menuSearch.Draw();
            std::string str(menuSearch.InputBuf);

            if (str == "") {
                ImGui::Text("Start typing to see results.");
                return;
            }
            ImGui::BeginChild("Search Results");
            for (auto& [menuLabel, entries, cvar] : menuEntries) {
                for (auto& index : entries) {
                    for (int i = 0; i < sidebarEntries[index].columnWidgets.size(); i++) {
                        for (auto& widget : sidebarEntries[index].columnWidgets[i]) {
                            if (widget.widgetType == WIDGET_SEARCH || widget.widgetType == WIDGET_SEPARATOR ||
                                widget.widgetType == WIDGET_SEPARATOR_TEXT || widget.isHidden) {
                                continue;
                            }
                            std::string ctr(widget.widgetName);
                            std::transform(str.begin(), str.end(), str.begin(), ::tolower);
                            str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
                            std::transform(ctr.begin(), ctr.end(), ctr.begin(), ::tolower);
                            ctr.erase(std::remove(ctr.begin(), ctr.end(), ' '), ctr.end());
                            if (ctr.find(str) != std::string::npos) {
                                SearchMenuGetItem(widget);
                                ImGui::PushStyleColor(ImGuiCol_Text, UIWidgets::Colors::Gray);
                                std::string origin =
                                    fmt::format("  ({} -> {}, Clmn {})", menuLabel, sidebarEntries[index].label, i + 1);
                                ImGui::Text(origin.c_str());
                                ImGui::PopStyleColor();
                            }
                        }
                    }
                }
            }
            ImGui::EndChild();
        } break;
        default:
            break;
    }
}
} // namespace BenGui
