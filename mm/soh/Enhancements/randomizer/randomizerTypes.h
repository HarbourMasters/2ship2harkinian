/* soh/Enhancements/randomizer/randomizerTypes.h shim.
 * 2ship's randomizer (2s2h/Rando) is a separate, incompatible system. This keeps
 * the include resolvable; RG_x / RandomizerGet / RandomizerCheck usages are ported
 * standalone or cut (the get-item draw funcs are decoupled from rando). The exact
 * undefined symbols surface at build time. */
#ifndef NEI_SHIM_RANDOMIZERTYPES_H
#define NEI_SHIM_RANDOMIZERTYPES_H
#include "soh/_nei_compat_core.h"

// Placeholder RandomizerGet IDs for the NEI custom items (Skijer's NEI). 2ship's rando
// (2s2h/Rando) is a separate system; these are distinct stub IDs so sNeiItems[] compiles.
// TODO: map to real 2s2h RandoItemId when the custom items are added to the randomizer
// (with their get-item DLs), like OoT.
enum {
    RG_NEI_STUB_BASE = 0x4000,
    RG_ROCS_CAPE,
    RG_DESIRE_SENSOR,
    RG_HYLIAS_GRACE,
    RG_ZONAI_PERMAFROST,
    RG_DEMISE_DESTRUCTION,
    RG_DEKU_LEAF,
    RG_SWITCH_HOOK,
    RG_MOGMA_MITTS,
    RG_GUST_JAR,
    RG_BALL_AND_CHAIN,
    RG_WHIP,
    RG_SPINNER,
    RG_CANE_OF_SOMARIA,
    RG_DOMINION_ROD,
    RG_TIME_GATE,
    RG_BOMB_ARROWS,
    RG_FIRE_ROD,
    RG_ICE_ROD,
    RG_LIGHT_ROD,
    RG_BEETLE,
    RG_SHOVEL,
    RG_MINISH_CAP,
    RG_LANTERN,
    RG_POKEBALL,
    RG_BOTTLE_WITH_MAGIC_MUSHROOM,
};
#endif
