#include <libultraship/bridge/consolevariablebridge.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/BenGui/HudEditor.h"
#include "2s2h/Enhancements/FrameInterpolation/FrameInterpolation.h"

extern "C" {
#include "overlays/actors/ovl_En_Arrow/z_en_arrow.h"
#include "overlays/actors/ovl_En_Bom/z_en_bom.h"
#include "overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"
}

#include "assets/objects/object_gi_bomb_1/object_gi_bomb_1.h"

#define CVAR_NAME "gEnhancements.Equipment.BombArrows"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

#define BOMB_ARROW_FLAG (1 << 0)
#define BOMB_ARROW_IS_SET(arrow) (((arrow)->actor.home.rot.x & BOMB_ARROW_FLAG) != 0)
#define BOMB_ARROW_SET(arrow) ((arrow)->actor.home.rot.x |= BOMB_ARROW_FLAG)

extern "C" Gfx* ResourceMgr_LoadGfxByName(const char* path);

struct BombGiFlashDl {
    Gfx* dl = nullptr;
    size_t length = 0;
    size_t primIdx[8]{};
    size_t primCount = 0;
    size_t envIdx[8]{};
    size_t envCount = 0;
};

static BombGiFlashDl sBombGiFlash;

static size_t FindDlLength(const Gfx* dl) {
    for (size_t i = 0; i < 4096; i++) {
        u8 opcode = (u8)(dl[i].words.w0 >> 24);
        if (opcode == 0xDF) { // G_ENDDL
            return i + 1;
        }
    }
    return 0;
}

static u8 GetR(u32 rgba) {
    return (u8)(rgba >> 24);
}
static u8 GetG(u32 rgba) {
    return (u8)(rgba >> 16);
}
static u8 GetB(u32 rgba) {
    return (u8)(rgba >> 8);
}
static u32 PackRGBA(u8 r, u8 g, u8 b, u8 a = 255) {
    return ((u32)r << 24) | ((u32)g << 16) | ((u32)b << 8) | (u32)a;
}

static Gfx* GetBombGiFlashDl() {
    if (sBombGiFlash.dl != nullptr) {
        return sBombGiFlash.dl;
    }

    Gfx* original = ResourceMgr_LoadGfxByName(gGiBombDL);
    if (original == nullptr) {
        return nullptr;
    }

    size_t length = FindDlLength(original);
    if (length == 0) {
        return nullptr;
    }

    Gfx* copy = (Gfx*)std::malloc(sizeof(Gfx) * length);
    if (copy == nullptr) {
        return nullptr;
    }

    std::memcpy(copy, original, sizeof(Gfx) * length);

    constexpr u8 kBasePrimR = 0x00, kBasePrimG = 0x3C, kBasePrimB = 0x50;
    constexpr u8 kBaseEnvR = 0x00, kBaseEnvG = 0x14, kBaseEnvB = 0x28;

    for (size_t i = 0; i < length; i++) {
        u8 opcode = (u8)(copy[i].words.w0 >> 24);
        if (opcode == 0xFA) { // gDPSetPrimColor
            u32 rgba = copy[i].words.w1;
            if (GetR(rgba) == kBasePrimR && GetG(rgba) == kBasePrimG && GetB(rgba) == kBasePrimB &&
                sBombGiFlash.primCount < (sizeof(sBombGiFlash.primIdx) / sizeof(sBombGiFlash.primIdx[0]))) {
                sBombGiFlash.primIdx[sBombGiFlash.primCount++] = i;
            }
        } else if (opcode == 0xFB) { // gDPSetEnvColor
            u32 rgba = copy[i].words.w1;
            if (GetR(rgba) == kBaseEnvR && GetG(rgba) == kBaseEnvG && GetB(rgba) == kBaseEnvB &&
                sBombGiFlash.envCount < (sizeof(sBombGiFlash.envIdx) / sizeof(sBombGiFlash.envIdx[0]))) {
                sBombGiFlash.envIdx[sBombGiFlash.envCount++] = i;
            }
        }
    }

    sBombGiFlash.dl = copy;
    sBombGiFlash.length = length;
    return sBombGiFlash.dl;
}

