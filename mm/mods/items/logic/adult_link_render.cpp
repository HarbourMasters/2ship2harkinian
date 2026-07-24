/*
 * adult_link_render.cpp - render OoT adult Link over MM's human Link (Skijer's NEI).
 * See adult_link_render.h for the design.
 *
 * The skeleton is OoT's gLinkAdultSkel loaded pre-parsed from the un-indexed oot.o2r via
 * MmAssets_LoadFromOotArchive; each limb DL is deep-patched via OotAssets_LoadGfxDirect so its
 * vtx/tex are inlined and resolve at draw time. We draw through the ENGINE's real Player_DrawImpl +
 * Player_PostLimbDrawGameplay so held items (sword, shield, bow, hookshot, ...), trails, colliders
 * and the shield/hookshot logic all work — but with a CUSTOM overrideLimbDraw that wraps the vanilla
 * Player_OverrideLimbDrawGameplayDefault and re-points the four equipment limbs (both hands, the
 * sheath, the waist) to OoT-ADULT DLs chosen from player->leftHandType/rightHandType/sheath state.
 * (For the Human form the vanilla override reads hardcoded MM-child hand arrays, so repointing
 * player->*HandDLists alone is NOT enough — the override callback is the clean seam.)
 *
 * New .cpp -> registered in build/x64/mm/2ship.vcxproj directly (do NOT run cmake).
 */

#include "adult_link_render.h"

#include <libultraship/bridge.h>
#include <spdlog/spdlog.h>
#include <math.h>

extern "C" {
#include "macros.h"
#include "functions.h"
#include "variables.h"
#include "z64lib.h"                                 // Lib_SegmentedToVirtual
#include "mods/oot_asset_loader/oot_asset_loader.h" // OotAssets_LoadGfxDirect / LoadTexOrDList / Available
#include "mods/nei_save.h"                          // Nei_Save()->timeGateAdultMode

// Loads an OoT resource already-parsed straight off the oot.o2r archive handle, bypassing the
// (rejected) global index. Defined in mods/transformation_masks/assets/mm_asset_loader.cpp.
void* MmAssets_LoadFromOotArchive(const char* path, size_t* outSize);

// MM per-form eye/mouth texture tables that Player_DrawImpl binds to segments 0x08/0x09. We swap the
// Human slots to the OoT-adult textures for the draw so the adult head samples the right eyes.
extern TexturePtr sPlayerEyesTextures[PLAYER_FORM_MAX][PLAYER_EYES_MAX];
extern TexturePtr sPlayerMouthTextures[PLAYER_FORM_MAX][PLAYER_MOUTH_MAX];
}

#define ADULT_LINK_SCALE_DEFAULT 0.01f
#define ADULT_LINK_COLLIDER_HEIGHT_DEFAULT 90
#define ADULT_LINK_LIMB_COUNT 21

// OTR path helper (all adult DLs live under this object).
#define ALB(sym) "__OTR__objects/object_link_boy/" sym

// Limb index -> near display list OTR path. NULL = control bone (no geometry) -> drawn invisible.
static const char* sLimbNearDL[ADULT_LINK_LIMB_COUNT] = {
    /* 0  Root          */ NULL,
    /* 1  Waist         */ ALB("gLinkAdultWaistNearDL"),
    /* 2  LowerControl  */ NULL,
    /* 3  RightThigh    */ ALB("gLinkAdultRightThighNearDL"),
    /* 4  RightLeg      */ ALB("gLinkAdultRightLegNearDL"),
    /* 5  RightFoot     */ ALB("gLinkAdultRightFootNearDL"),
    /* 6  LeftThigh     */ ALB("gLinkAdultLeftThighNearDL"),
    /* 7  LeftLeg       */ ALB("gLinkAdultLeftLegNearDL"),
    /* 8  LeftFoot      */ ALB("gLinkAdultLeftFootNearDL"),
    /* 9  UpperControl  */ NULL,
    /* 10 Head          */ ALB("gLinkAdultHeadNearDL"),
    /* 11 Hat           */ ALB("gLinkAdultHatNearDL"),
    /* 12 Collar        */ ALB("gLinkAdultCollarNearDL"),
    /* 13 LeftShoulder  */ ALB("gLinkAdultLeftShoulderNearDL"),
    /* 14 LeftArm       */ ALB("gLinkAdultLeftArmNearDL"),
    /* 15 LeftHand      */ ALB("gLinkAdultLeftHandNearDL"),
    /* 16 RightShoulder */ ALB("gLinkAdultRightShoulderNearDL"),
    /* 17 RightArm      */ ALB("gLinkAdultRightArmNearDL"),
    /* 18 RightHand     */ ALB("gLinkAdultRightHandNearDL"),
    /* 19 SwordAndSheath*/ NULL, // driven by the override at PLAYER_LIMB_SHEATH instead
    /* 20 Torso         */ ALB("gLinkAdultTorsoNearDL"),
};

