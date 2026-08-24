/**
 * ext_equip_behavior.c - Behavior handlers for extended equipment
 *
 * Unity build hub: includes individual behavior files and dispatches
 * update/draw/hit callbacks to active equipment.
 *
 * Included by extended_equipment.c (unity build).
 */

// No extra includes — inherits all from extended_equipment.c (unity build root)
// Somaria cane DL header included by extended_equipment.c (unity root)

// OoT→MM state-flag compat. The item TU (z_player.c) gets these from mods/nei_oot_compat.h, but
// this equipment TU (extended_equipment.c) doesn't include it, and the ported behavior files
// (equip_ikaxe.c, byrna, breastplate, …) plus items/helpers/equip_helper.h reference the OoT flag
// names. Same MM-bit mapping sm64_mario.c / nei_oot_compat.h already established; #ifndef-guarded
// so this no-ops if a shared header defines them.
#ifndef PLAYER_STATE1_IN_CUTSCENE
#define PLAYER_STATE1_IN_CUTSCENE 0u
#endif
#ifndef PLAYER_STATE1_IN_ITEM_CS
#define PLAYER_STATE1_IN_ITEM_CS 0u
#endif
#ifndef PLAYER_STATE1_LOADING
#define PLAYER_STATE1_LOADING PLAYER_STATE1_200 // bit 9 — scene load / picto setup
#endif
#ifndef PLAYER_STATE1_GETTING_ITEM
#define PLAYER_STATE1_GETTING_ITEM PLAYER_STATE1_400 // bit 10
#endif
#ifndef PLAYER_STATE1_DAMAGED
#define PLAYER_STATE1_DAMAGED PLAYER_STATE1_4000000 // bit 26 — damage knockback
#endif
#ifndef PLAYER_STATE1_HANGING_OFF_LEDGE
#define PLAYER_STATE1_HANGING_OFF_LEDGE 0u
#endif
#ifndef PLAYER_STATE1_CLIMBING_LEDGE
#define PLAYER_STATE1_CLIMBING_LEDGE PLAYER_STATE1_4 // MM bit 2 = "Climbing ledge"
#endif
#ifndef PLAYER_STATE1_CLIMBING_LADDER
#define PLAYER_STATE1_CLIMBING_LADDER PLAYER_STATE1_200000
#endif
#ifndef PLAYER_STATE1_ON_HORSE
#define PLAYER_STATE1_ON_HORSE PLAYER_STATE1_800000
#endif
#ifndef PLAYER_STATE1_HOOKSHOT_FALLING
#define PLAYER_STATE1_HOOKSHOT_FALLING 0u
#endif

// ---------------------------------------------------------------------------
// Include behavior implementations
// ---------------------------------------------------------------------------
#include "behaviors/equip_byrna.c"
#include "behaviors/equip_ikaxe.c"
#include "behaviors/equip_pegasus.c"
#include "behaviors/equip_dragonscale.c"
#include "behaviors/equip_ikana.c"
#include "behaviors/equip_magiccape.c"
#include "behaviors/equip_breastplate.c"
#include "behaviors/equip_pendant.c"
#include "behaviors/equip_divine_shield.c"
#include "behaviors/equip_champion.c"
#include "behaviors/equip_sages_tunic.c"
#include "behaviors/equip_foursword.c"
#include "behaviors/equip_vanilla_tunic_boots.c"
// Skijer 2026-07-29 kaleido re-layout: the four slots that changed hands.
#include "behaviors/equip_trident.c"     // sword 3 (was the Iron Knuckle's Axe, now the Hammer upgrade)
#include "behaviors/equip_kite_shield.c" // shield 2 (was the Gerudo Scimitar placeholder)
#include "behaviors/equip_climb_boots.c" // boots 2 (was the Pendant of Memories)
#include "behaviors/equip_roc_boots.c"   // boots 3 (was the deleted Water Dragon Scale)

