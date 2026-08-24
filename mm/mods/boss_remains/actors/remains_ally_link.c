/**
 * remains_ally_link.c - Twinmold's remains summons a friendly DARK LINK that fights alongside you.
 *
 * ACTOR_REMAINS_ALLY_LINK (0x2C1). Like the other three boss-remains allies this file is authored here
 * and UNITY-#included into boss_remains.cpp inside its extern "C" block, so RemainsAllyLink_Profile gets
 * C linkage and matches the DEFINE_ACTOR row in actor_table.h. NOT in CMake/vcxproj.
 *
 * DESIGN — A MIRROR OF LINK:
 *   Unlike the other allies (which are real MM creatures), this one IS Link: it drives the actual human
 *   player skeleton (gLinkHumanSkel, object_link_child) through SkelAnime_InitPlayer + the PlayerAnimation
 *   system — the same pair the player himself uses — and plays real player animations (run / idle / sword
 *   slash / bow shot). It draws with a dark tint, so it reads as a Dark Link. Everything is loaded by OTR
 *   path, so the profile stays GAMEPLAY_KEEP and object_link_child never has to be resident.
 *
 * IT FIGHTS WITH WHATEVER *LINK* IS CARRYING:
 *   Every frame it reads the real save inventory and picks an attack from what Link actually owns:
 *     - Bow + arrows  → shoots a real ACTOR_EN_ARROW at range (the same actor Link's bow fires).
 *     - Sword         → melee slash (damage scales with which sword Link has equipped).
 *     - Hammer (NEI)  → heavier melee.
 *     - Deku Nuts     → point-blank nut (ACTOR_EN_ARROW / ARROW_TYPE_DEKU_NUT, exactly how the player
 *                       throws one in z_player.c:18755).
 *     - NOTHING       → it carries no weapon and never attacks; it just follows Link.
 *   It never spends Link's ammo — it mirrors his loadout, it doesn't share his pouch.
 *
 * FRIENDLY:
 *   The melee collider is AT_ON | AT_TYPE_PLAYER (hits enemies, can never hit Link) and it carries no
 *   AC/OC at all, so it is invulnerable and never shoves anyone. It only ever targets ACTORCAT_ENEMY /
 *   ACTORCAT_BOSS (RemainsAlly_FindNearestEnemy), so Link is never a target.
 *
 * ACTORCAT_MISC (not ENEMY — must not pollute enemy-count / room-clear / battle BGM).
 */

#include "remains_ally_common.h"                         // shared helpers + z64.h
#include "objects/object_link_child/object_link_child.h" // gLinkHumanSkel + hand/sword DLs
#include "objects/gameplay_keep/gameplay_keep.h"
#include "overlays/actors/ovl_En_Arrow/z_en_arrow.h" // ArrowType (ARROW_TYPE_NORMAL / _DEKU_NUT)
#include "overlays/actors/ovl_En_Bom/z_en_bom.h"     // EnBom + BOMB_EXPLOSIVE_TYPE_BOMB / BOMB_TYPE_BODY
#include "../../nei_save.h"                          // Nei_Save() — the NEI custom-item save block

// Compiled as part of the C++ TU boss_remains.cpp, so the OTR-path asset symbols are `const char[]` and
// need explicit casts to the pointer types the engine wants (implicit in C, not in C++).

// ============================================================================
// TUNING
// ============================================================================

#define ALLY_LINK_SCALE 0.01f       // human Link's own scale
#define ALLY_LINK_FOLLOW_DIST 90.0f // how close it trails Link when idle
#define ALLY_LINK_RUN_SPEED 5.2f
#define ALLY_LINK_SEEK_RANGE 700.0f // how far it looks for something to fight
#define ALLY_LINK_MELEE_RANGE 70.0f // inside this it swings instead of shooting
#define ALLY_LINK_NUT_RANGE 140.0f  // deku-nut throw range (short, like the player's)
#define ALLY_LINK_MELEE_FRAMES 8    // frames the swing's AT collider stays live
#define ALLY_LINK_MELEE_CD 28       // frames between swings
#define ALLY_LINK_SHOOT_CD 45       // frames between bow shots
#define ALLY_LINK_BOMB_CD 70        // frames between bombs (slowest — it's the heavy option)

