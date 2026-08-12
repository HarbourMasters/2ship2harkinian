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
#include <string>
#include <cstring>
#include <cstdlib>

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

// Alt-assets (mod / texture-pack) resolution so appearance mods can override the adult model. BenPort.
// ResourceMgr_LoadGfxByName dereferences a null resource -> only call it once FileExists confirms the
// "alt/" path is present.
bool ResourceMgr_IsAltAssetsEnabled(void);
uint8_t ResourceMgr_FileExists(const char* path);
Gfx* ResourceMgr_LoadGfxByName(const char* path);
// Loads a skeleton by name; with alt-assets on it resolves the "alt/" (mod) version first. Returns the
// header or NULL (e.g. an un-indexed oot.o2r-only base with no mod present). See HarpoonDummyPlayer.
SkeletonHeader* ResourceMgr_LoadSkeletonByName(const char* path, SkelAnime* skelAnime);
char* ResourceMgr_LoadTexOrDListByName(const char* path); // load an indexed texture (e.g. a mod's alt/ eye tex)
char** ResourceMgr_ListFiles(const char* searchMask, int* resultSize); // enumerate archive files (for mod discovery)

// MM per-form eye/mouth texture tables that Player_DrawImpl binds to segments 0x08/0x09. We swap the
// Human slots to the OoT-adult textures for the draw so the adult head samples the right eyes.
extern TexturePtr sPlayerEyesTextures[PLAYER_FORM_MAX][PLAYER_EYES_MAX];
extern TexturePtr sPlayerMouthTextures[PLAYER_FORM_MAX][PLAYER_MOUTH_MAX];

// NEI custom-item in-hand draw dispatcher (beetle, cane of somaria, ball&chain, ...). In the vanilla
// path it runs AFTER the skeleton draw (which we skip via the early return), reading the world-space
// hand positions the skeleton draw wrote. custom_items.h:611, extern "C".
s32 CustomItems_OverrideDraw(Player* player, PlayState* play);
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
static u8 sReady = 0;
static u8 sSetupFailedPermanently = 0;
static u8 sIsMod = 0; // 1 = a user alt-assets mod supplied the skeleton (full model replacement)

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

// A MOD head mesh binds its own eye/mouth via segments 0x08/0x09 (recomp/Fast64 preserves the vanilla
// structure). We DISCOVER the mod's eye/mouth textures generically by matching each blink state as a
// substring of the mod's alt-object filenames — so any exporter's suffix works, not just one mod. The
// state order maps to MM's PlayerEyes index (open/half/closed used for blinking; the rest positional).
// Canonical OoT adult-Link eye/mouth resource names (object_link_boy). A mod that ships gLinkAdultSkel
// almost always ships these under the same object with the same names, so we can load each slot by NAME
// directly — robust across archive formats (the new .o2r doesn't match a ResourceMgr_ListFiles wildcard,
// which returned 0 and left every slot on the MM-child eye = garbage on the adult head).
static const char* sOotEyeName[PLAYER_EYES_MAX] = {
    "gLinkAdultEyesOpenTex",     "gLinkAdultEyesHalfTex",      "gLinkAdultEyesClosedfTex",
    "gLinkAdultEyesRollLeftTex", "gLinkAdultEyesRollRightTex", "gLinkAdultEyesShockTex",
    "gLinkAdultEyesUnk1Tex",     "gLinkAdultEyesUnk2Tex",
};
static const char* sOotMouthName[PLAYER_MOUTH_MAX] = {
    "gLinkAdultMouth1Tex", "gLinkAdultMouth2Tex", "gLinkAdultMouth3Tex", "gLinkAdultMouth4Tex",
};
// sPlayerEyesTextures holds OTR PATH STRINGS (e.g. "__OTR__objects/.../gLinkHumanEyesOpenTex"), which
// the gfx interpreter resolves at draw time — NOT raw pixel pointers. So we store the mod eye/mouth
// PATHS (with the __OTR__ prefix) and hand those to Player_DrawImpl, letting it resolve the mod texture.
static std::string sModEyePath[PLAYER_EYES_MAX];
static std::string sModMouthPath[PLAYER_MOUTH_MAX];
// RAW pixel pointers for the mod eye/mouth. We bind these directly on segments 0x08/0x09 instead of a
// path string: MM's own eye PATHS resolve only because their base is indexed; a mod's texture exists
// ONLY as alt/... (no indexed base), and the interpreter's GetBaseTexturePath strips the "alt/" ->
// un-indexed base -> fails. Raw pixels (gfx_check_image_signature sees non-__OTR__ data) bypass all of
// that — exactly how a zobj/ML64 swap feeds the eye texels from the object.
static void* sModEye[PLAYER_EYES_MAX];
static void* sModMouth[PLAYER_MOUTH_MAX];