// ---- one-time-loaded state ----
static FlexSkeletonHeader* sSkel = NULL;
static void* sEyeTex = NULL;
static void* sMouthTex = NULL;
static u8 sReady = 0;
static u8 sSetupFailedPermanently = 0;

// Equipment DLs chosen at draw time by the override (deep-patched adult DLs).
static Gfx* sDL_LHOpen;
static Gfx* sDL_LHClosed;
static Gfx* sDL_LHSword;  // holding one-hand sword (Master Sword)
static Gfx* sDL_LHBgs;    // two-hand (Biggoron/Great Fairy's)
static Gfx* sDL_RHOpen;
static Gfx* sDL_RHClosed;
static Gfx* sDL_RHShield; // shield raised in hand (Hylian)
static Gfx* sDL_RHBow;
static Gfx* sDL_RHOcarina;
static Gfx* sDL_RHHookshot;
static Gfx* sDL_Waist;
static Gfx* sDL_SheathEmpty;   // both sword & shield off the back
static Gfx* sDL_SheathSword;   // sword on back only (shield raised)
static Gfx* sDL_SheathShield;  // shield on back only (sword drawn)
static Gfx* sDL_SheathBoth;    // sword + shield on back (idle)

static Gfx* AdultLink_LoadDL(const char* otrPath) {
    Gfx* dl = (Gfx*)OotAssets_LoadGfxDirect(otrPath);
    if (dl == NULL) {
        SPDLOG_WARN("[AdultLink] equip DL load FAILED: {}", otrPath);
    }
    return dl;
}

