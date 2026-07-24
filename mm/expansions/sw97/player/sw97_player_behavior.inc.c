/**
 * sw97_player_behavior.inc.c - Player behavior hooks for SW97 Medallion Spells
 *
 * Original actors: z64proto/sw97 team (Spaceworld '97 Experience)
 * Adapted for Ship of Harkinian (Shipwright)
 *
 * Provides CVar-gated hooks for:
 * - Magic spell actor spawning (6 spells mapped to spell indices 0-5)
 * - Helper functions for medallion/arrow item identification
 * - Medallion-to-arrow item conversion
 */

// Runtime actor IDs (set by sw97_init.cpp via ActorDB)
extern s16 gSw97ActorId_MagicFire;
extern s16 gSw97ActorId_MagicIce;
extern s16 gSw97ActorId_MagicLight;
extern s16 gSw97ActorId_MagicDark;
extern s16 gSw97ActorId_MagicSoul;
extern s16 gSw97ActorId_MagicWind;
extern s16 gSw97ActorId_ArrowFire;
extern s16 gSw97ActorId_ArrowIce;
extern s16 gSw97ActorId_ArrowLight;
extern s16 gSw97ActorId_ArrowDark;
extern s16 gSw97ActorId_ArrowSoul;
extern s16 gSw97ActorId_ArrowWind;

// SW97 magic spell costs: indices 0-5 match Player_ActionToMagicSpell output
// 0=Wind(12), 1=Soul(24), 2=Dark(12), 3=Ice(24), 4=Light(24), 5=Fire(12)
static u8 sSw97MagicSpellCosts[] = { 12, 24, 12, 24, 24, 12 };

// SW97 magic arrow costs: all 4 except light which is 8
static u8 sSw97MagicArrowCosts[] = { 4, 4, 8, 4, 4, 4 };

/**
 * Spawn the correct SW97 magic spell actor based on spell index.
 * Called from Player_SpawnMagicSpell in z_player.c.
 *
 * Spell index mapping (from Player_ActionToMagicSpell):
 *   0 = IA_MAGIC_SPELL_15 = Forest Medallion → MagicWind
 *   1 = IA_MAGIC_SPELL_16 = Spirit Medallion → MagicSoul
 *   2 = IA_MAGIC_SPELL_17 = Shadow Medallion → MagicDark
 *   3 = IA_FARORES_WIND   = Water Medallion  → MagicIce
 *   4 = IA_NAYRUS_LOVE    = Light Medallion  → MagicLight
 *   5 = IA_DINS_FIRE       = Fire Medallion   → MagicFire
 *
 * Returns the spawned actor, or NULL if SW97 spells are disabled.
 */
static Actor* Sw97_TrySpawnMagicSpell(PlayState* play, Player* player, s32 spell) {
    // Skijer's NEI FIX: NO SW97_MEDALLIONS_ENABLED() recheck here. This is only ever reached from
    // OotSpells_SpawnMagicSpell when the cast was ARMED as a medallion (sOotSw97SpellActive) — the
    // enable decision already happened at arm time. Rechecking the CVar mid-cast made the spawn
    // return NULL (and previously fall through to the WRONG vanilla spell actor) whenever the CVar
    // was off, even though the player had a medallion equipped and fully charged the cast.

    if (spell < 0 || spell >= 6) {
        return NULL;
    }

    // Shadow medallion heart→magic exchange is handled out-of-band in
    // soh/Enhancements/ShadowMedallionExchange.cpp via an OnPlayerUpdate hook,
    // so the exchange works even when the player has zero magic (otherwise the
    // cast flow short-circuits before reaching this function).

    s16* spellActorIds[] = {
        &gSw97ActorId_MagicWind,  // 0 = Forest
        &gSw97ActorId_MagicSoul,  // 1 = Spirit
        &gSw97ActorId_MagicDark,  // 2 = Shadow
        &gSw97ActorId_MagicIce,   // 3 = Water
        &gSw97ActorId_MagicLight, // 4 = Light
        &gSw97ActorId_MagicFire,  // 5 = Fire
    };

    s16 actorId = *spellActorIds[spell];
    if (actorId < 0) {
        return NULL;
    }

    Actor* spawned = Actor_Spawn(&play->actorCtx, play, actorId, player->actor.world.pos.x,
                                 player->actor.world.pos.y, player->actor.world.pos.z, 0, 0, 0, 0);

    // Tell teammates to spawn the same spell-effect actor on their side.
    // Spells follow the caster (attached_to_owner=1) — their visual stays
    // around the caster's dummy as long as the spell is active. Map the
    // spell index back to the corresponding HARPOON_VFX_KIND_SW97_MAGIC_*.
    if (spawned != NULL) {
        s32 vfxKindByIndex[] = {
            HARPOON_VFX_KIND_SW97_MAGIC_WIND,   // 0
            HARPOON_VFX_KIND_SW97_MAGIC_SOUL,   // 1
            HARPOON_VFX_KIND_SW97_MAGIC_DARK,   // 2
            HARPOON_VFX_KIND_SW97_MAGIC_ICE,    // 3
            HARPOON_VFX_KIND_SW97_MAGIC_LIGHT,  // 4
            HARPOON_VFX_KIND_SW97_MAGIC_FIRE,   // 5
        };
        Harpoon_NotifyVfxSpawn(spawned, vfxKindByIndex[spell], /*attachedToOwner=*/1);
    }
    return spawned;
}

