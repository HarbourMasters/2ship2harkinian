/**
 * equip_ikaxe.c - Hammer Upgrade: Iron Knuckle's Axe
 *
 * Level 2 of the progressive Hammer (RG_PROGRESSIVE_HAMMER / weaponUpgrades bit).
 * The player wields their real hammer on a C-button; this only ENHANCES it — no
 * sword-slot hijack, no forced equip. Driven by WeaponUpgrade_HasHammerAxe()
 * from ExtEquip_UpdateBehavior (independent of the extended-equipment cheat).
 *
 * While the hammer is out:
 * - Chunky anim speeds (slow windup → fast impact)
 * - Double damage via meleeWeaponQuads
 * - 2x hitbox reach (in z_player_lib.c)
 * - Slower walk speed
 * - Hammer DL is hidden and the Iron Knuckle Axe model is drawn in its place
 * - Tomahawk throw (R + B hold)
 *
 * Included by ext_equip_behavior.c (unity build).
 */

// Inline IK Axe DL (extracted from decomp, segments resolved)
#include "equipment/objects/ikaxe_DL/model.inc.c"

#include "overlays/actors/ovl_En_Boom/z_en_boom.h"

// First-person aim (FirstPerson_*) and equipped-button polling (ItemInput_*), used by
// equip_ikaxe_throw.inc.c for the rod-style C-Up aim + equipped-C launch.
#include "items/helpers/camera_helper.h"
#include "items/helpers/equip_helper.h"

// z_player.c helper: plays Link's boomerang throw animation and drops him into the vanilla
// handsfree boomerang-out state while the axe is in the air (no spawn — the mod spawns the axe).
extern void Player_StartIKAxeThrow(Player* this, PlayState* play);
// z_player.c helper: on catch, restore the hammer's melee upper-action (the boomerang catch chain
// leaves the ranged-item upper-action, which makes the next C-press fire a bow).
extern void Player_EndIKAxeThrow(Player* this);
// z_player.c: the OoT bow/boomerang held-aim camera setter (OoT func_80834EB8). Puts the player in
// unk_AA5 = PLAYER_UNKAA5_3 → func_8083868C picks CAM_MODE_SLINGSHOT (a HELD aim that keeps the
// yaw + draws Link's body), instead of the C-Up first-person peek (which re-centered + hid the
// body so the reticle couldn't draw). Setting unk_AA5 != 0 also makes Player_ActionHandler_0 skip
// the vanilla C-Up first-person entry (func_80831814) on its own.
extern bool func_80831010(Player* this, PlayState* play);

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
#define IKAXE_ANIM_SLOW 0.35f     // Slow windup start (heavy)
#define IKAXE_ANIM_FAST 1.50f     // Fast at impact
#define IKAXE_MAX_WALK_SPEED 6.0f // Slower walk
#define IKAXE_DOUBLE_DAMAGE 2     // Damage multiplier
#define IKAXE_THROW_HOLD 15       // Hold B frames to throw
#define IKAXE_THROW_RETURN 30     // Flight frames before return
#define IKAXE_THROW_PARAMS 99     // En_Boom params for axe variant

// ---------------------------------------------------------------------------
// Tomahawk throw state
// ---------------------------------------------------------------------------
static s16 sIKAxeBHoldFrames = 0;
static u8 sIKAxeThrown = 0;
static u8 sIKAxeAimActive = 0; // first-person aim is on

// ---------------------------------------------------------------------------
// Animation Speed Override (chunky)
// ---------------------------------------------------------------------------
static void IKAxe_ModifyAnimSpeed(Player* player) {
    if (player->meleeWeaponState == 0)
        return;

    f32 totalFrames = player->skelAnime.endFrame;
    if (totalFrames <= 0.0f)
        return;

    f32 progress = player->skelAnime.curFrame / totalFrames;

    // Smooth ease-in curve: starts slow (heavy windup), smoothly accelerates to impact.
    // t^2 gives a natural "weight" feel — no abrupt speed change.
    f32 t = progress * progress; // quadratic ease-in
    player->skelAnime.playSpeed = IKAXE_ANIM_SLOW + (IKAXE_ANIM_FAST - IKAXE_ANIM_SLOW) * t;
}

// ---------------------------------------------------------------------------
// Tomahawk Throw States
// ---------------------------------------------------------------------------
#define IKAXE_THROW_IDLE 0
#define IKAXE_THROW_CHARGING 1 // Holding B, aim pose
#define IKAXE_THROW_FLYING 2   // Axe in the air

static u8 sIKAxeThrowState = IKAXE_THROW_IDLE;

// Throw system in separate file for clarity
#include "equip_ikaxe_throw.inc.c"

