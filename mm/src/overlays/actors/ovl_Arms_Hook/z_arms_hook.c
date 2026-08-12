/*
 * File: z_arms_hook.c
 * Overlay: ovl_Arms_Hook
 * Description: Hookshot tip
 */

#include "z_arms_hook.h"
#include "objects/gameplay_keep/gameplay_keep.h"
#include "objects/object_link_child/object_link_child.h"
#include "2s2h/GameInteractor/GameInteractor.h"
// Skijer's NEI: the switch hook's live "look at it to pick it" selection is now the shared
// remote selector, so the Phantom Hourglass and friends reuse the exact same feel.
#include "../../../../mods/items/helpers/target_select_helper.h"

#define FLAGS (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED)

void ArmsHook_Init(Actor* thisx, PlayState* play);
void ArmsHook_Destroy(Actor* thisx, PlayState* play);
void ArmsHook_Update(Actor* thisx, PlayState* play);
void ArmsHook_Draw(Actor* thisx, PlayState* play);

void ArmsHook_Wait(ArmsHook* this, PlayState* play);
void ArmsHook_Shoot(ArmsHook* this, PlayState* play);
void ArmsHook_SwitchSwap(ArmsHook* this, PlayState* play); // Skijer's NEI switchhook: OoA swap state

// Skijer's NEI switchhook — Oracle of Ages INSTANT position swap (one hook exists at a time, so module
// statics are safe). StartSwap snapshots both actors' hitboxes and teleports them into each other's
// spot; SwitchSwap then HOLDS them there for a couple frames so the aim action can't clobber the
// teleport, then releases Link; ArmsHook_Update re-anchors the hitboxes onto their real position every
// frame meanwhile. sSwap*Start = each actor's ORIGINAL spot, which is the OTHER actor's destination.
#define ARMSHOOK_SWAP_HOLD_FRAMES 2
static Vec3f sSwapLinkStart;
static Vec3f sSwapTargetStart;
static Actor* sSwapTarget;
static s16 sSwapTimer;

// Skijer's NEI switchhook — hitbox re-anchoring (item_switchhook.c). world.pos moves the model, never
// the colliders, and a one-shot shift by the teleport vector is not enough: after the swap each actor
// is still moved by its OWN update — shoved back out of a wall it materialised inside, dropped onto
// the floor below — and the hitbox has to follow every bit of that. So both actors are re-anchored
// onto their current position each frame until they stop moving. The window is generous because a
// swapped object can fall for a while; re-anchoring is an absolute reposition, so it costs nothing to
// keep running and cannot drift.
#define ARMSHOOK_SWAP_SETTLE_FRAMES 40
extern void SwitchHook_CaptureSwapColliders(PlayState* play, s32 slot, Actor* actor);
extern void SwitchHook_ReanchorSwapColliders(PlayState* play);
extern void SwitchHook_ClearSwapColliders(void);
static s16 sSwapAnchorTimer = 0;

// Skijer's NEI switchhook — while > 0, the PLAYER's scene collision is fully bypassed, exactly like
// the gCheats.NoClip CVar (z_bgcheck.c consults SwitchHook_PlayerNoClip() next to that CVar), so the
// swap can materialize Link behind walls / below floors.
static s16 sSwitchNoClipTimer = 0;

s32 SwitchHook_PlayerNoClip(void) {
    return sSwitchNoClipTimer > 0;
}

// Skijer's NEI switchhook — an actor is swappable if it's an enemy, a prop (pots, crates, torches,
// grass...), a chest, or an NPC (signs, scarecrows, cuccos...). Bosses, the player, background and
// scene actors are never swapped. This is exactly target_select_helper's common-target notion, so
// the definition lives there now and both games (and every other item that wants a remote pick)
// share one copy.
static s32 ArmsHook_IsSwappable(Actor* actor) {
    return TargetSelect_IsCommonTarget(actor);
}

// Skijer's NEI switchhook — Ultrahand-style CONTINUOUS selection: while the switch hook is in hand,
// the swappable actor closest to Link's LOOK DIRECTION (yaw only — Y is ignored), within longshot
// range, is the live selection. It's tinted blue every frame; firing auto-aims the hook at it, so
// you can swap without precise aiming. The scan itself is TargetSelect_Scan.
static Actor* sSwitchSelection = NULL;

static Actor* ArmsHook_SelectSwapCandidate(PlayState* play) {
    return TargetSelect_Scan(play, TargetSelect_IsCommonTarget);
}

// Nearest swappable actor within `range` of `pos` (scans only the swappable categories). The switch
// hook uses this to find its target by PROXIMITY, so the swap can start before the hook's collider
// reaches the actor and breaks it (pots) / grabs it (chests).
static Actor* ArmsHook_FindSwappable(PlayState* play, Vec3f* pos, f32 range) {
    return TargetSelect_FindNearest(play, NULL, 0, TargetSelect_IsCommonTarget, pos, range);
}

