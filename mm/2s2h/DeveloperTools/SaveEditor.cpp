#include "SaveEditor.h"
#include "2s2h/BenGui/UIWidgets.hpp"

extern "C" {
#include <z64.h>
#include <z64save.h>
#include <macros.h>
#include <variables.h>
#include <functions.h>
extern PlayState* gPlayState;
extern SaveContext gSaveContext;
extern TexturePtr gItemIcons[131];
extern u8 gItemSlots[77];
void Interface_LoadItemIconImpl(PlayState* play, u8 btn);
void Interface_NewDay(PlayState* play, s32 day);
extern RegEditor* gRegEditor;
}

bool safeMode = true;
constexpr float INV_GRID_WIDTH = 46.0f;
constexpr float INV_GRID_HEIGHT = 58.0f;
constexpr float INV_GRID_ICON_SIZE = 40.0f;
constexpr float INV_GRID_PADDING = 10.0f;
constexpr float INV_GRID_TOP_MARGIN = 20.0f;
constexpr uint16_t HEART_COUNT_MIN = 3;
constexpr uint16_t HEART_COUNT_MAX = 20;
constexpr int16_t S16_ZERO = 0;
constexpr int8_t S8_ZERO = 0;
constexpr u8 U8_ZERO = 0;
constexpr u8 REG_PAGES_MAX = REG_PAGES;
constexpr u8 REG_GROUPS_MAX = REG_GROUPS - 1;
const char* MAGIC_LEVEL_NAMES[3] = { "No Magic", "Single Magic", "Double Magic" };
constexpr int8_t MAGIC_LEVEL_MAX = 2;
const char* WALLET_LEVEL_NAMES[3] = { "Child Wallet", "Adult Wallet", "Giant Wallet" };
constexpr u8 WALLET_LEVEL_MAX = 2;

InventorySlot selectedInventorySlot = SLOT_NONE;
std::vector<ItemId> safeItemsForInventorySlot[SLOT_MASK_FIERCE_DEITY + 1] = {};

void initSafeItemsForInventorySlot() {
    for (int i = 0; i < sizeof(gItemSlots); i++) {
        InventorySlot slot = static_cast<InventorySlot>(gItemSlots[i]);
        switch (slot) {
            case SLOT_BOTTLE_1:
                if (i != ITEM_LONGSHOT) { // No longshot in bottles
                    safeItemsForInventorySlot[SLOT_BOTTLE_1].push_back(static_cast<ItemId>(i));
                    safeItemsForInventorySlot[SLOT_BOTTLE_2].push_back(static_cast<ItemId>(i));
                    safeItemsForInventorySlot[SLOT_BOTTLE_3].push_back(static_cast<ItemId>(i));
                    safeItemsForInventorySlot[SLOT_BOTTLE_4].push_back(static_cast<ItemId>(i));
                    safeItemsForInventorySlot[SLOT_BOTTLE_5].push_back(static_cast<ItemId>(i));
                    safeItemsForInventorySlot[SLOT_BOTTLE_6].push_back(static_cast<ItemId>(i));
                }
                break;
            case SLOT_BOW:
                if (i == ITEM_BOW) { // No elemental bows here
                    safeItemsForInventorySlot[slot].push_back(static_cast<ItemId>(i));
                }
                break;
            case SLOT_TRADE_KEY_MAMA:
                if (i != ITEM_SLINGSHOT) { // No slingshot in trade items
                    safeItemsForInventorySlot[slot].push_back(static_cast<ItemId>(i));
                }
                break;
            case SLOT_TRADE_DEED:
                if (i != ITEM_OCARINA_FAIRY) { // No fairy ocarina in trade items
                    safeItemsForInventorySlot[slot].push_back(static_cast<ItemId>(i));
                }
                break;
            default:
                safeItemsForInventorySlot[slot].push_back(static_cast<ItemId>(i));
                break;
        }
    }
}

char z2ASCII(int code) {
    int ret;
    if (code < 10) { //Digits
        ret = code + 0x30;
    } else if (code >= 10 && code < 36) { //Uppercase letters
        ret = code + 0x37;
    } else if (code >= 36 && code < 62) { //Lowercase letters
        ret = code + 0x3D;
    } else if (code == 62) { //Space
        ret = code - 0x1E;
    } else if (code == 63 || code == 64) { // _ and .
        ret = code - 0x12;
    } else {
        ret = code;
    }
    return char(ret);
}

char ASCII2z(int code) {
    int ret;
    if (code >= 0x30 && code < 0x3A) { //Digits
        ret = code - 0x30;
    } else if (code >= 0x41 && code < 0x5B) { //Uppercase letters
        ret = code - 0x37;
    } else if (code >= 0x61 && code < 0x7B) { //Lowercase letters
        ret = code - 0x3D;
    } else if (code == 0x20 || code == 0x0) { //Space
        ret = 62;
    } else if (code == 0x5F || code == 0x2E) { // _ and .
        ret = code + 0x12;
    } else {
        ret = code;
    }
    return char(ret);
}

