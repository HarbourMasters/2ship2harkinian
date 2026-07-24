// =============================================================================
// CombatSync — PvP weapon table + local hit detection (Phase E).
//
// Sends COMBAT.DEAL_DAMAGE when the local player swings / shoots / explodes
// near a remote player. The damage table mirrors Harpoon SoH so the server
// relay treats hits identically across both ports.
//
// Hit detection in this iteration is proximity-based ONLY (no real AT collider
// integration with z_collision_check.c yet). That's enough for form attacks
// and adjacency melee, but proper projectile/sword routing is a follow-up.
// =============================================================================

#include "CombatSync.h"
#include "../Harpoon.h"
#include <nlohmann/json.hpp>
#include <cmath>

extern "C" {
#include "z64.h"
#include "z64actor.h"
#include "z64player.h"
#include "macros.h"
#include "variables.h"
}

uint8_t HarpoonDamageTable::Damage(uint16_t weapon) {
    switch (weapon) {
        case HARPOON_WEAPON_KOKIRI_SWORD: return 4;   // 1 heart
        case HARPOON_WEAPON_RAZOR_SWORD:  return 8;
        case HARPOON_WEAPON_GILDED_SWORD: return 8;
        case HARPOON_WEAPON_GREAT_FAIRY:  return 16;  // 4 hearts
        case HARPOON_WEAPON_BOMB:         return 16;
        case HARPOON_WEAPON_BOMBCHU:      return 8;
        case HARPOON_WEAPON_POWDER_KEG:   return 32;
        case HARPOON_WEAPON_ARROW:        return 4;
        case HARPOON_WEAPON_FIRE_ARROW:   return 8;
        case HARPOON_WEAPON_ICE_ARROW:    return 8;
        case HARPOON_WEAPON_LIGHT_ARROW:  return 16;
        case HARPOON_WEAPON_HOOKSHOT:     return 2;
        case HARPOON_WEAPON_GORON_ROLL:   return 8;
        case HARPOON_WEAPON_GORON_SPIKE:  return 12;
        case HARPOON_WEAPON_ZORA_ELECTRIC: return 8;
        case HARPOON_WEAPON_ZORA_FIN:      return 4;
        case HARPOON_WEAPON_DEKU_SPIN:     return 4;
        case HARPOON_WEAPON_DEKU_BUBBLE:   return 4;
        case HARPOON_WEAPON_FD_BEAM:       return 16;
        case HARPOON_WEAPON_FD_SPIN:       return 24;
        case HARPOON_WEAPON_BLAST_MASK:    return 16;
        default: return 0;
    }
}

namespace {

void SendDealDamage(uint32_t targetCid, uint16_t weapon) {
    uint8_t dmg = HarpoonDamageTable::Damage(weapon);
    if (dmg == 0) return;
    nlohmann::json p = {
        { "type", HarpoonPT::COMBAT_DEAL_DAMAGE },
        { "payload", {
            { "targetCid", targetCid },
            { "damage", (int)dmg },
            { "weaponSource", (int)weapon },
        }}
    };
    Harpoon::Instance()->SendJson(p);
}

float DistSq3(float ax, float ay, float az, float bx, float by, float bz) {
    float dx = ax - bx, dy = ay - by, dz = az - bz;
    return dx * dx + dy * dy + dz * dz;
}

// Returns the weapon id that the local Link is currently using to inflict
// melee damage on contact, or HARPOON_WEAPON_NONE if not attacking.
uint16_t ResolveLocalMeleeWeapon(Player* player) {
    // meleeWeaponState > 0 indicates a swing in progress (vanilla MM convention).
    if (player->meleeWeaponState == 0) return HARPOON_WEAPON_NONE;
    switch (player->heldItemAction) {
        case PLAYER_IA_SWORD_KOKIRI:     return HARPOON_WEAPON_KOKIRI_SWORD;
        case PLAYER_IA_SWORD_RAZOR:      return HARPOON_WEAPON_RAZOR_SWORD;
        case PLAYER_IA_SWORD_GILDED:     return HARPOON_WEAPON_GILDED_SWORD;
        case PLAYER_IA_SWORD_TWO_HANDED: return HARPOON_WEAPON_GREAT_FAIRY;  // MM "two handed" == Great Fairy's Sword
        case PLAYER_IA_DEKU_STICK:       return HARPOON_WEAPON_DEKU_STICK;
        default: return HARPOON_WEAPON_NONE;
    }
}

// Returns weapon id derived from the current form's active attack state.
uint16_t ResolveLocalFormAttack(Player* player) {
    switch (player->transformation) {
        case 1: /* Goron */
            if (player->speedXZ > 8.0f) return HARPOON_WEAPON_GORON_ROLL;
            break;
        case 2: /* Zora */
            // Approximation: when zora barrier/electric is up, melee state ticks.
            if (player->meleeWeaponState > 0) return HARPOON_WEAPON_ZORA_ELECTRIC;
            break;
        case 3: /* Deku */
            if (player->meleeWeaponState > 0) return HARPOON_WEAPON_DEKU_SPIN;
            break;
        case 0: /* FD */
            if (player->meleeWeaponState > 0) return HARPOON_WEAPON_FD_BEAM;
            break;
        default: break;
    }
    return HARPOON_WEAPON_NONE;
}

}  // namespace

void HarpoonCombat_OnLocalPlayerUpdate(PlayState* play, Player* player) {
    auto* h = Harpoon::Instance();
    if (h->State() != HarpoonConnState::InRoom) return;
    if (!h->IsPvpActive()) return;  // gamemode gates PvP (e.g. geoguessr)

    uint16_t melee = ResolveLocalMeleeWeapon(player);
    uint16_t form  = ResolveLocalFormAttack(player);
    if (melee == HARPOON_WEAPON_NONE && form == HARPOON_WEAPON_NONE) return;

    constexpr float kMeleeRangeSq = 80.0f * 80.0f;
    constexpr float kFormRangeSq  = 100.0f * 100.0f;
    float px = player->actor.world.pos.x;
    float py = player->actor.world.pos.y;
    float pz = player->actor.world.pos.z;

    std::lock_guard<std::mutex> lk(h->StateMutex());
    for (const auto& [cid, c] : h->ClientsRaw()) {
        if (c.sceneId != play->sceneId) continue;
        float d2 = DistSq3(px, py, pz, c.posX, c.posY, c.posZ);
        if (melee != HARPOON_WEAPON_NONE && d2 < kMeleeRangeSq) {
            SendDealDamage(cid, melee);
        } else if (form != HARPOON_WEAPON_NONE && d2 < kFormRangeSq) {
            SendDealDamage(cid, form);
        }
    }
}