// --- Tactical thresholds ----------------------------------------------------
#define ALLY_LINK_BOMB_RANGE 280.0f     // how far out it will place a bomb
#define ALLY_LINK_BOMB_CLUSTER_R 120.0f // foes within this of the target count as "a group"
#define ALLY_LINK_BOMB_SAFE_DIST 170.0f // NEVER bomb if LINK is closer than this to the blast
#define ALLY_LINK_BOMB_FUSE 35          // short fuse so it isn't dodged, long enough to be fair
#define ALLY_LINK_AIRBORNE_Y 55.0f      // target this far overhead is out of sword reach
#define ALLY_LINK_SWARM_COUNT 2         // foes at nut range that count as "swarmed"

// Ground collision, matched to the other remains allies (remains_ally_common.c). FLAG_4 is the one that
// actually snaps the actor onto the floor — without it the ally sinks through the ground the moment it
// stops moving (e.g. planting itself to swing).
#define ALLY_LINK_WALL_HEIGHT 26.0f
#define ALLY_LINK_WALL_RADIUS 10.0f
#define ALLY_LINK_BGCHECK_FLAGS (UPDBGCHECKINFO_FLAG_1 | UPDBGCHECKINFO_FLAG_4)

// Animation slots (see AllyLink_SetAnim). Frame counts from link_animetion.xml; endFrame is
// FrameCount-1 because the one-past-the-end frame renders the bind/rest pose.
#define ALLY_LINK_ANIM_NONE 0xFF
#define ALLY_LINK_ANIM_IDLE 0
#define ALLY_LINK_ANIM_RUN 1
#define ALLY_LINK_ANIM_MELEE 2
#define ALLY_LINK_ANIM_SHOOT 3

// ============================================================================
// STRUCT
// ============================================================================

typedef struct RemainsAllyLink RemainsAllyLink;

struct RemainsAllyLink {
    /* Actor */ Actor actor;
    /* Anim  */ SkelAnime skelAnime;
    /* Hit   */ ColliderCylinder collider;
    /* AI    */ s16 meleeTimer; // >0 while the swing is live
    /*       */ s16 cooldown;   // shared attack cooldown
    /*       */ u8 animState;   // ALLY_LINK_ANIM_*
    // Byte buffers, NOT Vec3s[PLAYER_LIMB_MAX]: SkelAnime_InitPlayer with the player's `1 | 8` flags wants
    // (limbCount + 1) Vec3s plus a u16, and it ALIGN16s the pointer we hand it — PLAYER_LIMB_BUF_SIZE is
    // exactly that size plus the 0xF alignment slack, and is what Player itself declares.
    /* Skel  */ u8 jointTableBuffer[PLAYER_LIMB_BUF_SIZE];
    /*       */ u8 morphTableBuffer[PLAYER_LIMB_BUF_SIZE];
};

static void RemainsAllyLink_Init(Actor* thisx, PlayState* play);
static void RemainsAllyLink_Destroy(Actor* thisx, PlayState* play);
static void RemainsAllyLink_Update(Actor* thisx, PlayState* play);
static void RemainsAllyLink_Draw(Actor* thisx, PlayState* play);

// The single live instance (Twinmold keeps exactly one companion).
static RemainsAllyLink* sActiveAllyLink = NULL;

s32 RemainsAllyLink_IsAlive(void) {
    return sActiveAllyLink != NULL;
}

// Retire the companion (mask taken off / transformed away).
void RemainsAllyLink_Despawn(PlayState* play) {
    (void)play;
    if (sActiveAllyLink != NULL) {
        Actor_Kill(&sActiveAllyLink->actor);
        sActiveAllyLink = NULL;
    }
}

