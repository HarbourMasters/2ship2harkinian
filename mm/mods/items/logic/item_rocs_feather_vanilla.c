/**
 * item_rocs_feather_vanilla.c — the SHIP-VANILLA Roc's Feather, ported 1:1 from Shipwright.
 *
 * WHICH FEATHER IS THIS
 * ---------------------
 * There are TWO Roc's Feathers in this fork and they are separate items that coexist freely:
 *
 *   ship-vanilla  ITEM_ROCS_FEATHER (0xA6)  <- THIS FILE. Shares the Nayru's Love cell on page 0,
 *                                              ownership = NeiSaveData.ootSpellsOwned bit 0x8.
 *   Skijer's      ITEM_ROCS_FEATHER_SKIJER  <- item_rocsfeather.c. Page 2, SLOT_ROCS, upgrades into
 *                                              the Roc's Cape.
 *
 * Owning one says nothing about owning the other. Do not merge them.
 *
 * SOURCE
 * ------
 * soh/soh/Enhancements/randomizer/RocsFeather.cpp. Every constant below is that file's, unchanged:
 * one jump per landing re-armed after 3 grounded frames, launch 7.0/7.5, forward nudge 5.0, ripple
 * 200->300, splash 150, effect scale 1.0/1.5.
 *
 * WHAT HAD TO CHANGE, AND WHY
 * ---------------------------
 * 1. SoH suppresses the item by answering false to VB_CHANGE_HELD_ITEM_AND_USE_ITEM. That hook does
 *    not exist in 2Ship — the C-button dispatch here is unhooked — so the equivalent is an early
 *    return at the top of Player_UseItem. Same net effect: the press is consumed and the vanilla
 *    use-item path never sees the item.
 * 2. OoT's func_80838940 is split in MM into func_80834D50 (setup + anim) and func_80834CD0 (the
 *    launch). func_80834D50 is the direct analog and is what we call.
 * 3. Field renames: OoT linearVelocity -> MM speedXZ.
 * 4. PLAYER_STATE2_HOPPING has no MM analog (nei_oot_compat.h stubs it to 0, which would make the
 *    clear a silent no-op). MM has no sidehop/backflip ledge-grab lockout bit to clear, so the line
 *    is dropped rather than pretended — see the note at the clear site below.
 * 5. The animation is SoH's own gPlayerAnim_link_rocs_feather_jump_Data, copied byte for byte into
 *    mm/assets/custom/misc/link_animetion/gPlayerAnim_nei_rocs_feather_jump. OoT and MM share the
 *    player animation format exactly (67 s16/frame, same 22-limb skeleton), so no translation is
 *    needed. It must be loaded through NeiAnim_Load: the archived resource is a RAW s16 payload, and
 *    casting it straight to a PlayerAnimationHeader reads frame-0 data as frameCount/segment.
 *
 * Skijer's NEI
 */

#include "z64.h"
#include "macros.h"
#include "functions.h"
#include "variables.h"
#include "z64effect_ss.h"            // EffectSsGRipple_Spawn / EffectSsGSplash_Spawn (not pulled in by z64.h)
#include "mods/extended_inventory.h" // NayrusWheel_HasRocs
#include "mods/items/anim/nei_anims.h"

// MM's jump helper pair (z_player.c). func_80834D50 is OoT's func_80838940: sets up the in-air
// action (Player_Action_25), plays the animation, then launches via func_80834CD0.
extern void func_80834D50(PlayState* play, Player* this, PlayerAnimationHeader* anim, f32 speed, u16 sfxId);

#define ROCS_FEATHER_MAX_USES 1
#define ROCS_FEATHER_REARM_FRAMES 3

static u8 sRocsUseCount = 0;
static u8 sRocsGroundTimer = 0;

/**
 * Per-frame. Re-arms the jump once Link has been grounded for a few frames.
 *
 * The delay is not cosmetic: bgCheckFlags reports GROUND for a frame or two during the launch
 * itself, so re-arming on the first grounded frame would hand back the jump mid-takeoff and turn
 * one press into an infinite climb. Called unconditionally from CustomItems_Update — it is the
 * counterpart of SoH's OnPlayerUpdate hook, and it must keep ticking even when the feather is not
 * the held item, or the counter would stay stuck wherever you left it.
 */