char* getPlayerName(char* playerNameBuf) {
    for (int i = 0; i < 8; i++) {
        playerNameBuf[i] = z2ASCII(gSaveContext.save.saveInfo.playerData.playerName[i]);
    }
    playerNameBuf[8] = '\0';
    return playerNameBuf;
}

int setPlayerName(ImGuiInputTextCallbackData* data) {
    for (int i = 0; i < 8; i++) {
        gSaveContext.save.saveInfo.playerData.playerName[i] = ASCII2z(data->Buf[i]);
    }
    return 0;
};

void DrawGeneralTab() {
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 8.0f));
    ImGui::BeginChild("generalTab", ImVec2(0, 0), true);
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f - 4.0f);

    ImGui::BeginGroup();
    ImGui::Text("Player Name");
    ImGui::PushStyleColor(ImGuiCol_FrameBg, UIWidgets::Colors::Gray);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 8.0f));
    static char playerNameBuf[9];
    ImGui::InputText("##playerNameInput", getPlayerName(playerNameBuf), 9, ImGuiInputTextFlags_CallbackAlways | ImGuiInputTextFlags_AutoSelectAll, setPlayerName);
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(1);
    ImGui::EndGroup();

    ImGui::SameLine();
    ImGui::SetCursorPosY(0.0f);
    ImGui::BeginGroup();
    ImGui::Text("Time");
    UIWidgets::PushStyleSlider();
    static u16 minTime = 0;
    static u16 maxTime = 0xFFFF;
    // Get current time and format as digital string
    u16 curMinutes = (s32)TIME_TO_MINUTES_F(gSaveContext.save.time) % 60;
    u16 curHours = (s32)TIME_TO_MINUTES_F(gSaveContext.save.time) / 60;
    std::string minutes = (curMinutes < 10 ? "0" : "") + std::to_string(curMinutes);
    std::string hours = "";
    std::string ampm = "";
    // BENTODO: Switch to CVar if we ever add 24 hour mode display
    if (false) {
        if (curHours < 10) {
            hours += "0";
        }
    } else {
        ampm = curHours >= 12 ? " pm" : " am";
        curHours = curHours % 12 ? curHours % 12 : 12;
    }
    hours += std::to_string(curHours);
    std::string timeString = hours + ":" + minutes + ampm + " (0x%x)";
    ImGui::SliderScalar("##timeInput", ImGuiDataType_U16, &gSaveContext.save.time, &minTime, &maxTime, timeString.c_str());
    UIWidgets::PopStyleSlider();
    ImGui::EndGroup();

    ImGui::BeginGroup();
    if (UIWidgets::Button("Max Health", { .color = UIWidgets::Colors::Red, .size = UIWidgets::Sizes::Inline })) {
        gSaveContext.save.saveInfo.playerData.doubleDefense = 1;
        gSaveContext.save.saveInfo.inventory.defenseHearts = 20;
        gSaveContext.save.saveInfo.playerData.healthCapacity = gSaveContext.save.saveInfo.playerData.health = 20 * 16;
        gSaveContext.save.saveInfo.inventory.questItems &= ~0xF0000000;
    }
    ImGui::SameLine();
    if (UIWidgets::Button("Reset##resetHealthButton", { .color = UIWidgets::Colors::Gray, .size = UIWidgets::Sizes::Inline })) {
        gSaveContext.save.saveInfo.playerData.doubleDefense = 0;
        gSaveContext.save.saveInfo.inventory.defenseHearts = 0;
        gSaveContext.save.saveInfo.playerData.healthCapacity = gSaveContext.save.saveInfo.playerData.health = 3 * 16;
        gSaveContext.save.saveInfo.inventory.questItems &= ~0xF0000000;
    }
    ImGui::SameLine();
    if (UIWidgets::Checkbox("DD", (bool *)&gSaveContext.save.saveInfo.playerData.doubleDefense, { .color = UIWidgets::Colors::Red })) {
        if (gSaveContext.save.saveInfo.playerData.doubleDefense) {
            gSaveContext.save.saveInfo.inventory.defenseHearts = 20;
        } else {
            gSaveContext.save.saveInfo.inventory.defenseHearts = 0;
        }
    }
    UIWidgets::PushStyleSlider(UIWidgets::Colors::DarkRed);
    int16_t heartCount = (int16_t)gSaveContext.save.saveInfo.playerData.healthCapacity / 16;
    if (ImGui::SliderScalar("##heartCountSlider", ImGuiDataType_U16, &heartCount, &HEART_COUNT_MIN, &HEART_COUNT_MAX, "Max Hearts: %d")) {
        gSaveContext.save.saveInfo.playerData.healthCapacity = heartCount * 16;
        gSaveContext.save.saveInfo.playerData.health = MIN(gSaveContext.save.saveInfo.playerData.health, gSaveContext.save.saveInfo.playerData.healthCapacity);
    }
    ImGui::SliderScalar("##healthSlider", ImGuiDataType_S16, &gSaveContext.save.saveInfo.playerData.health, &S16_ZERO, &gSaveContext.save.saveInfo.playerData.healthCapacity, "Health: %d");
    UIWidgets::PopStyleSlider();
    ImGui::EndGroup();

    // Time skip buttons
    ImGui::SameLine();
    ImGui::BeginGroup();
    static const std::array<std::pair<int8_t, const char*>, 4> timeSkipAmounts = {
        { { -60, "-1hr" }, { -15, "-15m" }, { 15, "+15m" }, { 60, "+1hr" } }
    };
    for (size_t i = 0; i < timeSkipAmounts.size(); i++) {
        const auto& skip = timeSkipAmounts.at(i);
        if (UIWidgets::Button(skip.second, { .color = UIWidgets::Colors::Indigo, .size = UIWidgets::Sizes::Inline })) {
            gSaveContext.save.time += CLOCK_TIME(0, skip.first);
        }
        if (i < timeSkipAmounts.size() - 1) {
            ImGui::SameLine();
        }
    }
    // Day slider
    UIWidgets::PushStyleSlider();
    static s32 minDay = 1;
    static s32 maxDay = 3;
    if (ImGui::SliderScalar("##dayInput", ImGuiDataType_S32, &gSaveContext.save.day, &minDay, &maxDay, "Day: %d")) {
        gSaveContext.save.eventDayCount = CURRENT_DAY;
        // Reset the time to the start of day/night
        if (gSaveContext.save.time < CLOCK_TIME(6, 0) || gSaveContext.save.time > CLOCK_TIME(18, 0)) {
            gSaveContext.save.time = CLOCK_TIME(18, 1);
        } else {
            gSaveContext.save.time = CLOCK_TIME(6, 1);
        }
        if (gPlayState != nullptr) {
            Interface_NewDay(gPlayState, CURRENT_DAY);
            // Inverting setup actors forces actors to kill/respawn for new day
            gPlayState->numSetupActors = -gPlayState->numSetupActors;
        }
    }
    // Time speed slider
    // Values are added to R_TIME_SPEED which is generally 3 in normal play state
    static s32 minSpeed = -3; // Time frozen
    static s32 maxSpeed = 7;
    std::string timeSpeedString = "Time Speed: " + std::to_string(gSaveContext.save.timeSpeedOffset + 3);
    ImGui::SliderScalar("##timeSpeedInput", ImGuiDataType_S32, &gSaveContext.save.timeSpeedOffset, &minSpeed, &maxSpeed, timeSpeedString.c_str());
    UIWidgets::PopStyleSlider();
    ImGui::EndGroup();

    ImGui::BeginGroup();
    if (UIWidgets::Button("Max Magic", { .color = UIWidgets::Colors::DarkGreen, .size = UIWidgets::Sizes::Inline })) {
        gSaveContext.magicCapacity = gSaveContext.save.saveInfo.playerData.magic = MAGIC_DOUBLE_METER;
        gSaveContext.save.saveInfo.playerData.magicLevel = 2;
        gSaveContext.save.saveInfo.playerData.isMagicAcquired = true;
        gSaveContext.save.saveInfo.playerData.isDoubleMagicAcquired = true;
    }
    ImGui::SameLine();
    if (UIWidgets::Button("Reset##resetMagicButton", { .color = UIWidgets::Colors::Gray, .size = UIWidgets::Sizes::Inline })) {
        gSaveContext.magicCapacity = gSaveContext.save.saveInfo.playerData.magic = 0;
        gSaveContext.save.saveInfo.playerData.magicLevel = 0;
        gSaveContext.save.saveInfo.playerData.isMagicAcquired = false;
        gSaveContext.save.saveInfo.playerData.isDoubleMagicAcquired = false;
    }
    UIWidgets::PushStyleSlider(UIWidgets::Colors::DarkGreen);
    if (ImGui::SliderScalar("##magicLevelSlider", ImGuiDataType_S8, &gSaveContext.save.saveInfo.playerData.magicLevel, &S8_ZERO, &MAGIC_LEVEL_MAX, MAGIC_LEVEL_NAMES[gSaveContext.save.saveInfo.playerData.magicLevel])) {
        gSaveContext.magicCapacity = gSaveContext.save.saveInfo.playerData.magicLevel * MAGIC_NORMAL_METER;
        gSaveContext.save.saveInfo.playerData.magic = MIN(gSaveContext.save.saveInfo.playerData.magic, gSaveContext.magicCapacity);
        switch (gSaveContext.save.saveInfo.playerData.magicLevel) {
            case 0:
                gSaveContext.save.saveInfo.playerData.isMagicAcquired = false;
                gSaveContext.save.saveInfo.playerData.isDoubleMagicAcquired = false;
                break;
            case 1:
                gSaveContext.save.saveInfo.playerData.isMagicAcquired = true;
                gSaveContext.save.saveInfo.playerData.isDoubleMagicAcquired = false;
                break;
            case 2:
                gSaveContext.save.saveInfo.playerData.isMagicAcquired = true;
                gSaveContext.save.saveInfo.playerData.isDoubleMagicAcquired = true;
                break;
        }
    }
    ImGui::SliderScalar("##magicSlider", ImGuiDataType_S8, &gSaveContext.save.saveInfo.playerData.magic, &S8_ZERO, &gSaveContext.magicCapacity, "Magic: %d");
    UIWidgets::PopStyleSlider();
    ImGui::EndGroup();

    ImGui::SameLine();

    ImGui::BeginGroup();
    if (UIWidgets::Button("Max Rupees", { .color = UIWidgets::Colors::Green, .size = UIWidgets::Sizes::Inline })) {
        Inventory_ChangeUpgrade(UPG_WALLET, 2);
        gSaveContext.save.saveInfo.playerData.rupees = CUR_CAPACITY(UPG_WALLET);
    }
    ImGui::SameLine();
    if (UIWidgets::Button("Reset##resetRupeesButton", { .color = UIWidgets::Colors::Gray, .size = UIWidgets::Sizes::Inline })) {
        gSaveContext.save.saveInfo.playerData.rupees = 0;
        Inventory_ChangeUpgrade(UPG_WALLET, 0);
    }
    UIWidgets::PushStyleSlider(UIWidgets::Colors::Green);
    u8 currentWalletLevel = CUR_UPG_VALUE(UPG_WALLET);
    if (ImGui::SliderScalar("##walletLevelSlider", ImGuiDataType_U8, &currentWalletLevel, &U8_ZERO, &WALLET_LEVEL_MAX, WALLET_LEVEL_NAMES[currentWalletLevel])) {
        Inventory_ChangeUpgrade(UPG_WALLET, currentWalletLevel);
        gSaveContext.save.saveInfo.playerData.rupees = MIN(gSaveContext.save.saveInfo.playerData.rupees, CUR_CAPACITY(UPG_WALLET));
    }
    s16 walletCapacity = CUR_CAPACITY(UPG_WALLET);
    ImGui::SliderScalar("##rupeesSlider", ImGuiDataType_S16, &gSaveContext.save.saveInfo.playerData.rupees, &S16_ZERO, &walletCapacity, "Rupees: %d");
    UIWidgets::PopStyleSlider();
    ImGui::EndGroup();

    ImGui::BeginGroup();
    ImGui::Text("Bombers Code: %d %d %d %d %d",
        gSaveContext.save.saveInfo.bomberCode[0],
        gSaveContext.save.saveInfo.bomberCode[1],
        gSaveContext.save.saveInfo.bomberCode[2],
        gSaveContext.save.saveInfo.bomberCode[3],
        gSaveContext.save.saveInfo.bomberCode[4]
    );

    ImGui::Text("Banked Rupees: %d", HS_GET_BANK_RUPEES());
    UIWidgets::Checkbox("Snowhead Cleared", (bool*)&gSaveContext.save.snowheadCleared, { .color = UIWidgets::Colors::Gray });
    UIWidgets::Checkbox("Has Tatl", (bool*)&gSaveContext.save.hasTatl, { .color = UIWidgets::Colors::Gray });
    ImGui::EndGroup();

    ImGui::PopItemWidth();
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

