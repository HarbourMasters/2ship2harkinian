/*
 * File: z_kaleido_item.c
 * Overlay: ovl_kaleido_scope
 * Description: Pause Menu - Item Page
 */

#include "z_kaleido_scope.h"
#include "interface/parameter_static/parameter_static.h"

#include "2s2h/BenGui/HudEditor.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/CustomMessage/PauseItemDescriptions.h" // NEI: C-Up descriptions for custom items
#include <libultraship/bridge/consolevariablebridge.h>
#include "mods/extended_inventory.h" // NEI: page-aware kaleido (ExtInv_GetInventorySlot/GetSlotItem/GetItemIcon/SwitchPage/Update)
#include "mods/items/custom_bottles.h"                      // NEI: bottle randomizer wheels A/B (Skijer's NEI)
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
    u16 slotItem = ExtInv_GetSlotItem(realSlot);
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
//
// `leftTex`/`rightTex` override the icon that would be derived from the item id, and `leftTint`/
// `rightTint` (RGB triples) override the white prim colour. Both are optional (NULL = derive/white)
// and exist for wheels whose entries are STATES of one item rather than different items, so the id
// alone cannot tell the two sides apart: the Lantern's fire types differ only by flame colour, and
// the Gale Boomerang is the same item id with a different texture. Ported from SoH's
// KaleidoCycle_DrawRocStyle, which carries the same two parameters for the same reason.
static void KaleidoScope_DrawItemCycleExtrasTinted(PlayState* play, u8 slot, u8 canCycle, u8 leftItem, u8 rightItem,
                                                   u8 forceShow, void* leftTex, void* rightTex, const u8* leftTint,
                                                   const u8* rightTint) {
    PauseContext* pauseCtx = &play->pauseCtx;
    u8 isCycling = (gCurrentItemCyclingSlot == slot);
    u16 slotItem = ExtInv_GetSlotItem(ExtInv_GetInventorySlot(slot));
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

    // Candidates belong to the cell you are POINTING AT, not to every cell that happens to own a
    // wheel. Without this gate each wheel painted its two minis permanently: resting 15px below the
    // cell centre, they landed on the top edge of the cell underneath — which on the OoT page-0
    // layout is exactly "little icons under Din's Fire / Farore's Wind / Nayru's Love" — and the
    // bottom row (bottles, trade, OoT masks) pushed them clean outside the page frame. The A-button
    // hint further down was already hover-gated; the candidates were not. Keeping the timer in the
    // condition lets a cell you just left finish closing instead of popping. Skijer's NEI
    u8 hovered = (pauseCtx->cursorSlot[PAUSE_ITEM] == slot) && (pauseCtx->cursorSpecialPos == 0);

    if (canCycle && (slotItem != ITEM_NONE) && (showLeftItem || showRightItem) &&
        (hovered || isCycling || (sSlotCycleActiveAnimTimer[slot] > 0))) {
        // Exact OoT/SoH transform baked per-vertex (a matrix fights MM's live page transform):
        // candidates REST small (scale 0.325) tucked 15px below the cell, and grow to full 32px
        // flanking the item as the wheel opens (anim timer 0..5).
        f32 animScale = (5 - sSlotCycleActiveAnimTimer[slot]) / 5.0f; // 1 rest .. 0 open
        f32 finalScale = 1.0f - (0.675f * animScale);                 // 0.325 rest .. 1.0 open
        s16 qSize = (s16)(32.0f * finalScale);                        // candidate quad px
        s16 flank = (s16)(32.0f * finalScale);                        // candidate center offset from cell
        s16 yShift = (s16)(-15.0f * animScale);                       // 15 below at rest .. 0 open
        s16 yTop;
        s32 vi;

        // True cell center from opposite-corner vtx (layout-agnostic).
        cx = (cellVtx[0].v.ob[0] + cellVtx[3].v.ob[0]) / 2;
        cy = (cellVtx[0].v.ob[1] + cellVtx[3].v.ob[1]) / 2;
        yTop = cy + yShift + qSize / 2;

        // The two sides can carry different prim colours now, so the colour is set per side rather
        // than once up front. White when no tint was given, which is the old behaviour exactly.
        static const u8 sNoTint[3] = { 255, 255, 255 };
        const u8* lt = (leftTint != NULL) ? leftTint : sNoTint;
        const u8* rt = (rightTint != NULL) ? rightTint : sNoTint;
        u8 candAlpha = isCycling ? pauseCtx->alpha : (u8)(pauseCtx->alpha * 3 / 4);

        gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);

        // Each side resolves its icon FIRST and skips the quad when it comes back NULL — the trade
        // and OoT-mask previews are OTR paths gated on oot.o2r, and painting a NULL "texture" draws
        // a garbage quad beside the cell (the smashed-looking wheel previews). Skijer's NEI
        if (showLeftItem) {
            void* lTex = (leftTex != NULL) ? leftTex : ExtInv_GetItemIcon(leftItem);
            u8 lsz = ExtInv_GetItemIconSize(leftItem);
            if (lTex != NULL) {
                Vtx* v = GRAPH_ALLOC(play->state.gfxCtx, 4 * sizeof(Vtx));
                s16 x0 = (cx - flank) - qSize / 2;
                for (vi = 0; vi < 4; vi++) {
                    v[vi] = cellVtx[0];
                }
                v[0].v.ob[0] = x0;
                v[0].v.ob[1] = yTop;
                v[0].v.tc[0] = 0;
                v[0].v.tc[1] = 0;
                v[1].v.ob[0] = x0 + qSize;
                v[1].v.ob[1] = yTop;
                v[1].v.tc[0] = lsz << 5;
                v[1].v.tc[1] = 0;
                v[2].v.ob[0] = x0;
                v[2].v.ob[1] = yTop - qSize;
                v[2].v.tc[0] = 0;
                v[2].v.tc[1] = lsz << 5;
                v[3].v.ob[0] = x0 + qSize;
                v[3].v.ob[1] = yTop - qSize;
                v[3].v.tc[0] = lsz << 5;
                v[3].v.tc[1] = lsz << 5;
                gSPVertex(POLY_OPA_DISP++, v, 4, 0);
                gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, lt[0], lt[1], lt[2], candAlpha);
                KaleidoScope_DrawTexQuadRGBA32(play->state.gfxCtx, lTex, lsz, lsz, 0);
            }
        }
        if (showRightItem) {
            void* rTex = (rightTex != NULL) ? rightTex : ExtInv_GetItemIcon(rightItem);
            u8 rsz = ExtInv_GetItemIconSize(rightItem);
            if (rTex != NULL) {
                Vtx* v = GRAPH_ALLOC(play->state.gfxCtx, 4 * sizeof(Vtx));
                s16 x0 = (cx + flank) - qSize / 2;
                for (vi = 0; vi < 4; vi++) {
                    v[vi] = cellVtx[0];
                }
                v[0].v.ob[0] = x0;
                v[0].v.ob[1] = yTop;
                v[0].v.tc[0] = 0;
                v[0].v.tc[1] = 0;
                v[1].v.ob[0] = x0 + qSize;
                v[1].v.ob[1] = yTop;
                v[1].v.tc[0] = rsz << 5;
                v[1].v.tc[1] = 0;
                v[2].v.ob[0] = x0;
                v[2].v.ob[1] = yTop - qSize;
                v[2].v.tc[0] = 0;
                v[2].v.tc[1] = rsz << 5;
                v[3].v.ob[0] = x0 + qSize;
                v[3].v.ob[1] = yTop - qSize;
                v[3].v.tc[0] = rsz << 5;
                v[3].v.tc[1] = rsz << 5;
                gSPVertex(POLY_OPA_DISP++, v, 4, 0);
                gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, rt[0], rt[1], rt[2], candAlpha);
                KaleidoScope_DrawTexQuadRGBA32(play->state.gfxCtx, rTex, rsz, rsz, 0);
            }
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
            av[0].v.ob[0] = ax - 7;
            av[0].v.ob[1] = ay + 5;
            av[0].v.tc[0] = 0;
            av[0].v.tc[1] = 0;
            av[1].v.ob[0] = ax + 7;
            av[1].v.ob[1] = ay + 5;
            av[1].v.tc[0] = 24 << 5;
            av[1].v.tc[1] = 0;
            av[2].v.ob[0] = ax - 7;
            av[2].v.ob[1] = ay - 5;
            av[2].v.tc[0] = 0;
            av[2].v.tc[1] = 16 << 5;
            av[3].v.ob[0] = ax + 7;
            av[3].v.ob[1] = ay - 5;
            av[3].v.tc[0] = 24 << 5;
            av[3].v.tc[1] = 16 << 5;
            gSPVertex(POLY_OPA_DISP++, av, 4, 0);
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 0, 100, 255, pauseCtx->alpha);
            gDPLoadTextureBlock(POLY_OPA_DISP++, gABtnSymbolTex, G_IM_FMT_IA, G_IM_SIZ_8b, 24, 16, 0,
                                G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP, 4, 4, G_TX_NOLOD, G_TX_NOLOD);
            gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);
        }
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