// ---------------------------------------------------------------------------
// Sword behaviors
// ---------------------------------------------------------------------------
static void ExtEquip_Behavior_Sword1(Player* player, PlayState* play) {
    Byrna_Behavior(player, play);
}

static void ExtEquip_Behavior_Sword2(Player* player, PlayState* play) {
    FourSword_Behavior(player, play);
}

static void ExtEquip_Behavior_Sword3(Player* player, PlayState* play) {
    // The Iron Knuckle's Axe left this slot for good — it is the HAMMER UPGRADE now, driven from
    // ExtEquip_UpdateBehavior via WeaponUpgrade_HasHammerAxe(). The slot holds the TRIDENT.
    Trident_Behavior(player, play);
}

// ---------------------------------------------------------------------------
// Shield behaviors (stubs)
// ---------------------------------------------------------------------------
static void ExtEquip_Behavior_Shield1(Player* player, PlayState* play) {
    DivineShield_Behavior(player, play);
}

static void ExtEquip_Behavior_Shield2(Player* player, PlayState* play) {
    KiteShield_Behavior(player, play);
}

static void ExtEquip_Behavior_Shield3(Player* player, PlayState* play) {
    Ikana_Behavior(player, play);
}

// ---------------------------------------------------------------------------
// Tunic behaviors (stubs)
// ---------------------------------------------------------------------------
static void ExtEquip_Behavior_Tunic1(Player* player, PlayState* play) {
    Champion_Behavior(player, play);
}

static void ExtEquip_Behavior_Tunic2(Player* player, PlayState* play) {
    Breastplate_Behavior(player, play); // MAGIC TUNIC: rupee-paid damage immunity + fire/water timer skip
}

static void ExtEquip_Behavior_Tunic3(Player* player, PlayState* play) {
    Sages_Behavior(player, play);
}

// ---------------------------------------------------------------------------
// Boots behaviors
// ---------------------------------------------------------------------------
static void ExtEquip_Behavior_Boots1(Player* player, PlayState* play) {
    Pegasus_Behavior(player, play);
}

static void ExtEquip_Behavior_Boots2(Player* player, PlayState* play) {
    // The Pendant of Memories keeps its left-column cell (ownership = the adult trade wheel) and its
    // moveset is dispatched cheat-independently; this GRID slot is the CLIMB BOOTS.
    ClimbBoots_Behavior(player, play);
}

static void ExtEquip_Behavior_Boots3(Player* player, PlayState* play) {
    // The Water Dragon Scale is deleted (its Zora swim is the Zora Tunic's permanent effect); this
    // slot is the ROC BOOTS.
    RocBoots_Behavior(player, play);
}

// ---------------------------------------------------------------------------
// Behavior dispatch tables
// ---------------------------------------------------------------------------
typedef void (*ExtEquipBehaviorFunc)(Player*, PlayState*);

static const ExtEquipBehaviorFunc sExtSwordBehaviors[3] = {
    ExtEquip_Behavior_Sword1,
    ExtEquip_Behavior_Sword2,
    ExtEquip_Behavior_Sword3,
};

static const ExtEquipBehaviorFunc sExtShieldBehaviors[3] = {
    ExtEquip_Behavior_Shield1,
    ExtEquip_Behavior_Shield2,
    ExtEquip_Behavior_Shield3,
};

static const ExtEquipBehaviorFunc sExtTunicBehaviors[3] = {
    ExtEquip_Behavior_Tunic1,
    ExtEquip_Behavior_Tunic2,
    ExtEquip_Behavior_Tunic3,
};

static const ExtEquipBehaviorFunc sExtBootsBehaviors[3] = {
    ExtEquip_Behavior_Boots1,
    ExtEquip_Behavior_Boots2,
    ExtEquip_Behavior_Boots3,
};

