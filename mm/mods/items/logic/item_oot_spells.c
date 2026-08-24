/**
 * item_oot_spells.c — OoT magic spells for MM (Skijer's NEI).
 *
 * DIRECT 1:1 port of OoT's shared spell-cast pipeline (SoH z_player.c), which drives
 * Din's Fire / Farore's Wind / Nayru's Love AND the six SW97 medallion spells:
 *   Player_ActionToMagicSpell (z_player_lib.c:867)  → OotSpells_ActionToMagicSpell
 *   use-item magic gate       (z_player.c:3883-3895)→ OotSpells_TryUseItem   (called from Player_UseItem)
 *   ActionHandler_13 dispatch (z_player.c:6695-6713)→ OotSpells_HandleCsItem (called from Player_ActionHandler_13)
 *   func_8083AF44 (cast setup)                      → OotSpells_SetupCast
 *   Player_Action_808507F4 (4-phase cast action)    → Player_Action_OotMagicSpell
 *   Player_SpawnMagicSpell                          → OotSpells_SpawnMagicSpell
 *   Player_Action_8085063C (FW Return/Leave menu)   → Player_Action_OotFwMenu
 *   Player_Action_8085076C (FW warp-in arrival)     → Player_Action_OotFwArrive
 *   Player_Init Nayru re-spawn (soh 11994-11998)    → OotSpells_OnPlayerInit
 *
 * Index model (CRITICAL, from the SoH fork):
 *   magicSpell = itemAction - PLAYER_IA_MAGIC_SPELL_15 ∈ 0..5 (indexes sMagicSpellCosts)
 *   av1        = sSw97SpellActive ? magicSpell : (magicSpell - 3)   (indexes anim/voice tables +
 *                Player_SpawnMagicSpell: vanilla 0=FW/1=Nayru/2=Din, medallion 0..5)
 *
 * MM adaptations (names only, no logic changes):
 *   Magic_RequestChange(cost, WAIT_PREVIEW) == Magic_Consume(play, cost, MAGIC_CONSUME_WAIT_PREVIEW)
 *   OoT func_80839FFC (return to idle)     == MM func_80836988
 *   OoT func_8005B1A4 (release camera)     == MM Camera_SetFinishedFlag
 *   OoT IN_ITEM_CS|IN_CUTSCENE bits        == MM PLAYER_STATE1_10000000|PLAYER_STATE1_20000000
 *   OoT unk_6AD = 4 (item-CS pending)      == MM unk_AA5 = PLAYER_UNKAA5_5
 *   OoT Din's OnePointCutscene pull-back cam + func_80835EA4 csModes: NO MM analog (CutsceneManager
 *   architecture) — omitted; the normal camera stays. Gameplay identical.
 *   OoT gSaveContext.fw: MM lacks it AND MM clobbers respawn[RESPAWN_MODE_TOP] on every normal
 *   spawn (z_player.c Player_Init respawnFlag==0 path) — the warp point persists in NeiSaveData
 *   (fw* fields) and is materialized into respawn[RESPAWN_MODE_TOP] only when returning.
 *   gSaveContext.nayrusLoveTimer is a REAL MM save field (z64save.h "shield_magic_timer").
 *
 * Spell actors are the sw97 ports (real OoT overlays): Din's = Sw97 MagicFire, Farore's = MagicWind
 * (with gOotSpellsWindNoTornado latched so the vanilla cast skips the Forest-medallion tornado),
 * Nayru's = MagicDark. Registered as real actors: ACTOR_SW97_MAGIC_* (tables/actor_table.h).
 *
 * Included in the z_player.c TU via mods/items/logic/custom_items.c (unity build).
 */

#include "expansions/sw97/sw97_config.h" // SW97_MEDALLIONS_ENABLED
#include "../../extended_equipment.h"    // MAGIC_REQ (Magic Cape halves spell cost)

// ---------------------------------------------------------------------------
// z_player.c internals (defined later in this TU) — forward declarations
// ---------------------------------------------------------------------------
s32 Player_SetAction(PlayState* play, Player* this, PlayerActionFunc actionFunc, s32 arg3);
void Player_SetAction_PreserveItemAction(PlayState* play, Player* this, PlayerActionFunc actionFunc, s32 arg3);
void Player_Anim_PlayOnce(PlayState* play, Player* this, PlayerAnimationHeader* anim);
void func_80836988(Player* this, PlayState* play); // OoT func_80839FFC: end action → idle
void func_8082DAD4(Player* this);                  // OoT func_80832224: stop movement + unk_AA5 = 0
s32 Player_DecelerateToZero(Player* this);
void Player_AnimSfx_PlayVoice(Player* this, u16 sfxId);
bool Player_UpdateUpperBody(Player* this, PlayState* play);
void Player_Draw(Actor* thisx, PlayState* play);
// AnimSfxEntry is a z_player.c-local typedef (declared at z_player.c ~156, AFTER this file is
// included) — mirror the 4-byte layout under our own name; the tag-based prototype below is
// compatible with the later real one, and call sites cast.
typedef struct OotSpellsAnimSfx {
    u16 sfxId;
    s16 flags;
} OotSpellsAnimSfx;
struct AnimSfxEntry;
void Player_PlayAnimSfx(Player* this, struct AnimSfxEntry* entry);
// sw97 statics defined later in this TU (sw97_player_behavior.inc.c via sw97_router.c).
static Actor* Sw97_TrySpawnMagicSpell(PlayState* play, Player* player, s32 spell);
static s32 Sw97_IsMedallionItem(s32 item);

