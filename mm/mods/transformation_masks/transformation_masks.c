/**
 * transformation_masks.c — STUB.
 *
 * The SoH-ported transformation-FORM + mask-wear reimplementation (it recreated
 * MM's native Deku/Goron/Zora/Fierce-Deity forms + native mask-wearing, plus the
 * custom Garo/Gerudo/Pikachu forms) was REMOVED: 2ship IS Majora's Mask, so those
 * systems already exist natively and the reimplementation only fought them.
 *
 * These no-ops keep the handful of custom-item files that still reference the old
 * form API compiling ("not transformed / all items allowed / no form voice / no
 * dragon-scale swim"). The custom forms will be rebuilt MM-native later:
 *   - Pikachu  = full player replacement (standalone).
 *   - Garo/Gerudo/Kafei = new native forms modeled on Fierce Deity.
 * As each effect is reimplemented, its stub here is replaced with the real hook.
 */
#include "mods/transformation_masks/transformation_masks.h"

// ── Form-state queries — never transformed (MM natives own real forms) ──────────
u8 TransformMasks_IsEnabled(void) {
    return 0;
}
u8 TransformMasks_IsTransformed(void) {
    return 0;
}
u8 TransformMasks_IsTransformedAny(void) {
    return 0;
}
u8 TransformMasks_IsFDSkinMode(void) {
    return 0;
}
MmPlayerTransformation MmPlayer_GetForm(void) {
    return MM_PLAYER_FORM_HUMAN;
}
MmPlayerTransformation MmForm_GetCurrentForm(void) {
    return MM_PLAYER_FORM_HUMAN;
}

// ── Item / slot restriction — allow everything (no form gating) ─────────────────
u8 TransformMasks_IsItemAllowed(s32 item) {
    (void)item;
    return 1;
}
u8 TransformMasks_IsSlotAllowed(u8 slot) {
    (void)slot;
    return 1;
}

// ── No-op update / handlers ─────────────────────────────────────────────────────
void MmForm_Update(PlayState* play, Player* player) {
    (void)play;
    (void)player;
}
void MmForm_SaveAndRestrictEquips(PlayState* play, Player* player) {
    (void)play;
    (void)player;
}
void TransformMasks_HandleMaskUse(PlayState* play, Player* player, s32 item) {
    (void)play;
    (void)player;
    (void)item;
}
void TransformMasks_FilterB(Input* input) {
    (void)input;
}

// ── Form voice — no override (caller falls back to Link's normal voice) ─────────
u8 TransformMasks_TryPlayMmVoice(u16 ootVoiceSfxId, Vec3f* pos) {
    (void)ootVoiceSfxId;
    (void)pos;
    return 0;
}

// ── MM form-asset DL load — none (forms removed) ────────────────────────────────
void* TransformMasks_LoadMmDL(const char* path) {
    (void)path;
    return NULL;
}

// ── Dragon Scale Zora-swim — disabled until the swim effect is reimplemented ────
u8 TransformMasks_IsZoraSwimEnabled(void) {
    return 0;
}
u8 TransformMasks_DragonScaleEnterSwim(void* play, void* player) {
    (void)play;
    (void)player;
    return 0;
}
void TransformMasks_DragonScaleSwimUpdate(void* play, void* player) {
    (void)play;
    (void)player;
}
void TransformMasks_DragonScaleExitSwim(void* player) {
    (void)player;
}
