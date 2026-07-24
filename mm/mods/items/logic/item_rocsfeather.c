/**
 * item_rocsfeather.c - Roc's Feather from Oracle games
 *
 * Controls:
 *   C Button: High jump (works on ground and in water)
 *
 * Features:
 *   - Single high jump with sparkle effects
 *   - Reduced jump velocity in water
 *
 * Jump animation:
 *   We run natively on MM, so the jump animation is a native MM Link animation played directly
 *   from gameplay_keep (no anim translator / mm.o2r lookup / CVar opt-in needed — that machinery
 *   only existed for the SoH/OoT base where MM anims are foreign). Played after a 2-frame delay so
 *   the engine's own jump-state animation change settles first.
 *
 * State tracking:
 *   rfMmAnimTimer > 0 means "in air due to Roc's item"
 *   rfMmAnimTimer < 0 means "pending animation" (waiting frames before playing)
 */

#include "z64.h"
#include "item_rocsfeather.h"
#include "../custom_items.h"
#include "../helpers/equip_helper.h"
#include "../helpers/fx_helper.h"
#include "../helpers/item_voice.h"
#include "macros.h"
#include "functions.h"
#include "variables.h"

// Native MM jump animations. gameplay_keep is included earlier in this z_player.c TU, so these are
// already declared as OTR path strings — cast straight to PlayerAnimationHeader* (same idiom as
// item_oot_boomerang.c). Skijer's NEI
#define ROCS_ANIM_BACKFLIP ((PlayerAnimationHeader*)gPlayerAnim_link_fighter_backturn_jump)
#define ROCS_ANIM_ROLLJUMP ((PlayerAnimationHeader*)gPlayerAnim_link_normal_newroll_jump_20f)

// z_player.c internal (not in any header); this item file is pasted into the z_player.c TU before
// the definition, so forward-declare it (same as item_oot_spells.c).
void Player_Anim_PlayOnce(PlayState* play, Player* this, PlayerAnimationHeader* anim);

// Pending animation type (stored when waiting for delay)
static s32 sRfPendingAnimType = 0; // 0=none, 1=backflip, 2=roll jump

void Handle_RocsFeather(Player* p, PlayState* play) {
    ItemInputState in;
    ItemInput_Update(&in, ITEM_ROCS_FEATHER_SKIJER, p, play);

    if (!in.wasEquipped)
        return;
    if (ItemInput_IsBlockedEx(p, play, 1))
        return; // Skip water blocker - Roc's Feather works in water

    s32 isOnGround = (p->actor.bgCheckFlags & BGCHECKFLAG_GROUND);
    s32 inWater = (p->stateFlags1 & PLAYER_STATE1_IN_WATER);

    // Reset states when on ground
    if (isOnGround) {
        rfMmAnimTimer = 0;
        sRfPendingAnimType = 0;
    }

    // Handle pending animation (negative timer = waiting frames)
    if (rfMmAnimTimer < 0) {
        rfMmAnimTimer++; // Count towards 0
        if (rfMmAnimTimer == 0) {
            // Delay finished, play the native MM jump animation now
            PlayerAnimationHeader* anim = (sRfPendingAnimType == 2) ? ROCS_ANIM_ROLLJUMP : ROCS_ANIM_BACKFLIP;
            Player_Anim_PlayOnce(play, p, anim);
            sRfPendingAnimType = 0;
            rfMmAnimTimer = 999; // Now in "air by Roc's" state
        }
    }

    if (!in.isPressed)
        return;

    // Can jump on ground OR in water (with reduced force)
    if (isOnGround || inWater) {
        f32 jumpVel = inWater ? ROCSFEATHER_WATER_JUMP_VELOCITY : ROCSFEATHER_JUMP_VELOCITY;
        p->actor.velocity.y = jumpVel;
        ItemVoice_Play(p, ROCSFEATHER_SOUND_JUMP_ADULT, ROCSFEATHER_SOUND_JUMP_CHILD);
        FX_SpawnSparkles(p, play);

        // Schedule the native MM jump animation after a 2-frame delay (let the engine's own
        // jump-state animation change settle first, else it overwrites ours immediately).
        rfMmAnimTimer = -2; // Negative = pending, will count up to 0
        sRfPendingAnimType = CVarGetInteger("gMods.RocsItems.InvertAnims", 0) ? 2 : 1;
    }
}
