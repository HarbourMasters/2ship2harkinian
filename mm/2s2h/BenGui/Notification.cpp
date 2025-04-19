/**
 * @file Notification.cpp
 * @brief Implementation of the notification system
 *
 * This file has undergone a refactor to support multiple notification styles while preserving
 * the original simple notification behavior. The goal was to maintain backward compatibility
 * while providing a foundation for enhanced notification features. The original notification
 * system has been preserved as much as possible, with new styles being added alongside it.
 *
 * Current notification styles:
 * - DEFAULT: The original simple notification style
 * - ENHANCED: A more visually rich notification style with animations and better formatting
 */

#include "Notification.h"
#include <libultraship/libultraship.h>
#include <spdlog/spdlog.h>

extern "C" {
#include "functions.h"
#include "macros.h"
#include "variables.h"
}

namespace Notification {

// --- Constants for Styling and Animation ---
constexpr float MARGIN = 30.0f;
constexpr float PADDING = 10.0f;
constexpr float DEFAULT_ROUNDING = 4.0f;
constexpr float ENHANCED_ROUNDING = 8.0f;
constexpr float ENHANCED_BORDER_SIZE = 2.0f;
constexpr float DEFAULT_SIZE_MULTIPLIER = 1.8f; // Matches legacy CVar default
constexpr float DEFAULT_ICON_SIZE = 22.0f;
constexpr float ENHANCED_ICON_SIZE = 48.0f;
constexpr float ENHANCED_WIDTH = 350.0f;
constexpr float ENHANCED_HEIGHT_FOR_SPACING = 150.0f; // Used for vertical position calculation
constexpr float DEFAULT_FADE_OUT_START_TIME = 4.0f;
constexpr float DEFAULT_FADE_OUT_DURATION = 3.0f;
constexpr float SUBTLE_ANIM_FADE_IN_DURATION = 0.2f;
constexpr float SUBTLE_ANIM_FADE_OUT_START_TIME = 1.5f;
constexpr float SUBTLE_ANIM_SLIDE_DISTANCE = 30.0f;
constexpr float LEGACY_ANIM_FADE_OUT_START_TIME = 1.0f;
constexpr float ANIMATION_SPEED_MULTIPLIER = 1.5f;
const ImVec4 ENHANCED_BG_COLOR = ImVec4(0.1f, 0.1f, 0.1f, 0.9f);
const ImVec4 ENHANCED_BORDER_COLOR = ImVec4(1.0f, 0.85f, 0.0f, 1.0f); // Achievement Gold
const ImVec4 DEFAULT_PREFIX_COLOR = ImVec4(0.5f, 0.5f, 1.0f, 1.0f);
const ImVec4 DEFAULT_MESSAGE_COLOR = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
const ImVec4 DEFAULT_SUFFIX_COLOR = ImVec4(1.0f, 0.5f, 0.5f, 1.0f);
const ImVec4 ENHANCED_PREFIX_COLOR = ImVec4(1.0f, 0.85f, 0.0f, 1.0f); // Achievement Gold
const ImVec4 ENHANCED_MESSAGE_COLOR = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White
const ImVec4 ENHANCED_SUFFIX_COLOR = ImVec4(1.0f, 0.85f, 0.0f, 1.0f); // Achievement Gold
// --- End Constants ---

static uint32_t nextId = 0;
static std::vector<Options> notifications = {};

void Window::Draw() {
    auto vp = ImGui::GetMainViewport();

    // const float margin = 30.0f;
    // const float padding = 10.0f;

    int position = CVarGetInteger("gNotifications.Position", 3);

    // Calculate base position exactly as in legacy code.
    ImVec2 basePosition;
    switch (position) {
        case 0: // Top Left
            basePosition = ImVec2(vp->Pos.x + MARGIN, vp->Pos.y + MARGIN);
            break;
        case 1: // Top Right
            basePosition = ImVec2(vp->Pos.x + vp->Size.x - MARGIN, vp->Pos.y + MARGIN);
            break;
        case 2: // Bottom Left
            basePosition = ImVec2(vp->Pos.x + MARGIN, vp->Pos.y + vp->Size.y - MARGIN);
            break;
        case 3: // Bottom Right
            basePosition = ImVec2(vp->Pos.x + vp->Size.x - MARGIN, vp->Pos.y + vp->Size.y - MARGIN);
            break;
        case 4: // Hidden
            return;
    }

    // Push the legacy style settings for default notifications.
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, CVarGetFloat("gNotifications.BgOpacity", 0.5f)));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, DEFAULT_ROUNDING);
    // Use CVar directly for size multiplier here as it's dynamic
    float currentDefaultSizeMult = CVarGetFloat("gNotifications.Size", DEFAULT_SIZE_MULTIPLIER);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f * currentDefaultSizeMult, 6.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f * currentDefaultSizeMult, 8.0f));

    // Process each notification.
    for (int index = 0; index < notifications.size(); ++index) {
        auto& notification = notifications[index];
        int inverseIndex = -ABS(index - (notifications.size() - 1));

        ImVec2 notificationPos;
        if (notification.style == NotificationStyle::ENHANCED) {
            // For enhanced notifications, use fixed dimensions.
            // const float enhancedWidth = 300.0f;  // Use ENHANCED_WIDTH
            // const float enhancedHeight = 150.0f; // Use ENHANCED_HEIGHT_FOR_SPACING

            switch (position) {
                case 0: // Top Left
                    notificationPos = ImVec2(basePosition.x,
                                             basePosition.y + ((ENHANCED_HEIGHT_FOR_SPACING + PADDING) * inverseIndex));
                    break;
                case 1: // Top Right
                    notificationPos = ImVec2(basePosition.x - ENHANCED_WIDTH,
                                             basePosition.y + ((ENHANCED_HEIGHT_FOR_SPACING + PADDING) * inverseIndex));
                    break;
                case 2: // Bottom Left
                    notificationPos = ImVec2(basePosition.x, basePosition.y - ((ENHANCED_HEIGHT_FOR_SPACING + PADDING) *
                                                                               (inverseIndex + 1)));
                    break;
                case 3: // Bottom Right
                    notificationPos =
                        ImVec2(basePosition.x - ENHANCED_WIDTH,
                               basePosition.y - ((ENHANCED_HEIGHT_FOR_SPACING + PADDING) * (inverseIndex + 1)));
                    break;
                default:
                    notificationPos = basePosition;
                    break;
            }
            DrawEnhancedNotification(notification, notificationPos);
        } else {
            // Default branch: preserving legacy notification behavior.
            ImGui::SetNextWindowViewport(vp->ID);
            // Use constant for fade time check
            if (notification.remainingTime < DEFAULT_FADE_OUT_START_TIME) {
                // Use constants for fade calculation
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, (notification.remainingTime -
                                                          (DEFAULT_FADE_OUT_START_TIME - DEFAULT_FADE_OUT_DURATION)) /
                                                             DEFAULT_FADE_OUT_DURATION);
            } else {
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
            }
            std::string windowName = "notification#" + std::to_string(notification.id);
            ImGui::Begin(windowName.c_str(), nullptr,
                         ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav |
                             ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoScrollbar);

            // Use CVar directly for size multiplier here
            ImGui::SetWindowFontScale(CVarGetFloat("gNotifications.Size", DEFAULT_SIZE_MULTIPLIER));

            ImVec2 currentWinSize = ImGui::GetWindowSize();
            switch (position) {
                case 0: // Top Left
                    notificationPos =
                        ImVec2(basePosition.x, basePosition.y + ((currentWinSize.y + PADDING) * inverseIndex));
                    break;
                case 1: // Top Right
                    notificationPos = ImVec2(basePosition.x - currentWinSize.x,
                                             basePosition.y + ((currentWinSize.y + PADDING) * inverseIndex));
                    break;
                case 2: // Bottom Left
                    notificationPos =
                        ImVec2(basePosition.x, basePosition.y - ((currentWinSize.y + PADDING) * (inverseIndex + 1)));
                    break;
                case 3: // Bottom Right
                    notificationPos = ImVec2(basePosition.x - currentWinSize.x,
                                             basePosition.y - ((currentWinSize.y + PADDING) * (inverseIndex + 1)));
                    break;
            }
            ImGui::SetWindowPos(notificationPos);
            ImGui::AlignTextToFramePadding();

            if (notification.itemIcon != nullptr) {
                // Use CVar directly for size multiplier here, use constant for base size
                float currentIconSize =
                    DEFAULT_ICON_SIZE * CVarGetFloat("gNotifications.Size", DEFAULT_SIZE_MULTIPLIER);
                ImGui::Image(
                    Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(notification.itemIcon),
                    ImVec2(currentIconSize, currentIconSize));
                ImGui::SameLine();
            }
            if (!notification.prefix.empty()) {
                ImGui::TextColored(notification.prefixColor, "%s", notification.prefix.c_str());
                ImGui::SameLine();
            }
            ImGui::TextColored(notification.messageColor, "%s", notification.message.c_str());
            if (!notification.suffix.empty()) {
                ImGui::SameLine();
                ImGui::TextColored(notification.suffixColor, "%s", notification.suffix.c_str());
            }

            ImGui::End();
            ImGui::PopStyleVar();
        }
    }
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
}

