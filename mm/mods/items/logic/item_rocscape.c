/**
 * item_rocscape.c - Roc's Cape from Four Swords Adventures
 *
 * Jump animation (native MM):
 *   - Ground jump: MM backflip (plays once after 2 frame delay)
 *   - Double jump: MM roll jump (plays once)
 *   Played directly from gameplay_keep — we run natively on MM, so no anim translator / mm.o2r
 *   lookup / CVar opt-in is needed (that machinery only existed for the SoH/OoT base).
 *
 * State tracking:
 *   rcMmAnimTimer > 0 means "in air due to Roc's item"
 *   rcMmAnimTimer < 0 means "pending animation" (waiting frames before playing)
 */

#include "z64.h"
#include "item_rocscape.h"
#include "../custom_items.h"
#include "../helpers/equip_helper.h"
#include "../helpers/fx_helper.h"
#include "../helpers/item_voice.h"
#include "macros.h"
#include "functions.h"
#include "variables.h"

// Native MM jump animations (gameplay_keep is included earlier in this z_player.c TU). Skijer's NEI
#define ROCS_ANIM_BACKFLIP ((PlayerAnimationHeader*)gPlayerAnim_link_fighter_backturn_jump)
#define ROCS_ANIM_ROLLJUMP ((PlayerAnimationHeader*)gPlayerAnim_link_normal_newroll_jump_20f)

// z_player.c internal (not in any header) — forward-declare for this TU.
void Player_Anim_PlayOnce(PlayState* play, Player* this, PlayerAnimationHeader* anim);

// Pending animation type (stored when waiting for delay)
static s32 sPendingAnimType = 0; // 0=none, 1=backflip, 2=roll jump

void Handle_RocsCape(Player* p, PlayState* play) {
    ItemInputState in;
    ItemInput_Update(&in, ITEM_ROCS_CAPE, p, play);

    if (!in.wasEquipped)
        return;
    if (ItemInput_IsBlockedEx(p, play, 1))
        return;

    s32 isOnGround = (p->actor.bgCheckFlags & BGCHECKFLAG_GROUND);
    s32 inWater = (p->stateFlags1 & PLAYER_STATE1_IN_WATER);

    // Reset states when on ground
    if (isOnGround) {
        rcJumpCount = 0;
        rcMmAnimTimer = 0;
        sPendingAnimType = 0;
    }

    // Handle pending animation (negative timer = waiting frames)
    if (rcMmAnimTimer < 0) {
        rcMmAnimTimer++; // Count towards 0
        if (rcMmAnimTimer == 0) {
            // Delay finished, play the native MM jump animation now
            PlayerAnimationHeader* anim = (sPendingAnimType == 2) ? ROCS_ANIM_ROLLJUMP : ROCS_ANIM_BACKFLIP;
            Player_Anim_PlayOnce(play, p, anim);
            sPendingAnimType = 0;
            rcMmAnimTimer = 999; // Now in "air by Roc's" state
        }
    }

    if (!in.isPressed)
        return;

    if (isOnGround || inWater) {
        // Ground/water jump (first jump)
        f32 jumpVel = inWater ? ROCSCAPE_WATER_JUMP_VELOCITY : ROCSCAPE_JUMP_VELOCITY;
        p->actor.velocity.y = jumpVel;
        ItemVoice_Play(p, ROCSCAPE_SOUND_JUMP_ADULT, ROCSCAPE_SOUND_JUMP_CHILD);
        FX_SpawnSparkles(p, play);

        // Schedule the native MM jump animation after a 2-frame delay
        rcMmAnimTimer = -2; // Negative = pending, will count up to 0
        sPendingAnimType = CVarGetInteger("gMods.RocsItems.InvertAnims", 0) ? 2 : 1;

    } else if (rcJumpCount == 0) {
        // Double jump (second jump, while in air)
        rcJumpCount = 1;
        p->actor.velocity.y = ROCSCAPE_DOUBLE_JUMP_VELOCITY;
        ItemVoice_Play(p, ROCSCAPE_SOUND_DOUBLE_ADULT, ROCSCAPE_SOUND_DOUBLE_CHILD);
        FX_SpawnSparkles(p, play);

        // Spawn shockwave
        Vec3f shockwavePos;
        shockwavePos.x = p->actor.world.pos.x;
        shockwavePos.y = p->actor.floorHeight + 2.0f;
        shockwavePos.z = p->actor.world.pos.z;
        FX_SpawnShockwaveSmall(play, &shockwavePos, 60, 150);

        // Schedule the native MM jump animation after a 2-frame delay (inverted vs the ground jump:
        // the double jump gets the roll by default).
        rcMmAnimTimer = -2;
        sPendingAnimType = CVarGetInteger("gMods.RocsItems.InvertAnims", 0) ? 1 : 2;
    }
}
