/**
 * item_sheikah_slate.c — Sheikah Slate (Skijer's NEI)
 *
 * Four runes share ONE page-2 cell (SLOT_SHEIKAH_SLATE) behind ONE ext item id
 * (EXT_ITEM_SHEIKAH_SLATE). Which rune is live is NeiSaveData.slateMode, cycled by the kaleido
 * wheel; ownership is the NeiSaveData.slateRunesOwned bitmask, one sibling pickup per rune
 * (RG_SLATE_RUNE_* — the wand idiom: gettable in any order, no levels). This file is where the
 * active rune turns into behavior.
 *
 * CONTROLS (BotW)
 * --------------
 *   C (the button holding the slate) : FIRST press draws the slate into Link's hand (equip, the
 *                                      Cane of Somaria idiom); every press after that casts the
 *                                      ACTIVE rune, with the arm extended like a hookshot shot
 *   HOLD L                           : the world pauses and the rune row opens — stick left/right
 *                                      picks, releasing L confirms, B cancels
 *
 * The slate has no PlayerItemAction: it lives in the u16 EXT item space, so it rides a C button
 * through the ext-button marker (ITEM_EXT_BUTTON + the parallel u16 store) rather than through
 * equips.buttonItems, and the press is read here in a per-frame tick — the Spiritual Stones idiom —
 * instead of through the player's held-item dispatch.
 *
 * STATUS
 * ------
 * The four rune behaviors are deliberately unimplemented — each is its own task with its own spec.
 * What IS live: the slate owns its cell, lights runes one sibling pickup at a time, shows the
 * active rune's badge on the cell/HUD icon, cycles runes in the kaleido AND in the hold-L wheel,
 * and each rune pickup has its own textbox + flame-tinted get-item model.
 *
 * Kept 1:1 with soh/mods/items/logic/item_sheikah_slate.c so a rune implemented in one game is a
 * copy-paste into the other. The ONE divergence is the ext-button accessor: MM's button array is
 * per-form, OoT's is flat.
 */

#include "global.h"
#include "mods/extended_inventory.h"         // Slate_GetRune / SLATE_RUNE_*
#include "mods/items/helpers/equip_helper.h" // equip SFX + the shared blocking checks
// box_menu.c is unity-included just before this file in custom_items.c, so its BoxMenu_*
// declarations are already in scope — it has no header (see the note at its top).

// Stasis: the first rune with real behaviour. Included here (not globbed) so it shares this
// translation unit — and so it needs no header, which would drag 2ship into a CMake regeneration.
#include "../../actors/stasis_rune.c"

extern s32 func_8083485C(Player* this, PlayState* play); // generic "held item" upper action
// Ext-button store: which u16 item a button really holds when it shows ITEM_EXT_BUTTON.
extern u16 ExtButton_GetItem(s32 form, s32 btn);

/**
 * Per-rune cast. `rune` is a SLATE_RUNE_*; returns 1 if the rune actually fired (so the caller can
 * play cast/error feedback). All four are stubs awaiting their specs.
 */
s32 Slate_CastRune(Player* player, PlayState* play, u8 rune) {
    switch (rune) {
        case SLATE_RUNE_BOMB:
            // TODO(rune): Remote Bomb — place a round rune bomb, second press detonates.
            break;
        case SLATE_RUNE_STASIS:
            return Stasis_Cast(play, player);
        case SLATE_RUNE_CRYONIS:
            // TODO(rune): Cryonis — raise a standable ice pillar from water surfaces.
            break;
        case SLATE_RUNE_MASTER_CYCLE:
            // TODO(rune): Master Cycle Zero — summon the rideable Sheikah bike.
            break;
        default:
            break;
    }
    (void)player;
    (void)play;
    return 0;
}

/**
 * Per-rune upper action, wand-shaped. Not reachable today (no item action — see header note);
 * wired so a future C-equip only needs a registry row.
 */
s32 Player_UpperAction_SheikahSlate(Player* player, PlayState* play) {
    switch (Slate_GetRune()) {
        case SLATE_RUNE_BOMB:
        case SLATE_RUNE_STASIS:
        case SLATE_RUNE_CRYONIS:
        case SLATE_RUNE_MASTER_CYCLE:
            // Per-rune held/aim behavior goes here once the casts above exist.
            break;
        default:
            break;
    }

    return func_8083485C(player, play);
}