static void UpdateBombGiFlashColors(PlayState* play) {
    if (sBombGiFlash.dl == nullptr) {
        return;
    }

    const float t = 0.5f + 0.5f * Math_SinS(play->gameplayFrames * 7000);
    auto lerp8 = [&](int a, int b) -> u8 { return (u8)std::clamp((int)std::lround(a + (b - a) * t), 0, 255); };

    const u8 primR = lerp8(0x00, 0xFF);
    const u8 primG = lerp8(0x3C, 0x28);
    const u8 primB = lerp8(0x50, 0x00);

    const u8 envR = lerp8(0x00, 0x7F);
    const u8 envG = 0x14;
    const u8 envB = lerp8(0x28, 0x00);

    const u32 primRGBA = PackRGBA(primR, primG, primB);
    const u32 envRGBA = PackRGBA(envR, envG, envB);

    for (size_t i = 0; i < sBombGiFlash.primCount; i++) {
        sBombGiFlash.dl[sBombGiFlash.primIdx[i]].words.w1 = primRGBA;
    }
    for (size_t i = 0; i < sBombGiFlash.envCount; i++) {
        sBombGiFlash.dl[sBombGiFlash.envIdx[i]].words.w1 = envRGBA;
    }
}

static const s16 sBCButtonXPositions[] = {
    167, // EQUIP_SLOT_B
    227, // EQUIP_SLOT_C_LEFT
    249, // EQUIP_SLOT_C_DOWN
    271, // EQUIP_SLOT_C_RIGHT
};

static const s16 sBCButtonYPositions[] = {
    17, // EQUIP_SLOT_B
    18, // EQUIP_SLOT_C_LEFT
    34, // EQUIP_SLOT_C_DOWN
    18, // EQUIP_SLOT_C_RIGHT
};

static const s16 sItemIconTextureDimensions[] = {
    30, // EQUIP_SLOT_B
    24, // EQUIP_SLOT_C_LEFT
    24, // EQUIP_SLOT_C_DOWN
    24, // EQUIP_SLOT_C_RIGHT
};

static const s16 sDpadItemIconLeft[] = {
    295, // EQUIP_SLOT_D_RIGHT
    263, // EQUIP_SLOT_D_LEFT
    279, // EQUIP_SLOT_D_DOWN
    279, // EQUIP_SLOT_D_UP
};

static const s16 sDpadItemIconTop[] = {
    63, // EQUIP_SLOT_D_RIGHT
    63, // EQUIP_SLOT_D_LEFT
    79, // EQUIP_SLOT_D_DOWN
    47, // EQUIP_SLOT_D_UP
};

static bool IsBowItem(ItemId item) {
    return (item == ITEM_BOW) || ((item >= ITEM_BOW_FIRE) && (item <= ITEM_BOW_LIGHT));
}

static bool ApplyHudEditorAdjustments(HudEditorElementID element, s16* rectLeft, s16* rectTop, s16* rectWidth,
                                      s16* rectHeight, s16* dsdx, s16* dtdy) {
    HudEditor_SetActiveElement(element);
    if (HudEditor_ShouldOverrideDraw()) {
        if (HudEditor_IsActiveElementHidden()) {
            return false;
        }
        HudEditor_ModifyDrawValues(rectLeft, rectTop, rectWidth, rectHeight, dsdx, dtdy);
    }
    return true;
}

static bool ResolvePauseEquipTarget(const PauseContext* pauseCtx, s32* targetSlot, bool* isDpad) {
    switch (pauseCtx->equipTargetCBtn) {
        case PAUSE_EQUIP_C_LEFT:
            *targetSlot = EQUIP_SLOT_C_LEFT;
            *isDpad = false;
            return true;
        case PAUSE_EQUIP_C_DOWN:
            *targetSlot = EQUIP_SLOT_C_DOWN;
            *isDpad = false;
            return true;
        case PAUSE_EQUIP_C_RIGHT:
            *targetSlot = EQUIP_SLOT_C_RIGHT;
            *isDpad = false;
            return true;
        case PAUSE_EQUIP_D_RIGHT:
            *targetSlot = EQUIP_SLOT_D_RIGHT;
            *isDpad = true;
            return true;
        case PAUSE_EQUIP_D_LEFT:
            *targetSlot = EQUIP_SLOT_D_LEFT;
            *isDpad = true;
            return true;
        case PAUSE_EQUIP_D_DOWN:
            *targetSlot = EQUIP_SLOT_D_DOWN;
            *isDpad = true;
            return true;
        case PAUSE_EQUIP_D_UP:
            *targetSlot = EQUIP_SLOT_D_UP;
            *isDpad = true;
            return true;
        default:
            return false;
    }
}

