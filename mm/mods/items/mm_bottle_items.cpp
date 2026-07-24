// Skijer's NEI — Bottle Randomizer runtime for the two extra items:
//   Net             -> SLOT_BOTTLE_3 (behavior deferred; just projected so it shows/equips)
//   Bottomless Bottle -> SLOT_BOTTLE_4 (a normal bottle slot with a per-content use-counter "ammo")
//
// Design (see custom_bottles.h): the Bottomless Bottle slot ALWAYS holds a real bottle content, so
// catching, drinking and selling (En_Hy) all run through 100% vanilla code. The only added layer is a
// use-counter: each empty (Inventory_UpdateBottleItem with item == ITEM_BOTTLE on SLOT_BOTTLE_4)
// decrements it; while >0 the content is kept (auto-refill), at 0 it becomes an empty Bottomless
// Bottle. Filling (catch) resets the counter to the content's max uses. The empty Bottomless Bottle
// item uses PLAYER_IA_BOTTLE (see extended_player.c) so the empty-bottle catch action triggers.
//
// 2ship port: MM has no "VB_UPDATE_BOTTLE_ITEM" pre-write should-gate. Instead Inventory_UpdateBottleItem
// (z_parameter.c) writes the slot/button FIRST and then fires the OnBottleContentsUpdate(item) hook. So
// the counter driver runs as a POST-write fix-up: when SLOT_BOTTLE_4 was just emptied to ITEM_BOTTLE and
// charges remain, we re-write the content back into the slot + button (undoing the empty). The
// OnInterfaceDrawStart enforcer then keeps the slot icon + C-button consistent each frame.
//
// Two hooks:
//   OnBottleContentsUpdate — intercept fill/empty of SLOT_BOTTLE_4 to drive the counter (post-write).
//   OnInterfaceDrawStart   — project ownership + counter state into inventory.items / buttonItems each
//                            frame (so the slot icon + C-button reflect content / empty-bottomless,
//                            and re-assert the content the frame after a >0 drain).

#include "soh/Enhancements/game-interactor/GameInteractor.h"
#include "soh/Enhancements/game-interactor/GameInteractor_Hooks.h"
#include "soh/ShipInit.hpp"
#include "mods/items/custom_bottles.h"

extern "C" {
#include "z64.h"
#include "functions.h"
#include "variables.h"
extern SaveContext gSaveContext;
extern PlayState* gPlayState;
}

// Sync a bottle slot to `want` and any C-button equipping it (reloading the C-button icon on change).
// 2ship: inventory + equips are nested under save.saveInfo; buttonItems / cButtonSlots are 2D
// [form][button] (use the human-form row 0 — C-buttons are form-agnostic on MM). C-buttons are
// EQUIP_SLOT_C_LEFT(1) / C_DOWN(2) / C_RIGHT(3).
static void BottleItems_SyncSlot(PlayState* play, u8 slot, u8 want) {
    if (gSaveContext.save.saveInfo.inventory.items[slot] != want) {
        gSaveContext.save.saveInfo.inventory.items[slot] = want;
    }
    for (s16 btn = EQUIP_SLOT_C_LEFT; btn <= EQUIP_SLOT_C_RIGHT; btn++) {
        if (C_SLOT_EQUIP(0, btn) == slot && BUTTON_ITEM_EQUIP(0, btn) != want) {
            BUTTON_ITEM_EQUIP(0, btn) = want;
            if (play != NULL) {
                Interface_LoadItemIcon(play, (u8)btn);
            }
        }
    }
}

// A VANILLA bottle item (empty bottle or any content, MM range 0x12 ITEM_BOTTLE .. 0x27
// ITEM_OBABA_DRINK). These are "residue" when sitting raw in the six MM bottle slots — the first four
// belong to the Bottle Randomizer layout [Wheel A][Wheel B][Net][Bottomless] now, and 5/6 feed it.
static u8 BottleItems_IsVanillaBottle(u8 item) {
    return (item >= ITEM_BOTTLE) && (item <= ITEM_OBABA_DRINK);
}

// Move a vanilla bottle item into the wheel inventory (first free bottleSlots entry, preferring the
// given wheel so it stays where the player saw it). 1 = migrated.
static u8 BottleItems_MigrateToWheel(u8 item, u8 preferWheel) {
    int base = (preferWheel == BOTTLE_WHEEL_B) ? 4 : 0;
    for (int k = 0; k < 8; k++) {
        int i = (base + k) % 8;
        if (Bottle_GetSlot((uint8_t)i) == BOTTLE_SLOT_EMPTY) {
            Bottle_SetSlot((uint8_t)i, item);
            return 1;
        }
    }
    return 0; // wheels full — leave the bottle alone (never destroy items)
}