template<typename C, typename T>
bool contains(C&& c, T e) { 
    return std::find(std::begin(c), std::end(c), e) != std::end(c);
};

void DrawAmmoInput(InventorySlot slot) {
    int x = slot % 6;
    int y = ((int)floor(slot / 6) % 4);
    ItemId currentItemId = static_cast<ItemId>(gSaveContext.save.saveInfo.inventory.items[slot]);

    ImGui::SetCursorPos(ImVec2(x * INV_GRID_WIDTH + INV_GRID_PADDING + 7.0f, y * INV_GRID_HEIGHT + INV_GRID_TOP_MARGIN + INV_GRID_PADDING + (INV_GRID_ICON_SIZE - 2.0f)));
    ImGui::PushItemWidth(24.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, UIWidgets::Colors::Gray);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));
    if (ImGui::InputScalar("##ammoInput", ImGuiDataType_S8, &AMMO(currentItemId))) {
        // Crashes when < 0 and graphical issues when > 99
        AMMO(currentItemId) = MAX(0, MIN(AMMO(currentItemId), 99));
    }
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(1);
    ImGui::PopItemWidth();
}

void DrawEquipItemMenu(InventorySlot slot) {
    ItemId currentItemId = static_cast<ItemId>(gSaveContext.save.saveInfo.inventory.items[slot]);

    if (ImGui::BeginMenu("Equip")) {
        if (ImGui::MenuItem("C-Left")) {
            gSaveContext.save.saveInfo.equips.buttonItems[CUR_FORM][EQUIP_SLOT_C_LEFT] = currentItemId;
            gSaveContext.save.saveInfo.equips.cButtonSlots[CUR_FORM][EQUIP_SLOT_C_LEFT] = slot;
            Interface_LoadItemIconImpl(gPlayState, EQUIP_SLOT_C_LEFT);
        }
        if (ImGui::MenuItem("C-Down")) {
            gSaveContext.save.saveInfo.equips.buttonItems[CUR_FORM][EQUIP_SLOT_C_DOWN] = currentItemId;
            gSaveContext.save.saveInfo.equips.cButtonSlots[CUR_FORM][EQUIP_SLOT_C_DOWN] = slot;
            Interface_LoadItemIconImpl(gPlayState, EQUIP_SLOT_C_DOWN);
        }
        if (ImGui::MenuItem("C-Right")) {
            gSaveContext.save.saveInfo.equips.buttonItems[CUR_FORM][EQUIP_SLOT_C_RIGHT] = currentItemId;
            gSaveContext.save.saveInfo.equips.cButtonSlots[CUR_FORM][EQUIP_SLOT_C_RIGHT] = slot;
            Interface_LoadItemIconImpl(gPlayState, EQUIP_SLOT_C_RIGHT);
        }
        if (ImGui::MenuItem("D-Up")) {
            gSaveContext.save.additionalSaveInfo.dpadEquips.dpadItems[CUR_FORM][EQUIP_SLOT_D_RIGHT] = currentItemId;
            gSaveContext.save.additionalSaveInfo.dpadEquips.dpadSlots[CUR_FORM][EQUIP_SLOT_D_RIGHT] = slot;
            Interface_Dpad_LoadItemIconImpl(gPlayState, EQUIP_SLOT_D_RIGHT);
        }
        if (ImGui::MenuItem("D-Down")) {
            gSaveContext.save.additionalSaveInfo.dpadEquips.dpadItems[CUR_FORM][EQUIP_SLOT_D_LEFT] = currentItemId;
            gSaveContext.save.additionalSaveInfo.dpadEquips.dpadSlots[CUR_FORM][EQUIP_SLOT_D_LEFT] = slot;
            Interface_Dpad_LoadItemIconImpl(gPlayState, EQUIP_SLOT_D_LEFT);
        }
        if (ImGui::MenuItem("D-Left")) {
            gSaveContext.save.additionalSaveInfo.dpadEquips.dpadItems[CUR_FORM][EQUIP_SLOT_D_DOWN] = currentItemId;
            gSaveContext.save.additionalSaveInfo.dpadEquips.dpadSlots[CUR_FORM][EQUIP_SLOT_D_DOWN] = slot;
            Interface_Dpad_LoadItemIconImpl(gPlayState, EQUIP_SLOT_D_DOWN);
        }
        if (ImGui::MenuItem("D-Right")) {
            gSaveContext.save.additionalSaveInfo.dpadEquips.dpadItems[CUR_FORM][EQUIP_SLOT_D_UP] = currentItemId;
            gSaveContext.save.additionalSaveInfo.dpadEquips.dpadSlots[CUR_FORM][EQUIP_SLOT_D_UP] = slot;
            Interface_Dpad_LoadItemIconImpl(gPlayState, EQUIP_SLOT_D_UP);
        }
        ImGui::EndMenu();
    }
}

