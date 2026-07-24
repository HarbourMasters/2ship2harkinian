/**
 * item_oot_boomerang.c — OoT Boomerang for human Link (Skijer's NEI).
 *
 * DIRECT 1:1 port of OoT's player-side boomerang chain (SoH z_player.c):
 *   func_80834F2C (use gate)      → OotBoom_UseGate
 *   func_80834D2C (aim entry)     → OotBoom_Entry
 *   func_80835884 (wind-up)       → OotBoom_UpperAction_WindUp
 *   func_808358F0 (aim loop)      → OotBoom_UpperAction_Aim
 *   func_808359FC (throw/spawn)   → OotBoom_UpperAction_Throw
 *   func_80835B60 (wait return)   → OotBoom_UpperAction_WaitForReturn
 *   func_80835C08 (catch)         → OotBoom_UpperAction_Catch
 *   func_80835800 (dispatcher)    → Player_UpperAction_OotBoomerang (installed for PLAYER_IA_BOOMERANG)
 *   Player_InitBoomerangIA        → Player_InitOotBoomerangIA
 *
 * Only names are adapted to MM's engine — no logic changes:
 *   OoT func_80834758 (shield gate)         == MM func_80830B88
 *   OoT func_80834EB8 (aim camera)          == MM func_80831010
 *   OoT sUseHeldItem / sHeldItemButtonIsHeldDown == MM sPlayerUseHeldItem / sPlayerHeldItemButtonIsHeldDown
 *   OoT unk_834 (aim indicator)             == MM unk_ACC   (MM's func_8082EF20 already switches the
 *                                              lower-body stance to link_boom_throw_waitR/L on it!)
 *   OoT unk_870 (arm blend, picks throwR/L) == MM unk_B40
 *   OoT unk_A73 = 4 (post-throw hand timer) == MM unk_D57 = 4
 *   OoT PLAYER_STATE1_USING_BOOMERANG (1<<24)  == MM PLAYER_STATE1_USING_ZORA_BOOMERANG
 *   OoT PLAYER_STATE1_BOOMERANG_THROWN (1<<25) == MM PLAYER_STATE1_ZORA_BOOMERANG_THROWN
 *   OoT LinkAnimation_* / boomerangActor    == MM PlayerAnimation_* / zoraBoomerangActor
 *
 * The flight actor is MM's native En_Boom spawned with params = OOT_BOOMERANG (single boomerang,
 * OoT damage/model/spin/grab — see z_en_boom.c). EnBoom_Destroy clears THROWN and raises
 * PLAYER_STATE3_ZORA_BOOMERANG_CAUGHT on catch, exactly like OoT clears BOOMERANG_THROWN.
 * The FOLLOWBOOMERANG camera keys off THROWN + zoraBoomerangActor natively (z_player.c ~12340).
 *
 * Included in the z_player.c TU via mods/items/logic/custom_items.c (unity build).
 */

#include "overlays/actors/ovl_En_Boom/z_en_boom.h"
#include "mods/anim_translator/mm_anim_loader.h"

// ---------------------------------------------------------------------------
// z_player.c internals (defined later in this TU) — forward declarations
// ---------------------------------------------------------------------------
s32 func_80830B88(PlayState* play, Player* this);          // OoT func_80834758: R-shield gate
bool func_80830F9C(PlayState* play);                       // bButtonAmmoPlusOne B-press check
bool func_80831010(Player* this, PlayState* play);         // OoT func_80834EB8: aim camera
void Player_SetUpperAction(PlayState* play, Player* this, PlayerUpperActionFunc upperActionFunc);
s32 Player_UpdateHostileLockOn(Player* this);
void Player_SetParallel(Player* this);
void Player_AnimSfx_PlayVoice(Player* this, u16 sfxId);
s32 OotBoom_CopyMeleeAttackJoints(Player* this, PlayState* play); // defined next to sMeleeAttackAnimInfo
extern s32 sPlayerUseHeldItem;              // OoT sUseHeldItem
extern s32 sPlayerHeldItemButtonIsHeldDown; // OoT sHeldItemButtonIsHeldDown

// Native MM anims (gPlayerAnim_link_boom_throw_waitR / link_uma_anim_walk are already declared as
// OTR path strings by objects/gameplay_keep/gameplay_keep.h, included earlier in this TU).
#define OOT_BOOM_NATIVE_WAITR ((PlayerAnimationHeader*)gPlayerAnim_link_boom_throw_waitR)