static void BottleItems_Enforce(PlayState* play) {
    // Net catch filled the ACTIVE slot of a wheel — reflect it on the vanilla slot + C-button NOW
    // (out of kaleido the wheels only sync on pause; a visible catch shouldn't wait for that).
    {
        uint8_t w, item;
        if (Bottle_ConsumeCatchSync(&w, &item)) {
            BottleItems_SyncSlot(play, (w == BOTTLE_WHEEL_A) ? SLOT_BOTTLE_1 : SLOT_BOTTLE_2, item);
        }
    }

    // ── Vanilla-bottle residue killer (Skijer's NEI) ─────────────────────────────────────────────
    // Any vanilla bottle the game still hands out (gives, chests, shops, old saves) lands raw in
    // SLOT_BOTTLE_1..6. Detect it, migrate its content into a wheel bottle, and convert the row to
    // the new layout: the first residue also grants Net + Bottomless so the slots become
    // [Wheel A][Wheel B][Net][Bottomless] (+5/6 emptied) and no vanilla remnants fight them again.
    {
        u8 cur3 = gSaveContext.save.saveInfo.inventory.items[SLOT_BOTTLE_3];
        if (cur3 != ITEM_NET && BottleItems_IsVanillaBottle(cur3) &&
            BottleItems_MigrateToWheel(cur3, BOTTLE_WHEEL_A)) {
            gSaveContext.save.saveInfo.inventory.items[SLOT_BOTTLE_3] = ITEM_NONE; // Net refills below
            Bottle_SetNetOwned(1);
            Bottle_SetBottomlessOwned(1);
        }
        // SLOT_BOTTLE_4: only when Bottomless ISN'T owned — once owned, the adopt logic below absorbs
        // any external content into the bottomless counter instead.
        u8 cur4 = gSaveContext.save.saveInfo.inventory.items[SLOT_BOTTLE_4];
        if (!Bottle_BottomlessOwned() && BottleItems_IsVanillaBottle(cur4) &&
            BottleItems_MigrateToWheel(cur4, BOTTLE_WHEEL_B)) {
            gSaveContext.save.saveInfo.inventory.items[SLOT_BOTTLE_4] = ITEM_NONE;
            Bottle_SetNetOwned(1);
            Bottle_SetBottomlessOwned(1);
        }
        // MM's two EXTRA bottle slots (5/6): the randomizer row doesn't use them — anything there is
        // residue too; migrate + keep them empty.
        for (u8 xslot = SLOT_BOTTLE_5; xslot <= SLOT_BOTTLE_6; xslot++) {
            u8 curX = gSaveContext.save.saveInfo.inventory.items[xslot];
            if (BottleItems_IsVanillaBottle(curX) &&
                BottleItems_MigrateToWheel(curX, (xslot == SLOT_BOTTLE_5) ? BOTTLE_WHEEL_A : BOTTLE_WHEEL_B)) {
                gSaveContext.save.saveInfo.inventory.items[xslot] = ITEM_NONE;
                Bottle_SetNetOwned(1);
                Bottle_SetBottomlessOwned(1);
            }
        }
    }

    // ── Wheels A/B: per-frame reconcile (SLOT_BOTTLE_1/2) ────────────────────────────────────────
    // The kaleido used to be the ONLY sync point, so drinks/catches/gives only reconciled on pause.
    // Run the same Persist/resync/RecordActive cycle every gameplay frame. Skipped while paused —
    // the kaleido wheel code runs its own Persist/step/RecordActive there and must not be fought.
    if (play == NULL || play->pauseCtx.state == PAUSE_STATE_OFF) {
        for (int w = 0; w < 2; w++) {
            u8 slot = (w == BOTTLE_WHEEL_A) ? SLOT_BOTTLE_1 : SLOT_BOTTLE_2;
            u16 cur = gSaveContext.save.saveInfo.inventory.items[slot];
            // Drink/refill of the ACTIVE bottle -> persist into the wheel state.
            Bottle_WheelPersist((uint8_t)w, cur);
            // A vanilla bottle the wheel doesn't know = residue (give/chest/old save): migrate it.
            u8 unmanaged = BottleItems_IsVanillaBottle((u8)cur) && !Bottle_WheelContains((uint8_t)w, cur);
            if (unmanaged && BottleItems_MigrateToWheel((u8)cur, (u8)w)) {
                Bottle_SetNetOwned(1);
                Bottle_SetBottomlessOwned(1);
                unmanaged = 0; // now wheel-owned (this wheel, or the other if this one was full)
            }
            // Show a bottle the wheel owns (covers post-migration and wheel-emptied cases). NEVER
            // overwrite an unmigrated residue (wheels full) — that would destroy the item.
            u16 first = Bottle_WheelFirstItem((uint8_t)w);
            if (!unmanaged && first != BOTTLE_SLOT_EMPTY && !Bottle_WheelContains((uint8_t)w, cur)) {
                BottleItems_SyncSlot(play, slot, (u8)first);
                cur = first;
            }
            Bottle_WheelRecordActive((uint8_t)w, cur);
        }
    }

    // Net (SLOT_BOTTLE_3) — owned -> show ITEM_NET (C-button refreshed too), else clear if it was ours.
    if (Bottle_NetOwned()) {
        BottleItems_SyncSlot(play, SLOT_BOTTLE_3, ITEM_NET);
    } else if (gSaveContext.save.saveInfo.inventory.items[SLOT_BOTTLE_3] == ITEM_NET) {
        gSaveContext.save.saveInfo.inventory.items[SLOT_BOTTLE_3] = ITEM_NONE;
    }

    // Bottomless Bottle (SLOT_BOTTLE_4) — owned -> shows its content (identified only by the counter
    // number); when empty it looks like a plain empty bottle (ITEM_BOTTLE) so the vanilla catch works
    // natively (no special empty art).
    if (Bottle_BottomlessOwned()) {
        // Adopt an external fill: if the slot got a real content not via our hook (shop potion, cow
        // milk, ...), record it + reset the counter. (A real content differs from ITEM_BOTTLE / empties.)
        u8 cur = gSaveContext.save.saveInfo.inventory.items[SLOT_BOTTLE_4];
        if (cur != ITEM_BOTTLE && cur != ITEM_BOTTOMLESS_BOTTLE && cur != ITEM_NONE &&
            cur != Bottle_BottomlessContent()) {
            Bottle_BottomlessFill(cur);
        }
        u8 want = Bottle_BottomlessIsEmpty() ? (u8)ITEM_BOTTLE : Bottle_BottomlessContent();
        BottleItems_SyncSlot(play, SLOT_BOTTLE_4, want);
    } else if (gSaveContext.save.saveInfo.inventory.items[SLOT_BOTTLE_4] == ITEM_BOTTOMLESS_BOTTLE) {
        gSaveContext.save.saveInfo.inventory.items[SLOT_BOTTLE_4] = ITEM_NONE;
    }
}

