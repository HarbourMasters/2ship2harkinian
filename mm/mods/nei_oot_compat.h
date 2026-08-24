/*
 * nei_oot_compat.h — OoT→MM compat for the ported NEI item code (Skijer's NEI).
 *
 * The form engine's oot_player_compat.h was removed with the forms, but the ITEM
 * files use the same OoT PLAYER_STATE flags / PLAYER_IA names / M_PI / ITEM names.
 * They're remapped here to their MM equivalents (verified by meaning against MM's
 * z_player.c). Every alias is #ifndef-guarded so a real MM definition always wins.
 * Include AFTER z64.h (needs the MM PLAYER_STATE, PLAYER_IA and ITEM symbols).
 */
#ifndef NEI_OOT_COMPAT_H
#define NEI_OOT_COMPAT_H

#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ── OoT PLAYER_STATE1 flags → MM (same-meaning bit) ─────────────────────────── */
#ifndef PLAYER_STATE1_ITEM_IN_HAND
#define PLAYER_STATE1_ITEM_IN_HAND PLAYER_STATE1_8
#endif
#ifndef PLAYER_STATE1_INPUT_DISABLED
#define PLAYER_STATE1_INPUT_DISABLED PLAYER_STATE1_20
#endif
#ifndef PLAYER_STATE1_CLIMBING_LADDER
#define PLAYER_STATE1_CLIMBING_LADDER PLAYER_STATE1_200000
#endif
#ifndef PLAYER_STATE1_SHIELDING
#define PLAYER_STATE1_SHIELDING PLAYER_STATE1_400000
#endif
#ifndef PLAYER_STATE1_USING_BOOMERANG
#define PLAYER_STATE1_USING_BOOMERANG PLAYER_STATE1_USING_ZORA_BOOMERANG
#endif
#ifndef PLAYER_STATE1_BOOMERANG_THROWN
#define PLAYER_STATE1_BOOMERANG_THROWN PLAYER_STATE1_ZORA_BOOMERANG_THROWN
#endif
#ifndef PLAYER_STATE1_IN_WATER
#define PLAYER_STATE1_IN_WATER PLAYER_STATE1_8000000
#endif
/* OoT-only states with no MM analog → 0 (`& flag` reads false, `|= flag` is a no-op). */
#ifndef PLAYER_STATE1_READY_TO_FIRE
#define PLAYER_STATE1_READY_TO_FIRE 0
#endif
#ifndef PLAYER_STATE1_HOOKSHOT_FALLING
#define PLAYER_STATE1_HOOKSHOT_FALLING 0
#endif
#ifndef PLAYER_STATE1_START_CHANGING_HELD_ITEM
#define PLAYER_STATE1_START_CHANGING_HELD_ITEM 0
#endif
#ifndef PLAYER_STATE1_FIRST_PERSON
#define PLAYER_STATE1_FIRST_PERSON PLAYER_STATE1_100000 /* MM bit 20 = first-person / C-up aiming */
#endif
/* OoT air-state flags with no clean MM analog. Here they appear only in clear-ops
   (clawshot bullet-time reset) plus a degraded pendant float → map to 0 (no-op).
   TODO: wire MM-native air-state detection when the air mechanics are revisited. */
#ifndef PLAYER_STATE1_JUMPING
#define PLAYER_STATE1_JUMPING 0
#endif
#ifndef PLAYER_STATE1_FREEFALL
#define PLAYER_STATE1_FREEFALL 0
#endif
#ifndef PLAYER_STATE3_MIDAIR
#define PLAYER_STATE3_MIDAIR 0
#endif
#ifndef PLAYER_STATE2_DISABLE_ROTATION_Z_TARGET
#define PLAYER_STATE2_DISABLE_ROTATION_Z_TARGET PLAYER_STATE2_20
#endif

/* ── OoT PLAYER_IA (item action) names → MM ──────────────────────────────────── */
#ifndef PLAYER_IA_SWORD_MASTER
#define PLAYER_IA_SWORD_MASTER PLAYER_IA_SWORD_GILDED
#endif
#ifndef PLAYER_IA_SWORD_BIGGORON
#define PLAYER_IA_SWORD_BIGGORON PLAYER_IA_SWORD_TWO_HANDED
#endif
#ifndef PLAYER_IA_FISHING_POLE
#define PLAYER_IA_FISHING_POLE PLAYER_IA_FISHING_ROD
#endif
/* NEI: PLAYER_IA_SLINGSHOT is now a REAL item action (0x54, z64player.h — always defined before
   this header, so this #ifndef never fires; kept as a guarded mirror like PLAYER_IA_HAMMER).
   It was previously aliased to PLAYER_IA_DEKU_NUT. */