ActorProfile Arms_Hook_Profile = {
    /**/ ACTOR_ARMS_HOOK,
    /**/ ACTORCAT_ITEMACTION,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(ArmsHook),
    /**/ ArmsHook_Init,
    /**/ ArmsHook_Destroy,
    /**/ ArmsHook_Update,
    /**/ ArmsHook_Draw,
};

static ColliderQuadInit D_808C1BC0 = {
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
        { 0x00000080, 0x00, 0x02 },
        { 0xF7CFFFFF, 0x00, 0x00 },
        ATELEM_ON | ATELEM_NEAREST | ATELEM_SFX_NORMAL,
        ACELEM_NONE,
        OCELEM_NONE,
    },
    { { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } } },
};

void ArmsHook_SetupAction(ArmsHook* this, ArmsHookActionFunc actionFunc) {
    this->actionFunc = actionFunc;
}

void ArmsHook_Init(Actor* thisx, PlayState* play) {
    ArmsHook* this = (ArmsHook*)thisx;

    Collider_InitQuad(play, &this->collider);
    Collider_SetQuad(play, &this->collider, &this->actor, &D_808C1BC0);
    ArmsHook_SetupAction(this, ArmsHook_Wait);
    this->unk1E0 = this->actor.world.pos;
}

void ArmsHook_Destroy(Actor* thisx, PlayState* play) {
    ArmsHook* this = (ArmsHook*)thisx;

    if (this->attachedActor != NULL) {
        this->attachedActor->flags &= ~ACTOR_FLAG_HOOKSHOT_ATTACHED;
    }
    // Skijer's NEI switchhook: drop the module-static selection/swap pointers with the hook actor,
    // so a scene change / item swap can't leave them dangling; end any player-noclip window.
    sSwitchSelection = NULL;
    sSwapTarget = NULL;
    sSwitchNoClipTimer = 0;
    sSwapAnchorTimer = 0;
    SwitchHook_ClearSwapColliders();
    Collider_DestroyQuad(play, &this->collider);
}

void ArmsHook_Wait(ArmsHook* this, PlayState* play) {
    extern u8 Nei_ArmsHookVariant(Player * player);

    // Skijer's NEI switchhook — Ultrahand-style live selection: every frame the hook is IN HAND,
    // pick the swappable actor in Link's look direction (Y ignored, longshot range) and tint it
    // blue. Firing then auto-aims at it, so you can swap without precise aiming.
    if (this->actor.parent != NULL) {
        if (Nei_ArmsHookVariant(GET_PLAYER(play)) == 4) { // NEI_HOOK_VARIANT_SWITCHHOOK
            extern u8 SwitchHook_IsAimingManual(void);

            if (SwitchHook_IsAimingManual()) {
                // C-Up manual aim: you fire exactly where you look — no live selection, no auto-aim.
                sSwitchSelection = NULL;
            } else {
                sSwitchSelection = ArmsHook_SelectSwapCandidate(play);
                if (sSwitchSelection != NULL) {
                    TargetSelect_Highlight(sSwitchSelection, 4);
                }
            }
        } else {
            sSwitchSelection = NULL;
        }
    }

    if (this->actor.parent == NULL) {
        // Skijer's NEI hookshot overhaul: reach + travel speed scale with the active hookshot-cell
        // variant (reach ~= speed * timer). Vanilla MM was a flat 20.0f / timer 26 (= Longshot).
        //   Hookshot (1): 20 * 13     Longshot (2): 20 * 26
        //   Ultrashot (4): 40 * 26 (2x speed)     Clawshot (2): 15 * 35 (0.75x speed)
        u8 variant = Nei_ArmsHookVariant(GET_PLAYER(play));
        f32 speed = 20.0f;
        s32 timer = 26;

        // Skijer's NEI switchhook — charges: each fired swap costs one of 5; empty/grayed out means
        // the shot simply doesn't come out (error beep, hook stays in hand).
        if (variant == 4) {
            extern s32 SwitchHook_ConsumeCharge(void);
            extern void SwitchHook_OnFired(Player* p);

            if (!SwitchHook_ConsumeCharge()) {
                // Should not be reached (func_80831194 blocks the launch player-side first), but if
                // it is: restore the FULL held state — parent alone leaves player->heldActor NULL
                // ("hook in flight" forever = softlock).
                Player* chargePlayer = GET_PLAYER(play);

                Audio_PlaySfx(NA_SE_SY_ERROR);
                this->actor.parent = &chargePlayer->actor;
                chargePlayer->heldActor = &this->actor;
                chargePlayer->actor.child = &this->actor;
                return;
            }
            // Manual aim ends at the launch (drops the aim camera; the shot direction is already
            // the player's aimed world.rot from the vanilla aim flow).
            SwitchHook_OnFired(GET_PLAYER(play));
        }
        switch (variant) {
            case 0: // NEI_HOOK_VARIANT_HOOKSHOT — dist 1
                speed = 20.0f;
                timer = 13;
                break;
            case 1: // NEI_HOOK_VARIANT_LONGSHOT — dist 2
                speed = 20.0f;
                timer = 26;
                break;
            case 2: // NEI_HOOK_VARIANT_ULTRASHOT — dist 4, 2x speed
                speed = 40.0f;
                timer = 26;
                break;
            case 3: // NEI_HOOK_VARIANT_CLAWSHOT — dist 2, 0.75x speed
                speed = 15.0f;
                timer = 35;
                break;
            case 4: // NEI_HOOK_VARIANT_SWITCHHOOK — dist 2 (Longshot range; dist 1 was too short); swaps on hit
                speed = 20.0f;
                timer = 26;
                break;
        }
        // Switch Hook: the swap is a clean position exchange — kill the hook's attack damage so it
        // doesn't BREAK the pot or damage/grab the actor it's supposed to swap with. Detection is by
        // proximity in ArmsHook_Shoot, not by a collider hit. Other variants keep DMG_HOOKSHOT.
        if (variant == 4) { // NEI_HOOK_VARIANT_SWITCHHOOK
            this->collider.elem.atDmgInfo.dmgFlags = 0;
            this->collider.elem.atDmgInfo.damage = 0;
        } else {
            this->collider.elem.atDmgInfo.dmgFlags = 0x00000080; // DMG_HOOKSHOT
            this->collider.elem.atDmgInfo.damage = 2;
        }

        // Switch Hook auto-aim: fly straight at the live selection (Actor_SetSpeeds builds the
        // velocity from world.rot, so aim BEFORE it). Refresh its tint so it stays blue in flight.
        if ((variant == 4) && (sSwitchSelection != NULL) && (sSwitchSelection->update != NULL)) {
            f32 dx = sSwitchSelection->world.pos.x - this->actor.world.pos.x;
            f32 dy = sSwitchSelection->focus.pos.y - this->actor.world.pos.y;
            f32 dz = sSwitchSelection->world.pos.z - this->actor.world.pos.z;
            f32 distXZ = sqrtf((dx * dx) + (dz * dz));

            this->actor.world.rot.y = Math_Atan2S(dx, dz);
            this->actor.world.rot.x = Math_Atan2S(-dy, distXZ);
            this->actor.shape.rot = this->actor.world.rot;
            TargetSelect_Highlight(sSwitchSelection, 30);
        }

        ArmsHook_SetupAction(this, ArmsHook_Shoot);
        Actor_SetSpeeds(&this->actor, speed);
        this->actor.parent = &GET_PLAYER(play)->actor;
        this->timer = timer;
    }
}

