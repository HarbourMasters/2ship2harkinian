/**
 * extended_equipment.h - Extended equipment system (cheat)
 *
 * Adds 12 new equipment pieces (3 swords, 3 shields, 3 tunics, 3 boots)
 * accessible via L button on the pause menu equipment page.
 * All extended equipment is "owned" when the cheat CVar is enabled.
 *
 * Page switching: Press L on equipment screen to toggle vanilla/extended.
 */
#ifndef EXTENDED_EQUIPMENT_H
#define EXTENDED_EQUIPMENT_H

#include <libultraship/libultra.h>
#include "z64item.h"
#include "color.h" // Color_RGBA8 (EquipDrawModel prim/env)

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------------
// CVar keys
// ---------------------------------------------------------------------------
#define CVAR_EXT_EQUIP_ENABLED "gCheats.ExtEquip.Enabled"
// Extended equipment ownership bits in upper 16 of inventory.equipment
// Bit = 16 + equipType*3 + (index-1)
#define EXT_EQUIP_OWNED_SHIFT 16

// ---------------------------------------------------------------------------
// Extended equipment item IDs (for icon/name lookup, NOT stored in inventory)
// ---------------------------------------------------------------------------
#define ITEM_EXT_SWORD_1 0xE0
#define ITEM_EXT_SWORD_2 0xE1
#define ITEM_EXT_SWORD_3 0xE2
#define ITEM_EXT_SHIELD_1 0xE3
#define ITEM_EXT_SHIELD_2 0xE4
#define ITEM_EXT_SHIELD_3 0xE5
#define ITEM_EXT_TUNIC_1 0xE6
#define ITEM_EXT_TUNIC_2 0xE7
#define ITEM_EXT_TUNIC_3 0xE8
#define ITEM_EXT_BOOTS_1 0xE9
#define ITEM_EXT_BOOTS_2 0xEA
#define ITEM_EXT_BOOTS_3 0xEB

// ---------------------------------------------------------------------------
// Extended equipment indices (1-based, 0 = none)
// ---------------------------------------------------------------------------
typedef enum { EXT_EQUIP_NONE = 0, EXT_EQUIP_1 = 1, EXT_EQUIP_2 = 2, EXT_EQUIP_3 = 3, EXT_EQUIP_MAX = 4 } ExtEquipIndex;

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------
typedef struct {
    int equipPage;       // 0 = vanilla, 1 = extended
    s16 pageSwitchTimer; // Cooldown (15 frames)
    u8 currentExtSword;  // 0=none, 1-3=ext sword
    u8 currentExtShield; // 0=none, 1-3=ext shield
    u8 currentExtTunic;  // 0=none, 1-3=ext tunic
    u8 currentExtBoots;  // 0=none, 1-3=ext boots
} ExtendedEquipmentState;

extern ExtendedEquipmentState gExtEquipState;

// ---------------------------------------------------------------------------
// Page management
// ---------------------------------------------------------------------------

/** Initialize state from CVars */
void ExtEquip_Init(void);

/** Update per frame (cooldown timer) */
void ExtEquip_Update(void);

/** @return Current equipment page (0=vanilla, 1=extended) */
int ExtEquip_GetPage(void);

// ---------------------------------------------------------------------------
// Age requirements (per ext equipment piece)
// ---------------------------------------------------------------------------

/** @return Age requirement value (AGE_REQ_NONE=9, AGE_REQ_ADULT=0, AGE_REQ_CHILD=1) */
u8 ExtEquip_GetAgeReq(s16 equipType, u8 index);

/** @return 1 if Link's current age satisfies the requirement, 0 otherwise */
u8 ExtEquip_CheckAgeReq(s16 equipType, u8 index);

/** Toggle between vanilla and extended page */
void ExtEquip_SwitchPage(void);

/** @return true if page switch cooldown elapsed */
u8 ExtEquip_CanSwitch(void);

/** @return true if the extra equipment cheat is enabled */
u8 ExtEquip_IsEnabled(void);

// ---------------------------------------------------------------------------
// Equip / Unequip
// ---------------------------------------------------------------------------

/**
 * Equip an extended equipment piece.
 * @param equipType EQUIP_TYPE_SWORD/SHIELD/TUNIC/BOOTS
 * @param index     1-3 (ext equipment index)
 */
void ExtEquip_Equip(s16 equipType, u8 index);

/**
 * Unequip extended equipment of a given type (set to 0).
 * Called when vanilla equipment is equipped.
 * @param equipType EQUIP_TYPE_SWORD/SHIELD/TUNIC/BOOTS
 */
void ExtEquip_Unequip(s16 equipType);

/**
 * @param equipType EQUIP_TYPE_SWORD/SHIELD/TUNIC/BOOTS
 * @return Current extended equipment index (0=none, 1-3=equipped)
 */
