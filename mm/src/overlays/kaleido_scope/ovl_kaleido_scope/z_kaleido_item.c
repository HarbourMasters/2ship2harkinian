/*
 * File: z_kaleido_item.c
 * Overlay: ovl_kaleido_scope
 * Description: Pause Menu - Item Page
 */

#include "z_kaleido_scope.h"
#include "interface/parameter_static/parameter_static.h"

#include "2s2h/BenGui/HudEditor.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include "mods/extended_inventory.h" // NEI: page-aware kaleido (ExtInv_GetInventorySlot/GetSlotItem/GetItemIcon/SwitchPage/Update)
#include "mods/items/custom_bottles.h" // NEI: bottle randomizer wheels A/B (Skijer's NEI)
#include "archives/icon_item_static/icon_item_static_yar.h" // gABtnSymbolTex + gPausePromptCursorTex (cycle overlay)

void Interface_LoadItemIconImpl(PlayState* play, u8 btn);

// ============================================================================
// NEI item cycles — port of SoH's KaleidoScope_HandleItemCycleExtras /
// DrawItemCycleExtras (Skijer's NEI). A slot that "can cycle" shows an A-button
// hint; A opens the cycle, stick left/right swaps the slot item (left/right
// candidates), A again (or moving away) closes it. Used by the bottle wheels;
// the per-item selectors (lantern/gust/arrows/picto/powerkeg) hook in later.
// ============================================================================
static s16 gCurrentItemCyclingSlot = -1;
static s16 sSlotCycleActiveAnimTimer[ITEM_NUM_SLOTS] = { 0 };

// Left/right extra-item quads (32x32 texcoords), centered on the slot.
static Vtx sCycleExtraItemVtx[] = {
    VTX(-48, 16, 0, 0 << 5, 0 << 5, 0xFF, 0xFF, 0xFF, 0xFF),
    VTX(-16, 16, 0, 32 << 5, 0 << 5, 0xFF, 0xFF, 0xFF, 0xFF),
    VTX(-48, -16, 0, 0 << 5, 32 << 5, 0xFF, 0xFF, 0xFF, 0xFF),
    VTX(-16, -16, 0, 32 << 5, 32 << 5, 0xFF, 0xFF, 0xFF, 0xFF),
    VTX(16, 16, 0, 0 << 5, 0 << 5, 0xFF, 0xFF, 0xFF, 0xFF),
    VTX(48, 16, 0, 32 << 5, 0 << 5, 0xFF, 0xFF, 0xFF, 0xFF),
    VTX(16, -16, 0, 0 << 5, 32 << 5, 0xFF, 0xFF, 0xFF, 0xFF),
    VTX(48, -16, 0, 32 << 5, 32 << 5, 0xFF, 0xFF, 0xFF, 0xFF),
};
// Dark circles behind the extra items while cycling (48x48 I4 prompt texture).
static Vtx sCycleCircleVtx[] = {
    VTX(-56, 24, 0, 0 << 5, 0 << 5, 0xFF, 0xFF, 0xFF, 0xFF),
    VTX(-8, 24, 0, 48 << 5, 0 << 5, 0xFF, 0xFF, 0xFF, 0xFF),
    VTX(-56, -24, 0, 0 << 5, 48 << 5, 0xFF, 0xFF, 0xFF, 0xFF),
    VTX(-8, -24, 0, 48 << 5, 48 << 5, 0xFF, 0xFF, 0xFF, 0xFF),
    VTX(8, 24, 0, 0 << 5, 0 << 5, 0xFF, 0xFF, 0xFF, 0xFF),
    VTX(56, 24, 0, 48 << 5, 0 << 5, 0xFF, 0xFF, 0xFF, 0xFF),
    VTX(8, -24, 0, 0 << 5, 48 << 5, 0xFF, 0xFF, 0xFF, 0xFF),
    VTX(56, -24, 0, 48 << 5, 48 << 5, 0xFF, 0xFF, 0xFF, 0xFF),
};
// A-button hint (24x16 IA8, drawn 1.5x)
static Vtx sCycleAButtonVtx[] = {
    VTX(-18, 12, 0, 0 << 5, 0 << 5, 0xFF, 0xFF, 0xFF, 0xFF),
    VTX(18, 12, 0, 24 << 5, 0 << 5, 0xFF, 0xFF, 0xFF, 0xFF),
    VTX(-18, -12, 0, 0 << 5, 16 << 5, 0xFF, 0xFF, 0xFF, 0xFF),
    VTX(18, -12, 0, 24 << 5, 16 << 5, 0xFF, 0xFF, 0xFF, 0xFF),
};

s32 KaleidoScope_IsItemCycling(void) {
    return gCurrentItemCyclingSlot != -1;
}

void KaleidoScope_HandleItemCycleExtras(PlayState* play, u8 slot, u8 canCycle, u8 leftItem, u8 rightItem,
                                        u8 replaceCButtons) {
    Input* input = CONTROLLER1(&play->state);
    PauseContext* pauseCtx = &play->pauseCtx;
    s32 realSlot = ExtInv_GetInventorySlot(slot); // slot param is the VISUAL cell (page-aware)
    u8 slotItem = ExtInv_GetSlotItem(realSlot);
    u8 hasLeftItem = (leftItem != ITEM_NONE) && (slotItem != leftItem);
    u8 hasRightItem = (rightItem != ITEM_NONE) && (slotItem != rightItem) && (leftItem != rightItem);
    s32 i;

    // Overflow/wrap: a 2-item wheel swaps with EITHER stick direction (clawshot <-> hookshot)
    if (hasLeftItem && !hasRightItem) {
        rightItem = leftItem;
        hasRightItem = true;
    } else if (!hasLeftItem && hasRightItem) {
        leftItem = rightItem;
        hasLeftItem = true;
    }

    if (canCycle && (pauseCtx->cursorSlot[PAUSE_ITEM] == slot) && CHECK_BTN_ALL(input->press.button, BTN_A) &&
        (hasLeftItem || hasRightItem)) {
        Audio_PlaySfx(NA_SE_SY_DECIDE);
        gCurrentItemCyclingSlot = (gCurrentItemCyclingSlot == slot) ? -1 : slot;
    }

    if (gCurrentItemCyclingSlot == slot) {
        pauseCtx->cursorColorSet = 8;
        if (pauseCtx->stickAdjX > 30) {
            Audio_PlaySfx(NA_SE_SY_CURSOR);
            if (replaceCButtons) {
                for (i = EQUIP_SLOT_C_LEFT; i <= EQUIP_SLOT_C_RIGHT; i++) {
                    if (BUTTON_ITEM_EQUIP(0, i) == ExtInv_GetSlotItem(realSlot)) {
                        BUTTON_ITEM_EQUIP(0, i) = rightItem;
                        // keep the button SLOT coherent with the item's home slot — MM flows
                        // (pictograph capture!) key on GET_CUR_FORM_BTN_SLOT and hang otherwise
                        if (rightItem < 52) {
                            C_SLOT_EQUIP(0, i) = gItemSlots[rightItem];
                        }
                        Interface_LoadItemIconImpl(play, i);
                        break;
                    }
                }
            }
            ExtInv_SetSlotItem(realSlot, rightItem);
        } else if (pauseCtx->stickAdjX < -30) {
            Audio_PlaySfx(NA_SE_SY_CURSOR);
            if (replaceCButtons) {
                for (i = EQUIP_SLOT_C_LEFT; i <= EQUIP_SLOT_C_RIGHT; i++) {
                    if (BUTTON_ITEM_EQUIP(0, i) == ExtInv_GetSlotItem(realSlot)) {
                        BUTTON_ITEM_EQUIP(0, i) = leftItem;
                        if (leftItem < 52) {
                            C_SLOT_EQUIP(0, i) = gItemSlots[leftItem];
                        }
                        Interface_LoadItemIconImpl(play, i);
                        break;
                    }
                }
            }
            ExtInv_SetSlotItem(realSlot, leftItem);
        }
        // Close if the cursor left the slot
        gCurrentItemCyclingSlot = (pauseCtx->cursorSlot[PAUSE_ITEM] == slot) ? slot : -1;
    }
}

// `forceShow` (used by the bottle wheels) shows the previews + A indicator even when prev/next share
// the slot's value — needed because multiple EMPTY bottles all read as ITEM_BOTTLE but are distinct
// slots (index-based cycling). Skijer's NEI
static void KaleidoScope_DrawItemCycleExtrasImpl(PlayState* play, u8 slot, u8 canCycle, u8 leftItem, u8 rightItem,
                                                 u8 forceShow) {
    PauseContext* pauseCtx = &play->pauseCtx;
    u8 isCycling = (gCurrentItemCyclingSlot == slot);
    u8 slotItem = ExtInv_GetSlotItem(ExtInv_GetInventorySlot(slot));
    u8 showLeftItem = (leftItem != ITEM_NONE) && (forceShow || (slotItem != leftItem));
    u8 showRightItem = (rightItem != ITEM_NONE) && (forceShow || ((slotItem != rightItem) && (leftItem != rightItem)));
    Vtx* cellVtx = &pauseCtx->itemVtx[slot * 4];
    s16 cx;
    s16 cy;

    OPEN_DISPS(play->state.gfxCtx);

    if (isCycling) {
        if (sSlotCycleActiveAnimTimer[slot] < 5) {
            sSlotCycleActiveAnimTimer[slot]++;
        }
    } else if (sSlotCycleActiveAnimTimer[slot] > 0) {
        sSlotCycleActiveAnimTimer[slot]--;
    }

    if (canCycle && (slotItem != ITEM_NONE) && (showLeftItem || showRightItem)) {
        // Exact OoT/SoH transform baked per-vertex (a matrix fights MM's live page transform):
        // candidates REST small (scale 0.325) tucked 15px below the cell, and grow to full 32px
        // flanking the item as the wheel opens (anim timer 0..5).
        f32 animScale = (5 - sSlotCycleActiveAnimTimer[slot]) / 5.0f; // 1 rest .. 0 open
        f32 finalScale = 1.0f - (0.675f * animScale);                // 0.325 rest .. 1.0 open
        s16 qSize = (s16)(32.0f * finalScale);                       // candidate quad px
        s16 flank = (s16)(32.0f * finalScale);                       // candidate center offset from cell
        s16 yShift = (s16)(-15.0f * animScale);                      // 15 below at rest .. 0 open
        s16 yTop;
        s32 vi;

        // True cell center from opposite-corner vtx (layout-agnostic).
        cx = (cellVtx[0].v.ob[0] + cellVtx[3].v.ob[0]) / 2;
        cy = (cellVtx[0].v.ob[1] + cellVtx[3].v.ob[1]) / 2;
        yTop = cy + yShift + qSize / 2;

        gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255,
                        isCycling ? pauseCtx->alpha : (u8)(pauseCtx->alpha * 3 / 4));

        if (showLeftItem) {
            u8 lsz = ExtInv_GetItemIconSize(leftItem);
            Vtx* v = GRAPH_ALLOC(play->state.gfxCtx, 4 * sizeof(Vtx));
            s16 x0 = (cx - flank) - qSize / 2;
            for (vi = 0; vi < 4; vi++) {
                v[vi] = cellVtx[0];
            }
            v[0].v.ob[0] = x0;         v[0].v.ob[1] = yTop;         v[0].v.tc[0] = 0;        v[0].v.tc[1] = 0;
            v[1].v.ob[0] = x0 + qSize; v[1].v.ob[1] = yTop;         v[1].v.tc[0] = lsz << 5; v[1].v.tc[1] = 0;
            v[2].v.ob[0] = x0;         v[2].v.ob[1] = yTop - qSize; v[2].v.tc[0] = 0;        v[2].v.tc[1] = lsz << 5;
            v[3].v.ob[0] = x0 + qSize; v[3].v.ob[1] = yTop - qSize; v[3].v.tc[0] = lsz << 5; v[3].v.tc[1] = lsz << 5;
            gSPVertex(POLY_OPA_DISP++, v, 4, 0);
            KaleidoScope_DrawTexQuadRGBA32(play->state.gfxCtx, ExtInv_GetItemIcon(leftItem), lsz, lsz, 0);
        }
        if (showRightItem) {
            u8 rsz = ExtInv_GetItemIconSize(rightItem);
            Vtx* v = GRAPH_ALLOC(play->state.gfxCtx, 4 * sizeof(Vtx));
            s16 x0 = (cx + flank) - qSize / 2;
            for (vi = 0; vi < 4; vi++) {
                v[vi] = cellVtx[0];
            }
            v[0].v.ob[0] = x0;         v[0].v.ob[1] = yTop;         v[0].v.tc[0] = 0;        v[0].v.tc[1] = 0;
            v[1].v.ob[0] = x0 + qSize; v[1].v.ob[1] = yTop;         v[1].v.tc[0] = rsz << 5; v[1].v.tc[1] = 0;
            v[2].v.ob[0] = x0;         v[2].v.ob[1] = yTop - qSize; v[2].v.tc[0] = 0;        v[2].v.tc[1] = rsz << 5;
            v[3].v.ob[0] = x0 + qSize; v[3].v.ob[1] = yTop - qSize; v[3].v.tc[0] = rsz << 5; v[3].v.tc[1] = rsz << 5;
            gSPVertex(POLY_OPA_DISP++, v, 4, 0);
            KaleidoScope_DrawTexQuadRGBA32(play->state.gfxCtx, ExtInv_GetItemIcon(rightItem), rsz, rsz, 0);
        }

        // Small A-button hint centered between the resting minis (only when hovered & closed).
        if (!isCycling && (sSlotCycleActiveAnimTimer[slot] == 0) && (pauseCtx->cursorSlot[PAUSE_ITEM] == slot) &&
            (pauseCtx->cursorSpecialPos == 0)) {
            Vtx* av = GRAPH_ALLOC(play->state.gfxCtx, 4 * sizeof(Vtx));
            s16 ax = cx;
            s16 ay = cy + yShift;
            for (vi = 0; vi < 4; vi++) {
                av[vi] = cellVtx[0];
            }
            av[0].v.ob[0] = ax - 7; av[0].v.ob[1] = ay + 5; av[0].v.tc[0] = 0;       av[0].v.tc[1] = 0;
            av[1].v.ob[0] = ax + 7; av[1].v.ob[1] = ay + 5; av[1].v.tc[0] = 24 << 5; av[1].v.tc[1] = 0;
            av[2].v.ob[0] = ax - 7; av[2].v.ob[1] = ay - 5; av[2].v.tc[0] = 0;       av[2].v.tc[1] = 16 << 5;
            av[3].v.ob[0] = ax + 7; av[3].v.ob[1] = ay - 5; av[3].v.tc[0] = 24 << 5; av[3].v.tc[1] = 16 << 5;
            gSPVertex(POLY_OPA_DISP++, av, 4, 0);
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 0, 100, 255, pauseCtx->alpha);
            gDPLoadTextureBlock(POLY_OPA_DISP++, gABtnSymbolTex, G_IM_FMT_IA, G_IM_SIZ_8b, 24, 16, 0,
                                G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP, 4, 4, G_TX_NOLOD, G_TX_NOLOD);
            gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);
        }
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

// Public entry (value-based show gates) — trade/keg/bow/etc. cyclers use this.
void KaleidoScope_DrawItemCycleExtras(PlayState* play, u8 slot, u8 canCycle, u8 leftItem, u8 rightItem) {
    KaleidoScope_DrawItemCycleExtrasImpl(play, slot, canCycle, leftItem, rightItem, false);
}

// --- OoT page-0 wheel helpers (Skijer's NEI) ---
#include "mods/nei_save.h"
extern u8 gItemSlots[]; // vanilla item -> home slot (52 entries)
extern u8 TradeAdult_OwnedCount(void);
extern u8 TradeAdult_PrevItem(u8 cur);
extern u8 TradeAdult_NextItem(u8 cur);

// Bow cell wheel list: vanilla bow + the six SW97 elemental arrows
static const u8 sBowWheelItems[] = { ITEM_BOW,           ITEM_SW97_ARROW_FIRE, ITEM_SW97_ARROW_ICE,
                                     ITEM_SW97_ARROW_LIGHT, ITEM_SW97_ARROW_DARK, ITEM_SW97_ARROW_SOUL,
                                     ITEM_SW97_ARROW_WIND };
