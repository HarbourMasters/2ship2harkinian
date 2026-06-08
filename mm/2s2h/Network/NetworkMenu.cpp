#include "2s2h/BenGui/UIWidgets.hpp"
#include "ShipUtils.h"
#include "2s2h/BenGui/BenMenu.h"
#include "2s2h/BenGui/Notification.h"
#include "2s2h/Network/Sail/Sail.h"
#include "2s2h/Network/Anchor/Anchor.h"

namespace BenGui {
extern std::shared_ptr<BenMenu> mBenMenu;
} // namespace BenGui
using namespace UIWidgets;

void RegisterNetworkMenu() {
    // Add Network Menu
    BenGui::mBenMenu->AddMenuEntry("Network", "gSettings.Menu.NetworkSidebarSection");
    WidgetPath path;

#ifndef ENABLE_NETWORKING
    path = { "Network", "Info", SECTION_COLUMN_1 };
    BenGui::mBenMenu->AddSidebarEntry("Network", path.sidebarName, 2);

    BenGui::mBenMenu
        ->AddWidget(path,
                    ICON_FA_EXCLAMATION_TRIANGLE
                    " The Network features are unavailable because SoH was compiled without "
                    "network support (\"ENABLE_NETWORKING\" build flag).",
                    WIDGET_TEXT)
        .Options(TextOptions().Color(Colors::Orange));
    return;
#endif

    // Sail
    path = { "Network", "Sail", SECTION_COLUMN_1 };
    BenGui::mBenMenu->AddSidebarEntry("Network", path.sidebarName, 3);
    BenGui::mBenMenu->AddWidget(path, "Host & Port", WIDGET_CUSTOM).CustomFunction([](WidgetInfo& info) {
        ImGui::BeginDisabled(Sail::Instance->isEnabled);
        ImGui::Text("%s", info.name.c_str());
        CVarInputString("##HostSail", "gNetwork.Sail.Host",
                        InputOptions()
                            .PlaceholderText("127.0.0.1")
                            .DefaultValue("127.0.0.1")
                            .Size(ImVec2(ImGui::GetContentRegionAvail().x - ImGui::GetFontSize() * 7, 0))
                            .LabelPosition(LabelPosition::None));
        ImGui::SameLine();
        ImGui::Text(":");
        ImGui::SameLine();
        CVarInputInt("##PortSail", "gNetwork.Sail.Port",
                     InputOptions()
                         .PlaceholderText("43384")
                         .DefaultValue("43384")
                         .Size(ImVec2(ImGui::GetFontSize() * 5, 0))
                         .LabelPosition(LabelPosition::None));
        ImGui::EndDisabled();
    });
    BenGui::mBenMenu->AddWidget(path, "Enable##Sail", WIDGET_BUTTON)
        .PreFunc([](WidgetInfo& info) {
            std::string host = CVarGetString("gNetwork.Sail.Host", "127.0.0.1");
            uint16_t port = CVarGetInteger("gNetwork.Sail.Port", 43384);
            info.options->disabled = !(!isStringEmpty(host) && port > 1024 && port < 65535);
            if (Sail::Instance->isEnabled) {
                info.name = "Disable##Sail";
            } else {
                info.name = "Enable##Sail";
            }
        })
        .Callback([](WidgetInfo& info) {
            if (Sail::Instance->isEnabled) {
                CVarClear("gNetwork.Sail.Enabled");
                Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
                Sail::Instance->Disable();
            } else {
                CVarSetInteger("gNetwork.Sail.Enabled", 1);
                Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
                Sail::Instance->Enable();
            }
        });
    BenGui::mBenMenu->AddWidget(path, "Connecting...", WIDGET_TEXT).PreFunc([](WidgetInfo& info) {
        info.isHidden = !Sail::Instance->isEnabled;
        if (Sail::Instance->isConnected) {
            info.name = "Connected##Sail";
        } else {
            info.name = "Connecting...##Sail";
        }
    });
    path.column = SECTION_COLUMN_2;
    BenGui::mBenMenu->AddWidget(path,
                                "Sail is a networking protocol designed to facilitate remote "
                                "control of the 2Ship2Harkinian client. It is intended to "
                                "be utilized alongside a Sail server, for which we provide a "
                                "few straightforward implementations on our GitHub. The current "
                                "implementations available allow integration with Twitch chat "
                                "and SAMMI Bot, feel free to contribute your own!\n"
                                "\n"
                                "Click this button to copy the link to the Sail Github "
                                "page to your clipboard.",
                                WIDGET_TEXT);
    BenGui::mBenMenu->AddWidget(path, ICON_FA_CLIPBOARD "##Sail", WIDGET_BUTTON)
        .Callback([](WidgetInfo& info) {
            ImGui::SetClipboardText("https://github.com/HarbourMasters/sail");
            Notification::Emit({
                .message = "Copied to clipboard",
            });
        })
        .Options(ButtonOptions().Tooltip("https://github.com/HarbourMasters/sail"));

    // Anchor
    path = { "Network", "Anchor", SECTION_COLUMN_1 };
    BenGui::mBenMenu->AddSidebarEntry("Network", path.sidebarName, 4);
    BenGui::mBenMenu->AddWidget(path, "Host & Port", WIDGET_CUSTOM).CustomFunction([](WidgetInfo& info) {
        ImGui::BeginDisabled(Anchor::Instance->isEnabled);
        ImGui::Text("%s", info.name.c_str());
        CVarInputString("##HostAnchor", "gNetwork.Anchor.Host",
                        InputOptions()
                            .PlaceholderText("anchor.hm64.org")
                            .DefaultValue("anchor.hm64.org")
                            .Size(ImVec2(ImGui::GetContentRegionAvail().x - ImGui::GetFontSize() * 7, 0))
                            .LabelPosition(LabelPosition::None));
        ImGui::SameLine();
        ImGui::Text(":");
        ImGui::SameLine();
        CVarInputInt("##PortAnchor", "gNetwork.Anchor.Port",
                     InputOptions()
                         .PlaceholderText("43383")
                         .DefaultValue("43383")
                         .Size(ImVec2(ImGui::GetFontSize() * 5, 0))
                         .LabelPosition(LabelPosition::None));
        ImGui::EndDisabled();
    });
    BenGui::mBenMenu->AddWidget(path, "Name", WIDGET_CUSTOM).CustomFunction([](WidgetInfo& info) {
        ImGui::BeginDisabled(Anchor::Instance->isEnabled);
        CVarInputString("Name##Anchor", "gNetwork.Anchor.Name", InputOptions().PlaceholderText("Player"));
        CVarInputString("Room ID##Anchor", "gNetwork.Anchor.RoomId", InputOptions().PlaceholderText("(optional)"));
        CVarInputString("Team ID##Anchor", "gNetwork.Anchor.TeamId",
                        InputOptions().PlaceholderText("default").DefaultValue("default"));
        ImGui::EndDisabled();
    });
    BenGui::mBenMenu->AddWidget(path, "Enable##Anchor", WIDGET_BUTTON)
        .PreFunc([](WidgetInfo& info) {
            std::string host = CVarGetString("gNetwork.Anchor.Host", "anchor.hm64.org");
            uint16_t port = CVarGetInteger("gNetwork.Anchor.Port", 43383);
            info.options->disabled = !(!isStringEmpty(host) && port > 1024 && port < 65535);
            info.name = Anchor::Instance->isEnabled ? "Disable##Anchor" : "Enable##Anchor";
        })
        .Callback([](WidgetInfo& info) {
            if (Anchor::Instance->isEnabled) {
                CVarClear("gNetwork.Anchor.Enabled");
                Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
                Anchor::Instance->Disable();
            } else {
                CVarSetInteger("gNetwork.Anchor.Enabled", 1);
                Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
                Anchor::Instance->Enable();
            }
        });
    BenGui::mBenMenu->AddWidget(path, "Connecting...", WIDGET_TEXT).PreFunc([](WidgetInfo& info) {
        info.isHidden = !Anchor::Instance->isEnabled;
        info.name = Anchor::Instance->isConnected ? "Connected##Anchor" : "Connecting...##Anchor";
    });
    BenGui::mBenMenu->AddWidget(path, "Players##Anchor", WIDGET_CUSTOM).CustomFunction([](WidgetInfo& info) {
        if (!Anchor::Instance->isConnected) {
            return;
        }
        ImGui::Text("Players in room:");
        for (auto& [clientId, client] : Anchor::Instance->clients) {
            if (!client.online) {
                continue;
            }
            ImGui::TextColored(ImVec4(client.color.r / 255.0f, client.color.g / 255.0f, client.color.b / 255.0f, 1.0f),
                               "%s%s", client.name.c_str(), client.self ? " (you)" : "");
        }
    });
    path.column = SECTION_COLUMN_2;
    BenGui::mBenMenu->AddWidget(path,
                                "Anchor enables co-op multiplayer by syncing progress (and, in later "
                                "versions, player positions) between everyone connected to the same "
                                "room. Pick a Room ID to play privately with friends, and share a Team "
                                "ID to share items and flags.\n"
                                "\n"
                                "The default host is the public Anchor server. You can also self-host "
                                "the Anchor server from its GitHub repository.",
                                WIDGET_TEXT);
    BenGui::mBenMenu->AddWidget(path, ICON_FA_CLIPBOARD "##Anchor", WIDGET_BUTTON)
        .Callback([](WidgetInfo& info) {
            ImGui::SetClipboardText("https://github.com/garrettjoecox/anchor");
            Notification::Emit({
                .message = "Copied to clipboard",
            });
        })
        .Options(ButtonOptions().Tooltip("https://github.com/garrettjoecox/anchor"));
}

static RegisterMenuInitFunc initFunc(RegisterNetworkMenu);