#ifndef PLAYER_IA_SLINGSHOT
#define PLAYER_IA_SLINGSHOT 0x54
#endif
#ifndef PLAYER_IA_BOTTLE
#define PLAYER_IA_BOTTLE PLAYER_IA_BOTTLE_EMPTY
#endif
#ifndef PLAYER_IA_LONGSHOT
#define PLAYER_IA_LONGSHOT PLAYER_IA_HOOKSHOT
#endif
/* OoT-only IAs (sword cutscene, OoT bow variants) → NONE (no MM analog). */
#ifndef PLAYER_IA_SWORD_CS
#define PLAYER_IA_SWORD_CS PLAYER_IA_NONE
#endif
// NEI: the six magic-spell IAs are now REAL contiguous values (0x55..0x5A, z64player.h — always
// defined before this header, so these #ifndefs never fire; kept as guarded mirrors like
// PLAYER_IA_HAMMER). They were previously aliased to PLAYER_IA_NONE (spells were inert).
#ifndef PLAYER_IA_MAGIC_SPELL_15
#define PLAYER_IA_MAGIC_SPELL_15 0x55
#endif
#ifndef PLAYER_IA_MAGIC_SPELL_16
#define PLAYER_IA_MAGIC_SPELL_16 0x56
#endif
#ifndef PLAYER_IA_MAGIC_SPELL_17
#define PLAYER_IA_MAGIC_SPELL_17 0x57
#endif
#ifndef PLAYER_IA_FARORES_WIND
#define PLAYER_IA_FARORES_WIND 0x58
#endif
#ifndef PLAYER_IA_NAYRUS_LOVE
#define PLAYER_IA_NAYRUS_LOVE 0x59
#endif
#ifndef PLAYER_IA_DINS_FIRE
#define PLAYER_IA_DINS_FIRE 0x5A
#endif
#ifndef PLAYER_IA_BOW_0C
#define PLAYER_IA_BOW_0C PLAYER_IA_NONE
#endif
#ifndef PLAYER_IA_BOW_0D
#define PLAYER_IA_BOW_0D PLAYER_IA_NONE
#endif
#ifndef PLAYER_IA_BOW_0E
#define PLAYER_IA_BOW_0E PLAYER_IA_NONE
#endif

/* ── OoT ITEM names → MM ─────────────────────────────────────────────────────── */
#ifndef ITEM_FISHING_POLE
#define ITEM_FISHING_POLE ITEM_FISHING_ROD
#endif
#ifndef ITEM_LAST_USED
#define ITEM_LAST_USED ITEM_FE /* OoT "repeat last item" — dead in MM (placeholder value) */
#endif

/* ── OoT PLAYER_BODYPART names → MM (abbreviated → full) ─────────────────────── */
#ifndef PLAYER_BODYPART_L_HAND
#define PLAYER_BODYPART_L_HAND PLAYER_BODYPART_LEFT_HAND
#endif
#ifndef PLAYER_BODYPART_R_HAND
#define PLAYER_BODYPART_R_HAND PLAYER_BODYPART_RIGHT_HAND
#endif
#ifndef PLAYER_BODYPART_L_FOREARM
#define PLAYER_BODYPART_L_FOREARM PLAYER_BODYPART_LEFT_FOREARM
#endif
#ifndef PLAYER_BODYPART_R_FOREARM
#define PLAYER_BODYPART_R_FOREARM PLAYER_BODYPART_RIGHT_FOREARM
#endif
#ifndef PLAYER_BODYPART_L_SHOULDER
#define PLAYER_BODYPART_L_SHOULDER PLAYER_BODYPART_LEFT_SHOULDER
#endif
#ifndef PLAYER_BODYPART_R_SHOULDER
#define PLAYER_BODYPART_R_SHOULDER PLAYER_BODYPART_RIGHT_SHOULDER
#endif
#ifndef PLAYER_BODYPART_L_THIGH
#define PLAYER_BODYPART_L_THIGH PLAYER_BODYPART_LEFT_THIGH
#endif
#ifndef PLAYER_BODYPART_R_THIGH
#define PLAYER_BODYPART_R_THIGH PLAYER_BODYPART_RIGHT_THIGH
#endif
#ifndef PLAYER_BODYPART_L_SHIN
#define PLAYER_BODYPART_L_SHIN PLAYER_BODYPART_LEFT_SHIN
#endif
#ifndef PLAYER_BODYPART_R_SHIN
#define PLAYER_BODYPART_R_SHIN PLAYER_BODYPART_RIGHT_SHIN
#endif
#ifndef PLAYER_BODYPART_L_FOOT
#define PLAYER_BODYPART_L_FOOT PLAYER_BODYPART_LEFT_FOOT
#endif
#ifndef PLAYER_BODYPART_R_FOOT
#define PLAYER_BODYPART_R_FOOT PLAYER_BODYPART_RIGHT_FOOT
#endif

/* ── OoT misc constants → MM ─────────────────────────────────────────────────── */
#ifndef SUBCAM_FREE
#define SUBCAM_FREE CAM_ID_NONE /* OoT "no active subcamera" sentinel → MM CAM_ID_NONE (-1) */
#endif
#ifndef ENTR_LOST_WOODS_SOUTH_EXIT
#define ENTR_LOST_WOODS_SOUTH_EXIT ENTR_SCENE_LOST_WOODS /* FleetShipCombo-only warp; dead in standalone MM */
#endif