static bool IsBombArrowButton(const Player* player) {
    if (player->heldItemButton < 0) {
        return false;
    }

    if (IS_HELD_DPAD(player->heldItemButton)) {
        s32 dpadSlot = HELD_ITEM_TO_DPAD(player->heldItemButton);

        return (DPAD_BUTTON_ITEM_EQUIP(0, dpadSlot) == ITEM_BOW) && (DPAD_SLOT_EQUIP(0, dpadSlot) == SLOT_BOMB);
    }

    if (player->heldItemButton == EQUIP_SLOT_B) {
        return false;
    }

    EquipSlot slot = (EquipSlot)player->heldItemButton;

    return (BUTTON_ITEM_EQUIP(0, slot) == ITEM_BOW) && (C_SLOT_EQUIP(0, slot) == SLOT_BOMB);
}

static void EquipBombArrowToSlot(PlayState* play, s32 targetSlot, bool isDpad) {
    if (isDpad) {
        DPAD_BUTTON_ITEM_EQUIP(0, targetSlot) = ITEM_BOW;
        DPAD_SLOT_EQUIP(0, targetSlot) = SLOT_BOMB;
        Interface_Dpad_LoadItemIcon(play, targetSlot);
        gSaveContext.shipSaveContext.dpad.status[targetSlot] = BTN_ENABLED;
    } else {
        BUTTON_ITEM_EQUIP(0, targetSlot) = ITEM_BOW;
        C_SLOT_EQUIP(0, targetSlot) = SLOT_BOMB;
        Interface_LoadItemIcon(play, targetSlot);
        gSaveContext.buttonStatus[targetSlot] = BTN_ENABLED;
    }
}

static void GetSlotData(s32 slot, bool isDpad, ItemId* item, s32* equipSlot) {
    if (isDpad) {
        *item = (ItemId)DPAD_BUTTON_ITEM_EQUIP(0, slot);
        *equipSlot = DPAD_SLOT_EQUIP(0, slot);
    } else {
        *item = (ItemId)BUTTON_ITEM_EQUIP(0, slot);
        *equipSlot = C_SLOT_EQUIP(0, slot);
    }
}

static void SetSlotData(PlayState* play, s32 slot, bool isDpad, ItemId item, s32 equipSlot) {
    if (isDpad) {
        DPAD_BUTTON_ITEM_EQUIP(0, slot) = item;
        DPAD_SLOT_EQUIP(0, slot) = equipSlot;
        Interface_Dpad_LoadItemIcon(play, slot);
    } else {
        BUTTON_ITEM_EQUIP(0, slot) = item;
        C_SLOT_EQUIP(0, slot) = equipSlot;
        Interface_LoadItemIcon(play, slot);
    }
}

static void SwapItem(PlayState* play, s32 slot1, bool isDpad1, s32 slot2, bool isDpad2) {
    ItemId item1;
    s32 equipSlot1;
    GetSlotData(slot1, isDpad1, &item1, &equipSlot1);

    ItemId item2;
    s32 equipSlot2;
    GetSlotData(slot2, isDpad2, &item2, &equipSlot2);

    SetSlotData(play, slot1, isDpad1, item2, equipSlot2);
    SetSlotData(play, slot2, isDpad2, item1, equipSlot1);
}

