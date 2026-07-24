/**
 * object_net.c — Bug-catching Net held model + catch volume (ITEM_NET). MM port of the SoH version.
 *
 * Model lives in 2ship.o2r (objects/object_nei_net, same asset as OoT). Drawn in Link's LEFT hand
 * (the sword hand) from Player_PostLimbDrawGameplay at PLAYER_LIMB_LEFT_HAND, so the CURRENT matrix
 * is the hand BONE — the net inherits the hand's full rotation (wrist ROLL) and swings 1:1 with the
 * sword (the Net wields via the Kokiri Sword IA; identity is heldItemId == ITEM_NET).
 *
 * The in-hand placement is FINAL (tuned in OoT via the old gNetModel.* sliders, baked as constants):
 * Scale 0.49, RotX -5, RotY -169, RotZ 64, Offset (3.2, 4.3, -1.0); catch radius 30.
 *
 * Also owns the CATCH: 5 world-space samples spanning the whole net DL (grip -> hoop, from the model
 * bbox X[-67,9] Y[-14,73]) are recomputed every draw with the same matrix as the model; while the
 * melee swing is active, Net_CaptureAtBlade scoops the nearest catchable (fairy/fish/bug) crossing
 * them into an empty bottle (Bottle_CatchIntoEmpty). No damage, no trail — it's a net.
 *
 * Included by object_custom_items.c (unity build inside z_player.c's TU); z_player_lib.c calls the
 * extern entry points.
 */

#ifndef ITEM_NET
#define ITEM_NET 0xF7 // mirror of z64item.h to avoid include-order issues
#endif

extern u8 ResourceMgr_FileExists(const char* resName);
extern Gfx* ResourceMgr_LoadGfxByName(const char* path);
extern uint8_t Bottle_CatchIntoEmpty(uint16_t content);

#define NET_CATCH_PTS 5
#define NET_CATCH_RADIUS 30.0f

// World-space catch samples (grip -> hoop head), updated each draw. Consumed by Net_CaptureAtBlade.
Vec3f gNetCatchPts[NET_CATCH_PTS];
u8 gNetCatchPtsValid = 0;

static Gfx* Net_GetCachedDL(const char* otr, Gfx** cache, u8* tried) {
    if (!*tried) {
        *tried = 1;
        if (ResourceMgr_FileExists(otr)) {
            *cache = ResourceMgr_LoadGfxByName(otr);
        }
    }
    return *cache;
}

// Opaque = the handle/rim/wrap. XLU = the semitransparent white netting.
static Gfx* Net_GetDL(void) {
    static Gfx* sDL = NULL;
    static u8 sTried = 0;
    return Net_GetCachedDL("__OTR__objects/object_nei_net/g_net_dl", &sDL, &sTried);
}
static Gfx* Net_GetXluDL(void) {
    static Gfx* sDL = NULL;
    static u8 sTried = 0;
    return Net_GetCachedDL("__OTR__objects/object_nei_net/g_net_xlu_dl", &sDL, &sTried);
}

