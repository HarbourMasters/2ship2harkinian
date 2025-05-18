#ifndef BENMENU_H
#define BENMENU_H

#include "UIWidgets.hpp"
#include "Menu.h"
#include "2s2h/Enhancements/Enhancements.h"
#include "2s2h/DeveloperTools/DeveloperTools.h"
#include "graphic/Fast3D/backends/gfx_rendering_api.h"

namespace BenGui {

static const std::unordered_map<int32_t, const char*> menuThemeOptions = {
    { UIWidgets::Colors::Red, "Red" },
    { UIWidgets::Colors::DarkRed, "Dark Red" },
    { UIWidgets::Colors::Orange, "Orange" },
    { UIWidgets::Colors::Green, "Green" },
    { UIWidgets::Colors::DarkGreen, "Dark Green" },
    { UIWidgets::Colors::LightBlue, "Light Blue" },
    { UIWidgets::Colors::Blue, "Blue" },
    { UIWidgets::Colors::DarkBlue, "Dark Blue" },
    { UIWidgets::Colors::Indigo, "Indigo" },
    { UIWidgets::Colors::Violet, "Violet" },
    { UIWidgets::Colors::Purple, "Purple" },
    { UIWidgets::Colors::Brown, "Brown" },
    { UIWidgets::Colors::Gray, "Gray" },
    { UIWidgets::Colors::DarkGray, "Dark Gray" },
};

static const std::vector<const char*> alwaysWinDoggyraceOptions = {
    "Off",                       // ALWAYS_WIN_DOGGY_RACE_OFF
    "When owning Mask of Truth", // ALWAYS_WIN_DOGGY_RACE_MASKOFTRUTH
    "Always",                    // ALWAYS_WIN_DOGGY_RACE_ALWAYS
};

static const std::vector<const char*> cremiaRewardOptions = {
    "Vanilla", // CREMIA_REWARD_RANDOM
    "Hug",     // CREMIA_REWARD_ALWAYS_HUG
    "Rupee",   // CREMIA_REWARD_ALWAYS_RUPEE
};

static const std::vector<const char*> gibdoTradeSequenceOptions = {
    "Vanilla",  // GIBDO_TRADE_SEQUENCE_VANILLA
    "MM3D",     // GIBDO_TRADE_SEQUENCE_MM3D
    "No trade", // GIBDO_TRADE_SEQUENCE_NO_TRADE
};

static const std::vector<const char*> clockTypeOptions = {
    "Original",   // CLOCK_TYPE_ORIGINAL
    "MM3D style", // CLOCK_TYPE_3DS
    "Text only",  // CLOCK_TYPE_TEXT_BASED
};

static const std::vector<const char*> textureFilteringMap = {
    "Three-Point", // Fast::FILTER_THREE_POINT,
    "Linear",      // Fast::FILTER_LINEAR
    "None",        // Fast::FILTER_NONE
};

static const std::vector<const char*> motionBlurOptions = {
    "Dynamic (default)", // MOTION_BLUR_DYNAMIC
    "Always Off",        // MOTION_BLUR_ALWAYS_OFF
    "Always On",         // MOTION_BLUR_ALWAYS_ON
};
static const std::vector<const char*> debugSaveOptions = {
    "100% save",          // DEBUG_SAVE_INFO_COMPLETE
    "Vanilla debug save", // DEBUG_SAVE_INFO_VANILLA_DEBUG
    "Empty save",         // DEBUG_SAVE_INFO_NONE
};

static const std::vector<const char*> logLevels = {
    "Trace",    // DEBUG_LOG_TRACE
    "Debug",    // DEBUG_LOG_DEBUG
    "Info",     // DEBUG_LOG_INFO
    "Warn",     // DEBUG_LOG_WARN
    "Error",    // DEBUG_LOG_ERROR
    "Critical", // DEBUG_LOG_CRITICAL
    "Off",      // DEBUG_LOG_OFF
};

static const std::vector<const char*> timeStopOptions = {
    "Off",                     // TIME_STOP_OFF
    "Temples",                 // TIME_STOP_TEMPLES
    "Temples + Mini Dungeons", // TIME_STOP_TEMPLES_DUNGEONS
};

static const std::vector<const char*> notificationPosition = {
    "Top Left", "Top Right", "Bottom Left", "Bottom Right", "Hidden",
};

static const std::vector<const char*> dekuGuardSearchBallsOptions = {
    "Never",      // DEKU_GUARD_SEARCH_BALLS_NEVER
    "Night Only", // DEKU_GUARD_SEARCH_BALLS_NIGHT_ONLY
    "Always",     // DEKU_GUARD_SEARCH_BALLS_ALWAYS
};

static const std::vector<const char*> skipGetItemCutscenesOptions = {
    "Never",
    "Junk Items Only",
    "Everything But Major",
    "Always",
};

static const std::vector<const char*> powerCrouchStabOptions = {
    "Patched (US/EU)",
    "Unpatched (JP)",
    "Unpatched (OoT)",
};

static const std::unordered_map<int32_t, const char*> damageMultiplierOptions = {
    { 0, "1x" }, { 1, "2x" }, { 2, "4x" }, { 3, "8x" }, { 4, "16x" }, { 10, "1 Hit KO" },
};

class BenMenu : public Ship::Menu {
  public:
    BenMenu(const std::string& consoleVariable, const std::string& name);

    void InitElement() override;
    void DrawElement() override;
    void UpdateElement() override;
    void Draw() override;

    void AddSidebarEntry(std::string sectionName, std::string sidbarName, uint32_t columnCount);
    WidgetInfo& AddWidget(WidgetPath& pathInfo, std::string widgetName, WidgetType widgetType);
    void AddSettings();
    void AddEnhancements();
    void AddDevTools();
};
} // namespace BenGui

#endif // BENMENU_H