// Slingshot cell wheel list: Fairy Slingshot + the six SW97 elemental bullets (twin of the bow
// wheel — user decision: elemental arrows and bullets coexist, one per cell). Skijer's NEI.
static const u8 sSlingWheelItems[] = { ITEM_FAIRY_SLINGSHOT,   ITEM_SW97_BULLET_FIRE, ITEM_SW97_BULLET_ICE,
                                       ITEM_SW97_BULLET_LIGHT, ITEM_SW97_BULLET_DARK, ITEM_SW97_BULLET_SOUL,
                                       ITEM_SW97_BULLET_WIND };
static u8 KaleidoSlingWheel_Neighbor(u8 cur, s32 dir) {
    s32 n = ARRAY_COUNT(sSlingWheelItems);
    s32 i;
    for (i = 0; i < n; i++) {
        if (sSlingWheelItems[i] == cur) {
            return sSlingWheelItems[(i + dir + n) % n];
        }
    }
    return cur;
}
static u8 KaleidoBowWheel_Neighbor(u8 cur, s32 dir) {
    s32 n = ARRAY_COUNT(sBowWheelItems);
    s32 i;
    for (i = 0; i < n; i++) {
        if (sBowWheelItems[i] == cur) {
            return sBowWheelItems[(i + dir + n) % n];
        }
    }
    return cur;
}

// Orchestrators — bottle wheels A/B + the OoT page-0 cell wheels (bombs/keg, bow/SW97,
// hookshot modes, lens/picto, nayru/rocs, adult trade). Lantern/gust selectors: per-item pass.
// Bottle wheel selector — INDEX-based (Skijer's NEI, mirror of the SoH version). Unlike
// KaleidoScope_HandleItemCycleExtras (which swaps by VALUE and so can't move between two identical
// empty bottles), this steps the ACTIVE SLOT so every physical bottle — empty ones included — is
// reachable, and the wheel stays cyclable even when every bottle is empty. A opens/closes; stick L/R
// steps the slot and updates the visible slot + any C-button showing it.
static void Bottle_WheelHandle(PlayState* play, u8 wheel, u8 kaleidoSlot) {
    Input* input = CONTROLLER1(&play->state);
    PauseContext* pauseCtx = &play->pauseCtx;
    s32 i;

    // Keep the visible slot showing a bottle the wheel actually owns (covers give/drink/catch that
    // changed it out of kaleido, and the initial projection).
    u16 first = Bottle_WheelFirstItem(wheel);
    if ((first != ITEM_NONE) && !Bottle_WheelContains(wheel, ExtInv_GetSlotItem(kaleidoSlot))) {
        ExtInv_SetSlotItem(kaleidoSlot, (u8)first);
    }

    if (Bottle_WheelBottleCount(wheel) < 2) {
        return; // 0 or 1 bottle: nothing to cycle
    }

    if ((pauseCtx->cursorSlot[PAUSE_ITEM] == kaleidoSlot) && CHECK_BTN_ALL(input->press.button, BTN_A)) {
        Audio_PlaySfx(NA_SE_SY_DECIDE);
        gCurrentItemCyclingSlot = (gCurrentItemCyclingSlot == kaleidoSlot) ? -1 : kaleidoSlot;
    }
    if (gCurrentItemCyclingSlot == kaleidoSlot) {
        pauseCtx->cursorColorSet = 8;
        s8 dir = 0;
        if (pauseCtx->stickAdjX > 30) {
            dir = 1;
        } else if (pauseCtx->stickAdjX < -30) {
            dir = -1;
        }
        if (dir != 0) {
            Audio_PlaySfx(NA_SE_SY_CURSOR);
            u8 oldItem = ExtInv_GetSlotItem(kaleidoSlot);
            u8 newItem = (u8)Bottle_WheelStep(wheel, dir);
            // Re-point the C-button equipped to THIS wheel's bottle. C_SLOT stays the wheel's vanilla
            // slot (NOT the content's gItemSlots home) so drink/catch write back to the wheel slot.
            for (i = EQUIP_SLOT_C_LEFT; i <= EQUIP_SLOT_C_RIGHT; i++) {
                if ((BUTTON_ITEM_EQUIP(0, i) == oldItem) && (C_SLOT_EQUIP(0, i) == kaleidoSlot)) {
                    BUTTON_ITEM_EQUIP(0, i) = newItem;
                    C_SLOT_EQUIP(0, i) = kaleidoSlot;
                    Interface_LoadItemIconImpl(play, i);
                    break;
                }
            }
            ExtInv_SetSlotItem(kaleidoSlot, newItem);
        }
        gCurrentItemCyclingSlot = (pauseCtx->cursorSlot[PAUSE_ITEM] == kaleidoSlot) ? kaleidoSlot : -1;
    }
}

void KaleidoScope_HandleItemCycles(PlayState* play) {
    if (ExtInv_GetCurrentPage() != 0) {
        return;
    }

    // [2] Bombs <-> Power Keg (keg = MM native item; both live on the bomb cell)
    {
        u8 kegOwned = (ExtInv_GetSlotItem(SLOT_POWDER_KEG) != ITEM_NONE) || Nei_Save()->powerKegOwned;
        KaleidoScope_HandleItemCycleExtras(play, 2, kegOwned, ITEM_BOMB, ITEM_POWDER_KEG, true);
    }
    // [3] Bow <-> SW97 elemental arrows (functional: SW97 IAs alias to bow/slingshot)
    if (CVarGetInteger("gEnhancements.SkijerNEI.SW97Medallions", 0)) {
        u8 cur = ExtInv_GetSlotItem(ExtInv_GetInventorySlot(3));
        KaleidoScope_HandleItemCycleExtras(play, 3, cur != ITEM_NONE, KaleidoBowWheel_Neighbor(cur, -1),
                                           KaleidoBowWheel_Neighbor(cur, 1), true);
    }
    // [6] Fairy Slingshot <-> SW97 elemental bullets (twin wheel; Skijer's NEI slingshot pass)
    if (CVarGetInteger("gEnhancements.SkijerNEI.SW97Medallions", 0)) {
        u8 cur = ExtInv_GetSlotItem(ExtInv_GetInventorySlot(6));
        KaleidoScope_HandleItemCycleExtras(play, 6, cur != ITEM_NONE, KaleidoSlingWheel_Neighbor(cur, -1),
                                           KaleidoSlingWheel_Neighbor(cur, 1), true);
    }
    // [9] Clawshot (MM hookshot, functional) <-> OoT Hookshot/Longshot
    {
        u8 lvl = Nei_Save()->ootHookshotLevel;
        u8 ootHook = (lvl >= 2) ? ITEM_LONGSHOT_OOT : ITEM_HOOKSHOT_OOT;
        KaleidoScope_HandleItemCycleExtras(play, 9, lvl > 0, ITEM_HOOKSHOT, ootHook, true);
    }
    // [13] Lens of Truth <-> Pictograph Box (both MM native)
    {
        u8 pictoOwned = (gSaveContext.save.saveInfo.inventory.items[SLOT_PICTOGRAPH_BOX] != ITEM_NONE);
        KaleidoScope_HandleItemCycleExtras(play, 13, pictoOwned, ITEM_LENS_OF_TRUTH, ITEM_PICTOGRAPH_BOX, true);
    }
    // [17] Nayru's Love <-> Roc's Feather (rocs = functional page-2 custom item)
    {
        u8 rocs = (Nei_GetOwnedItem(SLOT_ROCS) != ITEM_NONE) ? ITEM_ROCS_FEATHER : ITEM_NONE; // ship-vanilla feather
        KaleidoScope_HandleItemCycleExtras(play, 17, rocs != ITEM_NONE, ITEM_NAYRUS_LOVE, rocs, true);
    }
    // [22] Adult trade wheel (OoT + MM trade items, trade_items.c)
    {
        u8 tradeCur = ExtInv_GetSlotItem(ExtInv_GetInventorySlot(22));
        KaleidoScope_HandleItemCycleExtras(play, 22, TradeAdult_OwnedCount() > 1, TradeAdult_PrevItem(tradeCur),
                                           TradeAdult_NextItem(tradeCur), true);
    }
    // Bottle wheels A/B — index-based selectors so every bottle (empty ones included) is reachable
    // and the wheel is always cyclable with >=2 bottles. Persist/RecordActive around the step keep
    // in-game drink/catch changes reconciled with the wheel state (as in the SoH version).
    {
        Bottle_WheelPersist(BOTTLE_WHEEL_A, ExtInv_GetSlotItem(SLOT_BOTTLE_1));
        Bottle_WheelHandle(play, BOTTLE_WHEEL_A, SLOT_BOTTLE_1);
        Bottle_WheelRecordActive(BOTTLE_WHEEL_A, ExtInv_GetSlotItem(SLOT_BOTTLE_1));

        Bottle_WheelPersist(BOTTLE_WHEEL_B, ExtInv_GetSlotItem(SLOT_BOTTLE_2));
        Bottle_WheelHandle(play, BOTTLE_WHEEL_B, SLOT_BOTTLE_2);
        Bottle_WheelRecordActive(BOTTLE_WHEEL_B, ExtInv_GetSlotItem(SLOT_BOTTLE_2));
    }
}

void KaleidoScope_DrawItemCycles(PlayState* play) {
    if (ExtInv_GetCurrentPage() != 0) {
        return;
    }

    // OoT page-0 cell wheels (mirror of KaleidoScope_HandleItemCycles)
    {
        u8 kegOwned = (ExtInv_GetSlotItem(SLOT_POWDER_KEG) != ITEM_NONE) || Nei_Save()->powerKegOwned;
        KaleidoScope_DrawItemCycleExtras(play, 2, kegOwned, ITEM_BOMB, ITEM_POWDER_KEG);
    }
    if (CVarGetInteger("gEnhancements.SkijerNEI.SW97Medallions", 0)) {
        u8 cur = ExtInv_GetSlotItem(ExtInv_GetInventorySlot(3));
        KaleidoScope_DrawItemCycleExtras(play, 3, cur != ITEM_NONE, KaleidoBowWheel_Neighbor(cur, -1),
                                         KaleidoBowWheel_Neighbor(cur, 1));
    }
    if (CVarGetInteger("gEnhancements.SkijerNEI.SW97Medallions", 0)) { // slingshot twin wheel (Skijer's NEI)
        u8 cur = ExtInv_GetSlotItem(ExtInv_GetInventorySlot(6));
        KaleidoScope_DrawItemCycleExtras(play, 6, cur != ITEM_NONE, KaleidoSlingWheel_Neighbor(cur, -1),
                                         KaleidoSlingWheel_Neighbor(cur, 1));
    }
    {
        u8 lvl = Nei_Save()->ootHookshotLevel;
        u8 ootHook = (lvl >= 2) ? ITEM_LONGSHOT_OOT : ITEM_HOOKSHOT_OOT;
        KaleidoScope_DrawItemCycleExtras(play, 9, lvl > 0, ITEM_HOOKSHOT, ootHook);

        // Skijer's NEI — Ultrashot: at level 3 the cell keeps the Longshot ICON; a small Light
        // medallion on the cell's TOP-RIGHT corner (+ the "Ultrashot" name tex) is what tells it
        // apart. Same per-vertex quad scheme as the wheel minis above.
        if ((lvl >= 3) && (ExtInv_GetSlotItem(ExtInv_GetInventorySlot(9)) == ITEM_LONGSHOT_OOT)) {
            PauseContext* pauseCtx = &play->pauseCtx;
            Vtx* cellVtx = &pauseCtx->itemVtx[9 * 4];
            Vtx* mv = GRAPH_ALLOC(play->state.gfxCtx, 4 * sizeof(Vtx));
            s16 cx = (cellVtx[0].v.ob[0] + cellVtx[3].v.ob[0]) / 2;
            s16 cy = (cellVtx[0].v.ob[1] + cellVtx[3].v.ob[1]) / 2;
            s16 mSize = 14;
            s16 mx0 = cx + 18 - mSize; // marker's right edge 2px past the 32px cell's right edge
            s16 myTop = cy + 18;       // marker's top edge 2px past the cell's top edge
            s32 mvi;

            OPEN_DISPS(play->state.gfxCtx);

            gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);

            for (mvi = 0; mvi < 4; mvi++) {
                mv[mvi] = cellVtx[0];
            }
            mv[0].v.ob[0] = mx0;         mv[0].v.ob[1] = myTop;         mv[0].v.tc[0] = 0;       mv[0].v.tc[1] = 0;
            mv[1].v.ob[0] = mx0 + mSize; mv[1].v.ob[1] = myTop;         mv[1].v.tc[0] = 24 << 5; mv[1].v.tc[1] = 0;
            mv[2].v.ob[0] = mx0;         mv[2].v.ob[1] = myTop - mSize; mv[2].v.tc[0] = 0;       mv[2].v.tc[1] = 24 << 5;
            mv[3].v.ob[0] = mx0 + mSize; mv[3].v.ob[1] = myTop - mSize; mv[3].v.tc[0] = 24 << 5; mv[3].v.tc[1] = 24 << 5;
            gSPVertex(POLY_OPA_DISP++, mv, 4, 0);
            KaleidoScope_DrawTexQuadRGBA32(
                play->state.gfxCtx, (void*)"__OTR__textures/icon_item_24_static/gQuestIconMedallionLightTex", 24, 24,
                0);

            CLOSE_DISPS(play->state.gfxCtx);
        }
    }
    {
        u8 pictoOwned = (gSaveContext.save.saveInfo.inventory.items[SLOT_PICTOGRAPH_BOX] != ITEM_NONE);
        KaleidoScope_DrawItemCycleExtras(play, 13, pictoOwned, ITEM_LENS_OF_TRUTH, ITEM_PICTOGRAPH_BOX);
    }
    {
        u8 rocs = (Nei_GetOwnedItem(SLOT_ROCS) != ITEM_NONE) ? ITEM_ROCS_FEATHER : ITEM_NONE; // ship-vanilla feather
        KaleidoScope_DrawItemCycleExtras(play, 17, rocs != ITEM_NONE, ITEM_NAYRUS_LOVE, rocs);
    }
    {
        u8 tradeCur = ExtInv_GetSlotItem(ExtInv_GetInventorySlot(22));
        KaleidoScope_DrawItemCycleExtras(play, 22, TradeAdult_OwnedCount() > 1, TradeAdult_PrevItem(tradeCur),
                                         TradeAdult_NextItem(tradeCur));
    }

    // Bottle wheels A/B — previews are the prev/next SLOT (index-based) with forceShow, so the
    // mini-icons + A indicator appear even between identical EMPTY bottles. Skijer's NEI
    {
        KaleidoScope_DrawItemCycleExtrasImpl(play, SLOT_BOTTLE_1, Bottle_WheelBottleCount(BOTTLE_WHEEL_A) > 1,
                                             Bottle_WheelPeek(BOTTLE_WHEEL_A, -1), Bottle_WheelPeek(BOTTLE_WHEEL_A, 1),
                                             true);
        KaleidoScope_DrawItemCycleExtrasImpl(play, SLOT_BOTTLE_2, Bottle_WheelBottleCount(BOTTLE_WHEEL_B) > 1,
                                             Bottle_WheelPeek(BOTTLE_WHEEL_B, -1), Bottle_WheelPeek(BOTTLE_WHEEL_B, 1),
                                             true);
    }
}

s16 sEquipState = EQUIP_STATE_MAGIC_ARROW_GROW_ORB;

// Timer to hold magic arrow icon over magic arrow slot before moving when equipping.
s16 sEquipMagicArrowSlotHoldTimer = 0;

// Number of frames to move icon from slot to target position when equipping.
s16 sEquipAnimTimer = 10;