// Melee swing hitbox. AT_TYPE_PLAYER = hits enemies, never Link. Damage is patched per swing from
// whichever weapon Link owns (sword tier / hammer).
static ColliderCylinderInit sAllyLinkColliderInit = {
    {
        COL_MATERIAL_NONE,
        AT_ON | AT_TYPE_PLAYER,
        AC_NONE,
        OC1_NONE,
        OC2_NONE,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        { DMG_SWORD, 0x00, 0x02 },
        { 0xF7CFFFFF, 0x00, 0x00 },
        ATELEM_ON | ATELEM_SFX_NORMAL,
        ACELEM_NONE,
        OCELEM_NONE,
    },
    { 45, 70, 0, { 0, 0, 0 } },
};

// ============================================================================
// LINK'S LOADOUT — read straight from the real save
// ============================================================================

static s32 AllyLink_SwordTier(void) {
    return GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD); // EQUIP_VALUE_SWORD_NONE..DIETY
}

static s32 AllyLink_HasSword(void) {
    return AllyLink_SwordTier() != EQUIP_VALUE_SWORD_NONE;
}

// The Megaton Hammer is a NEI custom item — it does NOT live in the vanilla inventory array (ITEM_HAMMER
// is 0xED, so SLOT()/INV_CONTENT would index out of bounds). It's tracked in the NEI save block, the same
// place ExtInv_GetOotSlotItem reads it from.
static s32 AllyLink_HasHammer(void) {
    NeiSaveData* nei = Nei_Save();
    return (nei != NULL) && nei->ootHammerOwned;
}

static s32 AllyLink_HasBow(void) {
    return (INV_CONTENT(ITEM_BOW) == ITEM_BOW) && (AMMO(ITEM_BOW) > 0);
}

static s32 AllyLink_HasNuts(void) {
    return (INV_CONTENT(ITEM_DEKU_NUT) == ITEM_DEKU_NUT) && (AMMO(ITEM_DEKU_NUT) > 0);
}

static s32 AllyLink_HasBombs(void) {
    return (INV_CONTENT(ITEM_BOMB) == ITEM_BOMB) && (AMMO(ITEM_BOMB) > 0);
}

// ============================================================================
// TACTICAL READS — what the situation actually allows
// ============================================================================

// Can we actually hit it from here, or is there geometry in the way? Arrows and bombs are wasted on a
// target behind a wall, so ranged options check this first.
static s32 AllyLink_HasLineOfSight(RemainsAllyLink* self, PlayState* play, Actor* target) {
    Vec3f from;
    Vec3f to;
    Vec3f hit;
    CollisionPoly* poly;
    s32 bgId;

    from = self->actor.world.pos;
    from.y += 40.0f; // shoot from chest height, not the feet
    to = target->world.pos;
    to.y += 20.0f;

    // Returns true when the line hits something → blocked.
    return !BgCheck_ProjectileLineTest(&play->colCtx, &from, &to, &hit, &poly, true, false, false, true, &bgId);
}

// Out of sword reach vertically: either it isn't standing on anything or it's simply well above us
// (keese, guays, peahats...). Those need the bow.
static s32 AllyLink_TargetIsAirborne(RemainsAllyLink* self, Actor* target) {
    f32 yDiff = target->world.pos.y - self->actor.world.pos.y;

    if (yDiff < ALLY_LINK_AIRBORNE_Y) {
        return false;
    }
    return ((target->bgCheckFlags & BGCHECKFLAG_GROUND) == 0) || (yDiff > (ALLY_LINK_AIRBORNE_Y * 2.0f));
}

// How many live foes sit within `radius` of a point — drives "is this worth a bomb?".
static s32 AllyLink_CountEnemiesNear(PlayState* play, Vec3f* at, f32 radius) {
    s32 count = 0;

    for (s32 cat = 0; cat < 2; cat++) {
        Actor* a = play->actorCtx.actorLists[(cat == 0) ? ACTORCAT_ENEMY : ACTORCAT_BOSS].first;

        for (; a != NULL; a = a->next) {
            if ((a->update == NULL) || (a->colChkInfo.health == 0)) {
                continue;
            }
            f32 dx = a->world.pos.x - at->x;
            f32 dz = a->world.pos.z - at->z;
            if (sqrtf(SQ(dx) + SQ(dz)) <= radius) {
                count++;
            }
        }
    }
    return count;
}

