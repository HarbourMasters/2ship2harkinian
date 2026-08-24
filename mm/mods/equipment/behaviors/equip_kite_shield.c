/**
 * equip_kite_shield.c - Kite Shield (Extended Shield Slot 2) — 2ship / MM
 *
 * BEHAVIOR: SHIELD SURFING (BotW). PRESS R IN MID AIR with the Kite Shield equipped (on his back
 * or in his hand, either counts) and it drops under his feet and he rides it: downhill accelerates with no speed cap,
 * uphill bleeds speed, flat bleeds it very slowly. Narrow, long floor geometry — a beam, a ledge, the Hyrule Field
 * fence tops — is a grind rail that pins him and carries him along it. A hops (and lets go of a rail), B spins, B+R
 * dismounts, C items still work.
 *
 * Port of the SoH module of the same name. State and the draw-time predicates live here; the
 * engine is mods/equipment/kite_surf.c, included late in z_player.c. See the API notes at the top
 * of that file for what had to change between the two decomps.
 *
 * MM-ONLY: gated to PLAYER_FORM_HUMAN. Deku/Goron/Zora have their own movement and skeletons, and
 * the board placement below is measured in human Link's limb space.
 *
 * Included by ext_equip_behavior.c (unity build).
 */

// Board placement under the feet, applied from ExtEquip_DrawKiteSurfBoard on the ROOT limb.
// Defined here and not in kite_surf.c because extended_equipment.c draws it, and that file is
// included long before the engine is.
//
// Same values as SoH, and that is deliberate: the right-hand limb's shield-quad space is
// {-4500,-3000,-600}..{1500,3000,-600} in MM (z_player_lib.c) — the identical 6000-unit box OoT
// uses — so the two limb spaces share a scale and the tuned numbers carry over.
#define KSURF_BOARD_SCALE 46.95f
#define KSURF_BOARD_ROT_X 51.81f
#define KSURF_BOARD_ROT_Y 47.84f
#define KSURF_BOARD_ROT_Z 13.16f
#define KSURF_BOARD_OFF_X (-126.82f)
#define KSURF_BOARD_OFF_Y (-885.38f)
#define KSURF_BOARD_OFF_Z (-225.91f)

// MM Link is one size — there is no child/adult split to scale for, so the SoH 11/17 age ratio has
// no counterpart here.

// Extra waist crouch on TOP of the riding animation, in degrees. 0 because the pose is the vanilla
// downhill-slide clip, which already bends the knees.
#define KSURF_CROUCH_DEG (-13.31f)

// Rotation of the UPPER body (torso, arms, head) and the LOWER body (both legs) while riding.
// All zero = untouched; these are the dials for the stance.
#define KSURF_UPPER_ROT_X 9.08f
#define KSURF_UPPER_ROT_Y 10.15f
#define KSURF_UPPER_ROT_Z (-11.22f)
#define KSURF_LOWER_ROT_X (-15.49f)
#define KSURF_LOWER_ROT_Y 30.44f
#define KSURF_LOWER_ROT_Z (-15.49f)

// Live lean of the torso into the turn, in degrees, on top of the static rotations.
#define KSURF_UPPER_LEAN_DEG 5.0f

// The same live turn signal for the lower body, much bigger, so the hips swing as he carves. Only
// one axis is on by default; move the 30 to whichever one reads as turning, and note that a
// NEGATIVE value is a normal answer — this limb's space does not map the way you would guess.
#define KSURF_LOWER_TURN_X 30.0f
#define KSURF_LOWER_TURN_Y 0.0f
#define KSURF_LOWER_TURN_Z 0.0f

typedef enum {
    /* 0 */ KSURF_OFF,
    /* 1 */ KSURF_MOUNT,    // playing the equip animation, board coming out
    /* 2 */ KSURF_RIDE,     // free riding
    /* 3 */ KSURF_RAIL,     // pinned to the centre of a narrow floor strip
    /* 4 */ KSURF_DISMOUNT, // getting off, control handed back next frame
} KiteSurfState;