void NextItemInSlot(InventorySlot slot) {
    ItemId currentItemId = static_cast<ItemId>(gSaveContext.save.saveInfo.inventory.items[slot]);
    size_t currentItemIndex = find(safeItemsForInventorySlot[slot].begin(), safeItemsForInventorySlot[slot].end(), currentItemId) - safeItemsForInventorySlot[slot].begin();

    if (currentItemId == ITEM_NONE) {
        gSaveContext.save.saveInfo.inventory.items[slot] = safeItemsForInventorySlot[slot][0];

        if (gSaveContext.save.saveInfo.inventory.items[slot] == ITEM_BOW) {
            Inventory_ChangeUpgrade(UPG_QUIVER, 1);
            AMMO(ITEM_BOW) = CUR_CAPACITY(UPG_QUIVER);
        } else if (gSaveContext.save.saveInfo.inventory.items[slot] == ITEM_BOMB || gSaveContext.save.saveInfo.inventory.items[slot] == ITEM_BOMBCHU) {
            if (CUR_UPG_VALUE(UPG_BOMB_BAG) == 0) {
                Inventory_ChangeUpgrade(UPG_BOMB_BAG, 1);
            }
            AMMO(gSaveContext.save.saveInfo.inventory.items[slot]) = CUR_CAPACITY(UPG_BOMB_BAG);
        }
    } else if (currentItemId == ITEM_BOW) {
        if (CUR_UPG_VALUE(UPG_QUIVER) < 3) {
            Inventory_ChangeUpgrade(UPG_QUIVER, CUR_UPG_VALUE(UPG_QUIVER) + 1);
            AMMO(ITEM_BOW) = CUR_CAPACITY(UPG_QUIVER);
        } else {
            Inventory_DeleteItem(gSaveContext.save.saveInfo.inventory.items[slot], slot);
            Inventory_ChangeUpgrade(UPG_QUIVER, 0);
            AMMO(ITEM_BOW) = 0;
        }
    } else if (currentItemId == ITEM_BOMB || currentItemId == ITEM_BOMBCHU) {
        if (CUR_UPG_VALUE(UPG_BOMB_BAG) < 3) {
            Inventory_ChangeUpgrade(UPG_BOMB_BAG, CUR_UPG_VALUE(UPG_BOMB_BAG) + 1);
            AMMO(ITEM_BOMB) = AMMO(ITEM_BOMBCHU) = CUR_CAPACITY(UPG_BOMB_BAG);
        } else {
            Inventory_DeleteItem(ITEM_BOMB, SLOT_BOMB);
            Inventory_DeleteItem(ITEM_BOMBCHU, SLOT_BOMBCHU);
            Inventory_ChangeUpgrade(UPG_BOMB_BAG, 0);
            AMMO(ITEM_BOMB) = AMMO(ITEM_BOMBCHU) = 0;
        }
    } else if (currentItemIndex < safeItemsForInventorySlot[slot].size() - 1) {
        Inventory_ReplaceItem(gPlayState, currentItemId, safeItemsForInventorySlot[slot][currentItemIndex + 1]);
    } else {
        Inventory_DeleteItem(gSaveContext.save.saveInfo.inventory.items[slot], slot);
    }
}

