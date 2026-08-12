/**
 * equip_helper.c - Item input and equip state management
 */

#include "equip_helper.h"
#include "../custom_items.h"
#include "../../extended_equipment.h" // MAGIC_REQ — Magic Cape halves all magic costs
#include "macros.h"
#include "functions.h"
#include "variables.h"
#include "libultraship/bridge.h"
#include "transformation_masks/transformation_masks.h"
#include "extended_inventory.h" // Sw97_* — Bomb Arrows rides the bow's element flag (Skijer's NEI)

typedef struct {
    u32 frameCount;
    u8 cachedItems[8];
    u16 cachedButtons[256];
} EquipCache;

static EquipCache sEquipCache = { 0 };

u8 ItemEquip_GetItemOnSlot(u8 slot) {
    // sButtonMasks[4..7] is D-Up, D-Down, D-Left, D-Right — keep this table in that same order.
    static const u8 sDpadSlots[4] = {
        EQUIP_SLOT_D_UP,
        EQUIP_SLOT_D_DOWN,
        EQUIP_SLOT_D_LEFT,
        EQUIP_SLOT_D_RIGHT,
    };

    if (slot == 0) {
        // B is the only per-transformation button.
        return BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B);
    }
    if (slot < 4) {
        // C-button equipment is shared by every transformation.
        return BUTTON_ITEM_EQUIP(0, slot);
    }
    if (slot < 8) {
        if (!CVarGetInteger("gEnhancements.Dpad.DpadEquips", 0)) {
            return ITEM_NONE;
        }
        return DPAD_BUTTON_ITEM_EQUIP(0, sDpadSlots[slot - 4]);
    }
    return ITEM_NONE;
}

static void EquipCache_Update(PlayState* play) {
    if (sEquipCache.frameCount == play->gameplayFrames)
        return;
    sEquipCache.frameCount = play->gameplayFrames;

    for (int i = 0; i < 256; i++)
        sEquipCache.cachedButtons[i] = 0;

    // Slot 0 = B button (sButtonMasks[0] = BTN_B). Start at 0 so custom
    // items equipped to B (e.g. Roc's Feather, transformation masks) get
    // registered alongside C-buttons / D-pad slots. Previously the loop
    // started at slot 1, so B-equipped custom items never received input.
    for (u8 slot = 0; slot < 8; slot++) {
        u8 itemId = ItemEquip_GetItemOnSlot(slot);

        sEquipCache.cachedItems[slot] = itemId;
        if (itemId != ITEM_NONE) {
            sEquipCache.cachedButtons[itemId] = sButtonMasks[slot];
        }
        // Skijer's NEI — Bomb Arrows has no inventory slot and never reaches a button; it is the
        // 7th value of the weapon's element flag. Everything in item_bombarrows.c asks this cache
        // "which button is ITEM_BOMB_ARROWS on?", so aliasing it onto the weapon's button here is
        // what keeps that whole state machine (baButtonMask, press edges, cleanup) working untouched.
        // Both weapons qualify: bomb arrows on the bow, bomb bullets on the slingshot.
        if ((Sw97_IsBowItem(itemId) && (Sw97_EffectiveElement(0) == SW97_ELEM_BOMB)) ||
            (Sw97_IsSlingItem(itemId) && (Sw97_EffectiveElement(1) == SW97_ELEM_BOMB))) {
            sEquipCache.cachedButtons[ITEM_BOMB_ARROWS] = sButtonMasks[slot];
        }
    }
}

u16 ItemInput_GetEquippedButton(u8 itemId, PlayState* play) {
    EquipCache_Update(play);
    return sEquipCache.cachedButtons[itemId];
}

// mods/actors/cane_pacci.c — while Ultrahand mode is up the D-pad rotates and moves
// the held object.
u8 Pacci_UltrahandModeActive(void);