/* ── OoT-only actor IDs with no MM equivalent → never-matching sentinels ──────────
 * These NEI items reference OoT actors absent in MM. Sentinels (unique, high, never a
 * real MM actor id) let the code compile; the `actor->id == ACTOR_*` interactions stay
 * INERT in MM until adapted to real MM actors. Per-item list + reward-preservation
 * reminders: mm/mods/items/OOT_ADAPT_TODO.md. */
#ifndef ACTOR_BG_HAKA_HUTA
#define ACTOR_BG_HAKA_HUTA 0x7F01
#endif
#ifndef ACTOR_BG_HEAVY_BLOCK
#define ACTOR_BG_HEAVY_BLOCK 0x7F02
#endif
#ifndef ACTOR_BG_MIZU_SHUTTER
#define ACTOR_BG_MIZU_SHUTTER 0x7F03
#endif
#ifndef ACTOR_BG_PO_SYOKUDAI
#define ACTOR_BG_PO_SYOKUDAI 0x7F04
#endif
#ifndef ACTOR_BG_SPOT18_FUTA
#define ACTOR_BG_SPOT18_FUTA 0x7F05
#endif
#ifndef ACTOR_BG_SPOT18_SHUTTER
#define ACTOR_BG_SPOT18_SHUTTER 0x7F06
#endif
#ifndef ACTOR_DOOR_GERUDO
#define ACTOR_DOOR_GERUDO 0x7F07
#endif
#ifndef ACTOR_DOOR_KILLER
#define ACTOR_DOOR_KILLER 0x7F08
#endif
#ifndef ACTOR_DOOR_TOKI
#define ACTOR_DOOR_TOKI 0x7F09
#endif
#ifndef ACTOR_EN_ANUBICE
#define ACTOR_EN_ANUBICE 0x7F0A
#endif
#ifndef ACTOR_EN_ANUBICE_FIRE
#define ACTOR_EN_ANUBICE_FIRE 0x7F0B
#endif
#ifndef ACTOR_EN_BW
#define ACTOR_EN_BW 0x7F0C
#endif
#ifndef ACTOR_EN_DH
#define ACTOR_EN_DH 0x7F0D
#endif
#ifndef ACTOR_EN_DHA
#define ACTOR_EN_DHA 0x7F0E
#endif
#ifndef ACTOR_EN_GELDB
#define ACTOR_EN_GELDB 0x7F0F
#endif
#ifndef ACTOR_EN_G_SWITCH
#define ACTOR_EN_G_SWITCH 0x7F10
#endif
#ifndef ACTOR_EN_HINTNUTS
#define ACTOR_EN_HINTNUTS 0x7F11
#endif
#ifndef ACTOR_EN_ICE_HONO
#define ACTOR_EN_ICE_HONO 0x7F12
#endif
#ifndef ACTOR_EN_KAKASI2
#define ACTOR_EN_KAKASI2 0x7F13
#endif
#ifndef ACTOR_EN_KAKASI3
#define ACTOR_EN_KAKASI3 0x7F14
#endif
#ifndef ACTOR_EN_KO
#define ACTOR_EN_KO 0x7F15
#endif
#ifndef ACTOR_EN_LIGHTBOX
#define ACTOR_EN_LIGHTBOX 0x7F16
#endif
#ifndef ACTOR_EN_MD
#define ACTOR_EN_MD 0x7F17
#endif
#ifndef ACTOR_EN_PO_DESERT
#define ACTOR_EN_PO_DESERT 0x7F18
#endif
#ifndef ACTOR_EN_PO_FIELD
#define ACTOR_EN_PO_FIELD 0x7F19
#endif
#ifndef ACTOR_EN_PO_RELAY
#define ACTOR_EN_PO_RELAY 0x7F1A
#endif
#ifndef ACTOR_EN_REEBA
#define ACTOR_EN_REEBA 0x7F1B
#endif
#ifndef ACTOR_EN_RU1
#define ACTOR_EN_RU1 0x7F1C
#endif
#ifndef ACTOR_EN_RU2
#define ACTOR_EN_RU2 0x7F1D
#endif
#ifndef ACTOR_EN_SA
#define ACTOR_EN_SA 0x7F1E
#endif
#ifndef ACTOR_EN_SKJ
#define ACTOR_EN_SKJ 0x7F1F
#endif
#ifndef ACTOR_EN_ZF
#define ACTOR_EN_ZF 0x7F20
#endif
#ifndef ACTOR_OBJ_DEKUJR
#define ACTOR_OBJ_DEKUJR 0x7F21
#endif
/* OoT switch-hook target flag — no MM actor sets it, so 0 (inert) until switchhook
 * is adapted to mark MM hookable actors. */
#ifndef ACTOR_FLAG_SWITCHHOOKABLE
#define ACTOR_FLAG_SWITCHHOOKABLE 0
#endif

/* ── OoT SFX names → MM (equivalent, else NA_SE_NONE silence placeholder) ─────────
 * Enemy/event-specific OoT sfx (Anubis, Phantom Ganon, Ganon, Triforce) have no MM
 * source → NA_SE_NONE (silent). Refine to real MM sfx during item polish. */