// Melee damage from Link's best melee weapon (hammer hits hardest, then sword tier).
static u8 AllyLink_MeleeDamage(void) {
    if (AllyLink_HasHammer()) {
        return 8;
    }
    switch (AllyLink_SwordTier()) {
        case EQUIP_VALUE_SWORD_KOKIRI:
            return 2;
        case EQUIP_VALUE_SWORD_RAZOR:
            return 4;
        case EQUIP_VALUE_SWORD_GILDED:
            return 6;
        case EQUIP_VALUE_SWORD_DIETY:
            return 8;
        default:
            return 0;
    }
}

// ============================================================================
// ANIMATION
// ============================================================================

static void AllyLink_SetAnim(RemainsAllyLink* self, PlayState* play, u8 which) {
    if (self->animState == which) {
        return; // already playing — don't restart it every frame
    }
    self->animState = which;

    switch (which) {
        case ALLY_LINK_ANIM_RUN: // run_free = 20 frames
            PlayerAnimation_Change(play, &self->skelAnime, (PlayerAnimationHeader*)gPlayerAnim_link_normal_run_free,
                                   1.0f, 0.0f, 19.0f, ANIMMODE_LOOP, -4.0f);
            break;
        case ALLY_LINK_ANIM_MELEE: // fighter_normal_kiru = 5 frames
            PlayerAnimation_Change(play, &self->skelAnime, (PlayerAnimationHeader*)gPlayerAnim_link_fighter_normal_kiru,
                                   1.0f, 0.0f, 4.0f, ANIMMODE_ONCE, -2.0f);
            break;
        case ALLY_LINK_ANIM_SHOOT: // bow_bow_shoot_next = 8 frames
            PlayerAnimation_Change(play, &self->skelAnime, (PlayerAnimationHeader*)gPlayerAnim_link_bow_bow_shoot_next,
                                   1.0f, 0.0f, 7.0f, ANIMMODE_ONCE, -2.0f);
            break;
        case ALLY_LINK_ANIM_IDLE: // waitR_free = 29 frames
        default:
            PlayerAnimation_Change(play, &self->skelAnime, (PlayerAnimationHeader*)gPlayerAnim_link_normal_waitR_free,
                                   1.0f, 0.0f, 28.0f, ANIMMODE_LOOP, -4.0f);
            break;
    }
}

// ============================================================================
// ATTACKS
// ============================================================================

static void AllyLink_FaceActor(RemainsAllyLink* self, Actor* target) {
    s16 yaw = Actor_WorldYawTowardActor(&self->actor, target);
    Math_SmoothStepToS(&self->actor.shape.rot.y, yaw, 3, 0x2000, 0);
    self->actor.world.rot.y = self->actor.shape.rot.y;
}

// Fire a real player arrow (the same actor Link's bow spawns). arrowType picks a normal shot or a
// point-blank deku nut, mirroring z_player.c's own two spawn sites.
static void AllyLink_ShootArrow(RemainsAllyLink* self, PlayState* play, Actor* target, s32 arrowType) {
    self->actor.shape.rot.y = Actor_WorldYawTowardActor(&self->actor, target);
    self->actor.world.rot.y = self->actor.shape.rot.y;

    Actor_Spawn(&play->actorCtx, play, ACTOR_EN_ARROW, self->actor.world.pos.x, self->actor.world.pos.y + 40.0f,
                self->actor.world.pos.z, 0xFA0, self->actor.shape.rot.y, 0, arrowType);

    Actor_PlaySfx(&self->actor, (arrowType == ARROW_TYPE_DEKU_NUT) ? NA_SE_IT_DEKU : NA_SE_IT_ARROW_SHOT);
}

static void AllyLink_StartMelee(RemainsAllyLink* self, PlayState* play, Actor* target) {
    self->actor.speed = 0.0f;
    AllyLink_FaceActor(self, target);
    AllyLink_SetAnim(self, play, ALLY_LINK_ANIM_MELEE);
    self->meleeTimer = ALLY_LINK_MELEE_FRAMES;
    self->cooldown = ALLY_LINK_MELEE_CD;
    self->collider.elem.atDmgInfo.damage = AllyLink_MeleeDamage();
    Actor_PlaySfx(&self->actor, AllyLink_HasHammer() ? NA_SE_IT_HAMMER_SWING : NA_SE_IT_SWORD_SWING);
}