/**
 * Start pulling Player so he flies toward the hookshot's current location.
 * Setting Player's parent pointer indicates that he should begin flying.
 * See `Player_UpdateUpperBody` and `Player_Action_HookshotFly` for Player's side of the interation.
 */
void ArmsHook_PullPlayer(ArmsHook* this) {
    this->actor.child = this->actor.parent;
    this->actor.parent->parent = &this->actor;
}

s32 ArmsHook_AttachToPlayer(ArmsHook* this, Player* player) {
    player->actor.child = &this->actor;
    player->heldActor = &this->actor;
    if (this->actor.child != NULL) {
        player->actor.parent = this->actor.child = NULL;
        return 1;
    }
    return 0;
}

void ArmsHook_DetachFromActor(ArmsHook* this) {
    if (this->attachedActor != NULL) {
        this->attachedActor->flags &= ~ACTOR_FLAG_HOOKSHOT_ATTACHED;
        this->attachedActor = NULL;
    }
}

s32 ArmsHook_CheckForCancel(ArmsHook* this) {
    Player* player = (Player*)this->actor.parent;

    if (Player_IsHoldingHookshot(player)) {
        if ((player->itemAction != player->heldItemAction) || (player->actor.flags & ACTOR_FLAG_TALK) ||
            (player->stateFlags1 & (PLAYER_STATE1_DEAD | PLAYER_STATE1_4000000))) {
            this->timer = 0;
            ArmsHook_DetachFromActor(this);
            Math_Vec3f_Copy(&this->actor.world.pos, &player->rightHandWorld.pos);
            return 1;
        }
    }
    return 0;
}

void ArmsHook_AttachToActor(ArmsHook* this, Actor* actor) {
    actor->flags |= ACTOR_FLAG_HOOKSHOT_ATTACHED;
    this->attachedActor = actor;
    Math_Vec3f_Diff(&actor->world.pos, &this->actor.world.pos, &this->attachPointOffset);
}