// Chain forward declarations (they reference each other).
static s32 OotBoom_UpperAction_WindUp(Player* this, PlayState* play);
static s32 OotBoom_UpperAction_Aim(Player* this, PlayState* play);
static s32 OotBoom_UpperAction_Throw(Player* this, PlayState* play);
static s32 OotBoom_UpperAction_WaitForReturn(Player* this, PlayState* play);
static s32 OotBoom_UpperAction_Catch(Player* this, PlayState* play);
s32 Player_UpperAction_OotBoomerang(Player* this, PlayState* play);

// ---------------------------------------------------------------------------
// Animations — the OoT-only ones come from the companion oot.o2r
// ---------------------------------------------------------------------------
// MM's gameplay_keep only kept the boomerang aim poses (link_boom_throw_wait{L,R}); the entry,
// throw and catch anims are OoT-only. Loaded through MmAnim_LoadByPath, which builds a proper
// runtime PlayerAnimationHeader from the raw frame data (the companion's own gameplay_keep anim
// headers keep UNRESOLVED segmented addresses that crash AnimTaskQueue_AddLoadPlayerFrame — see
// mm/expansions/sw97/actors/spells/z_magic_wind.inc.c for the precedent). OoT and MM human Link
// share the frame layout (22 limb-slots * 3 + 1 = 67 s16 per frame), so the data is drop-in.
typedef enum {
    /* 0 */ OOT_BOOM_ANIM_WAIT2WAITR, // gPlayerAnim_link_boom_throw_wait2waitR — short entry into aim
    /* 1 */ OOT_BOOM_ANIM_THROW_R,    // gPlayerAnim_link_boom_throwR
    /* 2 */ OOT_BOOM_ANIM_THROW_L,    // gPlayerAnim_link_boom_throwL
    /* 3 */ OOT_BOOM_ANIM_CATCH,      // gPlayerAnim_link_boom_catch
    /* 4 */ OOT_BOOM_ANIM_MAX
} OotBoomAnim;

typedef struct {
    const char* namedData;  // companion with named entries (misc/link_animetion/<sym>_Data)
    const char* headerPath; // SoH-style companion: gameplay_keep header (frameCount + data offset)
} OotBoomAnimPaths;

static const OotBoomAnimPaths sOotBoomAnimPaths[OOT_BOOM_ANIM_MAX] = {
    { "misc/link_animetion/gPlayerAnim_link_boom_throw_wait2waitR_Data",
      "__OTR__objects/gameplay_keep/gPlayerAnim_link_boom_throw_wait2waitR" },
    { "misc/link_animetion/gPlayerAnim_link_boom_throwR_Data",
      "__OTR__objects/gameplay_keep/gPlayerAnim_link_boom_throwR" },
    { "misc/link_animetion/gPlayerAnim_link_boom_throwL_Data",
      "__OTR__objects/gameplay_keep/gPlayerAnim_link_boom_throwL" },
    { "misc/link_animetion/gPlayerAnim_link_boom_catch_Data",
      "__OTR__objects/gameplay_keep/gPlayerAnim_link_boom_catch" },
};

// 2ship's AnimationFactory fully supports the companion's SoH Link-anim format: the resource is
// {type=Link, frameCount, dataPath} and the factory RESOLVES dataPath (the gPlayerAnimData_%06X
// payload) at import, so segmentVoid comes back as a ready RAM pointer — the header is directly
// usable. Verified against the user's oot.o2r: all four headers + their gPlayerAnimData entries
// exist, and none of its 573 link_animetion names clash with mm.o2r.
static PlayerAnimationHeader* OotBoom_GetAnim(s32 which) {
    const OotBoomAnimPaths* paths = &sOotBoomAnimPaths[which];
    PlayerAnimationHeader* anim = NULL;

    // 1) Companion gameplay_keep header (SoH-style o2r — the normal case).
    if (ResourceMgr_FileExists(paths->headerPath)) {
        extern AnimationHeaderCommon* ResourceMgr_LoadAnimByName(const char* path);
        PlayerAnimationHeader* hdr = (PlayerAnimationHeader*)ResourceMgr_LoadAnimByName(paths->headerPath);

        if ((hdr != NULL) && (hdr->common.frameCount > 0) && (hdr->segmentVoid != NULL)) {
            anim = hdr;
        }
    }

    // 2) Companion built with named raw-data entries instead (magic_wind precedent).
    if (anim == NULL) {
        anim = (PlayerAnimationHeader*)MmAnim_LoadByPath(paths->namedData, 15, 22);
    }

    // 3) Companion absent/incomplete: degrade to the native aim pose instead of crashing.
    if (anim == NULL) {
        anim = OOT_BOOM_NATIVE_WAITR;
    }
    return anim;
}

