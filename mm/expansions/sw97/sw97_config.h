/**
 * sw97_config.h - CVar definitions for SW97 Medallion Spells
 *
 * Original actors: z64proto/sw97 team (Spaceworld '97 Experience)
 * Adapted for Ship of Harkinian (Shipwright)
 */
#ifndef SW97_CONFIG_H
#define SW97_CONFIG_H

// Single CVar toggle — enables medallion spell/arrow equipping
#define SW97_MEDALLIONS_CVAR "gEnhancements.SkijerNEI.SW97Medallions"
#define SW97_MEDALLIONS_ENABLED() CVarGetInteger(SW97_MEDALLIONS_CVAR, 0)

// SW97 elemental-arrow EnArrow `params` values. NEI slingshot pass: renumbered to match the
// REAL ArrowType enum in z_en_arrow.h — 0x09..0x0E are now ARROW_TYPE_SEED_FIRE..WIND (elemental
// slingshot seeds) and the SW97 bow arrows moved to 0x0F..0x14 (ARROW_TYPE_SW97_FIRE..WIND).
// Raw values (not enum refs) so this header stays include-order independent; keep in sync.
#ifndef ARROW_SW97_FIRE
#define ARROW_SW97_FIRE  0x0F
#define ARROW_SW97_ICE   0x10
#define ARROW_SW97_LIGHT 0x11
#define ARROW_SW97_DARK  0x12
#define ARROW_SW97_SOUL  0x13
#define ARROW_SW97_WIND  0x14
#endif

#endif // SW97_CONFIG_H