// Post-write counter driver: fires after every bottle update; we only act on SLOT_BOTTLE_4. The slot
// has already been written to `item`, so we detect "our" update by inventory.items[SLOT_BOTTLE_4] == item.
static void BottleItems_OnBottleUpdate(u8 item) {
    if (!Bottle_BottomlessOwned()) {
        return; // Bottomless Bottle not owned
    }
    if (gSaveContext.save.saveInfo.inventory.items[SLOT_BOTTLE_4] != item) {
        return; // a different bottle slot was updated, not SLOT_BOTTLE_4
    }
    if (item == ITEM_BOTTLE) {
        // Emptying: spend one use. If charges remain, restore the content in the slot + button
        // (auto-refill — undo the just-written empty). At 0, leave it empty; the enforcer shows the
        // empty Bottomless Bottle next frame.
        uint8_t remaining = Bottle_BottomlessConsume();
        if (remaining > 0) {
            BottleItems_SyncSlot(gPlayState, SLOT_BOTTLE_4, Bottle_BottomlessContent());
        }
    } else {
        // Filling (catch): record content + reset the counter to its max uses.
        Bottle_BottomlessFill(item);
    }
}

static void BottleItems_Register() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnBottleContentsUpdate>(
        BottleItems_OnBottleUpdate);

    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnInterfaceDrawStart>(
        []() { BottleItems_Enforce(gPlayState); });

    // Session wheel trackers must not leak across files — with the per-frame reconcile a stale
    // last-set from the previous file would ghost-write its slot value into the fresh wheel.
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSaveInit>(
        [](s16 fileNum) { Bottle_WheelResetTracking(); });
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSaveLoad>(
        [](s16 fileNum) { Bottle_WheelResetTracking(); });
}

static RegisterShipInitFunc gBottleItemsInit(BottleItems_Register);