static void RegisterBombArrowPauseEquip() {
    COND_VB_SHOULD(VB_KALEIDO_EQUIP_ITEM_TO_BUTTON, CVAR, {
        va_arg(args, int);
        u16 cursorItem = va_arg(args, int);

        PauseContext* pauseCtx = &gPlayState->pauseCtx;
        s32 targetSlot = -1;
        bool isDpad = false;

        if (!ResolvePauseEquipTarget(pauseCtx, &targetSlot, &isDpad)) {
            return;
        }

        if (IsBowItem((ItemId)cursorItem) || (cursorItem >= ITEM_ARROW_FIRE && cursorItem <= ITEM_ARROW_LIGHT)) {
            // Check C-Buttons for Bomb Arrows
            for (int i = EQUIP_SLOT_C_LEFT; i <= EQUIP_SLOT_C_RIGHT; i++) {
                if (IsBowItem((ItemId)BUTTON_ITEM_EQUIP(0, i)) && C_SLOT_EQUIP(0, i) == SLOT_BOMB) {
                    if (targetSlot != i || isDpad) { // Swap only if different slot
                        SwapItem(gPlayState, i, false, targetSlot, isDpad);
                        return;
                    }
                }
            }
            // Check D-Pad for Bomb Arrows
            for (int i = EQUIP_SLOT_D_RIGHT; i <= EQUIP_SLOT_D_UP; i++) {
                if (IsBowItem((ItemId)DPAD_BUTTON_ITEM_EQUIP(0, i)) && DPAD_SLOT_EQUIP(0, i) == SLOT_BOMB) {
                    if (targetSlot != i || !isDpad) { // Swap only if different slot
                        SwapItem(gPlayState, i, true, targetSlot, isDpad);
                        return;
                    }
                }
            }
            return;
        }

        if (cursorItem != ITEM_BOMB) {
            return;
        }

        ItemId equippedItem =
            static_cast<ItemId>(isDpad ? DPAD_BUTTON_ITEM_EQUIP(0, targetSlot) : BUTTON_ITEM_EQUIP(0, targetSlot));

        if (!IsBowItem(equippedItem)) {
            return;
        }

        EquipBombArrowToSlot(gPlayState, targetSlot, isDpad);
        Audio_PlaySfx(NA_SE_SY_DECIDE);
        *should = false;
    });
}

// Hook helpers for bomb arrows
static void BombArrow_OnInit(Actor* actor) {
    if (gPlayState == nullptr) {
        return;
    }

    Player* player = GET_PLAYER(gPlayState);

    if (player == nullptr || actor->parent != &player->actor) {
        return;
    }

    if (!IsBombArrowButton(player) || (AMMO(ITEM_BOMB) <= 0)) {
        return;
    }

    EnArrow* arrow = (EnArrow*)actor;

    if (!ARROW_IS_ARROW(arrow->actor.params)) {
        return;
    }

    BOMB_ARROW_SET(arrow);
    Inventory_ChangeAmmo(ITEM_BOMB, -1);
}

static void BombArrow_OnUpdate(Actor* actor) {
    if (gPlayState == nullptr) {
        return;
    }

    EnArrow* arrow = (EnArrow*)actor;

    if (!BOMB_ARROW_IS_SET(arrow) || !ARROW_IS_ARROW(arrow->actor.params)) {
        return;
    }

    // Keep fuse loop playing while drawn/in flight
    Actor_PlaySfx(actor, NA_SE_IT_BOMB_IGNIT - SFX_FLAG);

    Vec3f fusePos;
    Vec3f fuseVel = { 0.001f, 0.001f, 0.001f }; // fuse spread
    Vec3f fuseAccel = { 0.1f, 0.1f, 0.1f };

    Matrix_Push();
    Matrix_SetTranslateRotateYXZ(actor->world.pos.x, actor->world.pos.y + (actor->shape.yOffset * actor->scale.y),
                                 actor->world.pos.z, &actor->shape.rot);
    Matrix_Scale(actor->scale.x, actor->scale.y, actor->scale.z, MTXMODE_APPLY);
    Matrix_Translate(0.0f, 0.0f, 1000.0f, MTXMODE_APPLY);
    Vec3f fuseLocal = { 50.0f, 300.0f, 0.0f }; // fuse position
    Matrix_MultVec3f(&fuseLocal, &fusePos);
    Matrix_Pop();

    Color_RGBA8 fusePrim = { 255, 255, 150, 255 };
    Color_RGBA8 fuseEnv = { 255, 0, 0, 0 };
    EffectSsGSpk_SpawnAccel(gPlayState, actor, &fusePos, &fuseVel, &fuseAccel, &fusePrim, &fuseEnv, 30,
                            1); // spark size

    bool hit = (arrow->collider.base.atFlags & AT_HIT) || (arrow->unk_262 != 0);

    if (!hit) {
        return;
    }

    EnBom* bomb = (EnBom*)Actor_Spawn(&gPlayState->actorCtx, gPlayState, ACTOR_EN_BOM, arrow->actor.world.pos.x,
                                      arrow->actor.world.pos.y, arrow->actor.world.pos.z, 0, 0, 0, BOMB_TYPE_BODY);
    if (bomb != NULL) {
        bomb->timer = 0;
    }

    if (arrow->actor.child != NULL) {
        Actor_Kill(arrow->actor.child);
    }

    Actor_Kill(&arrow->actor);
}

