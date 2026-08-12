/* soh/Enhancements/randomizer/randomizerTypes.h shim.
 * 2ship's randomizer (2s2h/Rando) is a separate, incompatible system. This keeps
 * the include resolvable; RG_x / RandomizerGet / RandomizerCheck usages are ported
 * standalone or cut (the get-item draw funcs are decoupled from rando). The exact
 * undefined symbols surface at build time. */
#ifndef NEI_SHIM_RANDOMIZERTYPES_H
#define NEI_SHIM_RANDOMIZERTYPES_H
#include "soh/_nei_compat_core.h"

// The `rg` column of sNeiItems[] (mods/extended_player.c). Skijer's NEI
//
// This used to be a block of made-up ids starting at 0x4000 ("RG_NEI_STUB_BASE") whose only job was
// to make the ported rows compile — they named nothing, and MM's Nei_FindByRg() had no callers, so
// nobody ever noticed they were fiction. They are now aliases of the REAL 2ship placement ids
// (RandoItemId, RI_OOT_NEI_*), which is what SoH's side of the same table carries (real RG_* there).
// Consequences:
//   - Nei_FindByRg(RI_OOT_NEI_WHIP) now actually finds the Whip row in MM, same as in OoT.
//   - The column can be trusted by anything reading it later (spoiler, logs, the shared table).
// Rando/Types.h is plain C (no includes, no C++), so it is safe from this header even though
// extended_player.c is compiled inside z_player.c's unity translation unit.
#include "2s2h/Rando/Types.h"

#define RG_ROCS_CAPE RI_OOT_PROGRESSIVE_ROC // level 2 of the Roc chain; the feather row is NEI_NO_RG
#define RG_DESIRE_SENSOR RI_OOT_NEI_DESIRE_SENSOR
#define RG_HYLIAS_GRACE RI_OOT_NEI_HYLIAS_GRACE
#define RG_ZONAI_PERMAFROST RI_OOT_NEI_ZONAI_PERMAFROST
#define RG_DEMISE_DESTRUCTION RI_OOT_NEI_DEMISE_DESTRUCTION
#define RG_DEKU_LEAF RI_OOT_NEI_DEKU_LEAF
#define RG_SWITCH_HOOK RI_OOT_NEI_SWITCH_HOOK
#define RG_MOGMA_MITTS RI_OOT_NEI_MOGMA_MITTS
#define RG_GUST_JAR RI_OOT_NEI_GUST_JAR
#define RG_BALL_AND_CHAIN RI_OOT_NEI_BALL_AND_CHAIN
#define RG_WHIP RI_OOT_NEI_WHIP
#define RG_SPINNER RI_OOT_NEI_SPINNER
#define RG_CANE_OF_SOMARIA RI_OOT_NEI_CANE_OF_SOMARIA
#define RG_DOMINION_ROD RI_OOT_NEI_DOMINION_ROD
#define RG_TIME_GATE RI_OOT_NEI_TIME_GATE
#define RG_BOMB_ARROWS RI_OOT_NEI_BOMB_ARROWS
#define RG_FIRE_ROD RI_OOT_NEI_FIRE_ROD
#define RG_ICE_ROD RI_OOT_NEI_ICE_ROD
#define RG_LIGHT_ROD RI_OOT_NEI_LIGHT_ROD
#define RG_BEETLE RI_OOT_NEI_BEETLE
#define RG_SHOVEL RI_OOT_NEI_SHOVEL
#define RG_MINISH_CAP RI_OOT_NEI_MINISH_CAP
#define RG_LANTERN RI_OOT_NEI_LANTERN
#define RG_POKEBALL RI_OOT_NEI_POKE_BALL
#define RG_BOTTLE_WITH_MAGIC_MUSHROOM RI_OOT_BOTTLE_MAGIC_MUSHROOM
#define RG_ELEMENTAL_WAND RI_OOT_NEI_ELEMENTAL_WAND

#endif
