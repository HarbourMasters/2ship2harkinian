/* The Sage's Tunic grants passive resistances from owned OoT medallions. */

static void Sages_Behavior(Player* player, PlayState* play) {
    s32 i;

    (void)play;

    ExtEquip_SagesFlashTick();

    if (ExtEquip_HasSagesResistance(SAGES_RESIST_FIRE) && player->bodyIsBurning) {
        for (i = 0; i < PLAYER_BODYPART_MAX; i++) {
            player->bodyFlameTimers[i] = 0;
        }
        player->bodyIsBurning = false;
        ExtEquip_SagesFlash(SAGES_RESIST_FIRE);
    }
    if (ExtEquip_HasSagesResistance(SAGES_RESIST_THUNDER)) {
        if (player->bodyShockTimer != 0) {
            ExtEquip_SagesFlash(SAGES_RESIST_THUNDER);
        }
        player->bodyShockTimer = 0;
    }
}