#ifndef NA_SE_VO_LI_AUTO_JUMP_KID
#define NA_SE_VO_LI_AUTO_JUMP_KID NA_SE_VO_LI_AUTO_JUMP
#endif
#ifndef NA_SE_PL_LAND
#define NA_SE_PL_LAND NA_SE_NONE
#endif
#ifndef NA_SE_EN_ANUBIS_FIRE
#define NA_SE_EN_ANUBIS_FIRE NA_SE_NONE
#endif
#ifndef NA_SE_EN_ANUBIS_FIREBOMB
#define NA_SE_EN_ANUBIS_FIREBOMB NA_SE_NONE
#endif
#ifndef NA_SE_EN_FANTOM_HIT_THUNDER
#define NA_SE_EN_FANTOM_HIT_THUNDER NA_SE_NONE
#endif
#ifndef NA_SE_EN_FANTOM_SPARK
#define NA_SE_EN_FANTOM_SPARK NA_SE_NONE
#endif
#ifndef NA_SE_EV_TRIFORCE_FLASH
#define NA_SE_EV_TRIFORCE_FLASH NA_SE_NONE
#endif
#ifndef NA_SE_EN_GANON_AT_RETURN
#define NA_SE_EN_GANON_AT_RETURN NA_SE_NONE
#endif
#ifndef NA_SE_EN_GANON_LAUGH
#define NA_SE_EN_GANON_LAUGH NA_SE_NONE
#endif

/* ── OoT PLAYER_LIMB abbreviations → MM full names ────────────────────────────── */
#ifndef PLAYER_LIMB_L_SHOULDER
#define PLAYER_LIMB_L_SHOULDER PLAYER_LIMB_LEFT_SHOULDER
#endif

/* OoT sidehop/backflip state — no named MM analog → 0 (hop-detection inert; affects
 * champion flurry-rush + rod hop checks). TODO: wire the MM hop flag when revisited. */
#ifndef PLAYER_STATE2_HOPPING
#define PLAYER_STATE2_HOPPING 0
#endif
#ifndef PLAYER_LIMB_L_FOREARM
#define PLAYER_LIMB_L_FOREARM PLAYER_LIMB_LEFT_FOREARM
#endif
#ifndef PLAYER_LIMB_L_HAND
#define PLAYER_LIMB_L_HAND PLAYER_LIMB_LEFT_HAND
#endif
#ifndef PLAYER_LIMB_R_SHOULDER
#define PLAYER_LIMB_R_SHOULDER PLAYER_LIMB_RIGHT_SHOULDER
#endif
#ifndef PLAYER_LIMB_R_FOREARM
#define PLAYER_LIMB_R_FOREARM PLAYER_LIMB_RIGHT_FOREARM
#endif
#ifndef PLAYER_LIMB_R_HAND
#define PLAYER_LIMB_R_HAND PLAYER_LIMB_RIGHT_HAND
#endif

/* OoT actor category → MM (plural spelling) */
#ifndef ACTORCAT_EXPLOSIVE
#define ACTORCAT_EXPLOSIVE ACTORCAT_EXPLOSIVES
#endif

/* SoH randomizer macro — 2ship's rando is C++ (mm/2s2h/Rando), unreachable from these
 * C item TUs. Treat as "not rando" until an item is wired to it. */
#ifndef IS_RANDO
#define IS_RANDO 0
#endif

/* OoT "turn-around" camera setting → MM CAM_SET_NONE. NOTE: the Camera_SetCameraData
 * CALLS still pass OoT's extra arg — those call sites are fixed/stubbed per-file. */
#ifndef CAM_SET_TURN_AROUND
#define CAM_SET_TURN_AROUND CAM_SET_NONE
#endif

/* ── OoT-only constants used by items → sentinels/placeholders ─────────────────────
 * These have no MM equivalent; the referencing code stays inert where OoT-specific.
 * See mm/mods/items/OOT_ADAPT_TODO.md. */
/* OoT child-Link sword voice → MM's (single-age) adult sword voice, NOT silence. MM has no child
 * Link, so every _KID voice should fall back to the normal one (same pattern as
 * NA_SE_VO_LI_AUTO_JUMP_KID above). Was NA_SE_NONE, which made the beetle throw voice silent. */