typedef struct {
    /* state machine */
    u8 state;
    s16 timer;
    /* rider */
    s16 stopFrames; // consecutive grounded frames under KSURF_STOP_SPEED
    s16 spinFrames; // >0 while a spin attack owns the animation (pause released)
    s16 leanPitch;  // smoothed floor pitch along the heading, applied to shape + limbs
    s16 leanRoll;   // smoothed turn lean of the waist
    s16 upperLean;  // smaller live lean of the torso into the turn
    f32 turn;       // -1..+1, how hard and which way he is carving; drives the lower-body swing
    /* rail */
    s16 railAxisYaw;
    s16 railMiss;   // consecutive frames a probe came back empty
    s16 railDetach; // >0 suppresses rail detection after A let go of one
    /* board trick spin */
    s16 boardSpin;
    s16 boardSpinRate;
    /* takeover bookkeeping */
    void* ownedAction; // actionFunc at entry; a change means damage/cutscene stole the player
} KiteSurfCtx;

static KiteSurfCtx sKSurf = { KSURF_OFF };

// True whenever the surf owns the player at all (mount and dismount included).
// Read by func_80836F10 (fall damage) and by the walk-off handler in z_player.c.
u8 KiteSurf_IsActive(void) {
    return sKSurf.state != KSURF_OFF;
}

// True only while actually riding — the board is under his feet and the hand/back shield must not
// draw. Read by ExtEquip_DrawShieldCommon.
u8 KiteSurf_IsRiding(void) {
    return (sKSurf.state == KSURF_RIDE) || (sKSurf.state == KSURF_RAIL);
}

// Crouch, stance and carve of the body over the board.
//
// Applied at DRAW time from the limb callback and NOT by writing skelAnime.jointTable from the
// update hook: PlayerAnimation_Update only QUEUES the joint fill into the animation context, which
// is processed after every actor has updated — anything the update hook wrote would be overwritten
// before it was ever drawn.
void KiteSurf_AdjustLimb(s32 limbIndex, Vec3s* rot) {
    if (!KiteSurf_IsRiding() || (rot == NULL)) {
        return;
    }
    if (sKSurf.spinFrames > 0) {
        // Hands off during a spin attack. The surf does NOT end for it — the board keeps drawing
        // and he keeps his speed — but the spin clip has to turn him cleanly, and the board hangs
        // off the ROOT limb so it comes round with him.
        return;
    }

    if (limbIndex == PLAYER_LIMB_WAIST) {
        rot->x += (s16)(KSURF_CROUCH_DEG * 182.04f) + (s16)(sKSurf.leanPitch / 2);
        rot->z += sKSurf.leanRoll;
    } else if (limbIndex == PLAYER_LIMB_LOWER_ROOT) {
        // Bottom half — this limb carries both legs. Static pose plus the live carve, where
        // sKSurf.turn is -1..+1 with how hard he is turning and which way.
        rot->x += (s16)((KSURF_LOWER_ROT_X + (KSURF_LOWER_TURN_X * sKSurf.turn)) * 182.04f);
        rot->y += (s16)((KSURF_LOWER_ROT_Y + (KSURF_LOWER_TURN_Y * sKSurf.turn)) * 182.04f);
        rot->z += (s16)((KSURF_LOWER_ROT_Z + (KSURF_LOWER_TURN_Z * sKSurf.turn)) * 182.04f);
    } else if (limbIndex == PLAYER_LIMB_UPPER_ROOT) {
        rot->x += (s16)(KSURF_UPPER_ROT_X * 182.04f);
        rot->y += (s16)(KSURF_UPPER_ROT_Y * 182.04f);
        // Z carries the live lean into the turn on top of its static setting.
        rot->z += (s16)(KSURF_UPPER_ROT_Z * 182.04f) + sKSurf.upperLean;
    }
}

// Defined in mods/equipment/kite_surf.c (included late in z_player.c).
extern void KiteSurf_Tick(Player* player, PlayState* play);
extern void KiteSurf_Abort(Player* player);

// Deliberately EMPTY. The ext-equipment dispatcher runs BEFORE Player_UpdateCommon in MM, and the
// surf needs everything that function sets up — the control input pointer, the processed control
// stick, and a pause flag that survives to the gate. So the real per-frame driver is
// KiteSurf_PostUpdate, called from Player_Update right after Player_UpdateCommon. This slot stays
// so the dispatch table and the _Cleanup pairing keep their shape.
static void KiteShield_Behavior(Player* player, PlayState* play) {
    (void)player;
    (void)play;
}

// Called when the Kite Shield is unequipped — hand the player back mid-ride.
static void KiteShield_Cleanup(void) {
    if (sKSurf.state == KSURF_OFF) {
        return;
    }
    if (gPlayState != NULL) {
        KiteSurf_Abort(GET_PLAYER(gPlayState));
    }
    sKSurf.state = KSURF_OFF;
}