// Draw the net on the CURRENT (left-hand bone) matrix. Caller wraps with Matrix_Push/Pop.
void CustomItems_DrawNet(Player* player, PlayState* play) {
    if (player->heldItemId != ITEM_NET) {
        return;
    }

    Gfx* dl = Net_GetDL();
    Gfx* xluDL = Net_GetXluDL();
    if (dl == NULL && xluDL == NULL) {
        return;
    }

    OPEN_DISPS(play->state.gfxCtx);

    // The hand-bone matrix bakes in Link's actor scale (~0.01) — divide the world-magnitude constants
    // by it so they render at the tuned size (same trick as the SoH version).
    f32 as = player->actor.scale.x;
    f32 inv = (as > 0.0001f) ? (1.0f / as) : 100.0f;

    f32 scale = 0.49f * inv;
    f32 ox = 3.2f * inv;
    f32 oy = 4.3f * inv;
    f32 oz = -1.0f * inv;
    f32 rx = -5.0f * (M_PI / 180.0f);
    f32 ry = -169.0f * (M_PI / 180.0f);
    f32 rz = 64.0f * (M_PI / 180.0f);

    // model-local orientation + offset (baked), applied on top of the hand-bone matrix
    Matrix_RotateXF(rx, MTXMODE_APPLY);
    Matrix_RotateYF(ry, MTXMODE_APPLY);
    Matrix_RotateZF(rz, MTXMODE_APPLY);
    Matrix_Translate(ox, oy, oz, MTXMODE_APPLY);
    Matrix_Scale(scale, scale, scale, MTXMODE_APPLY);

    // Catch samples: model-space line from the grip (origin) to the hoop head, transformed by the
    // CURRENT matrix — i.e. exactly where the net renders.
    {
        static const Vec3f sNetSpan = { -57.0f, 55.0f, 0.0f }; // grip -> hoop-head (model units, bbox)
        s32 i;
        for (i = 0; i < NET_CATCH_PTS; i++) {
            f32 t = (f32)i / (NET_CATCH_PTS - 1);
            Vec3f p = { sNetSpan.x * t, sNetSpan.y * t, sNetSpan.z * t };
            Matrix_MultVec3f(&p, &gNetCatchPts[i]);
        }
        gNetCatchPtsValid = 1;
    }

    if (dl != NULL) {
        Gfx_SetupDL25_Opa(play->state.gfxCtx);
        MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
        gSPDisplayList(POLY_OPA_DISP++, dl);
    }
    if (xluDL != NULL) {
        Gfx_SetupDL25_Xlu(play->state.gfxCtx);
        MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
        gSPDisplayList(POLY_XLU_DISP++, xluDL);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

// MM catchables the net can scoop (mirror of the vanilla swing-catch table): healing fairies
// (EN_ELF FAIRY_TYPE_2/6 — never Tatl, she isn't an EN_ELF), fish, and bugs.
static u8 Net_ContentForActor(Actor* actor) {
    switch (actor->id) {
        case ACTOR_EN_ELF: {
            u8 fairyType = actor->params & 0xF; // FAIRY_GET_TYPE
            return (fairyType == 2 || fairyType == 6) ? ITEM_FAIRY : ITEM_NONE;
        }
        case ACTOR_EN_FISH:
            return ITEM_FISH;
        case ACTOR_EN_MUSHI2:
            return ITEM_BUG;
        default:
            return ITEM_NONE;
    }
}

static u8 sNetCaughtThisSwing = 0; // one bottle catch per swing; reset between swings

// Min distance from an actor to the net's catch samples (the whole DL, grip -> hoop).
static f32 Net_DistToCatchVolume(Actor* actor) {
    f32 best = 99999.0f;
    s32 i;
    for (i = 0; i < NET_CATCH_PTS; i++) {
        f32 dx = actor->world.pos.x - gNetCatchPts[i].x;
        f32 dy = actor->world.pos.y - gNetCatchPts[i].y;
        f32 dz = actor->world.pos.z - gNetCatchPts[i].z;
        f32 d = sqrtf(dx * dx + dy * dy + dz * dz);
        if (d < best) {
            best = d;
        }
    }
    return best;
}

// Reset the per-swing latch (called from z_player_lib when the melee swing isn't active).
void Net_ResetSwingCatch(void) {
    sNetCaughtThisSwing = 0;
}

// Scoop the nearest catchable crossing the net volume into an empty bottle. Runs only while the
// sword-swing (melee weapon state) is active with the Net held. No damage is dealt.
void Net_CaptureAtBlade(PlayState* play, Player* player) {
    Actor* best = NULL;
    u8 bestContent = ITEM_NONE;
    f32 bestDist = NET_CATCH_RADIUS;
    s32 cat;

    if (!gNetCatchPtsValid || sNetCaughtThisSwing) {
        return;
    }

    for (cat = 0; cat < ACTORCAT_MAX; cat++) {
        Actor* actor = play->actorCtx.actorLists[cat].first;
        while (actor != NULL) {
            u8 content = Net_ContentForActor(actor);
            if (content != ITEM_NONE) {
                f32 d = Net_DistToCatchVolume(actor);
                if (d < bestDist) {
                    bestDist = d;
                    best = actor;
                    bestContent = content;
                }
            }
            actor = actor->next;
        }
    }

    if (best == NULL) {
        return;
    }

    u8 placed = Bottle_CatchIntoEmpty(bestContent);
    if (!placed) {
        // Fallback: no wheel/bottomless space — fill the first empty VANILLA bottle slot, refreshing
        // any C-button that shows it (covers saves not driven by NEI bottleSlots).
        s32 bs;
        for (bs = SLOT_BOTTLE_1; bs <= SLOT_BOTTLE_6; bs++) {
            if (gSaveContext.save.saveInfo.inventory.items[bs] == ITEM_BOTTLE) {
                s32 btn;
                gSaveContext.save.saveInfo.inventory.items[bs] = bestContent;
                for (btn = EQUIP_SLOT_C_LEFT; btn <= EQUIP_SLOT_C_RIGHT; btn++) {
                    if (C_SLOT_EQUIP(0, btn) == bs) {
                        BUTTON_ITEM_EQUIP(0, btn) = bestContent;
                        Interface_LoadItemIconImpl(play, btn);
                    }
                }
                placed = 1;
                break;
            }
        }
    }
    if (placed) {
        Audio_PlaySfx(NA_SE_SY_GET_ITEM);
        Actor_Kill(best);
        sNetCaughtThisSwing = 1;
    }
}