#ifndef NA_SE_VO_LI_SWORD_N_KID
#define NA_SE_VO_LI_SWORD_N_KID NA_SE_VO_LI_SWORD_N
#endif
// NEI: real Megaton Hammer IA (0x7C) — matches z64player.h (which always wins as it's included
// first). Was PLAYER_IA_NONE, which broke the IK-Axe / hammer detection.
#ifndef PLAYER_IA_HAMMER
#define PLAYER_IA_HAMMER 0x7C
#endif
#ifndef QUEST_MEDALLION_FOREST
#define QUEST_MEDALLION_FOREST 0
#endif
#ifndef QUEST_MEDALLION_FIRE
#define QUEST_MEDALLION_FIRE 0
#endif
#ifndef QUEST_MEDALLION_WATER
#define QUEST_MEDALLION_WATER 0
#endif
#ifndef QUEST_MEDALLION_SHADOW
#define QUEST_MEDALLION_SHADOW 0
#endif
#ifndef QUEST_MEDALLION_SPIRIT
#define QUEST_MEDALLION_SPIRIT 0
#endif
#ifndef QUEST_MEDALLION_LIGHT
#define QUEST_MEDALLION_LIGHT 0
#endif
#ifndef ROCK_LARGE
#define ROCK_LARGE 0
#endif
#ifndef ROCK_SMALL
#define ROCK_SMALL 1
#endif
#ifndef VB_ROCK_DROP_ITEM
#define VB_ROCK_DROP_ITEM 0
#endif
#ifndef SLOT_LENS
#define SLOT_LENS 0
#endif
/* OoT adult trade-quest items — no MM equivalents → ITEM_NONE (trade table inert). */
#ifndef ITEM_POCKET_EGG
#define ITEM_POCKET_EGG ITEM_NONE
#endif
#ifndef ITEM_POCKET_CUCCO
#define ITEM_POCKET_CUCCO ITEM_NONE
#endif
#ifndef ITEM_COJIRO
#define ITEM_COJIRO ITEM_NONE
#endif
#ifndef ITEM_ODD_MUSHROOM
#define ITEM_ODD_MUSHROOM ITEM_NONE
#endif
#ifndef ITEM_ODD_POTION
#define ITEM_ODD_POTION ITEM_NONE
#endif
#ifndef ITEM_SAW
#define ITEM_SAW ITEM_NONE
#endif
#ifndef ITEM_SWORD_BROKEN
#define ITEM_SWORD_BROKEN ITEM_NONE
#endif
#ifndef ITEM_PRESCRIPTION
#define ITEM_PRESCRIPTION ITEM_NONE
#endif
#ifndef ITEM_FROG
#define ITEM_FROG ITEM_NONE
#endif
#ifndef ITEM_EYEDROPS
#define ITEM_EYEDROPS ITEM_NONE
#endif
#ifndef ITEM_CLAIM_CHECK
#define ITEM_CLAIM_CHECK ITEM_NONE
#endif
/* OoT *child* trade-quest items (the non-mask half of OoT's SLOT_TRADE_CHILD). Same story as the
 * adult chain above: no MM equivalent, no room left in the u8 id space → ITEM_NONE. They are visible
 * in MM through the unified wheel's index cursor + TradeAdult_IconPath(), not through an item id. */
#ifndef ITEM_WEIRD_EGG
#define ITEM_WEIRD_EGG ITEM_NONE
#endif
#ifndef ITEM_CHICKEN
#define ITEM_CHICKEN ITEM_NONE
#endif
#ifndef ITEM_LETTER_ZELDA
#define ITEM_LETTER_ZELDA ITEM_NONE
#endif
#ifndef EQUIP_TYPE_BOOTS
/* MM natively has SWORD=0/SHIELD=1/TUNIC=2; BOOTS is our accessory tag. 3 keeps it
 * unique (0 collided with EQUIP_TYPE_SWORD → duplicate switch cases) + matches the
 * sExtEquipIconPaths[4][3] indexing (sword/shield/tunic/boots = 0/1/2/3). */
#define EQUIP_TYPE_BOOTS 3
#endif
/* OoT Phantom-Ganon death sfx → silent (no MM source) */
#ifndef NA_SE_EN_FANTOM_DEAD
#define NA_SE_EN_FANTOM_DEAD NA_SE_NONE
#endif
/* OoT deku-nut arrow ammo → MM EnArrow deku-nut type */
#ifndef ARROW_NUT
#define ARROW_NUT ARROW_TYPE_DEKU_NUT
#endif
// NEI: real OoT Boomerang IA (0x62) — matches z64player.h (which always wins as it's included
// first). Was aliased to PLAYER_IA_ZORA_BOOMERANG, which only threw as Zora form.
#ifndef PLAYER_IA_BOOMERANG
#define PLAYER_IA_BOOMERANG 0x62
#endif
/* OoT per-limb joint-buffer count → MM max limb count (sizes a stack Vec3s[] array) */
#ifndef PLAYER_LIMB_BUF_COUNT
#define PLAYER_LIMB_BUF_COUNT PLAYER_LIMB_MAX
#endif
/* OoT Master-Sword pedestal-plant anim (pendant of memories pose) has no MM equivalent →
 * fall back to the normal idle anim so it compiles. Real pose is a TODO. */
#ifndef gPlayerAnim_002840
#define gPlayerAnim_002840 gPlayerAnim_link_normal_wait
#endif
#ifndef ARROW_NORMAL
#define ARROW_NORMAL 0
#endif
/* OoT slingshot seed pellet → MM's real seed EnArrow type (was 1 = ARROW_TYPE_NORMAL_HORSE,
   which made cucco-egg/foursword seed shots fire a horse arrow). NEI slingshot pass. */
