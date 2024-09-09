#include "2s2h/Enhancements/Enhancements.h"
#include "2s2h/DeveloperTools/DeveloperTools.h"
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
    ImVec4 color = COLOR_NONE;
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
    uint32_t columnCount;
    std::vector<std::vector<widgetInfo>> columnWidgets;
};

struct MainMenuEntry {
    std::string label;
    std::vector<SidebarEntry> sidebarEntries;
    const char* sidebarCvar;
};

namespace BenGui {
extern std::shared_ptr<std::vector<Ship::WindowBackend>> availableWindowBackends;
extern std::unordered_map<Ship::WindowBackend, const char*> availableWindowBackendsMap;
extern Ship::WindowBackend configWindowBackend;
extern void UpdateWindowBackendObjects();

std::vector<MainMenuEntry> menuEntries;
static ImGuiTextFilter menuSearch;
std::vector<SidebarEntry> settingsSidebar;
std::vector<SidebarEntry> enhancementsSidebar;
std::vector<SidebarEntry> devToolsSidebar;

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

void AddSettings() {
    // General
    settingsSidebar.push_back(
        { "Search",
          1,
          { { { "Menu Theme", "", "Searches all menus for the given text, including tooltips.", WIDGET_SEARCH } } } });
    // General Settings
    settingsSidebar.push_back(
        { "General",
          3,
          { { { "Menu Theme",
                "gSettings.MenuTheme",
                "Changes the Theme of the Menu Widgets.",
                WIDGET_COMBOBOX,
                { .comboBoxOptions = menuThemeOptions } },
#if not defined(__SWITCH__) and not defined(__WIIU__)
              { "Menubar Controller Navigation", CVAR_IMGUI_CONTROLLER_NAV,
                "Allows controller navigation of the SOH menu bar (Settings, Enhancements,...)\nCAUTION: "
                "This will disable game inputs while the menubar is visible.\n\nD-pad to move between "
                "items, A to select, and X to grab focus on the menu bar",
                WIDGET_CHECKBOX },
              { "Cursor Always Visible",
                "gSettings.CursorVisibility",
                "Makes the cursor always visible, even in full screen.",
                WIDGET_CHECKBOX,
                {},
                [](widgetInfo& info) {
                    Ship::Context::GetInstance()->GetWindow()->SetForceCursorVisibility(
                        CVarGetInteger("gSettings.CursorVisibility", 0));
                } },
#endif
              { "Hide Menu Hotkey Text", "gSettings.DisableMenuShortcutNotify",
                "Prevents showing the text telling you the shortcut to open the menu\n"
                "when the menu isn't open.",
                WIDGET_CHECKBOX } } } });
    // Audio Settings
    settingsSidebar.push_back(
        { "Audio",
          3,
          { { { "Master Volume: %.2f%%",
                "gSettings.Audio.MasterVolume",
                "Adjust overall sound volume.",
                WIDGET_SLIDER_FLOAT,
                { 0.0f, 100.0f, 100.0f } },
              { "Main Music Volume: %.2f%%",
                "gSettings.Audio.MainMusicVolume",
                "Adjust the Background Music volume.",
                WIDGET_SLIDER_FLOAT,
                { 0.0f, 100.0f, 100.0f },
                [](widgetInfo& info) {
                    AudioSeq_SetPortVolumeScale(SEQ_PLAYER_BGM_MAIN,
                                                CVarGetFloat("gSettings.Audio.MainMusicVolume", 1.0f));
                } },
              { "Sub Music Volume: %.2f%%",
                "gSettings.Audio.SubMusicVolume",
                "Adjust the Sub Music volume.",
                WIDGET_SLIDER_FLOAT,
                { 0.0f, 100.0f, 100.0f },
                [](widgetInfo& info) {
                    AudioSeq_SetPortVolumeScale(SEQ_PLAYER_BGM_SUB,
                                                CVarGetFloat("gSettings.Audio.SubMusicVolume", 1.0f));
                } },
              { "Sound Effects Volume: %.2f%%",
                "gSettings.Audio.SoundEffectsVolume",
                "Adjust the Sound Effects volume.",
                WIDGET_SLIDER_FLOAT,
                { 0.0f, 100.0f, 100.0f },
                [](widgetInfo& info) {
                    AudioSeq_SetPortVolumeScale(SEQ_PLAYER_SFX,
                                                CVarGetFloat("gSettings.Audio.SoundEffectsVolume", 1.0f));
                } },
              { "Fanfare Volume: %.2f%%",
                "gSettings.Audio.FanfareVolume",
                "Adjust the Fanfare volume.",
                WIDGET_SLIDER_FLOAT,
                { 0.0f, 100.0f, 100.0f },
                [](widgetInfo& info) {
                    AudioSeq_SetPortVolumeScale(SEQ_PLAYER_FANFARE,
                                                CVarGetFloat("gSettings.Audio.FanfareVolume", 1.0f));
                } },
              { "Ambience Volume: %.2f%%",
                "gSettings.Audio.AmbienceVolume",
                "Adjust the Ambient Sound volume.",
                WIDGET_SLIDER_FLOAT,
                { 0.0f, 100.0f, 100.0f },
                [](widgetInfo& info) {
                    AudioSeq_SetPortVolumeScale(SEQ_PLAYER_AMBIENCE,
                                                CVarGetFloat("gSettings.Audio.AmbienceVolume", 1.0f));
                } },
              { "Audio API", NULL, "Sets the audio API used by the game. Requires a relaunch to take effect.",
                WIDGET_AUDIO_BACKEND } } } });
    // Graphics Settings
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
    settingsSidebar.push_back(
        { "Graphics",
          3,
          { { { "Toggle Fullscreen",
                "gSettings.Fullscreen",
                "Toggles Fullscreen On/Off.",
                WIDGET_CHECKBOX,
                {},
                [](widgetInfo& info) { Ship::Context::GetInstance()->GetWindow()->ToggleFullscreen(); } },
#ifndef __APPLE__
              { "Internal Resolution: %f%%",
                CVAR_INTERNAL_RESOLUTION,
                "Multiplies your output resolution by the value inputted, as a more intensive but effective "
                "form of anti-aliasing.",
                WIDGET_SLIDER_FLOAT,
                { 50.0f, 200.0f, 100.0f },
                [](widgetInfo& info) {
                    Ship::Context::GetInstance()->GetWindow()->SetResolutionMultiplier(
                        CVarGetFloat(CVAR_INTERNAL_RESOLUTION, 1));
                } },
#endif
#ifndef __WIIU__
              { "Anti-aliasing (MSAA): %d",
                CVAR_MSAA_VALUE,
                "Activates MSAA (multi-sample anti-aliasing) from 2x up to 8x, to smooth the edges of rendered "
                "geometry.\n"
                "Higher sample count will result in smoother edges on models, but may reduce performance.",
                WIDGET_SLIDER_INT,
                { 1, 8, 1 },
                [](widgetInfo& info) {
                    Ship::Context::GetInstance()->GetWindow()->SetMsaaLevel(CVarGetInteger(CVAR_MSAA_VALUE, 1));
                } },
#endif

              { "Current FPS: %d",
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
                } },
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
                } },
              { "Match Refresh Rate",
                "gMatchRefreshRate",
                "Matches interpolation value to the current game's window refresh rate.",
                WIDGET_CHECKBOX,
                {},
                nullptr,
                [](widgetInfo& info) {
                    if (!disabledMap[DISABLE_FOR_NOT_DIRECTX].second.active)
                        info.isHidden = true;
                } },
              { "Jitter fix : >= % d FPS",
                "gExtraLatencyThreshold",
                "When Interpolation FPS setting is at least this threshold, add one frame of input "
                "lag (e.g. 16.6 ms for 60 FPS) in order to avoid jitter. This setting allows the "
                "CPU to work on one frame while GPU works on the previous frame.\nThis setting "
                "should be used when your computer is too slow to do CPU + GPU work in time.",
                WIDGET_SLIDER_INT,
                { 0, 360, 80 },
                nullptr,
                [](widgetInfo& info) { info.isHidden = disabledMap[DISABLE_FOR_NOT_DIRECTX].second.active; } },
              { "Renderer API (Needs reload)", NULL, "Sets the renderer API used by the game.", WIDGET_VIDEO_BACKEND },
              { "Enable Vsync",
                CVAR_VSYNC_ENABLED,
                "Enables Vsync.",
                WIDGET_CHECKBOX,
                {},
                nullptr,
                [](widgetInfo& info) { info.isHidden = disabledMap[DISABLE_FOR_NO_VSYNC].second.active; } },
              { "Windowed Fullscreen",
                CVAR_SDL_WINDOWED_FULLSCREEN,
                "Enables Windowed Fullscreen Mode.",
                WIDGET_CHECKBOX,
                {},
                nullptr,
                [](widgetInfo& info) {
                    info.isHidden = disabledMap[DISABLE_FOR_NO_WINDOWED_FULLSCREEN].second.active;
                } },
              { "Allow multi-windows",
                CVAR_ENABLE_MULTI_VIEWPORTS,
                "Allows multiple windows to be opened at once. Requires a reload to take effect.",
                WIDGET_CHECKBOX,
                {},
                nullptr,
                [](widgetInfo& info) { info.isHidden = disabledMap[DISABLE_FOR_NO_MULTI_VIEWPORT].second.active; } },
              { "Texture Filter (Needs reload)",
                CVAR_TEXTURE_FILTER,
                "Sets the applied Texture Filtering.",
                WIDGET_COMBOBOX,
                { .comboBoxOptions = textureFilteringMap } } } } });
    // Input Editor
    settingsSidebar.push_back({ "Input Editor",
                                1,
                                { { { "Popout Input Editor",
                                      "gWindows.BenInputEditor",
                                      "Enables the separate Input Editor window.",
                                      WIDGET_WINDOW_BUTTON,
                                      { .windowName = "2S2H Input Editor" } } } } });
}