// Skijer's NEI switchhook — begin the position swap with `target`: record start positions, STUN the
// target if it's an enemy (blue freeze, like the hookshot), SILENCE the flying-hook rattle, play the
// warp sfx, and hand off to the animated swap state.
static void ArmsHook_StartSwap(ArmsHook* this, PlayState* play, Player* player, Actor* target) {
    sSwapLinkStart = player->actor.world.pos;  // Link's original spot = target's destination
    sSwapTargetStart = target->world.pos;      // target's original spot = Link's destination
    sSwapTarget = target;
    sSwapTimer = 0;
    // Full player noclip (the gCheats.NoClip mechanism) through the swap + a few settle frames.
    sSwitchNoClipTimer = ARMSHOOK_SWAP_HOLD_FRAMES + 6;

    // Snapshot both actors' hitboxes WHILE THEY ARE STILL AT REST — the offsets recorded here are what
    // "the collider sits here relative to its actor" means for the rest of the swap.
    SwitchHook_CaptureSwapColliders(play, 0, &player->actor);
    SwitchHook_CaptureSwapColliders(play, 1, target);

    // INSTANT teleport: move BOTH actors (world.pos, prevPos, home.pos) into each other's spot right
    // away, freezing their physics. world.pos carries the model, the bg checks and the dyna mesh —
    // but NOT the colliders, which is what the re-anchor at the end of this function is for.
    player->actor.world.pos = sSwapTargetStart;
    player->actor.prevPos = sSwapTargetStart;
    player->actor.velocity.x = 0.0f;
    player->actor.velocity.y = 0.0f;
    player->actor.velocity.z = 0.0f;
    player->actor.speed = 0.0f;
    // Clear the ground/wall flags: with BGCHECKFLAG_GROUND still set, the player's scene collision
    // snaps him back up to his OLD floorHeight — a downward swap would move only the object.
    player->actor.bgCheckFlags = 0;
    player->invincibilityTimer = 20;

    target->world.pos = sSwapLinkStart;
    target->prevPos = sSwapLinkStart;
    target->home.pos = sSwapLinkStart;
    target->velocity.x = 0.0f;
    target->velocity.y = 0.0f;
    target->velocity.z = 0.0f;
    target->speed = 0.0f;
    target->bgCheckFlags = 0;

    // Skijer's NEI switchhook — MOVE THE HITBOXES, not just the models (the snapshot above was taken
    // before the teleport). world.pos alone leaves every collider behind, and a one-shot shift by the
    // teleport vector is not enough either: both actors keep being moved AFTER this — shoved back out
    // of a wall they materialised inside, dropped onto the floor below — and the hitbox has to follow
    // all of that. ArmsHook_Update re-anchors them onto their current position every frame until they
    // settle; this first pass just gets them right for the swap frame itself.
    SwitchHook_ReanchorSwapColliders(play);
    sSwapAnchorTimer = ARMSHOOK_SWAP_SETTLE_FRAMES;

    // Recolor the selected actor (highlight — the Ultrahand "recolor your selection" feel). For
    // enemies this doubles as the hookshot stun (blue freeze); props/chests/NPCs just flash to show
    // what got swapped.
    Actor_SetColorFilter(target, COLORFILTER_COLORFLAG_BLUE, 255, COLORFILTER_BUFFLAG_OPA, 80);
    // Kill the flying-hook chain rattle at its SOURCE: it's a flagged sfx re-emitted from the
    // player actor every frame while sfxId stays set (the "dragging" drone) — stopping the playing
    // instance alone let it re-spawn next frame.
    player->actor.sfxId = 0;
    AudioSfx_StopByPos(&player->actor.projectedPos);
    // Same treatment for the actor that just got yanked across the room: a continuous
    // sfx it was holding would carry on from its new spot with nothing to stop it.
    if (target->sfxId != 0) {
        target->sfxId = 0;
        AudioSfx_StopByPos(&target->projectedPos);
    }
    // Short ONE-SHOT swap cue. The previous NA_SE_EV_WARP_HOLE is a long looping ambience sample
    // that AudioSfx_StopByPos never managed to kill — it droned on forever after the swap.
    // Fired ONCE, so the continuous flag must come off. Bit 0x800 marks an sfx that the
    // caller re-requests every frame; the sound bank only ages those while they sit in
    // SFX_STATE_QUEUED, and its auto-reclaim path is explicitly gated on !(sfxId & 0xC00).
    // Fire one with the flag on and never ask again and it parks in PLAYING forever — that
    // is the sound that kept ringing after every swap. The sample is picked by sfxId & 0x1FF,
    // so clearing 0x800 changes the lifetime, never which sound you hear.
    Audio_PlaySfx_AtPos(&this->actor.projectedPos, NA_SE_IT_HOOKSHOT_STICK_OBJ - SFX_FLAG);
    ArmsHook_SetupAction(this, ArmsHook_SwitchSwap);
}