// C++ bridge (mods/items/logic/oot_spells_messages.cpp): Farore's Wind Return/Leave textbox.
extern void OotSpells_OpenFwTextbox(void);

// Farore's Wind warp-point pillar (z_actor.c, Skijer's NEI — OoT Actor_DrawFaroresWindPointer port).
extern void OotFw_NotifyPointSet(PlayState* play);
extern void OotFw_NotifyDispelled(void);
extern void OotFw_NotifyArrived(void);

// OoT restricts Farore's Wind to dungeons: z_parameter.c's sRestrictionFlags marks every
// overworld scene with the farores bit and dungeons are simply absent (default = allowed).
// MM mirror: the four temples, their boss lairs, and the Moon trials.
static s32 OotSpells_SceneAllowsFarores(PlayState* play) {
    switch (Play_GetOriginalSceneId(play->sceneId)) {
        case SCENE_MITURIN:    // Woodfall Temple
        case SCENE_MITURIN_BS: // Odolwa's Lair
        case SCENE_HAKUGIN:    // Snowhead Temple
        case SCENE_HAKUGIN_BS: // Goht's Lair
        case SCENE_SEA:        // Great Bay Temple
        case SCENE_SEA_BS:     // Gyorg's Lair
        case SCENE_INISIE_N:   // Stone Tower Temple
        case SCENE_INISIE_R:   // Inverted Stone Tower Temple
        case SCENE_INISIE_BS:  // Twinmold's Lair
        case SCENE_LAST_DEKU:  // Moon Trial (Deku)
        case SCENE_LAST_GORON: // Moon Trial (Goron)
        case SCENE_LAST_ZORA:  // Moon Trial (Zora)
        case SCENE_LAST_LINK:  // Moon Trial (Link)
        case SCENE_LAST_BS:    // Majora's Lair
            return true;
        default:
            return false;
    }
}

// One-shot latch read by Sw97 MagicWind_Init: vanilla Farore's Wind cast → no Forest tornado.
u8 gOotSpellsWindNoTornado = 0;

// ---------------------------------------------------------------------------
// OoT z_player_lib.c Player_ActionToMagicSpell — relies on the six IAs being
// contiguous at 0x55..0x5A (z64player.h).
// ---------------------------------------------------------------------------
s32 OotSpells_ActionToMagicSpell(Player* this, s32 itemAction) {
    s32 magicSpell = itemAction - PLAYER_IA_MAGIC_SPELL_15;

    if ((magicSpell >= 0) && (magicSpell < 6)) {
        return magicSpell;
    }
    return -1;
}

// OoT sMagicSpellCosts (z_player.c:1643, fork values — Din's costs 12 there, not stock 24).
// Indexed by magicSpell 0..5: SPELL_15, SPELL_16, SPELL_17, FARORES, NAYRUS, DINS.
static u8 sOotMagicSpellCosts[] = { 12, 24, 24, 12, 24, 12 };

// Whether the pending/active cast came from an SW97 medallion (OoT fork sSw97SpellActive).
static u8 sOotSw97SpellActive = false;

// Set while a Farore's Wind "Return" transition is in flight; consumed by OotSpells_OnPlayerInit
// to run the warp-in arrival on the other side.
static u8 sOotFwReturnPending = false;

// ---------------------------------------------------------------------------
// Cast animations — OoT-only, loaded from the companion oot.o2r (all 11 verified present).
// Same loader route as item_oot_boomerang.c: the companion gameplay_keep anim header resource is
// fully resolved by 2ship's AnimationFactory, so it is directly usable.
// ---------------------------------------------------------------------------
typedef enum {
    /* 0 */ OOT_SPELL_ANIM_TAME, // gPlayerAnim_link_magic_tame — phase A charge/kneel
    /* 1 */ OOT_SPELL_ANIM_KAZE1,
    /* 2 */ OOT_SPELL_ANIM_KAZE2,
    /* 3 */ OOT_SPELL_ANIM_KAZE3,
    /* 4 */ OOT_SPELL_ANIM_HONOO1,
    /* 5 */ OOT_SPELL_ANIM_HONOO2,
    /* 6 */ OOT_SPELL_ANIM_HONOO3,
    /* 7 */ OOT_SPELL_ANIM_TAMASHII1,
    /* 8 */ OOT_SPELL_ANIM_TAMASHII2,
    /* 9 */ OOT_SPELL_ANIM_TAMASHII3,
    /* 10 */ OOT_SPELL_ANIM_MAX
} OotSpellAnim;

