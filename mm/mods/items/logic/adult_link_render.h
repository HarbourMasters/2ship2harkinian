/*
 * adult_link_render.h - "Adult mode" for the Time Gate item (Skijer's NEI).
 *
 * Renders OoT's ADULT Link skeleton in place of MM's human Link, VISUAL + collider only.
 * MM has no age system (gSaveContext.save.linkAge is always 0), so this is a self-owned
 * persistent toggle stored in the NEI save (NeiSaveData::timeGateAdultMode). The Time Gate
 * item flips it; the render reads it every frame.
 *
 * Technique (mirrors soh/mods/transformation_masks/garo_hybrid_render.cpp, adapted to 2ship's
 * un-indexed oot.o2r): the OoT adult skeleton header is loaded already-parsed via
 * MmAssets_LoadFromOotArchive, and each limb's display list is swapped for a self-contained,
 * deep-patched Gfx* via OotAssets_LoadGfxDirect. It is drawn at the player's world pos, posed
 * by MM's OWN jointTable 1:1 (OoT adult Link and MM human Link share the same 21-limb Flex/LOD
 * hierarchy). Vanilla Link is hidden the same way the SM64 Mario expansion hides it (early
 * return at the top of Player_Draw).
 *
 * This is intended as the first, reusable instance of a general "custom player form" render.
 */
#ifndef ADULT_LINK_RENDER_H
#define ADULT_LINK_RENDER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "z64.h"

// Flip the persistent adult-mode flag (called from the Time Gate "Yes" branch). Persists in save.
void AdultLink_Toggle(void);

// 1 if adult mode is currently ON (reads Nei_Save()->timeGateAdultMode).
s32 AdultLink_IsActive(void);

// 1 if we should DRAW adult Link and HIDE vanilla Link this frame (active AND the OoT model is
// loaded/ready). Returns 0 when oot.o2r isn't available so the game falls back to vanilla Link.
s32 AdultLink_ShouldHide(void);

// Submit the OoT adult Link skeleton into POLY_OPA. Call from the top of Player_Draw.
void AdultLink_Draw(PlayState* play, Player* player);

// Per-frame: force the body collider to adult height while active (the physics skeleton stays
// child, so the auto-computed height would stay child otherwise). Call from CustomItems_Update.
void AdultLink_UpdateCollider(Player* player);

#ifdef __cplusplus
}
#endif

#endif // ADULT_LINK_RENDER_H