// Plain variant: both candidates are real items, so their icons come from the ids and no tint is
// wanted. Every wheel except the Lantern and the Gale Boomerang uses this.
static void KaleidoScope_DrawItemCycleExtrasImpl(PlayState* play, u8 slot, u8 canCycle, u8 leftItem, u8 rightItem,
                                                 u8 forceShow) {
    KaleidoScope_DrawItemCycleExtrasTinted(play, slot, canCycle, leftItem, rightItem, forceShow, NULL, NULL, NULL,
                                           NULL);
}

// Public entry (value-based show gates) — trade/keg/bow/etc. cyclers use this.
void KaleidoScope_DrawItemCycleExtras(PlayState* play, u8 slot, u8 canCycle, u8 leftItem, u8 rightItem) {
    KaleidoScope_DrawItemCycleExtrasImpl(play, slot, canCycle, leftItem, rightItem, false);
}

// --- OoT page-0 wheel helpers (Skijer's NEI) ---
#include "mods/nei_save.h"
extern u8 gItemSlots[]; // vanilla item -> home slot (52 entries)
// trade_items.c — unified trade wheel. NOTE: OwnedCount/OwnedAt return s32 there; this file used to
// declare OwnedCount as u8, which is a return-type mismatch (UB that happened to work because the
// count is small). Declared faithfully now. Skijer 2026-07-29
extern s32 TradeAdult_OwnedCount(void);
extern s32 TradeAdult_OwnedAt(s32 ordinal);
extern u8 TradeAdult_ItemId(s32 index);
extern void TradeAdult_FoldCurrent(u8 item);
extern u8 TradeAdult_PrevItem(u8 cur);
extern u8 TradeAdult_NextItem(u8 cur);
extern s32 TradeAdult_CursorIndex(void);
extern void TradeAdult_CursorStep(s32 dir);
extern u8 TradeAdult_CellItem(void);
extern u8 TradeAdult_NeighborCellItem(s32 dir);
extern s32 TradeAdult_NeighborIndex(s32 dir);

// (sBowWheelItems / sSlingWheelItems and their Neighbor helpers are gone — Skijer's NEI. They were
// lists of ITEM ids that the wheel SWAPPED INTO THE SLOT, which is what forced twelve inventory ids
// to exist for a three-bit value. They also had no ownership gate at all: the cursor walked the full
// seven entries whether or not you owned the medallion. Both problems are fixed by the flag wheel
// further down, which builds its entry list from Sw97_ElementOwned.)

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
            u16 oldItem = ExtInv_GetSlotItem(kaleidoSlot);
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

// ── Gust Jar element wheel (page 1, cell 13) — Skijer's NEI ─────────────────────────────────
// Port of SoH's GustJar_HandleElementCycle / GustJar_DrawElementCycle (soh z_kaleido_item.c:1057).
// Unlike every page-0 wheel this one does NOT swap the slot item: the cell always shows the gust
// jar. What it steps is the item's primed ELEMENT (gCustomItemState.gustJarElement), which
// item_gustjar.c turns into the blow's damage type. Available elements = bare wind + one per OWNED
// OoT medallion (Nei_Save()->ootQuestItems), resolved on the item side by GustJar_Element*().
#define GUSTJAR_KALEIDO_CELL 13 // SLOT_GUST_JAR (37) - 24 = page-1 visual cell

// item_gustjar.h can't be included here (it carries static tables); declare the entry points.
extern u8 GustJar_ElementCount(void);
extern u8 GustJar_GetElement(void);
extern void GustJar_SetElement(u8 element);
extern u8 GustJar_ElementNeighbor(u8 element, s32 dir);
extern u16 GustJar_ElementIcon(u8 element);

// ── Reusable press-A "wheel" selector (ported from SoH's KaleidoWheel_Run) ──
// MM already used exactly this input pattern, but written inline inside the Gust
// Jar handler. Lifting it out means the cane — and any later selector — gets the
// same behaviour without a second copy: A toggles the wheel open/closed on its
// cell, and while it is open a stick tilt calls onCycle(dir).
//
//   cell      : the PAGE-RELATIVE visual cell (slot - 24), which is what
//               pauseCtx->cursorSlot and gCurrentItemCyclingSlot hold here.
//   canToggle : ownership gate; a wheel with nothing to pick must not open.
//   onCycle   : applies the change for dir (+1 next / -1 prev).
// Skijer's NEI
typedef void (*KaleidoWheelCycleFunc)(PlayState* play, s32 dir);

static void KaleidoWheel_Run(PlayState* play, s32 cell, u8 canToggle, KaleidoWheelCycleFunc onCycle) {
    Input* input = CONTROLLER1(&play->state);
    PauseContext* pauseCtx = &play->pauseCtx;

    if (!canToggle) {
        return;
    }

    if ((pauseCtx->cursorSlot[PAUSE_ITEM] == cell) && CHECK_BTN_ALL(input->press.button, BTN_A)) {
        Audio_PlaySfx(NA_SE_SY_DECIDE);
        gCurrentItemCyclingSlot = (gCurrentItemCyclingSlot == cell) ? -1 : cell;
    }

    if (gCurrentItemCyclingSlot == cell) {
        s8 dir = 0;

        pauseCtx->cursorColorSet = 8;
        if (pauseCtx->stickAdjX > 30) {
            dir = 1;
        } else if (pauseCtx->stickAdjX < -30) {
            dir = -1;
        }
        if (dir != 0) {
            Audio_PlaySfx(NA_SE_SY_CURSOR);
            onCycle(play, dir);
        }
        // Close as soon as the cursor leaves the cell.
        gCurrentItemCyclingSlot = (pauseCtx->cursorSlot[PAUSE_ITEM] == cell) ? cell : -1;
    }
}

// ── Dual Cane: which cane the shared cell shows ─────────────────────────────
// Cane of Somaria and Cane of Pacci are ONE inventory cell (the player is nearly
// out of item slots), so which one is carried is a context variable rather than a
// second slot: NeiSaveData.caneType, flipped from here.
//
// The toggle only exists once BOTH chains are owned. The two progressions are
// independent and neither ever grants the other, so a player who has only found
// Somaria items has nothing to switch to.
#define CANE_KALEIDO_CELL 21 // SLOT_CANE_OF_SOMARIA (45) - 24 = page-1 visual cell

u8 Cane_GetType(void);
u8 Nei_CaneTypeOwned(u8 type);
void Nei_CaneSetType(u8 type);
u8 Nei_CaneTypeCount(void);
u8 Nei_CaneNextType(s8 dir);

// FOUR entries can live on this cell — Cane of Somaria, Trirod, Cane of Pacci and
// Ultrahand — because finishing a chain ADDS its end-item to the wheel instead of
// replacing the cane that led there. The wheel is worth opening from two onward.
static u8 Cane_WheelHasChoice(void) {
    return Nei_CaneTypeCount() > 1;
}

static void Cane_KaleidoCycle(PlayState* play, s32 dir) {
    // Walk to the next OWNED entry in that direction; locked ones are skipped.
    Nei_CaneSetType(Nei_CaneNextType((s8)((dir >= 0) ? 1 : -1)));
}

static void Cane_KaleidoHandle(PlayState* play) {
    KaleidoWheel_Run(play, CANE_KALEIDO_CELL, Cane_WheelHasChoice(), Cane_KaleidoCycle);
}

static void Cane_KaleidoDraw(PlayState* play) {
    if (!Cane_WheelHasChoice()) {
        return;
    }
    // Both neighbours are the same cell item, so forceShow is required: without it
    // the shared visual suppresses a side whose item equals the cell's own value.
    KaleidoScope_DrawItemCycleExtrasImpl(play, CANE_KALEIDO_CELL, true, ITEM_CANE_OF_SOMARIA, ITEM_CANE_OF_SOMARIA,
                                         true);
}

