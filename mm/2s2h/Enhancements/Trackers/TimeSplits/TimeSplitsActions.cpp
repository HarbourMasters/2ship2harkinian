#include "Timesplits.h"
#include "public/bridge/consolevariablebridge.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "Context.h"
#include "Window.h"
#include "2s2h/BenGui/UIWidgets.hpp"
#include <fstream>
#include <filesystem>

using json = nlohmann::json;

extern "C" {
#include "variables.h"
uint64_t GetUnixTimestamp();
}

#define CVAR_NAME "gSettings.TimeSplits.Enable"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

nlohmann::json TimesplitObject_to_json(const TimesplitObject& split) {
    return nlohmann::json{
        { "splitId", split.splitId },
        { "splitName", split.splitName },
        { "splitCurrentTime", split.splitCurrentTime },
        { "splitPreviousBest", split.splitPreviousBest },
        { "splitStatus", SPLIT_INACTIVE },
    };
}

TimesplitObject json_to_TimesplitObject(const nlohmann::json& jsonSplit) {
    TimesplitObject split;
    split.splitId = jsonSplit["splitId"];
    split.splitName = jsonSplit["splitName"].get<std::string>();
    split.splitCurrentTime = jsonSplit["splitCurrentTime"];
    split.splitPreviousBest = jsonSplit["splitPreviousBest"];
    split.splitStatus = jsonSplit["splitStatus"];
    return split;
}

uint32_t GetCurrentActiveSplit(std::vector<TimesplitObject> list) {
    for (size_t i = 0; i < splitList.size(); i++) {
        if (splitList[i].splitStatus == SPLIT_ACTIVE) {
            return (uint32_t)i;
        }
    }
    return -1;
}

TimesplitObject GetSplitObjectById(uint32_t itemId) {
    TimesplitObject splitObject;
    for (auto& list : splitObjectList) {
        if (list.splitId == itemId) {
            splitObject = list;
        }
    }
    return splitObject;
}

ImVec4 GetColorTint(uint32_t itemId) {
    auto findColor = songColorMap.find(itemId);
    if (findColor != songColorMap.end()) {
        return findColor->second;
    } else {
        return ImVec4(1, 1, 1, 1);
    }
}

void HandlePopUpContext(uint32_t popupId) {
    if (shouldPopUpOpen && ImGui::BeginPopup("ItemSubMenu")) {
        std::vector<uint32_t> itemList;

        for (auto& item : itemSubMenuList) {
            if (item.first == popupId) {
                itemList = item.second;
                break;
            }
        }

        if (itemList.size() == 0) {
            ImGui::EndPopup();
            return;
        }

        uint32_t slotIndex = 0;
        for (auto& list : itemList) {
            SplitsPushImageButtonStyle();
            if (ImGui::ImageButton(
                    std::to_string(list).c_str(),
                    Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(GetItemImageById(list)),
                    ImVec2(32.0f, 32.0f), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), GetColorTint(list))) {
                AddSplitEntryById(list);
                ImGui::CloseCurrentPopup();
                shouldPopUpOpen = false;
            }
            UIWidgets::Tooltip(GetSplitObjectById(list).splitName.c_str());
            SplitsPopImageButtonStyle();

            if (slotIndex == 4) {
                slotIndex = -1;
            } else {
                ImGui::SameLine();
            }
            slotIndex++;
        }
        ImGui::EndPopup();
    }
}

void HandleDragAndDrop(size_t i) {
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
        ImGui::SetDragDropPayload("SPLIT_DRAG", &i, sizeof(size_t));
        ImGui::ImageButton(std::to_string(splitList[i].splitId).c_str(),
                           Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
                               GetItemImageById(splitList[i].splitId)),
                           ImVec2(32.0f, 32.0f));
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SPLIT_DRAG")) {
            size_t srcIndex = *(const size_t*)payload->Data;
            if (srcIndex != i && srcIndex < splitList.size()) {
                auto item = splitList[srcIndex];
                splitList.erase(splitList.begin() + srcIndex);

                if (srcIndex < i) {
                    i--;
                }

                splitList.insert(splitList.begin() + i, item);
            }
        }
        ImGui::EndDragDropTarget();
    }
}

void CheckSplitsCompleted(uint32_t index) {
    if (index == splitList.size() - 1) {
        gSaveContext.save.shipSaveInfo.fileCompletedAt = GetUnixTimestamp();
    } else {
        splitList[index + 1].splitStatus = SPLIT_ACTIVE;
    }
}

void AddSplitEntryById(uint32_t itemId) {
    TimesplitObject splitObject = GetSplitObjectById(itemId);

    if (splitList.size() == 0) {
        splitObject.splitStatus = SPLIT_ACTIVE;
    }
    splitList.push_back(splitObject);
}