static s32 AdultLink_Setup(void) {
    if (sReady) {
        return 1;
    }
    if (sSetupFailedPermanently) {
        return 0;
    }

    // Do NOT gate on OotAssets_Available(): oot.o2r mounted via the BenPort path does not populate
    // sOotArchives, so Available() reads 0. MmAssets_LoadFromOotArchive self-recovers.
    FlexSkeletonHeader* skel = (FlexSkeletonHeader*)MmAssets_LoadFromOotArchive("objects/object_link_boy/gLinkAdultSkel", NULL);
    if (skel == NULL) {
        static u8 logged = 0;
        if (!logged) {
            logged = 1;
            SPDLOG_WARN("[AdultLink] MmAssets_LoadFromOotArchive(gLinkAdultSkel) returned NULL — retrying");
        }
        return 0;
    }

    SPDLOG_INFO("[AdultLink] loaded gLinkAdultSkel {} -> limbCount={} dListCount={}", (void*)skel,
                (int)skel->sh.limbCount, (int)skel->dListCount);

    if (skel->sh.limbCount != ADULT_LINK_LIMB_COUNT) {
        sSetupFailedPermanently = 1;
        SPDLOG_ERROR("[AdultLink] unexpected limbCount {} (expected {}) — adult mode disabled",
                     (int)skel->sh.limbCount, ADULT_LINK_LIMB_COUNT);
        return 0;
    }

    int loaded = 0, failed = 0;
    for (int i = 0; i < ADULT_LINK_LIMB_COUNT; i++) {
        LodLimb* limb = (LodLimb*)skel->sh.segment[i];
        if (limb == NULL) {
            continue;
        }
        const char* nearPath = sLimbNearDL[i];
        if (nearPath == NULL) {
            limb->dLists[0] = NULL;
            limb->dLists[1] = NULL;
            continue;
        }
        Gfx* dl = (Gfx*)OotAssets_LoadGfxDirect(nearPath);
        limb->dLists[0] = dl;
        limb->dLists[1] = dl;
        (dl != NULL) ? loaded++ : failed++;
    }

    // Equipment DLs the override selects from.
    sDL_LHOpen = AdultLink_LoadDL(ALB("gLinkAdultLeftHandNearDL"));
    sDL_LHClosed = AdultLink_LoadDL(ALB("gLinkAdultLeftHandClosedNearDL"));
    sDL_LHSword = AdultLink_LoadDL(ALB("gLinkAdultLeftHandHoldingMasterSwordNearDL"));
    sDL_LHBgs = AdultLink_LoadDL(ALB("gLinkAdultLeftHandHoldingBgsNearDL"));
    sDL_RHOpen = AdultLink_LoadDL(ALB("gLinkAdultRightHandNearDL"));
    sDL_RHClosed = AdultLink_LoadDL(ALB("gLinkAdultRightHandClosedNearDL"));
    sDL_RHShield = AdultLink_LoadDL(ALB("gLinkAdultRightHandHoldingHylianShieldNearDL"));
    sDL_RHBow = AdultLink_LoadDL(ALB("gLinkAdultRightHandHoldingBowNearDL"));
    sDL_RHOcarina = AdultLink_LoadDL(ALB("gLinkAdultRightHandHoldingOotNearDL"));
    sDL_RHHookshot = AdultLink_LoadDL(ALB("gLinkAdultRightHandHoldingHookshotNearDL"));
    sDL_Waist = AdultLink_LoadDL(ALB("gLinkAdultWaistNearDL"));
    sDL_SheathEmpty = AdultLink_LoadDL(ALB("gLinkAdultSheathNearDL"));
    sDL_SheathSword = AdultLink_LoadDL(ALB("gLinkAdultMasterSwordAndSheathNearDL"));
    sDL_SheathShield = AdultLink_LoadDL(ALB("gLinkAdultHylianShieldAndSheathNearDL"));
    sDL_SheathBoth = AdultLink_LoadDL(ALB("gLinkAdultHylianShieldSwordAndSheathNearDL"));

    // Eye/mouth textures MUST come off oot.o2r too (OotAssets_LoadTexOrDList uses the un-reaching
    // normal index). MmAssets_LoadFromOotArchive is archive-scoped.
    sEyeTex = MmAssets_LoadFromOotArchive("objects/object_link_boy/gLinkAdultEyesOpenTex", NULL);
    sMouthTex = MmAssets_LoadFromOotArchive("objects/object_link_boy/gLinkAdultMouth1Tex", NULL);

    sSkel = skel;
    sReady = 1;
    SPDLOG_INFO("[AdultLink] setup OK: {} limb DLs loaded, {} failed; eyeTex={} mouthTex={}", loaded, failed,
                (void*)sEyeTex, (void*)sMouthTex);
    return 1;
}