u8 gPlayerFormSlotRestrictions[PLAYER_FORM_MAX][ITEM_NUM_SLOTS] = {
    // PLAYER_FORM_FIERCE_DEITY
    {
        false, // SLOT_OCARINA
        false, // SLOT_BOW
        false, // SLOT_ARROW_FIRE
        false, // SLOT_ARROW_ICE
        false, // SLOT_ARROW_LIGHT
        false, // SLOT_TRADE_DEED
        false, // SLOT_BOMB
        false, // SLOT_BOMBCHU
        false, // SLOT_DEKU_STICK
        false, // SLOT_DEKU_NUT
        false, // SLOT_MAGIC_BEANS
        false, // SLOT_TRADE_KEY_MAMA
        false, // SLOT_POWDER_KEG
        false, // SLOT_PICTOGRAPH_BOX
        false, // SLOT_LENS_OF_TRUTH
        false, // SLOT_HOOKSHOT
        false, // SLOT_SWORD_GREAT_FAIRY
        false, // SLOT_TRADE_COUPLE
        true,  // SLOT_BOTTLE_1
        true,  // SLOT_BOTTLE_2
        true,  // SLOT_BOTTLE_3
        true,  // SLOT_BOTTLE_4
        true,  // SLOT_BOTTLE_5
        true,  // SLOT_BOTTLE_6
    },
    // PLAYER_FORM_GORON
    {
        true,  // SLOT_OCARINA
        false, // SLOT_BOW
        false, // SLOT_ARROW_FIRE
        false, // SLOT_ARROW_ICE
        false, // SLOT_ARROW_LIGHT
        true,  // SLOT_TRADE_DEED
        false, // SLOT_BOMB
        false, // SLOT_BOMBCHU
        false, // SLOT_DEKU_STICK
        false, // SLOT_DEKU_NUT
        false, // SLOT_MAGIC_BEANS
        true,  // SLOT_TRADE_KEY_MAMA
        true,  // SLOT_POWDER_KEG
        true,  // SLOT_PICTOGRAPH_BOX
        true,  // SLOT_LENS_OF_TRUTH
        false, // SLOT_HOOKSHOT
        false, // SLOT_SWORD_GREAT_FAIRY
        true,  // SLOT_TRADE_COUPLE
        true,  // SLOT_BOTTLE_1
        true,  // SLOT_BOTTLE_2
        true,  // SLOT_BOTTLE_3
        true,  // SLOT_BOTTLE_4
        true,  // SLOT_BOTTLE_5
        true,  // SLOT_BOTTLE_6
    },
    // PLAYER_FORM_ZORA
    {
        true,  // SLOT_OCARINA
        false, // SLOT_BOW
        false, // SLOT_ARROW_FIRE
        false, // SLOT_ARROW_ICE
        false, // SLOT_ARROW_LIGHT
        true,  // SLOT_TRADE_DEED
        false, // SLOT_BOMB
        false, // SLOT_BOMBCHU
        false, // SLOT_DEKU_STICK
        false, // SLOT_DEKU_NUT
        false, // SLOT_MAGIC_BEANS
        true,  // SLOT_TRADE_KEY_MAMA
        false, // SLOT_POWDER_KEG
        true,  // SLOT_PICTOGRAPH_BOX
        true,  // SLOT_LENS_OF_TRUTH
        false, // SLOT_HOOKSHOT
        false, // SLOT_SWORD_GREAT_FAIRY
        true,  // SLOT_TRADE_COUPLE
        true,  // SLOT_BOTTLE_1
        true,  // SLOT_BOTTLE_2
        true,  // SLOT_BOTTLE_3
        true,  // SLOT_BOTTLE_4
        true,  // SLOT_BOTTLE_5
        true,  // SLOT_BOTTLE_6
    },
    // PLAYER_FORM_DEKU
    {
        true,  // SLOT_OCARINA
        false, // SLOT_BOW
        false, // SLOT_ARROW_FIRE
        false, // SLOT_ARROW_ICE
        false, // SLOT_ARROW_LIGHT
        true,  // SLOT_TRADE_DEED
        false, // SLOT_BOMB
        false, // SLOT_BOMBCHU
        false, // SLOT_DEKU_STICK
        true,  // SLOT_DEKU_NUT
        false, // SLOT_MAGIC_BEANS
        true,  // SLOT_TRADE_KEY_MAMA
        false, // SLOT_POWDER_KEG
        true,  // SLOT_PICTOGRAPH_BOX
        true,  // SLOT_LENS_OF_TRUTH
        false, // SLOT_HOOKSHOT
        false, // SLOT_SWORD_GREAT_FAIRY
        true,  // SLOT_TRADE_COUPLE
        true,  // SLOT_BOTTLE_1
        true,  // SLOT_BOTTLE_2
        true,  // SLOT_BOTTLE_3
        true,  // SLOT_BOTTLE_4
        true,  // SLOT_BOTTLE_5
        true,  // SLOT_BOTTLE_6
    },
    // PLAYER_FORM_HUMAN
    {
        true,  // SLOT_OCARINA
        true,  // SLOT_BOW
        true,  // SLOT_ARROW_FIRE
        true,  // SLOT_ARROW_ICE
        true,  // SLOT_ARROW_LIGHT
        true,  // SLOT_TRADE_DEED
        true,  // SLOT_BOMB
        true,  // SLOT_BOMBCHU
        true,  // SLOT_DEKU_STICK
        true,  // SLOT_DEKU_NUT
        true,  // SLOT_MAGIC_BEANS
        true,  // SLOT_TRADE_KEY_MAMA
        false, // SLOT_POWDER_KEG
        true,  // SLOT_PICTOGRAPH_BOX
        true,  // SLOT_LENS_OF_TRUTH
        true,  // SLOT_HOOKSHOT
        true,  // SLOT_SWORD_GREAT_FAIRY
        true,  // SLOT_TRADE_COUPLE
        true,  // SLOT_BOTTLE_1
        true,  // SLOT_BOTTLE_2
        true,  // SLOT_BOTTLE_3
        true,  // SLOT_BOTTLE_4
        true,  // SLOT_BOTTLE_5
        true,  // SLOT_BOTTLE_6
    },
};

s16 sAmmoRectLeft[] = {
    95,  // SLOT_BOW
    62,  // SLOT_BOMB
    95,  // SLOT_BOMBCHU
    128, // SLOT_DEKU_STICK
    161, // SLOT_DEKU_NUT
    194, // SLOT_MAGIC_BEANS
    62,  // SLOT_POWDER_KEG
    95,  // SLOT_PICTOGRAPH_BOX
};

s16 sAmmoRectHeight[] = {
    85,  // SLOT_BOW
    117, // SLOT_BOMB
    117, // SLOT_BOMBCHU
    117, // SLOT_DEKU_STICK
    117, // SLOT_DEKU_NUT
    117, // SLOT_MAGIC_BEANS
    150, // SLOT_POWDER_KEG
    150, // SLOT_PICTOGRAPH_BOX
};

extern const char* gAmmoDigitTextures[10];

void KaleidoScope_DrawAmmoCount(PauseContext* pauseCtx, GraphicsContext* gfxCtx, s16 item, u16 ammoIndex) {
    // NEI: ammoIndex is now the VISUAL CELL (0..23); digits position derives from the cell.
    s16 nRectLeft = 62 + (ammoIndex % 6) * 33;
    static const s16 sNeiAmmoRowTop[4] = { 85, 117, 150, 183 };
    s16 nRectTop = sNeiAmmoRowTop[(ammoIndex / 6) & 3];
    s16 ammoUpperDigit;
    s16 ammo;

    if (!GameInteractor_Should(VB_KALEIDO_DRAW_AMMO_COUNT, true, pauseCtx, gfxCtx, item, ammoIndex)) {
        return;
    }

    if ((ammoIndex == SLOT_BOTTLE_4) && Bottle_BottomlessOwned()) {
        // Skijer's NEI Bottle Randomizer: the Bottomless Bottle cell ALWAYS shows its use-counter
        // while owned — 0 (grey) when empty, else the remaining uses. The number IS its identity.
        // Self-contained OPEN/CLOSE pair BEFORE the main one: OPEN_DISPS/CLOSE_DISPS are braced
        // macros, so an early CLOSE+return inside the outer scope's if would unbalance the braces.
        ammo = Bottle_BottomlessCount();

        OPEN_DISPS(gfxCtx);

        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);
        if (ammo == 0) {
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 130, 130, 130, pauseCtx->alpha);
        }
        for (ammoUpperDigit = 0; ammo >= 10; ammoUpperDigit++) {
            ammo -= 10;
        }
        gDPPipeSync(POLY_OPA_DISP++);
        if (ammoUpperDigit != 0) {
            POLY_OPA_DISP = Gfx_DrawTexRectIA8(POLY_OPA_DISP, gAmmoDigitTextures[ammoUpperDigit], 8, 8, nRectLeft,
                                               nRectTop, 8, 8, 1 << 10, 1 << 10);
        }
        POLY_OPA_DISP = Gfx_DrawTexRectIA8(POLY_OPA_DISP, gAmmoDigitTextures[ammo], 8, 8, nRectLeft + 6, nRectTop, 8,
                                           8, 1 << 10, 1 << 10);

        CLOSE_DISPS(gfxCtx);
        return;
    }

    OPEN_DISPS(gfxCtx);

    if (item == ITEM_PICTOGRAPH_BOX) {
        if (!CHECK_QUEST_ITEM(QUEST_PICTOGRAPH)) {
            ammo = 0;
        } else {
            ammo = 1;
        }
    } else if ((item == ITEM_FAIRY_SLINGSHOT) ||
               ((item >= ITEM_SW97_BULLET_FIRE) && (item <= ITEM_SW97_BULLET_WIND))) {
        // Skijer's NEI: seed pouch — 0xA3/0xA7.. sit outside the vanilla AMMO()/SLOT() tables
        item = ITEM_FAIRY_SLINGSHOT;
        ammo = Nei_SlingshotSeeds();
    } else {
        ammo = AMMO(item);
    }

    gDPPipeSync(POLY_OPA_DISP++);

    // Skijer's NEI: the slingshot family bypasses the SLOT() lookup (OOB for 0xA3) — it shares
    // MM's native ITEM_SLINGSHOT form restriction (human only) for the grey tint.
    if ((item == ITEM_FAIRY_SLINGSHOT) ? !gPlayerFormItemRestrictions[GET_PLAYER_FORM][ITEM_SLINGSHOT]
                                       : !gPlayerFormSlotRestrictions[GET_PLAYER_FORM][SLOT(item)]) {
        // Ammo item is restricted
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 100, 100, 100, pauseCtx->alpha);
    } else {
        // Default ammo
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);
        if (ammo == 0) {
            // Out of ammo
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 130, 130, 130, pauseCtx->alpha);
        } else if (((item == ITEM_BOMB) && (AMMO(item) == CUR_CAPACITY(UPG_BOMB_BAG))) ||
                   ((item == ITEM_BOW) && (AMMO(item) == CUR_CAPACITY(UPG_QUIVER))) ||
                   ((item == ITEM_DEKU_STICK) && (AMMO(item) == CUR_CAPACITY(UPG_DEKU_STICKS))) ||
                   ((item == ITEM_DEKU_NUT) && (AMMO(item) == CUR_CAPACITY(UPG_DEKU_NUTS))) ||
                   ((item == ITEM_BOMBCHU) && (AMMO(item) == CUR_CAPACITY(UPG_BOMB_BAG))) ||
                   ((item == ITEM_POWDER_KEG) && GameInteractor_Should(VB_POWDER_KEG_AMMO_AT_CAPACITY, ammo == 1)) ||
                   ((item == ITEM_PICTOGRAPH_BOX) && (ammo == 1)) || ((item == ITEM_MAGIC_BEANS) && (ammo == 20)) ||
                   ((item == ITEM_FAIRY_SLINGSHOT) && (ammo == Nei_SlingshotCapacity()))) { // Skijer's NEI
            // Ammo at capacity
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 120, 255, 0, pauseCtx->alpha);
        }
    }

    // Separate ammo into upper and lower digits
    for (ammoUpperDigit = 0; ammo >= 10; ammoUpperDigit++) {
        ammo -= 10;
    }

    gDPPipeSync(POLY_OPA_DISP++);

    // Draw upper digit
    if (ammoUpperDigit != 0) {
        POLY_OPA_DISP =
            Gfx_DrawTexRectIA8(POLY_OPA_DISP, gAmmoDigitTextures[ammoUpperDigit], 8, 8, nRectLeft,
                               nRectTop, 8, 8, 1 << 10, 1 << 10);
    }

    // Draw lower digit
    POLY_OPA_DISP = Gfx_DrawTexRectIA8(POLY_OPA_DISP, gAmmoDigitTextures[ammo], 8, 8, nRectLeft + 6,
                                       nRectTop, 8, 8, 1 << 10, 1 << 10);

    CLOSE_DISPS(gfxCtx);
}

void KaleidoScope_SetCursorVtxPos(PauseContext* pauseCtx, u16 vtxIndex, Vtx* vtx) {
    pauseCtx->cursorVtx[0].v.ob[0] = vtx[vtxIndex].v.ob[0];
    pauseCtx->cursorVtx[0].v.ob[1] = vtx[vtxIndex].v.ob[1];
}

static s16 sMagicArrowEffectsR[] = { 255, 100, 255 };
static s16 sMagicArrowEffectsG[] = { 0, 100, 255 };
static s16 sMagicArrowEffectsB[] = { 0, 255, 100 };

