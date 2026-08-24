/**
 * equip_breastplate.c - Spirit Breastplate (Extended Tunic Slot 2)
 *
 * Behavior: Magic Armor (TP-style) — rupee-cost damage immunity.
 * - Makes Link immune to all damage while wearing (rupees > 0)
 * - Each HP of damage received costs 1 rupee
 * - Absorbed hits play like a shield block: no knockback, no damage animation, no hurt voice
 *   (gated per damage path in z_player.c via Player_SpiritTunicAbsorbHit)
 * - 30% of each charge (ceil) SPILLS out of the wallet as real rupee pickups raining down on Link
 * - No rupees = slow movement (cursed weight)
 * - Passive rupee drain: 1 rupee per 30 frames
 *
 * The visual is a RECOLOR, not armor: the tunic DLs are patched orange (rupees) / near-black
 * (broke) from NeiTunic_UpdateEquipmentColor in 2s2h/BenGui/CosmeticEditor.cpp. The Iron Knuckle
 * armor draw further down is legacy and no longer reachable — ExtEquip_DrawBreastplate has no
 * callers.
 *
 * Damage immunity is implemented via a direct C call from Health_ChangeBy
 * in z_parameter.c to Breastplate_OnHealthChangeBefore (defined below).
 * Setting `player->invincibilityTimer = -1` from this behavior runs too late
 * in Player_Update (UpdateCommon ticks the timer back to 0 before the damage
 * handler runs), so we intercept at the Health_ChangeBy level instead.
 *
 * Included by ext_equip_behavior.c (unity build).
 */

// No extra includes — unity-built from ext_equip_behavior.c
extern void Rupees_ChangeBy(s16 rupeeChange);
u8 Breastplate_IsActive(void);

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
#define BREASTPLATE_RUPEE_INTERVAL 30 // Passive drain: 1 rupee every N frames
#define BREASTPLATE_SLOW_MULT 0.5f    // Speed multiplier when broke

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------
static s16 sBreastplateRupeeTick = 0;

// ---------------------------------------------------------------------------
// Main Behavior — runs every frame from ExtEquip_UpdateBehavior
// Handles passive rupee drain and the broke-mode movement penalty.
// Damage interception is handled separately by Breastplate_OnHealthChangeBefore.
// ---------------------------------------------------------------------------
static void Breastplate_Behavior(Player* player, PlayState* play) {
    // Skip during cutscenes, dying, etc.
    if (player->stateFlags1 & (PLAYER_STATE1_DEAD | PLAYER_STATE1_IN_CUTSCENE | PLAYER_STATE1_LOADING |
                               PLAYER_STATE1_IN_ITEM_CS | PLAYER_STATE1_GETTING_ITEM)) {
        return;
    }

    if (gSaveContext.save.saveInfo.playerData.rupees > 0) {
        sBreastplateRupeeTick++;
        if (sBreastplateRupeeTick >= BREASTPLATE_RUPEE_INTERVAL) {
            sBreastplateRupeeTick = 0;
            Rupees_ChangeBy(-1);
        }
    } else {
        // No rupees: heavy and slow
        sBreastplateRupeeTick = 0;
        player->linearVelocity *= BREASTPLATE_SLOW_MULT;
        player->actor.speed *= BREASTPLATE_SLOW_MULT;
    }
}

// ---------------------------------------------------------------------------
// Coin spill — ceil(30%) of the charge falls out of the wallet as REAL rupee actors (EnItem00),
// tossed around Link with the standard drop bounce (Item_DropCollectible: random yaw, vy 8).
// Decomposed into red/blue/green denominations so a big hit stays a handful of actors.
//
// Each coin gets a random bearing around Link at a short radius, mirroring the harpoon death-pile
// scatter (soh/Network/Harpoon/DroppedItems.cpp SpawnInScene): without it they share one XZ and
// z-fight into a single blob.
//
// The coins spawn ABOVE Link's head and rain down. This is not cosmetic: EnItem00_Update collects
// on `xzDistToPlayer <= 30 && |playerHeightRel| <= 50` (z_en_item00.c:539), and the scatter radius
// is well inside 30 units — spawning at Link's feet means he swallows the whole spill on the very
// frame it appears, so the drop is invisible and free. Clearing the vertical window at spawn keeps
// them uncollectable until they have fallen, and Item_DropCollectible's outward speed carries them
// past the 30-unit ring on the way down, so they must be walked back to.
//
// MM-only wrinkle vs the soh copy: with PLAYER_STATE3_1000 (Great Fairy's Mask) the window widens
// to 60 / +-100, so the mask can still vacuum the spill mid-fall. That is the mask doing its job —
// deliberately not fought here.
// ---------------------------------------------------------------------------
#define BREASTPLATE_SPILL_RADIUS_MIN 8.0f
#define BREASTPLATE_SPILL_RADIUS_MAX 20.0f
// Clearance above the head. Must keep (height + this) > 50 for every form so the spawn starts
// outside the pickup window; 40 leaves margin even for Deku, the shortest transformation.
#define BREASTPLATE_SPILL_HEIGHT_MARGIN 40.0f

