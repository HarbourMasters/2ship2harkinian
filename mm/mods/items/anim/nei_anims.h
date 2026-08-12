/**
 * nei_anims.h — NEI custom player animations, loaded from 2ship.o2r. Skijer's NEI
 *
 * These used to be s16 arrays compiled into the binary (anim/<item>/*_data.c). They now live in
 * the game archive as SOH_PlayerAnimation resources, built from those same arrays by
 * apps/nei_anims/anim_c_to_o2r.py into mm/assets/custom/misc/link_animetion/, which the normal
 * asset pipeline packs into 2ship.o2r (OTRExporter/OTRExporter/Main.cpp adds any non-PNG/JSON
 * custom asset to the archive verbatim at its relative path).
 *
 * A SOH_PlayerAnimation resource is a RAW s16 payload, NOT a PlayerAnimationHeader — casting it
 * directly would read frame-0 data as frameCount/segment and memcpy out of bounds.
 * ResourceMgr_LoadPlayerAnimAsHeader wraps it in a real header (frameCount = totalS16 / 67) and
 * caches it, so the pointer stays valid for the session.
 *
 * NeiAnim_Load returns NULL if the resource is missing (o2r not regenerated yet); every caller
 * must handle that instead of playing a NULL animation.
 */

#ifndef NEI_ANIMS_H
#define NEI_ANIMS_H

#include "z64.h"

#define NEI_ANIM_PATH(name) "__OTR__misc/link_animetion/gPlayerAnim_nei_" name

#define NEI_ANIM_DEKULEAF_BLOW NEI_ANIM_PATH("dekuleaf_blow")
#define NEI_ANIM_SOMARIA NEI_ANIM_PATH("somaria")
#define NEI_ANIM_DEMISE_DESTRUCTION NEI_ANIM_PATH("demise_destruction")
#define NEI_ANIM_DAMPE_DIG NEI_ANIM_PATH("dampe_dig")
// Ship-vanilla Roc's Feather jump. Unlike the four above this one was not generated from a C array:
// it is SoH's own gPlayerAnim_link_rocs_feather_jump_Data copied over byte for byte. That works
// because OoT and MM share the player animation format exactly — 67 s16 per frame, same limb order,
// same 22-limb skeleton (see mods/anim_translator/mm_anim_loader.h, which relies on the same fact in
// the other direction). 13 frames. Skijer's NEI
#define NEI_ANIM_ROCS_FEATHER_JUMP NEI_ANIM_PATH("rocs_feather_jump")

extern uint8_t ResourceMgr_FileExists(const char* resName);
extern PlayerAnimationHeader* ResourceMgr_LoadPlayerAnimAsHeader(const char* animPath);

// Loads a NEI animation out of the archive. NULL when the resource isn't present.
static inline LinkAnimationHeader* NeiAnim_Load(const char* path) {
    if (path == NULL || !ResourceMgr_FileExists(path)) {
        return NULL;
    }
    return (LinkAnimationHeader*)ResourceMgr_LoadPlayerAnimAsHeader(path);
}

#endif // NEI_ANIMS_H
