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
#include "item_desire_sensor.c"
#include "item_whip.c"
#include "item_ballchain.c"
#include "item_bombarrows.c"
#include "twilight_upgrade.c"
#include "weapon_upgrades.c"
#include "snap.c"      // Skijer's NEI: Pictograph Box engine (MM-format flags for 2Ship)
#include "picto_box.c" // Skijer's NEI: Pictograph Box image pipeline + capture
#include "power_keg.c" // Skijer's NEI: Power Keg (Bomb-slot wheel, form/strength gated)
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
#include "item_oot_spells.c"    // Skijer's NEI: OoT spells 1:1 (Din's/Farore's/Nayru's + SW97 medallions)
#include "item_dominionrod.c"
#include "item_cane_of_somaria.c"
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