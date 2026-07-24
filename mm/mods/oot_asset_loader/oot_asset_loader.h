/*
 * oot_asset_loader.h — load OoT-unique resources (Link equipment display lists,
 * medallion/item icons, OoT models) by their OTR path from oot.o2r / oot-mq.o2r.
 *
 * The archives themselves are added to the resource manager at LOWEST priority by
 * mm_asset_loader's LoadMmO2r (so MM/2ship win shared paths and there are no shadow
 * crashes). These wrappers just resolve a resource by name once the archive is in.
 *
 * Used by pak_loader and the kaleido icon path to pull OoT assets MM doesn't ship.
 * (Skijer's NEI.) Put oot.o2r / oot-mq.o2r next to 2ship.o2r or in the parent (../).
 */
#ifndef OOT_ASSET_LOADER_H
#define OOT_ASSET_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Load an OoT Gfx display list by OTR path, e.g.
 * "__OTR__objects/object_link_boy/gLinkAdultMasterSwordAndSheathNearDL".
 * Returns a Gfx* (as void*) or NULL if not found. */
void* OotAssets_LoadGfx(const char* otrPath);

/* Load an OoT texture or display list by OTR path (icons, etc.). void* / NULL. */
void* OotAssets_LoadTexOrDList(const char* otrPath);

/* Load an OoT Gfx display list DIRECTLY from the oot.o2r / oot-mq.o2r archive
 * handles, bypassing the resource-manager priority chain. Use for paths that are
 * SHADOWED by a same-named MM asset (e.g. object_gi_hookshot/gGiHookshotDL,
 * object_gi_shield_3/gGiMirrorShieldDL) — OotAssets_LoadGfx can never reach the
 * OoT version of those. Returns Gfx* (as void*) or NULL (missing / not mounted). */
void* OotAssets_LoadGfxDirect(const char* otrPath);

/* 1 if at least one OoT archive (oot.o2r / oot-mq.o2r) is loaded, else 0. */
unsigned char OotAssets_Available(void);

#ifdef __cplusplus
}
#endif

#endif /* OOT_ASSET_LOADER_H */
