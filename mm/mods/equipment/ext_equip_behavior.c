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
#include "behaviors/equip_foursword.c"
#include "behaviors/equip_vanilla_tunic_boots.c" // OoT vanilla tunics/boots effects (Skijer's NEI)

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
    // Ext sword slot 3 is unused: the Iron Knuckle's Axe is now the Hammer upgrade,
    // driven from ExtEquip_UpdateBehavior via WeaponUpgrade_HasHammerAxe().
    (void)player;
    (void)play;
}

// ---------------------------------------------------------------------------
// Shield behaviors (stubs)
// ---------------------------------------------------------------------------
static void ExtEquip_Behavior_Shield1(Player* player, PlayState* play) {
    DivineShield_Behavior(player, play);
}

static void ExtEquip_Behavior_Shield2(Player* player, PlayState* play) {
    (void)player;
    (void)play;
}

static void ExtEquip_Behavior_Shield3(Player* player, PlayState* play) {
    Ikana_Behavior(player, play);
}

// ---------------------------------------------------------------------------
// Tunic behaviors (stubs)
// ---------------------------------------------------------------------------
static void ExtEquip_Behavior_Tunic1(Player* player, PlayState* play) {
    // Tunic slot 1 is RETIRED (Skijer 2026-07-16) — the Magic Cape lives on the kaleido upgrade
    // column: half-cost passive while OWNED (MAGIC_REQ), cloth gated on visibility in the dispatch.
    (void)player;
    (void)play;
}

static void ExtEquip_Behavior_Tunic2(Player* player, PlayState* play) {
    Breastplate_Behavior(player, play);
}

static void ExtEquip_Behavior_Tunic3(Player* player, PlayState* play) {
    Champion_Behavior(player, play);
}

// ---------------------------------------------------------------------------
// Boots behaviors
// ---------------------------------------------------------------------------
static void ExtEquip_Behavior_Boots1(Player* player, PlayState* play) {
    Pegasus_Behavior(player, play);
}

static void ExtEquip_Behavior_Boots2(Player* player, PlayState* play) {
    // Boots slot 2 is RETIRED (Skijer 2026-07-16) — the Pendant of Memories lives on the kaleido
    // upgrade column; its moveset runs when its toggle is ON (see ExtEquip_DispatchBehavior).
    (void)player;
    (void)play;
}

static void ExtEquip_Behavior_Boots3(Player* player, PlayState* play) {
    // Boots slot 3 is FREE (Skijer 2026-07-16) — the Water Dragon Scale is deleted; its Zora swim is
    // now the ZORA TUNIC's permanent effect (Nei_IsZoraSwim gates in z_player.c: fast swim, no drown
    // timer, electric barrier). Slot reserved for Gravity Boots.
    (void)player;
    (void)play;
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
    // Champion's Tunic cleanup: clear forced model + screen tint when slot 3 is lost
    if (gExtEquipState.currentExtTunic != 3) {
        Champion_Cleanup(play);
    }

    if (gExtEquipState.currentExtSword > 0 && gExtEquipState.currentExtSword <= 3) {
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
    // Cane of Byrna: MP recovery on sword hit
    if (gExtEquipState.currentExtSword == 1) {
        Byrna_OnMeleeHit(player, play);
    }
    // Champion's Tunic: count hits during Flurry Rush window
    if (gExtEquipState.currentExtTunic == 3) {
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
}
