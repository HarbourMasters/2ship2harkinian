/*
 * _nei_dmg_compat.h — OoT-only DMG_* damage-flag names → MM equivalents.
 *
 * The copied NEI code (transformation_masks form engine, some item colliders)
 * uses DMG_* names that exist in OoT's z64collision_check.h but NOT MM's. Each is
 * mapped to the closest MM damage type so colliders compile and inflict a sane
 * damage class. These are RUNTIME approximations — retune if a weapon/form does
 * wrong damage. All guarded so they never clobber a real MM DMG_* macro.
 *
 * Token substitution is deferred to the use site (which includes the MM collision
 * header), so this header does not itself need z64collision_check.h.
 */
#ifndef NEI_DMG_COMPAT_H
#define NEI_DMG_COMPAT_H

/* ── Sword-family slashes → MM sword bit ─────────────────────────────────── */
#ifndef DMG_SLASH
#define DMG_SLASH DMG_SWORD
#endif
#ifndef DMG_SLASH_KOKIRI
#define DMG_SLASH_KOKIRI DMG_SWORD
#endif
#ifndef DMG_SLASH_MASTER
#define DMG_SLASH_MASTER DMG_SWORD
#endif
#ifndef DMG_SLASH_GIANT
#define DMG_SLASH_GIANT DMG_SWORD
#endif
#ifndef DMG_JUMP_KOKIRI
#define DMG_JUMP_KOKIRI DMG_SWORD
#endif
#ifndef DMG_JUMP_GIANT
#define DMG_JUMP_GIANT DMG_SWORD
#endif
#ifndef DMG_DEFAULT
#define DMG_DEFAULT DMG_SWORD
#endif

/* ── Spin attacks → MM spin attack ───────────────────────────────────────── */
#ifndef DMG_SPIN_KOKIRI
#define DMG_SPIN_KOKIRI DMG_SPIN_ATTACK
#endif
#ifndef DMG_SPIN_MASTER
#define DMG_SPIN_MASTER DMG_SPIN_ATTACK
#endif
#ifndef DMG_SPIN_GIANT
#define DMG_SPIN_GIANT DMG_SPIN_ATTACK
#endif

/* ── Hammer (MM has no hammer) → Goron attacks (per user): swing=punch, floor/jump=pound ─ */
#ifndef DMG_HAMMER_SWING
#define DMG_HAMMER_SWING DMG_GORON_PUNCH
#endif
#ifndef DMG_HAMMER_JUMP
#define DMG_HAMMER_JUMP DMG_GORON_POUND
#endif

/* ── Jump kick (Zora/FD aerial) → Zora punch (per mod comments) ───────────── */
#ifndef DMG_JUMP_MASTER
#define DMG_JUMP_MASTER DMG_ZORA_PUNCH
#endif

/* ── Boomerang → MM Zora boomerang (same bit 0x04) ────────────────────────── */
#ifndef DMG_BOOMERANG
#define DMG_BOOMERANG DMG_ZORA_BOOMERANG
#endif

/* ── Magic spells → matching elemental-arrow bits ────────────────────────── */
#ifndef DMG_MAGIC_FIRE
#define DMG_MAGIC_FIRE DMG_FIRE_ARROW
#endif
#ifndef DMG_MAGIC_ICE
#define DMG_MAGIC_ICE DMG_ICE_ARROW
#endif
#ifndef DMG_MAGIC_LIGHT
#define DMG_MAGIC_LIGHT DMG_LIGHT_ARROW
#endif

/* ── Arrow / ranged variants ─────────────────────────────────────────────── */
#ifndef DMG_ARROW_NORMAL
#define DMG_ARROW_NORMAL DMG_NORMAL_ARROW
#endif
#ifndef DMG_ARROW_FIRE
#define DMG_ARROW_FIRE DMG_FIRE_ARROW
#endif
#ifndef DMG_ARROW_ICE
#define DMG_ARROW_ICE DMG_ICE_ARROW
#endif
#ifndef DMG_ARROW_LIGHT
#define DMG_ARROW_LIGHT DMG_LIGHT_ARROW
#endif
#ifndef DMG_ARROW_UNK3
#define DMG_ARROW_UNK3 DMG_NORMAL_ARROW
#endif
#ifndef DMG_SLINGSHOT
#define DMG_SLINGSHOT DMG_DEKU_BUBBLE /* per user: slingshot ≈ Deku bubble projectile */
#endif

/* ── Misc ────────────────────────────────────────────────────────────────── */
#ifndef DMG_EXPLOSIVE
#define DMG_EXPLOSIVE DMG_EXPLOSIVES
#endif
#ifndef DMG_FIXED_DAMAGE
#define DMG_FIXED_DAMAGE DMG_UNBLOCKABLE
#endif

#endif /* NEI_DMG_COMPAT_H */