// Place a live bomb right on the target. EN_BOM's blast is AT_TYPE_ALL — it hurts EVERYONE, Link
// included — which is exactly why the chooser below refuses to bomb near him.
static void AllyLink_PlaceBomb(RemainsAllyLink* self, PlayState* play, Actor* target) {
    EnBom* bomb =
        (EnBom*)Actor_Spawn(&play->actorCtx, play, ACTOR_EN_BOM, target->world.pos.x, target->world.pos.y + 10.0f,
                            target->world.pos.z, BOMB_EXPLOSIVE_TYPE_BOMB, 0, 0, BOMB_TYPE_BODY);
    if (bomb != NULL) {
        bomb->timer = ALLY_LINK_BOMB_FUSE;
    }
    AllyLink_FaceActor(self, target);
    Actor_PlaySfx(&self->actor, NA_SE_PL_THROW);
}

// ============================================================================
// THE CHOICE — pick the right tool for THIS enemy, from what Link actually owns
// ============================================================================

typedef enum {
    ALLY_ATK_NONE,  // Link owns no weapon → this shadow can't fight either
    ALLY_ATK_CHASE, // right tool is melee, but we're not close enough yet
    ALLY_ATK_MELEE,
    ALLY_ATK_BOW,
    ALLY_ATK_BOMB,
    ALLY_ATK_NUT
} AllyLinkAttack;

static AllyLinkAttack AllyLink_ChooseAttack(RemainsAllyLink* self, PlayState* play, Actor* target, f32 dist) {
    Player* player = GET_PLAYER(play);
    s32 hasMelee = AllyLink_HasSword() || AllyLink_HasHammer();
    s32 hasBow = AllyLink_HasBow();
    s32 hasBombs = AllyLink_HasBombs();
    s32 hasNuts = AllyLink_HasNuts();
    s32 airborne = AllyLink_TargetIsAirborne(self, target);
    s32 los = AllyLink_HasLineOfSight(self, play, target);
    // Bombing near Link would blow HIM up. This is a support unit — measure before lighting anything.
    f32 linkToBlast = (player != NULL) ? Actor_WorldDistXZToActor(&player->actor, target) : 0.0f;
    s32 blastIsSafe = (player != NULL) && (linkToBlast > ALLY_LINK_BOMB_SAFE_DIST);

    // 1. Swarmed at point-blank → a nut stuns the whole crowd. Defensive first: this is what actually
    //    takes pressure off Link.
    if (hasNuts && (dist < ALLY_LINK_NUT_RANGE) &&
        (AllyLink_CountEnemiesNear(play, &self->actor.world.pos, ALLY_LINK_NUT_RANGE) >= ALLY_LINK_SWARM_COUNT)) {
        return ALLY_ATK_NUT;
    }

    // 2. Already in sword reach and the thing is actually reachable → swing. Melee costs no ammo, so it's
    //    always preferred when it works.
    if (hasMelee && (dist <= ALLY_LINK_MELEE_RANGE) && !airborne) {
        return ALLY_ATK_MELEE;
    }

    // 3. It's flying: a sword will never touch it, so the bow is the only real answer.
    if (hasBow && airborne && los) {
        return ALLY_ATK_BOW;
    }

    // 4. A GROUP at bombing distance, with Link clear of the blast → bomb is worth far more than one arrow.
    if (hasBombs && blastIsSafe && los && (dist > ALLY_LINK_MELEE_RANGE) && (dist <= ALLY_LINK_BOMB_RANGE) &&
        (AllyLink_CountEnemiesNear(play, &target->world.pos, ALLY_LINK_BOMB_CLUSTER_R) >= 2)) {
        return ALLY_ATK_BOMB;
    }

    // 5. Out of melee range with a clean shot → bow.
    if (hasBow && (dist > ALLY_LINK_MELEE_RANGE) && los) {
        return ALLY_ATK_BOW;
    }

    // 6. No bow, but bombs and a safe blast → bomb the single target.
    if (hasBombs && blastIsSafe && los && (dist > ALLY_LINK_MELEE_RANGE) && (dist <= ALLY_LINK_BOMB_RANGE)) {
        return ALLY_ATK_BOMB;
    }

    // 7. Melee is the tool but we're too far (or it's airborne and we have no bow) → close in.
    if (hasMelee) {
        return (dist <= ALLY_LINK_MELEE_RANGE) ? ALLY_ATK_MELEE : ALLY_ATK_CHASE;
    }

    // 8. Nuts only: they still stun, so get in range and use them.
    if (hasNuts) {
        return (dist <= ALLY_LINK_NUT_RANGE) ? ALLY_ATK_NUT : ALLY_ATK_CHASE;
    }

    return ALLY_ATK_NONE;
}