void RocsFeatherVanilla_Tick(Player* player) {
    if (player == NULL) {
        return;
    }

    if (player->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
        if (sRocsGroundTimer < ROCS_FEATHER_REARM_FRAMES) {
            sRocsGroundTimer++;
        }
    } else {
        sRocsGroundTimer = 0;
    }

    if (sRocsGroundTimer >= ROCS_FEATHER_REARM_FRAMES) {
        sRocsUseCount = 0;
    }
}

/**
 * Called from the top of Player_UseItem. Returns true when the press belonged to the feather, in
 * which case the caller must return immediately and let nothing else run for that item.
 *
 * Returning true even when the jump is on cooldown is deliberate and matches SoH: the item is
 * always "handled", the use just does nothing until it re-arms. Falling through instead would let
 * Player_UseItem process a 0xA6 it has no meaning for.
 */
s32 RocsFeatherVanilla_TryUse(PlayState* play, Player* this, s32 item) {
    LinkAnimationHeader* anim;
    Vec3f effectPos;
    f32 effectScale;

    if (item != ITEM_ROCS_FEATHER) {
        return 0;
    }
    // Ownership is the wheel's, not the cell's: the cell can be showing Nayru's Love while you still
    // own the feather, and the button would then hold a stale id.
    if (!NayrusWheel_HasRocs()) {
        return 0;
    }
    if (sRocsUseCount >= ROCS_FEATHER_MAX_USES) {
        return 1; // consumed, but still airborne / not re-armed
    }

    anim = NeiAnim_Load(NEI_ANIM_ROCS_FEATHER_JUMP);
    if (anim == NULL) {
        // o2r not regenerated yet. Refuse rather than play a NULL animation — func_80834D50 would
        // happily launch Link with no animation and leave him T-posing through the arc.
        Audio_PlaySfx(NA_SE_SY_ERROR);
        return 1;
    }

    sRocsUseCount++;

    // 5.8f is what SoH passes; func_80834D50 multiplies it by sWaterSpeedFactor and then the
    // explicit velocity.y below overwrites it on land. Keeping the call argument identical means the
    // underwater case (factor 0.5) still behaves the way it does in Shipwright.
    func_80834D50(play, this, (PlayerAnimationHeader*)anim, 5.8f, NA_SE_NONE);

    // Without this the previous action's animation morphs into the jump and Link visibly snaps.
    this->av2.actionVar2 = 1;
    this->speedXZ = 5.0f; // OoT linearVelocity — small forward nudge so it is a leap, not a hop
    this->actor.world.rot.y = this->yaw = this->actor.shape.rot.y;

    // MM has no child Link. SoH's child branch (7.0f / scale 1.0f) is therefore unreachable here;
    // the adult values are the only ones that can apply, so they are inlined rather than branched on
    // a constant. Deku/Goron/Zora forms deliberately get the same launch — the feather is not a
    // transformation item and SoH scales only by age.
    this->actor.velocity.y = 7.5f;
    effectScale = 1.5f;

    effectPos = this->actor.home.pos;
    effectPos.y += 3;
    EffectSsGRipple_Spawn(play, &effectPos, (s16)(200 * effectScale), (s16)(300 * effectScale), 1);
    EffectSsGSplash_Spawn(play, &effectPos, NULL, NULL, 0, (s16)(150 * effectScale));

    // SoH clears PLAYER_STATE2_HOPPING here to give back the ledge grab after a sidehop/backflip.
    // MM has no such bit — nei_oot_compat.h stubs the name to 0, so writing it would be a silent
    // no-op that reads like working code. Dropped on purpose; if a sidehop-into-feather ever turns
    // out to lose the ledge grab in MM, the fix belongs on MM's own hop state, not on a fake mask.

    Player_PlaySfx(this, NA_SE_PL_SKIP);
    return 1;
}
