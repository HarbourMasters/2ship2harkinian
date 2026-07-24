/*
 * nei_harpoon_stubs.cpp — temporary no-op stubs for the NEI / SW97 → Harpoon
 * netcode bridge. The 2ship Harpoon module doesn't expose these C entry points
 * yet, so the ported item/equipment code fails to link. These no-ops satisfy the
 * linker; behaviour is inert (parry/revive not synced to teammates, prop-hunt
 * always inactive) until wired to the real Harpoon combat / prop-hunt systems.
 *
 * User-approved: stub no-op for now (see the NEI port plan).
 * Lives under mods/*.cpp so the CONFIGURE_DEPENDS glob auto-picks it up.
 */
extern "C" {

// equip_divine_shield.c / equip_ikana.c — broadcast a shield parry to teammates.
void HarpoonCombat_BroadcastShieldParry_C(int shieldType, int effect) {
    (void)shieldType;
    (void)effect;
}

// equip_ikana.c — broadcast a Shield-of-Ikana self-revive to teammates.
void HarpoonCombat_BroadcastShieldRevive_C(int restoredHealth) {
    (void)restoredHealth;
}

// item_cane_of_somaria.c — is the Harpoon "prop hunt" mode active? (0 = no)
int HarpoonPropHunt_IsActive(void) {
    return 0;
}

} // extern "C"
