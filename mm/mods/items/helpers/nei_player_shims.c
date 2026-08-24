/**
 * nei_player_shims.c — REAL MM implementations of the OoT/SoH player helpers the NEI item
 * code calls for POSES and player state. These replace the no-op link stubs that made Link
 * stand still while using items (the stubs are removed from nei_link_stubs.cpp).
 *
 * Included FIRST in the custom_items.c helper block (inside the z_player.c TU), so every
 * later item/equipment/sw97 file sees real prototypes instead of implicit declarations.
 */

// MM natives defined later in this TU (z_player.c) — forward declarations.
void Player_Anim_PlayLoop(PlayState* play, Player* this, PlayerAnimationHeader* anim);
PlayerAnimationHeader* Player_GetIdleAnim(Player* this);
void Player_StopHorizontalMovement(Player* this);
// 2ship resource manager (BenPort.cpp).
extern char* ResourceMgr_LoadPlayerAnimByName(const char* animPath);
extern u8 ResourceMgr_FileExists(const char* resName);

// OoT gPlayerLimbToBodyPart — rebuilt for MM's PlayerLimb/PlayerBodyPart enums (used by the
// extended-equipment draws to anchor gear to body parts). -1 = structural limb with no body part.
const s8 gPlayerLimbToBodyPart[PLAYER_LIMB_MAX] = {
    -1,                             // PLAYER_LIMB_NONE
    -1,                             // PLAYER_LIMB_ROOT
    PLAYER_BODYPART_WAIST,          // PLAYER_LIMB_WAIST
    -1,                             // PLAYER_LIMB_LOWER_ROOT
    PLAYER_BODYPART_RIGHT_THIGH,    // PLAYER_LIMB_RIGHT_THIGH
    PLAYER_BODYPART_RIGHT_SHIN,     // PLAYER_LIMB_RIGHT_SHIN
    PLAYER_BODYPART_RIGHT_FOOT,     // PLAYER_LIMB_RIGHT_FOOT
    PLAYER_BODYPART_LEFT_THIGH,     // PLAYER_LIMB_LEFT_THIGH
    PLAYER_BODYPART_LEFT_SHIN,      // PLAYER_LIMB_LEFT_SHIN
    PLAYER_BODYPART_LEFT_FOOT,      // PLAYER_LIMB_LEFT_FOOT
    -1,                             // PLAYER_LIMB_UPPER_ROOT
    PLAYER_BODYPART_HEAD,           // PLAYER_LIMB_HEAD
    PLAYER_BODYPART_HAT,            // PLAYER_LIMB_HAT
    PLAYER_BODYPART_COLLAR,         // PLAYER_LIMB_COLLAR
    PLAYER_BODYPART_LEFT_SHOULDER,  // PLAYER_LIMB_LEFT_SHOULDER
    PLAYER_BODYPART_LEFT_FOREARM,   // PLAYER_LIMB_LEFT_FOREARM
    PLAYER_BODYPART_LEFT_HAND,      // PLAYER_LIMB_LEFT_HAND
    PLAYER_BODYPART_RIGHT_SHOULDER, // PLAYER_LIMB_RIGHT_SHOULDER
    PLAYER_BODYPART_RIGHT_FOREARM,  // PLAYER_LIMB_RIGHT_FOREARM
    PLAYER_BODYPART_RIGHT_HAND,     // PLAYER_LIMB_RIGHT_HAND
    PLAYER_BODYPART_SHEATH,         // PLAYER_LIMB_SHEATH
    PLAYER_BODYPART_TORSO,          // PLAYER_LIMB_TORSO
};

// OoT Player_AnimPlayLoop → MM Player_Anim_PlayLoop (identical semantics).
void Player_AnimPlayLoop(PlayState* play, Player* this, LinkAnimationHeader* anim) {
    Player_Anim_PlayLoop(play, this, anim);
}

// OoT Player_ZeroSpeedXZ → MM native stop-horizontal (linearVelocity + actor.speed = 0).
void Player_ZeroSpeedXZ(Player* this) {
    Player_StopHorizontalMovement(this);
}

// OoT/SoH: i-frames for dodges (SoH also reset damageFlickerAnimCounter — no MM equivalent field).
void Player_SetIntangibility(Player* player, s32 timer) {
    if (player->invincibilityTimer >= 0) {
        player->invincibilityTimer = (s8)timer;
    }
}

// OoT Player_UnsetMask(play): clear the worn (non-transformation) mask.
void Player_UnsetMask(PlayState* play) {
    Player* p = GET_PLAYER(play);
    p->currentMask = PLAYER_MASK_NONE;
}

// SoH func_8083485C: generic "no special upper action" update used as the default updateFn in
// the NeiItem registry. SoH's version also begins shielding on R; that check lives deep in
// OoT's upper-action machine — returning false lets MM's own action pipeline continue. TODO.
s32 func_8083485C(Player* this, PlayState* play) {
    (void)this;
    (void)play;
    return false;
}

// SoH func_80853080: return-to-idle (Player_SetupAction(Idle) + idle anim morph + yaw resync).
// MM port: morph to MM's idle anim + resync yaw (MM's action machine keeps its current action;
// good enough for the visual reset the items want). TODO: wire a real MM idle action switch.
void func_80853080(Player* this, PlayState* play) {
    PlayerAnimation_Change(play, &this->skelAnime, Player_GetIdleAnim(this), 1.0f, 0.0f, 0.0f, ANIMMODE_ONCE, -6.0f);
    this->yaw = this->actor.shape.rot.y;
    Player_StopHorizontalMovement(this);
}

// OoT AnimationContext_SetLoadFrame → MM AnimTaskQueue_AddLoadPlayerFrame (identical args).
// Restores the async pose loads (gust-jar carry, foursword stance, pegasus stab).
void AnimationContext_SetLoadFrame(PlayState* play, LinkAnimationHeader* anim, s32 frame, s32 limbCount, Vec3s* dst) {
    AnimTaskQueue_AddLoadPlayerFrame(play, anim, frame, limbCount, dst);
}

// ResourceMgr_LoadPlayerAnimAsHeader is defined ONCE, canonically, in 2s2h/BenPort.cpp — that version
// WRAPS the raw SOH_PlayerAnimation payload in a real PlayerAnimationHeader (frameCount = totalS16/67) and
// caches it. This shim used to ALSO define it (returning the raw payload cast straight to a header — the
// exact out-of-bounds bug BenPort's version was written to fix), which left the symbol defined twice
// (here inside the z_player.c unity TU AND in BenPort.obj). That duplicate is what triggered
// `z_player.obj : LNK1179 duplicate COMDAT 'ResourceMgr_LoadPlayerAnimAsHeader'`. Removed — every caller
// links against BenPort's correct, cached implementation (LinkAnimationHeader == PlayerAnimationHeader).