#ifndef ARROW_SEED
#define ARROW_SEED 6 /* ARROW_TYPE_SLINGSHOT */
#endif
#ifndef BOMB_BODY
#define BOMB_BODY 0
#endif
#ifndef ROOM_BEHAVIOR_TYPE1_5
#define ROOM_BEHAVIOR_TYPE1_5 0
#endif
#ifndef ENTR_RETURN_GROTTO
#define ENTR_RETURN_GROTTO 0
#endif
#ifndef ITEMGETINF_1C
#define ITEMGETINF_1C 0
#endif
#ifndef COLLECTFLAG_GRAVEDIGGING_HEART_PIECE
#define COLLECTFLAG_GRAVEDIGGING_HEART_PIECE 0
#endif
#ifndef PLAYER_MODELTYPE_RH_BOW_SLINGSHOT
#define PLAYER_MODELTYPE_RH_BOW_SLINGSHOT 0
#endif
/* OoT camera main-id + status. MM has no CAM_STAT enum → OoT sentinel values. */
#ifndef MAIN_CAM
#define MAIN_CAM CAM_ID_MAIN
#endif
#ifndef CAM_STAT_ACTIVE
#define CAM_STAT_ACTIVE 7
#endif
#ifndef CAM_STAT_WAIT
#define CAM_STAT_WAIT 1
#endif
/* ─────────────────────────────────────────────────────────────────────────
 * OoT→MM function renames (link-time). Verified 1:1: identical args, MM just
 * spells them differently. These only rewrite the OoT spellings — never MM's
 * own names — so MM engine code in the same TU is unaffected.
 * ───────────────────────────────────────────────────────────────────────── */
#ifndef Effect_Delete
#define Effect_Delete Effect_Destroy
#endif
#ifndef Audio_StopSfxById
#define Audio_StopSfxById AudioSfx_StopById
#endif
#ifndef Actor_MoveXZGravity
#define Actor_MoveXZGravity Actor_MoveWithGravity
#endif
#ifndef Quake_Add
#define Quake_Add Quake_Request
#endif
#ifndef LinkAnimation_PlayOnce
#define LinkAnimation_PlayOnce PlayerAnimation_PlayOnce
#endif
#ifndef LinkAnimation_Change
#define LinkAnimation_Change PlayerAnimation_Change
#endif
#ifndef LinkAnimation_Update
#define LinkAnimation_Update PlayerAnimation_Update
#endif
#ifndef LinkAnimation_PlayLoop
#define LinkAnimation_PlayLoop PlayerAnimation_PlayLoop
#endif
#ifndef Gfx_SetupDL_25Xlu
#define Gfx_SetupDL_25Xlu Gfx_SetupDL25_Xlu
#endif
#ifndef Gfx_SetupDL_25Opa
#define Gfx_SetupDL_25Opa Gfx_SetupDL25_Opa
#endif
#ifndef Gfx_SetupDL_27Xlu
#define Gfx_SetupDL_27Xlu Gfx_SetupDL27_Xlu
#endif
#ifndef Gfx_SetupDL_39Overlay
#define Gfx_SetupDL_39Overlay Gfx_SetupDL39_Overlay
#endif
/* OoT Matrix_NewMtx(gfxCtx, file, line) (alloc+convert) → MM Matrix_Finalize(gfxCtx).
 * 3-arg function-macro drops the debug file/line args; also the uppercase macro form. */
#ifndef Matrix_NewMtx
#define Matrix_NewMtx(gfxCtx, file, line) Matrix_Finalize(gfxCtx)
#endif
#ifndef MATRIX_NEWMTX
#define MATRIX_NEWMTX(gfxCtx) Matrix_Finalize(gfxCtx)
#endif
/* OoT quake setters → MM equivalents (identical arg lists). */
#ifndef Quake_SetQuakeValues
#define Quake_SetQuakeValues Quake_SetPerturbations
#endif
#ifndef Quake_SetCountdown
#define Quake_SetCountdown Quake_SetDuration
#endif
/* ── OoT macros used without a def in scope (became phantom link symbols) → MM forms ── */
#ifndef SEGMENTED_TO_VIRTUAL
#define SEGMENTED_TO_VIRTUAL(x) Lib_SegmentedToVirtual((void*)(x))
#endif
#ifndef SEG_ADDR
#define SEG_ADDR(seg, offset) ((void*)(uintptr_t)(((seg) << 24) | ((offset)&0x00FFFFFF)))
#endif
#ifndef MATRIX_TOMTX
#define MATRIX_TOMTX(mtx) Matrix_ToMtx(mtx)
#endif
#ifndef ZELDA_ARENA_MALLOC_DEBUG
#define ZELDA_ARENA_MALLOC_DEBUG(size) ZeldaArena_Malloc(size)
#endif
#ifndef ZELDA_ARENA_FREE_DEBUG
#define ZELDA_ARENA_FREE_DEBUG(ptr) ZeldaArena_Free(ptr)
#endif
#ifndef LOG_STRING
#define LOG_STRING(...) ((void)0)
#endif
/* MM's gold-skulltula system differs; the (triaged) minish-cap pod-soil check reports
 * "never collected" (0). TODO: wire to MM GS flags if minish is revived. */