void AddEnhancements() {
    // Camera Snap Fix
    enhancementsSidebar.push_back(
        { "Camera",
          3,
          { { { .widgetName = "Fixes", .widgetType = WIDGET_SEPARATOR_TEXT },
              { "Fix Targetting Camera Snap",
                "gEnhancements.Camera.FixTargettingCameraSnap",
                "Fixes the camera snap that occurs when you are moving and press the targetting button.",
                WIDGET_CHECKBOX,
                {} } },
            // Camera Enhancements
            { { .widgetName = "Cameras", .widgetType = WIDGET_SEPARATOR_TEXT },
              { "Free Look",
                "gEnhancements.Camera.FreeLook.Enable",
                "Enables free look camera control\nNote: You must remap C buttons off of the right "
                "stick in the controller config menu, and map the camera stick to the right stick.",
                WIDGET_CHECKBOX,
                {},
                [](widgetInfo& info) { RegisterCameraFreeLook(); },
                [](widgetInfo& info) {
                    if (disabledMap[DISABLE_FOR_DEBUG_CAM_ON].second.active)
                        info.activeDisables.push_back(DISABLE_FOR_DEBUG_CAM_ON);
                } },
              { "Camera Distance: %d",
                "gEnhancements.Camera.FreeLook.MaxCameraDistance",
                "Maximum Camera Distance for Free Look.",
                WIDGET_SLIDER_INT,
                { 100, 900, 185 },
                nullptr,
                [](widgetInfo& info) { info.isHidden = disabledMap[DISABLE_FOR_FREE_LOOK_OFF].second.active; } },
              { "Camera Transition Speed: %d",
                "gEnhancements.Camera.FreeLook.TransitionSpeed",
                "Can someone help me?",
                WIDGET_SLIDER_INT,
                { 1, 900, 25 },
                nullptr,
                [](widgetInfo& info) { info.isHidden = disabledMap[DISABLE_FOR_FREE_LOOK_OFF].second.active; } },
              { "Max Camera Height Angle: %.0f\xC2\xB0",
                "gEnhancements.Camera.FreeLook.MaxPitch",
                "Maximum Height of the Camera.",
                WIDGET_SLIDER_FLOAT,
                { -8900.0f, 8900.0f, 7200.0f },
                [](widgetInfo& info) { FreeLookPitchMinMax(); },
                [](widgetInfo& info) { info.isHidden = disabledMap[DISABLE_FOR_FREE_LOOK_OFF].second.active; } },
              { "Min Camera Height Angle: %.0f\xC2\xB0",
                "gEnhancements.Camera.FreeLook.MinPitch",
                "Minimum Height of the Camera.",
                WIDGET_SLIDER_FLOAT,
                { -8900.0f, 8900.0f, -4900.0f },
                [](widgetInfo& info) { FreeLookPitchMinMax(); },
                [](widgetInfo& info) { info.isHidden = disabledMap[DISABLE_FOR_FREE_LOOK_OFF].second.active; } },
              { "Debug Camera",
                "gEnhancements.Camera.DebugCam.Enable",
                "Enables free camera control.",
                WIDGET_CHECKBOX,
                {},
                ([](widgetInfo& info) { RegisterDebugCam(); }),
                [](widgetInfo& info) {
                    if (disabledMap[DISABLE_FOR_FREE_LOOK_ON].second.active) {
                        info.activeDisables.push_back(DISABLE_FOR_FREE_LOOK_ON);
                    }
                } },
              { "Invert Camera X Axis",
                "gEnhancements.Camera.RightStick.InvertXAxis",
                "Inverts the Camera X Axis",
                WIDGET_CHECKBOX,
                {},
                nullptr,
                [](widgetInfo& info) {
                    if (disabledMap[DISABLE_FOR_CAMERAS_OFF].second.active) {
                        info.activeDisables.push_back(DISABLE_FOR_CAMERAS_OFF);
                    }
                } },
              { "Invert Camera Y Axis",
                "gEnhancements.Camera.RightStick.InvertYAxis",
                "Inverts the Camera Y Axis",
                WIDGET_CHECKBOX,
                {},
                nullptr,
                [](widgetInfo& info) {
                    if (disabledMap[DISABLE_FOR_CAMERAS_OFF].second.active) {
                        info.activeDisables.push_back(DISABLE_FOR_CAMERAS_OFF);
                    }
                } },
              { "Third-Person Horizontal Sensitivity: %.0f",
                "gEnhancements.Camera.RightStick.CameraSensitivity.X",
                "Adjust the Sensitivity of the x axis when in Third Person.",
                WIDGET_SLIDER_FLOAT,
                { 1.0f, 500.0f, 100.0f },
                nullptr,
                [](widgetInfo& info) {
                    if (disabledMap[DISABLE_FOR_CAMERAS_OFF].second.active) {
                        info.activeDisables.push_back(DISABLE_FOR_CAMERAS_OFF);
                    }
                } },
              { "Third-Person Vertical Sensitivity: %.0f",
                "gEnhancements.Camera.RightStick.CameraSensitivity.Y",
                "Adjust the Sensitivity of the x axis when in Third Person.",
                WIDGET_SLIDER_FLOAT,
                { 1.0f, 500.0f, 100.0f },
                nullptr,
                [](widgetInfo& info) {
                    if (disabledMap[DISABLE_FOR_CAMERAS_OFF].second.active) {
                        info.activeDisables.push_back(DISABLE_FOR_CAMERAS_OFF);
                    }
                } },
              { "Enable Roll (6\xC2\xB0 of Freedom)",
                "gEnhancements.Camera.DebugCam.6DOF",
                "This allows for all six degrees of movement with the camera, NOTE: Yaw will work "
                "differently in this system, instead rotating around the focal point"
                ", rather than a polar axis.",
                WIDGET_CHECKBOX,
                {},
                nullptr,
                [](widgetInfo& info) { info.isHidden = disabledMap[DISABLE_FOR_DEBUG_CAM_OFF].second.active; } },
              { "Camera Speed: %.0f",
                "gEnhancements.Camera.DebugCam.CameraSpeed",
                "Adjusts the speed of the Camera.",
                WIDGET_SLIDER_FLOAT,
                { 10.0f, 300.0f, 50.0f },
                nullptr,
                [](widgetInfo& info) { info.isHidden = disabledMap[DISABLE_FOR_DEBUG_CAM_OFF].second.active; } } } } });
    // Cheats
    enhancementsSidebar.push_back(
        { "Cheats",
          3,
          { { { "Infinite Health", "gCheats.InfiniteHealth", "Always have full Hearts.", WIDGET_CHECKBOX, {} },
              { "Infinite Magic", "gCheats.InfiniteMagic", "Always have full Magic.", WIDGET_CHECKBOX, {} },
              { "Infinite Rupees", "gCheats.InfiniteRupees", "Always have a full Wallet.", WIDGET_CHECKBOX, {} },
              { "Infinite Consumables", "gCheats.InfiniteConsumables",
                "Always have max Consumables, you must have collected the consumables first.", WIDGET_CHECKBOX },
              { "Longer Deku Flower Glide",
                "gCheats.LongerFlowerGlide",
                "Allows Deku Link to glide longer, no longer dropping after a certain distance.",
                WIDGET_CHECKBOX,
                {},
                [](widgetInfo& info) { RegisterLongerFlowerGlide(); } },
              { "No Clip", "gCheats.NoClip", "Allows Link to phase through collision.", WIDGET_CHECKBOX },
              { "Unbreakable Razor Sword", "gCheats.UnbreakableRazorSword",
                "Allows to Razor Sword to be used indefinitely without dulling its blade.", WIDGET_CHECKBOX },
              { "Unrestricted Items", "gCheats.UnrestrictedItems", "Allows all Forms to use all Items.",
                WIDGET_CHECKBOX },
              { "Moon Jump on L",
                "gCheats.MoonJumpOnL",
                "Holding L makes you float into the air.",
                WIDGET_CHECKBOX,
                {},
                [](widgetInfo& info) { RegisterMoonJumpOnL(); } } } } });
    // Gameplay Enhancements
    enhancementsSidebar.push_back(
        { "Gameplay",
          3,
          { { { .widgetName = "Player", .widgetType = WIDGET_SEPARATOR_TEXT },
              { "Fast Deku Flower Launch",
                "gEnhancements.Player.FastFlowerLaunch",
                "Speeds up the time it takes to be able to get maximum height from launching out of a deku flower",
                WIDGET_CHECKBOX,
                {},
                ([](widgetInfo& info) { RegisterFastFlowerLaunch(); }) },
              { "Instant Putaway", "gEnhancements.Player.InstantPutaway",
                "Allows Link to instantly puts away held item without waiting.", WIDGET_CHECKBOX },
              { "Climb speed",
                "gEnhancements.PlayerMovement.ClimbSpeed",
                "Increases the speed at which Link climbs vines and ladders.",
                WIDGET_SLIDER_INT,
                { 1, 5, 1 } },
              { "Dpad Equips", "gEnhancements.Dpad.DpadEquips", "Allows you to equip items to your d-pad",
                WIDGET_CHECKBOX },
              { "Always Win Doggy Race",
                "gEnhancements.Minigames.AlwaysWinDoggyRace",
                "Makes the Doggy Race easier to win.",
                WIDGET_COMBOBOX,
                { .comboBoxOptions = alwaysWinDoggyraceOptions } } },
            { { .widgetName = "Modes", .widgetType = WIDGET_SEPARATOR_TEXT },
              { "Play as Kafei", "gModes.PlayAsKafei", "Requires scene reload to take effect.", WIDGET_CHECKBOX },
              { "Time Moves when you Move",
                "gModes.TimeMovesWhenYouMove",
                "Time only moves when Link is not standing still.",
                WIDGET_CHECKBOX,
                {},
                ([](widgetInfo& info) { RegisterTimeMovesWhenYouMove(); }) } },
            { { .widgetName = "Saving", .widgetType = WIDGET_SEPARATOR_TEXT },
              { "Persistent Owl Saves", "gEnhancements.Saving.PersistentOwlSaves",
                "Continuing a save will not remove the owl save. Playing Song of "
                "Time, allowing the moon to crash or finishing the "
                "game will remove the owl save and become the new last save.",
                WIDGET_CHECKBOX },
              { "Pause Menu Save", "gEnhancements.Saving.PauseSave",
                "Re-introduce the pause menu save system. Pressing B in the pause menu will give you the "
                "option to create a persistent Owl Save from your current location.\n\nWhen loading back "
                "into the game, you will be placed either at the entrance of the dungeon you saved in, or "
                "in South Clock Town.",
                WIDGET_CHECKBOX },
              { "Autosave",
                "gEnhancements.Saving.Autosave",
                "Automatically create a persistent Owl Save on the chosen interval.\n\nWhen loading "
                "back into the game, you will be placed either at the entrance of the dungeon you "
                "saved in, or in South Clock Town.",
                WIDGET_CHECKBOX,
                {},
                ([](widgetInfo& info) { RegisterAutosave(); }) },
              { "Autosave Interval: %d minutes",
                "gEnhancements.Saving.AutosaveInterval",
                "Sets the interval between Autosaves.",
                WIDGET_SLIDER_INT,
                { 1, 60, 5 },
                nullptr,
                [](widgetInfo& info) {
                    if (disabledMap[DISABLE_FOR_AUTO_SAVE_OFF].second.active) {
                        info.activeDisables.push_back(DISABLE_FOR_AUTO_SAVE_OFF);
                    }
                } },
              { .widgetName = "Time Cycle", .widgetType = WIDGET_SEPARATOR_TEXT },
              { "Do not reset Bottle content", "gEnhancements.Cycle.DoNotResetBottleContent",
                "Playing the Song Of Time will not reset the bottles' content.", WIDGET_CHECKBOX },
              { "Do not reset Consumables", "gEnhancements.Cycle.DoNotResetConsumables",
                "Playing the Song Of Time will not reset the consumables.", WIDGET_CHECKBOX },
              { "Do not reset Razor Sword", "gEnhancements.Cycle.DoNotResetRazorSword",
                "Playing the Song Of Time will not reset the Sword back to Kokiri Sword.", WIDGET_CHECKBOX },
              { "Do not reset Rupees", "gEnhancements.Cycle.DoNotResetRupees",
                "Playing the Song Of Time will not reset the your rupees.", WIDGET_CHECKBOX },
              { .widgetName = "Unstable",
                .widgetType = WIDGET_SEPARATOR_TEXT,
                .widgetOptions = { .color = UIWidgets::Colors::Yellow } },
              { "Disable Save Delay", "gEnhancements.Saving.DisableSaveDelay",
                "Removes the arbitrary 2 second timer for saving from the original game. This is known to "
                "cause issues when attempting the 0th Day Glitch",
                WIDGET_CHECKBOX } } } });
    // Graphics Enhancements
    enhancementsSidebar.push_back(
        { "Graphics",
          3,
          { { { .widgetName = "Clock", .widgetType = WIDGET_SEPARATOR_TEXT },
              { "Clock Type",
                "gEnhancements.Graphics.ClockType",
                "Swaps between Graphical and Text only Clock types.",
                WIDGET_COMBOBOX,
                { .comboBoxOptions = clockTypeOptions } },
              { "24 Hours Clock", "gEnhancements.Graphics.24HoursClock", "Changes from a 12 Hour to a 24 Hour Clock",
                WIDGET_CHECKBOX },
              { .widgetName = "Motion Blur", .widgetType = WIDGET_SEPARATOR_TEXT },
              { "Motion Blur Mode",
                "gEnhancements.Graphics.MotionBlur.Mode",
                "Selects the Mode for Motion Blur.",
                WIDGET_COMBOBOX,
                { .comboBoxOptions = motionBlurOptions },
                [](widgetInfo& info) {
                    if (CVarGetInteger("gEnhancements.Graphics.MotionBlur.Mode", 0) == 1) {
                        CVarSetInteger("gEnhancements.Graphics.MotionBlur.Toggle", 0);
                    }
                } },
              { "Interpolate", "gEnhancements.Graphics.MotionBlur.Interpolate",
                "Change motion blur capture to also happen on interpolated frames instead of only on game frames.\n"
                "This notably reduces the overall motion blur strength but smooths out the trails.",
                WIDGET_CHECKBOX },
              { "On/Off",
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
                } },
              { "Strength",
                "gEnhancements.Graphics.MotionBlur.Strength",
                "Motion Blur strength.",
                WIDGET_SLIDER_INT,
                { 0, 255, 180 } },
              { .widgetName = "Other", .widgetType = WIDGET_SEPARATOR_TEXT },
              { "Authentic Logo", "gEnhancements.Graphics.AuthenticLogo",
                "Hide the game version and build details and display the authentic "
                "model and texture on the boot logo start screen",
                WIDGET_CHECKBOX },
              { "Bow Reticle", "gEnhancements.Graphics.BowReticle", "Gives the bow a reticle when you draw an arrow.",
                WIDGET_CHECKBOX },
              { "Disable Black Bar Letterboxes", "gEnhancements.Graphics.DisableBlackBars",
                "Disables Black Bar Letterboxes during cutscenes and Z-targeting\nNote: there may be "
                "minor visual glitches that were covered up by the black bars\nPlease disable this "
                "setting before reporting a bug.",
                WIDGET_CHECKBOX },
              { .widgetName = "Unstable",
                .widgetType = WIDGET_SEPARATOR_TEXT,
                .widgetOptions = { .color = UIWidgets::Colors::Yellow } },
              { "Disable Scene Geometry Distance Check", "gEnhancements.Graphics.DisableSceneGeometryDistanceCheck",
                "Disables the distance check for scene geometry, allowing it to be drawn no matter how far "
                "away it is from the player. This may have unintended side effects.",
                WIDGET_CHECKBOX },
              { "Increase Actor Draw Distance: %dx",
                "gEnhancements.Graphics.IncreaseActorDrawDistance",
                "Increase the range in which Actors are drawn. This may have unintended side effects.",
                WIDGET_SLIDER_INT,
                { 1, 5, 1 },
                [](widgetInfo& info) {
                    CVarSetInteger("gEnhancements.Graphics.IncreaseActorUpdateDistance",
                                   MIN(CVarGetInteger("gEnhancements.Graphics.IncreaseActorDrawDistance", 1),
                                       CVarGetInteger("gEnhancements.Graphics.IncreaseActorUpdateDistance", 1)));
                } },
              { "Increase Actor Update Distance: %dx",
                "gEnhancements.Graphics.IncreaseActorUpdateDistance",
                "Increase the range in which Actors are updated. This may have unintended side effects.",
                WIDGET_SLIDER_INT,
                { 1, 5, 1 },
                [](widgetInfo& info) {
                    CVarSetInteger("gEnhancements.Graphics.IncreaseActorDrawDistance",
                                   MAX(CVarGetInteger("gEnhancements.Graphics.IncreaseActorDrawDistance", 1),
                                       CVarGetInteger("gEnhancements.Graphics.IncreaseActorUpdateDistance", 1)));
                } } } } });
    enhancementsSidebar.push_back(
        { "Items/Songs",
          3,
          { // Mask Enhancements
            { { .widgetName = "Masks", .widgetType = WIDGET_SEPARATOR_TEXT },
              { "Blast Mask has Powder Keg Force", "gEnhancements.Masks.BlastMaskKeg",
                "Blast Mask can also destroy objects only the Powder Keg can.", WIDGET_CHECKBOX },
              { "Fast Transformation", "gEnhancements.Masks.FastTransformation",
                "Removes the delay when using transormation masks.", WIDGET_CHECKBOX },
              { "Fierce Deity's Mask Anywhere", "gEnhancements.Masks.FierceDeitysAnywhere",
                "Allow using Fierce Deity's mask outside of boss rooms.", WIDGET_CHECKBOX },
              { "No Blast Mask Cooldown", "gEnhancements.Masks.NoBlastMaskCooldown",
                "Eliminates the Cooldown between Blast Mask usage.", WIDGET_CHECKBOX } },
            // Song Enhancements
            { { .widgetName = "Ocarina", .widgetType = WIDGET_SEPARATOR_TEXT },
              { "Enable Sun's Song", "gEnhancements.Songs.EnableSunsSong",
                "Enables the partially implemented Sun's Song. RIGHT-DOWN-UP-RIGHT-DOWN-UP to play it. "
                "This song will make time move very fast until either Link moves to a different scene, "
                "or when the time switches to a new time period.",
                WIDGET_CHECKBOX },
              { "Dpad Ocarina", "gEnhancements.Playback.DpadOcarina", "Enables using the Dpad for Ocarina playback.",
                WIDGET_CHECKBOX },
              { "Prevent Dropped Ocarina Inputs", "gEnhancements.Playback.NoDropOcarinaInput",
                "Prevent dropping inputs when playing the ocarina quickly.", WIDGET_CHECKBOX } } } });
    enhancementsSidebar.push_back(
        { "Time Savers",
          3,
          { // Cutscene Skips
            { { .widgetName = "Cutscenes", .widgetType = WIDGET_SEPARATOR_TEXT },
              { "Hide Title Cards", "gEnhancements.Cutscenes.HideTitleCards", "Hides Title Cards when entering areas.",
                WIDGET_CHECKBOX },
              { "Skip Entrance Cutscenes", "gEnhancements.Cutscenes.SkipEntranceCutscenes",
                "Skip cutscenes that occur when first entering a new area.", WIDGET_CHECKBOX },
              { "Skip to File Select", "gEnhancements.Cutscenes.SkipToFileSelect",
                "Skip the opening title sequence and go straight to the file select menu after boot.",
                WIDGET_CHECKBOX },
              { "Skip Intro Sequence", "gEnhancements.Cutscenes.SkipIntroSequence",
                "When starting a game you will be taken straight to South Clock Town as Deku Link.", WIDGET_CHECKBOX },
              { "Skip Story Cutscenes", "gEnhancements.Cutscenes.SkipStoryCutscenes",
                "Disclaimer: This doesn't do much yet, we will be progressively adding more skips over time.",
                WIDGET_CHECKBOX },
              { "Skip Misc Interactions", "gEnhancements.Cutscenes.SkipMiscInteractions",
                "Disclaimer: This doesn't do much yet, we will be progressively adding more skips over time.",
                WIDGET_CHECKBOX } },
            // Dialogue Enhancements
            { { .widgetName = "Dialogue", .widgetType = WIDGET_SEPARATOR_TEXT },
              { "Fast Bank Selection", "gEnhancements.Dialogue.FastBankSelection",
                "Pressing the Z or R buttons while the Deposit/Withdrawl Rupees dialogue is open will set "
                "the Rupees to Links current Rupees or 0 respectively.",
                WIDGET_CHECKBOX },
              { "Fast Text", "gEnhancements.Dialogue.FastText",
                "Speeds up text rendering, and enables holding of B progress to next message.", WIDGET_CHECKBOX },
              {
                  "Fast Magic Arrow Equip Animation",
                  "gEnhancements.Equipment.MagicArrowEquipSpeed",
                  "Removes the animation for equipping Magic Arrows.",
                  WIDGET_CHECKBOX,
              } } } });
    enhancementsSidebar.push_back(
        { "Fixes",
          3,
          { // Fixes
            { { .widgetName = "Fixes", .widgetType = WIDGET_SEPARATOR_TEXT },
              { "Fix Ammo Count Color", "gFixes.FixAmmoCountEnvColor",
                "Fixes a missing gDPSetEnvColor, which causes the ammo count to be "
                "the wrong color prior to obtaining magic or other conditions.",
                WIDGET_CHECKBOX },
              { "Fix Hess and Weirdshot Crash", "gEnhancements.Fixes.HessCrash",
                "Fixes a crash that can occur when performing a HESS or Weirdshot.", WIDGET_CHECKBOX },
              { "Fix Text Control Characters", "gEnhancements.Fixes.ControlCharacters",
                "Fixes certain control characters not functioning properly "
                "depending on their position within the text.",
                WIDGET_CHECKBOX } } } });
    enhancementsSidebar.push_back(
        { "Restorations",
          3,
          { // Restorations
            { { .widgetName = "Restorations", .widgetType = WIDGET_SEPARATOR_TEXT },
              { "Constant Distance Backflips and Sidehops", "gEnhancements.Restorations.ConstantFlipsHops",
                "Backflips and Sidehops travel a constant distance as they did in OoT.", WIDGET_CHECKBOX },
              { "Power Crouch Stab", "gEnhancements.Restorations.PowerCrouchStab",
                "Crouch stabs will use the power of Link's previous melee attack, as is in MM JP 1.0 and OoT.",
                WIDGET_CHECKBOX },
              { "Side Rolls", "gEnhancements.Restorations.SideRoll", "Restores side rolling from OoT.",
                WIDGET_CHECKBOX },
              { "Tatl ISG", "gEnhancements.Restorations.TatlISG", "Restores Navi ISG from OoT, but now with Tatl.",
                WIDGET_CHECKBOX } } } });
    enhancementsSidebar.push_back({ "HUD Editor",
                                    2,
                                    { // HUD Editor
                                      { { "Popout HUD Editor",
                                          "gWindows.HudEditor",
                                          "Enables the HUD Editor window, allowing you to modify your HUD",
                                          WIDGET_WINDOW_BUTTON,
                                          { .windowName = "HUD Editor" } } } } });
}