static void Breastplate_SpillRupees(PlayState* play, s16 rupeeCost) {
    Player* player = GET_PLAYER(play);
    s16 spill = (s16)((rupeeCost * 3 + 9) / 10); // ceil(rupeeCost * 0.3)
    f32 spawnY = player->actor.world.pos.y + Player_GetHeight(player) + BREASTPLATE_SPILL_HEIGHT_MARGIN;

    while (spill > 0) {
        u32 params;
        if (spill >= 20) {
            params = ITEM00_RUPEE_RED;
            spill -= 20;
        } else if (spill >= 5) {
            params = ITEM00_RUPEE_BLUE;
            spill -= 5;
        } else {
            params = ITEM00_RUPEE_GREEN;
            spill -= 1;
        }

        s16 angle = (s16)Rand_CenteredFloat(65536.0f);
        f32 radius = BREASTPLATE_SPILL_RADIUS_MIN +
                     Rand_ZeroOne() * (BREASTPLATE_SPILL_RADIUS_MAX - BREASTPLATE_SPILL_RADIUS_MIN);
        Vec3f pos = { player->actor.world.pos.x + Math_CosS(angle) * radius, spawnY,
                      player->actor.world.pos.z + Math_SinS(angle) * radius };

        Item_DropCollectible(play, &pos, params);
    }
}

// ---------------------------------------------------------------------------
// Pre-damage hook: convert incoming damage to rupee cost while breastplate
// is active and Link has rupees.
//
// Called directly from Health_ChangeBy in z_parameter.c BEFORE health is
// mutated. Setting *amount = 0 makes Health_ChangeBy return early without
// touching gSaveContext.save.saveInfo.playerData.health.
// ---------------------------------------------------------------------------
void Breastplate_OnHealthChangeBefore(PlayState* play, int16_t* amount) {
    if (!Breastplate_IsActive()) {
        return;
    }
    if (*amount >= 0) {
        return; // healing — pass through
    }
    if (gSaveContext.save.saveInfo.playerData.rupees <= 0) {
        return; // broke — take damage normally
    }

    s16 damageHP = -*amount;
    s16 rupeeCost = damageHP;
    if (rupeeCost > gSaveContext.save.saveInfo.playerData.rupees) {
        rupeeCost = (s16)gSaveContext.save.saveInfo.playerData.rupees;
    }
    Rupees_ChangeBy(-rupeeCost);
    Sfx_PlaySfxCentered(NA_SE_IT_SHIELD_BOUND);
    Breastplate_SpillRupees(play, rupeeCost);

    *amount = 0; // block the damage
}

// ---------------------------------------------------------------------------
// Tunic color tint: gold (has rupees) or dark (broke)
// Called from the tunic color system to override Link's tunic color
// ---------------------------------------------------------------------------
u8 Breastplate_IsActive(void) {
    return ExtEquip_IsEnabled() && gExtEquipState.currentExtTunic == 2;
}

// Returns 1 if player has rupees (gold mode), 0 if broke (dark mode)
u8 Breastplate_HasPower(void) {
    return gSaveContext.save.saveInfo.playerData.rupees > 0;
}

// ---------------------------------------------------------------------------
// Draw Iron Knuckle chest armor on Link's upper body
// Called from PostLimbDraw for PLAYER_LIMB_UPPER
// Uses inline DL from OOT decomp (no external object dependencies)
// Textures from soh.otr (always available)
// ---------------------------------------------------------------------------
#include "equipment/objects/breastplate_DL/model.inc.c"

// Helper: set color + alpha based on rupees
// Has rupees: golden, 20% alpha (51/255)
// No rupees:  golden, fully opaque (armor materializes when magic is spent)
#define BREASTPLATE_SET_MATERIAL()                                      \
    do {                                                                \
        if (gSaveContext.save.saveInfo.playerData.rupees > 0) {         \
            gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 225, 205, 115, 51);  \
            gDPSetEnvColor(POLY_XLU_DISP++, 25, 20, 0, 255);            \
        } else {                                                        \
            gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 225, 205, 115, 255); \
            gDPSetEnvColor(POLY_XLU_DISP++, 25, 20, 0, 255);            \
        }                                                               \
    } while (0)

// Base transform (user-tuned): Scale then translate to center on Link's torso
#define BP_SX 1.0f
#define BP_SY 0.7f
#define BP_SZ 1.2f
#define BP_TX -1000.0f
#define BP_TY 104.0f
#define BP_TZ 0.0f

static void Breastplate_DrawPiece(PlayState* play, Gfx* dl, f32 ikOffX, f32 ikOffY, f32 ikOffZ) {
    OPEN_DISPS(play->state.gfxCtx);

    Matrix_Push();
    Gfx_SetupDL_25Opa(play->state.gfxCtx);
    BREASTPLATE_SET_MATERIAL();

    // Apply base transform then IK skeleton offset
    Matrix_Scale(BP_SX, BP_SY, BP_SZ, MTXMODE_APPLY);
    Matrix_Translate(BP_TX + ikOffX, BP_TY + ikOffY, BP_TZ + ikOffZ, MTXMODE_APPLY);

    gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(POLY_XLU_DISP++, dl);
    Matrix_Pop();

    CLOSE_DISPS(play->state.gfxCtx);
}

static void Breastplate_Draw(PlayState* play) {
    // IK skeleton offsets from upper body root (Limb 1):
    // Chest plates:    (0, 0, 0)
    // R Pauldron:      (+1900, 0, -1184)
    // L Pauldron:      (+1900, 0, +1184)
    // Helmet marking:  (+2100, -200, 0)
    Breastplate_DrawPiece(play, gSpiritChestDL, 0.0f, 0.0f, 0.0f);
    Breastplate_DrawPiece(play, gSpiritPauldronRDL, 1900.0f, 0.0f, -1184.0f);
    Breastplate_DrawPiece(play, gSpiritPauldronLDL, 1900.0f, 0.0f, 1184.0f);
    Breastplate_DrawPiece(play, gSpiritHelmetMarkDL, 2100.0f, -200.0f, 0.0f);
}
