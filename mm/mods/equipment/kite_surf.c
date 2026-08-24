/**
 * kite_surf.c — Kite Shield SHIELD SURFING engine. 2ship / MM port of the SoH module.
 *
 * State and the draw-time predicates live in mods/equipment/behaviors/equip_kite_shield.c, which
 * rides the ext-equipment unity build at the TOP of z_player.c. This file is included MUCH later,
 * right after Player_UpdateCommon, because everything it drives the player with is defined in
 * between: Actor_GetSlopeDirection, Player_GetMovementSpeedAndYaw, Player_ProcessItemButtons,
 * func_80833864, the Player_ActionHandler_* family and the GET_PLAYER_ANIM table.
 *
 * WHAT CHANGED FROM THE SoH VERSION — the two that bite silently are the first pair, because the
 * names are CROSSED rather than merely different:
 *   · this->linearVelocity (intent)   -> this->speedXZ
 *   · this->actor.speedXZ (momentum)  -> this->actor.speed
 *   · sControlInput -> sPlayerControlInput      · Player_SetupAction -> Player_SetAction
 *   · Player_GetSlopeDirection -> Actor_GetSlopeDirection
 *   · LinkAnimation_* -> PlayerAnimation_*      · func_80837948 -> func_80833864
 *   · Player_HoldsTwoHandedWeapon -> Player_IsHoldingTwoHandedWeapon
 *   · Player_PlaySfx takes a Player*, not an Actor*
 *   · PLAYER_STATE3_PAUSE_ACTION_FUNC is SoH's invention; in MM the action-func gate is
 *     PLAYER_STATE3_4 (sm64_mario.c already aliases it, and this file does the same).
 *   · MM has NO PLAYER_STATE3_MIDAIR. OoT used it to stop the walk-off handler dragging an
 *     airborne Link into the fall action; MM's handler instead exempts specific action funcs BY
 *     IDENTITY, so the surf is exempted the same way BossRemains already is — see the
 *     KiteSurf_IsActive() line next to BossRemains_IsGohtCharging() in z_player.c.
 *
 * TAKEOVER CONTRACT (unchanged in substance from SoH):
 *   · The pause flag is re-asserted every frame; the engine clears it each update.
 *   · While paused NOTHING advances skelAnime — this file calls PlayerAnimation_Update itself.
 *   · speedXZ + yaw written here become real world velocity next frame either way; the engine
 *     keeps doing gravity and scene collision, which is what we want.
 *   · Player_SetAction is NOT gated by the pause. actionFunc changing under us means damage or a
 *     cutscene took the player — abort and let it run.
 */

#ifndef PLAYER_STATE3_PAUSE_ACTION_FUNC
#define PLAYER_STATE3_PAUSE_ACTION_FUNC PLAYER_STATE3_4
#endif

// ---------------------------------------------------------------------------
// Tunables — IMMUTABLE (2026-08-20), and identical to SoH's. These came out of tuning in game there;
// the CVar reads that used to wrap them are gone in both repos, so nothing at runtime can move them
// and the two games cannot drift apart through a stale config.
// ---------------------------------------------------------------------------

#define KSURF_MOUNT_FRAMES 8
#define KSURF_SLOPE_ACCEL 18.28f // multiplied by (1 - floorNormal.y) and by the downhill alignment
#define KSURF_FRICTION 0.1f      // per frame toward 0 — flat bleeds speed very slowly
#define KSURF_STICK_ACCEL 0.09f  // the small shove that stops him getting stuck on flat ground
#define KSURF_TURN_MAX 1310.98f
#define KSURF_TURN_MIN 250.0f
#define KSURF_STOP_SPEED 0.6f
#define KSURF_STOP_FRAMES 10
#define KSURF_HOP_VEL 9.0f
#define KSURF_SPIN_FRAMES 24
#define KSURF_SPIN_CHANCE 0.552f      // odds a hop throws a board shuvit
#define KSURF_BOARD_SPIN_RATE 3000.0f // binang per frame — a full turn in about 22 frames

// Rail = a narrow, LONG piece of floor. Defaults MEASURED off spot00's collision in OoT (the
// Hyrule Field fences): the long fence is 20 units wide — a HALF-WIDTH OF 10 — with the ground
// beside it 40 lower on one side and 240 on the other, and the switchback path is 20-28 wide with
// end ramps at 17 and 19 degrees. MM geometry is built to the same unit scale, so they carry over.
#define KSURF_RAIL_PROBE 100.27f       // lateral reach; /SAMPLES this is a 5-unit resolution
#define KSURF_RAIL_MAX_WIDTH 47.03f    // ground narrower than this across = a rail
#define KSURF_RAIL_EDGE_DROP 2.0f      // deviation from the strip's PLANE that counts as an edge
#define KSURF_RAIL_AHEAD 10.0f         // the strip has to keep going this far ahead to count as LONG
#define KSURF_RAIL_AHEAD_MAX 90.0f     // ceiling on that, because the floor plane is extrapolated over it
#define KSURF_RAIL_PROBE_UP 30.0f      // the floor rays start this high above him
#define KSURF_RAIL_SAMPLES 8           // steps per side when hunting the edge
#define KSURF_RAIL_RING 16             // probes in the ring that finds the strip's own direction
#define KSURF_RAIL_RING_RADIUS 26.23f  // must stay ABOVE half of MAX_WIDTH or nothing is recognised
#define KSURF_RAIL_RING_MAX_HITS 9     // more of the ring than this on solid ground = open floor
#define KSURF_RAIL_MAX_APPROACH 0x4000 // 67 deg, for grabbing a rail in the first place
#define KSURF_RAIL_MAX_TURN 0x5800     // 124 deg, for following one round a corner
#define KSURF_RAIL_GAP_STEPS 3         // forward samples that bridge a gap between segments
#define KSURF_RAIL_ATTRACT 90.0f       // how far out the magnet looks for a rail to drag him onto
#define KSURF_RAIL_ATTRACT_DIRS 8      // directions it sweeps
#define KSURF_RAIL_ATTRACT_RISE 30.0f  // it only takes rails within this much of his own height
#define KSURF_RAIL_ATTRACT_PULL 6.0f   // units per frame dragged toward one
#define KSURF_RAIL_SPEED 8.27f         // flat boost while railing
#define KSURF_RAIL_SNAP 60.0f          // cap on the per-frame centring; big enough that it is a PIN
#define KSURF_RAIL_GRACE 4
#define KSURF_RAIL_TURN 12000.0f    // base rate at which the heading tracks the strip
#define KSURF_RAIL_DETACH_FRAMES 30 // rail detection stays off this long after A let go of one
#define KSURF_TURN_FULL 0x0A00      // turn rate that counts as a full-strength carve
#define KSURF_TURN_SMOOTH 0.15f     // how fast that carve value chases the real turn rate
#define KSURF_LEAN_SCALE 0.75f      // how much of the floor pitch reaches the model
#define KSURF_POSE_FRAME 0.0f       // frame of the slope-slide clip we freeze on
#define KSURF_BONK_YAW 0x2000
#define KSURF_REMOUNT_LOCKOUT 25

