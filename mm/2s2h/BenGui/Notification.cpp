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

static uint32_t nextId = 0;
static std::vector<Options> notifications = {};

void Window::Draw() {
    auto vp = ImGui::GetMainViewport();

    const float margin = 30.0f;
    const float padding = 10.0f;

    int position = CVarGetInteger("gNotifications.Position", 3);

    // Calculate base position exactly as in legacy code.
    ImVec2 basePosition;
    switch (position) {
        case 0: // Top Left
            basePosition = ImVec2(vp->Pos.x + margin, vp->Pos.y + margin);
            break;
        case 1: // Top Right
            basePosition = ImVec2(vp->Pos.x + vp->Size.x - margin, vp->Pos.y + margin);
            break;
        case 2: // Bottom Left
            basePosition = ImVec2(vp->Pos.x + margin, vp->Pos.y + vp->Size.y - margin);
            break;
        case 3: // Bottom Right
            basePosition = ImVec2(vp->Pos.x + vp->Size.x - margin, vp->Pos.y + vp->Size.y - margin);
            break;
        case 4: // Hidden
            return;
    }

    // Push the legacy style settings for default notifications.
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, CVarGetFloat("gNotifications.BgOpacity", 0.5f)));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f * CVarGetFloat("gNotifications.Size", 1.8f), 6.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f * CVarGetFloat("gNotifications.Size", 1.8f), 8.0f));

    // Process each notification.
    for (int index = 0; index < notifications.size(); ++index) {
        auto& notification = notifications[index];
        int inverseIndex = -ABS(index - (notifications.size() - 1));

        ImVec2 notificationPos;
        if (notification.style == NotificationStyle::ENHANCED) {
            // For enhanced notifications, use fixed dimensions.
            const float enhancedWidth = 300.0f;  // New fixed width for enhanced notification.
            const float enhancedHeight = 150.0f; // New fixed height for spacing calculations.

            switch (position) {
                case 0: // Top Left
                    notificationPos =
                        ImVec2(basePosition.x, basePosition.y + ((enhancedHeight + padding) * inverseIndex));
                    break;
                case 1: // Top Right
                    notificationPos = ImVec2(basePosition.x - enhancedWidth,
                                             basePosition.y + ((enhancedHeight + padding) * inverseIndex));
                    break;
                case 2: // Bottom Left
                    notificationPos =
                        ImVec2(basePosition.x, basePosition.y - ((enhancedHeight + padding) * (inverseIndex + 1)));
                    break;
                case 3: // Bottom Right
                    notificationPos = ImVec2(basePosition.x - enhancedWidth,
                                             basePosition.y - ((enhancedHeight + padding) * (inverseIndex + 1)));
                    break;
                default:
                    notificationPos = basePosition;
                    break;
            }
            DrawEnhancedNotification(notification, notificationPos);
        } else {
            // Default branch: preserving legacy notification behavior.
            ImGui::SetNextWindowViewport(vp->ID);
            if (notification.remainingTime < 4.0f) {
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, (notification.remainingTime - 1) / 3.0f);
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

            ImGui::SetWindowFontScale(CVarGetFloat("gNotifications.Size", 1.8f));

            ImVec2 currentWinSize = ImGui::GetWindowSize();
            switch (position) {
                case 0: // Top Left
                    notificationPos =
                        ImVec2(basePosition.x, basePosition.y + ((currentWinSize.y + padding) * inverseIndex));
                    break;
                case 1: // Top Right
                    notificationPos = ImVec2(basePosition.x - currentWinSize.x,
                                             basePosition.y + ((currentWinSize.y + padding) * inverseIndex));
                    break;
                case 2: // Bottom Left
                    notificationPos =
                        ImVec2(basePosition.x, basePosition.y - ((currentWinSize.y + padding) * (inverseIndex + 1)));
                    break;
                case 3: // Bottom Right
                    notificationPos = ImVec2(basePosition.x - currentWinSize.x,
                                             basePosition.y - ((currentWinSize.y + padding) * (inverseIndex + 1)));
                    break;
            }
            ImGui::SetWindowPos(notificationPos);
            ImGui::AlignTextToFramePadding();

            if (notification.itemIcon != nullptr) {
                ImGui::Image(
                    Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(notification.itemIcon),
                    ImVec2(22 * CVarGetFloat("gNotifications.Size", 1.8f),
                           22 * CVarGetFloat("gNotifications.Size", 1.8f)));
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
        // Remove expired notifications.
        if (notification.remainingTime <= 0) {
            if (notification.style == NotificationStyle::ENHANCED) {}
            notifications.erase(notifications.begin() + index);
            --index;
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
        if (notification.animationProgress < 0.2f) {
            alpha = notification.animationProgress / 0.2f;
            slideOffset = (1.0f - (notification.animationProgress / 0.2f)) * 30.0f; // Reduced slide distance.
        } else if (notification.remainingTime < 1.5f) {
            alpha = notification.remainingTime / 1.5f;
        }
    } else {
        if (notification.remainingTime < 1.0f) {
            alpha = notification.remainingTime;
        }
    }

    // Adjust the position for animation:
    // For right-aligned notifications (positions 1 and 3) we add the slide offset,
    // so the notification slides in from the right.
    // For left-aligned notifications (positions 0 and 2) we subtract the slide offset.
    ImVec2 animatedPos = notificationPos;
    if (position == 1 || position == 3) {
        animatedPos.x += slideOffset - 50.0f; // Subtract 50.0f, somehow the notification is too far to the right.
    } else {
        animatedPos.x -= slideOffset;
    }

    // Set up style settings.
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 0.9f)); // Dark background.
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.85f, 0.0f, 1.0f));  // Achievement Gold.
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 10.0f));

    // Use a fixed width; adjust if needed.
    const float enhancedWidth = 350.0f;
    ImGui::SetNextWindowSize(ImVec2(enhancedWidth, 0)); // Auto–height with larger width.

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
        float iconSize = 48.0f;
        ImGui::Image(Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(notification.itemIcon),
                     ImVec2(iconSize, iconSize));
        ImGui::SameLine();
    }

    // Draw the text column.
    ImGui::BeginGroup();
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]); // Use a larger font.
    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "%s", notification.prefix.c_str());
    ImGui::PopFont();

    ImGui::TextWrapped("%s", notification.message.c_str());

    if (!notification.suffix.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "%s", notification.suffix.c_str());
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
            notification.animationProgress += deltaTime * 1.5f;
            if (notification.animationProgress > 1.0f) {
                notification.animationProgress = 1.0f;
            }
        }

        // Decrement remaining time.
        notification.remainingTime -= deltaTime;

        // Remove expired notifications.
        if (notification.remainingTime <= 0) {
            if (notification.style == NotificationStyle::ENHANCED) {}
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
    notification.prefixColor = ImVec4(1.0f, 0.85f, 0.0f, 1.0f); // Achievement Gold.
    notification.message = achievementName;
    notification.messageColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White.

    if (gamerscore > 0) {
        notification.suffix = std::to_string(gamerscore) + "G";
        notification.suffixColor = ImVec4(1.0f, 0.85f, 0.0f, 1.0f); // Achievement Gold.
    }

    notification.remainingTime = CVarGetFloat("gNotifications.Duration", 10.0f);

    // Show notification immediately
    notifications.push_back(notification);
    AudioSfx_PlaySfx(NA_SE_SY_CORRECT_CHIME, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                     &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
}

bool IsAchievementNotificationActive() {
    for (const auto& notification : notifications) {
        if (notification.style == NotificationStyle::ENHANCED && !notification.prefix.empty() &&
            notification.prefix == "Achievement Unlocked") {
            return true;
        }
    }
    return false;
}

} // namespace Notification