u8 ExtEquip_GetCurrent(s16 equipType);

// ---------------------------------------------------------------------------
// Magic Cape — upgrade-column PASSIVE (Skijer 2026-07-16, OoT-rework mirror): halves the magic cost
// (integer division = ROUND DOWN, so 1-MP items become FREE) whenever the cape is OWNED — it no
// longer needs to be "equipped" (the ext tunic slot is retired). Wrap any magic cost with
// MAGIC_REQ(); the ItemMagic_* helpers apply it centrally, direct reads (Four Sword, OoT spells,
// Deku Leaf, magic arrows) wrap the cost themselves. The kaleido A-toggle only controls the cape
// CLOTH visibility; this passive is always on while owned.
// ---------------------------------------------------------------------------
#define IS_MAGIC_CAPE_ACTIVE (ExtEquip_CapeOwned())
#define MAGIC_REQ(cost) (IS_MAGIC_CAPE_ACTIVE ? ((cost) / 2) : (cost))

// No ext slot is retired anymore (all 12 are live after the 2026-07-29 re-layout); kept so the
// kaleido / save-editor call sites stay put if a slot is ever parked again.
u8 ExtEquip_SlotRetired(s16 equipType, u8 index);

// Left-column passives (equipment page rows 0/1) — ownership lives in nei save fields, NOT ext bits:
u8 ExtEquip_CapeOwned(void);
void ExtEquip_GiveCape(void);
u8 ExtEquip_CapeVisible(void);          // owned && not hidden (draw the cloth)
void ExtEquip_ToggleCapeVisibility(void);
u8 ExtEquip_PendantOwned(void);         // owns the Pendant as EQUIPMENT (permanent once granted)
void ExtEquip_GivePendant(void);        // grant it (the adult trade slot does this automatically)
u8 ExtEquip_PendantActive(void);        // owned && effect toggle ON (the moveset gate)
void ExtEquip_TogglePendantEffect(void);
void* ExtEquip_GetCapeIcon(void);    // left-column icons (the pieces are not in the grid anymore)
void* ExtEquip_GetPendantIcon(void);

typedef enum {
    SAGES_RESIST_ICE,
    SAGES_RESIST_FIRE,
    SAGES_RESIST_THUNDER,
    SAGES_RESIST_STUN,
    SAGES_RESIST_FALL,
    SAGES_RESIST_WIND,
} SagesResistance;
u8 ExtEquip_IsChampionTunic(void);
u8 ExtEquip_IsSpiritTunic(void);
u8 ExtEquip_SpiritHasMoney(void); // Magic Tunic equipped AND rupees > 0 (its whole effect is gated on this)
u8 ExtEquip_IsSagesTunic(void);
u8 ExtEquip_HasSagesResistance(SagesResistance resistance);
void ExtEquip_SagesFlash(SagesResistance resistance); // a resistance just absorbed damage
void ExtEquip_SagesFlashTick(void);                   // per-frame decay (Sages_Behavior)
void ExtEquip_GetSagesTunicColor(u8* r, u8* g, u8* b);

// ---------------------------------------------------------------------------
// Ownership
// ---------------------------------------------------------------------------

/** @return true if the player owns this extended equipment piece */
u8 ExtEquip_HasItem(s16 equipType, u8 index);

/** Give the player an extended equipment piece */
void ExtEquip_GiveItem(s16 equipType, u8 index);

/** Remove an extended equipment piece from the player */
void ExtEquip_RemoveItem(s16 equipType, u8 index);

// ---------------------------------------------------------------------------
// Icons / Names
// ---------------------------------------------------------------------------

/**
 * Get icon texture for an extended equipment item.
 * @param equipType EQUIP_TYPE_SWORD/SHIELD/TUNIC/BOOTS
 * @param index     1-3
 * @return Pointer to 32x32 RGBA32 texture data
 */
void* ExtEquip_GetIcon(s16 equipType, u8 index);

/**
 * Get the extended item ID for a given equipment type and index.
 * @param equipType EQUIP_TYPE_SWORD/SHIELD/TUNIC/BOOTS
 * @param index     1-3
 * @return ITEM_EXT_xxx constant
 */
u16 ExtEquip_GetItemId(s16 equipType, u8 index);

/**
 * Toggle an extended equipment item from a C button press.
 * If the item's equipment type is already equipped with this index, unequip it.
 * Otherwise, equip it.
 * @param itemId ITEM_EXT_xxx constant (0xE0-0xEB)
 */
void ExtEquip_ToggleFromCButton(u16 itemId);

/**
 * Get name texture for an extended equipment item.
 * @param itemId ITEM_EXT_xxx constant
 * @param language Language index
 * @return Pointer to name texture, or NULL for placeholder
 */