/**
 * Check if an item ID is a quest medallion (spell mode).
 */
static s32 Sw97_IsMedallionItem(s32 item) {
    // Exact-match the six medallions. The ids are NOT contiguous (0xF3..0xF6 then 0xF9/0xFA with
    // 0xF7/0xF8 belonging to other sentinels), and this predicate now solely decides the
    // sOotSw97SpellActive medallion-cast latch — no false positives allowed.
    switch (item) {
        case ITEM_MEDALLION_FOREST:
        case ITEM_MEDALLION_FIRE:
        case ITEM_MEDALLION_WATER:
        case ITEM_MEDALLION_SPIRIT:
        case ITEM_MEDALLION_SHADOW:
        case ITEM_MEDALLION_LIGHT:
            return true;
        default:
            return false;
    }
}

/**
 * Check if an item ID is an SW97 arrow variant (arrow mode).
 */
static s32 Sw97_IsArrowItem(s32 item) {
    return (item >= ITEM_SW97_ARROW_FIRE && item <= ITEM_SW97_ARROW_WIND);
}

/**
 * Convert a medallion item to its corresponding SW97 arrow item.
 * Used by L+C swap in z_player.c.
 */
s32 Sw97_MedallionToArrowItem(s32 medallionItem) {
    switch (medallionItem) {
        case ITEM_MEDALLION_FIRE:
            return ITEM_SW97_ARROW_FIRE;
        case ITEM_MEDALLION_WATER:
            return ITEM_SW97_ARROW_ICE;
        case ITEM_MEDALLION_LIGHT:
            return ITEM_SW97_ARROW_LIGHT;
        case ITEM_MEDALLION_SHADOW:
            return ITEM_SW97_ARROW_DARK;
        case ITEM_MEDALLION_SPIRIT:
            return ITEM_SW97_ARROW_SOUL;
        case ITEM_MEDALLION_FOREST:
            return ITEM_SW97_ARROW_WIND;
        default:
            return ITEM_NONE;
    }
}

/**
 * Returns true while the Shadow Medallion spell (MagicDark) is active.
 * MagicDark drives gSaveContext.nayrusLoveTimer for its lifetime; in SW97 mode
 * the Shadow medallion replaces Nayru's Love (Light medallion is the new NL slot),
 * so a nonzero timer + SW97 enabled uniquely identifies "Shadow stealth is on".
 *
 * Consumed by z_actor.c so enemies/NPCs can't detect Link (same hook point as
 * MmMaskWear_IsStoneMaskActive).
 */
s32 Sw97_ShadowStealthActive(void) {
    if (!SW97_MEDALLIONS_ENABLED()) return 0;
    return gSaveContext.nayrusLoveTimer > 0;
}