// Custom limb-draw override: run the vanilla logic (matrices, body-part tracking, hand-type caching,
// leg adjust, upper-limb rot) then re-point the four equipment limbs to adult DLs. Everything the
// PostLimb pass keys off (player->*Type) is untouched, so trails/colliders/reticle still work.
static s32 AdultLink_OverrideLimb(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, Actor* actor) {
    s32 ret = Player_OverrideLimbDrawGameplayDefault(play, limbIndex, dList, pos, rot, actor);
    Player* p = (Player*)actor;

    switch (limbIndex) {
        case PLAYER_LIMB_WAIST:
            if (sDL_Waist != NULL) {
                *dList = sDL_Waist;
            }
            break;

        case PLAYER_LIMB_LEFT_HAND: {
            Gfx* d = sDL_LHOpen;
            switch (p->leftHandType) {
                case PLAYER_MODELTYPE_LH_CLOSED:
                    d = sDL_LHClosed;
                    break;
                case PLAYER_MODELTYPE_LH_ONE_HAND_SWORD:
                    d = sDL_LHSword;
                    break;
                case PLAYER_MODELTYPE_LH_TWO_HAND_SWORD:
                    d = sDL_LHBgs;
                    break;
                default:
                    break; // open / bottle / etc. -> open hand
            }
            if (d != NULL) {
                *dList = d;
            }
            break;
        }

        case PLAYER_LIMB_RIGHT_HAND: {
            Gfx* d = sDL_RHOpen;
            switch (p->rightHandType) {
                case PLAYER_MODELTYPE_RH_CLOSED:
                    d = sDL_RHClosed;
                    break;
                case PLAYER_MODELTYPE_RH_SHIELD:
                    d = sDL_RHShield;
                    break;
                case PLAYER_MODELTYPE_RH_BOW:
                    d = sDL_RHBow;
                    break;
                case PLAYER_MODELTYPE_RH_INSTRUMENT:
                    d = sDL_RHOcarina;
                    break;
                case PLAYER_MODELTYPE_RH_HOOKSHOT:
                    d = sDL_RHHookshot;
                    break;
                default:
                    break; // open / closed handled above
            }
            if (d != NULL) {
                *dList = d;
            }
            break;
        }

        case PLAYER_LIMB_SHEATH: {
            u8 swordDrawn = (p->leftHandType == PLAYER_MODELTYPE_LH_ONE_HAND_SWORD ||
                             p->leftHandType == PLAYER_MODELTYPE_LH_TWO_HAND_SWORD);
            u8 shieldRaised = (p->rightHandType == PLAYER_MODELTYPE_RH_SHIELD);
            Gfx* d;
            if (swordDrawn && shieldRaised) {
                d = sDL_SheathEmpty; // both in hand -> nothing on back
            } else if (shieldRaised) {
                d = sDL_SheathSword; // shield in hand -> sword on back
            } else if (swordDrawn) {
                d = sDL_SheathShield; // sword in hand -> shield on back
            } else {
                d = sDL_SheathBoth; // idle -> sword + shield on back
            }
            if (d != NULL) {
                *dList = d;
            }
            break;
        }
    }
    return ret;
}

extern "C" s32 AdultLink_IsActive(void) {
    // gAdultLink.ForceOn (console) forces adult mode ON regardless of the Time Gate, for isolated tests.
    return Nei_Save()->timeGateAdultMode != 0 || CVarGetInteger("gAdultLink.ForceOn", 0) != 0;
}

extern "C" void AdultLink_Toggle(void) {
    NeiSaveData* n = Nei_Save();
    n->timeGateAdultMode = n->timeGateAdultMode ? 0 : 1;
    SPDLOG_INFO("[AdultLink] toggle -> adult mode = {}", (int)n->timeGateAdultMode);
    if (n->timeGateAdultMode) {
        AdultLink_Setup();
    }
}

extern "C" s32 AdultLink_ShouldHide(void) {
    if (!AdultLink_IsActive()) {
        return 0;
    }
    return AdultLink_Setup();
}