void ArmsHook_Shoot(ArmsHook* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    // Skijer's NEI hookshot overhaul: which variant is in flight (fixed for the duration of the shot).
    extern u8 Nei_ArmsHookVariant(Player * player);
    u8 hookVariant = Nei_ArmsHookVariant(player);
    u8 clawshot = (hookVariant == 3);   // NEI_HOOK_VARIANT_CLAWSHOT
    u8 switchhook = (hookVariant == 4); // NEI_HOOK_VARIANT_SWITCHHOOK — swaps positions on hit

    if ((this->actor.parent == NULL) || !Player_IsHoldingHookshot(player)) {
        ArmsHook_DetachFromActor(this);
        Actor_Kill(&this->actor);
        return;
    }

    Actor_PlaySfx_Flagged2(&player->actor, NA_SE_IT_HOOKSHOT_CHAIN - SFX_FLAG);
    ArmsHook_CheckForCancel(this);

    if ((this->timer != 0) && (this->collider.base.atFlags & AT_HIT) &&
        (this->collider.elem.atHitElem->elemMaterial != ELEM_MATERIAL_UNK4)) {
        Actor* touchedActor = this->collider.base.at;

        // Skijer's NEI — Switch Hook: it swaps via proximity in the flight branch (its collider deals
        // no damage), so a collider hit shouldn't normally reach here. Guard anyway — on a swappable
        // hit run the same swap, and never let the switch hook fall through to the vanilla grab/pull.
        if (switchhook) {
            if (ArmsHook_IsSwappable(touchedActor)) {
                ArmsHook_StartSwap(this, play, player, touchedActor);
                return;
            }
        }
        // Skijer's NEI — Clawshot: grab any NON-BOSS ENEMY (ACTORCAT_ENEMY excludes bosses, props,
        // and NPCs by category) and drag it back to Link, bypassing the vanilla HOOKSHOT_PULLS_* /
        // HOOKABLE gate. It never pulls Link toward the target — the clawshot only reels enemies in.
        else if (clawshot) {
            if ((touchedActor->update != NULL) && (touchedActor->category == ACTORCAT_ENEMY)) {
                ArmsHook_AttachToActor(this, touchedActor);
            }
        } else if ((touchedActor->update != NULL) &&
                   (touchedActor->flags & (ACTOR_FLAG_HOOKSHOT_PULLS_ACTOR | ACTOR_FLAG_HOOKSHOT_PULLS_PLAYER))) {
            if (this->collider.elem.atHitElem->acElemFlags & ACELEM_HOOKABLE) {
                ArmsHook_AttachToActor(this, touchedActor);
                if (CHECK_FLAG_ALL(touchedActor->flags, ACTOR_FLAG_HOOKSHOT_PULLS_PLAYER)) {
                    ArmsHook_PullPlayer(this);
                }
            }
        }
        this->timer = 0;
        Audio_PlaySfx_AtPos(&this->actor.projectedPos, NA_SE_IT_HOOKSHOT_STICK_CRE);

        return;
    }

    if (DECR(this->timer) == 0) {
        Actor* attachedActor = this->attachedActor;
        Vec3f bodyDistDiffVec;
        Vec3f newPos;
        f32 bodyDistDiff;
        f32 phi_f16;
        s32 pad;

        if (attachedActor != NULL) {
            if ((attachedActor->update == NULL) ||
                !CHECK_FLAG_ALL(attachedActor->flags, ACTOR_FLAG_HOOKSHOT_ATTACHED)) {
                attachedActor = NULL;
                this->attachedActor = NULL;
            } else if (this->actor.child != NULL) {
                f32 curActorOffsetXYZ = Actor_WorldDistXYZToActor(&this->actor, attachedActor);
                f32 attachPointOffsetXYZ = sqrtf(SQXYZ(this->attachPointOffset));

                // Keep the hookshot actor at the same relative offset as the initial attachment even if the actor moves
                Math_Vec3f_Diff(&attachedActor->world.pos, &this->attachPointOffset, &this->actor.world.pos);

                // If the actor the hookshot is attached to is moving, the hookshot's current relative
                // position will be different than the initial attachment position.
                // If the distance between those two points is larger than 50 units, detach the hookshot.
                if ((curActorOffsetXYZ - attachPointOffsetXYZ) > 50.0f) {
                    ArmsHook_DetachFromActor(this);
                    attachedActor = NULL;
                }
            }
        }

        {
            f32 velocity;

            bodyDistDiff =
                Math_Vec3f_DistXYZAndStoreDiff(&player->rightHandWorld.pos, &this->actor.world.pos, &bodyDistDiffVec);

            if (bodyDistDiff < 30.0f) {
                velocity = 0.0f;
                phi_f16 = 0.0f;
            } else {
                if (this->actor.child != NULL) {
                    velocity = 30.0f;
                } else if (attachedActor != NULL) {
                    velocity = 50.0f;
                } else {
                    velocity = 200.0f;
                }
                // Skijer's NEI — Ultrashot: everything twice as fast, including Link's travel.
                if (hookVariant == 2) { // NEI_HOOK_VARIANT_ULTRASHOT
                    velocity *= 2.0f;
                }
                phi_f16 = bodyDistDiff - velocity;
                if (bodyDistDiff <= velocity) {
                    phi_f16 = 0.0f;
                }
                velocity = phi_f16 / bodyDistDiff;
            }

            newPos.x = bodyDistDiffVec.x * velocity;
            newPos.y = bodyDistDiffVec.y * velocity;
            newPos.z = bodyDistDiffVec.z * velocity;
        }

        if (this->actor.child == NULL) {
            // Not pulling Player
            Math_Vec3f_Sum(&player->rightHandWorld.pos, &newPos, &this->actor.world.pos);
            if (attachedActor != NULL) {
                Math_Vec3f_Sum(&this->actor.world.pos, &this->attachPointOffset, &attachedActor->world.pos);
                // Skijer's NEI — Clawshot: zero the grabbed enemy's own motion so its AI (gravity,
                // walk, attack pursuit) can't fight the drag we apply via world.pos.
                if (clawshot) {
                    attachedActor->velocity.x = 0.0f;
                    attachedActor->velocity.y = 0.0f;
                    attachedActor->velocity.z = 0.0f;
                    attachedActor->speed = 0.0f;
                    attachedActor->gravity = 0.0f;
                }
            }
        } else {
            // Pulling Player
            Math_Vec3f_Diff(&bodyDistDiffVec, &newPos, &player->actor.velocity);
            player->actor.world.rot.x = Math_Atan2S_XY(sqrtf(SQXZ(bodyDistDiffVec)), -bodyDistDiffVec.y);
        }

        if (phi_f16 < 50.0f) {
            ArmsHook_DetachFromActor(this);
            if (phi_f16 == 0.0f) {
                ArmsHook_SetupAction(this, ArmsHook_Wait);
                if (ArmsHook_AttachToPlayer(this, player)) {
                    Math_Vec3f_Diff(&this->actor.world.pos, &player->actor.world.pos, &player->actor.velocity);
                    player->actor.velocity.y -= 20.0f;
                }
            }
        }
    } else {
        CollisionPoly* poly;
        s32 bgId;
        Vec3f posResult;
        Vec3f prevFrameDiff;
        Vec3f sp60;

        Actor_MoveWithGravity(&this->actor);

        // Skijer's NEI switchhook — once the hook has flown clear of Link, scout ahead for a swappable
        // actor (enemy/prop/chest) and start the position swap. Detecting by PROXIMITY (not by the
        // collider hit) means the swap fires before the hook reaches the actor, so pots aren't broken
        // and chests aren't grabbed on the way in. The "clear of Link" gate keeps an actor right next
        // to you from being grabbed the instant you fire past it.
        if (switchhook) {
            f32 hdx = this->actor.world.pos.x - player->actor.world.pos.x;
            f32 hdy = this->actor.world.pos.y - player->actor.world.pos.y;
            f32 hdz = this->actor.world.pos.z - player->actor.world.pos.z;
            if (((hdx * hdx) + (hdy * hdy) + (hdz * hdz)) > (50.0f * 50.0f)) {
                Actor* swapTarget = NULL;

                if ((sSwitchSelection != NULL) && (sSwitchSelection->update != NULL)) {
                    // A live selection was made — the hook auto-aims at it, so swap ONLY with it
                    // (an unselected object crossing the path must not intercept the swap).
                    f32 sdx = sSwitchSelection->world.pos.x - this->actor.world.pos.x;
                    f32 sdy = sSwitchSelection->world.pos.y - this->actor.world.pos.y;
                    f32 sdz = sSwitchSelection->world.pos.z - this->actor.world.pos.z;

                    if (((sdx * sdx) + (sdy * sdy) + (sdz * sdz)) < (60.0f * 60.0f)) {
                        swapTarget = sSwitchSelection;
                    }
                } else {
                    swapTarget = ArmsHook_FindSwappable(play, &this->actor.world.pos, 45.0f);
                }
                if (swapTarget != NULL) {
                    ArmsHook_StartSwap(this, play, player, swapTarget);
                    return;
                }
            }
        }

        Math_Vec3f_Diff(&this->actor.world.pos, &this->actor.prevPos, &prevFrameDiff);
        Math_Vec3f_Sum(&this->unk1E0, &prevFrameDiff, &this->unk1E0);
        this->actor.shape.rot.x = Math_Atan2S_XY(this->actor.speed, -this->actor.velocity.y);
        sp60.x = this->unk1EC.x - (this->unk1E0.x - this->unk1EC.x);
        sp60.y = this->unk1EC.y - (this->unk1E0.y - this->unk1EC.y);
        sp60.z = this->unk1EC.z - (this->unk1E0.z - this->unk1EC.z);
        // Skijer's NEI switchhook — NOCLIP: the switch hook flies straight THROUGH scene geometry
        // (no wall/floor line test), so it can reach the blue-selected object even behind a wall.
        if (!switchhook &&
            BgCheck_EntityLineTest1(&play->colCtx, &sp60, &this->unk1E0, &posResult, &poly, true, true, true, true,
                                    &bgId) &&
            (!func_800B90AC(play, &this->actor, poly, bgId, &posResult) ||
             BgCheck_ProjectileLineTest(&play->colCtx, &sp60, &this->unk1E0, &posResult, &poly, true, true, true, true,
                                        &bgId))) {
            f32 nx = COLPOLY_GET_NORMAL(poly->normal.x);
            f32 nz = COLPOLY_GET_NORMAL(poly->normal.z);

            Math_Vec3f_Copy(&this->actor.world.pos, &posResult);
            this->actor.world.pos.x += 10.0f * nx;
            this->actor.world.pos.z += 10.0f * nz;
            this->timer = 1;
            // Skijer's NEI — Clawshot never grapples surfaces (it only reels enemies in), so it
            // reflects off every wall/floor instead of pulling Link toward the anchor.
            if (!clawshot && SurfaceType_IsHookshotSurface(&play->colCtx, poly, bgId)) {
                DynaPolyActor* dynaPolyActor;

                if (bgId != BGCHECK_SCENE) {
                    dynaPolyActor = DynaPoly_GetActor(&play->colCtx, bgId);
                    if (dynaPolyActor != NULL) {
                        ArmsHook_AttachToActor(this, &dynaPolyActor->actor);
                    }
                }
                ArmsHook_PullPlayer(this);
                Audio_PlaySfx_AtPos(&this->actor.projectedPos, NA_SE_IT_HOOKSHOT_STICK_OBJ);
            } else {
                CollisionCheck_SpawnShieldParticlesMetal(play, &this->actor.world.pos);
                Audio_PlaySfx_AtPos(&this->actor.projectedPos, NA_SE_IT_HOOKSHOT_REFLECT);
            }
        } else if (CHECK_BTN_ANY(CONTROLLER1(&play->state)->press.button, BTN_A | BTN_B | BTN_R | BTN_CUP | BTN_CLEFT |
                                                                              BTN_CRIGHT | BTN_CDOWN |
                                                                              BTN_DPAD_EQUIP)) {
            s32 pad;

            this->timer = 1;
        }
    }
}