/**
 * Runs once when the slate becomes the held item. Per-rune setup (aim reticles, target selectors,
 * helper actors) belongs here, keyed the same way as the dispatch above.
 */
void Player_InitSheikahSlateIA(PlayState* play, Player* player) {
    // No per-rune init needed while the behaviors are stubs. Kept as the named entry point so
    // adding a rune is a local change instead of a dispatch change.
    (void)play;
    (void)player;
}

// ============================================================================
// INPUT TICK — C casts, hold L opens the rune row
// ============================================================================

// How long L must be held before the row opens. Short enough to feel instant, long enough that a
// tap still reaches the vanilla Z-target.
#define SLATE_WHEEL_HOLD_FRAMES 8

static s16 sSlateHoldTimer = 0;
static u8 sSlateDrawn = 0;      // the tablet is out, in Link's hand
static s16 sSlateCastTimer = 0; // frames left of the cast pose
static s8 sSlatePrevInvinc = 0;
static u8 sSlatePrevRightHand = 0; // hand type to put back when the tablet is stowed

// How long the arm stays extended on a cast. The hookshot's own shot pose is short and snappy;
// this only has to cover the moment the rune fires.
#define SLATE_CAST_POSE_FRAMES 18

// Is the tablet currently in Link's hand? Read by the in-hand draw (object_sheikah_slate.c).
u8 Slate_IsDrawn(void) {
    return sSlateDrawn;
}

// Put it away. Idempotent, so every blocking path can call it unconditionally.
static void Slate_Stow(PlayState* play, Player* player) {
    if (!sSlateDrawn) {
        return;
    }
    sSlateDrawn = 0;
    sSlateCastTimer = 0;
    // The engine only recomputes the hand type when the item action changes, so the forced fist
    // would otherwise stay on long after the tablet is gone.
    player->rightHandType = sSlatePrevRightHand;
    ItemEquip_PlayUnequipSFX(play, player);
}

// The cast pose: the hookshot's aim/shot animation on the UPPER body only, so Link keeps walking
// and the tablet — which is drawn off the forearm→hand vector — swings out with the arm.
static void Slate_CastPose(PlayState* play, Player* player) {
    LinkAnimation_PlayOnce(play, &player->skelAnimeUpper, &gPlayerAnim_link_hook_shot_ready);
    sSlateCastTimer = SLATE_CAST_POSE_FRAMES;
}

// Every C button currently holding the slate. C items live at FORM 0 for every form — only B is
// per-form — so reading buttonItems[curForm][...] would miss them entirely.
static u16 Slate_EquippedButtonMask(void) {
    u16 mask = 0;

    if (ExtButton_GetItem(0, EQUIP_SLOT_C_LEFT) == EXT_ITEM_SHEIKAH_SLATE) {
        mask |= BTN_CLEFT;
    }
    if (ExtButton_GetItem(0, EQUIP_SLOT_C_DOWN) == EXT_ITEM_SHEIKAH_SLATE) {
        mask |= BTN_CDOWN;
    }
    if (ExtButton_GetItem(0, EQUIP_SLOT_C_RIGHT) == EXT_ITEM_SHEIKAH_SLATE) {
        mask |= BTN_CRIGHT;
    }
    return mask;
}

// The box-menu confirm: the highlighted rune becomes the active one.
static void Slate_OnWheelConfirm(s32 index) {
    Slate_SetRune((u8)index); // no-op if that rune is not owned
    if (gPlayState != NULL) {
        // HUD icon pointers are cached per button; the marker item never changes, so reload by hand.
        ExtInv_RefreshButtonIconsForItem(gPlayState, EXT_ITEM_SHEIKAH_SLATE);
    }
}

// Fills the row with ALL runes — locked ones included, drawn grayed and unselectable, so the wheel
// doubles as a reminder of what is still missing (the wand wheel's medallion previews do the same).
static s32 Slate_BuildWheel(BoxMenuEntry* out) {
    for (s32 r = 0; r < SLATE_RUNE_COUNT; r++) {
        out[r].iconPath = (const char*)Slate_RuneMiniIcon((u8)r);
        out[r].iconSize = 32;
        out[r].enabled = Slate_RuneOwned((u8)r);
    }
    return SLATE_RUNE_COUNT;
}

/**
 * Per-frame slate input, called from Player_UpdateCommon. Owns nothing else: if the slate is not
 * owned, or is on no button, this is a no-op.
 */