// ── Shovel <-> Dominion Rod wheel (page 2, shared cell 46) — 2026-08-06 re-layout ───────────────
// The user's page-2 layout puts the Dominion Rod ON the shovel cell as a wheel entry, freeing cell
// 47 for the Rod of Seasons. Ownership of each is its own NeiSaveData flag (a shared cell's value
// cannot say "both owned" — Power Keg idiom); the wheel only opens with both. The generic value-swap
// helper fits because the two entries are DISTINCT item ids, like bombs<->keg.
#define SHOVEL_KALEIDO_CELL (SLOT_SHOVEL - 24)

// Fold a pre-re-layout save into the new cell assignment. Runs once per kaleido frame from the
// page-1 handle — cheap, idempotent, and the only writer besides the gives. Saves may break per the
// user's call, but these four are one-line recoveries, not migrations:
//   47 held the Dominion Rod  -> flag + clear (47 is the Rod of Seasons now)
//   46 held either tool       -> backfill its flag (flags did not exist before)
//   44 held the Pokeball      -> flag + clear (44 is the Shadow Crystal now)
//   41 held Hylia's Grace     -> clear (the item is retired outright)
static void Page2Relayout_Heal(void) {
    NeiSaveData* nei = Nei_Save();

    if (ExtInv_GetSlotItem(SLOT_ROD_OF_SEASONS) == ITEM_DOMINION_ROD) {
        nei->dominionOwned = 1;
        ExtInv_SetSlotItem(SLOT_ROD_OF_SEASONS, ITEM_NONE);
        if (ExtInv_GetSlotItem(SLOT_SHOVEL) == ITEM_NONE) {
            ExtInv_SetSlotItem(SLOT_SHOVEL, ITEM_DOMINION_ROD);
        }
    }
    if (ExtInv_GetSlotItem(SLOT_SHOVEL) == ITEM_SHOVEL) {
        nei->shovelOwned = 1;
    } else if (ExtInv_GetSlotItem(SLOT_SHOVEL) == ITEM_DOMINION_ROD) {
        nei->dominionOwned = 1;
    }
    if (ExtInv_GetSlotItem(SLOT_SHADOW_CRYSTAL) == ITEM_POKEBALL) {
        nei->pokeballOwned = 1;
        ExtInv_SetSlotItem(SLOT_SHADOW_CRYSTAL, ITEM_NONE);
    }
    if (ExtInv_GetSlotItem(SLOT_PHANTOM_HOURGLASS) == ITEM_HYLIAS_GRACE) {
        ExtInv_SetSlotItem(SLOT_PHANTOM_HOURGLASS, ITEM_NONE);
    }
}

static void Shovel_KaleidoHandle(PlayState* play) {
    NeiSaveData* nei = Nei_Save();
    u16 cur = ExtInv_GetSlotItem(SLOT_SHOVEL);
    u8 other = (cur == ITEM_SHOVEL) ? ITEM_DOMINION_ROD : ITEM_SHOVEL;

    KaleidoScope_HandleItemCycleExtras(play, SHOVEL_KALEIDO_CELL, nei->shovelOwned && nei->dominionOwned, other, other,
                                       true);
}

static void Shovel_KaleidoDraw(PlayState* play) {
    NeiSaveData* nei = Nei_Save();
    u16 cur = ExtInv_GetSlotItem(SLOT_SHOVEL);
    u8 other = (cur == ITEM_SHOVEL) ? ITEM_DOMINION_ROD : ITEM_SHOVEL;

    KaleidoScope_DrawItemCycleExtras(play, SHOVEL_KALEIDO_CELL, nei->shovelOwned && nei->dominionOwned, other, other);
}

// ── Lantern fire-type wheel (ported from SoH) ───────────────────────────────
// Every fire type ever captured, plus "extinguish", on the lantern's own cell. The entries are
// STATES of one item, not different items, so the two candidate icons are the same lantern texture
// separated only by the flame colour — which is why the shared drawer grew its tint parameters.
#define LANTERN_KALEIDO_CELL (SLOT_LANTERN - 24) // page-1 visual cell
#define LANTERN_SELECTOR_MAX 5

extern u8 Lantern_GetFireType(void);
extern u8 Lantern_GetCapturedTypes(void);
extern void Lantern_SetFireType(u8 type);

// Tint per fire type: 0 NONE dim grey (extinguished), 1 REGULAR orange, 2 BLUE cyan, 3 POE magenta,
// 4 GREEN green. Same table as SoH's sLanternTypeTint so both games read identically.
static const u8 sLanternTypeTint[LANTERN_SELECTOR_MAX][3] = {
    { 110, 110, 110 }, { 255, 140, 40 }, { 60, 180, 255 }, { 220, 80, 220 }, { 80, 230, 100 },
};

static u8 Lantern_BuildEntries(u8 entries[LANTERN_SELECTOR_MAX]) {
    u8 captured = Lantern_GetCapturedTypes();
    u8 count = 0;

    for (u8 t = 0; t < LANTERN_SELECTOR_MAX; t++) {
        if (captured & (1 << t)) {
            entries[count++] = t;
        }
    }
    return count;
}

static u8 Lantern_KaleidoOwned(void) {
    return ExtInv_GetSlotItem(SLOT_LANTERN) != ITEM_NONE;
}

// Neighbour fire type in the captured list, wrapping. Returns the current one if it somehow is not
// in the list (a save written before a type was tracked), which keeps the wheel inert instead of
// jumping to an arbitrary flame.
static u8 Lantern_NeighborType(s32 dir) {
    u8 entries[LANTERN_SELECTOR_MAX];
    u8 count = Lantern_BuildEntries(entries);
    u8 cur = Lantern_GetFireType();

    for (u8 i = 0; i < count; i++) {
        if (entries[i] == cur) {
            return entries[(dir > 0) ? ((i + 1) % count) : ((i + count - 1) % count)];
        }
    }
    return cur;
}

static void Lantern_KaleidoCycle(PlayState* play, s32 dir) {
    Lantern_SetFireType(Lantern_NeighborType(dir));
}

static void Lantern_KaleidoHandle(PlayState* play) {
    u8 entries[LANTERN_SELECTOR_MAX];

    KaleidoWheel_Run(play, LANTERN_KALEIDO_CELL, Lantern_KaleidoOwned() && (Lantern_BuildEntries(entries) > 1),
                     Lantern_KaleidoCycle);
}

static void Lantern_KaleidoDraw(PlayState* play) {
    u8 entries[LANTERN_SELECTOR_MAX];

    if (!Lantern_KaleidoOwned() || (Lantern_BuildEntries(entries) <= 1)) {
        return;
    }
    // Both sides are ITEM_LANTERN, so forceShow is required — without it the shared drawer
    // suppresses a candidate whose item equals the cell's own value.
    KaleidoScope_DrawItemCycleExtrasTinted(play, LANTERN_KALEIDO_CELL, true, ITEM_LANTERN, ITEM_LANTERN, true, NULL,
                                           NULL, sLanternTypeTint[Lantern_NeighborType(-1)],
                                           sLanternTypeTint[Lantern_NeighborType(1)]);
}

// ── Gale Boomerang wheel (ported from SoH) ──────────────────────────────────
// Two-position toggle on the boomerang cell: vanilla <-> Gale. Only appears once the Twilight
// upgrade is owned, since otherwise there is nothing to switch to.
#define GALE_KALEIDO_CELL 12 // VSLOT_OOT_BOOMERANG's page-0 visual cell (row 3, col 1)

extern u8 TwilightUpgrade_HasGaleBoomerang(void);
extern u8 TwilightUpgrade_IsGaleBoomerangActive(void);
extern void TwilightUpgrade_SetGaleBoomerang(u8 on);

static void Gale_KaleidoCycle(PlayState* play, s32 dir) {
    // Two entries, so either direction is the same flip.
    TwilightUpgrade_SetGaleBoomerang(TwilightUpgrade_IsGaleBoomerangActive() ? 0 : 1);
}

static void Gale_KaleidoHandle(PlayState* play) {
    KaleidoWheel_Run(play, GALE_KALEIDO_CELL, TwilightUpgrade_HasGaleBoomerang(), Gale_KaleidoCycle);
}

static void Gale_KaleidoDraw(PlayState* play) {
    if (!TwilightUpgrade_HasGaleBoomerang()) {
        return;
    }
    // Explicit textures rather than the item id: ExtInv_GetItemIcon(ITEM_BOOMERANG) already swaps to
    // the gale art when the toggle is on, so deriving both sides from the id would show the SAME
    // icon twice and the wheel would say nothing. Left is always vanilla, right always gale.
    KaleidoScope_DrawItemCycleExtrasTinted(play, GALE_KALEIDO_CELL, true, ITEM_BOOMERANG, ITEM_BOOMERANG, true,
                                           (void*)"__OTR__textures/icon_item_static/gItemIconBoomerangTex",
                                           (void*)"__OTR__textures/icon_item_custom/gItemIconGaleBoomerangTex", NULL,
                                           NULL);
}