/**
 * Shadow Medallion heart→magic exchange.
 *
 * Hold the C-button that has ITEM_MEDALLION_SHADOW for SHADOW_EXCHANGE_HOLD_FRAMES
 * frames → spend 3 hearts, gain 24 magic. Disarmed until release.
 *
 * Must work even at zero magic (the vanilla cast pipeline short-circuits before
 * reaching Sw97_TrySpawnMagicSpell when magic is insufficient — playing only the
 * "no magic" error sound — so the exchange has to live in a per-frame tick).
 *
 * Called from z_player.c Player_UpdateCommon each frame.
 */
#define SHADOW_EXCHANGE_HOLD_FRAMES 20
#define SHADOW_EXCHANGE_HEART_COST  (3 * 0x10)  // 3 hearts × 16 HP
#define SHADOW_EXCHANGE_MAGIC_GAIN  24

void Sw97_TickShadowExchange(PlayState* play, Player* player) {
    if (!SW97_MEDALLIONS_ENABLED()) return;
    if (play == NULL || player == NULL) return;

    // Find which C-slot has the Shadow medallion. buttonItems[0]=B, [1..3]=C-LDR.
    u16 medallionMask = 0;
    if (gSaveContext.save.saveInfo.equips.buttonItems[1] == ITEM_MEDALLION_SHADOW) medallionMask |= BTN_CLEFT;
    if (gSaveContext.save.saveInfo.equips.buttonItems[2] == ITEM_MEDALLION_SHADOW) medallionMask |= BTN_CDOWN;
    if (gSaveContext.save.saveInfo.equips.buttonItems[3] == ITEM_MEDALLION_SHADOW) medallionMask |= BTN_CRIGHT;

    static s16 sShadowHoldFrames = 0;
    static u8 sShadowExchanged = 0;

    if (medallionMask == 0) {
        sShadowHoldFrames = 0;
        sShadowExchanged = 0;
        return;
    }

    u16 cur = play->state.input[0].cur.button;
    if (!(cur & medallionMask)) {
        sShadowHoldFrames = 0;
        sShadowExchanged = 0;
        return;
    }

    sShadowHoldFrames++;
    if (sShadowExchanged) return;
    if (sShadowHoldFrames < SHADOW_EXCHANGE_HOLD_FRAMES) return;
    if (gSaveContext.save.saveInfo.playerData.health <= SHADOW_EXCHANGE_HEART_COST) return;

    gSaveContext.save.saveInfo.playerData.health -= SHADOW_EXCHANGE_HEART_COST;
    gSaveContext.save.saveInfo.playerData.magic += SHADOW_EXCHANGE_MAGIC_GAIN;
    if (gSaveContext.save.saveInfo.playerData.magic > gSaveContext.magicCapacity) {
        gSaveContext.save.saveInfo.playerData.magic = gSaveContext.magicCapacity;
    }
    Actor_PlaySfx(&player->actor, NA_SE_SY_GET_RUPY);
    sShadowExchanged = 1;
}

/**
 * Shadow-element blindness — per-actor stealth.
 *
 * When Shadow ARROW (ARROW_SW97_0C) hits an enemy, OR when the Gust Jar's
 * Shadow-element BLOW pushes an enemy, the target is "blinded" for ~10
 * seconds: z_actor.c's distance-to-player calculation is spoofed to 32000
 * (same mechanism as Stone Mask + Shadow Medallion stealth), so the enemy
 * stops tracking Link until the timer expires.
 *
 * Storage is a small static table indexed by Actor*. Capacity 32 is plenty
 * for the worst-case crowd you'd reasonably blind in one fight. New tags
 * upsert (longer-of duration); expired slots are reused.
 *
 * `Sw97_TickBlindness` MUST be called once per frame from z_player.c so
 * `framesRemaining` actually counts down.
 */
#define SW97_BLIND_TABLE_SIZE 32
#define SW97_BLIND_DURATION   300  // 10 sec at SW97's 30fps timer convention

typedef struct {
    Actor* actor;
    s16    framesRemaining;
} Sw97BlindEntry;

