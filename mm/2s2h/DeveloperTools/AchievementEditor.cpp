#include "AchievementEditor.h"
#include "2s2h/BenGui/UIWidgets.hpp"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/BenGui/Notification.h"
#include <spdlog/spdlog.h>

void AchievementEditorWindow::InitElement() {
    // Initialize any necessary state here
}

void AchievementEditorWindow::DrawElement() {
    if (ImGui::BeginTabBar("AchievementEditorTabBar", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton)) {
        if (ImGui::BeginTabItem("Achievement List")) {
            DrawAchievementList();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Controls")) {
            DrawAchievementControls();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Stats")) {
            DrawAchievementStats();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}

void AchievementEditorWindow::DrawAchievementList() {
    if (!AchievementSystem::Instance) {
        ImGui::Text("Achievement system not initialized");
        return;
    }

    const auto& achievements = AchievementSystem::Instance->GetAchievements();
    static std::shared_ptr<Achievement> selectedAchievement = nullptr;

    // Split view into two columns
    ImGui::Columns(2, "achievementColumns", true);

    // Left column - Achievement list
    ImGui::BeginChild("achievementList", ImVec2(0, 0), true);
    for (const auto& achievement : achievements) {
        bool isSelected = (selectedAchievement == achievement);
        if (ImGui::Selectable(achievement->name.c_str(), isSelected)) {
            selectedAchievement = achievement;
        }
    }
    ImGui::EndChild();

    // Right column - Achievement details
    ImGui::NextColumn();
    ImGui::BeginChild("achievementDetails", ImVec2(0, 0), true);
    if (selectedAchievement) {
        DrawAchievementDetails(selectedAchievement);
    } else {
        ImGui::Text("Select an achievement to view details");
    }
    ImGui::EndChild();

    ImGui::Columns(1);
}

void AchievementEditorWindow::DrawAchievementDetails(std::shared_ptr<Achievement> achievement) {
    ImGui::Text("ID: %s", achievement->id.c_str());
    ImGui::Text("Name: %s", achievement->name.c_str());
    ImGui::Text("Description: %s", achievement->description.c_str());
    ImGui::Text("State: %s", achievement->state == AchievementState::UNLOCKED ? "Unlocked" : "Locked");
    ImGui::Text("Secret: %s", achievement->isSecret ? "Yes" : "No");
    ImGui::Text("Gamerscore: %d", achievement->gamerscore);

    ImGui::Separator();

    // Achievement state controls
    if (achievement->state == AchievementState::LOCKED) {
        if (ImGui::Button("Unlock Achievement")) {
            AchievementSystem::Instance->UnlockAchievement(achievement->id);
        }
    } else {
        if (ImGui::Button("Lock Achievement")) {
            achievement->state = AchievementState::LOCKED;
        }
    }

    // Test notification
    if (ImGui::Button("Test Notification")) {
        AchievementSystem::Instance->ShowEnhancedNotification(achievement);
    }
}

void AchievementEditorWindow::DrawAchievementControls() {
    if (!AchievementSystem::Instance) {
        ImGui::Text("Achievement system not initialized");
        return;
    }

    ImGui::Text("Achievement System Controls");
    ImGui::Separator();

    // Mass unlock/lock controls
    if (ImGui::Button("Unlock All Achievements")) {
        const auto& achievements = AchievementSystem::Instance->GetAchievements();
        for (const auto& achievement : achievements) {
            if (achievement->state == AchievementState::LOCKED) {
                AchievementSystem::Instance->UnlockAchievement(achievement->id);
            }
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Lock All Achievements")) {
        const auto& achievements = AchievementSystem::Instance->GetAchievements();
        for (const auto& achievement : achievements) {
            achievement->state = AchievementState::LOCKED;
        }
    }

    ImGui::Separator();

    // Notification test controls
    static AchievementNotificationType notificationType = AchievementNotificationType::ENHANCED;
    const char* notificationTypes[] = { "Simple", "Enhanced" };
    if (ImGui::Combo("Notification Type", (int*)&notificationType, notificationTypes,
                     IM_ARRAYSIZE(notificationTypes))) {
        // Update notification type if needed
    }

    static char testAchievementName[128] = "Test Achievement";
    ImGui::InputText("Test Achievement Name", testAchievementName, IM_ARRAYSIZE(testAchievementName));

    if (ImGui::Button("Test Notification")) {
        if (notificationType == AchievementNotificationType::SIMPLE) {
            AchievementSystem::Instance->ShowNotification(testAchievementName);
        } else {
            // Create a temporary achievement for testing
            auto testAchievement = std::make_shared<Achievement>("test_achievement", testAchievementName,
                                                                 "This is a test achievement", "", false, 0);
            AchievementSystem::Instance->ShowEnhancedNotification(testAchievement);
        }
    }
}

void AchievementEditorWindow::DrawAchievementStats() {
    if (!AchievementSystem::Instance) {
        ImGui::Text("Achievement system not initialized");
        return;
    }

    const auto& achievements = AchievementSystem::Instance->GetAchievements();
    size_t totalAchievements = achievements.size();
    size_t unlockedAchievements = AchievementSystem::Instance->GetUnlockedAchievementsCount();
    size_t lockedAchievements = totalAchievements - unlockedAchievements;

    ImGui::Text("Achievement Statistics");
    ImGui::Separator();

    ImGui::Text("Total Achievements: %zu", totalAchievements);
    ImGui::Text("Unlocked Achievements: %zu", unlockedAchievements);
    ImGui::Text("Locked Achievements: %zu", lockedAchievements);

    // Calculate completion percentage
    float completionPercentage =
        totalAchievements > 0 ? (static_cast<float>(unlockedAchievements) / totalAchievements) * 100.0f : 0.0f;
    ImGui::Text("Completion: %.1f%%", completionPercentage);

    // Progress bar
    ImGui::ProgressBar(completionPercentage / 100.0f, ImVec2(-1.0f, 0.0f));

    ImGui::Separator();

    // List of achievements by state
    if (ImGui::CollapsingHeader("Unlocked Achievements")) {
        for (const auto& achievement : achievements) {
            if (achievement->state == AchievementState::UNLOCKED) {
                ImGui::BulletText("%s", achievement->name.c_str());
            }
        }
    }

    if (ImGui::CollapsingHeader("Locked Achievements")) {
        for (const auto& achievement : achievements) {
            if (achievement->state == AchievementState::LOCKED) {
                ImGui::BulletText("%s", achievement->name.c_str());
            }
        }
    }
}