// ---------------------------------------------------------------------------
// IA init — OoT Player_InitBoomerangIA
// ---------------------------------------------------------------------------
void Player_InitOotBoomerangIA(PlayState* play, Player* this) {
    this->stateFlags1 |= PLAYER_STATE1_USING_ZORA_BOOMERANG; // OoT PLAYER_STATE1_USING_BOOMERANG (bit 24)
}

// ---------------------------------------------------------------------------
// Aim entry — OoT func_80834D2C (boomerang branch)
// ---------------------------------------------------------------------------
static s32 OotBoom_Entry(Player* this, PlayState* play) {
    Player_SetUpperAction(play, this, OotBoom_UpperAction_WindUp);
    this->unk_ACC = 10; // OoT this->unk_834 = 10 (aiming; also drives MM's boom-wait stance via func_8082EF20)
    PlayerAnimation_PlayOnce(play, &this->skelAnimeUpper, OotBoom_GetAnim(OOT_BOOM_ANIM_WAIT2WAITR));

    if (this->stateFlags1 & PLAYER_STATE1_800000) { // OoT PLAYER_STATE1_ON_HORSE
        Player_Anim_PlayLoop(play, this, (PlayerAnimationHeader*)gPlayerAnim_link_uma_anim_walk);
    } else if ((this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) && !Player_UpdateHostileLockOn(this)) {
        Player_Anim_PlayLoop(play, this, Player_GetIdleAnim(this));
    }
    return 1;
}

