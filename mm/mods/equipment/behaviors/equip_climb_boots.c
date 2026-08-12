/**
 * equip_climb_boots.c - Climb Boots (Extended Boots Slot 2)
 *
 * Takes over the slot the Pendant of Memories used to squat on. The Pendant is NOT gone: it lives
 * on the equipment page's left column (ownership = the adult trade wheel, TRADE_ADULT_PENDANT) and
 * its moveset is dispatched cheat-independently, so this slot is free for a real pair of boots.
 *
 * BEHAVIOR: not specified yet — stub. Grid slot, ownership bit, icon gItemIconClimbBootsTex and
 * name gClimbBootsNameTex are wired.
 *
 * Included by ext_equip_behavior.c (unity build).
 */

// Per-frame behavior while the Climb Boots are the equipped ext boots.
static void ClimbBoots_Behavior(Player* player, PlayState* play) {
    (void)player;
    (void)play;
}

// Called when the Climb Boots are unequipped (restore anything the behavior forced).
static void ClimbBoots_Cleanup(void) {
}
