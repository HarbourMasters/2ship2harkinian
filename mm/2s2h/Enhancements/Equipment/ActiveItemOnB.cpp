#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "z64interface.h"
#include "variables.h"

void Player_UseItem(PlayState* play, Player* player, ItemId item);
}

constexpr bool IsItemInScope(ItemId item) {
    return item != ITEM_NONE && (
        item == ITEM_BOW
        || item == ITEM_BOW_FIRE
        || item == ITEM_BOW_ICE
        || item == ITEM_BOW_LIGHT
        || item == ITEM_HOOKSHOT
        || item == ITEM_DEKU_STICK
        || item == ITEM_SWORD_GREAT_FAIRY
    );
}

constexpr bool PlayerHoldsItem(Player* player) {
    return (player->heldItemAction != PLAYER_IA_NONE && (ItemId)player->heldItemId != ITEM_NONE);
}

// constexpr bool IsAiming(Player* player) {
//     return (
//         player->unk_AA5 == PLAYER_UNKAA5_3
//         || (
//             player->unk_AA5 == PLAYER_UNKAA5_0
//             && (player->stateFlags1 & PLAYER_STATE1_PARALLEL)
//             && player->focusActor == NULL
//         )
//     );
// }

constexpr bool IsHoldingScoped(Player* player) {
    return PlayerHoldsItem(player) && IsItemInScope((ItemId)player->heldItemId);
}

static void SwitchButtonIcon(EquipSlot slot, ItemId icon) {
    if (icon == ITEM_NONE) { return; }

    Interface_LoadItemIconImpl(gPlayState, slot);
    if (icon < ARRAY_COUNT(gItemIcons)) {
        gPlayState->interfaceCtx.iconItemSegment[slot] = (char*)gItemIcons[icon];
    }
}

static struct ButtonState {
    ItemId stored = ITEM_NONE;
    ItemId override = ITEM_NONE;
} mBButtonState;

static void HandleGetItemOnButton(bool* should, EquipSlot slot, ItemId* pressedItem) {
    Player* player = GET_PLAYER(gPlayState);

    if (player->transformation != PLAYER_FORM_HUMAN) { return; }
    if (
        player->currentMask == PLAYER_MASK_BLAST
        || player->currentMask == PLAYER_MASK_BREMEN
        || player->currentMask == PLAYER_MASK_KAMARO
    ) { return; }

    if (slot == EQUIP_SLOT_B) {
        if (IsHoldingScoped(player)) {
            ItemId heldItem = (ItemId)player->heldItemId;

            mBButtonState.stored = (ItemId)BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B);
            mBButtonState.override = heldItem;
            BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) = mBButtonState.override;

            *pressedItem = heldItem;
        }
    } else if (IsItemInScope(*pressedItem)) {
        if (IsHoldingScoped(player)) {
            // put away
            if (player->heldItemAction > PLAYER_IA_LAST_USED) {
                Player_UseItem(gPlayState, player, ITEM_NONE);
            }
            SwitchButtonIcon(EQUIP_SLOT_B, mBButtonState.stored);
            *pressedItem = ITEM_NONE;
        } else {
            // view held item on B button
            SwitchButtonIcon(EQUIP_SLOT_B, *pressedItem);
        }
    }
}

void RestoreBButtonItem(Actor* actor) {
    if (mBButtonState.override == ITEM_NONE) { return; }
    // assert(mBButtonState.stored != ITEM_NONE);
    ItemId* item = (ItemId*)&BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B);

    if (*item != mBButtonState.override) {
        mBButtonState.stored = *item;
    } else {
        *item = mBButtonState.stored;
    }
    mBButtonState.override = ITEM_NONE;
}

void RegisterActiveItemOnB() {
    if (!CVarGetInteger("gEnhancements.Equipment.ActiveItemOnB", 0)) {
        SwitchButtonIcon(EQUIP_SLOT_B, mBButtonState.stored);
        mBButtonState.stored = ITEM_NONE;
    }
    COND_VB_SHOULD(
        VB_GET_ITEM_ON_BUTTON,
        CVarGetInteger("gEnhancements.Equipment.ActiveItemOnB", 0),
        {
            EquipSlot slot = (EquipSlot)va_arg(args, int);
            ItemId* item = va_arg(args, ItemId*);
            HandleGetItemOnButton(should, slot, item);
        }
    );
    COND_ID_HOOK(
        OnActorUpdate, ACTOR_PLAYER,
        CVarGetInteger("gEnhancements.Equipment.ActiveItemOnB", 0),
        RestoreBButtonItem
    );
}

static RegisterShipInitFunc initFunc(RegisterActiveItemOnB, { "gEnhancements.Equipment.ActiveItemOnB" });