// ============================================================================
// INIT / DESTROY
// ============================================================================

static void RemainsAllyLink_Init(Actor* thisx, PlayState* play) {
    RemainsAllyLink* self = (RemainsAllyLink*)thisx;

    Actor_SetScale(&self->actor, ALLY_LINK_SCALE);

    // Drive the REAL player skeleton with the REAL player-animation system — the same call Player_InitCommon
    // makes (z_player.c:11651), flags and all. The OTR path resolves gLinkHumanSkel at runtime, so
    // object_link_child never needs to be bound and the profile can stay GAMEPLAY_KEEP.
    // The `1 | 8` flags matter: they're what make limbCount cover the WHOLE skeleton (flags & 2 == 0 would
    // leave it at 1 limb and nothing would animate).
    SkelAnime_InitPlayer(play, &self->skelAnime, (FlexSkeletonHeader*)gLinkHumanSkel,
                         (PlayerAnimationHeader*)gPlayerAnim_link_normal_waitR_free, 1 | 8, self->jointTableBuffer,
                         self->morphTableBuffer, PLAYER_LIMB_MAX);
    // Player sets this right after its own init; without it the skeleton's root sits at the wrong offset.
    {
        Vec3s baseTransl = { -57, 3377, 0 }; // sPlayerSkeletonBaseTransl (z_player.c:11641)
        self->skelAnime.baseTransl = baseTransl;
    }

    ActorShape_Init(&self->actor.shape, 0.0f, ActorShadow_DrawCircle, 30.0f);
    Collider_InitCylinder(play, &self->collider);
    Collider_SetCylinder(play, &self->collider, &self->actor, &sAllyLinkColliderInit);

    self->actor.gravity = -2.0f;
    self->meleeTimer = 0;
    self->cooldown = 0;
    self->animState = ALLY_LINK_ANIM_NONE;
    AllyLink_SetAnim(self, play, ALLY_LINK_ANIM_IDLE);

    sActiveAllyLink = self;
}

static void RemainsAllyLink_Destroy(Actor* thisx, PlayState* play) {
    RemainsAllyLink* self = (RemainsAllyLink*)thisx;

    Collider_DestroyCylinder(play, &self->collider);
    if (sActiveAllyLink == self) {
        sActiveAllyLink = NULL;
    }
}

// ============================================================================
// UPDATE — pick a target, then pick an attack from LINK'S loadout
// ============================================================================