static const char* sOotSpellAnimPaths[OOT_SPELL_ANIM_MAX] = {
    "__OTR__objects/gameplay_keep/gPlayerAnim_link_magic_tame",
    "__OTR__objects/gameplay_keep/gPlayerAnim_link_magic_kaze1",
    "__OTR__objects/gameplay_keep/gPlayerAnim_link_magic_kaze2",
    "__OTR__objects/gameplay_keep/gPlayerAnim_link_magic_kaze3",
    "__OTR__objects/gameplay_keep/gPlayerAnim_link_magic_honoo1",
    "__OTR__objects/gameplay_keep/gPlayerAnim_link_magic_honoo2",
    "__OTR__objects/gameplay_keep/gPlayerAnim_link_magic_honoo3",
    "__OTR__objects/gameplay_keep/gPlayerAnim_link_magic_tamashii1",
    "__OTR__objects/gameplay_keep/gPlayerAnim_link_magic_tamashii2",
    "__OTR__objects/gameplay_keep/gPlayerAnim_link_magic_tamashii3",
};

static PlayerAnimationHeader* OotSpells_GetAnim(s32 which) {
    extern AnimationHeaderCommon* ResourceMgr_LoadAnimByName(const char* path);
    PlayerAnimationHeader* anim = NULL;

    if (ResourceMgr_FileExists(sOotSpellAnimPaths[which])) {
        PlayerAnimationHeader* hdr = (PlayerAnimationHeader*)ResourceMgr_LoadAnimByName(sOotSpellAnimPaths[which]);

        if ((hdr != NULL) && (hdr->common.frameCount > 0) && (hdr->segmentVoid != NULL)) {
            anim = hdr;
        }
    }
    // Companion absent: degrade to the native idle pose instead of crashing.
    if (anim == NULL) {
        anim = (PlayerAnimationHeader*)gPlayerAnim_link_normal_wait;
    }
    return anim;
}

// OoT D_80854A58/64/70 — per-av1 raise/hold/lower anims. NOTE (fork quirk kept 1:1): the fork
// reordered the actor table for the NAYRUS-before-DINS IA order but left these tables in the old
// {kaze, honoo, tamashii} x2 layout, so Nayru's plays the honoo windup and Din's the tamashii one.
static const u8 sOotSpellRaiseAnims[6] = {
    OOT_SPELL_ANIM_KAZE1, OOT_SPELL_ANIM_HONOO1,    OOT_SPELL_ANIM_TAMASHII1,
    OOT_SPELL_ANIM_KAZE1, OOT_SPELL_ANIM_TAMASHII1, OOT_SPELL_ANIM_HONOO1,
};
static const u8 sOotSpellHoldAnims[6] = {
    OOT_SPELL_ANIM_KAZE2, OOT_SPELL_ANIM_HONOO2,    OOT_SPELL_ANIM_TAMASHII2,
    OOT_SPELL_ANIM_KAZE2, OOT_SPELL_ANIM_TAMASHII2, OOT_SPELL_ANIM_HONOO2,
};
static const u8 sOotSpellLowerAnims[6] = {
    OOT_SPELL_ANIM_KAZE3, OOT_SPELL_ANIM_HONOO3,    OOT_SPELL_ANIM_TAMASHII3,
    OOT_SPELL_ANIM_KAZE3, OOT_SPELL_ANIM_TAMASHII3, OOT_SPELL_ANIM_HONOO3,
};

// OoT D_80854A7C — per-av1 hold-loop duration threshold.
static const u8 sOotSpellHoldDurations[6] = { 70, 10, 10, 70, 10, 10 };

// OoT D_80854A80 — phase-A anim sfx (skip + sword-draw voice at frame 20, footstep at 26).
// MM AnimSfxEntry layout mirrored via OotSpellsAnimSfx; flags hand-encode MM's ANIMSFX scheme
// (the real macros are defined later in this TU): flags = ±((type << 11) | frame), where the
// NEGATIVE sign on the LAST entry marks end-of-list (== OoT's convention).
#define OOT_SPELL_ANIMSFX_GO(type, frame) ((s16)(((type) << 11) | ((frame)&0x7FF)))
#define OOT_SPELL_ANIMSFX_END(type, frame) ((s16)(-(((type) << 11) | ((frame)&0x7FF))))
#define OOT_SPELL_SFXTYPE_GENERAL 1
#define OOT_SPELL_SFXTYPE_FLOOR 2
#define OOT_SPELL_SFXTYPE_VOICE 4

static OotSpellsAnimSfx sOotSpellCastSfx[] = {
    { NA_SE_PL_SKIP, OOT_SPELL_ANIMSFX_GO(OOT_SPELL_SFXTYPE_GENERAL, 20) },
    { NA_SE_VO_LI_SWORD_N, OOT_SPELL_ANIMSFX_GO(OOT_SPELL_SFXTYPE_VOICE, 20) },
    { NA_SE_PL_WALK_GROUND, OOT_SPELL_ANIMSFX_END(OOT_SPELL_SFXTYPE_FLOOR, 26) },
};