// NOTE ON TIMERS: every frame count above is copied from SoH UNCHANGED. MM's logic runs at 20 Hz
// (R_UPDATE_RATE 3), so converting them would make the whole thing run three times slower.

static s16 sKSurfRemountLockout = 0;

// ---------------------------------------------------------------------------
// Small helpers
// ---------------------------------------------------------------------------

// The ISG-safe melee teardown: kill the swing state and the live blade quads so nothing can keep an
// AT registered while we own the player.
//
// It does NOT set meleeWeaponAnimation = -1. Doing that without moving the action func off the
// melee attack is a crash in both games — the attack action indexes a table with it. Parking on
// Player_Action_Idle is what makes the teardown safe, and it gives the takeover check a known
// action to sit on.
static void KiteSurf_KillMeleeState(Player* player, PlayState* play) {
    // MM's OWN teardown, rather than the hand-rolled one this was ported with. It clears
    // meleeWeaponState AND PLAYER_STATE2_20000 (MM's non-magic spin-attack flag) AND the three
    // meleeWeaponInfo actives. That flag matters: MM does NOT clear it in its per-frame stateFlags2
    // reset, so a swing cancelled by mounting left it set for the whole ride, and En_M_Thunder reads
    // it — the spin energy stayed alive and discharged mid-surf.
    func_8082DC38(player);
    player->stateFlags1 &= ~PLAYER_STATE1_CHARGING_SPIN_ATTACK;
    player->unk_B08 = 0.0f; // spin charge amount (OoT's unk_858)
    Collider_ResetQuadAT(play, &player->meleeWeaponQuads[0].base);
    Collider_ResetQuadAT(play, &player->meleeWeaponQuads[1].base);

    // Flag 1 keeps the shield up — he is standing on one.
    Player_SetAction(play, player, Player_Action_Idle, 1);
}

// The riding pose: the vanilla DOWNHILL SLOPE SLIDE clip, held on one frame. That is the animation
// with the bent knees and the low centre of gravity — the same one the engine puts on when the
// ground gives way under him — so the surf reads as a slide instead of a man standing on a shield.
static void KiteSurf_HoldPose(Player* player, PlayState* play) {
    PlayerAnimationHeader* pose = (PlayerAnimationHeader*)&gPlayerAnim_link_normal_down_slope_slip;
    f32 frame = KSURF_POSE_FRAME;

    if (player->skelAnime.animation != pose) {
        PlayerAnimation_Change(play, &player->skelAnime, pose, 0.0f, frame, frame, ANIMMODE_ONCE, -4.0f);
    }
    player->skelAnime.playSpeed = 0.0f;
}

// Hand the player back. Safe from any state, including KiteShield_Cleanup when the shield is
// unequipped mid-ride.
void KiteSurf_Abort(Player* player) {
    if (sKSurf.state == KSURF_OFF) {
        return;
    }
    sKSurf.state = KSURF_OFF;
    sKSurf.timer = 0;
    sKSurf.stopFrames = 0;
    sKSurf.spinFrames = 0;
    sKSurf.railMiss = 0;
    sKSurf.railDetach = 0;
    sKSurf.boardSpin = 0;
    sKSurf.boardSpinRate = 0;
    sKSurf.leanPitch = 0;
    sKSurf.leanRoll = 0;
    sKSurf.upperLean = 0;
    sKSurf.turn = 0.0f;
    sKSurf.ownedAction = NULL;
    sKSurfRemountLockout = KSURF_REMOUNT_LOCKOUT;

    if (player != NULL) {
        player->stateFlags3 &= ~PLAYER_STATE3_PAUSE_ACTION_FUNC;
        player->skelAnime.playSpeed = 1.0f;
        player->actor.shape.rot.x = 0;
    }
}

