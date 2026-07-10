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
    "Shuffle Boss Remains", "Shuffle Cows", "Shuffle Shops", "Shuffle Songs", "Shuffle Stray Fairies", "Shuffle Tingle Maps",
};

std::vector<std::string> voyage3Starting = {
    "Bunny Hood", "Epona's Song", "Full Wallets", "Hero's Shield", 
    "Maps and Compasses", "Kokiri Sword", "Ocarina of Time", "Song of Time",        
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

//New Addition for Forced Junk Information
std::vector<std::pair<std::string, std::string>> voyage3ForcedJunk = {
    { "Beneath the Graveyard Dampe Chest", "The Dampe digging side quest, which can be accessed at Ikana graveyard "
										   "on night 3 with captains hat." },
    { "Minigames All Day Rewards", "North Clocktown Deku Playground all day reward.\n"
								   "East Clock Town Honey and Darling all day reward." },
    { "East Clock Town Shooting Gallery", "The reward for getting a perfect score in the East Clock Town Shooting Gallery." },
    { "Deku Shrine Mask of Scents", "Once you complete Woodfall Temple, the reward for completing the Butler race." },
    { "Great Bay Coast Fisherman Minigame",
      "Once you complete Great Bay Temple, the island hopping minigame ran by the fisherman." },
    { "Dungeon Stray Fairies", "Any stray fairy that are located in dungeons whose vanilla location were not in chests,\n"
							   "including stray fairies in bubbles, pots, or any stray fairy found by defeating enemies.\n"
							   "This makes the Great Fairy Mask effectively useless." },
    { "All Moon Checks", "Any check that is located on the moon." },
    { "Mountain Village Frog Choir", "Once you complete Snowhead Temple, the reward for returning all frogs with Don Gero's "
						"Mask. This makes the Don Gero Mask effectively useless." },
	{ "Stock Pot Inn Couples Mask", "The final reward in the Kafei and Anju story quest found on Night 3.\n" 
									"This makes the Pendant of Memories effectively useless." },
	{ "Waterfall Rapids Beaver Race #2", "The reward obtained by racing the older beaver brother a second time.\n" 
										 "With racing younger brother being disabled, you only need to race the older "
										 "brother once." },
};
//---

void DrawVoyage3Description() {
    ImGui::SeparatorText("Voyage 3 Settings");
    ImGui::TextColored(TEXT_COLOR(ORANGE), "Requirements");
    if (ImGui::BeginTable("Voyage3SeedSettings", 2)) {
        for (auto& [key, value] : voyage3Reqs) {
            ImGui::TableNextColumn();
            ImGui::TextColored(TEXT_COLOR(GREEN), key.c_str());
            ImGui::TableNextColumn();
            ImGui::Text(value.c_str());
        }
        ImGui::EndTable();
    }
    ImGui::Separator();
	ImGui::Separator();
    if (ImGui::BeginTable("Voyage3Req", 2)) {
        ImGui::TableNextColumn();
        ImGui::TextColored(TEXT_COLOR(ORANGE), "Included Shuffles");
         if (ImGui::BeginTable("Voyage3Shuffles", 2)) {
            for (auto& shuffle : voyage3Shuffles) {
                ImGui::TableNextColumn();
                ImGui::Text(shuffle.c_str());
            }
            ImGui::EndTable();
        }
        ImGui::TableNextColumn();
        ImGui::TextColored(TEXT_COLOR(ORANGE), "Starting Items");
        if (ImGui::BeginTable("Voyage3Starting", 2)) {
            for (auto& item : voyage3Starting) {
                ImGui::TableNextColumn();
                ImGui::Text(item.c_str());
            }
            ImGui::EndTable();
        }
        ImGui::EndTable();
    }
    ImGui::Separator();
	ImGui::Separator();
    ImGui::TextColored(TEXT_COLOR(ORANGE), "Hints");

    for (auto& [key, value] : voyage3Hints) {
        ImGui::TextColored(TEXT_COLOR(GREEN), key.c_str());
        ImGui::TextWrapped(value.c_str());
        ImGui::Separator();
    }

	ImGui::Separator();
	//New Addition to add forced junk information
    ImGui::TextColored(TEXT_COLOR(ORANGE), "Forced Junk");

    for (auto& [key, value] : voyage3ForcedJunk) {
        ImGui::TextColored(TEXT_COLOR(GREEN), key.c_str());
        ImGui::TextWrapped(value.c_str());
        ImGui::Separator();
    }
	//---
}