void KaleidoScope_DrawItemSelect(PlayState* play) {
    PauseContext* pauseCtx = &play->pauseCtx;
    u16 i;
    u16 j;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL42_Opa(play->state.gfxCtx);

    // NEI: tick the extended-inventory pagination timer (the L page-switch is handled in
    // KaleidoScope_UpdateItemCursor where input is fresh). Grid + cursor are page-aware via
    // ExtInv_GetInventorySlot (page 0 = identity, so vanilla behaviour is unchanged).
    ExtInv_Update();

    // Draw a white box around the items that are equipped on the C buttons
    // Loop over c-buttons (i) and vtx offset (j)
    gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);
    for (i = 0, j = ITEM_NUM_SLOTS * 4; i < 3; i++, j += 4) {
        if (GET_CUR_FORM_BTN_ITEM(i + 1) != ITEM_NONE) {
            ItemId item = GET_CUR_FORM_BTN_ITEM(i + 1);
            if (GameInteractor_Should(VB_DRAW_ITEM_EQUIPPED_OUTLINE, (GET_CUR_FORM_BTN_SLOT(i + 1) < ITEM_NUM_SLOTS),
                                      &item, (s32)(i + 1), 0, PAUSE_ITEM)) {
                gSPVertex(POLY_OPA_DISP++, &pauseCtx->itemVtx[j], 4, 0);
                POLY_OPA_DISP = Gfx_DrawTexQuadIA8(POLY_OPA_DISP, gEquippedItemOutlineTex, 32, 32, 0);
            }
        }
    }
    // #region 2S2H [Dpad]
    if (CVarGetInteger("gEnhancements.Dpad.DpadEquips", 0)) {
        for (i = EQUIP_SLOT_D_RIGHT; i <= EQUIP_SLOT_D_UP; i++, j += 4) {
            if (DPAD_GET_CUR_FORM_BTN_ITEM(i) != ITEM_NONE) {
                ItemId item = DPAD_GET_CUR_FORM_BTN_ITEM(i);
                if (GameInteractor_Should(VB_DRAW_ITEM_EQUIPPED_OUTLINE,
                                          (DPAD_GET_CUR_FORM_BTN_SLOT(i) < ITEM_NUM_SLOTS), &item, (s32)i, 1,
                                          PAUSE_ITEM)) {
                    gSPVertex(POLY_OPA_DISP++, &pauseCtx->itemVtx[j], 4, 0);
                    POLY_OPA_DISP = Gfx_DrawTexQuadIA8(POLY_OPA_DISP, gEquippedItemOutlineTex, 32, 32, 0);
                }
            }
        }
    }
    // #endregion

    gDPPipeSync(POLY_OPA_DISP++);

    // Draw the item icons
    // Loop over slots (i) and vtx offset (j)
    gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    for (j = 0, i = 0; i < ITEM_NUM_SLOTS; i++, j += 4) {
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);

        if (((void)0, ExtInv_GetSlotItem(ExtInv_GetInventorySlot(i))) != ITEM_NONE) {
            if ((pauseCtx->mainState == PAUSE_MAIN_STATE_IDLE) && (pauseCtx->pageIndex == PAUSE_ITEM) &&
                (pauseCtx->cursorSpecialPos == 0) && gPlayerFormSlotRestrictions[GET_PLAYER_FORM][i]) {
                if ((sEquipState == EQUIP_STATE_MAGIC_ARROW_HOVER_OVER_BOW_SLOT) && (i == SLOT_ARROW_ICE)) {
                    // Possible bug:
                    // Supposed to be `SLOT_BOW`, unchanged from OoT, instead increase size of ice arrow icon
                    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, sMagicArrowEffectsR[pauseCtx->equipTargetItem - 0xB5],
                                    sMagicArrowEffectsG[pauseCtx->equipTargetItem - 0xB5],
                                    sMagicArrowEffectsB[pauseCtx->equipTargetItem - 0xB5], pauseCtx->alpha);

                    pauseCtx->itemVtx[j + 0].v.ob[0] = pauseCtx->itemVtx[j + 2].v.ob[0] =
                        pauseCtx->itemVtx[j + 0].v.ob[0] - 2;
                    pauseCtx->itemVtx[j + 1].v.ob[0] = pauseCtx->itemVtx[j + 3].v.ob[0] =
                        pauseCtx->itemVtx[j + 0].v.ob[0] + 32;
                    pauseCtx->itemVtx[j + 0].v.ob[1] = pauseCtx->itemVtx[j + 1].v.ob[1] =
                        pauseCtx->itemVtx[j + 0].v.ob[1] + 2;
                    pauseCtx->itemVtx[j + 2].v.ob[1] = pauseCtx->itemVtx[j + 3].v.ob[1] =
                        pauseCtx->itemVtx[j + 0].v.ob[1] - 32;

                } else if (i == pauseCtx->cursorSlot[PAUSE_ITEM]) {
                    // Increase the size of the selected item
                    pauseCtx->itemVtx[j + 0].v.ob[0] = pauseCtx->itemVtx[j + 2].v.ob[0] =
                        pauseCtx->itemVtx[j + 0].v.ob[0] - 2;
                    pauseCtx->itemVtx[j + 1].v.ob[0] = pauseCtx->itemVtx[j + 3].v.ob[0] =
                        pauseCtx->itemVtx[j + 0].v.ob[0] + 32;
                    pauseCtx->itemVtx[j + 0].v.ob[1] = pauseCtx->itemVtx[j + 1].v.ob[1] =
                        pauseCtx->itemVtx[j + 0].v.ob[1] + 2;
                    pauseCtx->itemVtx[j + 2].v.ob[1] = pauseCtx->itemVtx[j + 3].v.ob[1] =
                        pauseCtx->itemVtx[j + 0].v.ob[1] - 32;
                }
            }
            // #region 2S2H [Port] Originally this was done in KaleidoScope_Update, but now we are using gSPGrayscale.
            ItemId itemId = ExtInv_GetSlotItem(ExtInv_GetInventorySlot(i));
            u8 itemRestricted = GameInteractor_Should(
                VB_ITEM_BE_RESTRICTED,
                ((s32)itemId < 114 ? !gPlayerFormItemRestrictions[GET_PLAYER_FORM][(s32)itemId] : false), &itemId);
            if (itemRestricted) {
                gDPSetGrayscaleColor(POLY_OPA_DISP++, 109, 109, 109, 255);
                gSPGrayscale(POLY_OPA_DISP++, true);
            }
            gSPVertex(POLY_OPA_DISP++, &pauseCtx->itemVtx[j + 0], 4, 0);
            {
                u16 nIt = ExtInv_GetSlotItem(ExtInv_GetInventorySlot(i));
                u8 nSz = ExtInv_GetItemIconSize(nIt);
                if (nIt >= ITEM_SW97_ARROW_FIRE && nIt <= ITEM_SW97_ARROW_WIND) {
                    // OoT style: the element's medallion at HALF alpha behind, bow icon on top
                    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha >> 1);
                    KaleidoScope_DrawTexQuadRGBA32(play->state.gfxCtx, ExtInv_GetItemIcon(nIt), 24, 24, 0);
                    gDPPipeSync(POLY_OPA_DISP++);
                    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);
                    gSPVertex(POLY_OPA_DISP++, &pauseCtx->itemVtx[j + 0], 4, 0);
                    KaleidoScope_DrawTexQuadRGBA32(play->state.gfxCtx, ExtInv_GetItemIcon(ITEM_BOW), 32, 32, 0);
                } else if (nIt >= ITEM_SW97_BULLET_FIRE && nIt <= ITEM_SW97_BULLET_WIND) {
                    // Slingshot-wheel twin: element medallion at HALF alpha behind, slingshot on top
                    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha >> 1);
                    KaleidoScope_DrawTexQuadRGBA32(play->state.gfxCtx, ExtInv_GetItemIcon(nIt), 24, 24, 0);
                    gDPPipeSync(POLY_OPA_DISP++);
                    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);
                    gSPVertex(POLY_OPA_DISP++, &pauseCtx->itemVtx[j + 0], 4, 0);
                    KaleidoScope_DrawTexQuadRGBA32(play->state.gfxCtx, ExtInv_GetItemIcon(ITEM_FAIRY_SLINGSHOT), 32,
                                                   32, 0);
                } else {
                    KaleidoScope_DrawTexQuadRGBA32(play->state.gfxCtx, ExtInv_GetItemIcon(nIt), nSz, nSz, 0);
                }
            }
            if (itemRestricted) {
                gSPGrayscale(POLY_OPA_DISP++, false);
            }
            // #endregion
        }
    }

    // Draw the ammo digits
    if (pauseCtx->pageIndex == PAUSE_ITEM) {
        if ((pauseCtx->state == PAUSE_STATE_MAIN) &&
            ((pauseCtx->mainState == PAUSE_MAIN_STATE_IDLE) || (pauseCtx->mainState == PAUSE_MAIN_STATE_EQUIP_ITEM)) &&
            (pauseCtx->state != PAUSE_STATE_SAVEPROMPT) && !IS_PAUSE_STATE_GAMEOVER(pauseCtx)) {
            Gfx_SetupDL39_Opa(play->state.gfxCtx);
            gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);

            // Loop over slots (i) and ammoIndex (j)
            for (j = 0, i = 0; i < ITEM_NUM_SLOTS; i++) {
                // NEI: page-0 layout is remapped, so ammo is ITEM-based (not slot-indexed). The
                // Bottomless Bottle cell (SLOT_BOTTLE_4) is SLOT-based: its counter always shows
                // while owned, regardless of which content sits in it. Skijer's NEI
                u8 bottomlessCell = (ExtInv_GetCurrentPage() == 0) && (i == SLOT_BOTTLE_4) && Bottle_BottomlessOwned();
                if (ExtInv_GetCurrentPage() == 0 &&
                    (bottomlessCell || ExtInv_ItemHasAmmo((u8)ExtInv_GetSlotItem(ExtInv_GetInventorySlot(i))))) {
                    if (bottomlessCell ||
                        (((void)0, ExtInv_GetSlotItem(ExtInv_GetInventorySlot(i))) != ITEM_NONE)) {
                        KaleidoScope_DrawAmmoCount(pauseCtx, play->state.gfxCtx, ((void)0, ExtInv_GetSlotItem(ExtInv_GetInventorySlot(i))), i);
                    }
                    j++;
                }
            }
            Gfx_SetupDL42_Opa(play->state.gfxCtx);
        }
    }

    // NEI: cycle overlays (A-hint, candidate icons) on top of the grid
    KaleidoScope_DrawItemCycles(play);

    CLOSE_DISPS(play->state.gfxCtx);
}

u8 sPlayerFormItems[PLAYER_FORM_MAX] = {
    ITEM_MASK_FIERCE_DEITY, // PLAYER_FORM_FIERCE_DEITY
    ITEM_MASK_GORON,        // PLAYER_FORM_GORON
    ITEM_MASK_ZORA,         // PLAYER_FORM_ZORA
    ITEM_MASK_DEKU,         // PLAYER_FORM_DEKU
    ITEM_NONE,              // PLAYER_FORM_HUMAN
};