static void KiteSurf_Start(Player* player, PlayState* play) {
    KiteSurf_KillMeleeState(player, play);

    // Take control from THIS frame. Entry always happens in mid air, and an airborne frame that is
    // not paused gets the action func replaced out from under us, which the takeover check then
    // reads as "something stole the player" — the surf would die before it started.
    player->stateFlags3 |= PLAYER_STATE3_PAUSE_ACTION_FUNC;

    sKSurf.state = KSURF_MOUNT;
    sKSurf.timer = 0;
    sKSurf.stopFrames = 0;
    sKSurf.spinFrames = 0;
    sKSurf.railMiss = 0;
    sKSurf.railDetach = 0;
    sKSurf.boardSpin = 0;
    sKSurf.boardSpinRate = 0;
    sKSurf.leanPitch = 0;
    sKSurf.leanRoll = 0;
    sKSurf.upperLean = 0;
    sKSurf.turn = 0.0f;
    sKSurf.ownedAction = (void*)player->actionFunc;

    Player_Anim_PlayOnceMorph(play, player, (PlayerAnimationHeader*)&gPlayerAnim_link_normal_down_slope_slip);
    Player_PlaySfx(player, NA_SE_IT_SHIELD_SWING); // OoT calls this same 0x181F NA_SE_IT_SHIELD_POSTURE
}

// Everything that must be true to be allowed to ride at all.
//
// Only MM's NAMED state-1 flags are used. The rest of that enum is still numeric in this decomp,
// and guessing which bit means what — even though the layout looks inherited from OoT — is the
// kind of assumption that fails silently. The cutscene and water cases are covered by MM's own
// idioms instead (csAction, the bg check flag), which is what the engine itself tests.
static u8 KiteSurf_Allowed(Player* player) {
    if (player->transformation != PLAYER_FORM_HUMAN) {
        return 0; // Deku/Goron/Zora have their own movement and skeletons
    }
    if (player->stateFlags1 & (PLAYER_STATE1_DEAD | PLAYER_STATE1_TALKING | PLAYER_STATE1_CARRYING_ACTOR)) {
        return 0;
    }
    if (player->csAction != PLAYER_CSACTION_NONE) {
        return 0;
    }
    if (player->actor.bgCheckFlags & BGCHECKFLAG_WATER) {
        return 0;
    }
    return 1;
}

// ---------------------------------------------------------------------------
// Rail detection — a rail is a piece of FLOOR that is narrow and long: a beam, a ledge, a fence
// top, a raised path. Not a corridor between two walls. Every probe here is a floor raycast.
//
// THE AXIS IS FOUND FIRST, AND WITHOUT REFERENCE TO WHERE HE IS LOOKING. Measuring the strip
// sideways from his own heading only works when he is already lined up with it: come at a beam at
// an angle and the sideways cut crosses it diagonally and reads far wider than it is, come at it
// square and the cut runs ALONG it and finds no edge at all.
//
// Instead a RING of floor probes goes round him. On a narrow strip the ones that land on solid
// ground form two opposite arcs pointing along it; on open ground they all land, which is how open
// ground is told apart for free. The arc closest to the way he is already going wins.
// ---------------------------------------------------------------------------

static f32 KiteSurf_FloorAtWorld(PlayState* play, Player* player, f32 x, f32 z) {
    Vec3f probe;
    CollisionPoly* poly = NULL;
    s32 bgId = BGCHECK_SCENE;

    probe.x = x;
    probe.y = player->actor.world.pos.y + KSURF_RAIL_PROBE_UP;
    probe.z = z;

    return BgCheck_EntityRaycastFloor3(&play->colCtx, &poly, &bgId, &probe);
}

// Is the ground at this world XZ part of the strip he is standing on?
//
// Measured against the PLANE of the poly under his feet, not a flat height. Rails slope: the Hyrule
// Field switchback's end ramps run at 17 and 19 degrees, which over the lookahead is 26 units of
// fall — twice the edge budget. Comparing to a flat height read that as "the strip has ended" and
// refused the rail on exactly the ramps. Symmetric, so a sunken channel counts too.
static u8 KiteSurf_OnStrip(PlayState* play, Player* player, f32 x, f32 z, f32 drop) {
    f32 y = KiteSurf_FloorAtWorld(play, player, x, z);
    Vec3f point;

    if (y <= BGCHECK_Y_MIN) {
        return 0;
    }
    if (player->actor.floorPoly == NULL) {
        return 1;
    }

    point.x = x;
    point.y = y;
    point.z = z;
    return fabsf(CollisionPoly_GetPointDistanceFromPlane(player->actor.floorPoly, &point)) <= drop;
}

static u8 KiteSurf_OnStripAt(PlayState* play, Player* player, s16 yaw, f32 dist, f32 drop) {
    return KiteSurf_OnStrip(play, player, player->actor.world.pos.x + (dist * Math_SinS(yaw)),
                            player->actor.world.pos.z + (dist * Math_CosS(yaw)), drop);
}

// ---------------------------------------------------------------------------
// The magnet: sweep for a rail near him and drag him onto it. HORIZONTAL only and gated on the
// candidate being at roughly his own height — hauling him up onto something well above would mean
// launching him vertically through its side.
// ---------------------------------------------------------------------------
static f32 KiteSurf_FloorPolyAt(PlayState* play, Player* player, f32 x, f32 z, CollisionPoly** outPoly) {
    Vec3f probe;
    s32 bgId = BGCHECK_SCENE;

    *outPoly = NULL;
    probe.x = x;
    probe.y = player->actor.world.pos.y + KSURF_RAIL_PROBE_UP;
    probe.z = z;

    return BgCheck_EntityRaycastFloor3(&play->colCtx, outPoly, &bgId, &probe);
}