void Window::DrawEnhancedNotification(const Options& notification, ImVec2 notificationPos) {
    // Retrieve the current position setting.
    int position = CVarGetInteger("gNotifications.Position", 3);

    ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);

    // Animation logic for enhanced notifications.
    float alpha = 1.0f;
    float slideOffset = 0.0f;
    bool useSubtleAnimations = CVarGetInteger("gNotifications.SubtleAnimations", 1) != 0;

    if (useSubtleAnimations) {
        // Use constants for animation timing
        if (notification.animationProgress < SUBTLE_ANIM_FADE_IN_DURATION) {
            alpha = notification.animationProgress / SUBTLE_ANIM_FADE_IN_DURATION;
            slideOffset =
                (1.0f - (notification.animationProgress / SUBTLE_ANIM_FADE_IN_DURATION)) * SUBTLE_ANIM_SLIDE_DISTANCE;
        } else if (notification.remainingTime < SUBTLE_ANIM_FADE_OUT_START_TIME) {
            alpha = notification.remainingTime / SUBTLE_ANIM_FADE_OUT_START_TIME;
        }
    } else {
        // Use constant for animation timing
        if (notification.remainingTime < LEGACY_ANIM_FADE_OUT_START_TIME) {
            alpha = notification.remainingTime / LEGACY_ANIM_FADE_OUT_START_TIME; // Simple fade out
        }
    }

    // Adjust the position for animation:
    // For right-aligned notifications (positions 1 and 3) we add the slide offset,
    // so the notification slides in from the right.
    // For left-aligned notifications (positions 0 and 2) we subtract the slide offset.
    ImVec2 animatedPos = notificationPos;
    // Remove magic number offset, adjust positioning if needed based on constants
    if (position == 1 || position == 3) {
        animatedPos.x += slideOffset; // Removed -50.0f adjustment, check visuals
    } else {
        animatedPos.x -= slideOffset;
    }

    // Set up style settings using constants
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ENHANCED_BG_COLOR);
    ImGui::PushStyleColor(ImGuiCol_Border, ENHANCED_BORDER_COLOR);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, ENHANCED_BORDER_SIZE);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, ENHANCED_ROUNDING);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 10.0f)); // Keep padding explicit for now

    // Use constant for width
    ImGui::SetNextWindowSize(ImVec2(ENHANCED_WIDTH, 0)); // Auto–height with larger width.

    std::string windowName = "enhanced_notification#" + std::to_string(notification.id);
    ImGui::Begin(windowName.c_str(), nullptr,
                 ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollWithMouse |
                     ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

    ImGui::SetWindowPos(animatedPos);

    // Build the layout for the enhanced notification.
    ImGui::BeginGroup();

    // Draw the icon, if available.
    if (notification.itemIcon != nullptr) {
        // Use constant for icon size
        ImGui::Image(Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(notification.itemIcon),
                     ImVec2(ENHANCED_ICON_SIZE, ENHANCED_ICON_SIZE));
        ImGui::SameLine();
    }

    // Draw the text column.
    ImGui::BeginGroup();
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]); // Use a larger font.
    // Use constant for color
    ImGui::TextColored(ENHANCED_PREFIX_COLOR, "%s", notification.prefix.c_str());
    ImGui::PopFont();

    ImGui::TextWrapped("%s", notification.message.c_str());

    if (!notification.suffix.empty()) {
        // Use constant for color
        ImGui::TextColored(ENHANCED_SUFFIX_COLOR, "%s", notification.suffix.c_str());
    }

    ImGui::EndGroup();
    ImGui::EndGroup();

    ImGui::End();

    // Restore style settings.
    ImGui::PopStyleVar(4);
    ImGui::PopStyleColor(2);
}