void ItemInput_Update(ItemInputState* out, u8 itemId, Player* player, PlayState* play) {
    out->equippedButton = ItemInput_GetEquippedButton(itemId, play);
    out->wasEquipped = (out->equippedButton != 0);

    // Custom items never go through Player_GetItemOnButton — they find themselves in
    // buttonItems and read the raw pad here — so the guard placed in that engine
    // function did nothing for them. Roc's Cape on a D-pad slot kept firing right
    // through Ultrahand mode because of exactly this second path. An item sitting on
    // the D-pad is simply not usable while the mode owns those buttons.
    if (out->wasEquipped && Pacci_UltrahandModeActive() &&
        (out->equippedButton & (BTN_DUP | BTN_DDOWN | BTN_DLEFT | BTN_DRIGHT))) {
        out->isPressed = out->isHeld = out->isReleased = out->otherButtonPressed = out->damageTaken = 0;
        return;
    }

    if (!out->wasEquipped) {
        out->isPressed = out->isHeld = out->isReleased = out->otherButtonPressed = out->damageTaken = 0;
        return;
    }

    u16 press = play->state.input[0].press.button;
    u16 held = play->state.input[0].cur.button;

    out->isPressed = (press & out->equippedButton) != 0;
    out->isHeld = (held & out->equippedButton) != 0;
    out->isReleased = !out->isHeld && !out->isPressed;
    out->otherButtonPressed = ItemInput_CheckOtherButtons(out->equippedButton, &play->state.input[0]);
    out->damageTaken = 0;
}

u8 ItemInput_CheckDamage(Player* player, s8* prevInvincibility) {
    u8 damage = (player->invincibilityTimer > 0 && *prevInvincibility == 0);
    *prevInvincibility = player->invincibilityTimer;
    return damage;
}

u8 ItemInput_CheckOtherButtons(u16 equippedButton, Input* input) {
    static const u16 sActionButtons = BTN_A | BTN_B | BTN_R | BTN_START | BTN_CLEFT | BTN_CDOWN | BTN_CRIGHT | BTN_DUP |
                                      BTN_DDOWN | BTN_DLEFT | BTN_DRIGHT;
    return (input->press.button & (sActionButtons & ~equippedButton)) != 0;
}

u8 ItemInput_IsBlockedEx(Player* player, PlayState* play, u8 skipOptionalBlockers) {
    // Custom items during transformation: allowed items stay on C-buttons
    // (MmForm_SaveAndRestrictEquips unequips blocked items on transform).
    // If an item is still equipped, the slot allowlist permits it.

    if (player->stateFlags1 & ITEM_BLOCK_STATE1)
        return 1;
    if (player->stateFlags1 & PLAYER_STATE1_START_CHANGING_HELD_ITEM)
        return 1;
    // (MM has no PlayState.shootingGalleryStatus — OoT shooting-gallery item-block guard dropped)

    if (!skipOptionalBlockers) {
        if (player->meleeWeaponState != 0)
            return 1;
        if (player->stateFlags1 & PLAYER_STATE1_SHIELDING)
            return 1;
        if ((player->stateFlags1 & PLAYER_STATE1_IN_WATER) && !(player->actor.bgCheckFlags & 0x0001))
            return 1;
    }

    return 0;
}

u8 ItemInput_IsBlocked(Player* player, PlayState* play) {
    return ItemInput_IsBlockedEx(player, play, 0);
}

void ItemInput_RequestItemChange(Player* player, PlayState* play) {
    if (player->heldItemAction >= 0 && player->heldItemAction != PLAYER_IA_NONE) {
        player->heldItemId = ITEM_NONE;
        player->stateFlags1 |= PLAYER_STATE1_START_CHANGING_HELD_ITEM;
    }
}

u8 ItemInput_CanInterrupt(Player* player) {
    if (player->meleeWeaponState != 0)
        return 0;
    if (player->stateFlags1 & (PLAYER_STATE1_CHARGING_SPIN_ATTACK | PLAYER_STATE1_CARRYING_ACTOR |
                               PLAYER_STATE1_READY_TO_FIRE | PLAYER_STATE1_BOOMERANG_THROWN))
        return 0;
    return 1;
}

