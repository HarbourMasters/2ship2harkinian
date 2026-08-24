// =============================================================================
// CombatSync — PvP damage transport (Phase E).
//
// Hit DETECTION is no longer here: every remote player now carries a real
// ACTOR_HARPOON_PEER with an AC ColliderCylinder (see HarpoonDummyPlayer.cpp),
// so the engine resolves our attacks against them exactly like it resolves them
// against an enemy — sword, form attacks (Player_SetCylinderForAttack), bombs,
// arrows and every NEI custom item that registers an AT collider, all for free
// and with the right element. HarpoonPeer_Update reads AC_HIT and calls
// HarpoonCombat_SendDamage below.
//
// Wire format matches SoH's SendPacket_Damage (targetClientId + damageEffect)
// so a shared relay treats both ports the same; the legacy 2ship field names
// are duplicated in the same payload for one version so older clients keep
// working.
// =============================================================================

#include "CombatSync.h"
#include "../Harpoon.h"
#include <nlohmann/json.hpp>

uint8_t HarpoonDamageTable::Damage(uint16_t weapon) {
    switch (weapon) {
        case HARPOON_WEAPON_KOKIRI_SWORD:
            return 16; // 1 heart (16 units per heart)
        case HARPOON_WEAPON_RAZOR_SWORD:
            return 32;
        case HARPOON_WEAPON_GILDED_SWORD:
            return 32;
        case HARPOON_WEAPON_GREAT_FAIRY:
            return 64; // 4 hearts
        case HARPOON_WEAPON_BOMB:
            return 64;
        case HARPOON_WEAPON_BOMBCHU:
            return 32;
        case HARPOON_WEAPON_POWDER_KEG:
            return 128;
        case HARPOON_WEAPON_ARROW:
            return 16;
        case HARPOON_WEAPON_FIRE_ARROW:
            return 32;
        case HARPOON_WEAPON_ICE_ARROW:
            return 32;
        case HARPOON_WEAPON_LIGHT_ARROW:
            return 64;
        case HARPOON_WEAPON_HOOKSHOT:
            return 8;
        case HARPOON_WEAPON_GORON_ROLL:
            return 32;
        case HARPOON_WEAPON_GORON_SPIKE:
            return 48;
        case HARPOON_WEAPON_ZORA_ELECTRIC:
            return 32;
        case HARPOON_WEAPON_ZORA_FIN:
            return 16;
        case HARPOON_WEAPON_DEKU_SPIN:
            return 16;
        case HARPOON_WEAPON_DEKU_BUBBLE:
            return 16;
        case HARPOON_WEAPON_FD_BEAM:
            return 64;
        case HARPOON_WEAPON_FD_SPIN:
            return 96;
        case HARPOON_WEAPON_BLAST_MASK:
            return 64;
        default:
            return 0;
    }
}

void HarpoonCombat_SendDamage(uint32_t targetClientId, uint8_t damageEffect, uint8_t damage) {
    auto* h = Harpoon::Instance();
    if (h->State() != HarpoonConnState::InRoom)
        return;
    if (!h->IsPvpActive())
        return; // gamemode gates PvP (e.g. geoguessr)
    if (damage == 0 && damageEffect == HARPOON_HIT_NONE)
        return;

    uint32_t own = h->OwnClientId();
    nlohmann::json p = { { "type", HarpoonPT::COMBAT_DEAL_DAMAGE },
                         { "payload",
                           {
                               { "clientId", own }, // attacker — receiver aims knockback at us
                               { "targetClientId", targetClientId },
                               { "damageEffect", (int)damageEffect },
                               { "damage", (int)damage },
                               // Legacy 2ship field names, kept for one version so pre-collision
                               // clients still take the hit. Drop once everyone has updated.
                               { "targetCid", targetClientId },
                               { "weaponSource", (int)damageEffect },
                           } } };
    h->SendJson(p);
}