void* ExtEquip_GetNameTex(u16 itemId, u8 language);

// ---------------------------------------------------------------------------
// Transform integration
// ---------------------------------------------------------------------------

/** Backup current ext equip state and unequip all. Called on transformation. */
void ExtEquip_UnequipForTransform(void);

/** Restore ext equip from backup. Called on detransformation to human. */
void ExtEquip_RestoreFromTransform(void);

/** Discard backup without restoring. Called on reset/reload/death. */
void ExtEquip_ClearTransformBackup(void);

// ---------------------------------------------------------------------------
// Divine Shield helpers (called from z_player_lib.c and z_player.c)
// ---------------------------------------------------------------------------
u8 DivineShield_IsWoodType(void);
u8 DivineShield_IsFireproof(void);
void DivineShield_OnShieldBlock(Player* player, PlayState* play);

// ---------------------------------------------------------------------------
// Shield of Ikana helpers (called from z_player.c at the bounce-detection point)
// ---------------------------------------------------------------------------
void Ikana_OnShieldBlock(Player* player, PlayState* play);

// ---------------------------------------------------------------------------
// Behavior state
// ---------------------------------------------------------------------------

typedef enum {
    PEGASUS_IDLE,
    PEGASUS_WINDUP,
    PEGASUS_RUNNING,
    PEGASUS_BONK,
} PegasusState;

typedef enum {
    DSCALE_INACTIVE,
    DSCALE_SWIMMING,
} DragonScaleState;

typedef struct {
    Vec3f offset; // relative offset from player world pos (set at spawn, Y = 0 keeps same ground height)
    u8 alive;     // 1 = active, 0 = dead / not spawned
} FourSwordClone;

typedef struct {
    // Cane of Byrna (Ext Sword 1)
    u8 byrnaSavedSwordEquip; // Original equips.equipment sword nibble
    u8 byrnaSavedButtonItem; // Original equips.buttonItems[0]
    f32 byrnaSavedSwordHealth; // Original swordHealth (GK durability)
    u8 byrnaSavedBgsFlag;      // Original bgsFlag (1=BGS, 0=GK)
    u8 byrnaActive;          // Whether Byrna has overridden sword state

    // Pegasus Anklet
    u8 pegasusState;
    s16 pegasusTimer;
    s16 pegasusMagicTick;
    u8 pegasusColInit;
    f32 pegasusWingAngle; // Pendulum angle for wing charm (radians)
    f32 pegasusWingVel;   // Pendulum angular velocity

    // Water Dragon Scale
    u8 dragonScaleState;
    s16 dragonScalePitch; // swim pitch angle
    s16 dragonScaleMagicTick;
    u8 dragonScaleColInit;

    // Iron Knuckle Axe (Ext Sword 3)
    u8 ikAxeSavedSwordEquip;
    u8 ikAxeSavedButtonItem;
    u8 ikAxeActive;
    u8 ikAxeDrawing; // 1 when hammer is out (hide vanilla sword DL), 0 in free mode

    // Four Sword (Ext Sword 2)
    u8 fourSwordSavedSwordEquip; // Original equips.equipment sword nibble
    u8 fourSwordSavedButtonItem; // Original equips.buttonItems[0]
    u8 fourSwordActive;                // pak loader is live
    s16 fourSwordBHoldTimer;           // frames B has been held while shielding
    u8 fourSwordCharging;              // 1 while charge is armed (B+shield >= threshold)
    u8 fourSwordCloneCount;            // number of currently alive clones (0-3)
    FourSwordClone fourSwordClones[3]; // per-clone data
    u8 fourSwordColInit;               // bitmask: bit i = colliders for clone i are initialised

    // Four Sword: rising-edge detection for Ivan-style item spawn
    u8 fourSwordPrevA73;       // previous player->unk_A73 (arrow/boomerang fire)
    u8 fourSwordPrevCarrying;  // previous PLAYER_STATE1_CARRYING_ACTOR bit
    u8 fourSwordPrevBoomerang; // previous (player->boomerangActor != NULL)
    s16 fourSwordItemCooldown; // global cooldown prevents actor spam (10 frames)
} ExtEquipBehaviorState;

extern ExtEquipBehaviorState gExtEquipBehavior;

// Champion's Tunic slow factor — 1.0f normal, 0.33f during Flurry Rush / Bullet Time
// Used in z_actor.c Actor_UpdatePos to scale non-player actor movement.
extern f32 gChampionSlowFactor;

// ---------------------------------------------------------------------------
// Behavior
// ---------------------------------------------------------------------------

/**
 * Called per frame from Player_Update when extended equipment is active.
 * Dispatches to individual behavior handlers.
 */
void ExtEquip_UpdateBehavior(void* player, void* play);

