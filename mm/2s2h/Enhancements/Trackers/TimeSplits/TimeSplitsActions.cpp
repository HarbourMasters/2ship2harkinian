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
            if (ImGui::ImageButton(std::to_string(list).c_str(),
                                   Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
                                       (const char*)gItemIcons[list]),
                                   ImVec2(32.0f, 32.0f), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0),
                                   GetColorTint(list))) {
                AddSplitEntry(list);
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

void HandleDragAndDrop(std::vector<TimesplitObject>& splitList, size_t i) {
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
        ImGui::SetDragDropPayload("SPLIT_DRAG", &i, sizeof(size_t));
        ImGui::ImageButton(
            std::to_string(splitList[i].splitId).c_str(),
                           Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
                               (const char*)gItemIcons[splitList[i].splitId]),
            ImVec2(32.0f, 32.0f));
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SPLIT_DRAG")) {
            size_t srcIndex = *(const size_t*)payload->Data;
            if (srcIndex != i && srcIndex < splitList.size()) {
                auto item = splitList[srcIndex];
                splitList.erase(splitList.begin() + srcIndex);

                // adjust index if needed (erase shifts indices)
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

void AddSplitEntry(uint32_t itemId) {
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

void UpdateSplitStatus(uint32_t itemId) {
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

void SplitSaveFileAction(uint32_t action, const char* listName) {
    std::string filename = Ship::Context::GetPathRelativeToAppDirectory("timesplitdata.json");
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
    }
}

//void UpdateSplitStatusTest(FlagType flagType, u32 flag) {
//    uint32_t activeIndex = GetCurrentActiveSplit(splitList);
//
//    SPDLOG_INFO("Flag Type: {} | Flag: {}", std::to_string(flagType), std::to_string(flag));
//
//    if (flagType == FLAG_CYCL_SCENE_CLEARED_ROOM && gPlayState->sceneId == SCENE_MITURIN_BS) {
//        int hi = 0;
//    }
//
//
//    if (activeIndex == -1) {
//        return;
//    }
//}

void RegisterTimesplits() {
    // COND_VB_SHOULD(VB_GIVE_ITEM_FROM_OFFER, CVAR, {
    //     GetItemId* item = va_arg(args, GetItemId*);
    //     Actor* actor = va_arg(args, Actor*);
    //
    //     UpdateSplitStatus((uint32_t)*item);
    // });

    COND_HOOK(OnItemGive, CVAR, [](u8 item) {
        if (item == ITEM_HEART_PIECE_2) {
            item = ITEM_HEART_PIECE;
        }
        if (item == ITEM_LONGSHOT) {
            item = ITEM_POTION_RED;
        }

        UpdateSplitStatus((uint32_t)item);
    });

    COND_HOOK(OnBottleContentsUpdate, CVAR, [](u8 item) { UpdateSplitStatus((uint32_t)item); });
    //COND_HOOK(OnActorKill, CVAR, [](Actor* actor) { UpdateSplitStatus((uint32_t)actor->id); });
    //COND_HOOK(OnFlagSet, CVAR, [](FlagType flagType, u32 flag) { UpdateSplitStatusTest(flagType, flag); });
}

static RegisterShipInitFunc initFunc(RegisterTimesplits, { CVAR_NAME });
