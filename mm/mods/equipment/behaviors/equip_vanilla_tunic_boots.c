/**
 * equip_vanilla_tunic_boots.c - OoT vanilla tunics/boots, REINTERPRETED for MM (Skijer's NEI).
 *
 * MM's Link has NO native Goron/Zora tunic or Iron/Hover boots slots; the equipped piece lives in
 * Nei_Save()->vanillaTunic (0 Kokiri / 1 Goron / 2 Zora) and vanillaBoots (0 Kokiri / 1 Iron /
 * 2 Hover). The classic OoT effects don't map to MM's context (MM human Link barely touches deep
 * water — the Zora form swims), so they're reinterpreted to be useful in MM (user decision
 * 2026-07-11):
 *   Goron tunic  -> FIREPROOF: extinguish the body-burn before it deals its damage-over-time tick.
 *   Zora tunic   -> GAS/POISON IMMUNITY (predicate VanillaTB_IsZoraGasImmune; consumed at the
 *                   swamp-gas / poison damage site when wired — reactive refund isn't reliable).
 *   Iron boots   -> ANTI-KNOCKBACK: damage knockback decays faster (much shorter backward slide),
 *                   "heavy / planted".
 *   Hover boots  -> AIR-FLOAT: the descent is softened while airborne, giving a brief float / hang.
 *
 * Runs every frame from ExtEquip_UpdateBehavior, cheat-independent (like the Great Fairy's Sword
 * upkeep). Reactive by design so it does NOT edit MM's core damage/physics paths.
 * Included by ext_equip_behavior.c (unity build).
 */

#define VTB_TUNIC_GORON 1
#define VTB_TUNIC_ZORA 2
#define VTB_BOOTS_IRON 1
#define VTB_BOOTS_HOVER 2

// Iron: per-frame multiplier applied to the horizontal knockback speed while in the damage state
// (0.5 halves the slide each frame → the knockback distance collapses quickly).
#define VTB_IRON_KNOCKBACK_DECAY 0.5f
// Iron SINK: while in water and not on the floor, add this much downward velocity per frame (capped)
// so Link drops toward the bottom instead of treading at the surface (OoT iron-boots feel).
#define VTB_IRON_SINK_ACCEL 1.5f
#define VTB_IRON_SINK_MAX (-6.0f)

u8 VanillaTB_IsGoronFireproof(void) {
    return Nei_Save()->vanillaTunic == VTB_TUNIC_GORON;
}
u8 VanillaTB_IsZoraGasImmune(void) {
    return Nei_Save()->vanillaTunic == VTB_TUNIC_ZORA;
}
// Skijer 2026-07-16 (ported from the OoT rework): wearing the ZORA TUNIC also grants the Zora
// SWIM package to human Link — fast swim (A boost), no drown timer, and the electric water
// barrier. Consumed by the Nei_IsZoraSwim() gates in z_player.c / z_parameter.c. Same predicate
// as the shock immunity; separate name so the swim wiring reads clearly.
u8 VanillaTB_IsZoraTunic(void) {
    return Nei_Save()->vanillaTunic == VTB_TUNIC_ZORA;
}
u8 VanillaTB_IsIronBoots(void) {
    return Nei_Save()->vanillaBoots == VTB_BOOTS_IRON;
}
u8 VanillaTB_IsHoverBoots(void) {
    return Nei_Save()->vanillaBoots == VTB_BOOTS_HOVER;
}

static void VanillaTB_Behavior(Player* player, PlayState* play) {
    // Human form only — the tunic/boots are Link's; transformed forms have their own bodies.
    if (player->transformation != PLAYER_FORM_HUMAN) {
        return;
    }
    if (player->stateFlags1 & (PLAYER_STATE1_DEAD | PLAYER_STATE1_IN_CUTSCENE)) {
        return;
    }

    // Goron tunic: FIREPROOF. Player_UpdateBodyBurn ticks -1 HP every 4 frames while any body-part
    // flame timer is up; snuff them so the burn never deals damage (and the flames stop spawning).
    if (VanillaTB_IsGoronFireproof() && player->bodyIsBurning) {
        s32 i;
        for (i = 0; i < PLAYER_BODYPART_MAX; i++) {
            player->bodyFlameTimers[i] = 0;
        }
        player->bodyIsBurning = false;
    }

    // Zora tunic: ELECTRIC-SHOCK immunity. Reinterpreted for MM — MM has no clean player-side
    // swamp-gas damage hook, but the Great Bay / Zora domain are full of electric hazards, so this is
    // the useful water-affinity effect. Snuff the shock timer so the stun never sticks. Safe: the
    // native `if (bodyShockTimer != 0)` gate around Player_UpdateBodyShock means zeroing it here also
    // stops the decrement (no underflow / stuck-shock).
    if (VanillaTB_IsZoraGasImmune() && player->bodyShockTimer != 0) {
        player->bodyShockTimer = 0;
    }

    // Iron boots: SINK. In water and off the floor, push the vertical velocity down so Link drops
    // toward the bottom instead of treading at the surface (OoT iron-boots feel). On land it's just
    // "heavy" — the damage knockback decays fast (anti-knockback).
    if (VanillaTB_IsIronBoots()) {
        if ((player->actor.depthInWater > 0.0f) && !(player->actor.bgCheckFlags & BGCHECKFLAG_GROUND)) {
            player->actor.velocity.y -= VTB_IRON_SINK_ACCEL;
            if (player->actor.velocity.y < VTB_IRON_SINK_MAX) {
                player->actor.velocity.y = VTB_IRON_SINK_MAX;
            }
        }
        if (player->stateFlags1 & PLAYER_STATE1_DAMAGED) {
            player->speedXZ *= VTB_IRON_KNOCKBACK_DECAY;
        }
    }

    // Hover boots WALK-ON-AIR is NOT handled here — a reactive velocity clamp from this per-frame hook
    // runs after MM has already committed the fall (gravity is applied in func_8083BF54), so Link left
    // the ledge before the clamp landed. It's ported to the core instead: func_8083BF54 kills gravity
    // + suspends the descent while the gNeiHoverTimer budget lasts, refilled on solid ground during
    // floor processing. See z_player.c (gNeiHoverTimer).
}