static u8 KiteSurf_OffPlane(PlayState* play, Player* player, CollisionPoly* poly, f32 x, f32 z, f32 drop) {
    CollisionPoly* ignored;
    f32 y = KiteSurf_FloorPolyAt(play, player, x, z, &ignored);
    Vec3f point;

    if (y <= BGCHECK_Y_MIN) {
        return 1;
    }
    point.x = x;
    point.y = y;
    point.z = z;
    return fabsf(CollisionPoly_GetPointDistanceFromPlane(poly, &point)) > drop;
}

static u8 KiteSurf_Attract(Player* player, PlayState* play, f32 drop) {
    f32 range = KSURF_RAIL_ATTRACT;
    f32 rise = KSURF_RAIL_ATTRACT_RISE;
    f32 pull = KSURF_RAIL_ATTRACT_PULL;
    f32 side = KSURF_RAIL_RING_RADIUS;
    s32 stepAng = 0x10000 / KSURF_RAIL_ATTRACT_DIRS;
    s16 heading = player->actor.shape.rot.y;
    s32 bestDiff = 0x7FFFFFFF;
    f32 bestX = 0.0f;
    f32 bestZ = 0.0f;
    u8 found = 0;
    s32 k;

    for (k = 0; k < KSURF_RAIL_ATTRACT_DIRS; k++) {
        s16 dir = (s16)(k * stepAng);
        f32 cx = player->actor.world.pos.x + (range * Math_SinS(dir));
        f32 cz = player->actor.world.pos.z + (range * Math_CosS(dir));
        CollisionPoly* poly;
        f32 cy = KiteSurf_FloorPolyAt(play, player, cx, cz, &poly);
        s16 across;
        s32 diff;

        if ((cy <= BGCHECK_Y_MIN) || (poly == NULL)) {
            continue;
        }
        if (fabsf(cy - player->actor.floorHeight) > rise) {
            continue;
        }

        across = dir + 0x4000;
        if (!KiteSurf_OffPlane(play, player, poly, cx + (side * Math_SinS(across)), cz + (side * Math_CosS(across)),
                               drop) ||
            !KiteSurf_OffPlane(play, player, poly, cx - (side * Math_SinS(across)), cz - (side * Math_CosS(across)),
                               drop)) {
            continue;
        }

        diff = ABS((s16)(dir - heading));
        if (diff < bestDiff) {
            bestDiff = diff;
            bestX = cx;
            bestZ = cz;
            found = 1;
        }
    }

    if (!found) {
        return 0;
    }

    Math_StepToF(&player->actor.world.pos.x, bestX, pull);
    Math_StepToF(&player->actor.world.pos.z, bestZ, pull);
    return 1;
}

// How far out to one side of `yaw` the ground lasts before it ends or drops away.
static f32 KiteSurf_EdgeDistance(PlayState* play, Player* player, s16 yaw, f32 sign, f32 reach, f32 drop) {
    f32 step = reach / (f32)KSURF_RAIL_SAMPLES;
    s16 side = yaw + (s16)((sign > 0.0f) ? 0x4000 : -0x4000);
    s32 i;

    for (i = 1; i <= KSURF_RAIL_SAMPLES; i++) {
        f32 d = step * (f32)i;

        if (!KiteSurf_OnStripAt(play, player, side, d, drop)) {
            return d;
        }
    }
    return 0.0f;
}

static u8 KiteSurf_FindAxis(PlayState* play, Player* player, f32 drop, s16* outYaw) {
    u8 hit[KSURF_RAIL_RING];
    f32 radius = KSURF_RAIL_RING_RADIUS;
    s32 maxHits = (s32)(f32)KSURF_RAIL_RING_MAX_HITS;
    // Loose while riding one, fussy while looking for one: entering must not let a strip crossing
    // his path yank him onto it, but following has to survive a right-angle corner.
    s32 maxApproach = (sKSurf.state == KSURF_RAIL) ? (s32)(f32)KSURF_RAIL_MAX_TURN : (s32)(f32)KSURF_RAIL_MAX_APPROACH;
    s32 stepAng = 0x10000 / KSURF_RAIL_RING;
    s16 heading = player->actor.shape.rot.y;
    s32 hits = 0;
    s32 origin = -1;
    s32 bestDiff = 0x7FFFFFFF;
    u8 found = 0;
    s32 pass;
    s32 k;

    // Two passes, the second at half radius. On the thinnest rails a ring at full radius only ever
    // catches the two probes pointing exactly along the strip, and misses even those if he is a
    // little off the centre line.
    for (pass = 0; pass < 2; pass++) {
        hits = 0;
        for (k = 0; k < KSURF_RAIL_RING; k++) {
            hit[k] = KiteSurf_OnStripAt(play, player, (s16)(k * stepAng), radius, drop);
            hits += hit[k];
        }
        if (hits != 0) {
            break;
        }
        radius *= 0.5f;
    }

    if ((hits == 0) || (hits > maxHits)) {
        return 0;
    }

    for (k = 0; k < KSURF_RAIL_RING; k++) {
        if (hit[k] && !hit[(k + KSURF_RAIL_RING - 1) % KSURF_RAIL_RING]) {
            origin = k;
            break;
        }
    }
    if (origin < 0) {
        return 0;
    }

    k = 0;
    while (k < KSURF_RAIL_RING) {
        s32 len = 0;
        s32 ang;
        s32 diff;

        if (!hit[(origin + k) % KSURF_RAIL_RING]) {
            k++;
            continue;
        }
        while ((len < KSURF_RAIL_RING) && hit[(origin + k + len) % KSURF_RAIL_RING]) {
            len++;
        }

        // Middle of this arc. Truncating the s32 to s16 IS the binang wrap.
        ang = (s32)((((f32)(origin + k)) + (((f32)len - 1.0f) * 0.5f)) * (f32)stepAng);
        diff = ABS((s16)((s16)ang - heading));

        if (diff < bestDiff) {
            bestDiff = diff;
            *outYaw = (s16)ang;
            found = 1;
        }
        k += len;
    }

    return found && (bestDiff <= maxApproach);
}

