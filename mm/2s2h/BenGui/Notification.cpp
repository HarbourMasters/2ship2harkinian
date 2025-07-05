#include "Notification.h"
#include <libultraship/libultraship.h>
#include <algorithm>

extern "C" {
#include "functions.h"
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

    // Process each notification
    for (int index = 0; index < notifications.size(); ++index) {
        auto& notification = notifications[index];
        int count = static_cast<int>(notifications.size());
        int inverseIndex = -(count - 1 - index);

        if (notification.isAchievement) {
            // Enhanced layout for achievements
            DrawEnhancedNotification(notification, basePosition, inverseIndex, position, padding);
        } else {
            // Original simple layout for regular notifications
            DrawRegularNotification(notification, basePosition, inverseIndex, position, padding, vp);
        }
    }
}

void Window::DrawRegularNotification(const Options& notification, ImVec2 basePosition, int inverseIndex, int position,
                                     float padding, ImGuiViewport* vp) {
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, CVarGetFloat("gNotifications.BgOpacity", 0.5f)));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f * CVarGetFloat("gNotifications.Size", 1.8f), 6.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f * CVarGetFloat("gNotifications.Size", 1.8f), 8.0f));

    ImGui::SetNextWindowViewport(vp->ID);
    if (notification.remainingTime < 4.0f) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, (notification.remainingTime - 1) / 3.0f);
    } else {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
    }

    std::string windowName = "notification#" + std::to_string(notification.id);
    ImGui::Begin(windowName.c_str(), nullptr,
                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing |
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoScrollbar);

    ImGui::SetWindowFontScale(CVarGetFloat("gNotifications.Size", 1.8f));

    ImVec2 currentWinSize = ImGui::GetWindowSize();
    ImVec2 notificationPos;
    switch (position) {
        case 0: // Top Left
            notificationPos = ImVec2(basePosition.x, basePosition.y + ((currentWinSize.y + padding) * inverseIndex));
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
        float iconSize = 22 * CVarGetFloat("gNotifications.Size", 1.8f);
        ImGui::Image(Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(notification.itemIcon),
                     ImVec2(iconSize, iconSize));
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
    ImGui::PopStyleVar(4);
    ImGui::PopStyleColor(2);
}

void Window::DrawEnhancedNotification(const Options& notification, ImVec2 basePosition, int inverseIndex, int position,
                                      float padding) {
    const float enhancedWidth = 380.0f;
    const float enhancedHeight = 100.0f; // For spacing calculations
    const float iconSize = 54.0f;
    const float textAreaWidth = enhancedWidth - iconSize - 30.0f; // Account for padding and spacing

    // Calculate position for enhanced notifications
    ImVec2 notificationPos;
    switch (position) {
        case 0: // Top Left
            notificationPos = ImVec2(basePosition.x, basePosition.y + ((enhancedHeight + padding) * inverseIndex));
            break;
        case 1: // Top Right
            notificationPos =
                ImVec2(basePosition.x - enhancedWidth, basePosition.y + ((enhancedHeight + padding) * inverseIndex));
            break;
        case 2: // Bottom Left
            notificationPos =
                ImVec2(basePosition.x, basePosition.y - ((enhancedHeight + padding) * (inverseIndex + 1)));
            break;
        case 3: // Bottom Right
            notificationPos = ImVec2(basePosition.x - enhancedWidth,
                                     basePosition.y - ((enhancedHeight + padding) * (inverseIndex + 1)));
            break;
    }

    // Enhanced styling
    ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);

    float alpha = 1.0f;
    if (notification.remainingTime < 4.0f) {
        alpha = (notification.remainingTime - 1) / 3.0f;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.12f, 0.95f)); // Darker, more opaque
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.85f, 0.0f, 1.0f));      // Gold border
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 12.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12.0f, 4.0f));

    ImGui::SetNextWindowSize(ImVec2(enhancedWidth, 0)); // Auto-height with fixed width

    std::string windowName = "enhanced_notification#" + std::to_string(notification.id);
    ImGui::Begin(windowName.c_str(), nullptr,
                 ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollWithMouse |
                     ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

    ImGui::SetWindowPos(notificationPos);

    // Layout with proper vertical centering
    if (notification.itemIcon != nullptr) {
        // Calculate estimated text content height for centering
        float lineHeight = ImGui::GetTextLineHeight();
        float largeLineHeight = lineHeight * 1.3f; // Approximate larger font height
        float spacing = ImGui::GetStyle().ItemSpacing.y;

        float totalTextHeight = 0;
        if (!notification.prefix.empty()) {
            totalTextHeight += largeLineHeight + spacing;
        }
        totalTextHeight += lineHeight; // Message line
        if (!notification.suffix.empty()) {
            totalTextHeight += spacing + lineHeight;
        }

        // Calculate vertical offset to center icon with text content
        float contentStartY = ImGui::GetCursorPosY();
        float iconOffsetY = std::max(0.0f, (totalTextHeight - iconSize) * 0.5f);

        // Position and draw icon
        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), contentStartY + iconOffsetY));
        ImGui::Image(Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(notification.itemIcon),
                     ImVec2(iconSize, iconSize));

        // Position text content next to icon
        ImGui::SameLine(0, 12.0f);
        ImGui::SetCursorPosY(contentStartY);

        ImGui::BeginGroup();
        if (!notification.prefix.empty()) {
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]); // Larger font for prefix
            ImGui::TextColored(notification.prefixColor, "%s", notification.prefix.c_str());
            ImGui::PopFont();
            ImGui::Spacing(); // Small gap between prefix and message
        }

        // Wrap text to fit in the available space
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + textAreaWidth);
        ImGui::TextColored(notification.messageColor, "%s", notification.message.c_str());
        ImGui::PopTextWrapPos();

        if (!notification.suffix.empty()) {
            ImGui::Spacing(); // Small gap before suffix
            ImGui::TextColored(notification.suffixColor, "%s", notification.suffix.c_str());
        }
        ImGui::EndGroup();

    } else {
        // No icon, just text with better spacing
        if (!notification.prefix.empty()) {
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
            ImGui::TextColored(notification.prefixColor, "%s", notification.prefix.c_str());
            ImGui::PopFont();
            ImGui::Spacing();
        }

        ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
        ImGui::TextColored(notification.messageColor, "%s", notification.message.c_str());
        ImGui::PopTextWrapPos();

        if (!notification.suffix.empty()) {
            ImGui::Spacing();
            ImGui::TextColored(notification.suffixColor, "%s", notification.suffix.c_str());
        }
    }

    ImGui::End();

    // Restore style settings
    ImGui::PopStyleVar(5);
    ImGui::PopStyleColor(2);
}

