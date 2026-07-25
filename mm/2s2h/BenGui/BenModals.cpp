#include "BenModals.h"
#include <imgui.h>
#include <vector>
#include <string>
#include "UIWidgets.hpp"
#include "BenGui.hpp"

struct BenModal {
    std::string title_;
    std::string message_;
    std::string button1_;
    std::string button2_;
    std::function<void()> button1callback_;
    std::function<void()> button2callback_;
};
std::vector<BenModal> modals;

bool closePopup = false;

void BenModalWindow::Draw() {
    if (!IsVisible()) {
        return;
    }
    DrawElement();
    // Sync up the IsVisible flag if it was changed by ImGui
    SyncVisibilityConsoleVariable();
}

void BenModalWindow::DrawElement() {
    if (modals.size() > 0) {
        BenModal curModal = modals.at(0);
        if (!ImGui::IsPopupOpen(curModal.title_.c_str())) {
            ImGui::OpenPopup(curModal.title_.c_str());
        }
        if (closePopup) {
            ImGui::CloseCurrentPopup();
            modals.erase(modals.begin());
            closePopup = false;
        }
        const ImGuiViewport* modalVp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(modalVp->WorkPos.x + modalVp->WorkSize.x * 0.5f,
                                       modalVp->WorkPos.y + modalVp->WorkSize.y * 0.5f),
                                ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), modalVp->WorkSize);
        if (ImGui::BeginPopupModal(curModal.title_.c_str(), NULL,
                                   ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize |
                                       ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                       ImGuiWindowFlags_NoSavedSettings)) {
            // Wrap: this popup is AlwaysAutoResize|NoResize|NoMove|NoScrollbar, so an unwrapped
            // long message (e.g. the first-run "needs game data" text) clips on both edges with
            // no way to scroll or move it.
            ImGui::PushTextWrapPos(ImGui::GetMainViewport()->WorkSize.x * 0.8f);
            ImGui::TextUnformatted(curModal.message_.c_str());
            ImGui::PopTextWrapPos();
            UIWidgets::PushStyleButton(THEME_COLOR);
            if (ImGui::Button(curModal.button1_.c_str())) {
                if (curModal.button1callback_ != nullptr) {
                    curModal.button1callback_();
                }
                ImGui::CloseCurrentPopup();
                modals.erase(modals.begin());
            }
            UIWidgets::PopStyleButton();
            if (curModal.button2_ != "") {
                ImGui::SameLine();
                UIWidgets::PushStyleButton(THEME_COLOR);
                if (ImGui::Button(curModal.button2_.c_str())) {
                    if (curModal.button2callback_ != nullptr) {
                        curModal.button2callback_();
                    }
                    ImGui::CloseCurrentPopup();
                    modals.erase(modals.begin());
                }
                UIWidgets::PopStyleButton();
            }
            ImGui::EndPopup();
        }
    }
}

void BenModalWindow::RegisterPopup(std::string title, std::string message, std::string button1, std::string button2,
                                   std::function<void()> button1callback, std::function<void()> button2callback) {
    modals.push_back({ title, message, button1, button2, button1callback, button2callback });
}

size_t BenModalWindow::PopupsQueued() {
    return modals.size();
}

bool BenModalWindow::IsPopupOpen(std::string title) {
    return !modals.empty() && modals.at(0).title_ == title;
}

void BenModalWindow::DismissPopup() {
    closePopup = true;
}