void Slate_TickInput(PlayState* play, Player* player) {
    BoxMenuEntry entries[SLATE_RUNE_COUNT];
    u16 btnMask;
    u16 held;

    // Stasis drives whatever it has frozen every frame, and must keep doing so even with the slate
    // stowed or the menu open — it owns another actor's update until it lets go.
    Stasis_Update(play, player);

    // Paint what a cast would grab, but only while the tablet is actually out on the Stasis rune —
    // otherwise every actor Link walks past would shimmer. Called every frame either way so the
    // highlight is taken off cleanly the moment any of that stops being true.
    Stasis_UpdateOffer(play, sSlateDrawn && (Slate_GetRune() == SLATE_RUNE_STASIS) && !BoxMenu_IsOpen());

    if (BoxMenu_IsOpen()) {
        return; // the menu owns the frame (and the game is paused anyway)
    }
    if (Nei_Save()->slateRunesOwned == 0) {
        sSlateHoldTimer = 0;
        if (sSlateDrawn) {
            sSlateDrawn = 0;
            player->rightHandType = sSlatePrevRightHand;
        }
        return; // no runes -> no slate powers at all
    }

    btnMask = Slate_EquippedButtonMask();
    held = play->state.input[0].cur.button;

    // Closed fist while the tablet is out, the way the Hookshot is gripped. The engine only
    // recomputes the hand type when the item action changes, and the slate has no item action, so
    // it is forced here every frame and put back by Slate_Stow. Applied before any early return so
    // the grip survives the frames the rune wheel owns.
    if (sSlateDrawn) {
        player->rightHandType = PLAYER_MODELTYPE_RH_CLOSED;
    }

    // ---- HOLD L: open the rune row ---------------------------------------
    // Only while the slate is IN HAND. Merely having it on a C button must leave L alone — it is
    // still Z-target for every other item, and stealing it there would break normal play.
    //
    // Read from cur.button, never press: L is Z-target and the player actor consumes its press bit
    // long before item code runs (the same trap the Dual Cane's L/R cycler documents).
    if (sSlateDrawn && (held & BTN_L)) {
        if (sSlateHoldTimer < (SLATE_WHEEL_HOLD_FRAMES + 1)) {
            sSlateHoldTimer++;
        }
        if (sSlateHoldTimer == SLATE_WHEEL_HOLD_FRAMES) {
            s32 count = Slate_BuildWheel(entries);

            BoxMenu_Open(play, entries, count, Slate_GetRune(), BTN_L, Slate_OnWheelConfirm);
        }
        return; // L is ours while it is down
    }
    sSlateHoldTimer = 0;

    // ---- C: first press draws the slate, the rest cast --------------------
    if (btnMask == 0) {
        Slate_Stow(play, player); // taken off the button while it was out
        return;
    }

    // Anything that would look wrong with a tablet in hand puts it away.
    // PLAYER_STATE1_8000000 (bit 27) is MM's native name for the flag OoT calls
    // PLAYER_STATE1_IN_WATER; the alias lives in nei_oot_compat.h, which this file does not pull in.
    if (ItemInput_IsBlocked(player, play) || (player->stateFlags1 & PLAYER_STATE1_8000000) ||
        (player->meleeWeaponState != 0)) {
        Slate_Stow(play, player);
        return;
    }
    if (ItemInput_CheckDamage(player, &sSlatePrevInvinc)) {
        Slate_Stow(play, player);
        return;
    }

    // Mid-cast: hold the pose and let the animation run out.
    if (sSlateCastTimer > 0) {
        sSlateCastTimer--;
        LinkAnimation_Update(play, &player->skelAnimeUpper);
        return;
    }

    if (play->state.input[0].press.button & btnMask) {
        if (!sSlateDrawn) {
            // Equip only — the press that draws the slate never also casts, same as the cane.
            sSlateDrawn = 1;
            sSlatePrevRightHand = player->rightHandType;
            ItemEquip_PlayEquipSFX(play, player);
            return;
        }
        Slate_CastPose(play, player);
        if (!Slate_CastRune(player, play, Slate_GetRune())) {
            // Every rune is still a stub, so this is the normal path today.
            Audio_PlaySfx(NA_SE_SY_ERROR);
        }
    }
}

// The in-hand model. Included here (not globbed) so it shares this translation unit and can read
// the equip state above — the same arrangement the cane uses for its own object file.
#include "../objects/object_sheikah_slate.c"