static void ExtEquip_DispatchBehavior(Player* player, PlayState* play) {
    // NOTE (Skijer 2026-07-16): the Cape/Pendant upgrade-column passives do NOT live here — this
    // dispatch only runs with the ext-equipment cheat ON. They run cheat-independent from
    // ExtEquip_UpdateBehavior (ownership-based), next to VanillaTB_Behavior.

    // Byrna cleanup: restore original sword when Byrna is no longer active
    if (gExtEquipState.currentExtSword != 1) {
        Byrna_Cleanup();
    }
    // Pegasus cleanup: disable collider when Pegasus boots are no longer active
    if (gExtEquipState.currentExtBoots != 1) {
        Pegasus_Cleanup();
    }
    // Four Sword cleanup: clear forced equipment when sword slot 2 is no longer active
    if (gExtEquipState.currentExtSword != 2) {
        FourSword_Cleanup();
    }
    // New-slot cleanups (Skijer 2026-07-29)
    if (gExtEquipState.currentExtSword != 3) {
        Trident_Cleanup();
    }
    if (gExtEquipState.currentExtShield != 2) {
        KiteShield_Cleanup();
    }
    if (gExtEquipState.currentExtBoots != 2) {
        ClimbBoots_Cleanup();
    }
    if (gExtEquipState.currentExtBoots != 3) {
        RocBoots_Cleanup();
    }
    // Champion's Tunic cleanup: release slow motion and clear its screen tint
    if (gExtEquipState.currentExtTunic != 1) {
        Champion_Cleanup(play);
    }

    // Trident reads current-frame input and owns player actions, so it runs late.
    if (gExtEquipState.currentExtSword > 0 && gExtEquipState.currentExtSword < 3) {
        sExtSwordBehaviors[gExtEquipState.currentExtSword - 1](player, play);
    }
    if (gExtEquipState.currentExtShield > 0 && gExtEquipState.currentExtShield <= 3) {
        sExtShieldBehaviors[gExtEquipState.currentExtShield - 1](player, play);
    }
    if (gExtEquipState.currentExtTunic > 0 && gExtEquipState.currentExtTunic <= 3) {
        sExtTunicBehaviors[gExtEquipState.currentExtTunic - 1](player, play);
    }
    if (gExtEquipState.currentExtBoots > 0 && gExtEquipState.currentExtBoots <= 3) {
        sExtBootsBehaviors[gExtEquipState.currentExtBoots - 1](player, play);
    }
}

// ---------------------------------------------------------------------------
// Melee hit dispatch (called from z_player.c)
// ---------------------------------------------------------------------------
static void ExtEquip_OnMeleeHitDispatch(Player* player, PlayState* play) {
    // (The Cane of Byrna is a dummy slot now — its HP/MP-on-hit recovery belongs to the Great Fairy's
    // Sword, dispatched from ExtEquip_UpdateBehavior.)
    // Trident
    if (gExtEquipState.currentExtSword == 3) {
        Trident_OnMeleeHit(player, play);
    }
    // Champion's Tunic: count hits during Flurry Rush window
    if (gExtEquipState.currentExtTunic == 1) {
        Champion_OnMeleeHit(player, play);
    }
}

// ---------------------------------------------------------------------------
// Draw dispatch (called from z_player.c draw section)
// ---------------------------------------------------------------------------
static void ExtEquip_DrawDispatch(Player* player, PlayState* play) {
    // Cane of Byrna: drawn from PostLimbDraw via ExtEquip_DrawSwordDL (follows limb matrix)
    // Pegasus Anklet: wind barrier
    if (gExtEquipState.currentExtBoots == 1) {
        Pegasus_Draw(player, play);
    }
    // Water Dragon Scale draw removed — item deleted (Zora swim = Zora Tunic effect).
    // Magic Cape cloth draw moved to ExtEquip_Draw (cheat-independent, ownership-based).
    // Four Sword: ghost clone Links
    if (gExtEquipState.currentExtSword == 2) {
        FourSword_Draw(player, play);
    }
    if (gExtEquipState.currentExtSword == 3) {
        Trident_Draw(player, play);
    }
}