static void BombArrow_OnDraw(Actor* actor) {
    if (gPlayState == nullptr) {
        return;
    }

    EnArrow* arrow = (EnArrow*)actor;
    if (!BOMB_ARROW_IS_SET(arrow) || !ARROW_IS_ARROW(arrow->actor.params)) {
        return;
    }

    OPEN_DISPS(gPlayState->state.gfxCtx);

    Gfx_SetupDL25_Opa(gPlayState->state.gfxCtx);

    Matrix_Push();

    Matrix_SetTranslateRotateYXZ(actor->world.pos.x, actor->world.pos.y + (actor->shape.yOffset * actor->scale.y),
                                 actor->world.pos.z, &actor->shape.rot);
    Matrix_Scale(actor->scale.x, actor->scale.y, actor->scale.z, MTXMODE_APPLY);
    // bomb position relative to arrow
    Matrix_Translate(0.0f, 0.0f, 1000.0f, MTXMODE_APPLY);

    // bomb scale
    Matrix_Scale(10.0f, 10.0f, 10.0f, MTXMODE_APPLY);

    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, gPlayState->state.gfxCtx);
    Gfx* dl = GetBombGiFlashDl();
    if (dl != nullptr) {
        UpdateBombGiFlashColors(gPlayState);
        gSPDisplayList(POLY_OPA_DISP++, dl);
    } else {
        gSPDisplayList(POLY_OPA_DISP++, (Gfx*)gGiBombDL);
    }

    Matrix_Pop();

    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

static void RegisterBombArrowBehavior() {
    // Mark arrows as bomb arrows on init and consume ammo
    COND_ID_HOOK(OnActorInit, ACTOR_EN_ARROW, CVAR, BombArrow_OnInit);

    // Single update hook: spawn fuse spark and detonate on hit
    COND_ID_HOOK(OnActorUpdate, ACTOR_EN_ARROW, CVAR, BombArrow_OnUpdate);

    // Draw hook: attach bomb model
    COND_ID_HOOK(OnActorDraw, ACTOR_EN_ARROW, CVAR, BombArrow_OnDraw);
}