static void RemainsAllyLink_Update(Actor* thisx, PlayState* play) {
    RemainsAllyLink* self = (RemainsAllyLink*)thisx;

    PlayerAnimation_Update(play, &self->skelAnime);

    if (self->cooldown > 0) {
        self->cooldown--;
    }

    Actor* target = RemainsAlly_FindNearestEnemy(play, &self->actor.world.pos, ALLY_LINK_SEEK_RANGE);

    // --- Mid-swing: hold position and keep the blade live -------------------
    if (self->meleeTimer > 0) {
        self->meleeTimer--;
        self->actor.speed = 0.0f;
        if (target != NULL) {
            AllyLink_FaceActor(self, target);
        }
        Collider_UpdateCylinder(&self->actor, &self->collider);
        CollisionCheck_SetAT(play, &play->colChkCtx, &self->collider.base);
        Actor_MoveWithGravity(&self->actor);
        Actor_UpdateBgCheckInfo(play, &self->actor, ALLY_LINK_WALL_HEIGHT, ALLY_LINK_WALL_RADIUS, 0.0f,
                                ALLY_LINK_BGCHECK_FLAGS);
        if (self->meleeTimer == 0) {
            self->animState = ALLY_LINK_ANIM_NONE; // let the next state re-trigger its anim
        }
        return;
    }

    // --- Nothing to fight: trail Link ---------------------------------------
    if (target == NULL) {
        RemainsAlly_FollowPlayer(play, &self->actor, ALLY_LINK_FOLLOW_DIST, ALLY_LINK_RUN_SPEED);
        AllyLink_SetAnim(self, play, (self->actor.speed > 0.5f) ? ALLY_LINK_ANIM_RUN : ALLY_LINK_ANIM_IDLE);
        return;
    }

    f32 dist = Actor_WorldDistXZToActor(&self->actor, target);
    AllyLinkAttack pick = AllyLink_ChooseAttack(self, play, target, dist);

    // Link owns nothing to fight with → the shadow just escorts him.
    if (pick == ALLY_ATK_NONE) {
        RemainsAlly_FollowPlayer(play, &self->actor, ALLY_LINK_FOLLOW_DIST, ALLY_LINK_RUN_SPEED);
        AllyLink_SetAnim(self, play, (self->actor.speed > 0.5f) ? ALLY_LINK_ANIM_RUN : ALLY_LINK_ANIM_IDLE);
        return;
    }

    // The chosen tool needs us closer → run it down.
    if (pick == ALLY_ATK_CHASE) {
        RemainsAlly_HomeTowardPos(play, &self->actor, &target->world.pos, ALLY_LINK_RUN_SPEED);
        AllyLink_SetAnim(self, play, ALLY_LINK_ANIM_RUN);
        return;
    }

    // Everything below attacks from a standstill: plant, face the target, and either commit now or hold
    // the stance until the cooldown lets us.
    self->actor.speed = 0.0f;

    if (self->cooldown != 0) {
        AllyLink_FaceActor(self, target);
        AllyLink_SetAnim(self, play, ALLY_LINK_ANIM_IDLE);
    } else {
        switch (pick) {
            case ALLY_ATK_MELEE:
                AllyLink_StartMelee(self, play, target);
                break;

            case ALLY_ATK_BOW:
                AllyLink_ShootArrow(self, play, target, ARROW_TYPE_NORMAL);
                AllyLink_SetAnim(self, play, ALLY_LINK_ANIM_SHOOT);
                self->cooldown = ALLY_LINK_SHOOT_CD;
                break;

            case ALLY_ATK_BOMB:
                AllyLink_PlaceBomb(self, play, target);
                AllyLink_SetAnim(self, play, ALLY_LINK_ANIM_SHOOT);
                self->cooldown = ALLY_LINK_BOMB_CD;
                break;

            case ALLY_ATK_NUT:
                AllyLink_ShootArrow(self, play, target, ARROW_TYPE_DEKU_NUT);
                AllyLink_SetAnim(self, play, ALLY_LINK_ANIM_SHOOT);
                self->cooldown = ALLY_LINK_SHOOT_CD;
                break;

            default:
                break;
        }
    }

    // A melee swing owns its movement for the next few frames (handled at the top of Update); every other
    // case still has to be held down on the floor.
    if (self->meleeTimer == 0) {
        Actor_MoveWithGravity(&self->actor);
        Actor_UpdateBgCheckInfo(play, &self->actor, ALLY_LINK_WALL_HEIGHT, ALLY_LINK_WALL_RADIUS, 0.0f,
                                ALLY_LINK_BGCHECK_FLAGS);
    }
}

// ============================================================================
// DRAW — Link's own skeleton, tinted dark, holding whatever sword Link has
// ============================================================================

// Draw-only Player template. We copy the LIVE player wholesale, then override just the transform and the
// joint table with ours — exactly how HarpoonDummyPlayer renders remote players. Going through the real
// player draw path (Player_DrawImpl + Player_OverrideLimbDrawGameplayDefault + Player_PostLimbDrawGameplay)
// is what makes the SHIELD, sword, sheath and tunic render at all; a bare SkelAnime_DrawFlexOpa draws only
// the naked skeleton, which is why the companion first showed up unarmed.
static Player sAllyLinkDrawTemplate;