static u8 KiteSurf_UpdateRail(Player* player, PlayState* play) {
    f32 maxWidth = KSURF_RAIL_MAX_WIDTH;
    f32 reach = KSURF_RAIL_PROBE;
    f32 drop = KSURF_RAIL_EDGE_DROP;
    f32 ahead = KSURF_RAIL_AHEAD;
    s16 axis;
    f32 dL;
    f32 dR;
    f32 centre;

    if (!KiteSurf_FindAxis(play, player, drop, &axis)) {
        return 0;
    }

    // Publish the heading NOW, before the gates below can bail out. Standing right on a corner the
    // width test reads across the new arm and finds the old arm's ground on one side, so it fails
    // for a frame or two — and those are exactly the frames that have to steer him round.
    if (sKSurf.state == KSURF_RAIL) {
        sKSurf.railAxisYaw = axis;
    }

    dL = KiteSurf_EdgeDistance(play, player, axis, -1.0f, reach, drop);
    dR = KiteSurf_EdgeDistance(play, player, axis, 1.0f, reach, drop);
    if ((dL == 0.0f) || (dR == 0.0f) || ((dL + dR) > maxWidth)) {
        return 0;
    }
    centre = (dR - dL) * 0.5f;

    // LONG, not just narrow. Look further ahead the faster he goes, but capped: the plane under his
    // feet is extrapolated over this distance and a switchback changes slope before it changes
    // direction.
    if (ahead < (player->speedXZ * 2.0f)) {
        ahead = player->speedXZ * 2.0f;
    }
    if (ahead > KSURF_RAIL_AHEAD_MAX) {
        ahead = KSURF_RAIL_AHEAD_MAX;
    }
    {
        // Sampled at several distances so the seam between two fence segments, a post, or a missing
        // collision quad does not read as "the strip ended". Any one landing is enough.
        s32 i;
        u8 goes = 0;

        for (i = 1; i <= KSURF_RAIL_GAP_STEPS; i++) {
            if (KiteSurf_OnStripAt(play, player, axis, ahead * ((f32)i / (f32)KSURF_RAIL_GAP_STEPS), drop)) {
                goes = 1;
                break;
            }
        }
        if (!goes) {
            return 0;
        }
    }

    sKSurf.railAxisYaw = axis;

    // PIN him to the centre line. Not a pull — the whole lateral error is taken out every frame, so
    // once a rail has him he cannot wander off it; the only ways off are the A hop, the strip
    // ending, or a head-on wall. One distance along the axis's lateral vector, because stepping x
    // and z separately would make the pull depend on which way he faces.
    {
        f32 snap = KSURF_RAIL_SNAP;
        f32 corr = CLAMP(centre, -snap, snap);
        s16 side = axis + 0x4000;

        player->actor.world.pos.x += corr * Math_SinS(side);
        player->actor.world.pos.z += corr * Math_CosS(side);
    }

    return 1;
}

// ---------------------------------------------------------------------------
// Anti-tunnelling. The player is moved by the engine's scene collision BEFORE this hook runs, so
// sub-stepping here would move him twice. Instead sweep the distance he is about to cover and clamp
// the speed so a frame can never start on the far side of a wall; the speed itself stays uncapped
// in open terrain, which is the point of the mechanic.
// ---------------------------------------------------------------------------
static void KiteSurf_ClampToWall(Player* player, PlayState* play) {
    Vec3f posA;
    Vec3f posB;
    Vec3f hit;
    CollisionPoly* poly = NULL;
    s32 bgId = BGCHECK_SCENE;
    f32 reach = player->speedXZ + 10.0f;
    f32 sn = Math_SinS(player->actor.shape.rot.y);
    f32 cs = Math_CosS(player->actor.shape.rot.y);
    f32 dist;

    if (player->speedXZ < 10.0f) {
        return; // the engine's own wall check already covers this much travel
    }

    posA.x = player->actor.world.pos.x;
    posA.y = player->actor.world.pos.y + 20.0f;
    posA.z = player->actor.world.pos.z;
    posB.x = posA.x + (sn * reach);
    posB.y = posA.y;
    posB.z = posA.z + (cs * reach);

    if (!BgCheck_EntityLineTest1(&play->colCtx, &posA, &posB, &hit, &poly, true, false, false, true, &bgId) ||
        (poly == NULL)) {
        return;
    }

    // Only a wall he is actually driving INTO — otherwise a curving rail reads its own outer side
    // as an obstacle and brakes every frame.
    {
        s16 wallYaw = Math_Atan2S_XY(COLPOLY_GET_NORMAL(poly->normal.z), COLPOLY_GET_NORMAL(poly->normal.x));

        if (Math_CosS(wallYaw - player->actor.shape.rot.y) > -0.5f) {
            return;
        }
    }

    dist = sqrtf(SQ(hit.x - posA.x) + SQ(hit.z - posA.z)) - 10.0f;
    if (dist < 0.0f) {
        dist = 0.0f;
    }
    if (player->speedXZ > dist) {
        player->speedXZ = dist;
    }
}

static u8 KiteSurf_HitWallHeadOn(Player* player) {
    s16 yawDiff;

    if (!(player->actor.bgCheckFlags & BGCHECKFLAG_PLAYER_WALL_INTERACT)) {
        return 0;
    }
    yawDiff = player->yaw - (s16)(player->actor.wallYaw + 0x8000);
    return (ABS(yawDiff) < KSURF_BONK_YAW) && (player->speedXZ > 4.0f);
}