void KaleidoScope_UpdateItemCursor(PlayState* play) {
    s32 pad1;
    PauseContext* pauseCtx = &play->pauseCtx;
    MessageContext* msgCtx = &play->msgCtx;
    u16 vtxIndex;
    u16 cursorItem;
    u16 cursorSlot;
    u8 magicArrowIndex;
    s16 cursorPoint;
    s16 cursorXIndex;
    s16 cursorYIndex;
    s16 oldCursorPoint;
    s16 moveCursorResult;
    s16 pad2;

    pauseCtx->cursorColorSet = PAUSE_CURSOR_COLOR_SET_WHITE;
    pauseCtx->nameColorSet = PAUSE_NAME_COLOR_SET_WHITE;

    if ((pauseCtx->state == PAUSE_STATE_MAIN) && (pauseCtx->mainState == PAUSE_MAIN_STATE_IDLE) &&
        (pauseCtx->pageIndex == PAUSE_ITEM) && !pauseCtx->itemDescriptionOn) {
        moveCursorResult = PAUSE_CURSOR_RESULT_NONE;
        oldCursorPoint = pauseCtx->cursorPoint[PAUSE_ITEM];

        cursorItem = pauseCtx->cursorItem[PAUSE_ITEM];

        // NEI: an open wheel OWNS the stick (stick cycles, A confirms/closes) - run the cycle
        // handler and skip all pause-cursor movement until it closes.
        if (KaleidoScope_IsItemCycling()) {
            KaleidoScope_HandleItemCycles(play);
            return;
        }

        // Move cursor left/right
        if (pauseCtx->cursorSpecialPos == 0) {
            // cursor is currently on a slot
            pauseCtx->cursorColorSet = PAUSE_CURSOR_COLOR_SET_YELLOW;

            if (ABS_ALT(pauseCtx->stickAdjX) > 30) {
                cursorPoint = pauseCtx->cursorPoint[PAUSE_ITEM];
                cursorXIndex = pauseCtx->cursorXIndex[PAUSE_ITEM];
                cursorYIndex = pauseCtx->cursorYIndex[PAUSE_ITEM];

                // Search for slot to move to
                while (moveCursorResult == PAUSE_CURSOR_RESULT_NONE) {
                    if (pauseCtx->stickAdjX < -30) {
                        // move cursor left
                        pauseCtx->cursorShrinkRate = 4.0f;
                        if (pauseCtx->cursorXIndex[PAUSE_ITEM] != 0) {
                            pauseCtx->cursorXIndex[PAUSE_ITEM]--;
                            pauseCtx->cursorPoint[PAUSE_ITEM]--;
                            moveCursorResult = PAUSE_CURSOR_RESULT_SLOT;
                        } else {
                            pauseCtx->cursorXIndex[PAUSE_ITEM] = cursorXIndex;
                            pauseCtx->cursorYIndex[PAUSE_ITEM]++;

                            if (pauseCtx->cursorYIndex[PAUSE_ITEM] >= 4) {
                                pauseCtx->cursorYIndex[PAUSE_ITEM] = 0;
                            }

                            pauseCtx->cursorPoint[PAUSE_ITEM] =
                                pauseCtx->cursorXIndex[PAUSE_ITEM] + (pauseCtx->cursorYIndex[PAUSE_ITEM] * 6);

                            if (pauseCtx->cursorPoint[PAUSE_ITEM] >= ITEM_NUM_SLOTS) {
                                pauseCtx->cursorPoint[PAUSE_ITEM] = pauseCtx->cursorXIndex[PAUSE_ITEM];
                            }

                            if (cursorYIndex == pauseCtx->cursorYIndex[PAUSE_ITEM]) {
                                pauseCtx->cursorXIndex[PAUSE_ITEM] = cursorXIndex;
                                pauseCtx->cursorPoint[PAUSE_ITEM] = cursorPoint;

                                KaleidoScope_MoveCursorToSpecialPos(play, PAUSE_CURSOR_PAGE_LEFT);

                                moveCursorResult = PAUSE_CURSOR_RESULT_SPECIAL_POS;
                            }
                        }
                    } else if (pauseCtx->stickAdjX > 30) {
                        // move cursor right
                        pauseCtx->cursorShrinkRate = 4.0f;
                        if (pauseCtx->cursorXIndex[PAUSE_ITEM] <= 4) {
                            pauseCtx->cursorXIndex[PAUSE_ITEM]++;
                            pauseCtx->cursorPoint[PAUSE_ITEM]++;
                            moveCursorResult = PAUSE_CURSOR_RESULT_SLOT;
                        } else {
                            pauseCtx->cursorXIndex[PAUSE_ITEM] = cursorXIndex;
                            pauseCtx->cursorYIndex[PAUSE_ITEM]++;

                            if (pauseCtx->cursorYIndex[PAUSE_ITEM] >= 4) {
                                pauseCtx->cursorYIndex[PAUSE_ITEM] = 0;
                            }

                            pauseCtx->cursorPoint[PAUSE_ITEM] =
                                pauseCtx->cursorXIndex[PAUSE_ITEM] + (pauseCtx->cursorYIndex[PAUSE_ITEM] * 6);

                            if (pauseCtx->cursorPoint[PAUSE_ITEM] >= ITEM_NUM_SLOTS) {
                                pauseCtx->cursorPoint[PAUSE_ITEM] = pauseCtx->cursorXIndex[PAUSE_ITEM];
                            }

                            if (cursorYIndex == pauseCtx->cursorYIndex[PAUSE_ITEM]) {
                                pauseCtx->cursorXIndex[PAUSE_ITEM] = cursorXIndex;
                                pauseCtx->cursorPoint[PAUSE_ITEM] = cursorPoint;

                                KaleidoScope_MoveCursorToSpecialPos(play, PAUSE_CURSOR_PAGE_RIGHT);

                                moveCursorResult = PAUSE_CURSOR_RESULT_SPECIAL_POS;
                            }
                        }
                    }
                }

                if (moveCursorResult == PAUSE_CURSOR_RESULT_SLOT) {
                    cursorItem = ExtInv_GetSlotItem(ExtInv_GetInventorySlot(pauseCtx->cursorPoint[PAUSE_ITEM]));
                }
            }
        } else if (pauseCtx->cursorSpecialPos == PAUSE_CURSOR_PAGE_LEFT) {
            if (pauseCtx->stickAdjX > 30) {
                KaleidoScope_MoveCursorFromSpecialPos(play);
                cursorYIndex = 0;
                cursorXIndex = 0;
                cursorPoint = 0; // top row, left column (SLOT_OCARINA)

                // Search for slot to move to
                while (true) {
                    // Check if current cursor has an item in its slot
                    if (ExtInv_GetSlotItem(ExtInv_GetInventorySlot(cursorPoint)) != ITEM_NONE) {
                        pauseCtx->cursorPoint[PAUSE_ITEM] = cursorPoint;
                        pauseCtx->cursorXIndex[PAUSE_ITEM] = cursorXIndex;
                        pauseCtx->cursorYIndex[PAUSE_ITEM] = cursorYIndex;
                        moveCursorResult = PAUSE_CURSOR_RESULT_SLOT;
                        break;
                    }

                    // move 1 row down and retry
                    cursorYIndex++;
                    cursorPoint += 6;
                    if (cursorYIndex < 4) {
                        continue;
                    }

                    // move 1 column right and retry
                    cursorYIndex = 0;
                    cursorPoint = cursorXIndex + 1;
                    cursorXIndex = cursorPoint;
                    if (cursorXIndex < 6) {
                        continue;
                    }

                    // No item available
                    KaleidoScope_MoveCursorToSpecialPos(play, PAUSE_CURSOR_PAGE_RIGHT);
                    break;
                }
            }
        } else { // PAUSE_CURSOR_PAGE_RIGHT
            if (pauseCtx->stickAdjX < -30) {
                KaleidoScope_MoveCursorFromSpecialPos(play);
                cursorXIndex = 5;
                cursorPoint = 5; // top row, right columne (SLOT_TRADE_DEED)
                cursorYIndex = 0;

                // Search for slot to move to
                while (true) {
                    // Check if current cursor has an item in its slot
                    if (ExtInv_GetSlotItem(ExtInv_GetInventorySlot(cursorPoint)) != ITEM_NONE) {
                        pauseCtx->cursorPoint[PAUSE_ITEM] = cursorPoint;
                        pauseCtx->cursorXIndex[PAUSE_ITEM] = cursorXIndex;
                        pauseCtx->cursorYIndex[PAUSE_ITEM] = cursorYIndex;
                        moveCursorResult = PAUSE_CURSOR_RESULT_SLOT;
                        break;
                    }

                    // move 1 row down and retry
                    cursorYIndex++;
                    cursorPoint += 6;
                    if (cursorYIndex < 4) {
                        continue;
                    }

                    // move 1 column left and retry
                    cursorYIndex = 0;
                    cursorPoint = cursorXIndex - 1;
                    cursorXIndex = cursorPoint;
                    if (cursorXIndex >= 0) {
                        continue;
                    }

                    // No item available
                    KaleidoScope_MoveCursorToSpecialPos(play, PAUSE_CURSOR_PAGE_LEFT);
                    break;
                }
            }
        }

        if (pauseCtx->cursorSpecialPos == 0) {
            // move cursor up/down
            if (ABS_ALT(pauseCtx->stickAdjY) > 30) {
                moveCursorResult = PAUSE_CURSOR_RESULT_NONE;

                cursorPoint = pauseCtx->cursorPoint[PAUSE_ITEM];
                cursorYIndex = pauseCtx->cursorYIndex[PAUSE_ITEM];

                while (moveCursorResult == PAUSE_CURSOR_RESULT_NONE) {
                    if (pauseCtx->stickAdjY > 30) {
                        // move cursor up
                        moveCursorResult = PAUSE_CURSOR_RESULT_SPECIAL_POS;
                        if (pauseCtx->cursorYIndex[PAUSE_ITEM] != 0) {
                            pauseCtx->cursorYIndex[PAUSE_ITEM]--;
                            pauseCtx->cursorShrinkRate = 4.0f;
                            pauseCtx->cursorPoint[PAUSE_ITEM] -= 6;
                            moveCursorResult = PAUSE_CURSOR_RESULT_SLOT;
                        } else {
                            pauseCtx->cursorYIndex[PAUSE_ITEM] = cursorYIndex;
                            pauseCtx->cursorPoint[PAUSE_ITEM] = cursorPoint;
                        }
                    } else if (pauseCtx->stickAdjY < -30) {
                        // move cursor down
                        moveCursorResult = PAUSE_CURSOR_RESULT_SPECIAL_POS;
                        if (pauseCtx->cursorYIndex[PAUSE_ITEM] < 3) {
                            pauseCtx->cursorYIndex[PAUSE_ITEM]++;
                            pauseCtx->cursorShrinkRate = 4.0f;
                            pauseCtx->cursorPoint[PAUSE_ITEM] += 6;
                            moveCursorResult = PAUSE_CURSOR_RESULT_SLOT;
                        } else {
                            pauseCtx->cursorYIndex[PAUSE_ITEM] = cursorYIndex;
                            pauseCtx->cursorPoint[PAUSE_ITEM] = cursorPoint;
                        }
                    }
                }
            }

            cursorSlot = pauseCtx->cursorPoint[PAUSE_ITEM];
            pauseCtx->cursorColorSet = PAUSE_CURSOR_COLOR_SET_YELLOW;

            if (moveCursorResult == PAUSE_CURSOR_RESULT_SLOT) {
                cursorItem = ExtInv_GetSlotItem(ExtInv_GetInventorySlot(pauseCtx->cursorPoint[PAUSE_ITEM]));
            } else if (moveCursorResult != PAUSE_CURSOR_RESULT_SPECIAL_POS) {
                cursorItem = ExtInv_GetSlotItem(ExtInv_GetInventorySlot(pauseCtx->cursorPoint[PAUSE_ITEM]));
            }

            if (cursorItem == ITEM_NONE) {
                cursorItem = PAUSE_ITEM_NONE;
                pauseCtx->cursorColorSet = PAUSE_CURSOR_COLOR_SET_WHITE;
            }

            if ((cursorItem != (u32)PAUSE_ITEM_NONE) && (msgCtx->msgLength == 0)) {
                if (gSaveContext.buttonStatus[EQUIP_SLOT_A] == BTN_DISABLED) {
                    gSaveContext.buttonStatus[EQUIP_SLOT_A] = BTN_ENABLED;
                    gSaveContext.hudVisibility = HUD_VISIBILITY_IDLE;
                    Interface_SetHudVisibility(HUD_VISIBILITY_ALL);
                }
            } else if (gSaveContext.buttonStatus[EQUIP_SLOT_A] != BTN_DISABLED) {
                gSaveContext.buttonStatus[EQUIP_SLOT_A] = BTN_DISABLED;
                gSaveContext.hudVisibility = HUD_VISIBILITY_IDLE;
                Interface_SetHudVisibility(HUD_VISIBILITY_ALL);
            }

            pauseCtx->cursorItem[PAUSE_ITEM] = cursorItem;
            pauseCtx->cursorSlot[PAUSE_ITEM] = cursorSlot;

            // NEI: L cycles the extended-inventory sub-page (vanilla / custom items / MM masks).
            // Handled here in the cursor update (like equip / C-Up description) so the press is
            // reliably detected. Takes effect next frame when cursorItem recomputes on the new page.
            if (CHECK_BTN_ALL(CONTROLLER1(&play->state)->press.button, BTN_L) && ExtInv_CanSwitchPage()) {
                ExtInv_SwitchPage();
                Audio_PlaySfx(NA_SE_SY_HP_RECOVER);
            }

            // NEI: per-slot item cycles (bottle wheels; more selectors later). While a cycle is
            // open it captures the input — skip equip/description handling below.
            KaleidoScope_HandleItemCycles(play);
            if (KaleidoScope_IsItemCycling()) {
                return;
            }

            if (cursorItem != PAUSE_ITEM_NONE) {
                // Equip item to the C buttons
                if ((pauseCtx->debugEditor == DEBUG_EDITOR_NONE) && !pauseCtx->itemDescriptionOn &&
                    (pauseCtx->state == PAUSE_STATE_MAIN) && (pauseCtx->mainState == PAUSE_MAIN_STATE_IDLE) &&
                    CHECK_BTN_ANY(CONTROLLER1(&play->state)->press.button,
                                  BTN_CLEFT | BTN_CDOWN | BTN_CRIGHT | BTN_DPAD_EQUIP)) {

                    // Ensure that a transformation mask can not be unequipped while being used
                    if (GET_PLAYER_FORM != PLAYER_FORM_HUMAN) {
                        if (1) {}
                        if (CHECK_BTN_ALL(CONTROLLER1(&play->state)->press.button, BTN_CLEFT)) {
                            if (sPlayerFormItems[GET_PLAYER_FORM] == BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT)) {
                                Audio_PlaySfx(NA_SE_SY_ERROR);
                                return;
                            }
                        } else if (CHECK_BTN_ALL(CONTROLLER1(&play->state)->press.button, BTN_CDOWN)) {
                            if (sPlayerFormItems[GET_PLAYER_FORM] == BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN)) {
                                Audio_PlaySfx(NA_SE_SY_ERROR);
                                return;
                            }
                        } else if (CHECK_BTN_ALL(CONTROLLER1(&play->state)->press.button, BTN_CRIGHT)) {
                            if (sPlayerFormItems[GET_PLAYER_FORM] == BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT)) {
                                Audio_PlaySfx(NA_SE_SY_ERROR);
                                return;
                            }
                        }
                        // #region 2S2H [Dpad]
                        else if (CVarGetInteger("gEnhancements.Dpad.DpadEquips", 0)) {
                            if (CHECK_BTN_ALL(CONTROLLER1(&play->state)->press.button, BTN_DRIGHT)) {
                                if (sPlayerFormItems[GET_PLAYER_FORM] ==
                                    DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT)) {
                                    Audio_PlaySfx(NA_SE_SY_ERROR);
                                    return;
                                }
                            } else if (CHECK_BTN_ALL(CONTROLLER1(&play->state)->press.button, BTN_DLEFT)) {
                                if (sPlayerFormItems[GET_PLAYER_FORM] == DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT)) {
                                    Audio_PlaySfx(NA_SE_SY_ERROR);
                                    return;
                                }
                            } else if (CHECK_BTN_ALL(CONTROLLER1(&play->state)->press.button, BTN_DDOWN)) {
                                if (sPlayerFormItems[GET_PLAYER_FORM] == DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN)) {
                                    Audio_PlaySfx(NA_SE_SY_ERROR);
                                    return;
                                }
                            } else if (CHECK_BTN_ALL(CONTROLLER1(&play->state)->press.button, BTN_DUP)) {
                                if (sPlayerFormItems[GET_PLAYER_FORM] == DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP)) {
                                    Audio_PlaySfx(NA_SE_SY_ERROR);
                                    return;
                                }
                            }
                        }
                        // #endregion
                    }

                    // Ensure that a non-transformation mask can not be unequipped while being used
                    if (CHECK_BTN_ALL(CONTROLLER1(&play->state)->press.button, BTN_CLEFT)) {
                        if ((Player_GetCurMaskItemId(play) != ITEM_NONE) &&
                            (Player_GetCurMaskItemId(play) == BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT))) {
                            Audio_PlaySfx(NA_SE_SY_ERROR);
                            return;
                        }
                        pauseCtx->equipTargetCBtn = PAUSE_EQUIP_C_LEFT;
                    } else if (CHECK_BTN_ALL(CONTROLLER1(&play->state)->press.button, BTN_CDOWN)) {
                        if ((Player_GetCurMaskItemId(play) != ITEM_NONE) &&
                            (Player_GetCurMaskItemId(play) == BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN))) {
                            Audio_PlaySfx(NA_SE_SY_ERROR);
                            return;
                        }
                        pauseCtx->equipTargetCBtn = PAUSE_EQUIP_C_DOWN;
                    } else if (CHECK_BTN_ALL(CONTROLLER1(&play->state)->press.button, BTN_CRIGHT)) {
                        if ((Player_GetCurMaskItemId(play) != ITEM_NONE) &&
                            (Player_GetCurMaskItemId(play) == BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT))) {
                            Audio_PlaySfx(NA_SE_SY_ERROR);
                            return;
                        }
                        pauseCtx->equipTargetCBtn = PAUSE_EQUIP_C_RIGHT;
                    }
                    // #region 2S2H [Dpad]
                    else if (CVarGetInteger("gEnhancements.Dpad.DpadEquips", 0)) {
                        if (CHECK_BTN_ALL(CONTROLLER1(&play->state)->press.button, BTN_DRIGHT)) {
                            if ((Player_GetCurMaskItemId(play) != ITEM_NONE) &&
                                (Player_GetCurMaskItemId(play) == DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT))) {
                                Audio_PlaySfx(NA_SE_SY_ERROR);
                                return;
                            }
                            pauseCtx->equipTargetCBtn = PAUSE_EQUIP_D_RIGHT;
                        } else if (CHECK_BTN_ALL(CONTROLLER1(&play->state)->press.button, BTN_DLEFT)) {
                            if ((Player_GetCurMaskItemId(play) != ITEM_NONE) &&
                                (Player_GetCurMaskItemId(play) == DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT))) {
                                Audio_PlaySfx(NA_SE_SY_ERROR);
                                return;
                            }
                            pauseCtx->equipTargetCBtn = PAUSE_EQUIP_D_LEFT;
                        } else if (CHECK_BTN_ALL(CONTROLLER1(&play->state)->press.button, BTN_DDOWN)) {
                            if ((Player_GetCurMaskItemId(play) != ITEM_NONE) &&
                                (Player_GetCurMaskItemId(play) == DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN))) {
                                Audio_PlaySfx(NA_SE_SY_ERROR);
                                return;
                            }
                            pauseCtx->equipTargetCBtn = PAUSE_EQUIP_D_DOWN;
                        } else if (CHECK_BTN_ALL(CONTROLLER1(&play->state)->press.button, BTN_DUP)) {
                            if ((Player_GetCurMaskItemId(play) != ITEM_NONE) &&
                                (Player_GetCurMaskItemId(play) == DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP))) {
                                Audio_PlaySfx(NA_SE_SY_ERROR);
                                return;
                            }
                            pauseCtx->equipTargetCBtn = PAUSE_EQUIP_D_UP;
                        }
                    }
                    // #endregion
                    if (!GameInteractor_Should(VB_KALEIDO_EQUIP_ITEM_TO_BUTTON, true, cursorSlot, cursorItem)) {
                        return;
                    }

                    // Equip item to the C buttons
                    pauseCtx->equipTargetItem = cursorItem;
                    // NEI: record the REAL save slot (page-aware). Vanilla mask equips store
                    // cursorSlot + ITEM_NUM_SLOTS (24..47, see z_kaleido_mask.c) — ExtInv page 2
                    // visual slots are 48..71, so translate back to the real mask slot range.
                    {
                        s32 realSlot = ExtInv_GetInventorySlot(cursorSlot);
                        if (realSlot >= 48 && realSlot < 72) {
                            realSlot -= 24; // page-2 visual -> real mask slot (24..47)
                        }
                        pauseCtx->equipTargetSlot = realSlot;
                    }
                    pauseCtx->mainState = PAUSE_MAIN_STATE_EQUIP_ITEM;
                    vtxIndex = cursorSlot * 4;
                    pauseCtx->equipAnimX = pauseCtx->itemVtx[vtxIndex].v.ob[0] * 10;
                    pauseCtx->equipAnimY = pauseCtx->itemVtx[vtxIndex].v.ob[1] * 10;
                    pauseCtx->equipAnimAlpha = 255;
                    sEquipMagicArrowSlotHoldTimer = 0;
                    sEquipState = EQUIP_STATE_MOVE_TO_C_BTN;
                    sEquipAnimTimer = 10;

                    if ((pauseCtx->equipTargetItem == ITEM_ARROW_FIRE) ||
                        (pauseCtx->equipTargetItem == ITEM_ARROW_ICE) ||
                        (pauseCtx->equipTargetItem == ITEM_ARROW_LIGHT)) {
                        magicArrowIndex = 0;
                        if (pauseCtx->equipTargetItem == ITEM_ARROW_ICE) {
                            magicArrowIndex = 1;
                        }
                        if (pauseCtx->equipTargetItem == ITEM_ARROW_LIGHT) {
                            magicArrowIndex = 2;
                        }
                        Audio_PlaySfx(NA_SE_SY_SET_FIRE_ARROW + magicArrowIndex);
                        pauseCtx->equipTargetItem = 0xB5 + magicArrowIndex;
                        pauseCtx->equipAnimAlpha = sEquipState = 0; // EQUIP_STATE_MAGIC_ARROW_GROW_ORB
                        sEquipAnimTimer = 6;
                    } else {
                        Audio_PlaySfx(NA_SE_SY_DECIDE);
                    }
                } else if ((pauseCtx->debugEditor == DEBUG_EDITOR_NONE) && (pauseCtx->state == PAUSE_STATE_MAIN) &&
                           (pauseCtx->mainState == PAUSE_MAIN_STATE_IDLE) &&
                           // NEI: item description moved A -> C-Up to match OoT, freeing A for the
                           // NEI item-cycle (KaleidoScope_HandleItemCycles). Other pages keep A.
                           CHECK_BTN_ALL(CONTROLLER1(&play->state)->press.button, BTN_CUP) &&
                           (msgCtx->msgLength == 0)) {
                    if (GameInteractor_Should(VB_KALEIDO_DISPLAY_ITEM_TEXT, true, &cursorItem)) {
                        // Give description on item through a message box
                        pauseCtx->itemDescriptionOn = true;
                        if (pauseCtx->cursorYIndex[PAUSE_ITEM] < 2) {
                            func_801514B0(play, 0x1700 + pauseCtx->cursorItem[PAUSE_ITEM], 3);
                        } else {
                            func_801514B0(play, 0x1700 + pauseCtx->cursorItem[PAUSE_ITEM], 1);
                        }
                    }
                }
            }
        } else {
            pauseCtx->cursorItem[PAUSE_ITEM] = PAUSE_ITEM_NONE;
        }

        if (oldCursorPoint != pauseCtx->cursorPoint[PAUSE_ITEM]) {
            Audio_PlaySfx(NA_SE_SY_CURSOR);
        }
    } else if ((pauseCtx->mainState == PAUSE_MAIN_STATE_EQUIP_ITEM) && (pauseCtx->pageIndex == PAUSE_ITEM)) {
        pauseCtx->cursorColorSet = PAUSE_CURSOR_COLOR_SET_YELLOW;
    }
}

s16 sCButtonPosX[] = {
    660, 900, 1140,
    // #region 2S2H [Dpad]
    1350, 1030, 1190, 1190
    // #endregion
};
s16 sCButtonPosY[] = {
    1100, 920, 1100,
    // #region 2S2H [Dpad]
    570, 570, 410, 730
    // #endregion
};