void DrawSlot(InventorySlot slot) {
    int x = slot % 6;
    int y = ((int)floor(slot / 6) % 4);
    ItemId currentItemId = static_cast<ItemId>(gSaveContext.save.saveInfo.inventory.items[slot]);

    ImGui::PushID(slot);

    if (
        currentItemId != ITEM_NONE &&
        currentItemId <= ITEM_BOW_LIGHT && // gItemSlots only has entries till 77 (ITEM_BOW_LIGHT)
        gItemSlots[currentItemId] <= SLOT_BOTTLE_6 && // There is only ammo data for the first page
        (
            (safeMode && gAmmoItems[gItemSlots[currentItemId]] != ITEM_NONE) || !safeMode
        )
    ) {
        DrawAmmoInput(slot);
    }

    ImGui::SetCursorPos(ImVec2(x * INV_GRID_WIDTH + INV_GRID_PADDING, y * INV_GRID_HEIGHT + INV_GRID_TOP_MARGIN + INV_GRID_PADDING));

    // isEquipped border
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.1f));
    if (
        gSaveContext.save.saveInfo.equips.cButtonSlots[CUR_FORM][EQUIP_SLOT_C_LEFT] == slot ||
        gSaveContext.save.saveInfo.equips.cButtonSlots[CUR_FORM][EQUIP_SLOT_C_DOWN] == slot ||
        gSaveContext.save.saveInfo.equips.cButtonSlots[CUR_FORM][EQUIP_SLOT_C_RIGHT] == slot
    ) {
        ImGui::PushStyleColor(ImGuiCol_Border, UIWidgets::Colors::White);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
    }
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);

    ImTextureID textureId = LUS::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName((const char*)gItemIcons[safeItemsForInventorySlot[slot][0]]);

    if (currentItemId != ITEM_NONE) {
        textureId = LUS::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName((const char*)gItemIcons[currentItemId]);
    }

    if (ImGui::ImageButton(textureId, ImVec2(INV_GRID_ICON_SIZE, INV_GRID_ICON_SIZE), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, currentItemId == ITEM_NONE ? 0.4f : 1.0f))) {
        if (safeMode && safeItemsForInventorySlot[slot].size() < 2) {
            NextItemInSlot(slot);
        } else {
            selectedInventorySlot = slot;
            ImGui::OpenPopup("InventorySlotPopup");
        }
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);

    if (ImGui::BeginPopup("InventorySlotPopup")) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.1f));
        if (ImGui::Button("##invNonePicker", ImVec2(INV_GRID_ICON_SIZE, INV_GRID_ICON_SIZE))) {
            Inventory_DeleteItem(gSaveContext.save.saveInfo.inventory.items[selectedInventorySlot], selectedInventorySlot);
            ImGui::CloseCurrentPopup();
        }
        UIWidgets::Tooltip("None");

        size_t availableItems = safeMode ? safeItemsForInventorySlot[selectedInventorySlot].size() : ITEM_FISHING_ROD;
        for (int32_t pickerIndex = 0; pickerIndex < availableItems; pickerIndex++) {
            if (((pickerIndex + 1) % 8) != 0) {
                ImGui::SameLine();
            }
            ItemId id = safeMode ? safeItemsForInventorySlot[selectedInventorySlot][pickerIndex] : static_cast<ItemId>(pickerIndex);
            if (ImGui::ImageButton(LUS::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName((const char*)gItemIcons[id]), ImVec2(INV_GRID_ICON_SIZE, INV_GRID_ICON_SIZE), ImVec2(0, 0), ImVec2(1, 1), 0)) {
                gSaveContext.save.saveInfo.inventory.items[selectedInventorySlot] = id;
                ImGui::CloseCurrentPopup();
            }
            // TODO: Add names for items
            // UIWidgets::SetLastItemHoverText(SohUtils::GetItemName(id));
        }
        ImGui::PopStyleColor(3);
        ImGui::EndPopup();
    }

    ImGui::PopID();

    if (currentItemId != ITEM_NONE && ImGui::BeginPopupContextItem()) {
        DrawEquipItemMenu(slot);
        ImGui::EndPopup();
    }
}