// OoT D_80854A8C — per-av1 spell voice/sfx during the raise anim.
static OotSpellsAnimSfx sOotSpellVoiceSfx[6][2] = {
    /* 0 wind  */ { { NA_SE_PL_WALK_GROUND, OOT_SPELL_ANIMSFX_GO(OOT_SPELL_SFXTYPE_FLOOR, 20) },
                    { NA_SE_VO_LI_MAGIC_FROL, OOT_SPELL_ANIMSFX_END(OOT_SPELL_SFXTYPE_VOICE, 30) } },
    /* 1 (Nayru slot) */
    { { NA_SE_PL_WALK_GROUND, OOT_SPELL_ANIMSFX_GO(OOT_SPELL_SFXTYPE_FLOOR, 20) },
      { NA_SE_VO_LI_MAGIC_NALE, OOT_SPELL_ANIMSFX_END(OOT_SPELL_SFXTYPE_VOICE, 44) } },
    /* 2 (Din slot) */
    { { NA_SE_VO_LI_MAGIC_ATTACK, OOT_SPELL_ANIMSFX_GO(OOT_SPELL_SFXTYPE_VOICE, 20) },
      { NA_SE_IT_SWORD_SWING_HARD, OOT_SPELL_ANIMSFX_END(OOT_SPELL_SFXTYPE_GENERAL, 20) } },
    /* 3 ice   */
    { { NA_SE_PL_WALK_GROUND, OOT_SPELL_ANIMSFX_GO(OOT_SPELL_SFXTYPE_FLOOR, 20) },
      { NA_SE_VO_LI_MAGIC_FROL, OOT_SPELL_ANIMSFX_END(OOT_SPELL_SFXTYPE_VOICE, 30) } },
    /* 4 light */
    { { NA_SE_VO_LI_MAGIC_ATTACK, OOT_SPELL_ANIMSFX_GO(OOT_SPELL_SFXTYPE_VOICE, 20) },
      { NA_SE_IT_SWORD_SWING_HARD, OOT_SPELL_ANIMSFX_END(OOT_SPELL_SFXTYPE_GENERAL, 20) } },
    /* 5 fire  */
    { { NA_SE_PL_WALK_GROUND, OOT_SPELL_ANIMSFX_GO(OOT_SPELL_SFXTYPE_FLOOR, 20) },
      { NA_SE_VO_LI_MAGIC_NALE, OOT_SPELL_ANIMSFX_END(OOT_SPELL_SFXTYPE_VOICE, 44) } },
};

// Chain forward declarations.
void Player_Action_OotMagicSpell(Player* this, PlayState* play);
void Player_Action_OotFwMenu(Player* this, PlayState* play);
void Player_Action_OotFwArrive(Player* this, PlayState* play);

// ---------------------------------------------------------------------------
// Spawn — OoT Player_SpawnMagicSpell (z_player.c:11771)
// ---------------------------------------------------------------------------
Actor* OotSpells_SpawnMagicSpell(PlayState* play, Player* this, s32 spell) {
    // The THREE vanilla OoT spells now have their OWN actor ids (separate from the 6 SW97 medallion
    // actors) — no more shared-actor params flags. Fork order: 0 = Farore's Wind, 1 = Nayru's Love,
    // 2 = Din's Fire. Each actor branches on its own id (z_magic_*.inc.c) for the vanilla behavior.
    static s16 sMagicSpellActorIds[] = { ACTOR_OOT_FARORES_WIND, ACTOR_OOT_NAYRUS_LOVE, ACTOR_OOT_DINS_FIRE };

    if (sOotSw97SpellActive) {
        // Skijer's NEI FIX: a medallion cast must NEVER fall through to the vanilla table below —
        // its `spell` is the medallion index 0..5, which the vanilla table would misread (0..2 spawn
        // Farore's/Nayru's/Din's — the "fire medallion casts Din's Fire" bug — and 3..5 silently
        // return NULL). On sw97 spawn failure return NULL; the phase-B caller already handles NULL
        // (Magic_Reset + latch clear) so the cast finishes cleanly with no effect instead of
        // stalling or casting the wrong spell.
        return Sw97_TrySpawnMagicSpell(play, this, spell);
    }

    if ((spell < 0) || (spell >= ARRAY_COUNT(sMagicSpellActorIds))) {
        return NULL;
    }

    // params only matters for Farore's Wind (0 = cast-grow swirl, 1 = warp shrink-in). Din's / Nayru's
    // ignore params (they branch on actor id). Farore's cast = spell 0 = params 0.
    return Actor_Spawn(&play->actorCtx, play, sMagicSpellActorIds[spell], this->actor.world.pos.x,
                       this->actor.world.pos.y, this->actor.world.pos.z, 0, 0, 0, (spell == 0) ? 0 : 1);
}

