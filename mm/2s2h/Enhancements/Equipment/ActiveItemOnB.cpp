#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "z64interface.h"
#include "variables.h"

void Player_UseItem(PlayState* play, Player* player, ItemId item);
}

constexpr bool IsFirstPersonItem(ItemId item) {
    return item != ITEM_NONE && (
        item == ITEM_BOW
        || item == ITEM_BOW_FIRE
        || item == ITEM_BOW_ICE
        || item == ITEM_BOW_LIGHT
        || item == ITEM_HOOKSHOT
    );
}

constexpr bool IsThirdPersonItem(ItemId item) {
    return item != ITEM_NONE && (
        item == ITEM_DEKU_STICK
        || item == ITEM_SWORD_GREAT_FAIRY
    );
}

constexpr bool IsItemInScope(ItemId item) {
    return item != ITEM_NONE && (IsFirstPersonItem(item) || IsThirdPersonItem(item));
}

constexpr bool PlayerHoldsItem(Player* player) {
    return (player->heldItemAction != PLAYER_IA_NONE && (ItemId)player->heldItemId != ITEM_NONE);
}

constexpr bool IsAimingWithFirstPersonItem(Player* player) {
    return IsItemInScope((ItemId)player->heldItemId) && (
        player->unk_AA5 == PLAYER_UNKAA5_3
        || (
            player->unk_AA5 == PLAYER_UNKAA5_0
            && (player->stateFlags1 & PLAYER_STATE1_PARALLEL)
            && player->focusActor == NULL
        )
    );
}

constexpr bool IsHoldingScoped(Player* player) {
    return (
        PlayerHoldsItem(player)
        && (IsThirdPersonItem((ItemId)player->heldItemId) || IsAimingWithFirstPersonItem(player))
    );
}

static void SwitchButtonIcon(EquipSlot slot, ItemId icon) {
    Interface_LoadItemIconImpl(gPlayState, slot);
    if (icon < ARRAY_COUNT(gItemIcons)) {
        gPlayState->interfaceCtx.iconItemSegment[slot] = (char*)gItemIcons[icon];
    }
}

static void HandleGetItemOnButton(bool* should, EquipSlot slot, ItemId* pressedItem) {
    Player* player = GET_PLAYER(gPlayState);

    if (player->transformation != PLAYER_FORM_HUMAN) { return; }
    if (
        player->currentMask == PLAYER_MASK_BLAST
        || player->currentMask == PLAYER_MASK_BREMEN
        || player->currentMask == PLAYER_MASK_KAMARO
    ) { return; }

    if (slot == EQUIP_SLOT_B && IsHoldingScoped(player)) {
        // fire!
        *pressedItem = (ItemId)player->heldItemId;
        *should = false;
    } else if (IsItemInScope(*pressedItem) && IsHoldingScoped(player)) {
        // strip item
        *pressedItem = ITEM_NONE;
        *should = false;
        if (player->heldItemAction > PLAYER_IA_LAST_USED) {
            Player_UseItem(gPlayState, player, ITEM_NONE);
        }
    }

    if (IsItemInScope(*pressedItem)) {
        SwitchButtonIcon(EQUIP_SLOT_B, *pressedItem);
    } else if (IsHoldingScoped(player)) {
        SwitchButtonIcon(EQUIP_SLOT_B, (ItemId)player->heldItemId);
    }
}

void RegisterActiveItemOnB() {
    COND_VB_SHOULD(
        VB_GET_ITEM_ON_BUTTON,
        CVarGetInteger("gEnhancements.Equipment.ActiveItemOnB", 0),
        {
            EquipSlot slot = (EquipSlot)va_arg(args, int);
            ItemId* item = va_arg(args, ItemId*);
            HandleGetItemOnButton(should, slot, item);
        }
    );
}

static RegisterShipInitFunc initFunc(RegisterActiveItemOnB, { "gEnhancements.Equipment.ActiveItemOnB" });