#ifndef GET_GS_FLAGS
#define GET_GS_FLAGS(group) 0
#endif
/* OoT generic dust/soft-sprite spawn (z_effect_soft_sprite_old_init.c) → MM's EffectSsDust_Spawn.
 * OoT's func_8002836C is literally `EffectSsDust_Spawn(play, 0, ...pos/vel/accel/prim/env/scale/
 * scaleStep/life..., 0)`, and MM keeps EffectSsDust_Spawn with the SAME signature (drawFlags first,
 * updateMode last). This was a no-op stub in nei_link_stubs.cpp, so every dust/wind effect built on
 * it was INVISIBLE (Deku Leaf wind, Gust Jar wind, Ivan's sparkle). Function-like macro so all C call
 * sites (9 args) resolve to the real effect. drawFlags 0 = DUST_DRAWFLAG0, updateMode 0 =
 * DUST_UPDATE_NORMAL (same literals OoT passed). Skijer's NEI */
#ifndef func_8002836C
#define func_8002836C(play, pos, vel, accel, prim, env, scale, step, life) \
    EffectSsDust_Spawn((play), 0, (pos), (vel), (accel), (prim), (env), (scale), (step), (life), 0)
#endif
/* OoT EffectSsKiraKira sparkles → MM has no KiraKira effect; forward to GSpk (the green/white spark
 * star). These were no-op STUBS in nei_link_stubs.cpp, so every sparkle built on them was invisible:
 * the Fire rod projectile sparks and the ENTIRE Light rod identity (trail + light-beam jump attack +
 * hit sparkles — the light rod is 100% KiraKira-based), plus Desire Sensor / Hylia's Grace / Cane of
 * Somaria / Dragon Scale. KiraKira's scale convention is ~10× MM GSpk's (OoT passes 400..5000, native
 * MM GSpk ~100), so divide the scale by 10. Function-like macros → all C call sites resolve to the
 * real effect (GET_PLAYER + EffectSsGSpk_SpawnAccel are in scope at every call site in this TU).
 * SpawnSmall has no scale/step args → small fixed values. Skijer's NEI */
#ifndef EffectSsKiraKira_SpawnDispersed
#define EffectSsKiraKira_SpawnDispersed(play, pos, vel, accel, prim, env, scale, step)              \
    EffectSsGSpk_SpawnAccel((play), &GET_PLAYER(play)->actor, (pos), (vel), (accel), (prim), (env), \
                            (s16)((scale) / 10), (step))
#endif
#ifndef EffectSsKiraKira_SpawnFocused
#define EffectSsKiraKira_SpawnFocused(play, pos, vel, accel, prim, env, scale, step)                \
    EffectSsGSpk_SpawnAccel((play), &GET_PLAYER(play)->actor, (pos), (vel), (accel), (prim), (env), \
                            (s16)((scale) / 10), (step))
#endif
#ifndef EffectSsKiraKira_SpawnSmall
#define EffectSsKiraKira_SpawnSmall(play, pos, vel, accel, prim, env) \
    EffectSsGSpk_SpawnAccel((play), &GET_PLAYER(play)->actor, (pos), (vel), (accel), (prim), (env), 60, 5)
#endif
/* OoT actor-sfx (flagged) helpers → MM equivalents. */
#ifndef func_8002F8F0
#define func_8002F8F0 Actor_PlaySfx_Flagged
#endif
#ifndef func_8002F974
#define func_8002F974 Actor_PlaySfx_Flagged2
#endif
/* OoT Lens-of-Truth disable → MM deactivate. */
#ifndef Actor_DisableLens
#define Actor_DisableLens Actor_DeactivateLens
#endif
/* OoT individual float-radian matrix rotates → MM *F forms (same (f32, MatrixMode) sig). */
#ifndef Matrix_RotateX
#define Matrix_RotateX Matrix_RotateXF
#endif
#ifndef Matrix_RotateY
#define Matrix_RotateY Matrix_RotateYF
#endif
#ifndef Matrix_RotateZ
#define Matrix_RotateZ Matrix_RotateZF
#endif
/* OoT EnIce flying-shard spawn (Vec3f pos) → MM EffectSsEnIce_SpawnFlying (same args). */
#ifndef EffectSsEnIce_SpawnFlyingVec3f
#define EffectSsEnIce_SpawnFlyingVec3f EffectSsEnIce_SpawnFlying
#endif
/* NEI player shims (real MM impls in mods/items/helpers/nei_player_shims.c, same TU).
 * Prototyped here because extended_inventory.c (z_player.c line ~75) calls them BEFORE the
 * shim definitions are included via custom_items.c (line ~76). */