void DrawItemsAndMasksTab() {
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 8.0f));
    // Group left side in a child for a scroll bar
    ImGui::BeginChild("leftSide##items", ImVec2(0, 0), ImGuiChildFlags_AutoResizeX);
    ImGui::BeginGroup();
    ImGui::BeginChild("itemsBox", ImVec2(INV_GRID_WIDTH * 6 + INV_GRID_PADDING * 2, INV_GRID_HEIGHT * 4 + INV_GRID_PADDING * 2 + INV_GRID_TOP_MARGIN), ImGuiChildFlags_Border);
    ImGui::Text("Items");
    for (uint32_t i = SLOT_OCARINA; i <= SLOT_BOTTLE_6; i++) {
        InventorySlot slot = static_cast<InventorySlot>(i);

        DrawSlot(slot);
    }
    ImGui::EndChild();
    ImGui::BeginChild("masksBox", ImVec2(INV_GRID_WIDTH * 6 + INV_GRID_PADDING * 2, INV_GRID_HEIGHT * 4 + INV_GRID_PADDING * 2 + INV_GRID_TOP_MARGIN), ImGuiChildFlags_Border);
    ImGui::Text("Masks");
    for (uint32_t i = SLOT_MASK_POSTMAN; i <= SLOT_MASK_FIERCE_DEITY; i++) {
        InventorySlot slot = static_cast<InventorySlot>(i);

        DrawSlot(slot);
    }
    ImGui::EndChild();
    ImGui::EndGroup();
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("equipsBox", ImVec2(0, 0), true);
    if (UIWidgets::Button("Give All##items")) {
        for (int32_t i = 0; i <= SLOT_MASK_FIERCE_DEITY; i++) {
            if (i >= SLOT_BOTTLE_1 && i <= SLOT_BOTTLE_6) {
                gSaveContext.save.saveInfo.inventory.items[i] = ITEM_BOTTLE;
            } else {
                gSaveContext.save.saveInfo.inventory.items[i] = safeItemsForInventorySlot[i].back();
            }
        }
        Inventory_ChangeUpgrade(UPG_BOMB_BAG, 3);
        Inventory_ChangeUpgrade(UPG_QUIVER, 3);
        AMMO(ITEM_BOW) = CUR_CAPACITY(UPG_QUIVER);
        AMMO(ITEM_BOMB) = AMMO(ITEM_BOMBCHU) = CUR_CAPACITY(UPG_BOMB_BAG);
        AMMO(ITEM_DEKU_STICK) = CUR_CAPACITY(UPG_DEKU_STICKS);
        AMMO(ITEM_DEKU_NUT) = CUR_CAPACITY(UPG_DEKU_NUTS);
        AMMO(ITEM_MAGIC_BEANS) = 20;
        AMMO(ITEM_POWDER_KEG) = 1;
    }
    if (UIWidgets::Button("Reset##items")) {
        for (int32_t i = 0; i <= SLOT_MASK_FIERCE_DEITY; i++) {
            Inventory_DeleteItem(gSaveContext.save.saveInfo.inventory.items[i], i);
        }
        AMMO(ITEM_BOW) = AMMO(ITEM_BOMB) = AMMO(ITEM_BOMBCHU) = AMMO(ITEM_DEKU_STICK) = AMMO(ITEM_DEKU_NUT) = AMMO(ITEM_MAGIC_BEANS) = AMMO(ITEM_POWDER_KEG) = 0;
        Inventory_ChangeUpgrade(UPG_BOMB_BAG, 0);
        Inventory_ChangeUpgrade(UPG_QUIVER, 0);
    }
    UIWidgets::Checkbox("Safe Mode", &safeMode);
    
    // Expose inputs to edit raw number values of equips
    // ImGui::Text("Equips");
    // ImGui::Text("C-Buttons");
    // ImGui::Text("D-Pad");
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

