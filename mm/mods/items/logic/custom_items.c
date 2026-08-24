/**
 * custom_items.c - Unity build aggregator for custom items
 *
 * This file includes all custom item implementation files for unity build.
 * Unity builds compile multiple .c files as one translation unit for faster
 * compile times and potential optimizations.
 *
 * Add new item logic files here to include them in the build.
 */

#include "../custom_items.h"
#include "z64.h"
#include "macros.h"
#include "functions.h"
#include "variables.h"
#include "objects/gameplay_keep/gameplay_keep.h"

// Helper modules
// REAL MM player shims (poses/anims/state) — FIRST so all later files see true prototypes.
#include "../helpers/nei_player_shims.c"
#include "../helpers/item_voice.c"
#include "../helpers/movement_helper.c"
#include "../helpers/equip_helper.c"
#include "../helpers/camera_helper.c"
#include "../helpers/cutscene_helper.c"
#include "../helpers/fx_helper.c"
#include "../helpers/combat_helper.c"
#include "../helpers/grappling_helper.c"
// Shared time control + rewind + remote selection (Skijer's NEI). timestop_helper must
// precede rewind_helper, which asks it whether the world is frozen.
#include "../helpers/timestop_helper.c"
#include "../helpers/rewind_helper.c"
#include "../helpers/target_select_helper.c"
#include "../custom_items_common.c"
#include "../objects/object_custom_items.c"

// MM Animation Loader (used by item_oot_boomerang.c; the Roc's items now play native MM anims
// directly from gameplay_keep, so they no longer need this).
#include "mods/anim_translator/mm_anim_loader.c"

// Item implementations
#include "item_rocsfeather.c"
#include "item_rocscape.c"
#include "item_dekuleaf.c"
#include "item_spinner.c"
// Shared magic-rod core (RodConfig + migrated RodCommon_* helpers). Must precede
// the per-rod files so its declarations + RodProjSet definition are in scope.
#include "item_rod_common.c"
#include "item_rod_fire.c"
#include "item_rod_ice.c"
#include "item_rod_light.c"
#include "item_lantern.c"
#include "item_pending_3.c"
#include "item_hylias_grace.c"
#include "item_demise_destruction.c"
#include "item_zonai_permafrost.c"
#include "item_mitts.c"
#include "item_shovel.c"
#include "item_switchhook.c"
#include "item_whip.c"
#include "item_ballchain.c"
#include "item_bombarrows.c"
#include "twilight_upgrade.c"
#include "weapon_upgrades.c"
// The NEI Pictograph Box (picto_box.c + picto_message.cpp + snap.c/snap.h) is DELETED here:
// MM owns the Pictograph Box natively — its own item/slot (SLOT_PICTOGRAPH_BOX,
// PLAYER_IA_PICTOGRAPH_BOX), the PICTO_BOX_STATE machine and the 0xF8 "keep?" textbox in
// z_parameter.c, the I8/I5 pipeline in z_play.c and the validation engine in z_snap.c. The
// NEI files were the OoT port of exactly that, so in 2ship they were pure duplication (snap.c
// even collided at link time with z_snap.c). Everything picto in this repo now goes through
// the vanilla path + its enhancements (BetterPictoMessage, ColorPictograph, PictoBoxOnCUp).
#include "power_keg.c"   // Skijer's NEI: Power Keg (Bomb-slot wheel, form/strength gated)
#include "trade_items.c" // Skijer's NEI: MM adult trade-quest items (SLOT_TRADE_ADULT 2D-grid wheel)
#include "item_gustjar.c"
#include "item_beetle.c"
#include "item_oot_boomerang.c" // Skijer's NEI: OoT Boomerang 1:1 (human Link, PLAYER_IA_BOOMERANG)
// Ivan the Fairy partner actor (SoH EnPartner port). Defines gEnPartnerId = ACTOR_EN_PARTNER;
// item_hylias_grace.c (HGRACE_STATE_IVAN) reaches it via its `extern s16 gEnPartnerId`. Skijer's NEI
#include "../../actors/z_en_partner.inc.c"
// Ocarina song effect for the 3 NEI custom songs (Fugue/Command/Ballad rings) — ACTOR_OCEFF_NEI,
// spawned by Message_SpawnSongEffect for ocarina slots 30-32. Skijer's NEI
#include "../../actors/z_oceff_nei.inc.c"
#include "item_oot_spells.c" // Skijer's NEI: OoT spells 1:1 (Din's/Farore's/Nayru's + SW97 medallions)
// Skijer's NEI: the SHIP-VANILLA Roc's Feather (Nayru's Love cell), 1:1 from Shipwright. NOT
// item_rocsfeather.c above — that one is Skijer's own feather on page 2. Two different items.
#include "item_rocs_feather_vanilla.c"
#include "item_dominionrod.c"
#include "item_cane_of_somaria.c"
#include "item_elemental_wand.c" // Skijer's NEI: six rods behind one item action (wandMode dispatch)
#include "../helpers/box_menu.c" // Skijer's NEI: generic hold-button box selector (slate runes, ...)
#include "item_sheikah_slate.c"  // Skijer's NEI: four runes behind one page-2 cell (slateMode dispatch)
#include "item_time_gate.c"
#include "item_minish_cap.c"
#include "../helpers/minish_kaleido.c"
// Broken-Modes form data (Link/Mario/Pikachu) — consumed by the equipment kaleido's form
// selector (z_kaleido_equipment.c). Not globbed (.c), so compile it in this TU.
#include "../../broken_items/broken_items.c"
// #include "item_postman_hat.c" // DELETED: MM has Postman's Hat native (kaleido_mask); the
// custom mailbox fast-travel (+ postman_kaleido) is removed. See delete-actors task.
// item_postman_hat.c appends `#include "../helpers/postman_kaleido.c"` at
// its tail so the kaleido body ends up in the same TU.

