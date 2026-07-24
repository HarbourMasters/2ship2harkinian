/**
 * item_switchhook.h - Switch Hook from Oracle of Ages
 *
 * Controls (hold-to-aim like Bomb Arrows):
 *   Hold C Button:   First-person aiming mode
 *   Release C:       Fire hook projectile
 *   Z-targeting:     Third-person aiming at target
 *
 * Features:
 *   - Swaps positions with swappable actors (pots, crates, certain enemies)
 *   - Deals hookshot damage to non-swappable actors and bounces back
 *   - Uses targeted actor (Z-target) for instant swap when available
 *   - Longshot distance (26 frames)
 *   - Usable by both child and adult Link
 *   - Hook tip rotated 180 degrees (reversed appearance)
 *   - Blue reticle during aiming (like Gust Jar suck mode)
 */

#ifndef ITEM_SWITCHHOOK_H
#define ITEM_SWITCHHOOK_H

#include "z64.h"
#include "../custom_items.h"

// ============================================================================
// CONFIGURATION
// ============================================================================

#define SWITCHHOOK_TIMER 13       // Skijer's NEI: Hookshot distance = 1 (was 26 = Longshot). 20 * 13
#define SWITCHHOOK_SPEED 20.0f    // Projectile speed
#define SWITCHHOOK_SWAP_FRAMES 15 // Frames for swap animation
#define SWITCHHOOK_DAMAGE 1       // Damage dealt to non-swappable actors

// ============================================================================
// STATES
// ============================================================================

#define SWITCHHOOK_STATE_IDLE 0       // Waiting for input
#define SWITCHHOOK_STATE_AIMING 1     // First-person aiming (hold C button)
#define SWITCHHOOK_STATE_SHOOTING 2   // Projectile flying forward
#define SWITCHHOOK_STATE_HIT_SWAP 3   // Hit swappable actor, performing swap
#define SWITCHHOOK_STATE_HIT_DAMAGE 4 // Hit non-swappable actor, damage + bounce
#define SWITCHHOOK_STATE_RETRACT 5    // Retracting back to player

// ============================================================================
// STATE ALIASES (shortcuts to gCustomItemState fields)
// ============================================================================

#define shActive gCustomItemState.switchHookActive
#define shState gCustomItemState.switchHookState
#define shFirstPerson gCustomItemState.switchHookFirstPerson
#define shProjPos gCustomItemState.switchHookProjPos
#define shProjYaw gCustomItemState.switchHookProjYaw
#define shProjPitch gCustomItemState.switchHookProjPitch
#define shTimer gCustomItemState.switchHookTimer
#define shTarget gCustomItemState.switchHookTarget
#define shLinkStartPos gCustomItemState.switchHookLinkStartPos
#define shTargetStartPos gCustomItemState.switchHookTargetStartPos
#define shSwapTimer gCustomItemState.switchHookSwapTimer
#define shCollider gCustomItemState.switchHookCollider
#define shButtonMask gCustomItemState.switchHookButtonMask
#define shVortexTimer gCustomItemState.switchHookVortexTimer

// ============================================================================
// ACTOR FLAG (for custom actors like Somaria Cubes)
// ============================================================================

#ifndef ACTOR_FLAG_SWITCHHOOKABLE
#define ACTOR_FLAG_SWITCHHOOKABLE (1 << 28)
#endif

// ============================================================================
// COLLIDER INIT
// ============================================================================

static ColliderQuadInit sSwitchHookQuadInit = {
    {
        COL_MATERIAL_NONE,
        AT_ON | AT_TYPE_PLAYER,
        AC_NONE,
        OC1_NONE,
        OC2_TYPE_PLAYER,
        COLSHAPE_QUAD,
    },
    {
        ELEM_MATERIAL_UNK2,
        { 0x00000080, 0x00, 0x01 }, // Hookshot damage flags
        { 0xFFCFFFFF, 0x00, 0x00 },
        ATELEM_ON | ATELEM_NEAREST | ATELEM_SFX_NORMAL,
        ACELEM_NONE,
        OCELEM_NONE,
    },
    { { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } } },
};

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Check if an actor can be swapped with.
 * @param actor The actor to check
 * @return 1 if swappable, 0 otherwise
 */
static inline s32 SwitchHook_CanSwap(Actor* actor) {
    if (actor == NULL)
        return 0;

    // Check custom flag first (for Somaria Cubes, etc.)
    if (actor->flags & ACTOR_FLAG_SWITCHHOOKABLE) {
        return 1;
    }

    // Skijer's NEI switchhook overhaul: no more hand-picked item list — switch positions with ANY
    // prop (pots, crates, torches, beehives...) or NON-BOSS enemy. ACTORCAT_ENEMY excludes bosses;
    // props/enemies are the only categories that swap — player, NPCs, background, items are ignored.
    if (actor->category == ACTORCAT_ENEMY || actor->category == ACTORCAT_PROP) {
        return 1;
    }

    return 0;
}

#endif // ITEM_SWITCHHOOK_H