// Load an adult-Link DL, preferring a user mod / texture-pack: if the "alt/" override of this resource
// is present (indexed), use it so appearance mods apply; otherwise deep-patch the oot.o2r base (which
// the alt-assets index can't reach). Only mod DLs that ship the FULL display list get overridden here;
// texture-only packs don't apply because the deep-patch inlines oot.o2r's base textures.
static Gfx* AdultLink_LoadDL(const char* otrPath) {
    const char* p = otrPath;
    if (std::strncmp(p, "__OTR__", 7) == 0) {
        p += 7;
    }
    if (ResourceMgr_IsAltAssetsEnabled()) {
        std::string altPath = std::string("alt/") + p; // gAltAssetPrefix == "alt/"
        if (ResourceMgr_FileExists(altPath.c_str())) {
            Gfx* mod = ResourceMgr_LoadGfxByName(altPath.c_str());
            if (mod != NULL) {
                return mod;
            }
        }
    }
    Gfx* dl = (Gfx*)OotAssets_LoadGfxDirect(otrPath);
    if (dl == NULL) {
        SPDLOG_WARN("[AdultLink] DL load FAILED: {}", otrPath);
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

    // Prefer a user MOD / alt-assets skeleton — a full appearance replacement (e.g. a recomp/Fast64
    // model whose .otr in the mods folder ships alt/objects/object_link_boy/gLinkAdultSkel + its own
    // limb meshes/textures). ResourceMgr_LoadSkeletonByName resolves the alt/ version through the normal
    // index; its limbs already reference the mod's indexed meshes, so nothing needs deep-patching. When
    // no mod is present it returns NULL (the oot.o2r base isn't indexed) and we deep-patch the base.
    FlexSkeletonHeader* skel = NULL;
    if (ResourceMgr_IsAltAssetsEnabled()) {
        skel = (FlexSkeletonHeader*)ResourceMgr_LoadSkeletonByName("objects/object_link_boy/gLinkAdultSkel", NULL);
    }
    sIsMod = (skel != NULL) ? 1 : 0;

    if (!sIsMod) {
        // Base skeleton from the un-indexed oot.o2r (self-recovering archive-scoped load).
        skel = (FlexSkeletonHeader*)MmAssets_LoadFromOotArchive("objects/object_link_boy/gLinkAdultSkel", NULL);
        if (skel == NULL) {
            static u8 logged = 0;
            if (!logged) {
                logged = 1;
                SPDLOG_WARN("[AdultLink] gLinkAdultSkel not found (mod alt + oot.o2r) — retrying");
            }
            return 0;
        }
    }

    SPDLOG_INFO("[AdultLink] skeleton {} (mod={}) -> limbCount={} dListCount={}", (void*)skel, (int)sIsMod,
                (int)skel->sh.limbCount, (int)skel->dListCount);

    if (skel->sh.limbCount != ADULT_LINK_LIMB_COUNT) {
        sSetupFailedPermanently = 1;
        SPDLOG_ERROR("[AdultLink] unexpected limbCount {} (expected {}) — adult mode disabled",
                     (int)skel->sh.limbCount, ADULT_LINK_LIMB_COUNT);
        return 0;
    }

    if (!sIsMod) {
        // Base skeleton: its limb DLs point at un-indexed oot.o2r assets — swap each for a self-contained
        // deep-patched Gfx*. (A mod skeleton's limbs already point at indexed mod meshes — leave them.)
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
            Gfx* dl = AdultLink_LoadDL(nearPath);
            limb->dLists[0] = dl;
            limb->dLists[1] = dl;
            (dl != NULL) ? loaded++ : failed++;
        }
        // Eye/mouth off oot.o2r as RAW texels for ALL 8 eye + 4 mouth slots. The deep-patched base head DL
        // still references segments 0x08/0x09 (OoT's adult head, like MM, samples the eye/mouth from those
        // segments), so we MUST bind an adult eye there — otherwise the segments keep MM's CHILD eye/mouth
        // and the adult face renders garbled. Bound at draw time (see AdultLink_Draw).
        int nEye = 0, nMouth = 0;
        for (int i = 0; i < PLAYER_EYES_MAX; i++) {
            std::string base = std::string("objects/object_link_boy/") + sOotEyeName[i];
            sModEye[i] = MmAssets_LoadFromOotArchive(base.c_str(), NULL);
            if (sModEye[i] != NULL) {
                nEye++;
            }
        }
        for (int i = 0; i < PLAYER_MOUTH_MAX; i++) {
            std::string base = std::string("objects/object_link_boy/") + sOotMouthName[i];
            sModMouth[i] = MmAssets_LoadFromOotArchive(base.c_str(), NULL);
            if (sModMouth[i] != NULL) {
                nMouth++;
            }
        }
        SPDLOG_INFO("[AdultLink] base deep-patch: {} limb DLs loaded, {} failed; {}/{} eye, {}/{} mouth off oot.o2r",
                    loaded, failed, nEye, (int)PLAYER_EYES_MAX, nMouth, (int)PLAYER_MOUTH_MAX);
    } else {
        // Mod skeleton: load its eye/mouth textures BY CANONICAL NAME (the head materials bind them via
        // segments 0x08/0x09). We bind the BASE path on the segment and let the interpreter's alt-assets
        // redirect resolve it to the mod's alt/ texture — exactly how MM's own forms resolve their eyes,
        // and robust across archive formats (ResourceMgr_ListFiles returned 0 on the new .o2r format,
        // which left every slot on the MM-child eye). Any slot the mod lacks falls back to the oot.o2r
        // base adult eye (raw pixels) — a real adult eye, never the child garbage.
        int nEye = 0, nMouth = 0, firstEye = -1, firstMouth = -1;
        for (int i = 0; i < PLAYER_EYES_MAX; i++) {
            std::string altc = std::string("alt/objects/object_link_boy/") + sOotEyeName[i];
            if (ResourceMgr_FileExists(altc.c_str())) {
                // Bind the base path; LoadResourceProcess auto-redirects base -> alt (the mod texture).
                sModEyePath[i] = std::string("__OTR__objects/object_link_boy/") + sOotEyeName[i];
                if (firstEye < 0) {
                    firstEye = i;
                }
                nEye++;
            } else {
                // oot.o2r base adult eye as raw texels (un-indexed archive; bound directly on the segment).
                std::string base = std::string("objects/object_link_boy/") + sOotEyeName[i];
                sModEye[i] = MmAssets_LoadFromOotArchive(base.c_str(), NULL);
            }
        }
        for (int i = 0; i < PLAYER_MOUTH_MAX; i++) {
            std::string altc = std::string("alt/objects/object_link_boy/") + sOotMouthName[i];
            if (ResourceMgr_FileExists(altc.c_str())) {
                sModMouthPath[i] = std::string("__OTR__objects/object_link_boy/") + sOotMouthName[i];
                if (firstMouth < 0) {
                    firstMouth = i;
                }
                nMouth++;
            } else {
                std::string base = std::string("objects/object_link_boy/") + sOotMouthName[i];
                sModMouth[i] = MmAssets_LoadFromOotArchive(base.c_str(), NULL);
            }
        }
        // Any still-empty slot (no mod texture, no oot.o2r base) -> reuse the first slot the mod DID ship,
        // so no eye/mouth index can ever fall through to the MM-child texture.
        if (firstEye >= 0) {
            for (int i = 0; i < PLAYER_EYES_MAX; i++) {
                if (sModEyePath[i].empty() && sModEye[i] == NULL) {
                    sModEyePath[i] = sModEyePath[firstEye];
                }
            }
        }
        if (firstMouth >= 0) {
            for (int i = 0; i < PLAYER_MOUTH_MAX; i++) {
                if (sModMouthPath[i].empty() && sModMouth[i] == NULL) {
                    sModMouthPath[i] = sModMouthPath[firstMouth];
                }
            }
        }
        SPDLOG_INFO("[AdultLink] mod textures (by-name): {}/{} eye, {}/{} mouth from mod alt (rest -> oot.o2r base)",
                    nEye, (int)PLAYER_EYES_MAX, nMouth, (int)PLAYER_MOUTH_MAX);

        // GROUND-TRUTH probe: this mod's CI8 skin/face materials load their TLUT (palette) from OoT base
        // resources (object_link_boyTLUT_*, gLinkAdultHeadTLUT) that live in oot.o2r, NOT in the mod. In soh
        // those are indexed (base game); in 2ship they resolve ONLY if oot.o2r is truly in the name index.
        // If they DON'T resolve, every CI8 skin/face texture renders palette-less = gray skin + black face
        // (exactly the reported symptom). Log whether each resolves so we know if that's the cause.
        static const char* sProbe[] = {
            "objects/object_link_boy/object_link_boyTLUT_005800", // boots/gauntlets/shoe palette
            "objects/object_link_boy/gLinkAdultHeadTLUT",         // FACE-skin palette (adult_ear)
            "objects/object_link_boy/sAdultEyes",                 // eye palette (mod-shipped, should exist)
            "objects/object_link_boy/gLinkHumanSkinTLUT",         // skin palette (mod-shipped)
        };
        for (int i = 0; i < (int)(sizeof(sProbe) / sizeof(sProbe[0])); i++) {
            std::string base = std::string("__OTR__") + sProbe[i];
            std::string alt = std::string("__OTR__alt/") + sProbe[i];
            void* basePtr = ResourceMgr_FileExists(sProbe[i]) ? (void*)ResourceMgr_LoadTexOrDListByName(base.c_str())
                                                              : NULL;
            std::string altc = std::string("alt/") + sProbe[i];
            int altExists = (int)ResourceMgr_FileExists(altc.c_str());
            void* ootPtr = MmAssets_LoadFromOotArchive(sProbe[i], NULL);
            SPDLOG_INFO("[AdultLink] TLUT probe '{}': baseIndexed={} basePtr={} altInMod={} ootArchive={}", sProbe[i],
                        (int)ResourceMgr_FileExists(sProbe[i]), basePtr, altExists, ootPtr);
        }
    }

    // Equipment DLs the override swaps in for held weapons (both paths). A mod usually lacks holding
    // variants, so these fall to the oot.o2r base; AdultLink_LoadDL prefers a mod alt/ when present.
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

    sSkel = skel;
    sReady = 1;
    SPDLOG_INFO("[AdultLink] setup OK (mod={})", (int)sIsMod);
    return 1;
}

