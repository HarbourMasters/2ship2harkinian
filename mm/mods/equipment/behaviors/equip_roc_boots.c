/**
 * equip_roc_boots.c - Roc Boots (Extended Boots Slot 3)
 *
 * Takes over the slot the deleted Water Dragon Scale used to hold (its Zora swim became the ZORA
 * TUNIC's permanent effect — see equip_dragonscale.c / Nei_IsZoraSwim).
 *
 * BEHAVIOR: not specified yet — stub. Grid slot, ownership bit, icon gItemIconRocBootsTex and name
 * gRocBootsNameTex are wired.
 *
 * Included by ext_equip_behavior.c (unity build).
 */

// Per-frame behavior while the Roc Boots are the equipped ext boots.
static void RocBoots_Behavior(Player* player, PlayState* play) {
    (void)player;
    (void)play;
}

// Called when the Roc Boots are unequipped (restore anything the behavior forced).
static void RocBoots_Cleanup(void) {
}
