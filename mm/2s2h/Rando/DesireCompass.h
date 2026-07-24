#ifndef RANDO_DESIRE_COMPASS_H
#define RANDO_DESIRE_COMPASS_H

// =============================================================================
// Desire Compass — the "brain" for the progressive Stone of Agony -> Desire
// Compass item. Given one of 8 categories (+ optional subcategory), it locates
// the nearest UNCOLLECTED rando check of that category whose actor is currently
// loaded in the scene, by iterating the live actor lists and resolving each
// actor to its RandoCheckId (see DesireCompass.cpp for the recognizer).
//
// MM rando checks carry no world coordinates, so "locate" only works for
// checks whose carrier actor is loaded in the CURRENT scene. Out-of-scene
// targeting (arrow-to-exit) is layered on top in the item logic, not here.
//
// This header is C-safe: the item logic (item_desire_sensor.c, a .c file
// compiled as part of the z_player translation unit) forward-declares and
// calls these extern "C" entry points.
// =============================================================================

#ifdef __cplusplus
extern "C" {
#endif

#include "z64math.h"

// The 8 dowsing-wheel categories (Skyward-Sword style radial menu order).
typedef enum {
    DCOMPASS_CAT_BOSS_SOULS = 0, // RI_SOUL_BOSS_*
    DCOMPASS_CAT_KEYS,           // RITYPE_SMALL_KEY / RITYPE_BOSS_KEY
    DCOMPASS_CAT_OTHER_SOULS,    // RI_SOUL_ENEMY_*
    DCOMPASS_CAT_MAJOR,          // RITYPE_MAJOR/MASK, minus souls/triforce/skills
    DCOMPASS_CAT_SKILLS,         // RI_ABILITY_*
    DCOMPASS_CAT_JUNK,           // RITYPE_JUNK
    DCOMPASS_CAT_TRIFORCE,       // RI_TRIFORCE_PIECE(_PREVIOUS)
    DCOMPASS_CAT_OTHER,          // everything else (lesser/health/token/fairy...)
    DCOMPASS_CAT_MAX
} DesireCompassCategory;

// Subcategory 0 always means "any within the category". Category-specific
// subcategories (currently only Keys: 1 = small keys, 2 = boss keys) are
// documented in DesireCompass.cpp / DCompass_SubcategoryCount.
#define DCOMPASS_SUBCAT_ANY 0

// Number of meaningful subcategories for a category (>= 1). The wheel's 2nd
// level uses this to know how many spokes to draw.
s32 Rando_DesireCompass_SubcategoryCount(DesireCompassCategory cat);

// Is the compass usable at all (i.e. are we in a rando save)? The item logic
// lives in the z_player C translation unit, where IS_RANDO is a hardcoded 0
// fallback (mods/nei_oot_compat.h) — so the real check has to happen here.
u8 Rando_DesireCompass_IsAvailable(void);

// Locate the nearest loaded, uncollected check of (cat, subcat) in the current
// scene. Returns 1 and fills *outPos (+ *outDist if non-NULL, XZ distance to
// the player) when a target is found; returns 0 otherwise.
u8 Rando_DesireCompass_LocateNearest(DesireCompassCategory cat, s32 subcat, Vec3f* outPos, f32* outDist);

// Count of still-shuffled, uncollected items of (cat, subcat) across the whole
// seed (not just the current scene). Drives the L1 "Stone of Agony" rumble
// sense and can label the wheel spokes. Returns 0 when none remain.
s32 Rando_DesireCompass_CountRemaining(DesireCompassCategory cat, s32 subcat);

#ifdef __cplusplus
}
#endif

#endif // RANDO_DESIRE_COMPASS_H
