#ifndef HARPOON_COMBAT_SYNC_H
#define HARPOON_COMBAT_SYNC_H

#include <cstdint>

// Harpoon weapon IDs — MM-only subset. The high byte categorises the source:
//   0x00xx vanilla items, 0x02xx form attacks, 0x06xx masks.
// Keep numeric values in sync with Harpoon SoH for cross-port compatibility.

enum HarpoonWeaponId : uint16_t {
    HARPOON_WEAPON_NONE         = 0x0000,

    // Vanilla MM melee.
    HARPOON_WEAPON_KOKIRI_SWORD = 0x0001,
    HARPOON_WEAPON_RAZOR_SWORD  = 0x0002,
    HARPOON_WEAPON_GILDED_SWORD = 0x0003,
    HARPOON_WEAPON_GREAT_FAIRY  = 0x0004,
    HARPOON_WEAPON_DEKU_STICK   = 0x0005,

    // Vanilla MM projectiles / explosives.
    HARPOON_WEAPON_BOMB         = 0x0010,
    HARPOON_WEAPON_BOMBCHU      = 0x0011,
    HARPOON_WEAPON_POWDER_KEG   = 0x0012,
    HARPOON_WEAPON_ARROW        = 0x0020,
    HARPOON_WEAPON_FIRE_ARROW   = 0x0021,
    HARPOON_WEAPON_ICE_ARROW    = 0x0022,
    HARPOON_WEAPON_LIGHT_ARROW  = 0x0023,
    HARPOON_WEAPON_HOOKSHOT     = 0x0030,

    // Form attacks.
    HARPOON_WEAPON_GORON_ROLL        = 0x0200,
    HARPOON_WEAPON_GORON_SPIKE       = 0x0201,
    HARPOON_WEAPON_ZORA_ELECTRIC     = 0x0210,
    HARPOON_WEAPON_ZORA_FIN          = 0x0211,
    HARPOON_WEAPON_DEKU_SPIN         = 0x0220,
    HARPOON_WEAPON_DEKU_BUBBLE       = 0x0221,
    HARPOON_WEAPON_FD_BEAM           = 0x0230,
    HARPOON_WEAPON_FD_SPIN           = 0x0231,

    // Mask weapons.
    HARPOON_WEAPON_BLAST_MASK   = 0x0600,
};

struct HarpoonDamageTable {
    static uint8_t Damage(uint16_t weapon);  // heart count (4 per heart)
};

#ifdef __cplusplus
struct PlayState_;
struct Player_;
typedef struct PlayState PlayState;
typedef struct Player Player;
void HarpoonCombat_OnLocalPlayerUpdate(PlayState* play, Player* player);
#endif

#endif