// Let the C buttons through while the surf holds the pause.
//
// Player_UpdateItems, NOT Player_ProcessItemButtons. In OoT the inner function self-gates, but MM
// keeps EIGHT gates in the wrapper — item change settled, alive, no cutscene, no csAction, no
// b-button ammo prompt, main camera, func_8082DA90, minigame timer — and calling the inner one raw
// walked straight past all of them. That is what made the Pictograph Box fire on its own while
// falling. Player_CanUpdateItems is the same predicate the vanilla call site tests.
//
// It has to be called at all because MM only reaches Player_UpdateItems from inside the action
// handler chain, which the pause stops.
static void KiteSurf_RunItemButtons(Player* player, PlayState* play) {
    void* before;

    if (!Player_CanUpdateItems(player)) {
        return;
    }

    before = (void*)player->actionFunc;
    Player_UpdateItems(player, play);

    // If that started something — a swing, drawing the sword, using an item — it has to be allowed
    // to PLAY. Re-asserting the pause on the very next frame freezes it half done, and because it
    // then never completes, the item handling starts it again the frame after, and the frame after
    // that: which is what turned landing into an endless spin attack. Reusing the spin window hands
    // the action a few frames of real control and suppresses this call while it runs.
    if ((void*)player->actionFunc != before) {
        sKSurf.spinFrames = KSURF_SPIN_FRAMES;
        sKSurf.ownedAction = (void*)player->actionFunc;
    }
}

