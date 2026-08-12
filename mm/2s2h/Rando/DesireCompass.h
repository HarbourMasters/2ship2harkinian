#ifndef RANDO_DESIRE_COMPASS_H
#define RANDO_DESIRE_COMPASS_H

// =============================================================================
// Quartz of Motion — level 2 of the progressive Stone of Agony.
//
// It is a SENSOR, not a compass: it never points anywhere. Pick a category from
// the kaleido (A on the Stone of Agony slot of the OoT quest page), pay 3
// hearts, and for the next 5 minutes it gives two signals, Sheikah-Sensor
// style:
//
//   1. "There is something here" — an on-screen indicator whenever the room you
//      just walked into holds an uncollected check of your category.
//   2. "You are getting closer" — a blip + rumble whose rate rises as you near
//      it, silent when there is nothing loaded.
//
// Detection is purely "is the carrier actor loaded right now", which means the
// current room. No routing, no world graph, no room tables.
//
// C-safe header: the kaleido (.c) calls these extern "C" entry points.
// =============================================================================

#ifdef __cplusplus
extern "C" {
#endif

#include "z64math.h"

// The 8 tracking categories, in kaleido-list order.
typedef enum {
    DCOMPASS_CAT_BOSS_SOULS = 0, // RI_SOUL_BOSS_*
    DCOMPASS_CAT_KEYS,           // RITYPE_SMALL_KEY / RITYPE_BOSS_KEY
    DCOMPASS_CAT_OTHER_SOULS,    // RI_SOUL_ENEMY_*
    DCOMPASS_CAT_MAJOR,          // RITYPE_MAJOR/MASK, minus souls/triforce/skills
    DCOMPASS_CAT_SKILLS,         // RI_ABILITY_*
    DCOMPASS_CAT_JUNK,           // RITYPE_JUNK
    DCOMPASS_CAT_TRIFORCE,       // RI_TRIFORCE_PIECE(_PREVIOUS)
    DCOMPASS_CAT_OTHER,          // everything else
    DCOMPASS_CAT_MAX
} DesireCompassCategory;

#define DCOMPASS_SUBCAT_ANY 0

#define DCOMPASS_DURATION_SECONDS 300 // 5 minutes
// Cost: 3 hearts of CURRENT health (0x10 units per heart) — the max-health
// capacity is never touched.
#define DCOMPASS_HEALTH_COST 0x30

// --- Queries -----------------------------------------------------------------

const char* Rando_DesireCompass_CategoryName(DesireCompassCategory cat);
s32 Rando_DesireCompass_SubcategoryCount(DesireCompassCategory cat);
s32 Rando_DesireCompass_CountRemaining(DesireCompassCategory cat, s32 subcat);
u8 Rando_DesireCompass_IsAvailable(void); // in a rando save?
u8 Rando_DesireCompass_IsOwned(void);     // Quartz obtained?

// --- Activation --------------------------------------------------------------

// Validate and queue an activation. Returns 1 if accepted — the caller (kaleido)
// should then close the pause menu. Nothing is charged yet: once gameplay
// resumes the tick plays a short attuning animation, spends the 3 hearts, and
// starts the sensor. Returns 0 if refused, leaving the list open.
u8 Rando_DesireCompass_RequestActivation(DesireCompassCategory cat, s32 subcat);
void Rando_DesireCompass_Cancel(void);
u8 Rando_DesireCompass_IsAttuning(void);

// --- Active-session state (read by the HUD) ----------------------------------

u8 Rando_DesireCompass_IsActive(void);
s32 Rando_DesireCompass_GetRemainingSeconds(void);
DesireCompassCategory Rando_DesireCompass_GetActiveCategory(void);

// Signal 1: this room holds an uncollected check of the tracked category.
u8 Rando_DesireCompass_RoomHasTarget(void);

// Frames left on the "you just entered a room with something" flash (0 = idle).
s32 Rando_DesireCompass_RoomAlertFrames(void);

// Signal 2: proximity, 0.0 (far / nothing) .. 1.0 (right on top of it).
f32 Rando_DesireCompass_GetProximity(void);

#ifdef __cplusplus
}
#endif

#endif // RANDO_DESIRE_COMPASS_H
