#ifndef NOTIFICATION_H
#define NOTIFICATION_H
#ifdef __cplusplus

#include <string>
#include <vector>
#include <queue>
#include <cstdint>
#include <GuiWindow.h>
namespace Notification {

enum class NotificationStyle {
    DEFAULT,
    ENHANCED // Enhanced notification with animations (formerly Xbox360)
};

struct Options {
    uint32_t id = 0;
    const char* itemIcon = nullptr;
    std::string prefix = "";
    ImVec4 prefixColor = ImVec4(0.5f, 0.5f, 1.0f, 1.0f);
    std::string message = "";
    ImVec4 messageColor = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
    std::string suffix = "";
    ImVec4 suffixColor = ImVec4(1.0f, 0.5f, 0.5f, 1.0f);
    float remainingTime = 0.0f; // Seconds
    NotificationStyle style = NotificationStyle::DEFAULT;
    float animationProgress = 0.0f;                       // Used for animation (0.0 to 1.0)
    int soundId = -1;                                     // Sound to play (-1 for default)
    bool isAchievement = false;                           // Flag to specifically identify achievement notifications
    ImVec4 borderColor = ImVec4(1.0f, 0.85f, 0.0f, 1.0f); // Default to ENHANCED_BORDER_COLOR (Achievement Gold)
};

class Window : public Ship::GuiWindow {
  public:
    using GuiWindow::GuiWindow;

    void InitElement() override{};
    void DrawElement() override{};
    void Draw() override;
    void UpdateElement() override;

  private:
    void DrawDefaultNotification(const Options& notification, ImVec2 notificationPos);
    void DrawEnhancedNotification(const Options& notification, ImVec2 notificationPos);
};

void Emit(Options notification);
void EmitWithSound(Options notification, int soundId);
void EmitAchievement(const char* iconPath, const std::string& achievementName, int gamerscore);

void EmitQuestItem(const char* iconPath, const char* name);
void EmitDungeonItem(const char* iconPath, const char* name);
void EmitAchievement(const char* iconPath, const char* name, int score);
void EmitAchievementProgress(const char* iconPath, const char* name, int current, int target);

bool IsNotificationActive();

} // namespace Notification

#endif // __cplusplus
#endif // NOTIFICATION_H