// ---------------------------------------------------------------------------
// Use-item magic gate — OoT z_player.c:3883-3895 (called from Player_UseItem).
// Returns true when the item was a spell (handled: either armed the cast or errored).
// ---------------------------------------------------------------------------
s32 OotSpells_TryUseItem(PlayState* play, Player* this, s32 itemAction, s32 item) {
    s32 magicSpell = OotSpells_ActionToMagicSpell(this, itemAction);

    if (magicSpell < 0) {
        return false;
    }

    // Self-heal a cast that was cut short before releasing its magic/CS state (e.g. a scene load or
    // death mid-animation): TryUseItem is only reached when Link has NORMAL control, so a lingering
    // non-IDLE magic state is always stale here and would otherwise error-lock every future spell.
    // The Lens of Truth drain (CONSUME_LENS) is a legitimate ongoing state — leave it be.
    if ((this->actionFunc != Player_Action_OotMagicSpell) && (gSaveContext.magicState != MAGIC_STATE_IDLE) &&
        (gSaveContext.magicState != MAGIC_STATE_CONSUME_LENS)) {
        Magic_Reset(play);
        sOotSw97SpellActive = false;
        this->stateFlags1 &= ~(PLAYER_STATE1_10000000 | PLAYER_STATE1_20000000);
    }

    {
        // Skijer's NEI FIX (medallion stall/no-op bug): the latch must be decided by the ITEM alone.
        // It used to be `SW97_MEDALLIONS_ENABLED() && Sw97_IsMedallionItem(item)`, but equipping
        // medallions (kaleido quest page, QuestPageInteract, default ON) is NOT gated by the SW97
        // CVar (default OFF). With the CVar off, a medallion cast armed with sOotSw97SpellActive =
        // false and ran the VANILLA index path av1 = magicSpell - 3:
        //   Forest/Spirit/Shadow (magicSpell 0..2) -> av1 < 0 -> the phase machine jumped straight
        //   to the END-wait with magicState stuck at METER_FLASH_2 (the WAIT_PREVIEW reservation is
        //   never consumed because no spell ever spawns) -> Link froze mid-cast FOREVER;
        //   Water/Light/Fire (3..5) -> av1 0..2 -> cast the WRONG vanilla spell (Farore's swirl /
        //   Nayru's diamond / Din's Fire sphere instead of the Ice/Light/Fire medallion effects).
        // A medallion on a button was explicitly equipped by the player, so its cast is always the
        // medallion spell; SW97_MEDALLIONS_ENABLED keeps gating the arrows/bullets/equip UI only.
        s32 isMedallionSpell = Sw97_IsMedallionItem(item);

        // OoT: Farore's Wind is dungeon-only — overworld scenes grey it out via
        // interfaceCtx->restrictions.farores (soh z_parameter.c:1227). MM mirror: error beep.
        if ((itemAction == PLAYER_IA_FARORES_WIND) && !isMedallionSpell && !OotSpells_SceneAllowsFarores(play)) {
            Audio_PlaySfx(NA_SE_SY_ERROR);
            return true;
        }

        // Farore's Wind with an existing warp point bypasses the magic check (opens the menu).
        if (((itemAction == PLAYER_IA_FARORES_WIND) && (Nei_Save()->fwSet != 0) && !isMedallionSpell) ||
            ((gSaveContext.magicCapacity != 0) && (gSaveContext.magicState == MAGIC_STATE_IDLE) &&
             (gSaveContext.save.saveInfo.playerData.magic >= MAGIC_REQ(sOotMagicSpellCosts[magicSpell])))) {
            this->itemAction = itemAction;
            this->unk_AA5 = PLAYER_UNKAA5_5; // OoT this->unk_6AD = 4
            sOotSw97SpellActive = isMedallionSpell;
        } else {
            Audio_PlaySfx(NA_SE_SY_ERROR); // OoT Sfx_PlaySfxCentered(NA_SE_SY_ERROR)
        }
    }
    return true;
}

// ---------------------------------------------------------------------------
// Cast setup — OoT func_8083AF44 (z_player.c:6614)
// ---------------------------------------------------------------------------
static void OotSpells_SetupCast(PlayState* play, Player* this, s32 magicSpell) {
    // Skijer's NEI safety net: IAs 0x55..0x57 (magicSpell 0..2) exist ONLY as SW97 medallion spells —
    // there is no vanilla av1 slot for them (av1 = magicSpell - 3 would go NEGATIVE and hard-lock the
    // phase machine below: instant END-wait with magicState never returning to IDLE). If any arming
    // path reaches here without the latch, force it.
    if (!sOotSw97SpellActive && (magicSpell < 3)) {
        sOotSw97SpellActive = true;
    }
    Player_SetAction_PreserveItemAction(play, this, Player_Action_OotMagicSpell, 0);
    this->av1.actionVar1 = sOotSw97SpellActive ? magicSpell : (magicSpell - 3);
    this->av2.actionVar2 = 0;

    // Make the WHOLE cast uninterruptible: IN_ITEM_CS | IN_CUTSCENE make Player_InBlockingCsMode()
    // true (z_player_lib.c:662), so the AC-damage gate (z_player.c func_80834600 etc.) skips Link.
    // OoT set these only when the spell spawned (phase B), leaving the "tame" charge (phase A)
    // exposed — a hit there switched Link's action away from the cast before it released the magic +
    // CS state, stranding magicState != IDLE and error-locking every future spell. Released at the
    // end of the cast in Player_Action_OotMagicSpell.
    this->stateFlags1 |= (PLAYER_STATE1_10000000 | PLAYER_STATE1_20000000);

    // OoT Magic_RequestChange(play, cost, MAGIC_CONSUME_WAIT_PREVIEW): reserve + meter flash;
    // the actual drain starts when the spell spawns (MAGIC_STATE_CONSUME_SETUP below).
    Magic_Consume(play, MAGIC_REQ(sOotMagicSpellCosts[magicSpell]), MAGIC_CONSUME_WAIT_PREVIEW);

    PlayerAnimation_PlayOnceSetSpeed(play, &this->skelAnime, OotSpells_GetAnim(OOT_SPELL_ANIM_TAME), 0.83f);

    // OoT: Din's (magicSpell == 5) starts a OnePointCutscene pull-back camera; the other spells
    // call func_80835EA4(play, 10). Neither system exists in MM (CutsceneManager architecture) —
    // the normal camera stays. Gameplay is unaffected.
}