// Custom limb-draw override: run the vanilla logic (matrices, body-part tracking, hand-type caching,
// leg adjust, upper-limb rot) then re-point the four equipment limbs to adult DLs. Everything the
// PostLimb pass keys off (player->*Type) is untouched, so trails/colliders/reticle still work.
static s32 AdultLink_OverrideLimb(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, Actor* actor) {
    // Snapshot the skeleton's OWN limb DL BEFORE the vanilla override runs. For a mod skeleton these are
    // the mod's meshes (waist / hands / sword+sheath); for the base they're the deep-patched adult DLs.
    Gfx* skelDL = *dList;
    s32 ret = Player_OverrideLimbDrawGameplayDefault(play, limbIndex, dList, pos, rot, actor);
    Player* p = (Player*)actor;

    // The vanilla override CLOBBERS *dList for the waist/hands/sheath with MM-CHILD DLs (read from
    // player->waistDLists / leftHandDLists / ... — that's why child parts leaked through, gerudo has the
    // same issue). We RESTORE the skeleton's own mesh for those limbs, substituting an oot.o2r base DL
    // only when a WEAPON is actually held (a mod usually ships no in-hand holding variants).
    switch (limbIndex) {
        case PLAYER_LIMB_WAIST:
            *dList = skelDL; // adult/mod waist — never MM child
            break;

        case PLAYER_LIMB_LEFT_HAND:
            if (p->leftHandType == PLAYER_MODELTYPE_LH_ONE_HAND_SWORD && sDL_LHSword != NULL) {
                *dList = sDL_LHSword;
            } else if (p->leftHandType == PLAYER_MODELTYPE_LH_TWO_HAND_SWORD && sDL_LHBgs != NULL) {
                *dList = sDL_LHBgs;
            } else if (!sIsMod && p->leftHandType == PLAYER_MODELTYPE_LH_CLOSED && sDL_LHClosed != NULL) {
                *dList = sDL_LHClosed;
            } else {
                *dList = skelDL; // open / bottle / mod hand -> the skeleton's own hand
            }
            break;

        case PLAYER_LIMB_RIGHT_HAND:
            if (p->rightHandType == PLAYER_MODELTYPE_RH_SHIELD && sDL_RHShield != NULL) {
                *dList = sDL_RHShield;
            } else if (p->rightHandType == PLAYER_MODELTYPE_RH_BOW && sDL_RHBow != NULL) {
                *dList = sDL_RHBow;
            } else if (p->rightHandType == PLAYER_MODELTYPE_RH_INSTRUMENT && sDL_RHOcarina != NULL) {
                *dList = sDL_RHOcarina;
            } else if (p->rightHandType == PLAYER_MODELTYPE_RH_HOOKSHOT && sDL_RHHookshot != NULL) {
                *dList = sDL_RHHookshot;
            } else if (!sIsMod && p->rightHandType == PLAYER_MODELTYPE_RH_CLOSED && sDL_RHClosed != NULL) {
                *dList = sDL_RHClosed;
            } else {
                *dList = skelDL;
            }
            break;

        case PLAYER_LIMB_SHEATH:
            if (sIsMod) {
                *dList = skelDL; // the mod's own sword+sheath mesh on the back
            } else {
                // Base skeleton has no sheath mesh (limb 19 = NULL) — pick the back setup by state.
                u8 swordDrawn = (p->leftHandType == PLAYER_MODELTYPE_LH_ONE_HAND_SWORD ||
                                 p->leftHandType == PLAYER_MODELTYPE_LH_TWO_HAND_SWORD);
                u8 shieldRaised = (p->rightHandType == PLAYER_MODELTYPE_RH_SHIELD);
                Gfx* d = (swordDrawn && shieldRaised) ? sDL_SheathEmpty
                         : shieldRaised               ? sDL_SheathSword
                         : swordDrawn                 ? sDL_SheathShield
                                                      : sDL_SheathBoth;
                if (d != NULL) {
                    *dList = d;
                }
            }
            break;
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
    // Adult Link only REPLACES the Human form. When the player is transformed (Deku / Goron / Zora /
    // Fierce Deity via a mask), draw that form normally — otherwise the adult overlay is drawn over every
    // transformation and it looks like the mask "no me convierte" while adult mode is on.
    if (gSaveContext.save.playerForm != PLAYER_FORM_HUMAN) {
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

    // Render-state + tunic ENV color. OoT adult Link's tunic/cap/collar material is grayscale, TINTED by
    // the ENV color (a mod that exports OoT adult Link keeps this — its tunic goes BLACK without a green
    // ENV, confirmed). So set the Kokiri-green ENV for both vanilla and mod. Player_DrawImpl opens its own
    // DISPS and appends after these commands.
    {
        OPEN_DISPS(play->state.gfxCtx);
        Gfx_SetupDL25_Opa(play->state.gfxCtx);
        gSPSegment(POLY_OPA_DISP++, 0x0C, (uintptr_t)gCullBackDList);
        u8 tr = (u8)CVarGetInteger("gAdultLink.TunicR", 30);
        u8 tg = (u8)CVarGetInteger("gAdultLink.TunicG", 105);
        u8 tb = (u8)CVarGetInteger("gAdultLink.TunicB", 27);
        gDPSetEnvColor(POLY_OPA_DISP++, tr, tg, tb, 255);
        // DIAGNOSTIC (gAdultLink.FullBright): bind a full-white ambient light so every G_LIGHTING
        // surface renders at TEXEL brightness regardless of its normals. If the black FACE lights up
        // with this on, the bug is lighting/SHADE (normals or light binding in this overlay); if it
        // STAYS black, the eye/face TEXTURE itself is the problem (geometry/UV/segment), not lighting.
        if (CVarGetInteger("gAdultLink.FullBright", 0)) {
            static Lights1 sFullBright = gdSPDefLights1(255, 255, 255, 255, 255, 255, 0, 0, 120);
            gSPSetLights1(POLY_OPA_DISP++, sFullBright);
        }
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
    // For a mod, bind the mod's eye/mouth OTR PATH STRING on the segment — the EXACT proven mechanism MM's
    // own forms + soh's transformation_masks (Zora/Goron) use. gfx_set_timg_handler_rdp then LoadResource-
    // Process()es it and populates the FULL texture metadata (width/height/type/HByteScale/VPixelScale)
    // from the resource, whereas a raw pixel pointer leaves those at defaults (scale=1) — which corrupts
    // any HD/upscaled mod eye. A direct "__OTR__alt/..." path resolves fine (GetBaseTexturePath only
    // affects HD-blend lookup, not loading). Fall back to raw texels only if the path never resolved.
    // Bind adult eye/mouth on segments 0x08/0x09 for BOTH vanilla (oot.o2r base texels) and mod (alt path).
    // A mod slot prefers its OTR path (full metadata); vanilla uses the oot.o2r raw texels. Either way the
    // adult head never keeps MM's child eye/mouth.
    for (int i = 0; i < PLAYER_EYES_MAX; i++) {
        savedEyes[i] = sPlayerEyesTextures[PLAYER_FORM_HUMAN][i];
        if (!sModEyePath[i].empty()) {
            sPlayerEyesTextures[PLAYER_FORM_HUMAN][i] = (TexturePtr)sModEyePath[i].c_str(); // mod eye OTR path
        } else if (sModEye[i] != NULL) {
            sPlayerEyesTextures[PLAYER_FORM_HUMAN][i] = (TexturePtr)sModEye[i]; // oot.o2r base / raw texels
        }
    }
    for (int i = 0; i < PLAYER_MOUTH_MAX; i++) {
        savedMouth[i] = sPlayerMouthTextures[PLAYER_FORM_HUMAN][i];
        if (!sModMouthPath[i].empty()) {
            sPlayerMouthTextures[PLAYER_FORM_HUMAN][i] = (TexturePtr)sModMouthPath[i].c_str();
        } else if (sModMouth[i] != NULL) {
            sPlayerMouthTextures[PLAYER_FORM_HUMAN][i] = (TexturePtr)sModMouth[i];
        }
    }

    // One-shot ground-truth diagnostic: what actually gets bound for the eye the animation selected.
    {
        static u8 loggedFace = 0;
        if (!loggedFace) {
            loggedFace = 1;
            s32 ei = GET_EYE_INDEX_FROM_JOINT_TABLE(player->skelAnime.jointTable);
            if (ei < 0) {
                ei = 0;
            }
            if (ei >= PLAYER_EYES_MAX) {
                ei = 0;
            }
            SPDLOG_INFO("[AdultLink] FACE bind (mod={}): eyeIndex={} eyePath='{}' rawPtr={} headLimbDL={}",
                        (int)sIsMod, ei, sModEyePath[ei].empty() ? "(none)" : sModEyePath[ei].c_str(), sModEye[ei],
                        (void*)((LodLimb*)sSkel->sh.segment[10])->dLists[0]);
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

    // Feet shadow: the vanilla Player_Draw path (which our early return skips) seeds shape.feetPos, and
    // ActorShadow_DrawFeet (invoked from z_actor.c AFTER actor->draw) reads it — otherwise Link casts no
    // shadow. MM seeds both feet with the actor world pos (a single blob under Link); mirror that.
    Math_Vec3f_Copy(&player->actor.shape.feetPos[0], &player->actor.world.pos);
    Math_Vec3f_Copy(&player->actor.shape.feetPos[1], &player->actor.world.pos);

    // Held NEI custom items (beetle, cane of somaria, ball&chain, rods, ...) are drawn post-skeleton in
    // the vanilla path we skipped. bodyPartsPos/leftHandWorld/rightHandWorld were just filled by
    // Player_DrawImpl+PostLimb, so each custom item seats on the adult hands exactly like on child Link.
    // (Boomerang/Net already drew inside PostLimb — this does not double them.)
    CustomItems_OverrideDraw(player, play);
}

extern "C" void AdultLink_UpdateCollider(Player* player) {
    if (!AdultLink_IsActive() || !sReady) {
        return;
    }
    player->cylinder.dim.height = (s16)CVarGetInteger("gAdultLink.ColliderHeight", ADULT_LINK_COLLIDER_HEIGHT_DEFAULT);
}