void Window::UpdateElement() {
    float deltaTime = ImGui::GetIO().DeltaTime;

    // First, update existing notifications
    for (int index = 0; index < notifications.size(); ++index) {
        auto& notification = notifications[index];

        // For enhanced notifications, update animation progress.
        if (notification.style == NotificationStyle::ENHANCED && notification.animationProgress < 1.0f) {
            // Use constant for animation speed
            notification.animationProgress += deltaTime * ANIMATION_SPEED_MULTIPLIER;
            if (notification.animationProgress > 1.0f) {
                notification.animationProgress = 1.0f;
            }
        }

        // Decrement remaining time.
        notification.remainingTime -= deltaTime;

        // Remove expired notifications.
        if (notification.remainingTime <= 0) {
            // No visual change needed for ENHANCED when removing based on time
            notifications.erase(notifications.begin() + index);
            --index;
        }
    }
}

void Emit(Options notification) {
    notification.id = nextId++;
    if (notification.remainingTime == 0.0f) {
        notification.remainingTime = CVarGetFloat("gNotifications.Duration", 10.0f);
    }
    notifications.push_back(notification);
    AudioSfx_PlaySfx(NA_SE_SY_METRONOME, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale,
                     &gSfxDefaultReverb);
}

void EmitWithSound(Options notification, int soundId) {
    notification.id = nextId++;
    if (notification.remainingTime == 0.0f) {
        notification.remainingTime = CVarGetFloat("gNotifications.Duration", 10.0f);
    }
    notifications.push_back(notification);

    if (soundId >= 0) {
        AudioSfx_PlaySfx(soundId, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale,
                         &gSfxDefaultReverb);
    }
}

void EmitAchievement(const char* iconPath, const std::string& achievementName, int gamerscore) {
    Options notification;
    notification.id = nextId++;
    notification.style = NotificationStyle::ENHANCED;
    notification.itemIcon = iconPath;
    notification.prefix = "Achievement Unlocked";
    notification.prefixColor = ENHANCED_PREFIX_COLOR; // Use constant
    notification.message = achievementName;
    notification.messageColor = ENHANCED_MESSAGE_COLOR; // Use constant

    if (gamerscore > 0) {
        notification.suffix = std::to_string(gamerscore) + "G";
        notification.suffixColor = ENHANCED_SUFFIX_COLOR; // Use constant
    }

    notification.remainingTime = CVarGetFloat("gNotifications.Duration", 10.0f);
    notification.isAchievement = true; // Mark this as an achievement notification

    // Show notification immediately
    notifications.push_back(notification);
    AudioSfx_PlaySfx(NA_SE_SY_CORRECT_CHIME, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                     &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
}

bool IsAchievementNotificationActive() {
    for (const auto& notification : notifications) {
        // Check the dedicated flag instead of style/prefix
        if (notification.isAchievement) {
            return true;
        }
    }
    return false;
}

} // namespace Notification
