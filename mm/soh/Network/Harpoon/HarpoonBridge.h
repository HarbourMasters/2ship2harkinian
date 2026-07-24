/* soh/Network/Harpoon/HarpoonBridge.h shim → 2ship's Harpoon. 2ship spells the
 * header Harpoon.h (no "Bridge"); if the API name differs, adapt at the call site. */
#ifndef NEI_SHIM_HARPOONBRIDGE_H
#define NEI_SHIM_HARPOONBRIDGE_H
#include "soh/_nei_compat_core.h"
#include "2s2h/Network/Harpoon/Harpoon.h"

/* --- SW97 spell-VFX teammate notify ---
 * 2ship's Harpoon module doesn't expose a VFX-spawn sync API yet, so stub the kind enum
 * + the notify here (the shim is the designated adaptation point). SW97 spells still cast
 * locally; teammates won't see them until this is wired to a real Harpoon VFX pipeline. TODO. */
#ifndef HARPOON_VFX_KIND_SW97_MAGIC_WIND
enum {
    HARPOON_VFX_KIND_SW97_MAGIC_WIND,
    HARPOON_VFX_KIND_SW97_MAGIC_SOUL,
    HARPOON_VFX_KIND_SW97_MAGIC_DARK,
    HARPOON_VFX_KIND_SW97_MAGIC_ICE,
    HARPOON_VFX_KIND_SW97_MAGIC_LIGHT,
    HARPOON_VFX_KIND_SW97_MAGIC_FIRE
};
static inline void Harpoon_NotifyVfxSpawn(void* spawnedActor, int vfxKind, int attachedToOwner) {
    (void)spawnedActor;
    (void)vfxKind;
    (void)attachedToOwner;
}
#endif

#endif