/**
 * Called from z_player.c when melee weapon quads register a hit (AT_HIT).
 * Used by Cane of Byrna for MP recovery.
 */
void ExtEquip_OnMeleeHit(void* player, void* play);

/**
 * Called from z_player.c draw section for equipment-specific visuals
 * (barriers, auras, etc.).
 */
void ExtEquip_DrawBehavior(void* player, void* play);

/**
 * Returns 1 if the vanilla sword DL should be hidden (replaced by ext equipment draw).
 * Called from z_player_lib.c in the limb draw callback.
 */
u8 ExtEquip_ShouldHideSwordDL(void);

/**
 * Returns the MM Mirror Shield OTR path if Shield of Ikana is equipped, NULL otherwise.
 * Called from z_player_lib.c to override shield DL.
 */
const char* ExtEquip_GetShieldDLOverride(void);

// ---------------------------------------------------------------------------
// Shared custom-equipment DRAW template. Every ext piece drew the same skeleton
// (open disps -> push -> [setup DL] -> [prim/env color] -> T*R*S matrix -> load
// matrix -> draw DL -> pop -> close) and only varied a handful of values. Fill an
// EquipDrawModel and call ExtEquip_DrawModel — new pieces become a one-liner.
// Rotation is RADIANS (X then Y then Z, applied after translate, before scale).
// ---------------------------------------------------------------------------
typedef struct EquipDrawModel {
    Gfx* dl;           // display list to draw (NULL = no-op)
    u8 xlu;            // 1 -> POLY_XLU pass (custom combiners that dirty the pipe); 0 -> POLY_OPA
    u8 setupDL;        // 1 -> emit Gfx_SetupDL_25Opa/Xlu first; 0 -> the DL provides its own state
    Vec3f translate;   // MTXMODE_APPLY, applied first
    Vec3f rotate;      // radians X,Y,Z, after translate (all 0 = none)
    Vec3f scale;       // applied last
    u8 setColor;       // 1 -> apply prim/env below (for lit/tinted models)
    Color_RGBA8 prim;
    Color_RGBA8 env;
} EquipDrawModel;

void ExtEquip_DrawModel(void* play, const EquipDrawModel* model);

/**
 * Draw the ext shield DL in the current matrix context (called from PostLimbDraw).
 * For Shield of Ikana: draws GI Mirror Shield model.
 */
void ExtEquip_DrawShieldDL(void* play);
void ExtEquip_DrawShieldBackDL(void* play);

/**
 * Draw Dragon Scale pendant at waist. Called from PostLimbDraw for PLAYER_LIMB_WAIST.
 */
void ExtEquip_DrawWaistScale(void* play);

/**
 * Draw the ext sword DL in the current matrix context (called from PostLimbDraw).
 * For Byrna: draws blue Somaria cane.
 */
void ExtEquip_DrawSwordDL(void* play);

/**
 * Draw anklet decoration on foot limbs (torus + fairy wings with pendulum).
 * Called from PostLimbDraw for PLAYER_LIMB_L_FOOT and PLAYER_LIMB_R_FOOT.
 * @param play PlayState
 * @param isRightFoot 1 for right foot, 0 for left foot
 */
void ExtEquip_DrawAnklet(void* play, s32 isRightFoot);

/**
 * Update pendulum physics for anklet wings. Called from Pegasus_Behavior.
 */
void ExtEquip_UpdateAnkletPhysics(void* player);

/**
 * Capture shoulder world positions for cloth physics (Magic Cape + Champion's Scarf).
 * Called from PostLimbDraw for PLAYER_LIMB_L_SHOULDER and PLAYER_LIMB_R_SHOULDER.
 * @param limbIndex The limb being drawn
 */
// ExtEquip_CaptureCapeShoulderPos removed — cape uses bodyPartsPos anchors.

/**
 * Suppress icon override for ext equipment (used by kaleido equipment screen).
 * When set to 1, ExtInv_GetItemIcon won't replace sword/shield icons.
 */
extern u8 gExtEquipSuppressIconOverride;
// 1 while the equipment page names one of its PAGE-2 GRID cells (disambiguates the one item id
// shared by the Pendant of Memories and the Climb Boots — see extended_equipment.c).
extern u8 gExtEquipGridNameContext;

// ---------------------------------------------------------------------------
// Shield of Ikana: Death Save
// ---------------------------------------------------------------------------

/** Check if Shield of Ikana should revive player instead of dying */
u8 ExtEquip_IkanaDeathSave(void* play);

/** Draw Spirit Breastplate (Iron Knuckle armor) on Link's torso.
 *  Called from PostLimbDraw for PLAYER_LIMB_UPPER. */
void ExtEquip_DrawBreastplate(void* play);

#ifdef __cplusplus
}
#endif

#endif // EXTENDED_EQUIPMENT_H