static Sw97BlindEntry sSw97Blinded[SW97_BLIND_TABLE_SIZE];

void Sw97_TagBlinded(Actor* actor, s16 frames) {
    if (actor == NULL || actor->update == NULL) return;
    s32 empty = -1;
    for (s32 i = 0; i < SW97_BLIND_TABLE_SIZE; i++) {
        if (sSw97Blinded[i].actor == actor) {
            if (frames > sSw97Blinded[i].framesRemaining) {
                sSw97Blinded[i].framesRemaining = frames;
            }
            return;
        }
        if (sSw97Blinded[i].actor == NULL && empty < 0) empty = i;
    }
    if (empty >= 0) {
        sSw97Blinded[empty].actor = actor;
        sSw97Blinded[empty].framesRemaining = frames;
    }
}

s32 Sw97_IsBlinded(Actor* actor) {
    if (actor == NULL) return 0;
    for (s32 i = 0; i < SW97_BLIND_TABLE_SIZE; i++) {
        if (sSw97Blinded[i].actor == actor && sSw97Blinded[i].framesRemaining > 0) {
            return 1;
        }
    }
    return 0;
}

void Sw97_TickBlindness(void) {
    for (s32 i = 0; i < SW97_BLIND_TABLE_SIZE; i++) {
        if (sSw97Blinded[i].actor == NULL) continue;
        // Drop dead actors immediately so we don't keep their pointer.
        if (sSw97Blinded[i].actor->update == NULL) {
            sSw97Blinded[i].actor = NULL;
            sSw97Blinded[i].framesRemaining = 0;
            continue;
        }
        if (--sSw97Blinded[i].framesRemaining <= 0) {
            sSw97Blinded[i].actor = NULL;
            sSw97Blinded[i].framesRemaining = 0;
        }
    }
}

/**
 * Cucco Mode — Soul arrow + Cucco → 30-second transformation.
 *
 * Triggered by `ArrowSoul_TryTransform` when the soul arrow hits an `EN_NIW`.
 * Visual: Cucco model swap on the Player draw function. Movement: a Flappy
 * Bird-style flap (A press while airborne = upward burst, reduced gravity for
 * slow fall). Bow / slingshot shots become elemental eggs (free, no magic
 * cost). R = spawn 3 attack-cuccos orbiting Link. B in air = peck dive.
 *
 * State is global so other systems (player draw hook, input intercept, egg
 * spawner) can query without threading through a parameter.
 */
#define CUCCO_MODE_FRAMES    1800  // 30 sec
#define CUCCO_FLAP_VELOCITY  9.0f  // upward burst per A press while airborne
#define CUCCO_GRAVITY       -1.2f  // Cucco terminal: gentle fall, not Link's -7
#define CUCCO_MAX_VY_DOWN   -3.0f  // Cucco terminal velocity cap (float, not plummet)
#define CUCCO_SPEED_MULT     1.15f // Slightly faster than Link
#define CUCCO_SPEED_MAX      11.0f // Cap so flap doesn't compound forever

s32 gSw97CuccoModeActive  = 0;
// Pending → waiting for Link to leave PLAYER_STATE1_IN_ITEM_CS (the
// first-person aim/throw cutscene). Same pattern as magic_soul.inc.c:117
// where the diamond update returns until the player is free. Without this
// the camera stays glued in first-person mode and breaks on entry.
s32 gSw97CuccoModePending = 0;
s32 gSw97CuccoModeTimer   = 0;
// Once-shot exit fx flag — guarantees the un-transform flash/sound only
// plays once even though Sw97_TickCuccoMode keeps running on inactive.
static s32 gSw97CuccoExitFx = 0;
// 180° flip animation on egg throw — counts down each frame, used by
// Sw97_DrawCuccoModel to rotate the model. ~12 frames = ~0.4s flip.
s32   gSw97CuccoFlipTimer = 0;
#define CUCCO_FLIP_FRAMES 12