void Window::UpdateElement() {
    float deltaTime = ImGui::GetIO().DeltaTime;

    for (int index = 0; index < notifications.size(); ++index) {
        auto& notification = notifications[index];

        // Decrement remaining time.
        notification.remainingTime -= deltaTime;

        // Remove expired notifications.
        if (notification.remainingTime <= 0) {
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

void EmitAchievement(const char* iconPath, const std::string& achievementName, int gamerscore) {
    Options notification;
    notification.id = nextId++;
    notification.itemIcon = iconPath;
    notification.prefix = "Achievement Unlocked";
    notification.prefixColor = ImVec4(1.0f, 0.85f, 0.0f, 1.0f); // Gold
    notification.message = achievementName;
    notification.messageColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White

    if (gamerscore > 0) {
        notification.suffix = std::to_string(gamerscore) + "G";
        notification.suffixColor = ImVec4(1.0f, 0.85f, 0.0f, 1.0f); // Gold
    }

    notification.remainingTime = CVarGetFloat("gNotifications.Duration", 10.0f);
    notification.isAchievement = true;

    notifications.push_back(notification);
    AudioSfx_PlaySfx(NA_SE_SY_CORRECT_CHIME, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                     &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
}

void EmitAchievementProgress(const char* iconPath, const char* name, int current, int target) {
    char progressText[128];
    snprintf(progressText, sizeof(progressText), "%s: %d / %d", name, current, target);

    Options notification;
    notification.id = nextId++;
    notification.itemIcon = iconPath;
    notification.prefix = "Progress Update";
    notification.prefixColor = ImVec4(0.8f, 0.8f, 0.8f, 1.0f); // Gray
    notification.message = progressText;
    notification.messageColor = ImVec4(0.8f, 0.8f, 0.8f, 1.0f); // Gray
    notification.remainingTime = CVarGetFloat("gNotifications.Duration", 10.0f);
    notification.isAchievement = true; // Get achievement styling

    notifications.push_back(notification);
    AudioSfx_PlaySfx(NA_SE_SY_SCHEDULE_WRITE, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                     &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
}

void EmitAchievementProgressWithEvent(const char* iconPath, const char* eventName, const char* achievementName,
                                      int current, int target) {
    char progressText[128];
    snprintf(progressText, sizeof(progressText), "%s\n%s: %d / %d", eventName, achievementName, current, target);

    Options notification;
    notification.id = nextId++;
    notification.itemIcon = iconPath;
    notification.prefix = "Event Completed";
    notification.prefixColor = ImVec4(0.4f, 0.8f, 1.0f, 1.0f); // Light blue
    notification.message = progressText;
    notification.messageColor = ImVec4(0.9f, 0.9f, 0.9f, 1.0f); // Light gray
    notification.remainingTime = CVarGetFloat("gNotifications.Duration", 10.0f);
    notification.isAchievement = true; // Get achievement styling

    notifications.push_back(notification);
    AudioSfx_PlaySfx(NA_SE_SY_SCHEDULE_WRITE, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                     &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
}

bool IsNotificationActive() {
    return !notifications.empty();
}

} // namespace Notification
