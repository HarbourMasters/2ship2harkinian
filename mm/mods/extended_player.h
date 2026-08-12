/**
 * extended_player.h - Extended player item action system
 *
 * Maps custom item IDs (ITEM_xxx) to player actions (PLAYER_IA_xxx).
 * Provides lookup functions for item behavior, model groups, and initialization.
 *
 * Used by: z_player.c, kaleido_scope, item logic files
 */
#ifndef EXTENDED_PLAYER_H
#define EXTENDED_PLAYER_H
#include <stdint.h>
#include <stdbool.h>
#include "z64player.h"
#include "z64item.h"
#ifdef __cplusplus
extern "C" {
#endif

// Vanilla array sizes — MM values (2ship). The old OoT values (56 / 67) cut both tables
// mid-mask-range: transformation masks (items 0x32..0x35, low IAs) resolved fine but every
// normal mask (item >= 0x38 / IA >= 0x43) fell out of the fallback and returned PLAYER_IA_NONE
// or missed its vanilla update-func/model-group — so pressing C with a normal mask did NOTHING.
// Diagnosed via the NEI-DBG UseItem trace (mask items arriving with ia=0x00).
#define VANILLA_SITEMACTIONS_SIZE 87 // MM sItemItemActions entries (z_player.c, items 0x00..0x56 incl. all masks)
#define VANILLA_PLAYER_IA_COUNT 83   // MM PLAYER_IA_MAX (0x53) — vanilla update-func/model-group tables

// Custom item range in ITEM_xxx enum
#define CUSTOM_ITEM_START ITEM_ROCS_FEATHER_SKIJER
#define CUSTOM_ITEM_END ITEM_MM_MASK_FIERCE_DEITY

// Custom PLAYER_IA range — re-based to +0x10 above MM's PLAYER_IA_MAX (0x53) so nothing
// collides with vanilla MM item-actions (0x00-0x52). heldItemAction is s8 → keep <= 0x7F.
// (MM masks + MM-native bottle contents are native in 2ship, not custom items here.)
//
// WARNING: this band (0x63-0x7F) is now COMPLETELY FULL — 29/29 taken. The only room left for a new
// item action is the gap between MM's PLAYER_IA_MAX (0x53) and 0x62, where z64player.h already
// parks PLAYER_IA_SLINGSHOT (0x54), the six magic spells (0x55-0x5A) and PLAYER_IA_BOOMERANG
// (0x62). That is safe for the same documented reason they are: the ExtPlayer_* getters route
// values >= 0x53 by EXACT MATCH and never index the [PLAYER_IA_MAX]-sized native tables. Free
// there today: 0x5D-0x61. Skijer's NEI
#define CUSTOM_PLAYER_IA_START 0x63
#define CUSTOM_PLAYER_IA_END 0x7F

// Elemental Wand (Skijer's NEI). ONE item action for all six rods — the active rod is
// NeiSaveData.wandMode, read by the behavior when it lands. 0x5C sits in the pre-band gap described
// above because 0x63-0x7F had nothing left. Per-rod behavior is a separate task; the init below is
// a stub so the item equips, draws and cycles today.
#define PLAYER_IA_ELEMENTAL_WAND 0x5C

// Page-2 custom item PLAYER_IA values (0x63-0x7C, unique). Keys into sNeiItems[] via ExtPlayer.
#define PLAYER_IA_ROCS_FEATHER_SKIJER 0x63
#define PLAYER_IA_ROCS_CAPE 0x64
#define PLAYER_IA_DESIRE_SENSOR 0x65
#define PLAYER_IA_HYLIAS_GRACE 0x66
#define PLAYER_IA_ZONAI_PERMAFROST 0x67
#define PLAYER_IA_DEMISE_DESTRUCTION 0x68
#define PLAYER_IA_DEKU_LEAF 0x69
#define PLAYER_IA_SWITCH_HOOK 0x6A
#define PLAYER_IA_MOGMA_MITTS 0x6B
#define PLAYER_IA_GUST_JAR 0x6C
#define PLAYER_IA_BALL_AND_CHAIN 0x6D
#define PLAYER_IA_WHIP 0x6E
#define PLAYER_IA_SPINNER 0x6F
#define PLAYER_IA_CANE_OF_SOMARIA 0x70
#define PLAYER_IA_DOMINION_ROD 0x71
#define PLAYER_IA_TIME_GATE 0x72
#define PLAYER_IA_BOMB_ARROWS 0x73
#define PLAYER_IA_ROD_FIRE 0x74
#define PLAYER_IA_ROD_ICE 0x75
#define PLAYER_IA_ROD_LIGHT 0x76
#define PLAYER_IA_BEETLE 0x77
#define PLAYER_IA_SHOVEL 0x78
#define PLAYER_IA_MINISH_CAP 0x79
#define PLAYER_IA_LANTERN 0x7A
#define PLAYER_IA_POKEBALL 0x7B
#define PLAYER_IA_UNUSED_5B 0x7C

// PLAYER_MODELGROUP_BGS (OoT Biggoron-sword hold) → MM has none; use the default group.
#ifndef PLAYER_MODELGROUP_BGS
#define PLAYER_MODELGROUP_BGS PLAYER_MODELGROUP_DEFAULT
#endif

// Genuinely-custom bottle items — Magic Mushroom bottle + Net + Bottomless Bottle.
// (MM masks + MM-native bottle contents removed: they're native in 2ship, not custom.)
#define PLAYER_IA_BOTTLE_MAGIC_MUSHROOM 0x7D
#define PLAYER_IA_NET 0x7E
#define PLAYER_IA_BOTTOMLESS_BOTTLE 0x7F

// ============================================================================
// FUNCTION POINTER TYPES
// ============================================================================
// Guard shared with extended_inventory.h (NeiItem) so neither header redefines
// these typedefs when both are included in one TU. Skijer's NEI
#ifndef NEI_ITEM_ACTION_FUNC_TYPES
#define NEI_ITEM_ACTION_FUNC_TYPES
struct Player;
struct PlayState;
typedef int32_t (*ItemActionUpdateFunc)(struct Player* player, struct PlayState* play);
typedef void (*ItemActionInitFunc)(struct PlayState* play, struct Player* player);
#endif

// ============================================================================
// HELPER FUNCTIONS - Use these instead of directly accessing arrays
// ============================================================================

/**
 * Get the PLAYER_IA_xxx value for a given ITEM_xxx value.
 * Handles both vanilla and custom items using switch for custom items.
 */
int8_t ExtPlayer_GetItemAction(int32_t item);

/**
 * Get the model group for a given PLAYER_IA_xxx value.
 * Handles both vanilla and custom item actions.
 */
uint8_t ExtPlayer_GetActionModelGroup(int32_t itemAction);

/**
 * Get the update function for a given PLAYER_IA_xxx value.
 * Handles both vanilla and custom item actions.
 */
ItemActionUpdateFunc ExtPlayer_GetItemActionUpdateFunc(int32_t itemAction);

/**
 * Get the init function for a given PLAYER_IA_xxx value.
 * Handles both vanilla and custom item actions.
 */
ItemActionInitFunc ExtPlayer_GetItemActionInitFunc(int32_t itemAction);

/**
 * Check if an item ID is a custom item.
 */
static inline bool ExtPlayer_IsCustomItem(int32_t item) {
    return (item >= CUSTOM_ITEM_START && item <= CUSTOM_ITEM_END);
}

/**
 * Check if an item ID is an MM mask item.
 */
static inline bool ExtPlayer_IsMmMaskItem(int32_t item) {
    return (item >= ITEM_MM_MASK_POSTMAN && item <= ITEM_MM_MASK_FIERCE_DEITY);
}

/**
 * Check if a PLAYER_IA value is a custom action.
 */
static inline bool ExtPlayer_IsCustomItemAction(int32_t itemAction) {
    return (itemAction >= CUSTOM_PLAYER_IA_START && itemAction <= CUSTOM_PLAYER_IA_END);
}

#ifdef __cplusplus
}
#endif
#endif // EXTENDED_PLAYER_H
