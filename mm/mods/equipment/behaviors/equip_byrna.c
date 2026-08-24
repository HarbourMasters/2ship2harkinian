/**
 * equip_byrna.c - Cane of Byrna (Extended Sword Slot 1)
 *
 * Behavior: Biggoron Sword IA (long range, two-handed) + HP & MP recovery on hit.
 * - Forces PLAYER_IA_SWORD_BIGGORON for long reach
 * - Forces swordHealth > 0 so charge/spin attacks work
 * - Draws Somaria cane mesh with BLUE materials at 1.15x scale
 * - Follows left hand rotation (sword hand)
 * - On melee hit: recover HP + MP
 *
 * Included by ext_equip_behavior.c (unity build).
 */

// Byrna 3D model (blue cane) now lives in soh.o2r as
// objects/object_somaria/g_byrna_cane_dl and is loaded at draw time in
// extended_equipment.c (Byrna_GetCaneDL). No inline C model here anymore.

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
#define BYRNA_HP_RECOVER 16         // HP recovered per hit
#define BYRNA_MP_RECOVER 4          // MP recovered per hit
#define BYRNA_SCALE (0.05f * 1.15f) // Somaria base scale * 1.15

// ---------------------------------------------------------------------------
// Melee Hit Callback
// ---------------------------------------------------------------------------
static void GreatFairySword_RecoverOnHit(Player* player, PlayState* play) {
    s32 damage = 0;

    if (player->meleeWeaponQuads[0].base.atFlags & AT_HIT) {
        damage = player->meleeWeaponQuads[0].elem.atDmgInfo.damage;
    } else if (player->meleeWeaponQuads[1].base.atFlags & AT_HIT) {
        damage = player->meleeWeaponQuads[1].elem.atDmgInfo.damage;
    }

    if (damage <= 0)
        return;

    // Recover 16 HP per hit
    Health_ChangeBy(play, BYRNA_HP_RECOVER);

    // Recover 16 MP per hit
    gSaveContext.save.saveInfo.playerData.magic += BYRNA_MP_RECOVER;
    if (gSaveContext.save.saveInfo.playerData.magic > gSaveContext.magicCapacity) {
        gSaveContext.save.saveInfo.playerData.magic = gSaveContext.magicCapacity;
    }
}

// ---------------------------------------------------------------------------
// Cane of Byrna — DUMMY (Skijer 2026-07-29). Its whole gameplay behavior (two-handed reach + HP/MP
// recovery on melee hit) belongs to the progressive double-hand sword line now: the Great Fairy's
// Sword owns it below, where it is the player's REAL sword instead of a sword-slot hijack. The grid
// slot, icon, name and hand model stay so the slot is visible; it is reserved for a future behavior.
// ---------------------------------------------------------------------------
static void Byrna_Behavior(Player* player, PlayState* play) {
    (void)player;
    (void)play;
}

static void Byrna_Cleanup(void) {
    // Nothing to restore: the slot no longer touches the sword equip / swordHealth / B button.
    gExtEquipBehavior.byrnaActive = 0;
}

// Draw is now handled by PostLimbDraw in z_player_lib.c via ExtEquip_DrawSwordDL
// This ensures the cane follows the exact same rotation as the sword during swings

// ---------------------------------------------------------------------------
// Great Fairy's Sword (NEI progressive BGS level 2) — same combat perks as the
// Cane of Byrna, but it IS the player's real Biggoron Sword (no sword-slot hijack).
// Driven by WeaponUpgrade_HasGreatFairy() from ExtEquip_UpdateBehavior, independent
// of the extended-equipment cheat. We only top up swordHealth/bgsFlag (so charge/spin
// always work and a Giant's Knife never "breaks") and recover HP+MP on each melee hit.
// ---------------------------------------------------------------------------
static void GreatFairySword_Behavior(Player* player, PlayState* play) {
    if (player->stateFlags1 & (PLAYER_STATE1_DEAD | PLAYER_STATE1_IN_CUTSCENE | PLAYER_STATE1_LOADING |
                               PLAYER_STATE1_IN_ITEM_CS | PLAYER_STATE1_GETTING_ITEM)) {
        return;
    }
    // Only while actually wielding the Biggoron Sword.
    if (player->heldItemAction != PLAYER_IA_SWORD_TWO_HANDED) {
        return;
    }
    if (gSaveContext.save.saveInfo.playerData.swordHealth <= 0) {
        gSaveContext.save.saveInfo.playerData.swordHealth = 8;
    }
}

static void GreatFairySword_OnMeleeHit(Player* player, PlayState* play) {
    GreatFairySword_RecoverOnHit(player, play);
}