// Cucco draw — direct copy of HGrace's draw-override pattern (no actor
// puppet). Skeleton inited once per cucco-mode session via Sw97_InitCuccoSkel,
// rendered via Sw97_DrawCucco which Link's actor.draw points at.
#include "objects/object_niw/object_niw.h"
static SkelAnime sCuccoSkel;
static Vec3s sCuccoJointTable[16];
static Vec3s sCuccoMorphTable[16];
static u8 sCuccoSkelInited = 0;

static void Sw97_InitCuccoSkel(PlayState* play) {
    if (sCuccoSkelInited) return;
    SkelAnime_InitFlex(play, &sCuccoSkel, (FlexSkeletonHeader*)&gNiwSkel,
                       (AnimationHeader*)&gNiwIdleAnim,
                       sCuccoJointTable, sCuccoMorphTable, 16);
    sCuccoSkelInited = 1;
}

// Null-body override — used to walk Link's skeleton without rendering any
// limb geometry. Same idea as GaroForm_OverrideLimbDraw in
// garo_post_limb.cpp:47.
static s32 Sw97_CuccoLinkOverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList,
                                          Vec3f* pos, Vec3s* rot, void* arg) {
    (void)play; (void)limbIndex; (void)pos; (void)rot; (void)arg;
    *dList = NULL;
    return 0;
}

// PostLimbDraw — refreshes the per-frame Link tracking fields that vanilla
// Player_Draw normally populates: bodyPartsPos[], focus.pos (Navi anchor),
// feetPos[] (shadow anchor). Without this, shadow + Navi stay frozen at the
// transformation point. Copied from GaroForm_PostLimbDraw (the essential
// shadow/Navi/bodyParts bits — Garo's sword-trail / held-actor branches are
// not needed for the cucco model swap).
static void Sw97_CuccoLinkPostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList,
                                       Vec3s* rot, void* thisx) {
    (void)dList; (void)rot;
    Player* player = (Player*)thisx;
    Vec3f zeroVec = { 0.0f, 0.0f, 0.0f };

    if (limbIndex > 0 && limbIndex < PLAYER_LIMB_MAX) {
        s8 bodyPart = gPlayerLimbToBodyPart[limbIndex];
        if (bodyPart >= 0) {
            Matrix_MultVec3f(&zeroVec, &player->bodyPartsPos[bodyPart]);
        }
    }
    if (limbIndex == PLAYER_LIMB_HEAD) {
        Vec3f headOffset = { 1100.0f, -700.0f, 0.0f };
        Matrix_MultVec3f(&headOffset, &player->actor.focus.pos);
    }
    if (limbIndex == PLAYER_LIMB_LEFT_FOOT || limbIndex == PLAYER_LIMB_RIGHT_FOOT) {
        Actor_SetFeetPos(&player->actor, limbIndex,
                         PLAYER_LIMB_LEFT_FOOT, &zeroVec,
                         PLAYER_LIMB_RIGHT_FOOT, &zeroVec);
    }
}

// Cucco visual at thisx->world.pos. Same scaled translate + Y-flip on egg
// throws as before, called from Sw97_DrawCuccoForm after the null-body pass.
static void Sw97_DrawCuccoModel(Actor* thisx, PlayState* play) {
    if (!sCuccoSkelInited) return;
    OPEN_DISPS(play->state.gfxCtx);
    Gfx_SetupDL25_Opa(play->state.gfxCtx);
    Matrix_Translate(thisx->world.pos.x, thisx->world.pos.y, thisx->world.pos.z, MTXMODE_NEW);
    f32 baseYaw = (f32)thisx->shape.rot.y * (M_PI / 32768.0f);
    f32 flipYaw = 0.0f;
    if (gSw97CuccoFlipTimer > 0) {
        f32 t = (f32)gSw97CuccoFlipTimer / (f32)CUCCO_FLIP_FRAMES;
        flipYaw = (1.0f - t) * M_PI;
    }
    Matrix_RotateYF(baseYaw + flipYaw, MTXMODE_APPLY);
    f32 s = 0.015f;
    Matrix_Scale(s, s, s, MTXMODE_APPLY);
    SkelAnime_DrawFlexOpa(play, sCuccoSkel.skeleton, sCuccoSkel.jointTable,
                          sCuccoSkel.dListCount, NULL, NULL, NULL);
    CLOSE_DISPS(play->state.gfxCtx);
}

