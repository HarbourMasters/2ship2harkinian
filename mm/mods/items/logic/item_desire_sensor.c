/**
 * item_desire_sensor.c - Desire Compass (reworked from the old Desire Sensor)
 *
 * Progressive item: Stone of Agony (lvl 1) -> Desire Compass (lvl 2).
 *
 * Controls (lvl 2 compass):
 *   Hold the equipped C button  -> open the 8-category dowsing wheel
 *   Tilt the control stick      -> select one of 8 categories (Skyward-Sword style)
 *   Release the button          -> confirm; permanently spend 1 heart container
 *                                  and begin dowsing toward the nearest loaded
 *                                  check of that category in the current scene
 *   Press again while dowsing   -> reopen the wheel to re-select
 *   Press another C button / take damage / unequip -> stop dowsing
 *
 * The "brain" (which check of a category is nearest, and where) lives on the
 * C++ side in 2s2h/Rando/DesireCompass.cpp. MM rando checks carry no world
 * coordinates, so the locate only points to checks whose carrier actor is
 * loaded in the current scene (arrow-to-exit for out-of-scene targets is a
 * later phase). The radial wheel + on-screen arrow are drawn in a later phase;
 * this file drives state, input, the permanent health cost, and proximity
 * rumble so the mechanic is testable on its own.
 */

#include "z64.h"
#include "item_desire_sensor.h"
#include "../custom_items.h"
#include "../helpers/equip_helper.h"
#include "macros.h"
#include "functions.h"
#include "variables.h"

// Forward-declare the C++ brain (2s2h/Rando/DesireCompass.cpp, extern "C").
// s32 category param is ABI-compatible with the DesireCompassCategory enum;
// values 0..7 follow the wheel order: 0 BossSouls, 1 Keys, 2 OtherSouls,
// 3 Major, 4 Skills, 5 Junk, 6 Triforce, 7 Other.
u8 Rando_DesireCompass_LocateNearest(s32 cat, s32 subcat, Vec3f* outPos, f32* outDist);
s32 Rando_DesireCompass_CountRemaining(s32 cat, s32 subcat);
s32 Rando_DesireCompass_SubcategoryCount(s32 cat);
// NOTE: IS_RANDO is a hardcoded 0 fallback inside this (z_player) translation
// unit — see mods/nei_oot_compat.h. The real rando check must come from C++.
u8 Rando_DesireCompass_IsAvailable(void);

#define DCOMPASS_CATEGORY_COUNT 8

static s8 sDCPrevInvinc = 0;
static u8 sDCStickHeld = 0; // debounce so one tilt = one spoke change

static const f32 DC_PI = 3.14159265f;

static void DC_Stop(void);

// =============================================================================
// Helpers
// =============================================================================

// Map a control-stick deflection to one of 8 categories. Sector 0 = straight
// up, advancing clockwise (up-right = 1, right = 2, ...). Matches the wheel
// order documented above.
static s32 DC_CategoryFromStick(s16 sx, s16 sy) {
    f32 angle = atan2f((f32)sx, (f32)sy); // 0 = up, +x = clockwise
    if (angle < 0.0f) {
        angle += 2.0f * DC_PI;
    }
    s32 sector = (s32)((angle + (DC_PI / 8.0f)) / (DC_PI / 4.0f)) % DCOMPASS_CATEGORY_COUNT;
    return sector;
}