extern "C" void AdultLink_Draw(PlayState* play, Player* player) {
    if (!sReady || sSkel == NULL) {
        return;
    }

    static u8 loggedDraw = 0;
    if (!loggedDraw) {
        loggedDraw = 1;
        SPDLOG_INFO("[AdultLink] first AdultLink_Draw (engine path) at pos ({},{},{})", player->actor.world.pos.x,
                    player->actor.world.pos.y, player->actor.world.pos.z);
    }

    // Render-state + Kokiri-green tunic ENV color (OoT adult tints tunic/cap/collar from env; without
    // it they render black). Player_DrawImpl opens its own DISPS and appends after these commands.
    {
        OPEN_DISPS(play->state.gfxCtx);
        Gfx_SetupDL25_Opa(play->state.gfxCtx);
        gSPSegment(POLY_OPA_DISP++, 0x0C, (uintptr_t)gCullBackDList);
        u8 tr = (u8)CVarGetInteger("gAdultLink.TunicR", 30);
        u8 tg = (u8)CVarGetInteger("gAdultLink.TunicG", 105);
        u8 tb = (u8)CVarGetInteger("gAdultLink.TunicB", 27);
        gDPSetEnvColor(POLY_OPA_DISP++, tr, tg, tb, 255);
        CLOSE_DISPS(play->state.gfxCtx);
    }

    f32 scale = CVarGetFloat("gAdultLink.Scale", ADULT_LINK_SCALE_DEFAULT);
    f32 yOff = CVarGetFloat("gAdultLink.YOffset", 0.0f);

    Matrix_Translate(player->actor.world.pos.x, player->actor.world.pos.y + yOff, player->actor.world.pos.z,
                     MTXMODE_NEW);
    Matrix_RotateYS(player->actor.shape.rot.y, MTXMODE_APPLY);
    Matrix_Scale(scale, scale, scale, MTXMODE_APPLY);

    // Neutralize the Human age scale so the adult root sits at its animation-native height (the shared
    // ageProperties table is restored right after — same save/mutate/restore pattern as item_minish_cap.c).
    f32 savedAgeScale = 0.0f;
    if (player->ageProperties != NULL) {
        savedAgeScale = player->ageProperties->unk_08;
        player->ageProperties->unk_08 = 1.0f;
    }

    // Point the Human eye/mouth arrays at the adult textures for this draw (Player_DrawImpl binds them
    // to segments 0x08/0x09). Restore afterward so nothing else is affected.
    TexturePtr savedEyes[PLAYER_EYES_MAX];
    TexturePtr savedMouth[PLAYER_MOUTH_MAX];
    for (int i = 0; i < PLAYER_EYES_MAX; i++) {
        savedEyes[i] = sPlayerEyesTextures[PLAYER_FORM_HUMAN][i];
        if (sEyeTex != NULL) {
            sPlayerEyesTextures[PLAYER_FORM_HUMAN][i] = (TexturePtr)sEyeTex;
        }
    }
    for (int i = 0; i < PLAYER_MOUTH_MAX; i++) {
        savedMouth[i] = sPlayerMouthTextures[PLAYER_FORM_HUMAN][i];
        if (sMouthTex != NULL) {
            sPlayerMouthTextures[PLAYER_FORM_HUMAN][i] = (TexturePtr)sMouthTex;
        }
    }

    // Draw through the real engine path so held items / trails / colliders all work; our override
    // supplies adult equipment DLs. Feed MM's OWN jointTable (identical 21-limb hierarchy) 1:1.
    Player_DrawImpl(play, sSkel->sh.segment, player->skelAnime.jointTable, sSkel->dListCount, 0, PLAYER_FORM_HUMAN,
                    player->currentBoots, player->actor.shape.face, AdultLink_OverrideLimb,
                    Player_PostLimbDrawGameplay, &player->actor);

    for (int i = 0; i < PLAYER_EYES_MAX; i++) {
        sPlayerEyesTextures[PLAYER_FORM_HUMAN][i] = savedEyes[i];
    }
    for (int i = 0; i < PLAYER_MOUTH_MAX; i++) {
        sPlayerMouthTextures[PLAYER_FORM_HUMAN][i] = savedMouth[i];
    }
    if (player->ageProperties != NULL) {
        player->ageProperties->unk_08 = savedAgeScale;
    }
}

extern "C" void AdultLink_UpdateCollider(Player* player) {
    if (!AdultLink_IsActive() || !sReady) {
        return;
    }
    player->cylinder.dim.height = (s16)CVarGetInteger("gAdultLink.ColliderHeight", ADULT_LINK_COLLIDER_HEIGHT_DEFAULT);
}
