#include "AchievementEditor.h"
#include "2s2h/BenGui/UIWidgets.hpp"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/BenGui/Notification.h"
#include <spdlog/spdlog.h>

extern "C" {
#include <variables.h> // Include for gSaveContext
}

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

        if (ImGui::BeginTabItem("Diagnostics")) {
            DrawDiagnostics();
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
        AchievementSystem::Instance->ShowEnhancedNotification(achievement, false);
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
            AchievementSystem::Instance->ShowEnhancedNotification(testAchievement, false);
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

void AchievementEditorWindow::DrawDiagnostics() {
    if (!AchievementSystem::Instance) {
        ImGui::Text("Achievement system not initialized");
        return;
    }

    ImGui::Text("Achievement System Diagnostics");
    ImGui::Separator();

    // Determine if we're in randomizer mode
    bool isRandomizerMode = IS_RANDO;

    // Show randomizer status
    ImGui::Text("Randomizer Mode: %s", isRandomizerMode ? "TRUE" : "FALSE");
    ImGui::Text("IS_RANDO Macro: %s", IS_RANDO ? "TRUE" : "FALSE");
    ImGui::Text("Save Type: %d", gSaveContext.save.shipSaveInfo.saveType);

    ImGui::Separator();

    // Show achievement counts by category
    const auto& achievements = AchievementSystem::Instance->GetAchievements();
    size_t totalAchievements = achievements.size();

    auto bothAchievements = AchievementSystem::Instance->GetAchievementsByCategory(AchievementCategory::BOTH);
    auto vanillaAchievements = AchievementSystem::Instance->GetAchievementsByCategory(AchievementCategory::VANILLA);
    auto randomizerAchievements =
        AchievementSystem::Instance->GetAchievementsByCategory(AchievementCategory::RANDOMIZER);

    ImGui::Text("Total Achievements: %zu", totalAchievements);
    ImGui::Text("BOTH Category: %zu", bothAchievements.size());
    ImGui::Text("VANILLA Category: %zu", vanillaAchievements.size());
    ImGui::Text("RANDOMIZER Category: %zu", randomizerAchievements.size());

    // Show visible achievements count
    size_t visibleCount = 0;
    for (const auto& achievement : achievements) {
        if (AchievementSystem::Instance->IsAchievementRelevantForGameMode(achievement->id, isRandomizerMode)) {
            visibleCount++;
        }
    }
    ImGui::Text("Achievements Visible: %zu", visibleCount);

    ImGui::Separator();

    // Detailed achievement status (collapsible)
    if (ImGui::CollapsingHeader("Randomizer Achievements Status")) {
        ImGui::Indent(10.0f);

        ImGui::Columns(3, "rando_achievements_columns", true);
        ImGui::Text("Achievement ID");
        ImGui::NextColumn();
        ImGui::Text("Status");
        ImGui::NextColumn();
        ImGui::Text("Relevant");
        ImGui::NextColumn();
        ImGui::Separator();

        for (const auto& achievement : randomizerAchievements) {
            ImGui::Text("%s", achievement->id.c_str());
            ImGui::NextColumn();
            ImGui::Text("%s", achievement->state == AchievementState::UNLOCKED ? "Unlocked" : "Locked");
            ImGui::NextColumn();
            ImGui::Text("%s",
                        AchievementSystem::Instance->IsAchievementRelevantForGameMode(achievement->id, isRandomizerMode)
                            ? "Yes"
                            : "No");
            ImGui::NextColumn();
        }

        ImGui::Columns(1);
        ImGui::Unindent(10.0f);
    }

    // Show info about randomizer checks
    if (ImGui::CollapsingHeader("Randomizer Checks Status")) {
        ImGui::Indent(10.0f);

        // Display first few check flags
        ImGui::Text("RANDO_SAVE_CHECKS Initialized: %s", IS_RANDO ? "Yes" : "No");

        if (IS_RANDO) {
            int obtainedChecks = 0;
            for (size_t i = 0; i < RC_MAX && i < 50; i++) { // Limit to first 50 for performance
                if (RANDO_SAVE_CHECKS[i].obtained) {
                    obtainedChecks++;
                }
            }
            ImGui::Text("First 50 Checks - Obtained: %d", obtainedChecks);

            if (ImGui::CollapsingHeader("Specific Rando Achievement Conditions")) {
                // Show specific checks for certain achievements
                bool firstItemCondition = false;
                for (size_t i = 0; i < RC_MAX; i++) {
                    if (RANDO_SAVE_CHECKS[i].obtained) {
                        firstItemCondition = true;
                        break;
                    }
                }
                ImGui::Text("rando_first_item condition met: %s", firstItemCondition ? "Yes" : "No");

                // Check if all masks are collected for rando_all_masks
                bool allMasksCondition = true;
                for (u8 i = ITEM_MASK_DEKU; i <= ITEM_MASK_GIANT; i++) {
                    if (INV_CONTENT(i) == ITEM_NONE) {
                        allMasksCondition = false;
                        break;
                    }
                }
                ImGui::Text("rando_all_masks condition met: %s", allMasksCondition ? "Yes" : "No");

                // Check if any transformation mask is in inventory for rando_playas
                bool transformMaskCondition =
                    (INV_CONTENT(ITEM_MASK_DEKU) == ITEM_MASK_DEKU || INV_CONTENT(ITEM_MASK_GORON) == ITEM_MASK_GORON ||
                     INV_CONTENT(ITEM_MASK_ZORA) == ITEM_MASK_ZORA);
                ImGui::Text("rando_playas condition met: %s", transformMaskCondition ? "Yes" : "No");

                // Check location count for rando_impossible
                int checkedLocations = 0;
                for (size_t i = 0; i < RC_MAX; i++) {
                    if (RANDO_SAVE_CHECKS[i].obtained) {
                        checkedLocations++;
                    }
                }
                ImGui::Text("rando_impossible locations found: %d/100", checkedLocations);

                // Check boss rush condition for rando_first_try
                bool hasAllRemains = CHECK_QUEST_ITEM(QUEST_REMAINS_ODOLWA) && CHECK_QUEST_ITEM(QUEST_REMAINS_GOHT) &&
                                     CHECK_QUEST_ITEM(QUEST_REMAINS_GYORG) && CHECK_QUEST_ITEM(QUEST_REMAINS_TWINMOLD);
                bool isFirstCycle = gSaveContext.save.isFirstCycle;
                ImGui::Text("rando_first_try (has all remains): %s", hasAllRemains ? "Yes" : "No");
                ImGui::Text("rando_first_try (is first cycle): %s", isFirstCycle ? "Yes" : "No");
                ImGui::Text("rando_first_try condition met: %s", (hasAllRemains && isFirstCycle) ? "Yes" : "No");
            }
        }

        ImGui::Unindent(10.0f);
    }
}