void Player_UnsetMask(struct PlayState* play);
/* OoT enemy sfx with no MM source → silent */
#ifndef NA_SE_EN_DODO_M_GND
#define NA_SE_EN_DODO_M_GND NA_SE_NONE
#endif
#ifndef NA_SE_EN_VALVAISA_FIRE
#define NA_SE_EN_VALVAISA_FIRE NA_SE_NONE
#endif
#ifndef NA_SE_EN_FANTOM_FIRE
#define NA_SE_EN_FANTOM_FIRE NA_SE_NONE
#endif
/* OoT Link boom/bow anims not provided by the MM anim loader → reuse the loaded
 * throw-wait anim as a placeholder (visual only; polish later). */
#ifndef gPlayerAnim_link_boom_throwR
#define gPlayerAnim_link_boom_throwR gPlayerAnim_link_boom_throw_waitR
#endif
#ifndef gPlayerAnim_link_boom_catch
#define gPlayerAnim_link_boom_catch gPlayerAnim_link_boom_throw_waitR
#endif
#ifndef gPlayerAnim_link_bow_bow_shoot
#define gPlayerAnim_link_bow_bow_shoot gPlayerAnim_link_boom_throw_waitR
#endif
/* OoT type name → MM (whole-token replace; leaves tatlHoverActorCategory untouched) */
#ifndef ActorCategory
#define ActorCategory ActorType
#endif
/* OoT Shadow-Temple heavy switch (somaria cube target) — no MM actor → sentinel. */
#ifndef ACTOR_BG_BDAN_SWITCH
#define ACTOR_BG_BDAN_SWITCH 0x7F22
#endif
/* OoT Bg_Bdan_Switch type (from the removed OoT header) → sentinel (somaria inert). */
#ifndef YELLOW_HEAVY
#define YELLOW_HEAVY 1
#endif
/* OoT PlayState.transiActorCtx → MM PlayState.transitionActors (same list/numActors). */
#ifndef transiActorCtx
#define transiActorCtx transitionActors
#endif
/* OoT Actor min-Y-velocity field → MM terminalVelocity */
#ifndef minVelocityY
#define minVelocityY terminalVelocity
#endif
/* OoT overworld scenes/entrances (minish-cap warp table) — no MM equivalents → 0 (inert). */
#ifndef SCENE_KOKIRI_FOREST
#define SCENE_KOKIRI_FOREST 0
#endif
#ifndef SCENE_LAKE_HYLIA
#define SCENE_LAKE_HYLIA 0
#endif
#ifndef SCENE_DEATH_MOUNTAIN_TRAIL
#define SCENE_DEATH_MOUNTAIN_TRAIL 0
#endif
#ifndef SCENE_DEATH_MOUNTAIN_CRATER
#define SCENE_DEATH_MOUNTAIN_CRATER 0
#endif
#ifndef SCENE_DESERT_COLOSSUS
#define SCENE_DESERT_COLOSSUS 0
#endif
#ifndef SCENE_GERUDO_VALLEY
#define SCENE_GERUDO_VALLEY 0
#endif
#ifndef SCENE_ZORAS_RIVER
#define SCENE_ZORAS_RIVER 0
#endif
#ifndef SCENE_GRAVEYARD
#define SCENE_GRAVEYARD 0
#endif
#ifndef SCENE_LOST_WOODS
#define SCENE_LOST_WOODS 0
#endif
#ifndef ENTR_KOKIRI_FOREST_0
#define ENTR_KOKIRI_FOREST_0 0
#endif
#ifndef ENTR_LAKE_HYLIA_NORTH_EXIT
#define ENTR_LAKE_HYLIA_NORTH_EXIT 0
#endif
#ifndef ENTR_GRAVEYARD_ENTRANCE
#define ENTR_GRAVEYARD_ENTRANCE 0
#endif
#ifndef ENTR_DEATH_MOUNTAIN_TRAIL_BOTTOM_EXIT
#define ENTR_DEATH_MOUNTAIN_TRAIL_BOTTOM_EXIT 0
#endif
#ifndef ENTR_DEATH_MOUNTAIN_CRATER_UPPER_EXIT
#define ENTR_DEATH_MOUNTAIN_CRATER_UPPER_EXIT 0
#endif
#ifndef ENTR_DESERT_COLOSSUS_EAST_EXIT
#define ENTR_DESERT_COLOSSUS_EAST_EXIT 0
#endif
#ifndef ENTR_GERUDO_VALLEY_EAST_EXIT
#define ENTR_GERUDO_VALLEY_EAST_EXIT 0
#endif
#ifndef ENTR_ZORAS_RIVER_WEST_EXIT
#define ENTR_ZORAS_RIVER_WEST_EXIT 0
#endif
/* OoT Actor water-distance field → MM depthInWater */
#ifndef yDistToWater
#define yDistToWater depthInWater
#endif
/* OoT upper-body limb index → MM upper-limb root */
#ifndef PLAYER_LIMB_UPPER
#define PLAYER_LIMB_UPPER PLAYER_LIMB_UPPER_ROOT
#endif
/* OoT Phantom-Ganon laugh sfx → silent (no MM source) */
#ifndef NA_SE_EN_FANTOM_LAUGH
#define NA_SE_EN_FANTOM_LAUGH NA_SE_NONE
#endif

#endif /* NEI_OOT_COMPAT_H */