static u8 GustJar_KaleidoOwned(void) {
    return ExtInv_GetSlotItem(SLOT_GUST_JAR) != ITEM_NONE;
}

static void GustJar_KaleidoHandle(PlayState* play) {
    Input* input = CONTROLLER1(&play->state);
    PauseContext* pauseCtx = &play->pauseCtx;

    if (!GustJar_KaleidoOwned() || (GustJar_ElementCount() < 2)) {
        return; // no gust jar, or only bare wind — nothing to pick between
    }

    if ((pauseCtx->cursorSlot[PAUSE_ITEM] == GUSTJAR_KALEIDO_CELL) && CHECK_BTN_ALL(input->press.button, BTN_A)) {
        Audio_PlaySfx(NA_SE_SY_DECIDE);
        gCurrentItemCyclingSlot = (gCurrentItemCyclingSlot == GUSTJAR_KALEIDO_CELL) ? -1 : GUSTJAR_KALEIDO_CELL;
    }
    if (gCurrentItemCyclingSlot == GUSTJAR_KALEIDO_CELL) {
        s8 dir = 0;

        pauseCtx->cursorColorSet = 8;
        if (pauseCtx->stickAdjX > 30) {
            dir = 1;
        } else if (pauseCtx->stickAdjX < -30) {
            dir = -1;
        }
        if (dir != 0) {
            Audio_PlaySfx(NA_SE_SY_CURSOR);
            GustJar_SetElement(GustJar_ElementNeighbor(GustJar_GetElement(), dir));
        }
        // Close if the cursor left the cell
        gCurrentItemCyclingSlot =
            (pauseCtx->cursorSlot[PAUSE_ITEM] == GUSTJAR_KALEIDO_CELL) ? GUSTJAR_KALEIDO_CELL : -1;
    }
}

static void GustJar_KaleidoDraw(PlayState* play) {
    PauseContext* pauseCtx = &play->pauseCtx;
    u8 elem;
    u8 count;

    if (!GustJar_KaleidoOwned()) {
        return;
    }
    elem = GustJar_GetElement();
    count = GustJar_ElementCount();

    // Prev/next medallion previews + the A hint, reusing the shared wheel visuals. forceShow
    // because the neighbours are medallion icons, never the cell's own item value.
    if (count > 1) {
        KaleidoScope_DrawItemCycleExtrasImpl(play, GUSTJAR_KALEIDO_CELL, true,
                                             (u8)GustJar_ElementIcon(GustJar_ElementNeighbor(elem, -1)),
                                             (u8)GustJar_ElementIcon(GustJar_ElementNeighbor(elem, 1)), true);
    }

    // Primed element as a medallion badge on the cell's top-right corner, so the current element
    // is readable without opening the wheel. Same per-vertex quad scheme as the Ultrashot marker.
    // GUST_ELEMENT_WIND (0) is the bare jar — no badge.
    if (elem != 0) {
        void* tex = ExtInv_GetItemIcon(GustJar_ElementIcon(elem));

        if (tex != NULL) {
            Vtx* cellVtx = &pauseCtx->itemVtx[GUSTJAR_KALEIDO_CELL * 4];
            Vtx* mv = GRAPH_ALLOC(play->state.gfxCtx, 4 * sizeof(Vtx));
            s16 cx = (cellVtx[0].v.ob[0] + cellVtx[3].v.ob[0]) / 2;
            s16 cy = (cellVtx[0].v.ob[1] + cellVtx[3].v.ob[1]) / 2;
            s16 mSize = 14;
            s16 mx0 = cx + 18 - mSize; // right edge 2px past the 32px cell
            s16 myTop = cy + 18;       // top edge 2px past the cell
            s32 mvi;

            OPEN_DISPS(play->state.gfxCtx);

            gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);

            for (mvi = 0; mvi < 4; mvi++) {
                mv[mvi] = cellVtx[0];
            }
            mv[0].v.ob[0] = mx0;
            mv[0].v.ob[1] = myTop;
            mv[0].v.tc[0] = 0;
            mv[0].v.tc[1] = 0;
            mv[1].v.ob[0] = mx0 + mSize;
            mv[1].v.ob[1] = myTop;
            mv[1].v.tc[0] = 24 << 5;
            mv[1].v.tc[1] = 0;
            mv[2].v.ob[0] = mx0;
            mv[2].v.ob[1] = myTop - mSize;
            mv[2].v.tc[0] = 0;
            mv[2].v.tc[1] = 24 << 5;
            mv[3].v.ob[0] = mx0 + mSize;
            mv[3].v.ob[1] = myTop - mSize;
            mv[3].v.tc[0] = 24 << 5;
            mv[3].v.tc[1] = 24 << 5;
            gSPVertex(POLY_OPA_DISP++, mv, 4, 0);
            KaleidoScope_DrawTexQuadRGBA32(play->state.gfxCtx, tex, 24, 24, 0);

            CLOSE_DISPS(play->state.gfxCtx);
        }
    }
}

// ── SW97 element wheel (bow cell 3 / slingshot cell 6) — Skijer's NEI ────────────────────────────
// Structurally the Gust Jar wheel above, not the old page-0 cycler: it never touches the cell's item
// or any C-button, it steps a FLAG. That is the whole point of the refactor — the twelve
// ITEM_SW97_ARROW_*/BULLET_* ids existed only so this wheel had something to write.
//
// Two independent flags, one per weapon, so a bow primed with spirit and a slingshot primed with
// wind coexist. Bomb Arrows is the last entry and only ever appears on the bow.
#define SW97_BOW_CELL 3
#define SW97_SLING_CELL 6

static void Sw97Wheel_Handle(PlayState* play, s32 cell, u8 isSling) {
    Input* input = CONTROLLER1(&play->state);
    PauseContext* pauseCtx = &play->pauseCtx;

    if (ExtInv_GetSlotItem(ExtInv_GetInventorySlot(cell)) == ITEM_NONE) {
        return; // weapon not owned
    }
    if (Sw97_ElementCount(isSling) < 2) {
        return; // only the bare weapon is available — nothing to pick between
    }

    if ((pauseCtx->cursorSlot[PAUSE_ITEM] == cell) && CHECK_BTN_ALL(input->press.button, BTN_A)) {
        Audio_PlaySfx(NA_SE_SY_DECIDE);
        gCurrentItemCyclingSlot = (gCurrentItemCyclingSlot == cell) ? -1 : cell;
    }
    if (gCurrentItemCyclingSlot == cell) {
        s8 dir = 0;

        pauseCtx->cursorColorSet = 8;
        if (pauseCtx->stickAdjX > 30) {
            dir = 1;
        } else if (pauseCtx->stickAdjX < -30) {
            dir = -1;
        }
        if (dir != 0) {
            Audio_PlaySfx(NA_SE_SY_CURSOR);
            Sw97_SetElement(isSling, Sw97_ElementNeighbor(isSling, Sw97_GetElement(isSling), dir));
            // The HUD composite is built from iconItemSegment[], which only refreshes when the
            // button's ITEM changes — and it no longer does. Ask for the reload by hand.
            Sw97_RefreshButtonIcons(play);
        }
        gCurrentItemCyclingSlot = (pauseCtx->cursorSlot[PAUSE_ITEM] == cell) ? cell : -1;
    }
}