void AddDevTools() {
    devToolsSidebar.push_back(
        { "General",
          3,
          { { { "Popout Menu", "gSettings.Menu.Popout", "Changes the menu display from overlay to windowed.",
                WIDGET_CHECKBOX },
              { "Open App Files Folder",
                "",
                "Opens the folder that contains the save and mods folders, etc,",
                WIDGET_BUTTON,
                {},
                [](widgetInfo& info) {
                    std::string filesPath = Ship::Context::GetInstance()->GetAppDirectoryPath();
                    SDL_OpenURL(std::string("file:///" + std::filesystem::absolute(filesPath).string()).c_str());
                } },
              { "Debug Mode",
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
                } },
              { "Better Map Select", "gDeveloperTools.BetterMapSelect.Enabled",
                "Overrides the original map select with a translated, more user-friendly version.", WIDGET_CHECKBOX },
              { "Debug Save File Mode",
                "gDeveloperTools.DebugSaveFileMode",
                "Change the behavior of creating saves while debug mode is enabled:\n\n"
                "- Empty Save: The default 3 heart save file in first cycle\n"
                "- Vanilla Debug Save: Uses the title screen save info (8 hearts, all items and masks)\n"
                "- 100\% Save: All items, equipment, mask, quast status and bombers notebook complete",
                WIDGET_COMBOBOX,
                { 0, 0, 0, debugSaveOptions },
                [](widgetInfo& info) { RegisterDebugSaveCreate(); } },
              { "Prevent Actor Update",
                "gDeveloperTools.PreventActorUpdate",
                "Prevents Actors from updating.",
                WIDGET_CHECKBOX,
                {},
                [](widgetInfo& info) { RegisterPreventActorUpdateHooks(); } },
              { "Prevent Actor Draw",
                "gDeveloperTools.PreventActorDraw",
                "Prevents Actors from drawing.",
                WIDGET_CHECKBOX,
                {},
                [](widgetInfo& info) { RegisterPreventActorDrawHooks(); } },
              { "Prevent Actor Init",
                "gDeveloperTools.PreventActorInit",
                "Prevents Actors from initializing.",
                WIDGET_CHECKBOX,
                {},
                [](widgetInfo& info) { RegisterPreventActorInitHooks(); } },
              { "Disable Object Dependency", "gDeveloperTools.DisableObjectDependency",
                "Disables dependencies when loading objects.", WIDGET_CHECKBOX },
              { "Log Level",
                "gDeveloperTools.LogLevel",
                "The log level determines which messages are printed to the "
                "console. This does not affect the log file output",
                WIDGET_COMBOBOX,
                { 0, 0, 0, logLevels },
                [](widgetInfo& info) {
                    Ship::Context::GetInstance()->GetLogger()->set_level(
                        (spdlog::level::level_enum)CVarGetInteger("gDeveloperTools.LogLevel", 1));
                } },
              { "Frame Advance",
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
                } },
              { "Advance 1",
                "",
                "Advance 1 frame.",
                WIDGET_BUTTON,
                {},
                [](widgetInfo& info) { CVarSetInteger("gDeveloperTools.FrameAdvanceTick", 1); } },
              { "Advance (Hold)", "", "Advance frames while the button is held.", WIDGET_BUTTON } } } });
    // dev tools windows
    devToolsSidebar.push_back({ "Collision Viewer",
                                2,
                                { { { "Popout Collision Viewer",
                                      "gWindows.CollisionViewer",
                                      "Makes collision visible on screen",
                                      WIDGET_WINDOW_BUTTON,
                                      { .windowName = "Collision Viewer" } } } } });
    devToolsSidebar.push_back(
        { "Stats",
          2,
          { { { "Popout Stats",
                "gOpenWindows.Stats",
                "Shows the stats window, with your FPS and frametimes, and the OS you're playing on",
                WIDGET_WINDOW_BUTTON,
                { .windowName = "Stats" } } } } });
    devToolsSidebar.push_back(
        { "Console",
          2,
          { { { "Popout Console",
                "gOpenWindows.Console",
                "Enables the console window, allowing you to input commands. Type help for some examples",
                WIDGET_WINDOW_BUTTON,
                { .windowName = "Console" } } } } });
    devToolsSidebar.push_back(
        { "Gfx Debugger",
          2,
          { { { "Popout Gfx Debugger",
                "gOpenWindows.GfxDebugger",
                "Enables the Gfx Debugger window, allowing you to input commands, type help for some examples",
                WIDGET_WINDOW_BUTTON,
                { .windowName = "GfxDebuggerWindow" } } } } });
    devToolsSidebar.push_back({ "Save Editor",
                                2,
                                { { { "Popout Save Editor",
                                      "gWindows.SaveEditor",
                                      "Enables the Save Editor window, allowing you to edit your save file",
                                      WIDGET_WINDOW_BUTTON,
                                      { .windowName = "Save Editor" } } } } });
    devToolsSidebar.push_back({ "Actor Viewer",
                                2,
                                { { { "Popout Actor Viewer",
                                      "gWindows.ActorViewer",
                                      "Enables the Actor Viewer window, allowing you to view actors in the world.",
                                      WIDGET_WINDOW_BUTTON,
                                      { .windowName = "Actor Viewer" } } } } });
    devToolsSidebar.push_back({ "Event Log",
                                2,
                                { { { "Popout Event Log",
                                      "gWindows.EventLog",
                                      "Enables the event log window",
                                      WIDGET_WINDOW_BUTTON,
                                      { .windowName = "Event Log" } } } } });
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
            std::string menuSearchText(menuSearch.InputBuf);

            if (menuSearchText == "") {
                ImGui::Text("Start typing to see results.");
                return;
            }
            ImGui::BeginChild("Search Results");
            for (auto& [menuLabel, menuSidebar, cvar] : menuEntries) {
                for (auto& sidebar : menuSidebar) {
                    for (auto& widgets : sidebar.columnWidgets) {
                        int column = 1;
                        for (auto& info : widgets) {
                            if (info.widgetType == WIDGET_SEARCH || info.widgetType == WIDGET_SEPARATOR ||
                                info.widgetType == WIDGET_SEPARATOR_TEXT || info.isHidden) {
                                continue;
                            }
                            std::string widgetStr = std::string(info.widgetName) + std::string(info.widgetTooltip);
                            std::transform(menuSearchText.begin(), menuSearchText.end(), menuSearchText.begin(),
                                           ::tolower);
                            menuSearchText.erase(std::remove(menuSearchText.begin(), menuSearchText.end(), ' '),
                                                 menuSearchText.end());
                            std::transform(widgetStr.begin(), widgetStr.end(), widgetStr.begin(), ::tolower);
                            widgetStr.erase(std::remove(widgetStr.begin(), widgetStr.end(), ' '), widgetStr.end());
                            if (widgetStr.find(menuSearchText) != std::string::npos) {
                                SearchMenuGetItem(info);
                                ImGui::PushStyleColor(ImGuiCol_Text, UIWidgets::Colors::Gray);
                                std::string origin =
                                    fmt::format("  ({} -> {}, Clmn {})", menuLabel, sidebar.label, column);
                                ImGui::Text(origin.c_str());
                                ImGui::PopStyleColor();
                            }
                        }
                        column++;
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