void RemoveSplitEntry(uint32_t splitId, uint32_t index) {
    uint32_t activeIndex = GetCurrentActiveSplit(splitList);

    if (activeIndex != -1) {
        if (splitList[activeIndex].splitId == splitId) {
            CheckSplitsCompleted(activeIndex);
        }
    }

    splitList.erase(splitList.begin() + index);
}

void SkipSplitEntry(uint32_t index) {
    if (splitList[index].splitStatus == SPLIT_ACTIVE) {
        CheckSplitsCompleted(index);
    }
    splitList[index].splitStatus = SPLIT_SKIPPED;
}

void UpdateSplitBests() {
    for (auto& splits : splitList) {
        if (splits.splitCurrentTime < splits.splitPreviousBest || splits.splitPreviousBest == 0) {
            splits.splitPreviousBest = splits.splitCurrentTime;
        }
    }
}

void UpdateSplitStatusById(uint32_t itemId) {
    uint32_t activeIndex = GetCurrentActiveSplit(splitList);

    if (activeIndex == -1) {
        return;
    }

    if (splitList[activeIndex].splitId == itemId) {
        splitList[activeIndex].splitCurrentTime =
            ((GetUnixTimestamp() - gSaveContext.save.shipSaveInfo.fileCreatedAt) / 100);
        splitList[activeIndex].splitStatus = SPLIT_COMPLETE;

        if (activeIndex == splitList.size() - 1) {
            CheckSplitsCompleted(activeIndex);
        } else {
            splitList[activeIndex + 1].splitStatus = SPLIT_ACTIVE;
        }
    }
}

void GetSplitByActorId(int16_t actorId) {
    uint32_t activeIndex = GetCurrentActiveSplit(splitList);

    switch (actorId) {
        case ACTOR_BOSS_01:
            UpdateSplitStatusById(SPLIT_KILLED_ODOLWA);
            break;
        case ACTOR_BOSS_02:
            UpdateSplitStatusById(SPLIT_KILLED_TWINMOLD);
            break;
        case ACTOR_BOSS_03:
            UpdateSplitStatusById(SPLIT_KILLED_GYORG);
            break;
        case ACTOR_BOSS_07:
            UpdateSplitStatusById(SPLIT_KILLED_MAJORA);
            break;
        case ACTOR_BOSS_HAKUGIN:
            UpdateSplitStatusById(SPLIT_KILLED_GOHT);
            break;
        default:
            break;
    }

    if (activeIndex == -1) {
        return;
    }
}

void SplitSaveFileAction(uint32_t action, std::string listName) {
    std::string filename = Ship::Context::GetPathRelativeToAppDirectory("2S2HTimeSplitData.json");
    json saveFile;
    json listArray = nlohmann::json::array();

    std::ifstream inputFile(filename);
    if (inputFile.is_open()) {
        inputFile >> saveFile;
        inputFile.close();
    }

    if (action == SPLIT_SAVE) {
        for (auto& data : splitList) {
            listArray.push_back(TimesplitObject_to_json(data));
        }
        saveFile[listName] = listArray;

        std::ofstream outputFile(filename);
        if (outputFile.is_open()) {
            outputFile << saveFile.dump(4);
            outputFile.close();
        }
    }

    if (action == SPLIT_LOAD) {
        if (saveFile.contains(listName)) {
            listArray = saveFile[listName];
            splitList.clear();

            for (auto& data : listArray) {
                splitList.push_back(json_to_TimesplitObject(data));
            }
            splitList[0].splitStatus = SPLIT_ACTIVE;
        }
    }

    if (action == SPLIT_RETRIEVE) {
        savedLists.clear();

        for (auto& data : saveFile.items()) {
            savedLists.push_back(data.key());
        }
        if (savedLists.size() == 0) {
            savedLists.push_back("Create a List First");
        }
    }
}

void RegisterTimesplits() {
    if (!std::filesystem::exists(Ship::Context::GetPathRelativeToAppDirectory("2S2HTimeSplitData.json"))) {
        json initFile;
        std::ofstream file(Ship::Context::GetPathRelativeToAppDirectory("2S2HTimeSplitData.json"));
        file << initFile.dump(4);
        file.close();
    }

    SplitSaveFileAction(SPLIT_RETRIEVE, "");

    COND_HOOK(OnItemGive, CVAR, [](u8 item) {
        if (item == ITEM_HEART_PIECE_2) {
            item = ITEM_HEART_PIECE;
        }
        if (item == ITEM_LONGSHOT) {
            item = ITEM_POTION_RED;
        }

        UpdateSplitStatusById((uint32_t)item);
    });

    COND_HOOK(OnBottleContentsUpdate, CVAR, [](u8 item) { UpdateSplitStatusById((uint32_t)item); });
    COND_HOOK(OnBossDefeated, CVAR, [](int16_t actorId) { GetSplitByActorId(actorId); });
}

static RegisterShipInitFunc initFunc(RegisterTimesplits, { CVAR_NAME });