// ---------------------------------------------------------------------------
// CS-item dispatch — OoT Player_ActionHandler_13 spell branch (z_player.c:6695-6713).
// Called from MM's Player_ActionHandler_13 at the top of the UNKAA5_5 chain.
// Returns true when the pending item was a spell (cast or FW menu started).
// ---------------------------------------------------------------------------
s32 OotSpells_HandleCsItem(Player* this, PlayState* play) {
    s32 magicSpell = OotSpells_ActionToMagicSpell(this, this->itemAction);

    if (magicSpell < 0) {
        return false;
    }

    if ((magicSpell != 3) || (Nei_Save()->fwSet == 0) || sOotSw97SpellActive) {
        OotSpells_SetupCast(play, this, magicSpell);
    } else {
        // Farore's Wind with a warp point set → Return/Leave menu (OoT Player_Action_8085063C).
        Player_SetAction_PreserveItemAction(play, this, Player_Action_OotFwMenu, 1);
        this->stateFlags1 |= (PLAYER_STATE1_10000000 | PLAYER_STATE1_20000000); // OoT IN_ITEM_CS | IN_CUTSCENE
        this->av2.actionVar2 = 0;
        Player_Anim_PlayOnce(play, this, Player_GetIdleAnim(this));
    }
    func_8082DAD4(this); // OoT func_80832224: stop movement + clear the pending item-CS mark
    return true;
}

