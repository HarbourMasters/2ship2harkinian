#include "Menu.h"
#include "BenPort.h"

std::vector<ImVec2> windowTypeSizes = { {} };

void BenMenu::InitElement() {
    menuWidth = 1600;
    menuHeight = 900;
}

void BenMenu::UpdateElement() {
    auto winHeight = ImGui::GetMainViewport()->Size.y;
    auto winWidth = ImGui::GetMainViewport()->Size.x;
    auto dpi = ImGui::GetMainViewport()->DpiScale;
    menuHeight = winHeight < 720 ? 720 : winHeight - 200;
    menuWidth = winWidth < 960 ? 960 : winWidth - 200;
}

void BenMenu::DrawElement() {
    ImGui::SetNextWindowSize({ static_cast<float>(menuWidth), static_cast<float>(menuHeight) });
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), 0, { 0.5f, 0.5f });
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.0f);
    ImGuiStyle& style = ImGui::GetStyle();
    // ImColor prevButtonCol = style.Colors[ImGuiCol_Button];
    // style.Colors[ImGuiCol_Button] = ImColor(0, 0, 0, 0);

    if (!ImGui::Begin("Settings Menu", NULL,
                      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::Button("Settings");
        ImGui::SameLine();
        ImGui::Checkbox("", NULL);
        ImGui::PopStyleVar(2);
        ImGui::End();
        return;
    }

    // style.Colors[ImGuiCol_Button] = prevButtonCol;
    ImGui::PopStyleVar(2);
    ImGui::End();
}