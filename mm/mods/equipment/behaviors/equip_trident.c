/**
 * equip_trident.c - Trident (Extended Sword Slot 3)
 *
 * Replaces the Iron Knuckle's Axe in this slot: the axe became the HAMMER UPGRADE
 * (WeaponUpgrade_HasHammerAxe -> IKAxe_Behavior, driven from ExtEquip_UpdateBehavior and
 * independent of the ext-equipment grid), so its code is untouched but no longer lives here.
 *
 * BEHAVIOR: not specified yet — stub. Everything else is wired (grid slot, ownership bit, icon
 * gItemIconTridentTex, name gTridentNameTex, dispatch below), so filling this in is the only
 * remaining step.
 *
 * Included by ext_equip_behavior.c (unity build).
 */

// Per-frame behavior while the Trident is the equipped ext sword.
static void Trident_Behavior(Player* player, PlayState* play) {
    (void)player;
    (void)play;
}

// Called when the Trident is unequipped (restore anything the behavior forced).
static void Trident_Cleanup(void) {
}

// Called from the melee-hit dispatch while the Trident is equipped.
static void Trident_OnMeleeHit(Player* player, PlayState* play) {
    (void)player;
    (void)play;
}