const char* regGroupNames[] = {
    "REG  (0)",
    "SREG (1)",
    "OREG (2)",
    "PREG (3)",
    "QREG (4)",
    "MREG (5)",
    "YREG (6)",
    "DREG (7)",
    "UREG (8)",
    "IREG (9)",
    "ZREG (10)",
    "CREG (11)",
    "NREG (12)",
    "KREG (13)",
    "XREG (14)",
    "cREG (15)",
    "sREG (16)",
    "iREG (17)",
    "WREG (18)",
    "AREG (19)",
    "VREG (20)",
    "HREG (21)",
    "GREG (22)",
    "mREG (23)",
    "nREG (24)",
    "BREG (25)",
    "dREG (26)",
    "kREG (27)",
    "bREG (28)",
};

void DrawRegEditorTab() {
    UIWidgets::PushStyleSlider();
    ImGui::SliderScalar("Reg Group", ImGuiDataType_U8, &gRegEditor->regGroup, &S8_ZERO, &REG_GROUPS_MAX, regGroupNames[gRegEditor->regGroup]);
    ImGui::SliderScalar("Reg Page", ImGuiDataType_U8, &gRegEditor->regPage, &S8_ZERO, &REG_PAGES_MAX, "%d");
    UIWidgets::PopStyleSlider();

    ImGui::BeginChild("regSliders", ImVec2(0, 0), ImGuiChildFlags_Border);

    for (int i = 0; i < REG_PER_PAGE; i++) {
        ImGui::PushID(i);
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f - 4.0f);
        ImGui::BeginGroup();
        ImGui::Text("%02X (%d)", i + gRegEditor->regPage * REG_PER_PAGE, i + gRegEditor->regPage * REG_PER_PAGE);
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_FrameBg, UIWidgets::Colors::Gray);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 8.0f));
        ImGui::InputScalar("##regInput", ImGuiDataType_S16, &gRegEditor->data[i + gRegEditor->regPage * REG_PER_PAGE + gRegEditor->regGroup * REG_PER_PAGE * REG_PAGES], NULL, NULL, "%d");
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(1);
        ImGui::EndGroup();
        ImGui::PopItemWidth();
        ImGui::PopID();
    }

    ImGui::EndChild();
}

