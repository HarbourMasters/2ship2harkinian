#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

extern "C" {
bool isPressingHeldItemButton(Player* player);
}

void RegisterTwoHandedSwordSpinAttack() {

    REGISTER_VB_SHOULD(VB_HELD_ITEM_BUTTON_PRESS, {
        if (CVarGetInteger("gEnhancements.Equipment.TwoHandedSwordSpinAttack", 0)) {
            // Allow the Great Fairy Sword, a C button item, to be held for charged spin attacks
            Player* player = (Player*)(opt);
            *should = isPressingHeldItemButton(player);
        }
    });
    REGISTER_VB_SHOULD(VB_MAGIC_SPIN_ATTACK_CHECK_FORM, {
        if (CVarGetInteger("gEnhancements.Equipment.TwoHandedSwordSpinAttack", 0)) {
            // Allow both Fierce Deity and human forms to use charged spin attacks
            uint8_t form = *(uint8_t*)opt;
            *should = (form == PLAYER_FORM_FIERCE_DEITY || form == PLAYER_FORM_HUMAN);
        }
    });
}