// ---------------------------------------------------------------------------
// Per-frame Behavior
// ---------------------------------------------------------------------------
static void IKAxe_Behavior(Player* player, PlayState* play) {
    if (player->stateFlags1 & (PLAYER_STATE1_DEAD | PLAYER_STATE1_IN_CUTSCENE | PLAYER_STATE1_LOADING |
                               PLAYER_STATE1_IN_ITEM_CS | PLAYER_STATE1_GETTING_ITEM)) {
        return;
    }

    // Mark active so IKAxe_Cleanup resets throw state when the upgrade is lost.
    gExtEquipBehavior.ikAxeActive = 1;

    // The player wields their REAL Megaton Hammer (on a C-button). We do NOT touch
    // buttonItems / the sword slot — this is an upgrade ON the hammer. isHolding is
    // simply "the hammer is currently out". Its natural cycle drives everything.
    u8 isHolding = (player->heldItemAction == PLAYER_IA_HAMMER);

    // Signal draw system: hide vanilla sword DL only when hammer is out
    gExtEquipBehavior.ikAxeDrawing = isHolding || (sIKAxeThrowState == IKAXE_THROW_CHARGING);

    // Double damage only when holding. When NOT holding, restore the vanilla melee-quad damage
    // (D_80854650 init = 1) so the doubled value doesn't follow Link onto the sword after the
    // hammer is put away — a stale value here changes the sword's hit and can break feel.
    if (isHolding) {
        player->meleeWeaponQuads[0].elem.atDmgInfo.damage = 4 * IKAXE_DOUBLE_DAMAGE;
        player->meleeWeaponQuads[1].elem.atDmgInfo.damage = 4 * IKAXE_DOUBLE_DAMAGE;
    } else {
        player->meleeWeaponQuads[0].elem.atDmgInfo.damage = 1;
        player->meleeWeaponQuads[1].elem.atDmgInfo.damage = 1;
    }

    // Tomahawk throw (R + B hold) — only when holding hammer
    IKAxe_UpdateThrow(player, play);

    // Break the Power Keg's gauntlet-tier obstacles during an Axe swing (silver rocks, heavy
    // blocks, rolling boulders). Defined in power_keg.c (item TU) — reached via extern.
    {
        extern void IKAxe_SwingBreakSweep(Player * player, PlayState * play);
        IKAxe_SwingBreakSweep(player, play);
    }

    // Walk cap + chunky anims only when holding and not throwing
    if (sIKAxeThrowState == IKAXE_THROW_IDLE && isHolding) {
        if (player->meleeWeaponState == 0 && player->speedXZ > IKAXE_MAX_WALK_SPEED) { // MM: OoT linearVelocity
            player->speedXZ = IKAXE_MAX_WALK_SPEED;
        }
        IKAxe_ModifyAnimSpeed(player);
    }
}

// ---------------------------------------------------------------------------
// Cleanup
// ---------------------------------------------------------------------------
static void IKAxe_Cleanup(void) {
    if (!gExtEquipBehavior.ikAxeActive)
        return;

    // No sword/buttonItems to restore — the hammer upgrade never hijacked them.
    // Just clear throw/aim/draw state so a stale axe model isn't left drawn.
    sIKAxeAimActive = 0;
    sIKAxeThrowState = IKAXE_THROW_IDLE;
    sIKAxeBHoldFrames = 0;
    sIKAxeThrown = 0;
    gExtEquipBehavior.ikAxeDrawing = 0;
    gExtEquipBehavior.ikAxeActive = 0;
}

// ---------------------------------------------------------------------------
// Draw — IK Axe DL on XLU
// ---------------------------------------------------------------------------
static void IKAxe_DrawAxe(PlayState* play) {
    // Axe is flying — En_Boom draws it
    if (sIKAxeThrowState == IKAXE_THROW_FLYING) {
        return;
    }

    // Free mode (putaway) — no axe in hand
    Player* drawPlayer = GET_PLAYER(play);
    if (drawPlayer->heldItemAction != PLAYER_IA_HAMMER) {
        return;
    }

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL_25Opa(play->state.gfxCtx);

    // Seat the axe in MM's young-Link LEFT-hand limb space (values dialed in in-game).
    Matrix_Translate(181.0f, -119.0f, -480.0f, MTXMODE_APPLY);
    Matrix_RotateZYX(-15185, -6854, 15863, MTXMODE_APPLY);
    Matrix_Scale(1.19f, 1.19f, 1.19f, MTXMODE_APPLY);

    gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(POLY_XLU_DISP++, gIKAxeInlineDL);

    CLOSE_DISPS(play->state.gfxCtx);
}