static void Sw97Wheel_Draw(PlayState* play, s32 cell, u8 isSling) {
    PauseContext* pauseCtx = &play->pauseCtx;
    u8 elem;

    if (ExtInv_GetSlotItem(ExtInv_GetInventorySlot(cell)) == ITEM_NONE) {
        return;
    }
    elem = Sw97_GetElement(isSling);

    // Prev/next previews + the A hint. forceShow because the neighbours are medallion icons, never
    // the cell's own item value.
    if (Sw97_ElementCount(isSling) > 1) {
        KaleidoScope_DrawItemCycleExtrasImpl(play, cell, true,
                                             (u8)Sw97_ElementIcon(Sw97_ElementNeighbor(isSling, elem, -1)),
                                             (u8)Sw97_ElementIcon(Sw97_ElementNeighbor(isSling, elem, 1)), true);
    }

    // Primed element as a badge on the cell's top-right corner, so it is readable without opening
    // the wheel. Same per-vertex quad scheme as the Gust Jar's. SW97_ELEM_NONE = bare weapon, no
    // badge. (The grid draw separately composites the medallion BEHIND the weapon at half alpha —
    // this badge is the small, always-legible marker on top of that.)
    if (elem != SW97_ELEM_NONE) {
        void* tex = ExtInv_GetItemIcon(Sw97_ElementIcon(elem));

        if (tex != NULL) {
            s16 texSize = (elem == SW97_ELEM_BOMB) ? 32 : 24; // bomb arrows icon is a full item
            Vtx* cellVtx = &pauseCtx->itemVtx[cell * 4];
            Vtx* mv = GRAPH_ALLOC(play->state.gfxCtx, 4 * sizeof(Vtx));
            s16 cx = (cellVtx[0].v.ob[0] + cellVtx[3].v.ob[0]) / 2;
            s16 cy = (cellVtx[0].v.ob[1] + cellVtx[3].v.ob[1]) / 2;
            s16 mSize = 14;
            s16 mx0 = cx + 18 - mSize;
            s16 myTop = cy + 18;
            s32 mvi;

            OPEN_DISPS(play->state.gfxCtx);

            gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);

            for (mvi = 0; mvi < 4; mvi++) {
                mv[mvi] = cellVtx[0];
            }
            mv[0].v.ob[0] = mx0;
            mv[0].v.ob[1] = myTop;
            mv[0].v.tc[0] = 0;
            mv[0].v.tc[1] = 0;
            mv[1].v.ob[0] = mx0 + mSize;
            mv[1].v.ob[1] = myTop;
            mv[1].v.tc[0] = texSize << 5;
            mv[1].v.tc[1] = 0;
            mv[2].v.ob[0] = mx0;
            mv[2].v.ob[1] = myTop - mSize;
            mv[2].v.tc[0] = 0;
            mv[2].v.tc[1] = texSize << 5;
            mv[3].v.ob[0] = mx0 + mSize;
            mv[3].v.ob[1] = myTop - mSize;
            mv[3].v.tc[0] = texSize << 5;
            mv[3].v.tc[1] = texSize << 5;
            gSPVertex(POLY_OPA_DISP++, mv, 4, 0);
            KaleidoScope_DrawTexQuadRGBA32(play->state.gfxCtx, tex, texSize, texSize, 0);

            CLOSE_DISPS(play->state.gfxCtx);
        }
    }
}

// ── Elemental Wand wheel (page 2, the cell Bomb Arrows vacated) — Skijer's NEI ───────────────────
// Six rods in ONE cell. The cell's icon and name follow the active mode (ExtInv_GetItemIcon /
// ExtInv_GetCustomItemNameTex resolve them from Wand_GetMode), and each side of the wheel previews
// the MEDALLION that unlocks the neighbouring rod — so the wheel doubles as a reminder of what is
// still missing. This is the pattern that keeps the page-2 slot array from growing.
#define WAND_KALEIDO_CELL (SLOT_ELEMENTAL_WAND - 24)

static void Wand_KaleidoHandle(PlayState* play) {
    Input* input = CONTROLLER1(&play->state);
    PauseContext* pauseCtx = &play->pauseCtx;

    if (ExtInv_GetSlotItem(SLOT_ELEMENTAL_WAND) == ITEM_NONE) {
        return;
    }
    if (Wand_ModeCount() < 2) {
        return;
    }

    if ((pauseCtx->cursorSlot[PAUSE_ITEM] == WAND_KALEIDO_CELL) && CHECK_BTN_ALL(input->press.button, BTN_A)) {
        Audio_PlaySfx(NA_SE_SY_DECIDE);
        gCurrentItemCyclingSlot = (gCurrentItemCyclingSlot == WAND_KALEIDO_CELL) ? -1 : WAND_KALEIDO_CELL;
    }
    if (gCurrentItemCyclingSlot == WAND_KALEIDO_CELL) {
        s8 dir = 0;

        pauseCtx->cursorColorSet = 8;
        if (pauseCtx->stickAdjX > 30) {
            dir = 1;
        } else if (pauseCtx->stickAdjX < -30) {
            dir = -1;
        }
        if (dir != 0) {
            Audio_PlaySfx(NA_SE_SY_CURSOR);
            Wand_SetMode(Wand_ModeNeighbor(Wand_GetMode(), dir));
            // The HUD caches the resolved icon per button (iconItemSegment[]); the item id did not
            // change, so ask for the reload — same reasoning as Sw97_RefreshButtonIcons.
            ExtInv_RefreshButtonIconsForItem(play, ITEM_ELEMENTAL_WAND);
        }
        gCurrentItemCyclingSlot = (pauseCtx->cursorSlot[PAUSE_ITEM] == WAND_KALEIDO_CELL) ? WAND_KALEIDO_CELL : -1;
    }
}

static void Wand_KaleidoDraw(PlayState* play) {
    PauseContext* pauseCtx = &play->pauseCtx;
    u8 mode;

    if ((ExtInv_GetSlotItem(SLOT_ELEMENTAL_WAND) == ITEM_NONE) || (Wand_ModeCount() < 2)) {
        return;
    }
    mode = Wand_GetMode();

    KaleidoScope_DrawItemCycleExtrasImpl(play, WAND_KALEIDO_CELL, true,
                                         (u8)Wand_ModeMedallion(Wand_ModeNeighbor(mode, -1)),
                                         (u8)Wand_ModeMedallion(Wand_ModeNeighbor(mode, 1)), true);

    // The active rod's medallion as a corner badge, same scheme as the SW97 wheel above.
    {
        void* tex = ExtInv_GetItemIcon(Wand_ModeMedallion(mode));

        if (tex != NULL) {
            Vtx* cellVtx = &pauseCtx->itemVtx[WAND_KALEIDO_CELL * 4];
            Vtx* mv = GRAPH_ALLOC(play->state.gfxCtx, 4 * sizeof(Vtx));
            s16 cx = (cellVtx[0].v.ob[0] + cellVtx[3].v.ob[0]) / 2;
            s16 cy = (cellVtx[0].v.ob[1] + cellVtx[3].v.ob[1]) / 2;
            s16 mSize = 14;
            s16 mx0 = cx + 18 - mSize;
            s16 myTop = cy + 18;
            s32 mvi;

            OPEN_DISPS(play->state.gfxCtx);

            gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);

            for (mvi = 0; mvi < 4; mvi++) {
                mv[mvi] = cellVtx[0];
            }
            mv[0].v.ob[0] = mx0;
            mv[0].v.ob[1] = myTop;
            mv[0].v.tc[0] = 0;
            mv[0].v.tc[1] = 0;
            mv[1].v.ob[0] = mx0 + mSize;
            mv[1].v.ob[1] = myTop;
            mv[1].v.tc[0] = 24 << 5;
            mv[1].v.tc[1] = 0;
            mv[2].v.ob[0] = mx0;
            mv[2].v.ob[1] = myTop - mSize;
            mv[2].v.tc[0] = 0;
            mv[2].v.tc[1] = 24 << 5;
            mv[3].v.ob[0] = mx0 + mSize;
            mv[3].v.ob[1] = myTop - mSize;
            mv[3].v.tc[0] = 24 << 5;
            mv[3].v.tc[1] = 24 << 5;
            gSPVertex(POLY_OPA_DISP++, mv, 4, 0);
            KaleidoScope_DrawTexQuadRGBA32(play->state.gfxCtx, tex, 24, 24, 0);

            CLOSE_DISPS(play->state.gfxCtx);
        }
    }
}

// ── Sheikah Slate rune wheel (page 2, SLOT_SHEIKAH_SLATE) — Skijer's NEI ─────────────────────────
// Four runes in ONE cell, wand idiom. The cell's icon follows the active rune (ExtInv_GetItemIcon
// returns the slate-with-badge composite from Slate_GetRune); the wheel previews the neighbouring
// runes' 24x24 glyphs on each side — the gust-jar "mini icons on top" look.
#define SLATE_KALEIDO_CELL (SLOT_SHEIKAH_SLATE - 24)