// Public entry — called from customequipment.cpp's VB_PLAYER_DRAW_BEGIN hook
// when cucco mode is active. Mirrors the GaroForm / MmForm pattern:
//   Pass 1: walk Link's skeleton with nulled DLs so PostLimbDraw refreshes
//           bodyPartsPos / focus.pos / feetPos[] → shadow + Navi follow
//   Pass 2: render the cucco model at Link's world.pos
void Sw97_DrawCuccoForm(PlayState* play, Player* player) {
    if (player->skelAnime.skeleton != NULL && player->skelAnime.jointTable != NULL) {
        SkelAnime_DrawFlexLod(play, player->skelAnime.skeleton, player->skelAnime.jointTable,
                              player->skelAnime.dListCount, Sw97_CuccoLinkOverrideLimbDraw,
                              Sw97_CuccoLinkPostLimbDraw, player, 0);
    }
    Sw97_DrawCuccoModel(&player->actor, play);
}

void Sw97_StartCuccoMode(void) {
    if (gSw97CuccoModeActive || gSw97CuccoModePending) return; // idempotent
    // Don't activate immediately — Link is in first-person aim CS right
    // now. Set pending and let the per-frame tick activate once the
    // first-person camera setting releases (mirror magic_soul.inc.c:117).
    gSw97CuccoModePending = 1;
}

static void Sw97_ActivateCuccoMode(void) {
    gSw97CuccoModePending = 0;
    gSw97CuccoModeActive  = 1;
    gSw97CuccoModeTimer   = CUCCO_MODE_FRAMES;
}

void Sw97_EndCuccoMode(void) {
    if (!gSw97CuccoModeActive && !gSw97CuccoModePending) return;
    gSw97CuccoModeActive  = 0;
    gSw97CuccoModePending = 0;
    gSw97CuccoModeTimer   = 0;
    gSw97CuccoExitFx      = 1;
    // Player flag cleanup + draw restoration happens in the inactive branch
    // of Sw97_TickCuccoMode on the next frame.
}

s32 Sw97_IsCuccoModeActive(void) {
    return gSw97CuccoModeActive;
}

// Scene tracking — reset state on scene change so the next tick doesn't
// reference a stale collision context / pos.
static s32 gSw97CuccoLastScene = -1;

// ───────────────────────────────────────────────────────────────────────
// Cucco eggs — bow/slingshot/sw97-arrow items on C-buttons spawn an
// elemental EnArrow projectile forward when pressed in cucco mode. Free
// (no ammo cost), bypasses the bow first-person CS entirely.
// ───────────────────────────────────────────────────────────────────────

// Map an equipped item ID to an EnArrow `params` value:
//   - ITEM_SW97_ARROW_FIRE..WIND  → ARROW_SW97_FIRE..0E (elemental)
//   - ITEM_BOW                    → ARROW_NORMAL (neutral)
//   - ITEM_SLINGSHOT              → ARROW_SEED (neutral seed)
// Returns -1 if the item isn't fireable in cucco mode.
static s32 Sw97_CuccoEgg_ItemToParams(u8 item) {
    if (item >= ITEM_SW97_ARROW_FIRE && item <= ITEM_SW97_ARROW_WIND) {
        return ARROW_SW97_FIRE + (item - ITEM_SW97_ARROW_FIRE);
    }
    if (item == ITEM_BOW)        return ARROW_NORMAL;
    if (item == ITEM_SLINGSHOT)  return ARROW_SEED;
    return -1;
}

#define CUCCO_EGG_SPEED      26.0f  // Forward velocity given to the egg
#define CUCCO_EGG_SPAWN_FWD  20.0f  // Distance ahead of Link to spawn

