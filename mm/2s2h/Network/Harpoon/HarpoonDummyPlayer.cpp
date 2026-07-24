// =============================================================================
// HarpoonDummyPlayer — renders remote players as Link. Drawn from
// OnPlayDrawWorldEnd.
//
// Primary path — "recycled player": we keep a fully-valid Player template for
// every form the LOCAL player has visited this session (you always start Human,
// and each transform caches that form). To draw a peer in form F we copy the F
// template, override the networked fields (pos, rot, jointTable, weapon / shield
// / mask / boots) and draw through the real player path (Player_DrawImpl +
// Player_PostLimbDrawGameplay) — so equipment and masks render exactly like the
// engine, EVEN WHEN the peer's form differs from ours. The template's skeleton /
// ageProperties / DL-group pointers are engine statics that stay valid after we
// transform away. The matrix uses the same yOffset Actor_Draw applies
// (z_actor.c) so the model is grounded.
//
// Fallback (a form we've never been in): resolve that form's skeleton through
// the engine ResourceMgr (form skeletons are o2r resources, not raw segmented
// pointers) and draw the body with Player_DrawImpl (no postLimbDraw, per-form
// rootAnimScale grounding). Only the body shows. Rare in practice — Human is
// always visited and MM forms carry no OoT-style equipment to miss.
//
// Each remote uses a stable per-client struct so Fast3D frame interpolation
// smooths motion between 30Hz packets.
// =============================================================================

#include "Harpoon.h"
#include "2s2h/Enhancements/FrameInterpolation/FrameInterpolation.h"
#include <spdlog/spdlog.h>
#include <map>

extern "C" {
#include "z64.h"
#include "z64actor.h"
#include "z64player.h"
#include "z64scene.h"
#include "sys_matrix.h"
#include "macros.h"
#include "functions.h"
#include "variables.h"
#include "main.h"
extern PlayState* gPlayState;
extern SaveContext gSaveContext;
int ResourceMgr_OTRSigCheck(char* imgData);
SkeletonHeader* ResourceMgr_LoadSkeletonByName(const char* path, SkelAnime* skelAnime);
}

static std::map<uint32_t, Player> sDummyPlayers;  // recycled players (per remote)
static std::map<uint32_t, Actor> sCrossActors;    // cross-form fallback actors

// A fully-valid Player snapshot for every form the LOCAL player has visited this
// session. You always start as Human (so the Human template is always available)
// and each transform caches that form's template. This lets us draw a peer in a
// DIFFERENT form than ours through the real player path (equipment + mask via
// Player_PostLimbDrawGameplay) — the skeleton/ageProperties/DL-group pointers it
// holds are engine statics that stay valid after we transform away.
static Player sFormTemplate[PLAYER_FORM_MAX];
static bool sFormTemplateValid[PLAYER_FORM_MAX] = { false };

// Per-form root-bone scale that grounds a foreign-form skeleton (values copied
// 1:1 from SoH's HarpoonDummyPlayer rootAnimScale / mm_player_form sFormProps).
// Index by PlayerTransformation: FD, Goron, Zora, Deku, Human.
static const f32 sFormRootAnimScale[PLAYER_FORM_MAX] = { 1.5f, 0.74f, 1.0f, 0.3f, 1.0f };
static s32 sCurrentDrawForm = PLAYER_FORM_HUMAN;  // set before each cross-form draw

// Cross-form root grounding: scale the root limb translation by the drawn form's
// rootAnimScale (matches SoH's RemoteMmForm_OverrideLimbDraw). Without this the
// foreign-form skeleton floats.
static s32 HarpoonCrossOverride(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, Actor* actor) {
    if (limbIndex == PLAYER_LIMB_ROOT && sCurrentDrawForm >= 0 && sCurrentDrawForm < PLAYER_FORM_MAX) {
        f32 s = sFormRootAnimScale[sCurrentDrawForm];
        pos->x *= s;
        pos->y *= s;
        pos->z *= s;
    }
    return false;
}

// Per-form skeleton resolved via the engine resource manager (mirrors
// SkelAnime_InitPlayer). Pointers are stable once loaded → cache them.
static void** sFormSkel[PLAYER_FORM_MAX] = { 0 };
static s32 sFormDList[PLAYER_FORM_MAX] = { 0 };
static bool sFormTried[PLAYER_FORM_MAX] = { 0 };