// ---------------------------------------------------------------------------
// The 4-phase cast action — OoT Player_Action_808507F4 (z_player.c:17044) 1:1.
// ---------------------------------------------------------------------------
void Player_Action_OotMagicSpell(Player* this, PlayState* play) {
    if (PlayerAnimation_Update(play, &this->skelAnime)) { // ---- anim just FINISHED ----
        if (this->av1.actionVar1 < 0) {                   // phase D done → END
            s32 endCast = (this->itemAction == PLAYER_IA_NAYRUS_LOVE) || sOotSw97SpellActive ||
                          (gSaveContext.magicState == MAGIC_STATE_IDLE);

            // Skijer's NEI fail-safe: METER_FLASH_2 here means the WAIT_PREVIEW magic reservation
            // from OotSpells_SetupCast was never consumed — no spell ever spawned (or the vanilla FW
            // free-recast quirk skipped the drain). Nothing will ever flip it back to IDLE for us,
            // so waiting would freeze Link in the end-of-cast pose forever. Release the reservation
            // and end the cast. (Normal casts are unaffected: their state is CONSUME_*/METER_FLASH_1
            // and the spell actor's Destroy runs Magic_Reset exactly like OoT.)
            if (!endCast && (gSaveContext.magicState == MAGIC_STATE_METER_FLASH_2)) {
                Magic_Reset(play);
                endCast = true;
            }
            if (endCast) {
                sOotSw97SpellActive = false;
                // Release the cast CS/invincibility raised in OotSpells_SetupCast.
                this->stateFlags1 &= ~(PLAYER_STATE1_10000000 | PLAYER_STATE1_20000000);
                func_80836988(this, play);                                 // OoT func_80839FFC → idle
                Camera_SetFinishedFlag(Play_GetCamera(play, CAM_ID_MAIN)); // OoT func_8005B1A4
            }
        } else {
            if (this->av2.actionVar2 == 0) { // tame(charge) done → phase B (raise + SPAWN)
                PlayerAnimation_PlayOnceSetSpeed(play, &this->skelAnime,
                                                 OotSpells_GetAnim(sOotSpellRaiseAnims[this->av1.actionVar1]), 0.83f);

                if (OotSpells_SpawnMagicSpell(play, this, this->av1.actionVar1) != NULL) {
                    this->stateFlags1 |= (PLAYER_STATE1_10000000 | PLAYER_STATE1_20000000);
                    // Vanilla FW re-cast with a warp point already set skips the drain (OoT quirk).
                    if (sOotSw97SpellActive || (this->av1.actionVar1 != 0) || (Nei_Save()->fwSet == 0)) {
                        gSaveContext.magicState = MAGIC_STATE_CONSUME_SETUP; // magic drains from here
                    }
                } else {
                    sOotSw97SpellActive = false;
                    Magic_Reset(play);
                }
            } else { // phase B done → phase C (hold loop)
                PlayerAnimation_PlayLoopSetSpeed(play, &this->skelAnime,
                                                 OotSpells_GetAnim(sOotSpellHoldAnims[this->av1.actionVar1]), 0.83f);

                if ((this->av1.actionVar1 == 0) && !sOotSw97SpellActive) { // Farore's Wind only
                    this->av2.actionVar2 = -10;                            // warp-point-save countdown
                }
            }
            this->av2.actionVar2++;
        }
    } else {                            // ---- anim still PLAYING ----
        if (this->av2.actionVar2 < 0) { // FW warp-point-save countdown (OoT z_player.c:17088)
            this->av2.actionVar2++;
            if (this->av2.actionVar2 == 0) {
                NeiSaveData* nei = Nei_Save();
                RespawnData* down = &gSaveContext.respawn[RESPAWN_MODE_DOWN];

                // OoT: Play_SetupRespawnPoint(RESPAWN_MODE_TOP) + gSaveContext.fw = respawn[DOWN].
                // MM clobbers respawn[TOP] on every normal spawn, so the point persists in
                // NeiSaveData and is rebuilt into respawn[TOP] when returning (Player_Action_OotFwMenu).
                nei->fwSet = 1;
                nei->fwPosX = down->pos.x;
                nei->fwPosY = down->pos.y;
                nei->fwPosZ = down->pos.z;
                nei->fwYaw = down->yaw;
                nei->fwPlayerParams = down->playerParams;
                nei->fwEntrance = down->entrance;
                nei->fwRoomIndex = down->roomIndex;
                nei->fwTempSwitchFlags = down->tempSwitchFlags;
                nei->fwTempCollectFlags = down->tempCollectFlags;

                // Start the pillar: grow at Link's cast spot + sparkle-fly to the stored point
                // (OoT respawn[TOP].data = 1 + Play_SetupRespawnPoint(TOP, 0x6FF)).
                OotFw_NotifyPointSet(play);

                this->av2.actionVar2 = 2;
            }
        } else if (this->av1.actionVar1 >= 0) {
            if (this->av2.actionVar2 == 0) {
                // phase-A sfx (OoT D_80854A80)
                Player_PlayAnimSfx(this, (struct AnimSfxEntry*)sOotSpellCastSfx);
            } else if (this->av2.actionVar2 == 1) {
                // per-spell voice (OoT D_80854A8C)
                Player_PlayAnimSfx(this, (struct AnimSfxEntry*)sOotSpellVoiceSfx[this->av1.actionVar1]);
                // Drop IN_ITEM_CS|IN_CUTSCENE at frame 30 for Din's Fire / the medallions once the
                // spell actor has spawned — OoT z_player.c:14886. THIS IS LOAD-BEARING FOR DAMAGE:
                // while PLAYER_STATE1_20000000 is set, Actor_UpdateAll FREEZES the enemy category
                // (z_actor.c:3081, sCategoryFreezeMasks & stateFlags1), so frozen enemies never
                // register their AC collider and the fire dome can't hit them. The charge (phase A)
                // stays protected because the flags were raised in OotSpells_SetupCast; here we
                // release them so the active fire can actually deal damage / light torches.
                if (((this->av1.actionVar1 == 2) || sOotSw97SpellActive) &&
                    PlayerAnimation_OnFrame(&this->skelAnime, 30.0f)) {
                    this->stateFlags1 &= ~(PLAYER_STATE1_10000000 | PLAYER_STATE1_20000000);
                }
            } else if (sOotSpellHoldDurations[this->av1.actionVar1] < this->av2.actionVar2++) {
                PlayerAnimation_PlayOnceSetSpeed(play, &this->skelAnime,
                                                 OotSpells_GetAnim(sOotSpellLowerAnims[this->av1.actionVar1]),
                                                 0.83f); // phase D lower-arms
                this->yaw = this->actor.shape.rot.y;
                this->av1.actionVar1 = -1;
            }
        }
    }

    Player_DecelerateToZero(this);
}