// Bremen-Mask follower + Mask-of-Scents mushroom-spot actors DELETED — MM has both
// masks native (kaleido_mask); these were custom En_Lightbox-based actors with no MM
// equivalent. No external refs (only an unused mushroomSpotsCollected state field).
// #include "../helpers/bremen_follower_actor.c"
// #include "../helpers/mushroom_spot_actor.c"

// Transformation Masks: REMOVED - now included directly in z_player.c
// (transformation_masks.c includes mask_goron.c internally)

// ── Net ──────────────────────────────────────────────────────────────────────
// A handler that exists purely so the net can be PUT AWAY like every other item.
// Before this it had none, and ItemEquip_Update is the only thing that watches the
// other action buttons — so pressing B (or reaching for anything else) simply never
// reached the net and it stayed glued to Link's hands.
//
// Unequipping goes through ItemInput_RequestItemChange, which is what actually
// sheathes: it clears heldItemId and raises PLAYER_STATE1_START_CHANGING_HELD_ITEM,
// so Link plays the putaway instead of the item blinking out. Same call the rods
// rely on.
static ItemEquipState sNetEquipState = { 0 };
static u8 sNetActive = 0;

static void Net_OnEquip(PlayState* play, Player* p) {
    sNetActive = 1;
}

static void Net_OnUnequip(PlayState* play, Player* p) {
    sNetActive = 0;
    ItemInput_RequestItemChange(p, play); // sheathe, do not just vanish
}

void Handle_Net(Player* p, PlayState* play) {
    ItemInputState in;

    ItemInput_Update(&in, ITEM_NET, p, play);

    if (!in.wasEquipped) {
        if (sNetActive) {
            Net_OnUnequip(play, p);
        }
        sNetEquipState.isEquipped = 0;
        return;
    }

    // No cast path: catching is handled by the bottle code, not from here. All this
    // does is keep the equip state honest so the item can be taken out of hand.
    ItemEquip_Update(&sNetEquipState, &in, Net_OnEquip, Net_OnUnequip, p, play);
}

u8 Net_IsActive(void) {
    return sNetActive;
}

s32 Player_UpperAction_Net(Player* player, PlayState* play) {
    s32 result = Player_UpperAction_Sword(player, play);

    // Disarm. Player_UpperAction_Sword arms the melee quads as part of the swing;
    // clearing AT here — after it runs, before CollisionCheck resolves — keeps the
    // animation and drops the damage. The net is a catching tool, not a blade.
    for (s32 i = 0; i < ARRAY_COUNT(player->meleeWeaponQuads); i++) {
        player->meleeWeaponQuads[i].base.atFlags &= ~AT_ON;
    }

    return result;
}