// #region 2S2H [Dpad]
void KaleidoScope_SwapDpadItemToCItem(PlayState* play, EquipSlot cEquipSlot) {
    PauseContext* pauseCtx = &play->pauseCtx;

    if (pauseCtx->equipTargetSlot == DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_RIGHT)) {
        if ((BUTTON_ITEM_EQUIP(0, cEquipSlot) & 0xFF) != ITEM_NONE) {
            if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                (((BUTTON_ITEM_EQUIP(0, cEquipSlot) & 0xFF) == ITEM_BOW) ||
                 (((BUTTON_ITEM_EQUIP(0, cEquipSlot) & 0xFF) >= ITEM_BOW_FIRE) &&
                  ((BUTTON_ITEM_EQUIP(0, cEquipSlot) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                pauseCtx->equipTargetSlot = SLOT_BOW;
            } else {
                DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) = BUTTON_ITEM_EQUIP(0, cEquipSlot);
                DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_RIGHT) = C_SLOT_EQUIP(0, cEquipSlot);
                Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_RIGHT);
            }
        } else {
            DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) = ITEM_NONE;
            DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_RIGHT) = SLOT_NONE;
        }
    } else if (pauseCtx->equipTargetSlot == DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_LEFT)) {
        if ((BUTTON_ITEM_EQUIP(0, cEquipSlot) & 0xFF) != ITEM_NONE) {
            if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                (((BUTTON_ITEM_EQUIP(0, cEquipSlot) & 0xFF) == ITEM_BOW) ||
                 (((BUTTON_ITEM_EQUIP(0, cEquipSlot) & 0xFF) >= ITEM_BOW_FIRE) &&
                  ((BUTTON_ITEM_EQUIP(0, cEquipSlot) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                pauseCtx->equipTargetSlot = SLOT_BOW;
            } else {
                DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) = BUTTON_ITEM_EQUIP(0, cEquipSlot);
                DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_LEFT) = C_SLOT_EQUIP(0, cEquipSlot);
                Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_LEFT);
            }
        } else {
            DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) = ITEM_NONE;
            DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_LEFT) = SLOT_NONE;
        }
    } else if (pauseCtx->equipTargetSlot == DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_DOWN)) {
        if ((BUTTON_ITEM_EQUIP(0, cEquipSlot) & 0xFF) != ITEM_NONE) {
            if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                (((BUTTON_ITEM_EQUIP(0, cEquipSlot) & 0xFF) == ITEM_BOW) ||
                 (((BUTTON_ITEM_EQUIP(0, cEquipSlot) & 0xFF) >= ITEM_BOW_FIRE) &&
                  ((BUTTON_ITEM_EQUIP(0, cEquipSlot) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                pauseCtx->equipTargetSlot = SLOT_BOW;
            } else {
                DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) = BUTTON_ITEM_EQUIP(0, cEquipSlot);
                DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_DOWN) = C_SLOT_EQUIP(0, cEquipSlot);
                Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_DOWN);
            }
        } else {
            DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) = ITEM_NONE;
            DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_DOWN) = SLOT_NONE;
        }
    } else if (pauseCtx->equipTargetSlot == DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_UP)) {
        if ((BUTTON_ITEM_EQUIP(0, cEquipSlot) & 0xFF) != ITEM_NONE) {
            if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                (((BUTTON_ITEM_EQUIP(0, cEquipSlot) & 0xFF) == ITEM_BOW) ||
                 (((BUTTON_ITEM_EQUIP(0, cEquipSlot) & 0xFF) >= ITEM_BOW_FIRE) &&
                  ((BUTTON_ITEM_EQUIP(0, cEquipSlot) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                pauseCtx->equipTargetSlot = SLOT_BOW;
            } else {
                DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) = BUTTON_ITEM_EQUIP(0, cEquipSlot);
                DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_UP) = C_SLOT_EQUIP(0, cEquipSlot);
                Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_UP);
            }
        } else {
            DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) = ITEM_NONE;
            DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_UP) = SLOT_NONE;
        }
    }
}

void KaleidoScope_UpdateDpadItemEquip(PlayState* play) {
    PauseContext* pauseCtx = &play->pauseCtx;

    if (pauseCtx->equipTargetCBtn == PAUSE_EQUIP_D_RIGHT) {
        // Swap if item is already equipped on other Item Buttons.
        if (pauseCtx->equipTargetSlot == C_SLOT_EQUIP(0, EQUIP_SLOT_C_LEFT)) {
            if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) & 0xFF) != ITEM_NONE) {
                if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                    (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) & 0xFF) == ITEM_BOW) ||
                     (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) & 0xFF) >= ITEM_BOW_FIRE) &&
                      ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                    pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                    pauseCtx->equipTargetSlot = SLOT_BOW;
                } else {
                    BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT);
                    C_SLOT_EQUIP(0, EQUIP_SLOT_C_LEFT) = DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_RIGHT);
                    Interface_LoadItemIcon(play, EQUIP_SLOT_C_LEFT);
                }
            } else {
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) = ITEM_NONE;
                C_SLOT_EQUIP(0, EQUIP_SLOT_C_LEFT) = SLOT_NONE;
            }
        } else if (pauseCtx->equipTargetSlot == C_SLOT_EQUIP(0, EQUIP_SLOT_C_DOWN)) {
            if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) & 0xFF) != ITEM_NONE) {
                if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                    (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) & 0xFF) == ITEM_BOW) ||
                     (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) & 0xFF) >= ITEM_BOW_FIRE) &&
                      ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                    pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                    pauseCtx->equipTargetSlot = SLOT_BOW;
                } else {
                    BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT);
                    C_SLOT_EQUIP(0, EQUIP_SLOT_C_DOWN) = DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_RIGHT);
                    Interface_LoadItemIcon(play, EQUIP_SLOT_C_DOWN);
                }
            } else {
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) = ITEM_NONE;
                C_SLOT_EQUIP(0, EQUIP_SLOT_C_DOWN) = SLOT_NONE;
            }
        } else if (pauseCtx->equipTargetSlot == C_SLOT_EQUIP(0, EQUIP_SLOT_C_RIGHT)) {
            if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) & 0xFF) != ITEM_NONE) {
                if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                    (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) & 0xFF) == ITEM_BOW) ||
                     (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) & 0xFF) >= ITEM_BOW_FIRE) &&
                      ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                    pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                    pauseCtx->equipTargetSlot = SLOT_BOW;
                } else {
                    BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT);
                    C_SLOT_EQUIP(0, EQUIP_SLOT_C_RIGHT) = DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_RIGHT);
                    Interface_LoadItemIcon(play, EQUIP_SLOT_C_RIGHT);
                }
            } else {
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) = ITEM_NONE;
                C_SLOT_EQUIP(0, EQUIP_SLOT_C_RIGHT) = SLOT_NONE;
            }
        } else if (pauseCtx->equipTargetSlot == DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_LEFT)) {
            if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) & 0xFF) != ITEM_NONE) {
                if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                    (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) & 0xFF) == ITEM_BOW) ||
                     (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) & 0xFF) >= ITEM_BOW_FIRE) &&
                      ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                    pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                    pauseCtx->equipTargetSlot = SLOT_BOW;
                } else {
                    DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT);
                    DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_LEFT) = DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_RIGHT);
                    Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_LEFT);
                }
            } else {
                DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) = ITEM_NONE;
                DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_LEFT) = SLOT_NONE;
            }
        } else if (pauseCtx->equipTargetSlot == DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_DOWN)) {
            if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) & 0xFF) != ITEM_NONE) {
                if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                    (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) & 0xFF) == ITEM_BOW) ||
                     (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) & 0xFF) >= ITEM_BOW_FIRE) &&
                      ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                    pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                    pauseCtx->equipTargetSlot = SLOT_BOW;
                } else {
                    DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT);
                    DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_DOWN) = DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_RIGHT);
                    Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_DOWN);
                }
            } else {
                DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) = ITEM_NONE;
                DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_DOWN) = SLOT_NONE;
            }
        } else if (pauseCtx->equipTargetSlot == DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_UP)) {
            if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) & 0xFF) != ITEM_NONE) {
                if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                    (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) & 0xFF) == ITEM_BOW) ||
                     (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) & 0xFF) >= ITEM_BOW_FIRE) &&
                      ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                    pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                    pauseCtx->equipTargetSlot = SLOT_BOW;
                } else {
                    DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT);
                    DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_UP) = DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_RIGHT);
                    Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_UP);
                }
            } else {
                DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) = ITEM_NONE;
                DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_UP) = SLOT_NONE;
            }
        }

        // Special case for magic arrows
        if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8)) {
            if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) == ITEM_BOW) ||
                ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) >= ITEM_BOW_FIRE) &&
                 (DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) <= ITEM_BOW_LIGHT))) {
                pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                pauseCtx->equipTargetSlot = SLOT_BOW;
            }
        } else if (pauseCtx->equipTargetItem == ITEM_BOW) {
            if ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) >= ITEM_BOW_FIRE) &&
                (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) <= ITEM_BOW_LIGHT)) {
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT);
                Interface_LoadItemIcon(play, EQUIP_SLOT_C_LEFT);
            } else if ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) >= ITEM_BOW_FIRE) &&
                       (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) <= ITEM_BOW_LIGHT)) {
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT);
                Interface_LoadItemIcon(play, EQUIP_SLOT_C_DOWN);
            } else if ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) >= ITEM_BOW_FIRE) &&
                       (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) <= ITEM_BOW_LIGHT)) {
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT);
                Interface_LoadItemIcon(play, EQUIP_SLOT_C_RIGHT);
            } else if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) >= ITEM_BOW_FIRE) &&
                       (DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) <= ITEM_BOW_LIGHT)) {
                DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT);
                Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_LEFT);
            } else if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) >= ITEM_BOW_FIRE) &&
                       (DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) <= ITEM_BOW_LIGHT)) {
                DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT);
                Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_DOWN);
            } else if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) >= ITEM_BOW_FIRE) &&
                       (DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) <= ITEM_BOW_LIGHT)) {
                DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT);
                Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_UP);
            }
        }

        // Equip item on DRight
        DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) = pauseCtx->equipTargetItem;
        DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_RIGHT) = pauseCtx->equipTargetSlot;
        Interface_Dpad_LoadItemIconImpl(play, EQUIP_SLOT_D_RIGHT);
    } else if (pauseCtx->equipTargetCBtn == PAUSE_EQUIP_D_LEFT) {
        // Swap if item is already equipped on other Item Buttons.
        if (pauseCtx->equipTargetSlot == C_SLOT_EQUIP(0, EQUIP_SLOT_C_LEFT)) {
            if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) & 0xFF) != ITEM_NONE) {
                if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                    (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) & 0xFF) == ITEM_BOW) ||
                     (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) & 0xFF) >= ITEM_BOW_FIRE) &&
                      ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                    pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                    pauseCtx->equipTargetSlot = SLOT_BOW;
                } else {
                    BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT);
                    C_SLOT_EQUIP(0, EQUIP_SLOT_C_LEFT) = DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_LEFT);
                    Interface_LoadItemIcon(play, EQUIP_SLOT_C_LEFT);
                }
            } else {
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) = ITEM_NONE;
                C_SLOT_EQUIP(0, EQUIP_SLOT_C_LEFT) = SLOT_NONE;
            }
        } else if (pauseCtx->equipTargetSlot == C_SLOT_EQUIP(0, EQUIP_SLOT_C_DOWN)) {
            if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) & 0xFF) != ITEM_NONE) {
                if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                    (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) & 0xFF) == ITEM_BOW) ||
                     (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) & 0xFF) >= ITEM_BOW_FIRE) &&
                      ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                    pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                    pauseCtx->equipTargetSlot = SLOT_BOW;
                } else {
                    BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT);
                    C_SLOT_EQUIP(0, EQUIP_SLOT_C_DOWN) = DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_LEFT);
                    Interface_LoadItemIcon(play, EQUIP_SLOT_C_DOWN);
                }
            } else {
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) = ITEM_NONE;
                C_SLOT_EQUIP(0, EQUIP_SLOT_C_DOWN) = SLOT_NONE;
            }
        } else if (pauseCtx->equipTargetSlot == C_SLOT_EQUIP(0, EQUIP_SLOT_C_RIGHT)) {
            if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) & 0xFF) != ITEM_NONE) {
                if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                    (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) & 0xFF) == ITEM_BOW) ||
                     (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) & 0xFF) >= ITEM_BOW_FIRE) &&
                      ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                    pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                    pauseCtx->equipTargetSlot = SLOT_BOW;
                } else {
                    BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT);
                    C_SLOT_EQUIP(0, EQUIP_SLOT_C_RIGHT) = DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_LEFT);
                    Interface_LoadItemIcon(play, EQUIP_SLOT_C_RIGHT);
                }
            } else {
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) = ITEM_NONE;
                C_SLOT_EQUIP(0, EQUIP_SLOT_C_RIGHT) = SLOT_NONE;
            }
        } else if (pauseCtx->equipTargetSlot == DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_RIGHT)) {
            if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) & 0xFF) != ITEM_NONE) {
                if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                    (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) & 0xFF) == ITEM_BOW) ||
                     (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) & 0xFF) >= ITEM_BOW_FIRE) &&
                      ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                    pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                    pauseCtx->equipTargetSlot = SLOT_BOW;
                } else {
                    DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT);
                    DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_RIGHT) = DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_LEFT);
                    Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_RIGHT);
                }
            } else {
                DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) = ITEM_NONE;
                DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_RIGHT) = SLOT_NONE;
            }
        } else if (pauseCtx->equipTargetSlot == DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_DOWN)) {
            if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) & 0xFF) != ITEM_NONE) {
                if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                    (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) & 0xFF) == ITEM_BOW) ||
                     (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) & 0xFF) >= ITEM_BOW_FIRE) &&
                      ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                    pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                    pauseCtx->equipTargetSlot = SLOT_BOW;
                } else {
                    DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT);
                    DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_DOWN) = DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_LEFT);
                    Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_DOWN);
                }
            } else {
                DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) = ITEM_NONE;
                DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_DOWN) = SLOT_NONE;
            }
        } else if (pauseCtx->equipTargetSlot == DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_UP)) {
            if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) & 0xFF) != ITEM_NONE) {
                if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                    (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) & 0xFF) == ITEM_BOW) ||
                     (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) & 0xFF) >= ITEM_BOW_FIRE) &&
                      ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                    pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                    pauseCtx->equipTargetSlot = SLOT_BOW;
                } else {
                    DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT);
                    DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_UP) = DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_LEFT);
                    Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_UP);
                }
            } else {
                DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) = ITEM_NONE;
                DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_UP) = SLOT_NONE;
            }
        }

        // Special case for magic arrows
        if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8)) {
            if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) == ITEM_BOW) ||
                ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) >= ITEM_BOW_FIRE) &&
                 (DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) <= ITEM_BOW_LIGHT))) {
                pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                pauseCtx->equipTargetSlot = SLOT_BOW;
            }
        } else if (pauseCtx->equipTargetItem == ITEM_BOW) {
            if ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) >= ITEM_BOW_FIRE) &&
                (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) <= ITEM_BOW_LIGHT)) {
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT);
                Interface_LoadItemIcon(play, EQUIP_SLOT_C_LEFT);
            } else if ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) >= ITEM_BOW_FIRE) &&
                       (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) <= ITEM_BOW_LIGHT)) {
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT);
                Interface_LoadItemIcon(play, EQUIP_SLOT_C_DOWN);
            } else if ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) >= ITEM_BOW_FIRE) &&
                       (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) <= ITEM_BOW_LIGHT)) {
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT);
                Interface_LoadItemIcon(play, EQUIP_SLOT_C_RIGHT);
            } else if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) >= ITEM_BOW_FIRE) &&
                       (DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) <= ITEM_BOW_LIGHT)) {
                DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT);
                Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_RIGHT);
            } else if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) >= ITEM_BOW_FIRE) &&
                       (DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) <= ITEM_BOW_LIGHT)) {
                DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT);
                Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_DOWN);
            } else if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) >= ITEM_BOW_FIRE) &&
                       (DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) <= ITEM_BOW_LIGHT)) {
                DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT);
                Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_UP);
            }
        }

        // Equip item on DLeft
        DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) = pauseCtx->equipTargetItem;
        DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_LEFT) = pauseCtx->equipTargetSlot;
        Interface_Dpad_LoadItemIconImpl(play, EQUIP_SLOT_D_LEFT);
    } else if (pauseCtx->equipTargetCBtn == PAUSE_EQUIP_D_DOWN) {
        // Swap if item is already equipped on other Item Buttons.
        if (pauseCtx->equipTargetSlot == C_SLOT_EQUIP(0, EQUIP_SLOT_C_LEFT)) {
            if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) & 0xFF) != ITEM_NONE) {
                if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                    (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) & 0xFF) == ITEM_BOW) ||
                     (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) & 0xFF) >= ITEM_BOW_FIRE) &&
                      ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                    pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                    pauseCtx->equipTargetSlot = SLOT_BOW;
                } else {
                    BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN);
                    C_SLOT_EQUIP(0, EQUIP_SLOT_C_LEFT) = DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_DOWN);
                    Interface_LoadItemIcon(play, EQUIP_SLOT_C_LEFT);
                }
            } else {
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) = ITEM_NONE;
                C_SLOT_EQUIP(0, EQUIP_SLOT_C_LEFT) = SLOT_NONE;
            }
        } else if (pauseCtx->equipTargetSlot == C_SLOT_EQUIP(0, EQUIP_SLOT_C_DOWN)) {
            if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) & 0xFF) != ITEM_NONE) {
                if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                    (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) & 0xFF) == ITEM_BOW) ||
                     (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) & 0xFF) >= ITEM_BOW_FIRE) &&
                      ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                    pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                    pauseCtx->equipTargetSlot = SLOT_BOW;
                } else {
                    BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN);
                    C_SLOT_EQUIP(0, EQUIP_SLOT_C_DOWN) = DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_DOWN);
                    Interface_LoadItemIcon(play, EQUIP_SLOT_C_DOWN);
                }
            } else {
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) = ITEM_NONE;
                C_SLOT_EQUIP(0, EQUIP_SLOT_C_DOWN) = SLOT_NONE;
            }
        } else if (pauseCtx->equipTargetSlot == C_SLOT_EQUIP(0, EQUIP_SLOT_C_RIGHT)) {
            if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) & 0xFF) != ITEM_NONE) {
                if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                    (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) & 0xFF) == ITEM_BOW) ||
                     (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) & 0xFF) >= ITEM_BOW_FIRE) &&
                      ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                    pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                    pauseCtx->equipTargetSlot = SLOT_BOW;
                } else {
                    BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN);
                    C_SLOT_EQUIP(0, EQUIP_SLOT_C_RIGHT) = DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_DOWN);
                    Interface_LoadItemIcon(play, EQUIP_SLOT_C_RIGHT);
                }
            } else {
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) = ITEM_NONE;
                C_SLOT_EQUIP(0, EQUIP_SLOT_C_RIGHT) = SLOT_NONE;
            }
        } else if (pauseCtx->equipTargetSlot == DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_RIGHT)) {
            if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) & 0xFF) != ITEM_NONE) {
                if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                    (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) & 0xFF) == ITEM_BOW) ||
                     (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) & 0xFF) >= ITEM_BOW_FIRE) &&
                      ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                    pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                    pauseCtx->equipTargetSlot = SLOT_BOW;
                } else {
                    DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN);
                    DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_RIGHT) = DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_DOWN);
                    Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_RIGHT);
                }
            } else {
                DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) = ITEM_NONE;
                DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_RIGHT) = SLOT_NONE;
            }
        } else if (pauseCtx->equipTargetSlot == DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_LEFT)) {
            if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) & 0xFF) != ITEM_NONE) {
                if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                    (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) & 0xFF) == ITEM_BOW) ||
                     (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) & 0xFF) >= ITEM_BOW_FIRE) &&
                      ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                    pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                    pauseCtx->equipTargetSlot = SLOT_BOW;
                } else {
                    DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN);
                    DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_LEFT) = DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_DOWN);
                    Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_LEFT);
                }
            } else {
                DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) = ITEM_NONE;
                DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_LEFT) = SLOT_NONE;
            }
        } else if (pauseCtx->equipTargetSlot == DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_UP)) {
            if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) & 0xFF) != ITEM_NONE) {
                if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                    (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) & 0xFF) == ITEM_BOW) ||
                     (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) & 0xFF) >= ITEM_BOW_FIRE) &&
                      ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                    pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                    pauseCtx->equipTargetSlot = SLOT_BOW;
                } else {
                    DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN);
                    DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_UP) = DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_DOWN);
                    Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_UP);
                }
            } else {
                DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) = ITEM_NONE;
                DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_UP) = SLOT_NONE;
            }
        }

        // Special case for magic arrows
        if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8)) {
            if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) == ITEM_BOW) ||
                ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) >= ITEM_BOW_FIRE) &&
                 (DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) <= ITEM_BOW_LIGHT))) {
                pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                pauseCtx->equipTargetSlot = SLOT_BOW;
            }
        } else if (pauseCtx->equipTargetItem == ITEM_BOW) {
            if ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) >= ITEM_BOW_FIRE) &&
                (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) <= ITEM_BOW_LIGHT)) {
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN);
                Interface_LoadItemIcon(play, EQUIP_SLOT_C_LEFT);
            } else if ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) >= ITEM_BOW_FIRE) &&
                       (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) <= ITEM_BOW_LIGHT)) {
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN);
                Interface_LoadItemIcon(play, EQUIP_SLOT_C_DOWN);
            } else if ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) >= ITEM_BOW_FIRE) &&
                       (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) <= ITEM_BOW_LIGHT)) {
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN);
                Interface_LoadItemIcon(play, EQUIP_SLOT_C_RIGHT);
            } else if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) >= ITEM_BOW_FIRE) &&
                       (DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) <= ITEM_BOW_LIGHT)) {
                DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN);
                Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_RIGHT);
            } else if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) >= ITEM_BOW_FIRE) &&
                       (DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) <= ITEM_BOW_LIGHT)) {
                DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN);
                Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_LEFT);
            } else if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) >= ITEM_BOW_FIRE) &&
                       (DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) <= ITEM_BOW_LIGHT)) {
                DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN);
                Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_UP);
            }
        }

        // Equip item on DDown
        DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) = pauseCtx->equipTargetItem;
        DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_DOWN) = pauseCtx->equipTargetSlot;
        Interface_Dpad_LoadItemIconImpl(play, EQUIP_SLOT_D_DOWN);
    } else if (pauseCtx->equipTargetCBtn == PAUSE_EQUIP_D_UP) {
        // Swap if item is already equipped on other Item Buttons.
        if (pauseCtx->equipTargetSlot == C_SLOT_EQUIP(0, EQUIP_SLOT_C_LEFT)) {
            if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) & 0xFF) != ITEM_NONE) {
                if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                    (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) & 0xFF) == ITEM_BOW) ||
                     (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) & 0xFF) >= ITEM_BOW_FIRE) &&
                      ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                    pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                    pauseCtx->equipTargetSlot = SLOT_BOW;
                } else {
                    BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP);
                    C_SLOT_EQUIP(0, EQUIP_SLOT_C_LEFT) = DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_UP);
                    Interface_LoadItemIcon(play, EQUIP_SLOT_C_LEFT);
                }
            } else {
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) = ITEM_NONE;
                C_SLOT_EQUIP(0, EQUIP_SLOT_C_LEFT) = SLOT_NONE;
            }
        } else if (pauseCtx->equipTargetSlot == C_SLOT_EQUIP(0, EQUIP_SLOT_C_DOWN)) {
            if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) & 0xFF) != ITEM_NONE) {
                if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                    (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) & 0xFF) == ITEM_BOW) ||
                     (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) & 0xFF) >= ITEM_BOW_FIRE) &&
                      ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                    pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                    pauseCtx->equipTargetSlot = SLOT_BOW;
                } else {
                    BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP);
                    C_SLOT_EQUIP(0, EQUIP_SLOT_C_DOWN) = DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_UP);
                    Interface_LoadItemIcon(play, EQUIP_SLOT_C_DOWN);
                }
            } else {
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) = ITEM_NONE;
                C_SLOT_EQUIP(0, EQUIP_SLOT_C_DOWN) = SLOT_NONE;
            }
        } else if (pauseCtx->equipTargetSlot == C_SLOT_EQUIP(0, EQUIP_SLOT_C_RIGHT)) {
            if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) & 0xFF) != ITEM_NONE) {
                if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                    (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) & 0xFF) == ITEM_BOW) ||
                     (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) & 0xFF) >= ITEM_BOW_FIRE) &&
                      ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                    pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                    pauseCtx->equipTargetSlot = SLOT_BOW;
                } else {
                    BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP);
                    C_SLOT_EQUIP(0, EQUIP_SLOT_C_RIGHT) = DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_UP);
                    Interface_LoadItemIcon(play, EQUIP_SLOT_C_RIGHT);
                }
            } else {
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) = ITEM_NONE;
                C_SLOT_EQUIP(0, EQUIP_SLOT_C_RIGHT) = SLOT_NONE;
            }
        } else if (pauseCtx->equipTargetSlot == DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_RIGHT)) {
            if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) & 0xFF) != ITEM_NONE) {
                if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                    (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) & 0xFF) == ITEM_BOW) ||
                     (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) & 0xFF) >= ITEM_BOW_FIRE) &&
                      ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                    pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                    pauseCtx->equipTargetSlot = SLOT_BOW;
                } else {
                    DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP);
                    DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_RIGHT) = DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_UP);
                    Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_RIGHT);
                }
            } else {
                DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) = ITEM_NONE;
                DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_RIGHT) = SLOT_NONE;
            }
        } else if (pauseCtx->equipTargetSlot == DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_LEFT)) {
            if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) & 0xFF) != ITEM_NONE) {
                if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                    (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) & 0xFF) == ITEM_BOW) ||
                     (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) & 0xFF) >= ITEM_BOW_FIRE) &&
                      ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                    pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                    pauseCtx->equipTargetSlot = SLOT_BOW;
                } else {
                    DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP);
                    DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_LEFT) = DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_UP);
                    Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_LEFT);
                }
            } else {
                DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) = ITEM_NONE;
                DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_LEFT) = SLOT_NONE;
            }
        } else if (pauseCtx->equipTargetSlot == DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_DOWN)) {
            if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) & 0xFF) != ITEM_NONE) {
                if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                    (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) & 0xFF) == ITEM_BOW) ||
                     (((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) & 0xFF) >= ITEM_BOW_FIRE) &&
                      ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                    pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                    pauseCtx->equipTargetSlot = SLOT_BOW;
                } else {
                    DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP);
                    DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_DOWN) = DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_UP);
                    Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_DOWN);
                }
            } else {
                DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) = ITEM_NONE;
                DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_DOWN) = SLOT_NONE;
            }
        }

        // Special case for magic arrows
        if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8)) {
            if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) == ITEM_BOW) ||
                ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) >= ITEM_BOW_FIRE) &&
                 (DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) <= ITEM_BOW_LIGHT))) {
                pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                pauseCtx->equipTargetSlot = SLOT_BOW;
            }
        } else if (pauseCtx->equipTargetItem == ITEM_BOW) {
            if ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) >= ITEM_BOW_FIRE) &&
                (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) <= ITEM_BOW_LIGHT)) {
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP);
                Interface_LoadItemIcon(play, EQUIP_SLOT_C_LEFT);
            } else if ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) >= ITEM_BOW_FIRE) &&
                       (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) <= ITEM_BOW_LIGHT)) {
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP);
                Interface_LoadItemIcon(play, EQUIP_SLOT_C_DOWN);
            } else if ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) >= ITEM_BOW_FIRE) &&
                       (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) <= ITEM_BOW_LIGHT)) {
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP);
                Interface_LoadItemIcon(play, EQUIP_SLOT_C_RIGHT);
            } else if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) >= ITEM_BOW_FIRE) &&
                       (DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) <= ITEM_BOW_LIGHT)) {
                DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP);
                Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_RIGHT);
            } else if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) >= ITEM_BOW_FIRE) &&
                       (DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) <= ITEM_BOW_LIGHT)) {
                DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP);
                Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_LEFT);
            } else if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) >= ITEM_BOW_FIRE) &&
                       (DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) <= ITEM_BOW_LIGHT)) {
                DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) = DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP);
                Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_DOWN);
            }
        }

        // Equip item on DUp
        DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) = pauseCtx->equipTargetItem;
        DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_UP) = pauseCtx->equipTargetSlot;
        Interface_Dpad_LoadItemIconImpl(play, EQUIP_SLOT_D_UP);
    }
}
// #endregion

