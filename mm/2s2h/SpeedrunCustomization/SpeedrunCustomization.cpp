#include "2s2h/BenGui/UIWidgets.hpp"

extern "C" {
#include "z64item.h"
#include "variables.h"
}

#define INTRO_GIVE_NUTS_CVAR_NAME "gSpeedrun.CutsceneSkipBehavior.Intro.GrantDekuNuts"
#define FIRST_CYCLE_GIVE_BOMB_BAG_CVAR_NAME "gSpeedrun.CutsceneSkipBehavior.FirstCycle.GrantBombBag"

struct SkippableItem {
    ItemId itemId;
    const char* tooltip;
    const char* cvarName;
    char defaultValue;
};

std::vector<SkippableItem> SkippableItems = {
    { ITEM_BOMB_BAG_20, "Bomb Bag (20)", FIRST_CYCLE_GIVE_BOMB_BAG_CVAR_NAME, 0 },
    { ITEM_DEKU_NUT, "Deku Nuts (10)", INTRO_GIVE_NUTS_CVAR_NAME, 1 },
};

const TexturePtr GetIconTexturePath(ItemId itemId) {
    return gItemIcons[itemId];
}

void SpeedrunCustomization_Draw() {
    f32 columnWidth = ImGui::GetContentRegionAvail().x / 3 - (ImGui::GetStyle().ItemSpacing.x * 2);

    ImGui::BeginChild("speedrunItemSkips", ImVec2(columnWidth, 0));
    ImGui::SeparatorText("Toggle Starting Item Skips");
    for (size_t i = 0; i < SkippableItems.size(); i++) {
        SkippableItem skippableItem = SkippableItems[i];

        const char* texturePath = (const char*)GetIconTexturePath(skippableItem.itemId);
        ImTextureID textureId = Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(texturePath);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.1f));
        int32_t itemSkipFlag = CVarGetInteger(skippableItem.cvarName, skippableItem.defaultValue);
        float alpha = itemSkipFlag ? 1.0f : 0.4f;

        if (ImGui::ImageButton(std::to_string(i).c_str(), textureId, ImVec2(columnWidth / 8, columnWidth / 8),
                               ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, alpha))) {
            CVarSetInteger(skippableItem.cvarName, !itemSkipFlag);
            Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
        }
        ImGui::SameLine();
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar(2);
        UIWidgets::Tooltip(skippableItem.tooltip);
    }
    ImGui::EndChild();
    ImGui::SameLine();
}