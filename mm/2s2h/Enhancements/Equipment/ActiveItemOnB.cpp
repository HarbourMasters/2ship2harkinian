#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "variables.h"
void Interface_LoadItemIconImpl(struct PlayState* play, u8 btn);
extern Input* sPlayerControlInput;
}

#define CVAR_NAME "gEnhancements.Equipment.ActiveItemOnB"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

static bool PlayerHasHeldItem(Player* player) {
    return player->heldItemAction != PLAYER_IA_NONE && player->heldItemId != ITEM_NONE;
}

void RegisterActiveItemOnB() {
    COND_VB_SHOULD(VB_GET_ITEM_ON_BUTTON, CVAR, {
        Player* player = GET_PLAYER(gPlayState);
        EquipSlot slot = (EquipSlot)va_arg(args, int);
        ItemId* item = va_arg(args, ItemId*);

        if (slot == EQUIP_SLOT_B && player->transformation == PLAYER_FORM_HUMAN &&
            player->currentMask != PLAYER_MASK_BLAST && player->currentMask != PLAYER_MASK_BREMEN &&
            player->currentMask != PLAYER_MASK_KAMARO) {
            if (PlayerHasHeldItem(player)) {
                *item = (ItemId)player->heldItemId;
            }
        }
    });

    COND_HOOK(OnInterfaceDrawStart, CVAR, []() {
        static ItemId sLastIconItem = ITEM_NONE;
        Player* player = GET_PLAYER(gPlayState);
        if (player == NULL || player->transformation != PLAYER_FORM_HUMAN) {
            if (sLastIconItem != ITEM_NONE) {
                sLastIconItem = ITEM_NONE;
            }
            return;
        }
        ItemId current = PlayerHasHeldItem(player) ? (ItemId)player->heldItemId : ITEM_NONE;
        if (current != sLastIconItem) {
            sLastIconItem = current;
            Interface_LoadItemIconImpl(gPlayState, EQUIP_SLOT_B);
            if (current != ITEM_NONE && current < ARRAY_COUNT(gItemIcons)) {
                gPlayState->interfaceCtx.iconItemSegment[EQUIP_SLOT_B] = (char*)gItemIcons[current];
            }
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterActiveItemOnB, { CVAR_NAME });
