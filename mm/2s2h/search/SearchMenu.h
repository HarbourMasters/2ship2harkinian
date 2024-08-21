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
using ModifierFunc = void (*)(widgetInfo&);
std::string disabledTempTooltip;
const char* disabledTooltip;
bool disabledValue = false;
ColorOption menuThemeIndex = COLOR_INDIGO;

typedef enum {
    WIDGET_CHECKBOX,
    WIDGET_COMBOBOX,
    WIDGET_SLIDER_INT,
    WIDGET_SLIDER_FLOAT,
    WIDGET_BUTTON,
    WIDGET_COLOR_24,
    WIDGET_COLOR_32,
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
    uint32_t widgetIndex;
    const char* widgetName;
    const char* widgetCVar;
    const char* widgetTooltip;
    uint32_t widgetType;
    WidgetOptions widgetOptions;
    VoidFunc widgetCallback = nullptr;
    ModifierFunc modifierFunc = nullptr;
    DisableVec activeDisables = {};
    bool isHidden = false;
};

struct disabledInfo {
    ConditionFunc evaluation;
    const char* reason;
    bool active = false;
    int32_t state = 0;
};

namespace BenGui {
extern std::shared_ptr<std::vector<Ship::WindowBackend>> availableWindowBackends;
extern std::unordered_map<Ship::WindowBackend, const char*> availableWindowBackendsMap;
extern Ship::WindowBackend configWindowBackend;
extern void UpdateWindowBackendObjects();

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
    { DISABLE_FOR_DEBUG_MODE_OFF, { []() -> bool 
      { return !CVarGetInteger("gDeveloperTools.DebugEnabled", 0); }, "Save Not Loaded" } },
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

typedef enum { SIDEBAR_SECTION_SETTINGS_GENERAL, SIDEBAR_SECTION_SETTINGS_GRAPHICS, SIDEBAR_SECTION_SETTINGS_ };

typedef enum {
    MENU_ITEM_MENU_THEME,
    MENU_ITEM_MENUBAR_CONTROLLER_NAV,
    MENU_ITEM_CURSOR_VISIBILITY,
    MENU_ITEM_HOTKEY_TEXT,
    MENU_ITEM_MASTER_VOLUME,
    MENU_ITEM_MAIN_MUSIC_VOLUME,
    MENU_ITEM_SUB_MUSIC_VOLUME,
    MENU_ITEM_SOUND_EFFECT_VOLUME,
    MENU_ITEM_FANFARE_VOLUME,
    MENU_ITEM_AMBIENT_VOLUME,
    MENU_ITEM_AUDIO_API,
    MENU_ITEM_TOGGLE_FULLSCREEN,
    MENU_ITEM_INTERNAL_RESOLUTION,
    MENU_ITEM_MSAA,
    MENU_ITEM_FRAME_RATE,
    MENU_ITEM_MATCH_REFRESH_RATE_BUTTON,
    MENU_ITEM_MATCH_REFRESH_RATE_CHECK,
    MENU_ITEM_RENDERER_API,
    MENU_ITEM_JITTER_FIX,
    MENU_ITEM_ENABLE_VSYNC,
    MENU_ITEM_ENABLE_WINDOWED_FULLSCREEN,
    MENU_ITEM_ENABLE_MULTI_VIEWPORT,
    MENU_ITEM_TEXTURE_FILTER,
    MENU_ITEM_INPUT_EDITOR,
    MENU_ITEM_FIX_TARGET_CAMERA_SNAP,
    MENU_ITEM_ENABLE_FREE_LOOK,
    MENU_ITEM_FREE_LOOK_CAMERA_DISTANCE,
    MENU_ITEM_FREE_LOOK_TRANSITION_SPEED,
    MENU_ITEM_FREE_LOOK_MAX_PITCH,
    MENU_ITEM_FREE_LOOK_MIN_PITCH,
    MENU_ITEM_ENABLE_DEBUG_CAMERA,
    MENU_ITEM_INVERT_CAMERA_X_AXIS,
    MENU_ITEM_INVERT_CAMERA_Y_AXIS,
    MENU_ITEM_THIRD_PERSON_CAMERA_X_SENSITIVITY,
    MENU_ITEM_THIRD_PERSON_CAMERA_Y_SENSITIVITY,
    MENU_ITEM_ENABLE_CAMERA_ROLL,
    MENU_ITEM_CAMERA_SPEED,
    MENU_ITEM_CHEATS_INFINITE_HEALTH,
    MENU_ITEM_CHEATS_INFINITE_MAGIC,
    MENU_ITEM_CHEATS_INFINITE_RUPEES,
    MENU_ITEM_CHEATS_INFINITE_CONSUMABLES,
    MENU_ITEM_CHEATS_LONG_FLOWER_GLIDE,
    MENU_ITEM_CHEATS_NO_CLIP,
    MENU_ITEM_CHEATS_INFINITE_RAZOR_SWORD,
    MENU_ITEM_CHEATS_UNRESTRICTED_ITEMS,
    MENU_ITEM_CHEATS_MOON_JUMP_ON_L,
    MENU_ITEM_FAST_FLOWER_LAUNCH,
    MENU_ITEM_INSTANT_PUTAWAY,
    MENU_ITEM_CLIMB_SPEED,
    MENU_ITEM_DPAD_EQUIPS,
    MENU_ITEM_ALWAYS_WIN_DOGGY_RACE,
    MENU_ITEM_PLAY_AS_KAFEI,
    MENU_ITEM_TIME_MOVES_WHEN_YOU_MOVE,
    MENU_ITEM_PERSIST_OWL_SAVES,
    MENU_ITEM_PAUSE_MENU_SAVE,
    MENU_ITEM_AUTOSAVE,
    MENU_ITEM_AUTOSAVE_INTERVAL,
    MENU_ITEM_DISABLE_BOTTLE_RESET,
    MENU_ITEM_DISABLE_CONSUMABLE_RESET,
    MENU_ITEM_DISABLE_RAZOR_SWORD_RESET,
    MENU_ITEM_DISABLE_RUPEE_RESET,
    MENU_ITEM_DISABLE_SAVE_DELAY,
    MENU_ITEM_CLOCK_OPTIONS,
    MENU_ITEM_MILITARY_CLOCK,
    MENU_ITEM_MOTION_BLUR_MODE,
    MENU_ITEM_MOTION_BLUR_INTERPOLATE,
    MENU_ITEM_MOTION_BLUR_ENABLE,
    MENU_ITEM_MOTION_BLUR_STRENGTH,
    MENU_ITEM_AUTHENTIC_LOGO,
    MENU_ITEM_BOW_RETICLE,
    MENU_ITEM_DISABLE_BLACK_BARS,
    MENU_ITEM_GEOMETRY_DISTANCE_CHECK,
    MENU_ITEM_ACTOR_DRAW_DISTANCE,
    MENU_ITEM_ACTOR_UPDATE_DISTANCE,
    MENU_ITEM_BLAST_MASK_KEG_FORCE,
    MENU_ITEM_FAST_TRANSFORMATION,
    MENU_ITEM_FIERCE_DEITY_ANYWHERE,
    MENU_ITEM_NO_BLAST_MASK_COOLDOWN,
    MENU_ITEM_ENABLE_SUNS_SONG,
    MENU_ITEM_DPAD_OCARINA,
    MENU_ITEM_PREVENT_DROPPED_OCARINA_INPUTS,
    MENU_ITEM_HIDE_TITLE_CARDS,
    MENU_ITEM_SKIP_ENTRANCE_CUTSCENES,
    MENU_ITEM_SKIP_TO_FILE_SELECT,
    MENU_ITEM_SKIP_INTRO_SEQUENCE,
    MENU_ITEM_SKIP_STORY_CUTSCENES,
    MENU_ITEM_SKIP_MISC_INTERACTIONS,
    MENU_ITEM_FAST_BANK_SELECTION,
    MENU_ITEM_FAST_TEXT,
    MENU_ITEM_FAST_MAGIC_ARROW_ANIM,
    MENU_ITEM_FIX_AMMO_COUNT_COLOR,
    MENU_ITEM_FIX_HESS_WEIRDSHOT,
    MENU_ITEM_FIX_TEXT_CONTROL_CHAR,
    MENU_ITEM_RESTORE_DISTANCE_FLIPS_HOPS,
    MENU_ITEM_RESTORE_POWER_CROUCH_STAB,
    MENU_ITEM_RESTORE_SIDEROLLS,
    MENU_ITEM_RESTORE_TATL_ISG,
    MENU_ITEM_MODERN_MENU_POPOUT,
    MENU_ITEM_OPEN_APP_FILES,
    MENU_ITEM_DEBUG_MODE_ENABLE,
    MENU_ITEM_DEBUG_BETTER_MAP_SELECT,
    MENU_ITEM_DEBUG_SAVE_FILE_MODE,
    MENU_ITEM_PREVENT_ACTOR_UPDATE,
    MENU_ITEM_PREVENT_ACTOR_DRAW,
    MENU_ITEM_PREVENT_ACTOR_INIT,
    MENU_ITEM_DISABLE_OBJECT_DEPENDECY,
    MENU_ITEM_DEBUG_LOG_LEVEL,
    MENU_ITEM_FRAME_ADVANCE_ENABLE,
    MENU_ITEM_FRAME_ADVANCE_SINGLE,
    MENU_ITEM_FRAME_ADVANCE_HOLD,
    MENU_ITEM_INPUT_EDITOR_WINDOW,
    MENU_ITEM_COLLISION_VIEWER_BUTTON,
    MENU_ITEM_STATS_BUTTON,
    MENU_ITEM_HUD_EDITOR_BUTTON
};

void FreeLookPitchMinMax() {
    f32 maxY = CVarGetFloat("gEnhancements.Camera.FreeLook.MaxPitch", 72.0f);
    f32 minY = CVarGetFloat("gEnhancements.Camera.FreeLook.MinPitch", -49.0f);
    CVarSetFloat("gEnhancements.Camera.FreeLook.MaxPitch", std::max(maxY, minY));
    CVarSetFloat("gEnhancements.Camera.FreeLook.MinPitch", std::min(maxY, minY));
}

widgetInfo enhancementList[] = {
    // Menu Theme
    { MENU_ITEM_MENU_THEME,
      "Menu Theme",
      "gSettings.MenuTheme",
      "Changes the Theme of the Menu Widgets.",
      WIDGET_COMBOBOX,
      { 0, 0, 0, menuThemeOptions } },
    // General Settings
    { MENU_ITEM_MENUBAR_CONTROLLER_NAV,
      "Menubar Controller Navigation",
      CVAR_IMGUI_CONTROLLER_NAV,
      "Allows controller navigation of the SOH menu bar (Settings, Enhancements,...)\nCAUTION: "
      "This will disable game inputs while the menubar is visible.\n\nD-pad to move between "
      "items, A to select, and X to grab focus on the menu bar",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_CURSOR_VISIBILITY,
      "Cursor Always Visible",
      "gSettings.CursorVisibility",
      "Makes the cursor always visible, even in full screen.",
      WIDGET_CHECKBOX,
      {},
      ([]() {
          Ship::Context::GetInstance()->GetWindow()->SetForceCursorVisibility(
              CVarGetInteger("gSettings.CursorVisibility", 0));
      }) },
    { MENU_ITEM_HOTKEY_TEXT,
      "Hide Menu Hotkey Text",
      "gSettings.DisableMenuShortcutNotify",
      "Prevents showing the text telling you the shortcut to open the menu\n"
      "when the menu isn't open.",
      WIDGET_CHECKBOX,
      {} },
    // Audio Settings
    { MENU_ITEM_MASTER_VOLUME,
      "Master Volume: %.2f%%",
      "gSettings.Audio.MasterVolume",
      "Adjust overall sound volume.",
      WIDGET_SLIDER_FLOAT,
      { 0.0f, 100.0f, 100.0f } },
    { MENU_ITEM_MAIN_MUSIC_VOLUME,
      "Main Music Volume: %.2f%%",
      "gSettings.Audio.MainMusicVolume",
      "Adjust the Background Music volume.",
      WIDGET_SLIDER_FLOAT,
      { 0.0f, 100.0f, 100.0f },
      ([]() {
          AudioSeq_SetPortVolumeScale(SEQ_PLAYER_BGM_MAIN, CVarGetFloat("gSettings.Audio.MainMusicVolume", 1.0f));
      }) },
    { MENU_ITEM_SUB_MUSIC_VOLUME,
      "Sub Music Volume: %.2f%%",
      "gSettings.Audio.SubMusicVolume",
      "Adjust the Sub Music volume.",
      WIDGET_SLIDER_FLOAT,
      { 0.0f, 100.0f, 100.0f },
      ([]() {
          AudioSeq_SetPortVolumeScale(SEQ_PLAYER_BGM_SUB, CVarGetFloat("gSettings.Audio.SubMusicVolume", 1.0f));
      }) },
    { MENU_ITEM_SOUND_EFFECT_VOLUME,
      "Sound Effects Volume: %.2f%%",
      "gSettings.Audio.SoundEffectsVolume",
      "Adjust the Sound Effects volume.",
      WIDGET_SLIDER_FLOAT,
      { 0.0f, 100.0f, 100.0f },
      ([]() {
          AudioSeq_SetPortVolumeScale(SEQ_PLAYER_SFX, CVarGetFloat("gSettings.Audio.SoundEffectsVolume", 1.0f));
      }) },
    { MENU_ITEM_FANFARE_VOLUME,
      "Fanfare Volume: %.2f%%",
      "gSettings.Audio.FanfareVolume",
      "Adjust the Fanfare volume.",
      WIDGET_SLIDER_FLOAT,
      { 0.0f, 100.0f, 100.0f },
      ([]() {
          AudioSeq_SetPortVolumeScale(SEQ_PLAYER_FANFARE, CVarGetFloat("gSettings.Audio.FanfareVolume", 1.0f));
      }) },
    { MENU_ITEM_AMBIENT_VOLUME,
      "Ambience Volume: %.2f%%",
      "gSettings.Audio.AmbienceVolume",
      "Adjust the Ambient Sound volume.",
      WIDGET_SLIDER_FLOAT,
      { 0.0f, 100.0f, 100.0f },
      ([]() {
          AudioSeq_SetPortVolumeScale(SEQ_PLAYER_AMBIENCE, CVarGetFloat("gSettings.Audio.AmbienceVolume", 1.0f));
      }) },
    { MENU_ITEM_AUDIO_API,
      "Audio API",
      NULL,
      "Sets the audio API used by the game. Requires a relaunch to take effect.",
      WIDGET_AUDIO_BACKEND },
    // Graphics Settings
    { MENU_ITEM_TOGGLE_FULLSCREEN,
      "Toggle Fullscreen",
      "gSettings.Fullscreen",
      "Toggles Fullscreen On/Off.",
      WIDGET_CHECKBOX,
      {},
      ([]() { Ship::Context::GetInstance()->GetWindow()->ToggleFullscreen(); }) },
    { MENU_ITEM_INTERNAL_RESOLUTION,
      "Internal Resolution: %f%%",
      CVAR_INTERNAL_RESOLUTION,
      "Multiplies your output resolution by the value inputted, as a more intensive but effective "
      "form of anti-aliasing.",
      WIDGET_SLIDER_FLOAT,
      { 50.0f, 200.0f, 100.0f },
      ([]() {
          Ship::Context::GetInstance()->GetWindow()->SetResolutionMultiplier(CVarGetFloat(CVAR_INTERNAL_RESOLUTION, 1));
      }) },
    { MENU_ITEM_MSAA,
      "Anti-aliasing (MSAA): %d",
      CVAR_MSAA_VALUE,
      "Activates MSAA (multi-sample anti-aliasing) from 2x up to 8x, to smooth the edges of rendered "
      "geometry.\n"
      "Higher sample count will result in smoother edges on models, but may reduce performance.",
      WIDGET_SLIDER_INT,
      { 1, 8, 1 },
      ([]() { Ship::Context::GetInstance()->GetWindow()->SetMsaaLevel(CVarGetInteger(CVAR_MSAA_VALUE, 1)); }) },
    { MENU_ITEM_FRAME_RATE,
      "Current FPS: %d",
      "gInterpolationFPS",
      "Uses Matrix Interpolation to create extra frames, resulting in smoother graphics. This is "
      "purely visual and does not impact game logic, execution of glitches etc.\n\n A higher target "
      "FPS than your monitor's refresh rate will waste resources, and might give a worse result.",
      WIDGET_SLIDER_INT,
      { 20, static_cast<int32_t>(Ship::Context::GetInstance()->GetWindow()->GetCurrentRefreshRate()), 20 },
      nullptr,
      [](widgetInfo& info) {
          if (Ship::Context::GetInstance()->GetWindow()->GetWindowBackend() == Ship::WindowBackend::FAST3D_DXGI_DX11 && 
              std::get<int32_t>(info.widgetOptions.max) != 360) {
              info.widgetOptions.max = 360;
              info.widgetTooltip = "Uses Matrix Interpolation to create extra frames, resulting in smoother graphics. This is "
                  "purely visual and does not impact game logic, execution of glitches etc.\n\n A higher target "
                  "FPS than your monitor's refresh rate will waste resources, and might give a worse result.";
          }
          int32_t defaultVal = std::get<int32_t>(info.widgetOptions.defaultVariant);
          if (CVarGetInteger(info.widgetCVar, defaultVal) == defaultVal) {
              info.widgetName = "Current FPS: Original (%d)";
          } else {
              info.widgetName = "Current FPS: %d";
          }
          if (disabledMap[DISABLE_FOR_MATCH_REFRESH_RATE_ON].second.active)
              info.activeDisables.push_back(DISABLE_FOR_MATCH_REFRESH_RATE_ON);
      } },
    { MENU_ITEM_MATCH_REFRESH_RATE_BUTTON,
      "Match Refresh Rate",
      "",
      "Matches interpolation value to the current game's window refresh rate.",
      WIDGET_BUTTON,
      {},
      ([]() {
          int hz = Ship::Context::GetInstance()->GetWindow()->GetCurrentRefreshRate();
          if (hz >= 20 && hz <= 360) {
              CVarSetInteger("gInterpolationFPS", hz);
              Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
          }
      }),
      [](widgetInfo& info) {
          if (disabledMap[DISABLE_FOR_NOT_DIRECTX].second.active)
              info.isHidden = true;
      } },
    { MENU_ITEM_MATCH_REFRESH_RATE_CHECK,
      "Match Refresh Rate",
      "gMatchRefreshRate",
      "Matches interpolation value to the current game's window refresh rate.",
      WIDGET_CHECKBOX,
      {},
      ([]() {
          int hz = Ship::Context::GetInstance()->GetWindow()->GetCurrentRefreshRate();
          if (hz >= 20 && hz <= 360) {
              CVarSetInteger("gInterpolationFPS", hz);
              Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
          }
      }),
      [](widgetInfo& info) {
          if (!disabledMap[DISABLE_FOR_NOT_DIRECTX].second.active)
              info.isHidden = true;
      } },
    { MENU_ITEM_JITTER_FIX,
      "Jitter fix : >= % d FPS",
      "gExtraLatencyThreshold",
      "When Interpolation FPS setting is at least this threshold, add one frame of input "
      "lag (e.g. 16.6 ms for 60 FPS) in order to avoid jitter. This setting allows the "
      "CPU to work on one frame while GPU works on the previous frame.\nThis setting "
      "should be used when your computer is too slow to do CPU + GPU work in time.",
      WIDGET_SLIDER_INT,
      { 0, 360, 80 },
      nullptr,
      [](widgetInfo& info) { info.isHidden = disabledMap[DISABLE_FOR_NOT_DIRECTX].second.active; } },
    { MENU_ITEM_RENDERER_API,
      "Renderer API (Needs reload)",
      NULL,
      "Sets the audio API used by the game. Requires a relaunch to take effect.",
      WIDGET_AUDIO_BACKEND },
    { MENU_ITEM_ENABLE_VSYNC, "Enable Vsync", CVAR_VSYNC_ENABLED, "Enables Vsync.", WIDGET_CHECKBOX, {}, nullptr,
      [](widgetInfo& info) { info.isHidden = disabledMap[DISABLE_FOR_NO_VSYNC].second.active; } },
    { MENU_ITEM_ENABLE_WINDOWED_FULLSCREEN,
      "Windowed Fullscreen",
      CVAR_SDL_WINDOWED_FULLSCREEN,
      "Enables Windowed Fullscreen Mode.",
      WIDGET_CHECKBOX,
      {},
      nullptr,
        [](widgetInfo& info) { info.isHidden = disabledMap[DISABLE_FOR_NO_WINDOWED_FULLSCREEN].second.active; } },
    { MENU_ITEM_ENABLE_MULTI_VIEWPORT,
      "Allow multi-windows",
      CVAR_ENABLE_MULTI_VIEWPORTS,
      "Allows multiple windows to be opened at once. Requires a reload to take effect.",
      WIDGET_CHECKBOX,
      {},
      nullptr,
        [](widgetInfo& info) { info.isHidden = disabledMap[DISABLE_FOR_NO_MULTI_VIEWPORT].second.active; } },
    { MENU_ITEM_TEXTURE_FILTER,
      "Texture Filter (Needs reload)",
      CVAR_TEXTURE_FILTER,
      "Sets the applied Texture Filtering.",
      WIDGET_COMBOBOX,
      { 0, 0, 0, textureFilteringMap } },
    // Input Editor
    { MENU_ITEM_INPUT_EDITOR,
      "Popout Input Editor",
      "gWindows.BenInputEditor",
      "Enables the separate Input Editor window.",
      WIDGET_BUTTON,
      {} },
    // Camera Snap Fix
    { MENU_ITEM_FIX_TARGET_CAMERA_SNAP,
      "Fix Targetting Camera Snap",
      "gEnhancements.Camera.FixTargettingCameraSnap",
      "Fixes the camera snap that occurs when you are moving and press the targetting button.",
      WIDGET_CHECKBOX,
      {} },
    // Camera Enhancements
    { MENU_ITEM_ENABLE_FREE_LOOK,
      "Free Look",
      "gEnhancements.Camera.FreeLook.Enable",
      "Enables free look camera control\nNote: You must remap C buttons off of the right "
      "stick in the controller config menu, and map the camera stick to the right stick.",
      WIDGET_CHECKBOX,
      {},
      ([]() { RegisterCameraFreeLook(); }),
      [](widgetInfo& info) {
          if (disabledMap[DISABLE_FOR_DEBUG_CAM_ON].second.active)
              info.activeDisables.push_back(DISABLE_FOR_DEBUG_CAM_ON);
      } },
    { MENU_ITEM_FREE_LOOK_CAMERA_DISTANCE,
      "Camera Distance: %d",
      "gEnhancements.Camera.FreeLook.MaxCameraDistance",
      "Maximum Camera Distance for Free Look.",
      WIDGET_SLIDER_INT,
      { 100, 900, 185 },
      nullptr,
      [](widgetInfo& info) { info.isHidden = disabledMap[DISABLE_FOR_FREE_LOOK_OFF].second.active; } },
    { MENU_ITEM_FREE_LOOK_TRANSITION_SPEED,
      "Camera Transition Speed: %d",
      "gEnhancements.Camera.FreeLook.TransitionSpeed",
      "Can someone help me?",
      WIDGET_SLIDER_INT,
      { 1, 900, 25 },
      nullptr,
      [](widgetInfo& info) { info.isHidden = disabledMap[DISABLE_FOR_FREE_LOOK_OFF].second.active; } },
    { MENU_ITEM_FREE_LOOK_MAX_PITCH,
      "Max Camera Height Angle: %.0f\xC2\xB0",
      "gEnhancements.Camera.FreeLook.MaxPitch",
      "Maximum Height of the Camera.",
      WIDGET_SLIDER_FLOAT,
      { -8900.0f, 8900.0f, 7200.0f },
      []() { FreeLookPitchMinMax(); },
      [](widgetInfo& info) { info.isHidden = disabledMap[DISABLE_FOR_FREE_LOOK_OFF].second.active; } },
    { MENU_ITEM_FREE_LOOK_MIN_PITCH,
      "Min Camera Height Angle: %.0f\xC2\xB0",
      "gEnhancements.Camera.FreeLook.MinPitch",
      "Minimum Height of the Camera.",
      WIDGET_SLIDER_FLOAT,
      { -8900.0f, 8900.0f, -4900.0f },
      []() { FreeLookPitchMinMax(); },
      [](widgetInfo& info) { info.isHidden = disabledMap[DISABLE_FOR_FREE_LOOK_OFF].second.active; } },
    { MENU_ITEM_ENABLE_DEBUG_CAMERA,
      "Debug Camera",
      "gEnhancements.Camera.DebugCam.Enable",
      "Enables free camera control.",
      WIDGET_CHECKBOX,
      {},
      ([]() { RegisterDebugCam(); }),
      [](widgetInfo& info) {
          if (disabledMap[DISABLE_FOR_FREE_LOOK_ON].second.active) {
              info.activeDisables.push_back(DISABLE_FOR_FREE_LOOK_ON);
          }
      } },
    { MENU_ITEM_INVERT_CAMERA_X_AXIS,
      "Invert Camera X Axis",
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
    { MENU_ITEM_INVERT_CAMERA_Y_AXIS,
      "Invert Camera Y Axis",
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
    { MENU_ITEM_THIRD_PERSON_CAMERA_X_SENSITIVITY,
      "Third-Person Horizontal Sensitivity: %.0f",
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
    { MENU_ITEM_THIRD_PERSON_CAMERA_Y_SENSITIVITY,
      "Third-Person Vertical Sensitivity: %.0f",
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
    { MENU_ITEM_ENABLE_CAMERA_ROLL,
      "Enable Roll (6\xC2\xB0 of Freedom)",
      "gEnhancements.Camera.DebugCam.6DOF",
      "This allows for all six degrees of movement with the camera, NOTE: Yaw will work "
      "differently in this system, instead rotating around the focal point"
      ", rather than a polar axis.",
      WIDGET_CHECKBOX,
      {},
      nullptr,
      [](widgetInfo& info) { info.isHidden = disabledMap[DISABLE_FOR_DEBUG_CAM_OFF].second.active; } },
    { MENU_ITEM_CAMERA_SPEED,
      "Camera Speed: %.0f",
      "gEnhancements.Camera.DebugCam.CameraSpeed",
      "Adjusts the speed of the Camera.",
      WIDGET_SLIDER_FLOAT,
      { 10.0f, 300.0f, 50.0f },
      nullptr,
      [](widgetInfo& info) { info.isHidden = disabledMap[DISABLE_FOR_DEBUG_CAM_OFF].second.active; } },
    // Cheats
    { MENU_ITEM_CHEATS_INFINITE_HEALTH,
      "Infinite Health",
      "gCheats.InfiniteHealth",
      "Always have full Hearts.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_CHEATS_INFINITE_MAGIC,
      "Infinite Magic",
      "gCheats.InfiniteMagic",
      "Always have full Magic.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_CHEATS_INFINITE_RUPEES,
      "Infinite Rupees",
      "gCheats.InfiniteRupees",
      "Always have a full Wallet.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_CHEATS_INFINITE_CONSUMABLES,
      "Infinite Consumables",
      "gCheats.InfiniteConsumables",
      "Always have max Consumables, you must have collected the consumables first.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_CHEATS_LONG_FLOWER_GLIDE,
      "Longer Deku Flower Glide",
      "gCheats.LongerFlowerGlide",
      "Allows Deku Link to glide longer, no longer dropping after a certain distance.",
      WIDGET_CHECKBOX,
      {},
      ([]() { RegisterLongerFlowerGlide(); }) },
    { MENU_ITEM_CHEATS_NO_CLIP,
      "No Clip",
      "gCheats.NoClip",
      "Allows Link to phase through collision.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_CHEATS_INFINITE_RAZOR_SWORD,
      "Unbreakable Razor Sword",
      "gCheats.UnbreakableRazorSword",
      "Allows to Razor Sword to be used indefinitely without dulling its blade.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_CHEATS_UNRESTRICTED_ITEMS,
      "Unrestricted Items",
      "gCheats.UnrestrictedItems",
      "Allows all Forms to use all Items.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_CHEATS_MOON_JUMP_ON_L,
      "Moon Jump on L",
      "gCheats.MoonJumpOnL",
      "Holding L makes you float into the air.",
      WIDGET_CHECKBOX,
      {},
      ([]() { RegisterMoonJumpOnL(); }) },
    // Gameplay Enhancements
    { MENU_ITEM_FAST_FLOWER_LAUNCH,
      "Fast Deku Flower Launch",
      "gEnhancements.Player.FastFlowerLaunch",
      "Speeds up the time it takes to be able to get maximum height from launching out of a deku flower",
      WIDGET_CHECKBOX,
      {},
      ([]() { RegisterFastFlowerLaunch(); }) },
    { MENU_ITEM_INSTANT_PUTAWAY,
      "Instant Putaway",
      "gEnhancements.Player.InstantPutaway",
      "Allows Link to instantly puts away held item without waiting.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_CLIMB_SPEED,
      "Climb speed",
      "gEnhancements.PlayerMovement.ClimbSpeed",
      "Increases the speed at which Link climbs vines and ladders.",
      WIDGET_SLIDER_INT,
      { 1, 5, 1 } },
    { MENU_ITEM_DPAD_EQUIPS,
      "Dpad Equips",
      "gEnhancements.Dpad.DpadEquips",
      "Allows you to equip items to your d-pad",
      WIDGET_CHECKBOX,
      {},
      nullptr },
    { MENU_ITEM_ALWAYS_WIN_DOGGY_RACE,
      "Always Win Doggy Race",
      "gEnhancements.Minigames.AlwaysWinDoggyRace",
      "Makes the Doggy Race easier to win.",
      WIDGET_COMBOBOX,
      { 0, 0, 0, alwaysWinDoggyraceOptions } },
    // Game Modes
    { MENU_ITEM_PLAY_AS_KAFEI,
      "Play as Kafei",
      "gModes.PlayAsKafei",
      "Requires scene reload to take effect.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_TIME_MOVES_WHEN_YOU_MOVE,
      "Time Moves when you Move",
      "gModes.TimeMovesWhenYouMove",
      "Time only moves when Link is not standing still.",
      WIDGET_CHECKBOX,
      {},
      ([]() { RegisterTimeMovesWhenYouMove(); }) },
    // Saving Enhancements
    { MENU_ITEM_PERSIST_OWL_SAVES,
      "Persistent Owl Saves",
      "gEnhancements.Saving.PersistentOwlSaves",
      "Continuing a save will not remove the owl save. Playing Song of "
      "Time, allowing the moon to crash or finishing the "
      "game will remove the owl save and become the new last save.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_PAUSE_MENU_SAVE,
      "Pause Menu Save",
      "gEnhancements.Saving.PauseSave",
      "Re-introduce the pause menu save system. Pressing B in the pause menu will give you the "
      "option to create a persistent Owl Save from your current location.\n\nWhen loading back "
      "into the game, you will be placed either at the entrance of the dungeon you saved in, or "
      "in South Clock Town.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_AUTOSAVE,
      "Autosave",
      "gEnhancements.Saving.Autosave",
      "Automatically create a persistent Owl Save on the chosen interval.\n\nWhen loading "
      "back into the game, you will be placed either at the entrance of the dungeon you "
      "saved in, or in South Clock Town.",
      WIDGET_CHECKBOX,
      {},
      ([]() { RegisterAutosave(); }) },
    { MENU_ITEM_AUTOSAVE_INTERVAL,
      "Autosave Interval: %d minutes",
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
    { MENU_ITEM_DISABLE_BOTTLE_RESET,
      "Do not reset Bottle content",
      "gEnhancements.Cycle.DoNotResetBottleContent",
      "Playing the Song Of Time will not reset the bottles' content.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_DISABLE_CONSUMABLE_RESET,
      "Do not reset Consumables",
      "gEnhancements.Cycle.DoNotResetConsumables",
      "Playing the Song Of Time will not reset the consumables.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_DISABLE_RAZOR_SWORD_RESET,
      "Do not reset Razor Sword",
      "gEnhancements.Cycle.DoNotResetRazorSword",
      "Playing the Song Of Time will not reset the Sword back to Kokiri Sword.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_DISABLE_RUPEE_RESET,
      "Do not reset Rupees",
      "gEnhancements.Cycle.DoNotResetRupees",
      "Playing the Song Of Time will not reset the your rupees.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_DISABLE_SAVE_DELAY,
      "Disable Save Delay",
      "gEnhancements.Saving.DisableSaveDelay",
      "Removes the arbitrary 2 second timer for saving from the original game. This is known to "
      "cause issues when attempting the 0th Day Glitch",
      WIDGET_CHECKBOX,
      {} },
    // Graphics Enhancements
    { MENU_ITEM_CLOCK_OPTIONS,
      "Clock Type",
      "gEnhancements.Graphics.ClockType",
      "Swaps between Graphical and Text only Clock types.",
      WIDGET_COMBOBOX,
      { 0, 0, 0, clockTypeOptions } },
    { MENU_ITEM_MILITARY_CLOCK,
      "24 Hours Clock",
      "gEnhancements.Graphics.24HoursClock",
      "Changes from a 12 Hour to a 24 Hour Clock",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_MOTION_BLUR_MODE,
      "Motion Blur Mode",
      "gEnhancements.Graphics.MotionBlur.Mode",
      "Selects the Mode for Motion Blur.",
      WIDGET_COMBOBOX,
      { 0, 0, 0, motionBlurOptions },
      ([]() {
          if (CVarGetInteger("gEnhancements.Graphics.MotionBlur.Mode", 0) == 1) {
              CVarSetInteger("gEnhancements.Graphics.MotionBlur.Toggle", 0);
          }
      }) },
    { MENU_ITEM_MOTION_BLUR_INTERPOLATE,
      "Interpolate",
      "gEnhancements.Graphics.MotionBlur.Interpolate",
      "Change motion blur capture to also happen on interpolated frames instead of only on game frames.\n"
      "This notably reduces the overall motion blur strength but smooths out the trails.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_MOTION_BLUR_ENABLE,
      "On/Off",
      "gEnhancements.Graphics.MotionBlur.Toggle",
      "Enables Motion Blur.",
      WIDGET_CHECKBOX,
      {},
      ([]() {
          if (CVarGetInteger("gEnhancements.Graphics.MotionBlur.Toggle", 0) == 0) {
              R_MOTION_BLUR_ENABLED;
              R_MOTION_BLUR_ALPHA = CVarGetInteger("gEnhancements.Graphics.MotionBlur.Strength", 0);
          } else {
              !R_MOTION_BLUR_ENABLED;
          }
      }) },
    { MENU_ITEM_MOTION_BLUR_STRENGTH,
      "Strength",
      "gEnhancements.Graphics.MotionBlur.Strength",
      "Motion Blur strength.",
      WIDGET_SLIDER_INT,
      { 0, 255, 180 } },
    { MENU_ITEM_AUTHENTIC_LOGO,
      "Authentic Logo",
      "gEnhancements.Graphics.AuthenticLogo",
      "Hide the game version and build details and display the authentic "
      "model and texture on the boot logo start screen",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_BOW_RETICLE,
      "Bow Reticle",
      "gEnhancements.Graphics.BowReticle",
      "Gives the bow a reticle when you draw an arrow.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_DISABLE_BLACK_BARS,
      "Disable Black Bar Letterboxes",
      "gEnhancements.Graphics.DisableBlackBars",
      "Disables Black Bar Letterboxes during cutscenes and Z-targeting\nNote: there may be "
      "minor visual glitches that were covered up by the black bars\nPlease disable this "
      "setting before reporting a bug.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_GEOMETRY_DISTANCE_CHECK,
      "Disable Scene Geometry Distance Check",
      "gEnhancements.Graphics.DisableSceneGeometryDistanceCheck",
      "Disables the distance check for scene geometry, allowing it to be drawn no matter how far "
      "away it is from the player. This may have unintended side effects.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_ACTOR_DRAW_DISTANCE,
      "Increase Actor Draw Distance: %dx",
      "gEnhancements.Graphics.IncreaseActorDrawDistance",
      "Increase the range in which Actors are drawn. This may have unintended side effects.",
      WIDGET_SLIDER_INT,
      { 1, 5, 1 },
      ([]() {
          CVarSetInteger("gEnhancements.Graphics.IncreaseActorUpdateDistance",
                         MIN(CVarGetInteger("gEnhancements.Graphics.IncreaseActorDrawDistance", 1),
                             CVarGetInteger("gEnhancements.Graphics.IncreaseActorUpdateDistance", 1)));
      }) },
    { MENU_ITEM_ACTOR_UPDATE_DISTANCE,
      "Increase Actor Update Distance: %dx",
      "gEnhancements.Graphics.IncreaseActorUpdateDistance",
      "Increase the range in which Actors are updated. This may have unintended side effects.",
      WIDGET_SLIDER_INT,
      { 1, 5, 1 },
      ([]() {
          CVarSetInteger("gEnhancements.Graphics.IncreaseActorDrawDistance",
                         MAX(CVarGetInteger("gEnhancements.Graphics.IncreaseActorDrawDistance", 1),
                             CVarGetInteger("gEnhancements.Graphics.IncreaseActorUpdateDistance", 1)));
      }) },
    // Mask Enhancements
    { MENU_ITEM_BLAST_MASK_KEG_FORCE,
      "Blast Mask has Powder Keg Force",
      "gEnhancements.Masks.BlastMaskKeg",
      "Blast Mask can also destroy objects only the Powder Keg can.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_FAST_TRANSFORMATION,
      "Fast Transformation",
      "gEnhancements.Masks.FastTransformation",
      "Removes the delay when using transormation masks.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_FIERCE_DEITY_ANYWHERE,
      "Fierce Deity's Mask Anywhere",
      "gEnhancements.Masks.FierceDeitysAnywhere",
      "Allow using Fierce Deity's mask outside of boss rooms.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_NO_BLAST_MASK_COOLDOWN,
      "No Blast Mask Cooldown",
      "gEnhancements.Masks.NoBlastMaskCooldown",
      "Eliminates the Cooldown between Blast Mask usage.",
      WIDGET_CHECKBOX,
      {} },
    // Song Enhancements
    { MENU_ITEM_ENABLE_SUNS_SONG,
      "Enable Sun's Song",
      "gEnhancements.Songs.EnableSunsSong",
      "Enables the partially implemented Sun's Song. RIGHT-DOWN-UP-RIGHT-DOWN-UP to play it. "
      "This song will make time move very fast until either Link moves to a different scene, "
      "or when the time switches to a new time period.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_DPAD_OCARINA,
      "Dpad Ocarina",
      "gEnhancements.Playback.DpadOcarina",
      "Enables using the Dpad for Ocarina playback.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_PREVENT_DROPPED_OCARINA_INPUTS,
      "Prevent Dropped Ocarina Inputs",
      "gEnhancements.Playback.NoDropOcarinaInput",
      "Prevent dropping inputs when playing the ocarina quickly.",
      WIDGET_CHECKBOX,
      {} },
    // Cutscene Skips
    { MENU_ITEM_HIDE_TITLE_CARDS,
      "Hide Title Cards",
      "gEnhancements.Cutscenes.HideTitleCards",
      "Hides Title Cards when entering areas.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_SKIP_ENTRANCE_CUTSCENES,
      "Skip Entrance Cutscenes",
      "gEnhancements.Cutscenes.SkipEntranceCutscenes",
      "Skip cutscenes that occur when first entering a new area.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_SKIP_TO_FILE_SELECT,
      "Skip to File Select",
      "gEnhancements.Cutscenes.SkipToFileSelect",
      "Skip the opening title sequence and go straight to the file select menu after boot.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_SKIP_INTRO_SEQUENCE,
      "Skip Intro Sequence",
      "gEnhancements.Cutscenes.SkipIntroSequence",
      "When starting a game you will be taken straight to South Clock Town as Deku Link.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_SKIP_STORY_CUTSCENES,
      "Skip Story Cutscenes",
      "gEnhancements.Cutscenes.SkipStoryCutscenes",
      "Disclaimer: This doesn't do much yet, we will be progressively adding more skips over time.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_SKIP_MISC_INTERACTIONS,
      "Skip Misc Interactions",
      "gEnhancements.Cutscenes.SkipMiscInteractions",
      "Disclaimer: This doesn't do much yet, we will be progressively adding more skips over time.",
      WIDGET_CHECKBOX,
      {} },
    // Dialogue Enhancements
    { MENU_ITEM_FAST_BANK_SELECTION,
      "Fast Bank Selection",
      "gEnhancements.Dialogue.FastBankSelection",
      "Pressing the Z or R buttons while the Deposit/Withdrawl Rupees dialogue is open will set "
      "the Rupees to Links current Rupees or 0 respectively.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_FAST_TEXT,
      "Fast Text",
      "gEnhancements.Dialogue.FastText",
      "Speeds up text rendering, and enables holding of B progress to next message.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_FAST_MAGIC_ARROW_ANIM,
      "Fast Magic Arrow Equip Animation",
      "gEnhancements.Equipment.MagicArrowEquipSpeed",
      "Removes the animation for equipping Magic Arrows.",
      WIDGET_CHECKBOX,
      {} },
    // Fixes
    { MENU_ITEM_FIX_AMMO_COUNT_COLOR,
      "Fix Ammo Count Color",
      "gFixes.FixAmmoCountEnvColor",
      "Fixes a missing gDPSetEnvColor, which causes the ammo count to be "
      "the wrong color prior to obtaining magic or other conditions.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_FIX_HESS_WEIRDSHOT,
      "Fix Hess and Weirdshot Crash",
      "gEnhancements.Fixes.HessCrash",
      "Fixes a crash that can occur when performing a HESS or Weirdshot.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_FIX_TEXT_CONTROL_CHAR,
      "Fix Text Control Characters",
      "gEnhancements.Fixes.ControlCharacters",
      "Fixes certain control characters not functioning properly "
      "depending on their position within the text.",
      WIDGET_CHECKBOX,
      {} },
    // Restorations
    { MENU_ITEM_RESTORE_DISTANCE_FLIPS_HOPS,
      "Constant Distance Backflips and Sidehops",
      "gEnhancements.Restorations.ConstantFlipsHops",
      "Backflips and Sidehops travel a constant distance as they did in OoT.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_RESTORE_POWER_CROUCH_STAB,
      "Power Crouch Stab",
      "gEnhancements.Restorations.PowerCrouchStab",
      "Crouch stabs will use the power of Link's previous melee attack, as is in MM JP 1.0 and OoT.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_RESTORE_SIDEROLLS,
      "Side Rolls",
      "gEnhancements.Restorations.SideRoll",
      "Restores side rolling from OoT.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_RESTORE_TATL_ISG,
      "Tatl ISG",
      "gEnhancements.Restorations.TatlISG",
      "Restores Navi ISG from OoT, but now with Tatl.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_MODERN_MENU_POPOUT,
      "Popout Menu",
      "gSettings.Menu.Popout",
      "Changes the menu display from overlay to windowed.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_OPEN_APP_FILES,
      "Open App Files Folder",
      "",
      "Opens the folder that contains the save and mods folders, etc,",
      WIDGET_BUTTON,
      {},
      ([]() {
          std::string filesPath = Ship::Context::GetInstance()->GetAppDirectoryPath();
          SDL_OpenURL(std::string("file:///" + std::filesystem::absolute(filesPath).string()).c_str());
      }) },
    { MENU_ITEM_DEBUG_MODE_ENABLE,
      "Debug Mode",
      "gDeveloperTools.DebugEnabled",
      "Enables Debug Mode, allowing you to select maps with L + R + Z.",
      WIDGET_CHECKBOX,
      {},
      ([]() {
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
      }) },
    { MENU_ITEM_DEBUG_BETTER_MAP_SELECT,
      "Better Map Select",
      "gDeveloperTools.BetterMapSelect.Enabled",
      "Overrides the original map select with a translated, more user-friendly version.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_DEBUG_SAVE_FILE_MODE,
      "Debug Save File Mode",
      "gDeveloperTools.DebugSaveFileMode",
      "Change the behavior of creating saves while debug mode is enabled:\n\n"
      "- Empty Save: The default 3 heart save file in first cycle\n"
      "- Vanilla Debug Save: Uses the title screen save info (8 hearts, all items and masks)\n"
      "- 100\% Save: All items, equipment, mask, quast status and bombers notebook complete",
      WIDGET_COMBOBOX,
      { 0, 0, 0, debugSaveOptions },
      ([]() { RegisterDebugSaveCreate(); }) },
    { MENU_ITEM_PREVENT_ACTOR_UPDATE,
      "Prevent Actor Update",
      "gDeveloperTools.PreventActorUpdate",
      "Prevents Actors from updating.",
      WIDGET_CHECKBOX,
      {},
      ([]() { RegisterPreventActorUpdateHooks(); }) },
    { MENU_ITEM_PREVENT_ACTOR_DRAW,
      "Prevent Actor Draw",
      "gDeveloperTools.PreventActorDraw",
      "Prevents Actors from drawing.",
      WIDGET_CHECKBOX,
      {},
      ([]() { RegisterPreventActorDrawHooks(); }) },
    { MENU_ITEM_PREVENT_ACTOR_INIT,
      "Prevent Actor Init",
      "gDeveloperTools.PreventActorInit",
      "Prevents Actors from initializing.",
      WIDGET_CHECKBOX,
      {},
      ([]() { RegisterPreventActorInitHooks(); }) },
    { MENU_ITEM_DISABLE_OBJECT_DEPENDECY,
      "Disable Object Dependency",
      "gDeveloperTools.DisableObjectDependency",
      "Disables dependencies when loading objects.",
      WIDGET_CHECKBOX,
      {} },
    { MENU_ITEM_DEBUG_LOG_LEVEL,
      "Log Level",
      "gDeveloperTools.LogLevel",
      "The log level determines which messages are printed to the "
      "console. This does not affect the log file output",
      WIDGET_COMBOBOX,
      { 0, 0, 0, logLevels },
      ([]() {
          Ship::Context::GetInstance()->GetLogger()->set_level(
              (spdlog::level::level_enum)CVarGetInteger("gDeveloperTools.LogLevel", 1));
      }) },
    { MENU_ITEM_FRAME_ADVANCE_ENABLE,
      "Frame Advance",
      "gDeveloperTools.FrameAdvance",
      "This allows you to advance through the game one frame at a time on command. "
      "To advance a frame, hold Z and tap R on the second controller. Holding Z "
      "and R will advance a frame every half second. You can also use the buttons below.",
      WIDGET_CHECKBOX,
      {},
      ([]() {
          if (CVarGetInteger("gDeveloperTools.FrameAdvance", 0) == 1) {
              gPlayState->frameAdvCtx.enabled = true;
          } else {
              gPlayState->frameAdvCtx.enabled = false;
          };
      }) },
    { MENU_ITEM_FRAME_ADVANCE_SINGLE, "Advance 1", "", "Advance 1 frame.", WIDGET_BUTTON, {}, ([]() {
          CVarSetInteger("gDeveloperTools.FrameAdvanceTick", 1);
      }) },
    { MENU_ITEM_FRAME_ADVANCE_HOLD, "Advance (Hold)", "", "Advance frames while the button is held.", WIDGET_BUTTON },
    // HUD Editor
    { MENU_ITEM_HUD_EDITOR_BUTTON, "HUD Editor", "gWindows.HudEditor", 
      "Enables the HUD Editor window, allowing you to modify your HUD", WIDGET_WINDOW_BUTTON },
    { MENU_ITEM_COLLISION_VIEWER_BUTTON, "Collision Viewer", "gWindows.CollisionViewer",
      "Makes collision visible on screen", WIDGET_WINDOW_BUTTON },
    { MENU_ITEM_STATS_BUTTON, "Stats", "gOpenWindows.Stats",
      "Shows the stats window, with your FPS and frametimes, and the OS you're playing on", WIDGET_WINDOW_BUTTON }
};

void SearchMenuGetItem(uint32_t index) {
    disabledTempTooltip = "This setting is disabled because: \n\n";
    disabledValue = false;
    disabledTooltip = " ";

    if (enhancementList[index].modifierFunc != nullptr) {
        enhancementList[index].activeDisables.clear();
        enhancementList[index].isHidden = false;
        enhancementList[index].modifierFunc(enhancementList[index]);
        if (enhancementList[index].isHidden) {
            return;
        }
        if (!enhancementList[index].activeDisables.empty()) {
            disabledValue = true;
            for (auto option : enhancementList[index].activeDisables) {
                disabledTempTooltip += std::string("- ") + disabledMap[option].second.reason + std::string("\n");
            }
            disabledTooltip = disabledTempTooltip.c_str();
        }
    }

    switch (enhancementList[index].widgetType) {
        case WIDGET_CHECKBOX:
            if (UIWidgets::CVarCheckbox(enhancementList[index].widgetName, enhancementList[index].widgetCVar,
                                        {
                                            .color = menuTheme[menuThemeIndex],
                                            .tooltip = enhancementList[index].widgetTooltip,
                                            .disabled = disabledValue,
                                            .disabledTooltip = disabledTooltip,
                                            .defaultValue = static_cast<bool>(
                                                std::get<int32_t>(enhancementList[index].widgetOptions.defaultVariant)),
                                        })) {
                if (enhancementList[index].widgetCallback != nullptr) {
                    enhancementList[index].widgetCallback();
                }
            };
            break;
        case WIDGET_AUDIO_BACKEND: {
            auto currentAudioBackend = Ship::Context::GetInstance()->GetAudio()->GetAudioBackend();
            if (UIWidgets::Combobox(
                "Audio API", &currentAudioBackend, audioBackendsMap,
                { .color = menuTheme[menuThemeIndex],
                  .tooltip = enhancementList[index].widgetTooltip,
                  .disabled = Ship::Context::GetInstance()->GetAudio()->GetAvailableAudioBackends()->size() <= 1,
                  .disabledTooltip = "Only one audio API is available on this platform." })) {
                Ship::Context::GetInstance()->GetAudio()->SetAudioBackend(currentAudioBackend);
            }
        } break;
        case WIDGET_VIDEO_BACKEND: {
            if (UIWidgets::Combobox("Renderer API (Needs reload)", &configWindowBackend, availableWindowBackendsMap,
                { .color = menuTheme[menuThemeIndex],
                  .tooltip = enhancementList[index].widgetTooltip,
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
        case WIDGET_COMBOBOX:
            if (UIWidgets::CVarCombobox(enhancementList[index].widgetName, enhancementList[index].widgetCVar,
                                        enhancementList[index].widgetOptions.comboBoxOptions,
                                        {
                                            .color = menuTheme[menuThemeIndex],
                                            .tooltip = enhancementList[index].widgetTooltip,
                                            .disabled = disabledValue,
                                            .disabledTooltip = disabledTooltip,
                                            .defaultIndex = static_cast<uint32_t>(
                                                std::get<int32_t>(enhancementList[index].widgetOptions.defaultVariant)),
                                        })) {
                if (enhancementList[index].widgetCallback != nullptr) {
                    enhancementList[index].widgetCallback();
                }
            }
            break;
        case WIDGET_SLIDER_INT:
            if (UIWidgets::CVarSliderInt(enhancementList[index].widgetName, enhancementList[index].widgetCVar,
                                         std::get<int32_t>(enhancementList[index].widgetOptions.min),
                                         std::get<int32_t>(enhancementList[index].widgetOptions.max),
                                         std::get<int32_t>(enhancementList[index].widgetOptions.defaultVariant),
                                         {
                                             .color = menuTheme[menuThemeIndex],
                                             .tooltip = enhancementList[index].widgetTooltip,
                                             .disabled = disabledValue,
                                             .disabledTooltip = disabledTooltip,
                                         })) {
                if (enhancementList[index].widgetCallback != nullptr) {
                    enhancementList[index].widgetCallback();
                }
            };
            break;
        case WIDGET_SLIDER_FLOAT: {
            float floatMin = (std::get<float>(enhancementList[index].widgetOptions.min) / 100);
            float floatMax = (std::get<float>(enhancementList[index].widgetOptions.max) / 100);
            float floatDefault = (std::get<float>(enhancementList[index].widgetOptions.defaultVariant) / 100);
            if (UIWidgets::CVarSliderFloat(enhancementList[index].widgetName, enhancementList[index].widgetCVar,
                                           floatMin, floatMax, floatDefault,
                                           {
                                               .color = menuTheme[menuThemeIndex],
                                               .tooltip = enhancementList[index].widgetTooltip,
                                               .disabled = disabledValue,
                                               .disabledTooltip = disabledTooltip,
                                           })) {
                if (enhancementList[index].widgetCallback != nullptr) {
                    enhancementList[index].widgetCallback();
                }
            }
        } break;
        case WIDGET_BUTTON:
            if (UIWidgets::Button(enhancementList[index].widgetName,
                                  {
                                      .color = menuTheme[menuThemeIndex],
                                      .tooltip = enhancementList[index].widgetTooltip,
                                      .disabled = disabledValue,
                                      .disabledTooltip = disabledTooltip,
                                  })) {
                if (enhancementList[index].widgetCallback != nullptr) {
                    enhancementList[index].widgetCallback();
                }
            }
            break;
        case WIDGET_WINDOW_BUTTON: {
            auto window =
                Ship::Context::GetInstance()->GetWindow()->GetGui()->GetGuiWindow(enhancementList[index].widgetName);
            std::string buttonText = "Popout ";
            buttonText.append(enhancementList[index].widgetName);
            UIWidgets::WindowButton(
                buttonText.c_str(), enhancementList[index].widgetCVar, window,
                { .tooltip = enhancementList[index].widgetTooltip });
            if (!window->IsVisible()) {
                window->DrawElement();
            }
        } break;
        default:
            break;
    }
}
}