// Permanently spend one heart container and start dowsing. Refuses (error SFX)
// outside rando or when it would drop the capacity below the floor.
static void DC_ConfirmAndStart(Player* p, PlayState* play) {
    if (!Rando_DesireCompass_IsAvailable()) {
        Audio_PlaySoundGeneral(DCOMPASS_SE_ERROR, &p->actor.world.pos, 4, &gSfxDefaultFreqAndVolScale,
                               &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
        return;
    }

    s16 cap = gSaveContext.save.saveInfo.playerData.healthCapacity;
    if (cap - DCOMPASS_HEART_UNITS < DCOMPASS_HEALTH_FLOOR) {
        Audio_PlaySoundGeneral(DCOMPASS_SE_ERROR, &p->actor.world.pos, 4, &gSfxDefaultFreqAndVolScale,
                               &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
        // Stop any in-progress dowsing too: otherwise reopening the wheel once
        // the capacity floor is reached would switch categories for free.
        DC_Stop();
        return;
    }

    // Permanent max-health cost (user-locked design).
    cap -= DCOMPASS_HEART_UNITS;
    gSaveContext.save.saveInfo.playerData.healthCapacity = cap;
    if (gSaveContext.save.saveInfo.playerData.health > cap) {
        gSaveContext.save.saveInfo.playerData.health = cap;
    }

    dcDowsing = 1;
    dcRumbleTimer = 0;
    Audio_PlaySoundGeneral(DCOMPASS_SE_DECIDE, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                           &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
}

static void DC_Stop(void) {
    dcDowsing = 0;
    dcWheelOpen = 0;
    dcHasTarget = 0;
    dcRumbleTimer = 0;
}

// Run one frame of dowsing: locate the nearest in-scene target of the active
// category, cache its yaw/pos, and pulse the rumble by proximity.
static void DC_RunDowsing(Player* p, PlayState* play) {
    Vec3f target;
    f32 dist = 0.0f;

    if (Rando_DesireCompass_LocateNearest(dcCategory, dcSubcat, &target, &dist)) {
        dcHasTarget = 1;
        dcTargetPos = target;
        dcTargetDist = dist;
        dcTargetYaw = Math_Vec3f_Yaw(&p->actor.world.pos, &target);

        // Rumble period shrinks as the target gets closer (hotter/colder).
        f32 t = dist / DCOMPASS_RUMBLE_NEAR_DIST;
        if (t > 1.0f) {
            t = 1.0f;
        }
        s16 period =
            (s16)(DCOMPASS_RUMBLE_PERIOD_NEAR + (s16)((DCOMPASS_RUMBLE_PERIOD_FAR - DCOMPASS_RUMBLE_PERIOD_NEAR) * t));

        if (dcRumbleTimer > 0) {
            dcRumbleTimer--;
        }
        if (dcRumbleTimer <= 0) {
            func_800AA000(50.0f, 80, 8, 4);
            if (dist < DCOMPASS_RUMBLE_NEAR_DIST) {
                Audio_PlaySoundGeneral(DCOMPASS_SE_PING, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                                       &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
            }
            dcRumbleTimer = period;
        }
    } else {
        // No locatable target of this category in the current scene. Arrow-to-
        // exit fallback is a later phase; for now just report "no target".
        dcHasTarget = 0;
    }
}

// =============================================================================
// Public API
// =============================================================================

void Player_InitDesireSensorIA(PlayState* play, Player* p) {
    DC_Stop();
    dcCategory = 0;
    dcSubcat = 0;
    sDCStickHeld = 0;
}

void Handle_DesireSensor(Player* p, PlayState* play) {
    ItemInputState in;
    ItemInput_Update(&in, ITEM_DESIRE_SENSOR, p, play);

    Input* input = &play->state.input[0];

    // Unequipped -> abort everything (health cost already paid stays spent).
    if (!in.wasEquipped) {
        DC_Stop();
        sDCStickHeld = 0;
        return;
    }

    // Damage cancels dowsing/selection (cost stays spent).
    if ((dcDowsing || dcWheelOpen) && ItemInput_CheckDamage(p, &sDCPrevInvinc)) {
        DC_Stop();
        return;
    }

    // ---- Radial wheel open: selecting a category ----
    if (dcWheelOpen) {
        if (in.isHeld) {
            s16 sx = input->cur.stick_x;
            s16 sy = input->cur.stick_y;
            f32 mag = sqrtf((f32)(sx * sx + sy * sy));
            if (mag > DCOMPASS_STICK_THRESHOLD) {
                if (!sDCStickHeld) {
                    s32 sector = DC_CategoryFromStick(sx, sy);
                    if (sector != dcCategory) {
                        dcCategory = (u8)sector;
                        Audio_PlaySoundGeneral(DCOMPASS_SE_CURSOR, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                                               &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
                    }
                    sDCStickHeld = 1;
                }
            } else {
                sDCStickHeld = 0;
            }
        } else {
            // Released -> confirm the selected category.
            dcWheelOpen = 0;
            sDCStickHeld = 0;
            DC_ConfirmAndStart(p, play);
        }
        return;
    }

    // ---- Dowsing active ----
    if (dcDowsing) {
        // Another C button stops the compass.
        if (in.otherButtonPressed) {
            DC_Stop();
            Audio_PlaySoundGeneral(DCOMPASS_SE_CANCEL, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                                   &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
            return;
        }
        // Fresh press reopens the wheel to re-select (dowsing continues until a
        // new category is confirmed, which spends another heart).
        if (in.isPressed) {
            dcWheelOpen = 1;
            sDCStickHeld = 0;
            return;
        }
        DC_RunDowsing(p, play);
        return;
    }

    // ---- Idle: waiting for the player to open the wheel ----
    if (ItemInput_IsBlocked(p, play)) {
        return;
    }
    if (in.isPressed) {
        dcWheelOpen = 1;
        sDCStickHeld = 0;
    }
}

s32 Player_UpperAction_DesireSensor(Player* p, PlayState* play) {
    return 0;
}

// =============================================================================
// Accessors for the C++ HUD (2s2h/Rando/DesireCompassHud.cpp). Same pattern as
// the SM64 caps HUD: state lives here in C, the overlay reads it through these.
// =============================================================================

u8 DesireCompass_IsWheelOpen(void) {
    return dcWheelOpen;
}

u8 DesireCompass_IsDowsing(void) {
    return dcDowsing;
}

s32 DesireCompass_GetCategory(void) {
    return (s32)dcCategory;
}

u8 DesireCompass_HasTarget(void) {
    return dcHasTarget;
}

f32 DesireCompass_GetTargetDist(void) {
    return dcTargetDist;
}

void DesireCompass_GetTargetPos(Vec3f* out) {
    if (out != NULL) {
        *out = dcTargetPos;
    }
}