void SaveEditorWindow::DrawElement() {
    ImGui::SetNextWindowSize(ImVec2(480,600), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Save Editor", &mIsVisible, ImGuiWindowFlags_NoFocusOnAppearing)) {
        ImGui::End();
        return;
    }

    if (ImGui::BeginTabBar("SaveContextTabBar", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton)) {
        if (ImGui::BeginTabItem("General")) {
            DrawGeneralTab();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Items & Masks")) {
            DrawItemsAndMasksTab();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Reg Editor")) {
            DrawRegEditorTab();
            ImGui::EndTabItem();
        }

        // if (ImGui::BeginTabItem("Equipment")) {
        //     DrawEquipmentTab();
        //     ImGui::EndTabItem();
        // }

        // if (ImGui::BeginTabItem("Quest Status")) {
        //     DrawQuestStatusTab();
        //     ImGui::EndTabItem();
        // }

        // if (ImGui::BeginTabItem("Flags")) {
        //     DrawFlagsTab();
        //     ImGui::EndTabItem();
        // }

        // if (ImGui::BeginTabItem("Player")) {
        //     DrawPlayerTab();
        //     ImGui::EndTabItem();
        // }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void SaveEditorWindow::InitElement() {
    initSafeItemsForInventorySlot();

    for (TexturePtr entry : gItemIcons) {
        const char* path = static_cast<const char*>(entry);
        LUS::Context::GetInstance()->GetWindow()->GetGui()->LoadGuiTexture(path, path, ImVec4(1, 1, 1, 1));
    }
}