// Skijer's NEI switchhook — end the swap the SAME way the vanilla hookshot recovers from a miss:
// return the hook to Link's hand and reset it to Wait. This releases Link back to holding the hookshot
// (free to move / re-fire) instead of leaving him frozen in the aim pose after the swap.
static void ArmsHook_ReleaseAfterSwap(ArmsHook* this, Player* player) {
    sSwapTarget = NULL;
    // Belt and braces on the chain rattle. It is a CONTINUOUS flagged sfx, and the
    // flagged-audio pass in Actor_UpdateAll runs off the projection loop that walks
    // every actor — so anything that leaves it latched keeps re-emitting it with no
    // owner left to clear it. StartSwap already clears the source on the player;
    // this kills any instance that is still ringing when the swap lets go.
    // The de-flagged id is what actually reaches the sound bank (the drone is queued as
    // NA_SE_IT_HOOKSHOT_CHAIN - SFX_FLAG), and the stop compares sfxId for exact equality,
    // so the flagged constant would silently match nothing.
    AudioSfx_StopById(NA_SE_IT_HOOKSHOT_CHAIN - SFX_FLAG);
    Math_Vec3f_Copy(&this->actor.world.pos, &player->rightHandWorld.pos);
    this->unk1E0 = player->rightHandWorld.pos;
    this->timer = 0;
    ArmsHook_SetupAction(this, ArmsHook_Wait);
    ArmsHook_AttachToPlayer(this, player);
}

