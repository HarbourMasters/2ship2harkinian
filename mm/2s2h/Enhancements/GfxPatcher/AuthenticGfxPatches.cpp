#include "AuthenticGfxPatches.h"

extern "C" {
#include <libultraship/libultra.h>
#include "objects/object_uch/object_uch.h"
#include "overlays/ovl_En_Syateki_Okuta/ovl_En_Syateki_Okuta.h"
#include "overlays/ovl_Obj_Jgame_Light/ovl_Obj_Jgame_Light.h"

void ResourceMgr_PatchGfxByName(const char* path, const char* patchName, int index, Gfx instruction);
void ResourceMgr_UnpatchGfxByName(const char* path, const char* patchName);
Gfx* ResourceMgr_LoadGfxByName(const char* path);
}

void PatchAlienEyeBeams() {
    // A bug in the Alien's eye beam DL is incorrectly referring to TEXEL1 in the color combiner when
    // the texture is loaded into TEXEL0, causing the CC rendering whatever was last loaded into TEXEL1.
    // Patching the instruction to TEXEL0 restores the effect as seen on hardware.
    ResourceMgr_PatchGfxByName(gAlienEyeBeamDL, "AlienEyeBeamFix", 4,
                               gsDPSetCombineLERP(NOISE, 0, PRIMITIVE, 0, 0, 0, 0, ENVIRONMENT, COMBINED, 0,
                                                  PRIMITIVE_ALPHA, PRIMITIVE, COMBINED, 0, TEXEL0, 0));
}

void PatchMiniGameCrossAndCircleSymbols() {
    // The X and O displayed in mini-games are incorrectly set to FMT_I instead of FMT_IA,
    // Fast3D throws an assert and does nothing as FMT_I with SIZ_16 is not a valid texture type.
    // Patching all the relevant instructions to use FMT_IA matching the exported texture
    const char* dLists[] = { gShootingGalleryOctorokCrossDL, gShootingGalleryOctorokCircleDL,
                             gObjJgameLightIncorrectDL, gObjJgameLightCorrectDL };

    for (auto& dl : dLists) {
        Gfx* instructions = ResourceMgr_LoadGfxByName(dl);

        // Use a mask to unset and set the new format while preserving the rest of the instruction
        const uintptr_t mask = UINTPTR_MAX ^ (_SHIFTL(7, 21, 3));

        Gfx in3 = instructions[3]; // G_SETTIMG_OTR_HASH
        in3.words.w0 = (in3.words.w0 & mask) | _SHIFTL(G_IM_FMT_IA, 21, 3);
        Gfx in5 = instructions[5]; // G_SETTILE
        in5.words.w0 = (in5.words.w0 & mask) | _SHIFTL(G_IM_FMT_IA, 21, 3);
        Gfx in9 = instructions[9]; // G_SETTILE
        in9.words.w0 = (in9.words.w0 & mask) | _SHIFTL(G_IM_FMT_IA, 21, 3);

        ResourceMgr_PatchGfxByName(dl, "SymbolFormatFix_3", 3, in3);
        ResourceMgr_PatchGfxByName(dl, "SymbolFormatFix_5", 5, in5);
        ResourceMgr_PatchGfxByName(dl, "SymbolFormatFix_9", 9, in9);
    }
}

// Applies required patches for authentic bugs to allow the game to play and render properly
void GfxPatcher_ApplyNecessaryAuthenticPatches() {
    PatchAlienEyeBeams();
    PatchMiniGameCrossAndCircleSymbols();
}
