#ifndef HARPOON_COMBAT_SYNC_H
#define HARPOON_COMBAT_SYNC_H

#include <cstdint>

// -----------------------------------------------------------------------------
// Damage response codes.
//
// These travel on the wire as COMBAT.DEAL_DAMAGE's "damageEffect" and are the
// SAME values SoH uses (soh Harpoon.h HarpoonDamageResponseType), so a shared
// relay/gamemode treats both ports identically.
//
// HARD LIMIT: these are stored in the effect NIBBLE of a MM DamageTable entry
// (DMG_ENTRY(damage, effect) packs effect into bits 4-7, and
// CollisionCheck_GetDamageAndEffectOnElementAC reads it back as
// `(attack[i] >> 4) & 0xF`). Anything above 15 is silently truncated.
// -----------------------------------------------------------------------------
enum HarpoonHitResponse : uint8_t {
    HARPOON_HIT_NONE = 0,
    HARPOON_HIT_KNOCKBACK_LARGE = 1,
    HARPOON_HIT_KNOCKBACK_SMALL = 2,
    HARPOON_HIT_FROZEN = 3,
    HARPOON_HIT_ELECTRIFIED = 4,
    HARPOON_HIT_STUN = 5,
    HARPOON_HIT_FIRE = 6,
    HARPOON_HIT_NORMAL = 7,
    HARPOON_HIT_WIND_BLOW = 8,
    HARPOON_HIT_LIGHT = 9,
    HARPOON_HIT_DARK = 10,
    HARPOON_HIT_SOUL_DRAIN = 11,
    HARPOON_HIT_WIND_PUSH = 12,
};

// Harpoon weapon IDs — MM-only subset. The high byte categorises the source:
//   0x00xx vanilla items, 0x02xx form attacks, 0x06xx masks.
//
// NOTE: since peers became real actors carrying an AC ColliderCylinder, melee /
// form / custom-item hits are resolved by the ENGINE (dmgFlags -> peer damage
// table -> damage + response) and no longer go through this enum. It stays for
// the projectile mirror and for anything that has to name a weapon explicitly.
enum HarpoonWeaponId : uint16_t {
    HARPOON_WEAPON_NONE = 0x0000,

    // Vanilla MM melee.
    HARPOON_WEAPON_KOKIRI_SWORD = 0x0001,
    HARPOON_WEAPON_RAZOR_SWORD = 0x0002,
    HARPOON_WEAPON_GILDED_SWORD = 0x0003,
    HARPOON_WEAPON_GREAT_FAIRY = 0x0004,
    HARPOON_WEAPON_DEKU_STICK = 0x0005,

    // Vanilla MM projectiles / explosives.
    HARPOON_WEAPON_BOMB = 0x0010,
    HARPOON_WEAPON_BOMBCHU = 0x0011,
    HARPOON_WEAPON_POWDER_KEG = 0x0012,
    HARPOON_WEAPON_ARROW = 0x0020,
    HARPOON_WEAPON_FIRE_ARROW = 0x0021,
    HARPOON_WEAPON_ICE_ARROW = 0x0022,
    HARPOON_WEAPON_LIGHT_ARROW = 0x0023,
    HARPOON_WEAPON_HOOKSHOT = 0x0030,

    // Form attacks.
    HARPOON_WEAPON_GORON_ROLL = 0x0200,
    HARPOON_WEAPON_GORON_SPIKE = 0x0201,
    HARPOON_WEAPON_ZORA_ELECTRIC = 0x0210,
    HARPOON_WEAPON_ZORA_FIN = 0x0211,
    HARPOON_WEAPON_DEKU_SPIN = 0x0220,
    HARPOON_WEAPON_DEKU_BUBBLE = 0x0221,
    HARPOON_WEAPON_FD_BEAM = 0x0230,
    HARPOON_WEAPON_FD_SPIN = 0x0231,

    // Mask weapons.
    HARPOON_WEAPON_BLAST_MASK = 0x0600,
};

struct HarpoonDamageTable {
    // Damage in engine health units (16 == one heart, see Health_ChangeBy).
    static uint8_t Damage(uint16_t weapon);
};

// Send COMBAT.DEAL_DAMAGE. `damage` is in DamageTable units (quarter hearts,
// what CollisionCheck writes into colChkInfo.damage); the receiver scales it.
// Called from HarpoonPeer_Update when our attack lands on a peer's AC cylinder.
void HarpoonCombat_SendDamage(uint32_t targetClientId, uint8_t damageEffect, uint8_t damage);

#endif