static void Sw97_CuccoEgg_Spawn(PlayState* play, Player* player, s16 arrowType) {
    s16 yaw = player->actor.shape.rot.y;
    f32 fx  = Math_SinS(yaw);
    f32 fz  = Math_CosS(yaw);

    Actor* arrow = Actor_Spawn(
        &play->actorCtx, play, ACTOR_EN_ARROW,
        player->actor.world.pos.x + fx * CUCCO_EGG_SPAWN_FWD,
        player->actor.world.pos.y + 30.0f,
        player->actor.world.pos.z + fz * CUCCO_EGG_SPAWN_FWD,
        0, yaw, 0,
        arrowType);
    if (arrow != NULL) {
        arrow->speed   = CUCCO_EGG_SPEED;
        arrow->velocity.x = fx * CUCCO_EGG_SPEED;
        arrow->velocity.z = fz * CUCCO_EGG_SPEED;
        arrow->velocity.y = 0.0f;
        // Parent = Link so SW97 hit hooks can identify the originator
        // (some elemental effects key off arrow->parent == GET_PLAYER).
        arrow->parent = &player->actor;
    }

    // Cosmetic feedback: 180° Y-flip animation + cucco squawk.
    gSw97CuccoFlipTimer = CUCCO_FLIP_FRAMES;
    Actor_PlaySfx(&player->actor, NA_SE_EV_CHICKEN_CRY_A);
}

static void Sw97_TickCuccoEggs(PlayState* play, Player* player) {
    if (!gSw97CuccoModeActive) return;

    // Same 7-button array Ivan uses (z_en_partner.c:829): 3 C-buttons +
    // 4 D-pad slots when DpadEquips CVar is on.
    static const u16 partnerButtons[7] = {
        BTN_CLEFT, BTN_CDOWN, BTN_CRIGHT,
        BTN_DUP, BTN_DDOWN, BTN_DLEFT, BTN_DRIGHT,
    };
    u8 buttonMax = 3;
    if (CVarGetInteger(CVAR_ENHANCEMENT("DpadEquips"), 0) != 0) {
        buttonMax = ARRAY_COUNT(gSaveContext.save.saveInfo.equips.cButtonSlots);
    }

    u16 pressedMask = play->state.input[0].press.button;

    for (u8 i = 0; i < buttonMax; i++) {
        if (!CHECK_BTN_ALL(pressedMask, partnerButtons[i])) continue;

        // C-button slot index = i + 1 (slot 0 is B-button = sword).
        if ((s32)(i + 1) >= (s32)ARRAY_COUNT(gSaveContext.save.saveInfo.equips.buttonItems)) break;
        u8 equipped = gSaveContext.save.saveInfo.equips.buttonItems[i + 1];

        s32 arrowType = Sw97_CuccoEgg_ItemToParams(equipped);
        if (arrowType < 0) continue;

        Sw97_CuccoEgg_Spawn(play, player, (s16)arrowType);
    }
}