static void Slate_KaleidoHandle(PlayState* play) {
    Input* input = CONTROLLER1(&play->state);
    PauseContext* pauseCtx = &play->pauseCtx;

    if (ExtInv_GetSlotItem(SLOT_SHEIKAH_SLATE) == ITEM_NONE) {
        return;
    }
    if (Slate_RuneCount() < 2) {
        return;
    }

    if ((pauseCtx->cursorSlot[PAUSE_ITEM] == SLATE_KALEIDO_CELL) && CHECK_BTN_ALL(input->press.button, BTN_A)) {
        Audio_PlaySfx(NA_SE_SY_DECIDE);
        gCurrentItemCyclingSlot = (gCurrentItemCyclingSlot == SLATE_KALEIDO_CELL) ? -1 : SLATE_KALEIDO_CELL;
    }
    if (gCurrentItemCyclingSlot == SLATE_KALEIDO_CELL) {
        s8 dir = 0;

        pauseCtx->cursorColorSet = 8;
        if (pauseCtx->stickAdjX > 30) {
            dir = 1;
        } else if (pauseCtx->stickAdjX < -30) {
            dir = -1;
        }
        if (dir != 0) {
            Audio_PlaySfx(NA_SE_SY_CURSOR);
            Slate_SetRune(Slate_RuneNeighbor(Slate_GetRune(), dir));
            // Same HUD icon-cache reload as the wand wheel (slate rides an EXT-button marker).
            ExtInv_RefreshButtonIconsForItem(play, EXT_ITEM_SHEIKAH_SLATE);
        }
        gCurrentItemCyclingSlot = (pauseCtx->cursorSlot[PAUSE_ITEM] == SLATE_KALEIDO_CELL) ? SLATE_KALEIDO_CELL : -1;
    }
}

// One 14px rune glyph quad at an offset from the cell centre — the wand's corner-badge scheme,
// reused for the wheel's side previews too (runes have no u8 item id, so the generic
// KaleidoScope_DrawItemCycleExtrasImpl side-item path cannot resolve them).
static void Slate_KaleidoDrawGlyph(PlayState* play, Vtx* cellVtx, void* tex, s16 offX, s16 offY, u8 alpha) {
    Vtx* mv = GRAPH_ALLOC(play->state.gfxCtx, 4 * sizeof(Vtx));
    s16 cx = (cellVtx[0].v.ob[0] + cellVtx[3].v.ob[0]) / 2;
    s16 cy = (cellVtx[0].v.ob[1] + cellVtx[3].v.ob[1]) / 2;
    s16 mSize = 14;
    s16 mx0 = cx + offX - mSize / 2;
    s16 myTop = cy + offY + mSize / 2;
    s32 mvi;

    OPEN_DISPS(play->state.gfxCtx);

    gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, alpha);

    for (mvi = 0; mvi < 4; mvi++) {
        mv[mvi] = cellVtx[0];
    }
    mv[0].v.ob[0] = mx0;
    mv[0].v.ob[1] = myTop;
    mv[0].v.tc[0] = 0;
    mv[0].v.tc[1] = 0;
    mv[1].v.ob[0] = mx0 + mSize;
    mv[1].v.ob[1] = myTop;
    mv[1].v.tc[0] = 32 << 5;
    mv[1].v.tc[1] = 0;
    mv[2].v.ob[0] = mx0;
    mv[2].v.ob[1] = myTop - mSize;
    mv[2].v.tc[0] = 0;
    mv[2].v.tc[1] = 32 << 5;
    mv[3].v.ob[0] = mx0 + mSize;
    mv[3].v.ob[1] = myTop - mSize;
    mv[3].v.tc[0] = 32 << 5;
    mv[3].v.tc[1] = 32 << 5;
    gSPVertex(POLY_OPA_DISP++, mv, 4, 0);
    KaleidoScope_DrawTexQuadRGBA32(play->state.gfxCtx, tex, 32, 32, 0);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void Slate_KaleidoDraw(PlayState* play) {
    PauseContext* pauseCtx = &play->pauseCtx;
    u8 rune;
    Vtx* cellVtx;

    if ((ExtInv_GetSlotItem(SLOT_SHEIKAH_SLATE) == ITEM_NONE) || (Slate_RuneCount() < 2)) {
        return;
    }
    rune = Slate_GetRune();
    cellVtx = &pauseCtx->itemVtx[SLATE_KALEIDO_CELL * 4];

    // Wheel open: neighbouring runes' glyphs on each side, half alpha.
    if (gCurrentItemCyclingSlot == SLATE_KALEIDO_CELL) {
        Slate_KaleidoDrawGlyph(play, cellVtx, Slate_RuneMiniIcon(Slate_RuneNeighbor(rune, -1)), -26, 0,
                               (u8)(pauseCtx->alpha >> 1));
        Slate_KaleidoDrawGlyph(play, cellVtx, Slate_RuneMiniIcon(Slate_RuneNeighbor(rune, 1)), 26, 0,
                               (u8)(pauseCtx->alpha >> 1));
    }

    // The active rune's glyph as a corner badge, same scheme as the wand's medallion badge.
    Slate_KaleidoDrawGlyph(play, cellVtx, Slate_RuneMiniIcon(rune), 11, 11, (u8)pauseCtx->alpha);
}

