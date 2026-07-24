#ifndef ITEM_DESIRE_SENSOR_H
#define ITEM_DESIRE_SENSOR_H

#include "../custom_items.h"

// =============================================================================
// Desire Compass (reworked from the old Desire Sensor). Progressive item:
// Stone of Agony (lvl 1, basic "something is in this scene" sense) -> Desire
// Compass (lvl 2, directional dowsing wheel). Holding the item button opens an
// 8-category radial wheel; releasing confirms a category, permanently spends
// one heart container, and begins dowsing toward the nearest loaded check of
// that category in the current scene. See item_desire_sensor.c + the C++ brain
// 2s2h/Rando/DesireCompass.{h,cpp}.
// =============================================================================

// Health cost model (user-locked): each activation permanently reduces MAX
// health by one heart (0x10 units). Refuse to activate if it would drop the
// capacity below the floor.
#define DCOMPASS_HEART_UNITS 0x10
#define DCOMPASS_HEALTH_FLOOR 0x10 // never reduce capacity below 1 heart

// Radial wheel: stick magnitude past this (out of ~60) counts as a tilt toward
// a category spoke.
#define DCOMPASS_STICK_THRESHOLD 30.0f

// Proximity rumble: base pulse period (frames) at max distance; scales down as
// the target gets closer.
#define DCOMPASS_RUMBLE_PERIOD_FAR 24
#define DCOMPASS_RUMBLE_PERIOD_NEAR 4
#define DCOMPASS_RUMBLE_NEAR_DIST 200.0f // XZ units treated as "on top of it"

// Sound effects
#define DCOMPASS_SE_CURSOR NA_SE_SY_CURSOR       // wheel spoke change
#define DCOMPASS_SE_DECIDE NA_SE_SY_DECIDE       // category confirmed / dowsing on
#define DCOMPASS_SE_CANCEL NA_SE_SY_CANCEL       // dowsing off
#define DCOMPASS_SE_ERROR NA_SE_SY_ERROR         // not rando / capacity floor hit
#define DCOMPASS_SE_PING NA_SE_SY_CORRECT_CHIME  // proximity ping when close

// State aliases (map to CustomItemState fields)
#define dcDowsing gCustomItemState.desireCompassDowsing
#define dcWheelOpen gCustomItemState.desireCompassWheelOpen
#define dcCategory gCustomItemState.desireCompassCategory
#define dcSubcat gCustomItemState.desireCompassSubcat
#define dcHasTarget gCustomItemState.desireCompassHasTarget
#define dcTargetYaw gCustomItemState.desireCompassTargetYaw
#define dcTargetDist gCustomItemState.desireCompassTargetDist
#define dcTargetPos gCustomItemState.desireCompassTargetPos
#define dcRumbleTimer gCustomItemState.desireCompassRumbleTimer

#endif // ITEM_DESIRE_SENSOR_H
