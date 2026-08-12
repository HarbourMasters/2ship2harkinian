/**
 * equip_kite_shield.c - Kite Shield (Extended Shield Slot 2)
 *
 * Takes over the slot the Gerudo Scimitar placeholder used to hold. The MODEL already exists
 * (objects/object_nei_kite_shield, drawn via ExtEquip_GetKiteShieldDL in extended_equipment.c);
 * only the gameplay behavior was ever missing.
 *
 * BEHAVIOR: not specified yet — stub. Grid slot, ownership bit, model, icon
 * gItemIconKiteShieldTex and name gKiteShieldNameTex are all wired.
 *
 * Included by ext_equip_behavior.c (unity build).
 */

// Per-frame behavior while the Kite Shield is the equipped ext shield.
static void KiteShield_Behavior(Player* player, PlayState* play) {
    (void)player;
    (void)play;
}

// Called when the Kite Shield is unequipped.
static void KiteShield_Cleanup(void) {
}

// A perfect-guard / block hook goes here when the behavior is specified — export it non-static and
// call it from z_player.c next to DivineShield_OnShieldBlock, the way equip_ikana.c does.
