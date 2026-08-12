#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "mods/extended_equipment.h"
}

static void RegisterSagesTunicHooks() {
    REGISTER_VB_SHOULD(VB_RECEIVE_FALL_DAMAGE, {
        if (ExtEquip_HasSagesResistance(SAGES_RESIST_FALL)) {
            ExtEquip_SagesFlash(SAGES_RESIST_FALL);
            *should = false;
        }
    });
    REGISTER_VB_SHOULD(VB_LIKE_LIKE_GRAB_PLAYER, {
        if (ExtEquip_HasSagesResistance(SAGES_RESIST_STUN)) {
            ExtEquip_SagesFlash(SAGES_RESIST_STUN);
            *should = false;
        }
    });
    REGISTER_VB_SHOULD(VB_REDEAD_GIBDO_FREEZE_PLAYER, {
        if (ExtEquip_HasSagesResistance(SAGES_RESIST_STUN)) {
            ExtEquip_SagesFlash(SAGES_RESIST_STUN);
            *should = false;
        }
    });
    REGISTER_VB_SHOULD(VB_ENEMY_GRAB_PLAYER, {
        if (ExtEquip_HasSagesResistance(SAGES_RESIST_STUN)) {
            ExtEquip_SagesFlash(SAGES_RESIST_STUN);
            *should = false;
        }
    });
}

static RegisterShipInitFunc initSagesTunicHooks(RegisterSagesTunicHooks);