// Skijer's NEI switchhook — after StartSwap teleports both actors, HOLD them at their swapped
// destinations for a couple frames (a STATIC hold, not a moving ease) so each actor's own update
// resyncs its collider from world.pos — catching up to the teleport — and the aim action can't pull
// Link off the spot. Then release Link back to holding the hookshot at the new position.
void ArmsHook_SwitchSwap(ArmsHook* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    Actor* target = sSwapTarget;

    // Target vanished mid-swap (killed/despawned) — just release cleanly.
    if ((target == NULL) || (target->update == NULL)) {
        ArmsHook_ReleaseAfterSwap(this, player);
        return;
    }

    // Keep BOTH pinned at their (already-swapped) destinations. The hitboxes need no attention here:
    // ArmsHook_Update re-anchors them onto whatever position this leaves behind, every frame.
    player->actor.world.pos = sSwapTargetStart; // Link's destination
    player->actor.prevPos = sSwapTargetStart;
    player->actor.velocity.x = 0.0f;
    player->actor.velocity.y = 0.0f;
    player->actor.velocity.z = 0.0f;
    player->actor.speed = 0.0f;
    // Keep the ground flags cleared through the hold — the player's scene collision runs BEFORE this
    // actor each frame and would otherwise snap him back up to his OLD floor on downward swaps.
    player->actor.bgCheckFlags = 0;
    player->invincibilityTimer = 10;

    target->world.pos = sSwapLinkStart; // target's destination
    target->prevPos = sSwapLinkStart;
    target->home.pos = sSwapLinkStart;
    target->velocity.x = 0.0f;
    target->velocity.y = 0.0f;
    target->velocity.z = 0.0f;
    target->speed = 0.0f;
    target->bgCheckFlags = 0;

    // Keep the hook on Link so its drawn chain stays short instead of streaking across the room.
    this->actor.world.pos = sSwapTargetStart;
    this->unk1E0 = sSwapTargetStart;

    if (++sSwapTimer >= ARMSHOOK_SWAP_HOLD_FRAMES) {
        // One-shot: clear the continuous flag (see ArmsHook_StartSwap).
        Audio_PlaySfx_AtPos(&player->actor.projectedPos, NA_SE_EV_ROLL_STAND - SFX_FLAG);
        ArmsHook_ReleaseAfterSwap(this, player);
    }
}