static void DrawBombArrowOverlayCButton(PlayState* play, EquipSlot slot, s16 alpha) {
    s16 rectLeft = sBCButtonXPositions[slot];
    s16 rectTop = sBCButtonYPositions[slot];
    s16 baseSize = sItemIconTextureDimensions[slot];
    s16 overlaySize = 12;
    s16 rectWidth = overlaySize;
    s16 rectHeight = overlaySize;
    s16 overlayLeft = rectLeft + (baseSize - overlaySize) / 2 + 2;
    s16 overlayTop = rectTop + (baseSize - overlaySize) / 2 - 2;
    s16 dsdx = ((ICON_ITEM_TEX_WIDTH << 10) / rectWidth) >> 1;
    s16 dtdy = ((ICON_ITEM_TEX_HEIGHT << 10) / rectHeight) >> 1;

    if (!ApplyHudEditorAdjustments((HudEditorElementID)slot, &overlayLeft, &overlayTop, &rectWidth, &rectHeight, &dsdx,
                                   &dtdy)) {
        return;
    }

    OPEN_DISPS(play->state.gfxCtx);

    gDPPipeSync(OVERLAY_DISP++);
    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, alpha);
    gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);

    gDPLoadTextureBlock(OVERLAY_DISP++, gItemIcons[ITEM_BOMB], G_IM_FMT_RGBA, G_IM_SIZ_32b, ICON_ITEM_TEX_WIDTH,
                        ICON_ITEM_TEX_HEIGHT, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK,
                        G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

    gSPTextureRectangle(OVERLAY_DISP++, overlayLeft << 2, overlayTop << 2, (overlayLeft + rectWidth) << 2,
                        (overlayTop + rectHeight) << 2, G_TX_RENDERTILE, 0, 0, dsdx << 1, dtdy << 1);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void DrawBombArrowOverlayDpad(PlayState* play, DpadEquipSlot slot, s16 alpha) {
    s16 rectLeft = sDpadItemIconLeft[slot];
    s16 rectTop = sDpadItemIconTop[slot];
    s16 baseSize = 16;
    s16 overlaySize = 8;
    s16 rectWidth = overlaySize;
    s16 rectHeight = overlaySize;
    // 2S2H [Enhancement] Offset to match Fire Arrow magic position
    s16 overlayLeft = rectLeft + (baseSize - overlaySize) / 2 + 2;
    s16 overlayTop = rectTop + (baseSize - overlaySize) / 2 - 2;
    s16 dsdx = ((ICON_ITEM_TEX_WIDTH << 10) / rectWidth) >> 1;
    s16 dtdy = ((ICON_ITEM_TEX_HEIGHT << 10) / rectHeight) >> 1;

    if (!ApplyHudEditorAdjustments(HUD_EDITOR_ELEMENT_D_PAD, &overlayLeft, &overlayTop, &rectWidth, &rectHeight, &dsdx,
                                   &dtdy)) {
        return;
    }

    OPEN_DISPS(play->state.gfxCtx);

    gDPPipeSync(OVERLAY_DISP++);
    gDPSetPrimColor(OVERLAY_DISP++, 0, 0, 255, 255, 255, alpha);
    gDPSetCombineMode(OVERLAY_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);

    gDPLoadTextureBlock(OVERLAY_DISP++, gItemIcons[ITEM_BOMB], G_IM_FMT_RGBA, G_IM_SIZ_32b, ICON_ITEM_TEX_WIDTH,
                        ICON_ITEM_TEX_HEIGHT, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK,
                        G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

    gSPTextureRectangle(OVERLAY_DISP++, overlayLeft << 2, overlayTop << 2, (overlayLeft + rectWidth) << 2,
                        (overlayTop + rectHeight) << 2, G_TX_RENDERTILE, 0, 0, dsdx << 1, dtdy << 1);

    CLOSE_DISPS(play->state.gfxCtx);
}

static void RegisterBombArrowOverlay() {
    COND_HOOK(OnGameStateDrawFinish, CVAR, []() {
        if (gPlayState == nullptr) {
            return;
        }

        InterfaceContext* interfaceCtx = &gPlayState->interfaceCtx;
        DpadInterface* dpadInterface = &gPlayState->interfaceCtx.shipInterface.dpad;

        for (s32 slot = EQUIP_SLOT_C_LEFT; slot <= EQUIP_SLOT_C_RIGHT; slot++) {
            if ((BUTTON_ITEM_EQUIP(0, slot) == ITEM_BOW) && (C_SLOT_EQUIP(0, slot) == SLOT_BOMB)) {
                s16 alpha = (slot == EQUIP_SLOT_C_LEFT)   ? interfaceCtx->cLeftAlpha
                            : (slot == EQUIP_SLOT_C_DOWN) ? interfaceCtx->cDownAlpha
                                                          : interfaceCtx->cRightAlpha;
                DrawBombArrowOverlayCButton(gPlayState, (EquipSlot)slot, alpha);
            }
        }

        for (s32 slot = EQUIP_SLOT_D_RIGHT; slot <= EQUIP_SLOT_D_UP; slot++) {
            if ((DPAD_BUTTON_ITEM_EQUIP(0, slot) == ITEM_BOW) && (DPAD_SLOT_EQUIP(0, slot) == SLOT_BOMB)) {
                s16 alpha = (slot == EQUIP_SLOT_D_RIGHT)  ? dpadInterface->dRightAlpha
                            : (slot == EQUIP_SLOT_D_LEFT) ? dpadInterface->dLeftAlpha
                            : (slot == EQUIP_SLOT_D_DOWN) ? dpadInterface->dDownAlpha
                                                          : dpadInterface->dUpAlpha;
                DrawBombArrowOverlayDpad(gPlayState, (DpadEquipSlot)slot, alpha);
            }
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterBombArrowPauseEquip, { CVAR_NAME });
static RegisterShipInitFunc initBehavior(RegisterBombArrowBehavior, { CVAR_NAME });
static RegisterShipInitFunc initOverlay(RegisterBombArrowOverlay, { CVAR_NAME });