static bool ResolveFormSkeleton(s32 form, void*** outSkel, s32* outDList) {
    if (!sFormTried[form]) {
        sFormTried[form] = true;
        FlexSkeletonHeader* hdrSeg = gPlayerSkeletons[form];
        if (hdrSeg != NULL) {
            if (ResourceMgr_OTRSigCheck((char*)hdrSeg) != 0) {
                hdrSeg = (FlexSkeletonHeader*)ResourceMgr_LoadSkeletonByName((const char*)hdrSeg, NULL);
            }
            if (hdrSeg != NULL) {
                FlexSkeletonHeader* hdr = (FlexSkeletonHeader*)Lib_SegmentedToVirtual(hdrSeg);
                if (hdr != NULL) {
                    sFormSkel[form] = (void**)Lib_SegmentedToVirtual(hdr->sh.segment);
                    sFormDList[form] = hdr->dListCount;
                }
            }
        }
    }
    if (sFormSkel[form] == NULL) return false;
    *outSkel = sFormSkel[form];
    *outDList = sFormDList[form];
    return true;
}

void HarpoonDummyPlayer_DrawAll(PlayState* play) {
    if (!play) return;
    auto* h = Harpoon::Instance();
    if (h->State() != HarpoonConnState::InRoom) return;

    Player* player = GET_PLAYER(play);
    if (player == NULL || player->skelAnime.skeleton == NULL) return;

    // While a (de)transformation is in progress, the target form
    // (gSaveContext.save.playerForm, read by FastTransformation to load the new
    // object) is already the NEW form while player->transformation is still the
    // OLD one until re-init. They differ ONLY during the transition window — the
    // exact frames where the form object/skelAnime are inconsistent and drawing
    // crashes the GPU. Skip those frames (this also covers the pre-`transformation`
    // frames a form-change check misses).
    if (GET_PLAYER_FORM != player->transformation) return;

    s16 myScene = play->sceneId;
    s32 myForm = player->transformation;
    if (myForm < 0 || myForm >= PLAYER_FORM_MAX) return;
    if (player->actor.objectSlot < 0 || !Object_IsLoaded(&play->objectCtx, player->actor.objectSlot)) {
        return;
    }

    // Extra settle window after the transformation finishes (skelAnime/object
    // may take a couple more frames to fully re-init).
    static s32 sLastForm = -1;
    static int sSettleFrames = 0;
    if (myForm != sLastForm) {
        sLastForm = myForm;
        sSettleFrames = 10;
    }
    if (sSettleFrames > 0) {
        sSettleFrames--;
        return;
    }

    void* objSegment = play->objectCtx.slots[player->actor.objectSlot].segment;
    if (objSegment == NULL) return;
    f32 yOffset = player->actor.shape.yOffset * player->actor.scale.y;

    // Snapshot the local player as the template for this form (stable here). Peers
    // in this form — now or later, even after we transform away — draw from it
    // with full equipment.
    sFormTemplate[myForm] = *player;
    sFormTemplateValid[myForm] = true;

    std::lock_guard<std::mutex> lk(h->StateMutex());

    static int diagCounter = 0;
    bool logThisFrame = ((diagCounter++ % 120) == 0);
    int drawnSame = 0, drawnCross = 0, skipped = 0;

    for (auto& [cid, c] : h->ClientsRaw()) {
        if (c.sceneId != myScene) continue;
        s32 form = c.transformation;
        if (form < 0 || form >= PLAYER_FORM_MAX) continue;

        // Bind the form's object on segment 6 if it's loaded: the current form's
        // object always is, Human's OBJECT_LINK_CHILD always is, other forms only
        // when they happen to be in the pool. MM-form skeletons are self-contained
        // resources and don't read segment 6, so a missing bind is harmless there.
        void* seg06 = NULL;
        {
            s32 fslot = (form == myForm) ? player->actor.objectSlot
                                         : Object_GetSlot(&play->objectCtx, gPlayerFormObjectIds[form]);
            if (fslot >= 0 && Object_IsLoaded(&play->objectCtx, fslot)) {
                seg06 = play->objectCtx.slots[fslot].segment;
            }
        }

        if (sFormTemplateValid[form]) {
            // We've been in this form, so we have a fully-valid Player template
            // for it. Draw the peer through the real player path → full equipment
            // and mask, regardless of which form WE are currently in.
            Player& dp = sDummyPlayers[cid];
            dp = sFormTemplate[form];
            dp.actor.world.pos.x = c.posX;
            dp.actor.world.pos.y = c.posY;
            dp.actor.world.pos.z = c.posZ;
            dp.actor.shape.rot.x = c.rotX;
            dp.actor.shape.rot.y = c.rotY;
            dp.actor.shape.rot.z = c.rotZ;
            dp.skelAnime.jointTable = (Vec3s*)c.jointTable;
            dp.currentBoots = c.currentBoots;
            dp.currentShield = c.currentShield;
            dp.itemAction = c.itemAction;
            dp.heldItemAction = c.heldItemAction;
            dp.currentMask = c.currentMask;
            f32 yOff = dp.actor.shape.yOffset * dp.actor.scale.y;

            OPEN_DISPS(play->state.gfxCtx);
            if (seg06 != NULL) {
                gSPSegment(POLY_OPA_DISP++, 0x06, (uintptr_t)seg06);
                gSPSegment(POLY_XLU_DISP++, 0x06, (uintptr_t)seg06);
            }
            gSPSegment(POLY_OPA_DISP++, 0x0C, (uintptr_t)gCullBackDList);
            gSPSegment(POLY_XLU_DISP++, 0x0C, (uintptr_t)gCullBackDList);
            Matrix_SetTranslateRotateYXZ(c.posX, c.posY + yOff, c.posZ, &dp.actor.shape.rot);
            Matrix_Scale(dp.actor.scale.x, dp.actor.scale.y, dp.actor.scale.z, MTXMODE_APPLY);
            Player_DrawImpl(play, dp.skelAnime.skeleton, dp.skelAnime.jointTable, dp.skelAnime.dListCount, 0,
                            (PlayerTransformation)form, dp.currentBoots, dp.actor.shape.face,
                            Player_OverrideLimbDrawGameplayDefault, Player_PostLimbDrawGameplay, &dp.actor);
            CLOSE_DISPS(play->state.gfxCtx);
            if (form == myForm) {
                drawnSame++;
            } else {
                drawnCross++;
            }
            continue;
        }

        // Fallback: a form we've never been in this session (no template). Draw
        // the body from the resource skeleton — no equipment, rootAnimScale
        // grounding. (Rare: Human is always visited, MM forms have no OoT-style
        // equipment to miss.)
        void** skel = NULL;
        s32 dListCount = 0;
        if (!ResolveFormSkeleton(form, &skel, &dListCount)) { skipped++; continue; }

        Actor& da = sCrossActors[cid];
        da.world.pos.x = c.posX;
        da.world.pos.y = c.posY;
        da.world.pos.z = c.posZ;
        Vec3s rot = { c.rotX, c.rotY, c.rotZ };

        sCurrentDrawForm = form;
        OPEN_DISPS(play->state.gfxCtx);
        gSPSegment(POLY_OPA_DISP++, 0x0C, (uintptr_t)gCullBackDList);
        gSPSegment(POLY_XLU_DISP++, 0x0C, (uintptr_t)gCullBackDList);
        if (seg06 != NULL) {
            gSPSegment(POLY_OPA_DISP++, 0x06, (uintptr_t)seg06);
            gSPSegment(POLY_XLU_DISP++, 0x06, (uintptr_t)seg06);
        }
        Matrix_SetTranslateRotateYXZ(c.posX, c.posY + yOffset, c.posZ, &rot);
        Matrix_Scale(player->actor.scale.x, player->actor.scale.y, player->actor.scale.z, MTXMODE_APPLY);
        Player_DrawImpl(play, skel, (Vec3s*)c.jointTable, dListCount, 0, (PlayerTransformation)form, 0, 0,
                        HarpoonCrossOverride, NULL, &da);
        CLOSE_DISPS(play->state.gfxCtx);
        drawnCross++;
    }

    if (logThisFrame) {
        SPDLOG_INFO("[Harpoon] DrawAll: clients={} myForm={} same={} cross={} skipped={}",
                    (int)h->ClientsRaw().size(), myForm, drawnSame, drawnCross, skipped);
    }
}

// Nametags previously anchored to a spawned actor; coop nametags are a TODO.
void HarpoonDummyPlayer_SyncNametags() {
}
