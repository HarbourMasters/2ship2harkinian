#pragma once
#include "2s2h/BenGui/UIWidgets.hpp"
#include <string>

#define COLOR_ORANGE UIWidgets::Colors::Orange
#define COLOR_GREEN UIWidgets::Colors::Green
#define TEXT_COLOR(color) UIWidgets::ColorValues.at(COLOR_##color)

std::vector<std::pair<std::string, std::string>> voyage3Reqs = {
    { "Dungeon Access:", "Requires Transformation & Song" },
    { "Moon Access Remains:", "4 Boss Remains & Oath to Order" },
    { "Stray Fairies Required:", "5" },
};

std::vector<std::string> voyage3Shuffles = {
    "Shuffle Songs", "Shuffle Stray Fairies", "Shuffle Cows", "Shuffle Shops", "Shuffle Boss Remains", "Shuffle Tingle Maps",
};

std::vector<std::string> voyage3Starting = {
    "Full Wallets",    "Maps and Compasses", "Kokiri Sword", "Hero's Shield",
    "Ocarina of Time", "Song of Time",       "Bunny Hood", "Epona's Song",
};

std::vector<std::pair<std::string, std::string>> voyage3Hints = {
    { "General Hints", "The Bomb shop 4th Item, Lottery, Great Fairy Fountains, and Mountain Smithy rewards can be "
                       "hinted by speaking to their respective NPCs." },
    { "Spider House Rewards", "Swamp is hinted within the Spider House.\n"
                              "Ocean is hinted in South Clock Town day 1 on top of the scaffolding." },
    { "Gossip Stone Static", "Similar to Ship of Harkinian, Gossip Stones will provide hints." },
    { "Boss Remains", "In South Clock Town there is a big poster on the side of the Chest Building, this will tell you "
                      "where each Boss Remain is located." },
    { "Oath to Order",
      "Once you have all 4 Remains, talking to Skull Kid on the Clock Tower Rooftop will hint at the song's location." },
    { "Hookshot", "The Zora near Pirates Fortress will hint the items location." },
    { "Transformation Masks", "In South Clock Town, the sign leaning up against the stall near the Business Scrub will "
                              "tell you where one of each mask is located." },
    { "Bank Reward", "The sign next to the bank will tell you what item is rewarded for the Heart Piece check." },
};

void DrawVoyage3Description() {
    ImGui::SeparatorText("Voyage 3 Settings");
    ImGui::TextColored(TEXT_COLOR(ORANGE), "Requirements");
    if (ImGui::BeginTable("Voyage3Req", 2)) {
        for (auto& [key, value] : voyage3Reqs) {
            ImGui::TableNextColumn();
            ImGui::TextColored(TEXT_COLOR(GREEN), key.c_str());
            ImGui::TableNextColumn();
            ImGui::Text(value.c_str());
        }
        ImGui::EndTable();
    }
    ImGui::Separator();
    if (ImGui::BeginTable("Voyage3Req", 2)) {
        ImGui::TableNextColumn();
        ImGui::TextColored(TEXT_COLOR(ORANGE), "Included Shuffles");
        for (auto& shuffle : voyage3Shuffles) {
            ImGui::Text(shuffle.c_str());
        }
        ImGui::TableNextColumn();
        ImGui::TextColored(TEXT_COLOR(ORANGE), "Starting Items");
        for (auto& item : voyage3Starting) {
            ImGui::Text(item.c_str());
        }
        ImGui::EndTable();
    }
    ImGui::Separator();
    ImGui::TextColored(TEXT_COLOR(ORANGE), "Hints");

    for (auto& [key, value] : voyage3Hints) {
        ImGui::TextColored(TEXT_COLOR(GREEN), key.c_str());
        ImGui::TextWrapped(value.c_str());
        ImGui::Separator();
    }
}