// ---------------------------------------------------------------------------
// Ride
// ---------------------------------------------------------------------------
static void KiteSurf_Ride(Player* player, PlayState* play) {
    // sPlayerControlInput is genuinely nullable in MM (z_player.c clears it), and every button
    // read below would be reading through it.
    Input* input = sPlayerControlInput;
    u8 grounded = (player->actor.bgCheckFlags & BGCHECKFLAG_GROUND) != 0;
    u8 pressedB = (input != NULL) && CHECK_BTN_ALL(input->press.button, BTN_B);
    f32 speedTarget = 0.0f;
    s16 yawTarget = player->actor.shape.rot.y;
    u8 hasStick = Player_GetMovementSpeedAndYaw(player, &speedTarget, &yawTarget, SPEED_MODE_LINEAR, play);

    // --- B + R dismounts; B alone spins ---
    if (pressedB) {
        if (CHECK_BTN_ALL(input->cur.button, BTN_R)) {
            sKSurf.state = KSURF_DISMOUNT;
            sKSurf.timer = 0;
            return;
        }
        if (Player_GetMeleeWeaponHeld(player) == PLAYER_MELEEWEAPON_NONE) {
            // Nothing in his hand, so there is no spin to do — B means DRAW THE SWORD. Hand this
            // press to the vanilla item handling instead of eating it, or the button does nothing
            // at all while surfing: the pause keeps the blade sheathed, and sheathed means there is
            // no melee weapon held.
            KiteSurf_RunItemButtons(player, play);
        } else if ((sKSurf.spinFrames == 0) && grounded) {
            // Grounded only: MM's ground slash refuses to start in mid air, so an airborne press
            // would re-arm every frame without ever producing a swing.
            func_80833864(play, player,
                          Player_IsHoldingTwoHandedWeapon(player) ? PLAYER_MWA_SPIN_ATTACK_2H
                                                                  : PLAYER_MWA_SPIN_ATTACK_1H);
            sKSurf.spinFrames = KSURF_SPIN_FRAMES;
            // That call changes the action func — re-own it so the takeover check does not read the
            // swing as a hostile steal.
            sKSurf.ownedAction = (void*)player->actionFunc;
        }
    }

    // --- A: hops off a rail (letting go of it), or just hops while free riding ---
    if ((input != NULL) && CHECK_BTN_ALL(input->press.button, BTN_A) && (grounded || (sKSurf.state == KSURF_RAIL))) {
        if (sKSurf.state == KSURF_RAIL) {
            // The lockout is what makes it stick: without it the next frame's probes see the same
            // strip and grab it straight back.
            sKSurf.state = KSURF_RIDE;
            sKSurf.railDetach = KSURF_RAIL_DETACH_FRAMES;
        }
        player->actor.velocity.y = KSURF_HOP_VEL;
        player->actor.bgCheckFlags &= ~BGCHECKFLAG_GROUND;
        grounded = 0;
        Player_PlaySfx(player, NA_SE_PL_SKIP);

        // Sometimes the board throws a shuvit on the way up. Random on purpose — a flourish, not
        // something to count on or spam.
        if ((sKSurf.boardSpinRate == 0) && (Rand_ZeroOne() < KSURF_SPIN_CHANCE)) {
            s16 rate = (s16)KSURF_BOARD_SPIN_RATE;

            sKSurf.boardSpinRate = (Rand_ZeroOne() < 0.5f) ? rate : (s16)-rate;
        }
    }

    // Board trick spin: runs while off the ground and lands square, because it is cosmetic and the
    // board must never be left crooked under him once he is riding again.
    if (grounded) {
        sKSurf.boardSpinRate = 0;
    }
    if (sKSurf.boardSpinRate != 0) {
        sKSurf.boardSpin += sKSurf.boardSpinRate;
    } else if (sKSurf.boardSpin != 0) {
        Math_ScaledStepToS(&sKSurf.boardSpin, 0, 4000);
    }

    // --- C items keep working: the pause would otherwise swallow them. Skipped on a B frame (B is
    //     ours) and during a spin (the pause is off then, so the action func already called it). ---
    if (!pressedB && (sKSurf.spinFrames == 0)) {
        KiteSurf_RunItemButtons(player, play);
    }

    // --- Rail ---
    if (sKSurf.railDetach > 0) {
        sKSurf.railDetach--;
    }

    if (grounded && (sKSurf.railDetach == 0) && KiteSurf_UpdateRail(player, play)) {
        sKSurf.state = KSURF_RAIL;
        sKSurf.railMiss = 0;
    } else if (grounded && (sKSurf.railDetach == 0) && (sKSurf.state != KSURF_RAIL) &&
               KiteSurf_Attract(player, play, KSURF_RAIL_EDGE_DROP)) {
        // Nothing under him, but there is a rail alongside — the magnet is dragging him across it.
        // No state change: once the drag puts him over the strip the branch above picks it up.
    } else if (sKSurf.state == KSURF_RAIL) {
        // A short grace, because a doorway, a post or a seam drops a probe for a frame or two
        // without the strip actually having ended.
        if (++sKSurf.railMiss >= KSURF_RAIL_GRACE) {
            sKSurf.state = KSURF_RIDE; // keeps the rail speed on the way out
        }
    }

    if (sKSurf.state == KSURF_RAIL) {
        // FOLLOW the geometry. The rate GROWS with how far off he is: a flat rate takes over three
        // frames on a right-angle corner, and at rail speed that is 80 units travelled while
        // turning, on segments only ~126 long — he would be off the far side before finishing.
        s16 delta = sKSurf.railAxisYaw - player->actor.shape.rot.y;
        s16 step = (s16)(KSURF_RAIL_TURN + (f32)(ABS(delta) >> 1));

        Math_SmoothStepToS(&player->actor.shape.rot.y, sKSurf.railAxisYaw, 2, step, 100);
        player->speedXZ = KSURF_RAIL_SPEED;
    }

    // --- Free steering (BotW): the stick turns him, the slope only feeds speed ---
    if ((sKSurf.state != KSURF_RAIL) && hasStick) {
        f32 turnMax = KSURF_TURN_MAX;
        f32 speed = player->speedXZ;
        s16 step;

        if (speed > 1.0f) {
            turnMax /= speed; // heavier the faster he goes
        }
        if (turnMax < KSURF_TURN_MIN) {
            turnMax = KSURF_TURN_MIN;
        }
        step = (s16)turnMax;
        Math_SmoothStepToS(&player->actor.shape.rot.y, yawTarget, 6, step, 100);
    }

    // --- Slope physics ---
    if (grounded && (player->actor.floorPoly != NULL)) {
        Vec3f slopeNormal;
        s16 downhillYaw;
        f32 steep;
        f32 align;

        // Deliberately NOT gated on the slippery-slope floor type the way the vanilla slide is:
        // every incline counts, which is what makes it generous.
        Actor_GetSlopeDirection(player->actor.floorPoly, &slopeNormal, &downhillYaw);

        steep = 1.0f - slopeNormal.y;
        align = Math_CosS(downhillYaw - player->actor.shape.rot.y);

        if (sKSurf.state != KSURF_RAIL) {
            player->speedXZ += KSURF_SLOPE_ACCEL * steep * align;
            Math_StepToF(&player->speedXZ, 0.0f, KSURF_FRICTION);

            if (hasStick) {
                player->speedXZ += KSURF_STICK_ACCEL * (speedTarget / 9.0f);
            }
        }

        Math_SmoothStepToS(&sKSurf.leanPitch, (s16)(player->floorPitch * KSURF_LEAN_SCALE), 3, 0x300, 0x40);
        player->actor.shape.rot.x = sKSurf.leanPitch;
    } else {
        // Airborne: keep the momentum, let the engine's gravity do the rest.
        Math_SmoothStepToS(&sKSurf.leanPitch, 0, 3, 0x300, 0x40);
        player->actor.shape.rot.x = sKSurf.leanPitch;
    }

    // Lean into the turn. player->yaw still holds LAST frame's heading here (it is synced further
    // down), so this difference is exactly how hard he is turning right now — the stick while free
    // riding, the strip's curve while on a rail.
    {
        s16 turnRate = player->actor.shape.rot.y - player->yaw;
        s16 want = (s16)(-turnRate * 2);
        s16 upperMax = (s16)(KSURF_UPPER_LEAN_DEG * 182.04f);
        f32 wantTurn;

        Math_SmoothStepToS(&sKSurf.leanRoll, CLAMP(want, -0x0A00, 0x0A00), 4, 0x200, 0x20);
        Math_SmoothStepToS(&sKSurf.upperLean, CLAMP(want, -upperMax, upperMax), 4, 0x200, 0x20);

        // Normalised carve for the lower body, kept as a fraction so each of that limb's axes can
        // scale it by its own amplitude, sign included.
        wantTurn = (f32)CLAMP(want, -KSURF_TURN_FULL, KSURF_TURN_FULL) / (f32)KSURF_TURN_FULL;
        sKSurf.turn += (wantTurn - sKSurf.turn) * KSURF_TURN_SMOOTH;
    }

    if (player->speedXZ < 0.0f) {
        player->speedXZ = 0.0f; // no reverse; NO upper cap by design
    }

    KiteSurf_ClampToWall(player, play);

    player->actor.speed = player->speedXZ;
    player->yaw = player->actor.shape.rot.y;

    if (grounded && (player->speedXZ > 1.0f)) {
        Audio_PlaySfx_AtPosWithSyncedFreqAndVolume(&player->actor.projectedPos,
                                                   Player_GetFloorSfx(player, NA_SE_PL_SLIP_LEVEL - SFX_FLAG),
                                                   player->actor.speed);
    }

    // --- Exits --- (taking damage is handled by KiteSurf_Allowed / the takeover check)
    if (KiteSurf_HitWallHeadOn(player)) {
        sKSurf.state = KSURF_DISMOUNT;
        sKSurf.timer = 0;
        return;
    }
    if (grounded && (player->speedXZ < KSURF_STOP_SPEED)) {
        if (++sKSurf.stopFrames >= KSURF_STOP_FRAMES) {
            sKSurf.state = KSURF_DISMOUNT;
            sKSurf.timer = 0;
        }
    } else {
        sKSurf.stopFrames = 0;
    }
}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------
// The real entry point, called from Player_Update AFTER Player_UpdateCommon (see the comment at
// that call site). Carries the gate the ext-equipment dispatcher would otherwise have applied:
// the cheat on, and the Kite Shield being the equipped ext shield.
void KiteSurf_PostUpdate(Player* player, PlayState* play) {
    if ((player == NULL) || (gPlayState == NULL) || (player != GET_PLAYER(gPlayState))) {
        return;
    }
    if (!ExtEquip_IsEnabled() || (gExtEquipState.currentExtShield != 2)) {
        KiteSurf_Abort(player);
        return;
    }
    KiteSurf_Tick(player, play);
}