void KaleidoScope_HandleItemCycles(PlayState* play) {
    s32 page = ExtInv_GetCurrentPage();
    // A wheel left open on one page must not be inherited by the same cell index on another —
    // page 0 cell 13 is lens/pictograph, page 1 cell 13 is the gust jar.
    static s8 sCycleOpenPage = 0;

    if (page != sCycleOpenPage) {
        gCurrentItemCyclingSlot = -1;
        sCycleOpenPage = (s8)page;
    }

    if (page == 1) {
        Page2Relayout_Heal(); // fold pre-2026-08-06 saves into the new cell assignment (idempotent)
        GustJar_KaleidoHandle(play);
        Cane_KaleidoHandle(play);
        Wand_KaleidoHandle(play);    // Elemental Wand rod selector (Skijer's NEI)
        Slate_KaleidoHandle(play);   // Sheikah Slate rune selector (Skijer's NEI)
        Lantern_KaleidoHandle(play); // Lantern fire-type selector (ported from SoH)
        Shovel_KaleidoHandle(play);  // Shovel <-> Dominion Rod (2026-08-06 re-layout)
        return;
    }
    if (page != 0) {
        return;
    }

    // [2] Bombs <-> Power Keg (keg = MM native item; both live on the bomb cell)
    {
        u8 kegOwned = (ExtInv_GetSlotItem(SLOT_POWDER_KEG) != ITEM_NONE) || Nei_Save()->powerKegOwned;
        KaleidoScope_HandleItemCycleExtras(play, 2, kegOwned, ITEM_BOMB, ITEM_POWDER_KEG, true);
    }
    // [3] Bow element wheel, [6] slingshot element wheel. These use the flag wheel, NOT
    // KaleidoScope_HandleItemCycleExtras — that helper swaps the cell's ITEM and writes the
    // C-button, which is exactly the behavior this refactor removes. Skijer's NEI
    // [12] Boomerang <-> Gale Boomerang (ported from SoH).
    Gale_KaleidoHandle(play);
    if (CVarGetInteger("gEnhancements.SkijerNEI.SW97Medallions", 0)) {
        Sw97Wheel_Handle(play, SW97_BOW_CELL, 0);
        Sw97Wheel_Handle(play, SW97_SLING_CELL, 1);
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
        // Both entries are independently obtainable, so each side of the wheel is offered on its own
        // ownership — and the wheel can cycle as soon as you have EITHER. Asking SLOT_ROCS (the
        // page-2 progressive) was the wrong check: it left a rando-placed ship-vanilla feather
        // visible in the cell but impossible to cycle off. Skijer's NEI
        u8 rocs = NayrusWheel_HasRocs() ? ITEM_ROCS_FEATHER : ITEM_NONE;
        u8 nayrus = NayrusWheel_HasNayrus() ? ITEM_NAYRUS_LOVE : ITEM_NONE;
        KaleidoScope_HandleItemCycleExtras(play, 17, (rocs != ITEM_NONE) && (nayrus != ITEM_NONE), nayrus, rocs, true);
    }
    // [22] Unified trade wheel (OoT + MM trade items, trade_items.c). 22 is the VISUAL cell (row 4,
    // col 5); ExtInv_GetInventorySlot maps it through sOotPage0Map to MM's real SLOT_TRADE_DEED.
    {
        // Dedicated handler instead of KaleidoScope_HandleItemCycleExtras: that helper cycles by
        // writing item IDs, and in MM the 14 OoT trade entries all resolve to ITEM_TRADE_PLACEHOLDER
        // (they have no MM item id at all), so it could not tell them apart — its own
        // `slotItem != leftItem` guard would see "same item" and refuse to step. This drives the save's
        // index cursor and the cell just mirrors it. Skijer 2026-07-29
        Input* tradeInput = CONTROLLER1(&play->state);
        PauseContext* tradePause = &play->pauseCtx;
        s32 tradeRealSlot = ExtInv_GetInventorySlot(22);
        u16 tradeCur = ExtInv_GetSlotItem(tradeRealSlot);

        // MM scatters its trade items across THREE native slots, and Item_Give sends each one to its
        // own home — so only the deeds/Moon's Tear ever landed in the unified cell, and Room Key,
        // Letter to Mama, Letter to Kafei and the Pendant stayed invisible in a slot the wheel does not
        // read. Fold all three into the bitmask so every trade item reaches the wheel regardless of
        // which slot the engine put it in. Non-trade contents fold to nothing (IndexOfItem -> -1).
        // Skijer 2026-07-30
        TradeAdult_FoldCurrent(tradeCur);                                // SLOT_TRADE_DEED (this cell)
        TradeAdult_FoldCurrent(ExtInv_GetSlotItem(SLOT_TRADE_KEY_MAMA)); // Room Key / Letter to Mama
        TradeAdult_FoldCurrent(ExtInv_GetSlotItem(SLOT_TRADE_COUPLE));   // Letter to Kafei / Pendant

        // Mirror the cursor into the cell. This also SEEDS it: an item earned in OoT arrives as a
        // tradeAdultOwned bit (FleetSync) while MM's cell is still ITEM_NONE — owned but invisible.
        u8 tradeCell = TradeAdult_CellItem();
        if (tradeCell != tradeCur) {
            ExtInv_SetSlotItem(tradeRealSlot, tradeCell);
            tradeCur = tradeCell;
        }

        u8 tradeCanCycle = (TradeAdult_OwnedCount() > 1);
        if (tradeCanCycle && (tradePause->cursorSlot[PAUSE_ITEM] == 22) &&
            CHECK_BTN_ALL(tradeInput->press.button, BTN_A)) {
            Audio_PlaySfx(NA_SE_SY_DECIDE);
            gCurrentItemCyclingSlot = (gCurrentItemCyclingSlot == 22) ? -1 : 22;
        }
        if (gCurrentItemCyclingSlot == 22) {
            tradePause->cursorColorSet = 8;
            s32 tradeDir = (tradePause->stickAdjX > 30) ? 1 : ((tradePause->stickAdjX < -30) ? -1 : 0);
            if (tradeCanCycle && (tradeDir != 0)) {
                Audio_PlaySfx(NA_SE_SY_CURSOR);
                TradeAdult_CursorStep(tradeDir);
                u8 tradeNew = TradeAdult_CellItem();
                // Follow the item on the C buttons, like the generic helper does. The button SLOT has
                // to stay coherent with the item's home slot or MM flows that read
                // GET_CUR_FORM_BTN_SLOT hang.
                for (s32 ci = EQUIP_SLOT_C_LEFT; ci <= EQUIP_SLOT_C_RIGHT; ci++) {
                    if (BUTTON_ITEM_EQUIP(0, ci) == tradeCur) {
                        BUTTON_ITEM_EQUIP(0, ci) = tradeNew;
                        if (tradeNew < 52) {
                            C_SLOT_EQUIP(0, ci) = gItemSlots[tradeNew];
                        }
                        Interface_LoadItemIconImpl(play, ci);
                        break;
                    }
                }
                ExtInv_SetSlotItem(tradeRealSlot, tradeNew);
            }
        }
    }

    // [23] OoT child-trade MASK wheel (cell 4,6 -> VSLOT_OOT_MASKS). Same dedicated-handler reasoning
    // as the trade cell above: the 8 OoT masks share one placeholder id, so the generic cycle helper
    // cannot tell them apart. The cell content is synthesized in ExtInv_GetOotSlotItem, so nothing is
    // written here — only the cursor moves. Skijer 2026-07-30
    {
        Input* maskInput = CONTROLLER1(&play->state);
        PauseContext* maskPause = &play->pauseCtx;
        u8 maskCanCycle = (OotMask_OwnedCount() > 1);

        if (maskCanCycle && (maskPause->cursorSlot[PAUSE_ITEM] == 23) &&
            CHECK_BTN_ALL(maskInput->press.button, BTN_A)) {
            Audio_PlaySfx(NA_SE_SY_DECIDE);
            gCurrentItemCyclingSlot = (gCurrentItemCyclingSlot == 23) ? -1 : 23;
        }
        if (gCurrentItemCyclingSlot == 23) {
            maskPause->cursorColorSet = 8;
            s32 maskDir = (maskPause->stickAdjX > 30) ? 1 : ((maskPause->stickAdjX < -30) ? -1 : 0);
            if (maskCanCycle && (maskDir != 0)) {
                Audio_PlaySfx(NA_SE_SY_CURSOR);
                OotMask_CursorStep(maskDir);
            }
        }
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
    s32 page = ExtInv_GetCurrentPage();

    if (page == 1) {
        GustJar_KaleidoDraw(play);
        Cane_KaleidoDraw(play);
        Wand_KaleidoDraw(play);    // Elemental Wand rod selector (Skijer's NEI)
        Slate_KaleidoDraw(play);   // Sheikah Slate rune selector (Skijer's NEI)
        Lantern_KaleidoDraw(play); // Lantern fire-type selector (ported from SoH)
        Shovel_KaleidoDraw(play);  // Shovel <-> Dominion Rod (2026-08-06 re-layout)
        return;
    }
    if (page != 0) {
        return;
    }

    // OoT page-0 cell wheels (mirror of KaleidoScope_HandleItemCycles)
    {
        u8 kegOwned = (ExtInv_GetSlotItem(SLOT_POWDER_KEG) != ITEM_NONE) || Nei_Save()->powerKegOwned;
        KaleidoScope_DrawItemCycleExtras(play, 2, kegOwned, ITEM_BOMB, ITEM_POWDER_KEG);
    }
    // [12] Boomerang <-> Gale Boomerang (ported from SoH).
    Gale_KaleidoDraw(play);
    if (CVarGetInteger("gEnhancements.SkijerNEI.SW97Medallions", 0)) { // Skijer's NEI element wheels
        Sw97Wheel_Draw(play, SW97_BOW_CELL, 0);
        Sw97Wheel_Draw(play, SW97_SLING_CELL, 1);
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
            mv[0].v.ob[0] = mx0;
            mv[0].v.ob[1] = myTop;
            mv[0].v.tc[0] = 0;
            mv[0].v.tc[1] = 0;
            mv[1].v.ob[0] = mx0 + mSize;
            mv[1].v.ob[1] = myTop;
            mv[1].v.tc[0] = 24 << 5;
            mv[1].v.tc[1] = 0;
            mv[2].v.ob[0] = mx0;
            mv[2].v.ob[1] = myTop - mSize;
            mv[2].v.tc[0] = 0;
            mv[2].v.tc[1] = 24 << 5;
            mv[3].v.ob[0] = mx0 + mSize;
            mv[3].v.ob[1] = myTop - mSize;
            mv[3].v.tc[0] = 24 << 5;
            mv[3].v.tc[1] = 24 << 5;
            gSPVertex(POLY_OPA_DISP++, mv, 4, 0);
            KaleidoScope_DrawTexQuadRGBA32(play->state.gfxCtx,
                                           (void*)"__OTR__textures/icon_item_24_static/gQuestIconMedallionLightTex", 24,
                                           24, 0);

            CLOSE_DISPS(play->state.gfxCtx);
        }
    }
    {
        u8 pictoOwned = (gSaveContext.save.saveInfo.inventory.items[SLOT_PICTOGRAPH_BOX] != ITEM_NONE);
        KaleidoScope_DrawItemCycleExtras(play, 13, pictoOwned, ITEM_LENS_OF_TRUTH, ITEM_PICTOGRAPH_BOX);
    }
    {
        // Same ownership pair as the handle pass above — they must agree or the cell draws a
        // candidate you cannot select.
        u8 rocs = NayrusWheel_HasRocs() ? ITEM_ROCS_FEATHER : ITEM_NONE;
        u8 nayrus = NayrusWheel_HasNayrus() ? ITEM_NAYRUS_LOVE : ITEM_NONE;
        KaleidoScope_DrawItemCycleExtras(play, 17, (rocs != ITEM_NONE) && (nayrus != ITEM_NONE), nayrus, rocs);
    }
    // [22] Unified trade wheel. Previews come from the index cursor's neighbours, not from
    // Prev/NextItem — those go through the item id, which is ITEM_NONE for every OoT entry and would
    // blank the arrows. Skijer 2026-07-29
    {
        KaleidoScope_DrawItemCycleExtras(play, 22, TradeAdult_OwnedCount() > 1, TradeAdult_NeighborCellItem(-1),
                                         TradeAdult_NeighborCellItem(1));
    }
    // [23] OoT child-trade mask wheel previews — neighbours of the mask cursor.
    {
        KaleidoScope_DrawItemCycleExtras(play, 23, OotMask_OwnedCount() > 1, OotMask_NeighborCellItem(-1),
                                         OotMask_NeighborCellItem(1));
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
        POLY_OPA_DISP = Gfx_DrawTexRectIA8(POLY_OPA_DISP, gAmmoDigitTextures[ammo], 8, 8, nRectLeft + 6, nRectTop, 8, 8,
                                           1 << 10, 1 << 10);

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
    } else if (item == ITEM_FAIRY_SLINGSHOT) {
        // Skijer's NEI: seed pouch — 0xA3 sits outside the vanilla AMMO()/SLOT() tables. The
        // elemental-bullet ids used to need folding in here; the cell holds a plain slingshot now.
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
        POLY_OPA_DISP = Gfx_DrawTexRectIA8(POLY_OPA_DISP, gAmmoDigitTextures[ammoUpperDigit], 8, 8, nRectLeft, nRectTop,
                                           8, 8, 1 << 10, 1 << 10);
    }

    // Draw lower digit
    POLY_OPA_DISP = Gfx_DrawTexRectIA8(POLY_OPA_DISP, gAmmoDigitTextures[ammo], 8, 8, nRectLeft + 6, nRectTop, 8, 8,
                                       1 << 10, 1 << 10);

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
                // Skijer's NEI: the two branches used to key on the elemental ITEM in the cell. The
                // cell holds a plain weapon now, so the trigger is the FLAG — and one branch covers
                // both weapons, since ExtInv_GetItemIcon(nIt) already gives the right one.
                u8 gIsSling = Sw97_IsSlingItem(nIt);
                u8 gElem = (Sw97_IsBowItem(nIt) || gIsSling) ? Sw97_EffectiveElement(gIsSling) : SW97_ELEM_NONE;
                // NULL-guarded: several cell icons resolve to an OTR path that may not exist yet
                // (trade/mask markers gate on oot.o2r, custom art on a rebuilt 2ship.o2r). Drawing a
                // NULL "texture" puts a garbage quad in the cell — visibly broken slots. An empty
                // cell until the art resolves is the correct degradation. Skijer's NEI
                void* nTex = ExtInv_GetItemIcon(nIt);

                if (nTex == NULL) {
                    // no resolvable art this frame — skip the quad, keep the loop's state intact
                } else if ((gElem >= SW97_ELEM_FIRE) && (gElem <= SW97_ELEM_WIND)) {
                    // OoT style: the element's medallion at HALF alpha behind, the weapon on top
                    void* mTex = ExtInv_GetItemIcon(Sw97_ElementIcon(gElem));
                    if (mTex != NULL) {
                        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha >> 1);
                        KaleidoScope_DrawTexQuadRGBA32(play->state.gfxCtx, mTex, 24, 24, 0);
                        gDPPipeSync(POLY_OPA_DISP++);
                    }
                    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);
                    gSPVertex(POLY_OPA_DISP++, &pauseCtx->itemVtx[j + 0], 4, 0);
                    KaleidoScope_DrawTexQuadRGBA32(play->state.gfxCtx, nTex, 32, 32, 0);
                } else {
                    KaleidoScope_DrawTexQuadRGBA32(play->state.gfxCtx, nTex, nSz, nSz, 0);
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
                    if (bottomlessCell || (((void)0, ExtInv_GetSlotItem(ExtInv_GetInventorySlot(i))) != ITEM_NONE)) {
                        KaleidoScope_DrawAmmoCount(pauseCtx, play->state.gfxCtx,
                                                   ((void)0, ExtInv_GetSlotItem(ExtInv_GetInventorySlot(i))), i);
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
                    //
                    // SLOT COLLISION FIX (Skijer 2026-07-30): every "is this item already on another
                    // button?" test in the equip machinery matches by SLOT ONLY —
                    // KaleidoScope_UpdateDpadItemEquip compares equipTargetSlot against
                    // C_SLOT_EQUIP(0, C_*) and KaleidoScope_SwapDpadItemToCItem compares it against
                    // DPAD_SLOT_EQUIP(0, D_*). Page-1 CUSTOM items used to land on 24..47, which is
                    // exactly the range vanilla MASK equips already occupy, so a custom item could
                    // collide with an unrelated mask/page-2 entry sitting on another button: the swap
                    // branch fired for the WRONG item and either wiped that button or swallowed the
                    // equip — which is why D-pad equipping from the second page misbehaved while
                    // page-0 items (slots 0..23, no overlap) always worked.
                    // Page-1 customs now get their own 72..95 band: unique, still u8, and still
                    // >= ITEM_NUM_SLOTS so every "not a plain inventory item" check keeps working.
                    {
                        s32 realSlot = ExtInv_GetInventorySlot(cursorSlot);
                        if (realSlot >= 48 && realSlot < 72) {
                            realSlot -= 24; // page-2 visual -> real mask slot (24..47)
                        } else if (realSlot >= 24 && realSlot < 48) {
                            realSlot += 48; // page-1 custom items -> 72..95 (no mask-range overlap)
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
                        // Give description on item through a message box.
                        //
                        // NEI: `0x1700 + itemId` only resolves for vanilla inventory items. Custom
                        // items (0xB6+) and any other id past that range are NOT in the message
                        // table, and Message_FindMessageNES leaves font->messageStart untouched on a
                        // miss — func_801514B0 would then read a stale (or NULL) MessageTableEntry.
                        // So: custom text first, vanilla message only when it really exists.
                        u8 textBoxPos = (pauseCtx->cursorYIndex[PAUSE_ITEM] < 2) ? 3 : 1;
                        u16 vanillaTextId = 0x1700 + pauseCtx->cursorItem[PAUSE_ITEM];
                        const char* customDesc = PauseItemDesc_Get(pauseCtx->cursorItem[PAUSE_ITEM], PAUSE_ITEM);

                        if (customDesc != NULL) {
                            pauseCtx->itemDescriptionOn = true;
                            PauseItemDesc_Show(play, customDesc, textBoxPos);
                        } else if (PauseItemDesc_VanillaTextExists(vanillaTextId)) {
                            pauseCtx->itemDescriptionOn = true;
                            func_801514B0(play, vanillaTextId, textBoxPos);
                        } else {
                            Audio_PlaySfx(NA_SE_SY_ERROR);
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

// Equip an item onto a C button, EXT-aware. Skijer's NEI: a u16 EXT id (>= EXT_ITEM_BASE, e.g. the
// Sheikah Slate) cannot live in the u8 buttonItems array, so it goes through the ext-button marker
// instead — ITEM_EXT_BUTTON parks in buttonItems and the real id in extButtons, which every icon
// site already resolves. Vanilla ids take the plain assignment.
static void KaleidoScope_EquipCButtonItem(PlayState* play, s32 cBtn, u16 item, u8 slot) {
    extern void ExtButton_SetItem(s32 form, s32 btn, u16 extId);
    extern void ExtButton_ClearItem(s32 form, s32 btn);

    if (item >= 0x0200) {
        ExtButton_SetItem(0, cBtn, item);
        C_SLOT_EQUIP(0, cBtn) = 0xFF; // not addressable as a vanilla inventory slot
    } else {
        ExtButton_ClearItem(0, cBtn); // drop any stale ext id this button carried
        BUTTON_ITEM_EQUIP(0, cBtn) = (u8)item;
        C_SLOT_EQUIP(0, cBtn) = slot;
    }
    Interface_LoadItemIconImpl(play, (u8)cBtn);
}

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
                KaleidoScope_EquipCButtonItem(play, EQUIP_SLOT_C_LEFT, pauseCtx->equipTargetItem,
                                              pauseCtx->equipTargetSlot);
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
                KaleidoScope_EquipCButtonItem(play, EQUIP_SLOT_C_DOWN, pauseCtx->equipTargetItem,
                                              pauseCtx->equipTargetSlot);
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
                KaleidoScope_EquipCButtonItem(play, EQUIP_SLOT_C_RIGHT, pauseCtx->equipTargetItem,
                                              pauseCtx->equipTargetSlot);
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