void ArmsHook_Update(Actor* thisx, PlayState* play) {
    ArmsHook* this = (ArmsHook*)thisx;

    // Skijer's NEI switchhook — run down the post-swap player-noclip window.
    if (sSwitchNoClipTimer > 0) {
        sSwitchNoClipTimer--;
    }

    this->actionFunc(this, play);

    // Skijer's NEI switchhook — pin both swapped actors' hitboxes onto wherever they ACTUALLY are now.
    // This runs after the action func so it sees the final position for the frame, and it keeps running
    // for a while after the swap releases so wall push-outs and falls carry the colliders with them.
    if (sSwapAnchorTimer > 0) {
        sSwapAnchorTimer--;
        SwitchHook_ReanchorSwapColliders(play);
        if (sSwapAnchorTimer == 0) {
            SwitchHook_ClearSwapColliders();
        }
    }

    this->unk1EC = this->unk1E0;
}

Vec3f D_808C1C10 = { 0.0f, 0.0f, 0.0f };
Vec3f D_808C1C1C = { 0.0f, 0.0f, 900.0f };
Vec3f D_808C1C28 = { 0.0f, 500.0f, -3000.0f };
Vec3f D_808C1C34 = { 0.0f, -500.0f, -3000.0f };
Vec3f D_808C1C40 = { 0.0f, 500.0f, 0.0f };
Vec3f D_808C1C4C = { 0.0f, -500.0f, 0.0f };

void ArmsHook_Draw(Actor* thisx, PlayState* play) {
    ArmsHook* this = (ArmsHook*)thisx;
    f32 f0;
    Player* player = GET_PLAYER(play);

    if ((player->actor.draw != NULL) && (player->rightHandType == PLAYER_MODELTYPE_RH_HOOKSHOT)) {
        Vec3f sp68;
        Vec3f sp5C;
        Vec3f sp50;
        f32 sp4C;
        f32 sp48;

        OPEN_DISPS(play->state.gfxCtx);

        if ((ArmsHook_Shoot != this->actionFunc) || (this->timer <= 0)) {
            Matrix_MultVec3f(&D_808C1C10, &this->unk1E0);
            Matrix_MultVec3f(&D_808C1C28, &sp5C);
            Matrix_MultVec3f(&D_808C1C34, &sp50);
            this->weaponInfo.active = false;
        } else {
            Matrix_MultVec3f(&D_808C1C1C, &this->unk1E0);
            Matrix_MultVec3f(&D_808C1C40, &sp5C);
            Matrix_MultVec3f(&D_808C1C4C, &sp50);
        }
        func_80126440(play, &this->collider, &this->weaponInfo, &sp5C, &sp50);
        Gfx_SetupDL25_Opa(play->state.gfxCtx);
        func_80122868(play, player);

        MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
        gSPDisplayList(POLY_OPA_DISP++, object_link_child_DL_01D960);
        Matrix_Translate(this->actor.world.pos.x, this->actor.world.pos.y, this->actor.world.pos.z, MTXMODE_NEW);
        Math_Vec3f_Diff(&player->rightHandWorld.pos, &this->actor.world.pos, &sp68);
        sp48 = SQXZ(sp68);
        sp4C = sqrtf(SQXZ(sp68));
        Matrix_RotateYS(Math_Atan2S(sp68.x, sp68.z), MTXMODE_APPLY);
        Matrix_RotateXS(Math_Atan2S(-sp68.y, sp4C), MTXMODE_APPLY);
        f0 = sqrtf(SQ(sp68.y) + sp48);
        Matrix_Scale(0.015f, 0.015f, f0 * 0.01f, MTXMODE_APPLY);
        MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
        gSPDisplayList(POLY_OPA_DISP++, gHookshotChainDL);
        func_801229A0(play, player);

        CLOSE_DISPS(play->state.gfxCtx);
    }
}