void KiteSurf_Tick(Player* player, PlayState* play) {
    if (player == NULL) {
        return;
    }

    if (!KiteSurf_Allowed(player)) {
        KiteSurf_Abort(player);
        return;
    }

    // Not riding yet: PRESS R while airborne to get on the board.
    //
    // R and not "just be airborne", which is what this used to do — every jump, every step off a
    // ledge and every knockback mounted him, so the shield felt like it was equipping itself.
    // R is free here because the shield cannot be raised in mid air anyway, and it reads as the
    // shield button doing a shield thing. Having the Kite Shield equipped is the only other
    // requirement: on his back or in his hand both count, since the gate is the ext SHIELD SLOT and
    // not what he happens to be holding.
    if (sKSurf.state == KSURF_OFF) {
        if (sKSurfRemountLockout > 0) {
            // Only ticks down on the ground, so a dismount over a pit does not re-arm halfway
            // through the fall.
            if (player->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
                sKSurfRemountLockout--;
            }
            return;
        }
        if ((sPlayerControlInput != NULL) && CHECK_BTN_ALL(sPlayerControlInput->press.button, BTN_R) &&
            !(player->actor.bgCheckFlags & BGCHECKFLAG_GROUND) && !Player_InBlockingCsMode(play, player)) {
            KiteSurf_Start(player, play);
        }
        return;
    }

    // Damage, a cutscene or a scripted move calling Player_SetAction goes straight through the
    // pause. If the action func is not the one we parked on, we no longer own the player.
    // Not while mounting: the entry frame is still settling and RIDE re-records it anyway.
    if ((sKSurf.state != KSURF_MOUNT) && (sKSurf.spinFrames == 0) && (sKSurf.ownedAction != NULL) &&
        ((void*)player->actionFunc != sKSurf.ownedAction)) {
        KiteSurf_Abort(player);
        return;
    }

    // Yield: a door, an NPC, a grab or a ledge has to be able to complete, or the surf is a
    // softlock waiting to happen.
    if (Player_ActionHandler_1(player, play) || Player_ActionHandler_Talk(player, play) ||
        Player_ActionHandler_2(player, play) || Player_ActionHandler_12(player, play)) {
        KiteSurf_Abort(player);
        return;
    }

    switch (sKSurf.state) {
        case KSURF_MOUNT:
            player->stateFlags3 |= PLAYER_STATE3_PAUSE_ACTION_FUNC;
            PlayerAnimation_Update(play, &player->skelAnime);
            if (++sKSurf.timer >= KSURF_MOUNT_FRAMES) {
                sKSurf.state = KSURF_RIDE;
                sKSurf.timer = 0;
                sKSurf.ownedAction = (void*)player->actionFunc;
            }
            break;

        case KSURF_RIDE:
        case KSURF_RAIL:
            if (sKSurf.spinFrames > 0) {
                // The spin owns the animation and the action func: leave the pause off and let the
                // vanilla swing play out. Speed is untouched, so he keeps sliding through it, and
                // the board rides the ROOT limb so it comes round with him.
                sKSurf.spinFrames--;
                // The window is a ceiling, not the length — the swing ending early takes control
                // back at once. The 4-frame grace is because the melee state is still settling.
                if ((sKSurf.spinFrames == 0) || ((sKSurf.spinFrames < (KSURF_SPIN_FRAMES - 4)) &&
                                                 (player->meleeWeaponState == PLAYER_MELEE_WEAPON_STATE_0))) {
                    sKSurf.spinFrames = 0;
                    sKSurf.ownedAction = (void*)player->actionFunc;
                }
                KiteSurf_Ride(player, play);
                break;
            }
            player->stateFlags3 |= PLAYER_STATE3_PAUSE_ACTION_FUNC;
            KiteSurf_HoldPose(player, play);
            PlayerAnimation_Update(play, &player->skelAnime);
            KiteSurf_Ride(player, play);
            break;

        case KSURF_DISMOUNT:
            KiteSurf_KillMeleeState(player, play);
            Player_PlaySfx(player, NA_SE_IT_SHIELD_SWING); // OoT calls this same 0x181F NA_SE_IT_SHIELD_POSTURE
            KiteSurf_Abort(player);
            break;
    }
}