void KaleidoScope_UpdateItemEquip(PlayState* play) {
    static s16 sEquipMagicArrowBowSlotHoldTimer = 0;
    PauseContext* pauseCtx = &play->pauseCtx;
    Vtx* bowItemVtx;
    u16 offsetX;
    u16 offsetY;

    // Grow glowing orb when equipping magic arrows
    if (sEquipState == EQUIP_STATE_MAGIC_ARROW_GROW_ORB) {
        pauseCtx->equipAnimAlpha += 14;
        if (pauseCtx->equipAnimAlpha > 255) {
            pauseCtx->equipAnimAlpha = 254;
            sEquipState++;
        }
        // Hover over magic arrow slot when the next state is reached
        sEquipMagicArrowSlotHoldTimer = 5;
        return;
    }

    if (sEquipState == EQUIP_STATE_MAGIC_ARROW_HOVER_OVER_BOW_SLOT) {
        sEquipMagicArrowBowSlotHoldTimer--;

        if (sEquipMagicArrowBowSlotHoldTimer == 0) {
            pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
            pauseCtx->equipTargetSlot = SLOT_BOW;
            sEquipAnimTimer = 6;
            pauseCtx->equipAnimScale = 320;
            pauseCtx->equipAnimShrinkRate = 40;
            sEquipState++;
            Audio_PlaySfx(NA_SE_SY_SYNTH_MAGIC_ARROW);
        }
        return;
    }

    // #region 2S2H [Cosmetic] Track the C button position vanilla values or HUD editor adjusted values
    s16 cButtonPosX = sCButtonPosX[pauseCtx->equipTargetCBtn];
    s16 cButtonPosY = sCButtonPosY[pauseCtx->equipTargetCBtn];

    HudEditor_SetActiveElement(pauseCtx->equipTargetCBtn < 3 ? HUD_EDITOR_ELEMENT_C_LEFT + pauseCtx->equipTargetCBtn
                                                             : HUD_EDITOR_ELEMENT_D_PAD);

    if (sEquipState == EQUIP_STATE_MOVE_TO_C_BTN && HudEditor_ShouldOverrideDraw()) {
        s16 equipAnimShrinkRate = 40;
        HudEditor_ModifyKaleidoEquipAnimValues(&cButtonPosX, &cButtonPosY, &equipAnimShrinkRate);

        // Override the anim shrink rate at the beginning (value is 320)
        if (pauseCtx->equipAnimScale == 320) {
            pauseCtx->equipAnimShrinkRate = equipAnimShrinkRate;
        }

        if (CVarGetInteger(hudEditorElements[hudEditorActiveElement].modeCvar, HUD_EDITOR_ELEMENT_MODE_VANILLA) ==
            HUD_EDITOR_ELEMENT_MODE_HIDDEN) {
            pauseCtx->equipAnimScale = 0;
            pauseCtx->equipAnimShrinkRate = 0;
        }
    } else if (sEquipState == EQUIP_STATE_MOVE_TO_C_BTN && pauseCtx->equipTargetCBtn >= 3) {
        // Equips to DPad need to shrink fast to be have a final smaller size (16px),
        // So we override the anim shrink rate at the beginning (value is 320)
        if (pauseCtx->equipAnimScale == 320) {
            pauseCtx->equipAnimShrinkRate = 160;
        }
    }

    HudEditor_SetActiveElement(HUD_EDITOR_ELEMENT_NONE);
    // #endregion

    if (sEquipState == EQUIP_STATE_MAGIC_ARROW_MOVE_TO_BOW_SLOT) {
        bowItemVtx = &pauseCtx->itemVtx[SLOT_BOW * 4];
        offsetX = ABS_ALT(pauseCtx->equipAnimX - bowItemVtx->v.ob[0] * 10) / sEquipAnimTimer;
        offsetY = ABS_ALT(pauseCtx->equipAnimY - bowItemVtx->v.ob[1] * 10) / sEquipAnimTimer;
    } else {
        // 2S2H [Cosmetic] Use position vars from above
        offsetX = ABS_ALT(pauseCtx->equipAnimX - cButtonPosX) / sEquipAnimTimer;
        offsetY = ABS_ALT(pauseCtx->equipAnimY - cButtonPosY) / sEquipAnimTimer;
    }

    if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipAnimAlpha < 254)) {
        pauseCtx->equipAnimAlpha += 14;
        if (pauseCtx->equipAnimAlpha > 255) {
            pauseCtx->equipAnimAlpha = 254;
        }
        sEquipMagicArrowSlotHoldTimer = 5;
        return;
    }

    if (sEquipMagicArrowSlotHoldTimer == 0) {
        pauseCtx->equipAnimScale -= pauseCtx->equipAnimShrinkRate / sEquipAnimTimer;
        pauseCtx->equipAnimShrinkRate -= pauseCtx->equipAnimShrinkRate / sEquipAnimTimer;

        // Update coordinates of item icon while being equipped
        if (sEquipState == EQUIP_STATE_MAGIC_ARROW_MOVE_TO_BOW_SLOT) {
            // target is the bow slot
            if (pauseCtx->equipAnimX >= (pauseCtx->itemVtx[SLOT_BOW * 4].v.ob[0] * 10)) {
                pauseCtx->equipAnimX -= offsetX;
            } else {
                pauseCtx->equipAnimX += offsetX;
            }

            if (pauseCtx->equipAnimY >= (pauseCtx->itemVtx[SLOT_BOW * 4].v.ob[1] * 10)) {
                pauseCtx->equipAnimY -= offsetY;
            } else {
                pauseCtx->equipAnimY += offsetY;
            }
        } else {
            // target is the c button
            // 2S2H [Cosmetic] Use position vars from above
            if (pauseCtx->equipAnimX >= cButtonPosX) {
                pauseCtx->equipAnimX -= offsetX;
            } else {
                pauseCtx->equipAnimX += offsetX;
            }

            if (pauseCtx->equipAnimY >= cButtonPosY) {
                pauseCtx->equipAnimY -= offsetY;
            } else {
                pauseCtx->equipAnimY += offsetY;
            }
        }

        sEquipAnimTimer--;
        if (sEquipAnimTimer == 0) {
            if (sEquipState == EQUIP_STATE_MAGIC_ARROW_MOVE_TO_BOW_SLOT) {
                sEquipState++;
                sEquipMagicArrowBowSlotHoldTimer = 4;
                return;
            }

            // Equip item onto c buttons
            if (pauseCtx->equipTargetCBtn == PAUSE_EQUIP_C_LEFT) {
                // Swap if item is already equipped on CDown or CRight.
                if (pauseCtx->equipTargetSlot == C_SLOT_EQUIP(0, EQUIP_SLOT_C_DOWN)) {
                    if ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) & 0xFF) != ITEM_NONE) {
                        if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                            (((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) & 0xFF) == ITEM_BOW) ||
                             (((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) & 0xFF) >= ITEM_BOW_FIRE) &&
                              ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                            pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                            pauseCtx->equipTargetSlot = SLOT_BOW;
                        } else {
                            BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) = BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT);
                            C_SLOT_EQUIP(0, EQUIP_SLOT_C_DOWN) = C_SLOT_EQUIP(0, EQUIP_SLOT_C_LEFT);
                            Interface_LoadItemIcon(play, EQUIP_SLOT_C_DOWN);
                        }
                    } else {
                        BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) = ITEM_NONE;
                        C_SLOT_EQUIP(0, EQUIP_SLOT_C_DOWN) = SLOT_NONE;
                    }
                } else if (pauseCtx->equipTargetSlot == C_SLOT_EQUIP(0, EQUIP_SLOT_C_RIGHT)) {
                    if ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) & 0xFF) != ITEM_NONE) {
                        if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                            (((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) & 0xFF) == ITEM_BOW) ||
                             (((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) & 0xFF) >= ITEM_BOW_FIRE) &&
                              ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                            pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                            pauseCtx->equipTargetSlot = SLOT_BOW;
                        } else {
                            BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) = BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT);
                            C_SLOT_EQUIP(0, EQUIP_SLOT_C_RIGHT) = C_SLOT_EQUIP(0, EQUIP_SLOT_C_LEFT);
                            Interface_LoadItemIcon(play, EQUIP_SLOT_C_RIGHT);
                        }
                    } else {
                        BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) = ITEM_NONE;
                        C_SLOT_EQUIP(0, EQUIP_SLOT_C_RIGHT) = SLOT_NONE;
                    }
                }
                // #region 2S2H [Dpad]
                KaleidoScope_SwapDpadItemToCItem(play, EQUIP_SLOT_C_LEFT);
                // #endregion

                // Special case for magic arrows
                if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8)) {
                    if ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) == ITEM_BOW) ||
                        ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) >= ITEM_BOW_FIRE) &&
                         (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) <= ITEM_BOW_LIGHT))) {
                        pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                        pauseCtx->equipTargetSlot = SLOT_BOW;
                    }
                } else if (pauseCtx->equipTargetItem == ITEM_BOW) {
                    if ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) >= ITEM_BOW_FIRE) &&
                        (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) <= ITEM_BOW_LIGHT)) {
                        BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) = BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT);
                        C_SLOT_EQUIP(0, EQUIP_SLOT_C_DOWN) = C_SLOT_EQUIP(0, EQUIP_SLOT_C_LEFT);
                        Interface_LoadItemIcon(play, EQUIP_SLOT_C_DOWN);
                    } else if ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) >= ITEM_BOW_FIRE) &&
                               (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) <= ITEM_BOW_LIGHT)) {
                        BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) = BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT);
                        C_SLOT_EQUIP(0, EQUIP_SLOT_C_RIGHT) = C_SLOT_EQUIP(0, EQUIP_SLOT_C_LEFT);
                        Interface_LoadItemIcon(play, EQUIP_SLOT_C_RIGHT);
                    }
                    // #region 2S2H [Dpad]
                    // Note Only C-Left has the swap of 'slot equips' here
                    if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) >= ITEM_BOW_FIRE) &&
                        (DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) <= ITEM_BOW_LIGHT)) {
                        DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) = BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT);
                        DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_RIGHT) = C_SLOT_EQUIP(0, EQUIP_SLOT_C_LEFT);
                        Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_RIGHT);
                    } else if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) >= ITEM_BOW_FIRE) &&
                               (DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) <= ITEM_BOW_LIGHT)) {
                        DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) = BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT);
                        DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_LEFT) = C_SLOT_EQUIP(0, EQUIP_SLOT_C_LEFT);
                        Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_LEFT);
                    } else if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) >= ITEM_BOW_FIRE) &&
                               (DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) <= ITEM_BOW_LIGHT)) {
                        DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) = BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT);
                        DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_DOWN) = C_SLOT_EQUIP(0, EQUIP_SLOT_C_LEFT);
                        Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_DOWN);
                    } else if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) >= ITEM_BOW_FIRE) &&
                               (DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) <= ITEM_BOW_LIGHT)) {
                        DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) = BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT);
                        DPAD_SLOT_EQUIP(0, EQUIP_SLOT_D_UP) = C_SLOT_EQUIP(0, EQUIP_SLOT_C_LEFT);
                        Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_UP);
                    }
                    // #endregion
                }

                // Equip item on CLeft
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) = pauseCtx->equipTargetItem;
                C_SLOT_EQUIP(0, EQUIP_SLOT_C_LEFT) = pauseCtx->equipTargetSlot;
                Interface_LoadItemIconImpl(play, EQUIP_SLOT_C_LEFT);
            } else if (pauseCtx->equipTargetCBtn == PAUSE_EQUIP_C_DOWN) {
                // Swap if item is already equipped on CLeft or CRight.
                if (pauseCtx->equipTargetSlot == C_SLOT_EQUIP(0, EQUIP_SLOT_C_LEFT)) {
                    if ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) & 0xFF) != ITEM_NONE) {
                        if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                            (((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) & 0xFF) == ITEM_BOW) ||
                             (((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) & 0xFF) >= ITEM_BOW_FIRE) &&
                              ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                            pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                            pauseCtx->equipTargetSlot = SLOT_BOW;
                        } else {
                            BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) = BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN);
                            C_SLOT_EQUIP(0, EQUIP_SLOT_C_LEFT) = C_SLOT_EQUIP(0, EQUIP_SLOT_C_DOWN);
                            Interface_LoadItemIcon(play, EQUIP_SLOT_C_LEFT);
                        }
                    } else {
                        BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) = ITEM_NONE;
                        C_SLOT_EQUIP(0, EQUIP_SLOT_C_LEFT) = SLOT_NONE;
                    }
                } else if (pauseCtx->equipTargetSlot == C_SLOT_EQUIP(0, EQUIP_SLOT_C_RIGHT)) {
                    if ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) & 0xFF) != ITEM_NONE) {
                        if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                            (((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) & 0xFF) == ITEM_BOW) ||
                             (((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) & 0xFF) >= ITEM_BOW_FIRE) &&
                              ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                            pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                            pauseCtx->equipTargetSlot = SLOT_BOW;
                        } else {
                            BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) = BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN);
                            C_SLOT_EQUIP(0, EQUIP_SLOT_C_RIGHT) = C_SLOT_EQUIP(0, EQUIP_SLOT_C_DOWN);
                            Interface_LoadItemIcon(play, EQUIP_SLOT_C_RIGHT);
                        }
                    } else {
                        BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) = ITEM_NONE;
                        C_SLOT_EQUIP(0, EQUIP_SLOT_C_RIGHT) = SLOT_NONE;
                    }
                }
                // #region 2S2H [Dpad]
                KaleidoScope_SwapDpadItemToCItem(play, EQUIP_SLOT_C_DOWN);
                // #endregion

                // Special case for magic arrows
                if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8)) {
                    if ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) == ITEM_BOW) ||
                        ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) >= ITEM_BOW_FIRE) &&
                         (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) <= ITEM_BOW_LIGHT))) {
                        pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                        pauseCtx->equipTargetSlot = SLOT_BOW;
                    }
                } else if (pauseCtx->equipTargetItem == ITEM_BOW) {
                    if ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) >= ITEM_BOW_FIRE) &&
                        (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) <= ITEM_BOW_LIGHT)) {
                        BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) = BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN);
                        Interface_LoadItemIcon(play, EQUIP_SLOT_C_LEFT);
                    } else if ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) >= ITEM_BOW_FIRE) &&
                               (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) <= ITEM_BOW_LIGHT)) {
                        BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) = BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN);
                        Interface_LoadItemIcon(play, EQUIP_SLOT_C_RIGHT);
                    }
                    // #region 2S2H [Dpad]
                    // Note Only C-Left has the swap of 'slot equips' here
                    if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) >= ITEM_BOW_FIRE) &&
                        (DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) <= ITEM_BOW_LIGHT)) {
                        DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) = BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN);
                        Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_RIGHT);
                    } else if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) >= ITEM_BOW_FIRE) &&
                               (DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) <= ITEM_BOW_LIGHT)) {
                        DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) = BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN);
                        Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_LEFT);
                    } else if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) >= ITEM_BOW_FIRE) &&
                               (DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) <= ITEM_BOW_LIGHT)) {
                        DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) = BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN);
                        Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_DOWN);
                    } else if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) >= ITEM_BOW_FIRE) &&
                               (DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) <= ITEM_BOW_LIGHT)) {
                        DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) = BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN);
                        Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_UP);
                    }
                    // #endregion
                }

                // Equip item on CDown
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) = pauseCtx->equipTargetItem;
                C_SLOT_EQUIP(0, EQUIP_SLOT_C_DOWN) = pauseCtx->equipTargetSlot;
                Interface_LoadItemIconImpl(play, EQUIP_SLOT_C_DOWN);
            } else if (pauseCtx->equipTargetCBtn ==
                       PAUSE_EQUIP_C_RIGHT) { // #Region 2S2H [Dpad] Added condition here to allow for other cases
                // Swap if item is already equipped on CLeft or CDown.
                if (pauseCtx->equipTargetSlot == C_SLOT_EQUIP(0, EQUIP_SLOT_C_LEFT)) {
                    if ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) & 0xFF) != ITEM_NONE) {
                        if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                            (((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) & 0xFF) == ITEM_BOW) ||
                             (((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) & 0xFF) >= ITEM_BOW_FIRE) &&
                              ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                            pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                            pauseCtx->equipTargetSlot = SLOT_BOW;
                        } else {
                            BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) = BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT);
                            C_SLOT_EQUIP(0, EQUIP_SLOT_C_LEFT) = C_SLOT_EQUIP(0, EQUIP_SLOT_C_RIGHT);
                            Interface_LoadItemIcon(play, EQUIP_SLOT_C_LEFT);
                        }
                    } else {
                        BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) = ITEM_NONE;
                        C_SLOT_EQUIP(0, EQUIP_SLOT_C_LEFT) = SLOT_NONE;
                    }
                } else if (pauseCtx->equipTargetSlot == C_SLOT_EQUIP(0, EQUIP_SLOT_C_DOWN)) {
                    if ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) & 0xFF) != ITEM_NONE) {
                        if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8) &&
                            (((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) & 0xFF) == ITEM_BOW) ||
                             (((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) & 0xFF) >= ITEM_BOW_FIRE) &&
                              ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) & 0xFF) <= ITEM_BOW_LIGHT)))) {
                            pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                            pauseCtx->equipTargetSlot = SLOT_BOW;
                        } else {
                            BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) = BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT);
                            C_SLOT_EQUIP(0, EQUIP_SLOT_C_DOWN) = C_SLOT_EQUIP(0, EQUIP_SLOT_C_RIGHT);
                            Interface_LoadItemIcon(play, EQUIP_SLOT_C_DOWN);
                        }
                    } else {
                        BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) = ITEM_NONE;
                        C_SLOT_EQUIP(0, EQUIP_SLOT_C_DOWN) = SLOT_NONE;
                    }
                }
                // #region 2S2H [Dpad]
                KaleidoScope_SwapDpadItemToCItem(play, EQUIP_SLOT_C_RIGHT);
                // #endregion

                // Special case for magic arrows
                if ((pauseCtx->equipTargetItem >= 0xB5) && (pauseCtx->equipTargetItem < 0xB8)) {
                    if ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) == ITEM_BOW) ||
                        ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) >= ITEM_BOW_FIRE) &&
                         (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) <= ITEM_BOW_LIGHT))) {
                        pauseCtx->equipTargetItem -= 0xB5 - ITEM_BOW_FIRE;
                        pauseCtx->equipTargetSlot = SLOT_BOW;
                    }
                } else if (pauseCtx->equipTargetItem == ITEM_BOW) {
                    if ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) >= ITEM_BOW_FIRE) &&
                        (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) <= ITEM_BOW_LIGHT)) {
                        BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_LEFT) = BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT);
                        Interface_LoadItemIcon(play, EQUIP_SLOT_C_LEFT);
                    } else if ((BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) >= ITEM_BOW_FIRE) &&
                               (BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) <= ITEM_BOW_LIGHT)) {
                        BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_DOWN) = BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT);
                        Interface_LoadItemIcon(play, EQUIP_SLOT_C_DOWN);
                    }
                    // #region 2S2H [Dpad]
                    // Note Only C-Left has the swap of 'slot equips' here
                    if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) >= ITEM_BOW_FIRE) &&
                        (DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) <= ITEM_BOW_LIGHT)) {
                        DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_RIGHT) = BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT);
                        Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_RIGHT);
                    } else if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) >= ITEM_BOW_FIRE) &&
                               (DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) <= ITEM_BOW_LIGHT)) {
                        DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_LEFT) = BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT);
                        Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_LEFT);
                    } else if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) >= ITEM_BOW_FIRE) &&
                               (DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) <= ITEM_BOW_LIGHT)) {
                        DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_DOWN) = BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT);
                        Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_DOWN);
                    } else if ((DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) >= ITEM_BOW_FIRE) &&
                               (DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) <= ITEM_BOW_LIGHT)) {
                        DPAD_BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_D_UP) = BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT);
                        Interface_Dpad_LoadItemIcon(play, EQUIP_SLOT_D_UP);
                    }
                    // #endregion
                }

                // Equip item on CRight
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_C_RIGHT) = pauseCtx->equipTargetItem;
                C_SLOT_EQUIP(0, EQUIP_SLOT_C_RIGHT) = pauseCtx->equipTargetSlot;
                Interface_LoadItemIconImpl(play, EQUIP_SLOT_C_RIGHT);
            }
            // #region 2S2H [Dpad]
            else if (CVarGetInteger("gEnhancements.Dpad.DpadEquips", 0)) {
                KaleidoScope_UpdateDpadItemEquip(play);
            }
            // #endregion

            // Reset params
            pauseCtx->mainState = PAUSE_MAIN_STATE_IDLE;
            sEquipAnimTimer = 10;
            pauseCtx->equipAnimScale = 320;
            pauseCtx->equipAnimShrinkRate = 40;
        }
    } else {
        sEquipMagicArrowSlotHoldTimer--;
        if (sEquipMagicArrowSlotHoldTimer == 0) {
            pauseCtx->equipAnimAlpha = 255;
        }
    }
}