// ---------------------------------------------------------------------------
// Farore's Wind Return/Leave menu — OoT Player_Action_8085063C (z_player.c:16925) 1:1.
// ---------------------------------------------------------------------------
void Player_Action_OotFwMenu(Player* this, PlayState* play) {
    this->stateFlags2 |= PLAYER_STATE2_20; // OoT PLAYER_STATE2_DISABLE_ROTATION_Z_TARGET

    PlayerAnimation_Update(play, &this->skelAnime);
    Player_UpdateUpperBody(this, play);

    if (this->av2.actionVar2 == 0) {
        OotSpells_OpenFwTextbox(); // OoT Message_StartTextbox(play, 0x3B, &this->actor)
        this->av2.actionVar2 = 1;
        return;
    }

    // MM DIFFERS FROM OoT: a plain TWO/THREE_CHOICE box does NOT self-close in Message_Update
    // (z_message.c MSGMODE_TEXT_DONE → the choice-endType branch is a no-op). The opener must detect
    // TEXT_STATE_CHOICE + the A press (Message_ShouldAdvance), read choiceIndex, act, and call
    // Message_CloseTextbox itself — exactly like Obj_Warpstone's owl-warp menu. (OoT's handler waited
    // on TEXT_STATE_CLOSING because OoT's message system closed choice boxes on A; MM's does not, so
    // that state never arrived and A/B appeared dead.)
    if ((Message_GetState(&play->msgCtx) == TEXT_STATE_CHOICE) && Message_ShouldAdvance(play)) {
        NeiSaveData* nei = Nei_Save();
        s32 choice = play->msgCtx.choiceIndex;

        Message_CloseTextbox(play);

        if (choice == 0) { // Return to the warp point
            RespawnData* top = &gSaveContext.respawn[RESPAWN_MODE_TOP];

            // Materialize the persisted warp point into respawn[TOP]; respawnFlag = 3 makes MM's
            // Player_Init place Link there (respawnIndex = respawnFlag - 1 = RESPAWN_MODE_TOP).
            top->pos.x = nei->fwPosX;
            top->pos.y = nei->fwPosY;
            top->pos.z = nei->fwPosZ;
            top->yaw = nei->fwYaw;
            top->playerParams = PLAYER_PARAMS(0xFF, PLAYER_START_MODE_D); // stationary; arrival is ours
            top->entrance = nei->fwEntrance;
            top->roomIndex = nei->fwRoomIndex;
            top->data = 1;
            top->tempSwitchFlags = nei->fwTempSwitchFlags;
            top->unk_18 = 0;
            top->tempCollectFlags = nei->fwTempCollectFlags;

            sOotFwReturnPending = true;
            gSaveContext.respawnFlag = 3;
            play->nextEntrance = nei->fwEntrance;
            play->transitionTrigger = TRANS_TRIGGER_START;
            play->transitionType = TRANS_TYPE_FADE_WHITE; // OoT TRANS_TYPE_FADE_WHITE_FAST
            return;
        }

        if (choice == 1) { // Dispel the warp point
            Vec3f fwPos;

            fwPos.x = nei->fwPosX;
            fwPos.y = nei->fwPosY;
            fwPos.z = nei->fwPosZ;
            nei->fwSet = 0;
            Audio_PlaySfx_AtPos(&fwPos, NA_SE_PL_MAGIC_WIND_VANISH);
            OotFw_NotifyDispelled(); // pillar shrinks back to nothing (OoT data = -data)
        }
        // choice == 2 (Exit) → just close, no effect (OoT 0x3B's third option).

        func_80853080(this, play); // shared OoT helper (already shimmed): back to idle anim/yaw
        func_80836988(this, play);
        Camera_SetFinishedFlag(Play_GetCamera(play, CAM_ID_MAIN));
        this->stateFlags1 &= ~(PLAYER_STATE1_10000000 | PLAYER_STATE1_20000000);
    }
}

// ---------------------------------------------------------------------------
// Farore's Wind warp-in arrival — OoT Player_StartMode_FaroresWind + Player_Action_8085076C.
// ---------------------------------------------------------------------------
void Player_Action_OotFwArrive(Player* this, PlayState* play) {
    this->av2.actionVar2++;

    if (this->av2.actionVar2 == 20) {
        Audio_PlaySfx_AtPos(&this->actor.world.pos, NA_SE_PL_MAGIC_WIND_WARP);
        // Spawn the vanilla Farore's Wind "warp arrival" visual (its own actor, params 1 = shrink-in;
        // never the tornado).
        Actor_Spawn(&play->actorCtx, play, ACTOR_OOT_FARORES_WIND, this->actor.world.pos.x, this->actor.world.pos.y,
                    this->actor.world.pos.z, 0, 0, 0, 1);
        // The point is consumed on arrival: the pillar fades into the camera and clears fwSet
        // when fully transparent (OoT Player_Action_8085076C bumping respawn[TOP].data past 40).
        OotFw_NotifyArrived();
    }

    if (this->av2.actionVar2 > 20) {
        this->actor.draw = Player_Draw;
        this->actor.world.pos.y += 60.0f; // OoT: drop in from 60 units up
        this->stateFlags1 &= ~PLAYER_STATE1_20000000;
        func_80836988(this, play);
    }
}

// ---------------------------------------------------------------------------
// Player_Init hook — OoT Player_Init:11994 (Nayru re-spawn) + FW arrival start mode.
// Called near the end of MM's Player_Init (patched in z_player.c).
// ---------------------------------------------------------------------------
void OotSpells_OnPlayerInit(PlayState* play, Player* this) {
    // Farore's Wind return arrival (OoT PLAYER_START_MODE_FARORES_WIND).
    if (sOotFwReturnPending) {
        sOotFwReturnPending = false;
        this->actor.draw = NULL; // invisible while warping in
        this->stateFlags1 |= PLAYER_STATE1_20000000;
        Player_SetAction(play, this, Player_Action_OotFwArrive, 0);
        this->av2.actionVar2 = 0;
    }

    // Nayru's Love persistence: re-spawn the BLUE diamond shield if its timer is still running
    // (OoT: magicState = METER_FLASH_1 + Player_SpawnMagicSpell(play, this, 1) → ACTOR_OOT_NAYRUS_LOVE,
    // which sees nayrusLoveTimer != 0 in Init and boots straight into the diamond phase).
    // The Shadow Medallion no longer persists here — it drives its OWN actor-local shadowTimer (not
    // nayrusLoveTimer), so it ends on scene change like a temporary stealth buff.
    if (gSaveContext.nayrusLoveTimer != 0) {
        gSaveContext.magicState = MAGIC_STATE_METER_FLASH_1;
        sOotSw97SpellActive = false;
        OotSpells_SpawnMagicSpell(play, this, 1);
    }
}