void Sw97_TickCuccoMode(PlayState* play, Player* player) {
    // ─── PENDING: wait for first-person aim CS to end ───────────────────
    // magic_soul.inc.c:117 pattern — defer activation until the player has
    // left PLAYER_STATE1_IN_ITEM_CS. Activating mid-aim leaves the camera
    // setting stuck in first-person mode and the next setting change
    // glitches the angle.
    if (gSw97CuccoModePending && player != NULL && play != NULL) {
        if (!(player->stateFlags1 & PLAYER_STATE1_IN_ITEM_CS)) {
            Sw97_ActivateCuccoMode();
            // Flash + sound on actual entry (mirrors magic_soul's flash
            // before kill on line 123).
            func_800AA000(200.0f, 150, 20, 80);
            Actor_PlaySfx(&player->actor, NA_SE_EV_CHICKEN_CRY_M);
        }
    }

    // ─── INACTIVE: cleanup ──────────────────────────────────────────────
    // The VB_PLAYER_DRAW_BEGIN hook in customequipment.cpp checks
    // Sw97_IsCuccoModeActive() each frame, so just deactivating the flag
    // is enough — no draw swap to undo. We only do the entry-exit flash
    // once via gSw97CuccoExitFx.
    if (!gSw97CuccoModeActive) {
        if (gSw97CuccoExitFx && player != NULL) {
            gSw97CuccoExitFx = 0;
            player->invincibilityTimer = 20;
            sCuccoSkelInited = 0;
            Actor_PlaySfx(&player->actor, NA_SE_EV_CHICKEN_CRY_M);
            func_800AA000(200.0f, 150, 20, 80);
        }
        gSw97CuccoLastScene = -1;
        return;
    }

    if (--gSw97CuccoModeTimer <= 0) {
        Sw97_EndCuccoMode();
        return;
    }

    if (player == NULL || play == NULL) return;

    // Scene change → skel seg pointers reference the old scene's gfxCtx; re-init.
    if (gSw97CuccoLastScene >= 0 && gSw97CuccoLastScene != play->sceneNum) {
        sCuccoSkelInited = 0;
    }
    gSw97CuccoLastScene = play->sceneNum;


    // ─── First-frame setup: init the cucco skel ─────────────────────────
    // We do NOT swap player->actor.draw — the customequipment.cpp
    // VB_PLAYER_DRAW_BEGIN hook detects Sw97_IsCuccoModeActive() and routes
    // through Sw97_DrawCuccoForm, which walks Link's skeleton (null body) to
    // keep shadow + Navi tracking, then draws the cucco model on top.
    if (!sCuccoSkelInited) {
        Sw97_InitCuccoSkel(play);
    }

    // ─── Ivan-style: do NOT disable input/colliders, do NOT zero velocity
    // and do NOT do manual position math. Vanilla Player movement runs
    // normally — sword swings, walking anim, item C-buttons, doors, ladders,
    // collision — and we only TWEAK the physics quantities the engine
    // already produced.

    // 1) Cucco fall: clamp downward velocity to terminal float speed.
    // The engine added vanilla gravity (~-7) into velocity.y this frame;
    // clipping it to -3 makes Link float instead of plummet, without
    // touching `actor.gravity` (which gets stomped each frame by
    // Player_StepHorizontalSpeed @ z_player.c:7870 anyway).
    if (player->actor.velocity.y < CUCCO_MAX_VY_DOWN) {
        player->actor.velocity.y = CUCCO_MAX_VY_DOWN;
    }

    // 2) Cucco speed: small horizontal boost over vanilla.
    player->speedXZ *= CUCCO_SPEED_MULT;
    player->actor.speed  *= CUCCO_SPEED_MULT;
    if (player->speedXZ > CUCCO_SPEED_MAX) {
        player->speedXZ = CUCCO_SPEED_MAX;
    }
    if (player->actor.speed > CUCCO_SPEED_MAX) {
        player->actor.speed = CUCCO_SPEED_MAX;
    }

    // 3) A press while airborne → flap burst (Flappy Bird).
    u8 grounded = (player->actor.bgCheckFlags & BGCHECKFLAG_GROUND) != 0;
    u8 aPress   = CHECK_BTN_ALL(play->state.input[0].press.button, BTN_A);
    if (aPress && !grounded) {
        player->actor.velocity.y = CUCCO_FLAP_VELOCITY;
        Actor_PlaySfx(&player->actor, NA_SE_EV_CHICKEN_CRY_A);
    }

    // ─── Eggs: C-button press fires an elemental arrow forward ──────────
    Sw97_TickCuccoEggs(play, player);

    // ─── Cucco wing animation rate, responsive to Link's state ──────────
    // Walking on ground → moderate flap (1.5×). Airborne moving → rapid flap
    // (3.0×). Airborne hover → 2.0×. Idle on ground → slow idle (0.7×).
    f32 hSpeed = fabsf(player->speedXZ);
    f32 rate;
    if (!grounded) {
        rate = (hSpeed > 1.5f) ? 3.0f : 2.0f;
    } else {
        rate = (hSpeed > 1.5f) ? 1.5f : 0.7f;
    }
    sCuccoSkel.playSpeed = rate;
    SkelAnime_Update(&sCuccoSkel);

    // Tick down the 180° flip timer used by Sw97_DrawCucco on egg throws.
    if (gSw97CuccoFlipTimer > 0) gSw97CuccoFlipTimer--;
}