static void RemainsAllyLink_Draw(Actor* thisx, PlayState* play) {
    RemainsAllyLink* self = (RemainsAllyLink*)thisx;
    Player* player = GET_PLAYER(play);

    if ((player == NULL) || (player->skelAnime.skeleton == NULL) || (player->transformation != PLAYER_FORM_HUMAN)) {
        return;
    }
    if ((player->actor.objectSlot < 0) || !Object_IsLoaded(&play->objectCtx, player->actor.objectSlot)) {
        return;
    }

    Player* dp = &sAllyLinkDrawTemplate;
    *dp = *player;                                         // full, valid Player (equipment/boots/tunic come along)
    dp->actor.world.pos = self->actor.world.pos;           // ...but OUR position,
    dp->actor.shape.rot = self->actor.shape.rot;           // ...OUR facing,
    dp->skelAnime.jointTable = self->skelAnime.jointTable; // ...and OUR animation pose.
    dp->currentMask = PLAYER_MASK_NONE;                    // the companion doesn't wear Link's mask

    // Ground it the same way Actor_Draw does (z_actor.c): shape.yOffset scaled by the actor scale. Without
    // this the model hovers above the floor — the "floating" the companion had on spawn.
    f32 yOff = dp->actor.shape.yOffset * dp->actor.scale.y;
    void* seg06 = play->objectCtx.slots[player->actor.objectSlot].segment;

    OPEN_DISPS(play->state.gfxCtx);

    if (seg06 != NULL) {
        gSPSegment(POLY_OPA_DISP++, 0x06, (uintptr_t)seg06);
        gSPSegment(POLY_XLU_DISP++, 0x06, (uintptr_t)seg06);
    }
    gSPSegment(POLY_OPA_DISP++, 0x0C, (uintptr_t)gCullBackDList);
    gSPSegment(POLY_XLU_DISP++, 0x0C, (uintptr_t)gCullBackDList);

    // DARK LINK: render the whole puppet in grayscale, crushed toward black.
    gSPGrayscale(POLY_OPA_DISP++, true);
    gDPSetGrayscaleColor(POLY_OPA_DISP++, 40, 40, 70, 255);

    Matrix_SetTranslateRotateYXZ(dp->actor.world.pos.x, dp->actor.world.pos.y + yOff, dp->actor.world.pos.z,
                                 &dp->actor.shape.rot);
    Matrix_Scale(dp->actor.scale.x, dp->actor.scale.y, dp->actor.scale.z, MTXMODE_APPLY);

    Player_DrawImpl(play, dp->skelAnime.skeleton, dp->skelAnime.jointTable, dp->skelAnime.dListCount, 0,
                    PLAYER_FORM_HUMAN, dp->currentBoots, dp->actor.shape.face, Player_OverrideLimbDrawGameplayDefault,
                    Player_PostLimbDrawGameplay, &dp->actor);

    gSPGrayscale(POLY_OPA_DISP++, false);

    CLOSE_DISPS(play->state.gfxCtx);
}

// ============================================================================
// ACTOR PROFILE + SPAWN
// ============================================================================

ActorProfile RemainsAllyLink_Profile = {
    /**/ ACTOR_REMAINS_ALLY_LINK,
    /**/ ACTORCAT_MISC,
    /**/ (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED),
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(RemainsAllyLink),
    /**/ RemainsAllyLink_Init,
    /**/ RemainsAllyLink_Destroy,
    /**/ RemainsAllyLink_Update,
    /**/ RemainsAllyLink_Draw,
};

Actor* RemainsAllyLink_Spawn(PlayState* play, Vec3f* pos, s16 rotY) {
    if ((play == NULL) || (pos == NULL)) {
        return NULL;
    }
    return Actor_Spawn(&play->actorCtx, play, ACTOR_REMAINS_ALLY_LINK, pos->x, pos->y, pos->z, 0, rotY, 0, 0);
}