void ItemEquip_PlayEquipSFX(PlayState* play, Player* player) {
    Audio_PlaySoundGeneral(NA_SE_PL_CHANGE_ARMS, &player->actor.world.pos, 4, &gSfxDefaultFreqAndVolScale,
                           &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
}

void ItemEquip_PlayUnequipSFX(PlayState* play, Player* player) {
    Audio_PlaySoundGeneral(NA_SE_PL_CHANGE_ARMS, &player->actor.world.pos, 4, &gSfxDefaultFreqAndVolScale,
                           &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
}

u8 ItemEquip_Update(ItemEquipState* state, ItemInputState* input, EquipCallback onEquip, UnequipCallback onUnequip,
                    Player* player, PlayState* play) {
    if (!input->wasEquipped) {
        if (state->isEquipped && onUnequip)
            onUnequip(play, player);
        state->isEquipped = 0;
        return 0;
    }

    if (ItemInput_CheckDamage(player, &state->prevInvincibility)) {
        if (state->isEquipped && onUnequip)
            onUnequip(play, player);
        state->isEquipped = 0;
        return 0;
    }

    if (input->otherButtonPressed) {
        if (state->isEquipped && onUnequip)
            onUnequip(play, player);
        state->isEquipped = 0;
        return 0;
    }

    if (!state->isEquipped && input->isPressed) {
        if (onEquip)
            onEquip(play, player);
        state->isEquipped = 1;
    }

    return state->isEquipped;
}

void ItemMagic_Consume(PlayState* play, s16 amount) {
    amount = MAGIC_REQ(amount); // Magic Cape (ext tunic 1) halves the cost
    if (gSaveContext.save.saveInfo.playerData.magic >= amount)
        gSaveContext.save.saveInfo.playerData.magic -= amount;
}

s32 ItemMagic_HasEnough(PlayState* play, s16 amount) {
    amount = MAGIC_REQ(amount); // Magic Cape (ext tunic 1) halves the requirement
    return (gSaveContext.magicCapacity > 0 && gSaveContext.save.saveInfo.playerData.magic >= amount);
}

u8 ItemSword_HasAnySword(void) {
    // MM tracks the current sword via the equipment bitfield (no per-sword "owned" bits like OoT).
    return (GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) != EQUIP_VALUE_SWORD_NONE);
}

u8 ItemSword_GetCurrentASword(void) {
    u8 aButton = gSaveContext.save.saveInfo.equips.buttonItems[CUR_FORM][0];
    if (aButton == ITEM_SWORD_KOKIRI || aButton == ITEM_SWORD_MASTER || aButton == ITEM_SWORD_BGS ||
        aButton == ITEM_SWORD_KNIFE) {
        return aButton;
    }
    return ITEM_NONE;
}

void ItemSword_EquipKokiriToA(void) {
    gSaveContext.save.saveInfo.equips.buttonItems[CUR_FORM][0] = ITEM_SWORD_KOKIRI;
}

void ItemSword_RestoreA(u8 prevItem) {
    if (prevItem != ITEM_NONE) {
        gSaveContext.save.saveInfo.equips.buttonItems[CUR_FORM][0] = prevItem;
    }
}

u8 ItemHeld_IsActive(Player* player, s32 itemAction) {
    return (player->heldItemAction == itemAction);
}

u16 ItemHeld_GetEquippedButton(u8 itemId, PlayState* play) {
    return ItemInput_GetEquippedButton(itemId, play);
}

u8 ItemHeld_IsButtonHeld(u8 itemId, Player* player, PlayState* play) {
    u16 button = ItemInput_GetEquippedButton(itemId, play);
    if (button == 0)
        return 0;
    return (play->state.input[0].cur.button & button) != 0;
}

u8 ItemHeld_IsButtonPressed(u8 itemId, Player* player, PlayState* play) {
    u16 button = ItemInput_GetEquippedButton(itemId, play);
    if (button == 0)
        return 0;
    return (play->state.input[0].press.button & button) != 0;
}