// ---------------------------------------------------------------------------
// Use gate — OoT func_80834F2C (== MM func_80831094, routed to our entry)
// ---------------------------------------------------------------------------
static s32 OotBoom_UseGate(Player* this, PlayState* play) {
    if ((this->doorType == PLAYER_DOORTYPE_NONE) && !(this->stateFlags1 & PLAYER_STATE1_ZORA_BOOMERANG_THROWN)) {
        if (sPlayerUseHeldItem || func_80830F9C(play)) {
            if (OotBoom_Entry(this, play)) {
                return func_80831010(this, play); // OoT func_80834EB8
            }
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Dispatcher (idle upper action for PLAYER_IA_BOOMERANG) — OoT func_80835800
// ---------------------------------------------------------------------------
s32 Player_UpperAction_OotBoomerang(Player* this, PlayState* play) {
    if (func_80830B88(play, this)) { // OoT func_80834758
        return true;
    }

    if (this->stateFlags1 & PLAYER_STATE1_ZORA_BOOMERANG_THROWN) {
        Player_SetUpperAction(play, this, OotBoom_UpperAction_WaitForReturn);
    } else if (OotBoom_UseGate(this, play)) {
        return true;
    }

    return false;
}

// ---------------------------------------------------------------------------
// Wind-up — OoT func_80835884
// ---------------------------------------------------------------------------
static s32 OotBoom_UpperAction_WindUp(Player* this, PlayState* play) {
    if (PlayerAnimation_Update(play, &this->skelAnimeUpper)) {
        Player_SetUpperAction(play, this, OotBoom_UpperAction_Aim);
        // OoT loops gPlayerAnim_link_boom_throw_waitR — still native in MM's gameplay_keep.
        PlayerAnimation_PlayLoop(play, &this->skelAnimeUpper, OOT_BOOM_NATIVE_WAITR);
    }

    func_80831010(this, play); // OoT func_80834EB8
    return true;
}

// ---------------------------------------------------------------------------
// Aim loop — OoT func_808358F0
// ---------------------------------------------------------------------------
static s32 OotBoom_UpperAction_Aim(Player* this, PlayState* play) {
    // OoT: while the main skelAnime plays the current melee attack (B-slash mid-aim), copy the
    // whole-body pose to the upper body instead of animating the aim.
    if (!OotBoom_CopyMeleeAttackJoints(this, play)) {
        PlayerAnimation_Update(play, &this->skelAnimeUpper);
    }

    func_80831010(this, play); // OoT func_80834EB8

    if (!sPlayerHeldItemButtonIsHeldDown) {
        Player_SetUpperAction(play, this, OotBoom_UpperAction_Throw);
        PlayerAnimation_PlayOnce(play, &this->skelAnimeUpper,
                                 (this->unk_B40 < 0.5f) // OoT (this->unk_870 < 0.5f)
                                     ? OotBoom_GetAnim(OOT_BOOM_ANIM_THROW_R)
                                     : OotBoom_GetAnim(OOT_BOOM_ANIM_THROW_L));
    }
    return true;
}

// ---------------------------------------------------------------------------
// Throw (spawns En_Boom on anim frame 6) — OoT func_808359FC (vanilla branch)
// ---------------------------------------------------------------------------
static s32 OotBoom_UpperAction_Throw(Player* this, PlayState* play) {
    if (PlayerAnimation_Update(play, &this->skelAnimeUpper)) {
        Player_SetUpperAction(play, this, OotBoom_UpperAction_WaitForReturn);
        this->unk_ACC = 0; // OoT this->unk_834 = 0
    } else if (PlayerAnimation_OnFrame(&this->skelAnimeUpper, 6.0f)) {
        f32 posX = (Math_SinS(this->actor.shape.rot.y) * 10.0f) + this->actor.world.pos.x;
        f32 posZ = (Math_CosS(this->actor.shape.rot.y) * 10.0f) + this->actor.world.pos.z;
        // OoT: +14000 (0x36B0) arc toward the Z-target when locked on (MM's Zora throw keeps the
        // same constant), else straight along Link's facing.
        s32 yaw = (this->focusActor != NULL) ? this->actor.shape.rot.y + 0x36B0 : this->actor.shape.rot.y;
        EnBoom* boomerang =
            (EnBoom*)Actor_Spawn(&play->actorCtx, play, ACTOR_EN_BOOM, posX, this->actor.world.pos.y + 30.0f, posZ,
                                 this->actor.focus.rot.x, yaw, 0, OOT_BOOMERANG);

        this->zoraBoomerangActor = (Actor*)boomerang; // OoT this->boomerangActor (FOLLOWBOOMERANG cam target)

        if (boomerang != NULL) {
            boomerang->moveTo = this->focusActor; // OoT homing handoff (moveTo = focusActor)
            // OoT returnTimer = 20 outbound frames. MM's En_Boom starts returning at unk_1CC <= 16,
            // so 20 + 16 = 36. unk_1CF/unk_1CE stay 0: proportional OoT turn law from the first
            // frames, no Zora fin launch arc.
            boomerang->unk_1CC = 36;

            this->stateFlags1 |= PLAYER_STATE1_ZORA_BOOMERANG_THROWN; // OoT PLAYER_STATE1_BOOMERANG_THROWN
            this->stateFlags3 &= ~PLAYER_STATE3_ZORA_BOOMERANG_CAUGHT;

            if (!Player_CheckHostileLockOn(this)) {
                Player_SetParallel(this);
            }

            this->unk_D57 = 4; // OoT this->unk_A73 = 4

            Player_PlaySfx(this, NA_SE_IT_BOOMERANG_THROW);
            Player_AnimSfx_PlayVoice(this, NA_SE_VO_LI_SWORD_N);
        }
    }
    return true;
}

// ---------------------------------------------------------------------------
// Airborne / wait for return — OoT func_80835B60
// ---------------------------------------------------------------------------
static s32 OotBoom_UpperAction_WaitForReturn(Player* this, PlayState* play) {
    if (func_80830B88(play, this)) { // OoT func_80834758
        return true;
    }

    // OoT watches BOOMERANG_THROWN being cleared by En_Boom on catch; MM's EnBoom_Destroy clears
    // the same bit for a single (child/parent-less) boomerang. The CAUGHT flag is MM's extra
    // signal — consume it too so it never leaks into the Zora fin chain.
    if (!(this->stateFlags1 & PLAYER_STATE1_ZORA_BOOMERANG_THROWN)) {
        Player_SetUpperAction(play, this, OotBoom_UpperAction_Catch);
        PlayerAnimation_PlayOnce(play, &this->skelAnimeUpper, OotBoom_GetAnim(OOT_BOOM_ANIM_CATCH));
        this->stateFlags3 &= ~PLAYER_STATE3_ZORA_BOOMERANG_CAUGHT;
        // (OoT also re-points the hand DL here via func_808357E8; our held model keys off the
        //  THROWN flag in z_player_lib.c's PostLimbDraw, so it reappears on its own.)
        Player_PlaySfx(this, NA_SE_PL_CATCH_BOOMERANG);
        Player_AnimSfx_PlayVoice(this, NA_SE_VO_LI_SWORD_N);
        return true;
    }

    return false;
}

// ---------------------------------------------------------------------------
// Catch — OoT func_80835C08
// ---------------------------------------------------------------------------
static s32 OotBoom_UpperAction_Catch(Player* this, PlayState* play) {
    if (!Player_UpperAction_OotBoomerang(this, play) && PlayerAnimation_Update(play, &this->skelAnimeUpper)) {
        Player_SetUpperAction(play, this, Player_UpperAction_OotBoomerang);
    }
    return true;
}

// ===========================================================================
// Iron Knuckle's Axe — tomahawk throw helpers (Skijer's NEI, hammer upgrade).
//
// DIRECT 1:1 of SoH's Player_StartIKAxeThrow / Player_EndIKAxeThrow (z_player.c). In OoT the
// thrown axe literally reuses the boomerang throw animation (gPlayerAnim_link_boom_throwR/L) and
// the handsfree boomerang wait-return upper action (func_80835B60), so it reuses the exact
// boomerang throw infrastructure ported above (including the crash-safe OotBoom_GetAnim loader —
// the raw gameplay_keep path-string idiom crashes AnimTaskQueue for player anims in MM).
//
// Name/engine adaptation only, no logic change:
//   SoH LinkAnimation_Update/PlayOnce   → MM PlayerAnimation_Update/PlayOnce
//   SoH this->upperSkelAnime            → MM this->skelAnimeUpper
//   SoH Player_SetUpperActionFunc(this, f) → MM Player_SetUpperAction(play, this, f)  (trivial set)
//   SoH func_80835B60 (handsfree wait)  → MM OotBoom_UpperAction_WaitForReturn
//   SoH func_8083485C (idle upper act)  → MM ExtPlayer_GetItemActionUpdateFunc(PLAYER_IA_HAMMER)
//   SoH this->unk_870 (throw R/L blend) → MM this->unk_B40
//
// The mod (equip_ikaxe_throw.inc.c) spawns the axe En_Boom itself and raises BOOMERANG_THROWN;
// these helpers only play Link's throw pose + drop him into the handsfree wait, and on catch
// restore the hammer's melee upper action (else the boomerang catch chain leaves the ranged
// upper action and the next C-press fires a bow).
// ---------------------------------------------------------------------------
static s32 Player_UpperAction_IKAxeThrow(Player* this, PlayState* play) {
    if (PlayerAnimation_Update(play, &this->skelAnimeUpper)) {
        Player_SetUpperAction(play, this, OotBoom_UpperAction_WaitForReturn);
    }
    return true;
}

void Player_StartIKAxeThrow(Player* this, PlayState* play) {
    Player_SetUpperAction(play, this, Player_UpperAction_IKAxeThrow);
    PlayerAnimation_PlayOnce(play, &this->skelAnimeUpper,
                             (this->unk_B40 < 0.5f) ? OotBoom_GetAnim(OOT_BOOM_ANIM_THROW_R)
                                                    : OotBoom_GetAnim(OOT_BOOM_ANIM_THROW_L));
}

void Player_EndIKAxeThrow(Player* this) {
    // Player_SetUpperAction is a trivial field assignment, so no PlayState is needed here.
    // ExtPlayer_GetItemActionUpdateFunc(PLAYER_IA_HAMMER) resolves to the same generic melee/idle
    // upper action SoH restores via func_8083485C.
    extern ItemActionUpdateFunc ExtPlayer_GetItemActionUpdateFunc(int32_t itemAction);
    this->upperActionFunc = (PlayerUpperActionFunc)ExtPlayer_GetItemActionUpdateFunc(PLAYER_IA_HAMMER);
    this->heldItemAction = PLAYER_IA_HAMMER;
    this->itemAction = PLAYER_IA_HAMMER;
}
