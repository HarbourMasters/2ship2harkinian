/**
 * spiritual_stone_statue.c - MM Owl Statue actor for spiritual-stone warp points.
 *
 * A REAL registered MM actor (ACTOR_SPIRITUAL_STONE_STATUE, DEFINE_ACTOR row in
 * actor_table.h; profile SpiritualStoneStatue_Profile below). It renders the
 * recolored MM owl statue DL from object_sek plus the matching floating jewel.
 * NOTE: the old ACTOR_EN_LIGHTBOX hijack could never work in MM — that id is an
 * OOB sentinel (0x7F16), so Actor_Spawn silently produced nothing.
 *
 * Stone index (0=Kokiri, 1=Goron, 2=Zora) rides in the spawn params and is
 * stashed in actor->home.rot.x by Statue_Init — the draw uses it to pick the
 * per-stone jewel + color tint. Lifetime is scene-bound (dies on scene unload,
 * the orchestrator re-spawns it on scene init for any warp in the new scene).
 *
 * Consumed via #include from spiritual_stones.cpp (the .c is NOT in vcxproj), so
 * the profile symbol lands in that TU's extern "C" block with C linkage.
 */

#include "spiritual_stone_statue.h"

#include "z64.h"
#include "macros.h"
#include "functions.h"
#include "variables.h"
#include "align_asset_macro.h"
#include "objects/gameplay_keep/gameplay_keep.h"
// OoT→MM compat shim: aliases Matrix_NewMtx → Matrix_Finalize for the jewel draw below (MM has no
// MATRIX_NEWMTX macro). (Formerly also aliased ACTOR_EN_LIGHTBOX/minVelocityY back when this was a
// host-actor hijack; now it's a real actor so those are unused, but the include is harmless.)
#include "nei_oot_compat.h"
#include <math.h>

// MM has NO OBJECT_GI_JEWEL (it is an OoT-only object; neither the object id nor the gGi*GemDL /
// gGi*SettingDL symbols exist in MM). The jewel display lists DO live in the companion oot.o2r
// (objects/object_gi_jewel/*). So we load them BY OTR PATH through the OoT asset loader — the exact
// same route Din's Fire uses for its flame DLs — instead of the MM object system (Object_Spawn +
// segment-6 rebind, which cannot work without the object). The resource manager resolves each DL's
// internal Vtx/texture references, so no object bank needs to be bound.
extern void* OotAssets_LoadGfx(const char* otrPath); // mm/mods/oot_asset_loader/oot_asset_loader.cpp

// OTR path for the MM owl statue DL. Same convention as elegy_shell_assets.h —
// pass the string pointer to Gfx_DrawDListOpa and the resource manager
// resolves it at render time. We use the OPENED variant so the wings show
// extended — the closed one hides them against the body.
#define dgOwlStatueOpenedDL "__OTR__objects/object_sek/gOwlStatueOpenedDL"
static const ALIGN_ASSET(2) char gOwlStatueOpenedDL_path[] = dgOwlStatueOpenedDL;

// Per-stone get-item jewel render data. Mirrors GetItem_DrawJewelKokiri/Goron/
// Zora in z_draw.c — the same DL pair + prim/env color pair the vanilla
// get-item draws use, so each spiritual stone floating above its owl statue
// matches the look of receiving it.
typedef struct {
    const char* gemDLPath;     // XLU pass (the gem itself, scintillating) — OTR path
    const char* settingDLPath; // OPA pass (the gold setting around the gem) — OTR path
    u8 primXlu[3];
    u8 envXlu[3];
    u8 primOpa[3];
    u8 envOpa[3];
} StoneJewel;

// Index matches SPIRITUAL_STONE_* enum in spiritual_stones.h. Colors mirror z_draw.c's
// GetItem_DrawJewel{Kokiri,Goron,Zora}. DLs are OTR paths into the companion oot.o2r.
static const StoneJewel sStoneJewels[3] = {
    // Kokiri Emerald — green
    {
        "__OTR__objects/object_gi_jewel/gGiKokiriEmeraldGemDL",
        "__OTR__objects/object_gi_jewel/gGiKokiriEmeraldSettingDL",
        { 255, 255, 160 }, { 0, 255, 0 }, { 255, 255, 170 }, { 150, 120, 0 },
    },
    // Goron Ruby — red
    {
        "__OTR__objects/object_gi_jewel/gGiGoronRubyGemDL",
        "__OTR__objects/object_gi_jewel/gGiGoronRubySettingDL",
        { 255, 170, 255 }, { 255, 0, 100 }, { 255, 255, 170 }, { 150, 120, 0 },
    },
    // Zora Sapphire — blue
    {
        "__OTR__objects/object_gi_jewel/gGiZoraSapphireGemDL",
        "__OTR__objects/object_gi_jewel/gGiZoraSapphireSettingDL",
        { 50, 255, 255 }, { 50, 0, 150 }, { 255, 255, 170 }, { 150, 120, 0 },
    },
};

// Companion jewel DLs, loaded once by OTR path and cached (retried while NULL — the companion may
// not be mounted yet at first draw). Returns NULL if oot.o2r is absent, in which case the gem draw
// is skipped (the owl statue still renders).
static Gfx* SpiritualStone_GemDL(int stone) {
    static Gfx* sCache[3] = { NULL, NULL, NULL };
    if (sCache[stone] == NULL) {
        sCache[stone] = (Gfx*)OotAssets_LoadGfx(sStoneJewels[stone].gemDLPath);
    }
    return sCache[stone];
}
static Gfx* SpiritualStone_SettingDL(int stone) {
    static Gfx* sCache[3] = { NULL, NULL, NULL };
    if (sCache[stone] == NULL) {
        sCache[stone] = (Gfx*)OotAssets_LoadGfx(sStoneJewels[stone].settingDLPath);
    }
    return sCache[stone];
}

// MM DLs branch into segment 0x0C for the scene cull list, which OOT leaves
// unset. Bind it to no-op gsSPEndDisplayList so those branches just return.
// Same trick as sSegment0xC_Noop in somaria_cubes.c.
static Gfx sStatueSegment0xC_Noop[] = {
    gsSPEndDisplayList(),
    gsSPEndDisplayList(),
    gsSPEndDisplayList(),
    gsSPEndDisplayList(),
};

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

static void Statue_Init(Actor* thisx, PlayState* play);
static void Statue_Destroy(Actor* thisx, PlayState* play);
static void Statue_Update(Actor* thisx, PlayState* play);
static void Statue_Draw(Actor* thisx, PlayState* play);

static ActorFunc sStatueUpdateFunc = Statue_Update;

#define STATUE_GET_STONE(actor) ((actor)->home.rot.x)
#define STATUE_SET_STONE(actor, s) ((actor)->home.rot.x = (s))

// Visual scale — 1/20 of the previous 0.1f. The MM owl statue is authored
// quite large; this brings it down to a marker-sized prop instead of an
// over-scale landmark.
#define STATUE_VISUAL_SCALE 0.005f

// ============================================================================
// INIT / DESTROY
// ============================================================================

static void Statue_Init(Actor* thisx, PlayState* play) {
    // The stone index (0=Kokiri 1=Goron 2=Zora) is passed in params by the spawner.
    STATUE_SET_STONE(thisx, thisx->params);

    // Purely visual marker — no physics, no shadow, marker-sized scale.
    thisx->gravity = 0.0f;
    thisx->terminalVelocity = 0.0f;
    thisx->shape.shadowDraw = NULL;
    thisx->shape.shadowScale = 0.0f;
    Actor_SetScale(thisx, STATUE_VISUAL_SCALE);
}

static void Statue_Destroy(Actor* thisx, PlayState* play) {
    (void)thisx;
    (void)play;
}

// ============================================================================
// UPDATE — nothing dynamic, the statue is purely decorative.
// ============================================================================

static void Statue_Update(Actor* thisx, PlayState* play) {
    // Nothing to do: the jewel DLs are loaded by OTR path in the draw (companion oot.o2r), not
    // through the MM object system, so there is no per-frame object request to pump.
    (void)thisx;
    (void)play;
}

// ============================================================================
// DRAW — recolored MM owl statue DL.
// ============================================================================

// Float / spin parameters for the jewel hovering above each statue. The
// statue itself is tiny (STATUE_VISUAL_SCALE 0.005f) so we keep the jewel
// just slightly above ground and lean on the larger jewel scale to read
// against the small statue.
#define JEWEL_HOVER_Y_BASE    35.0f // height above the statue's anchor
#define JEWEL_HOVER_AMPLITUDE  2.0f // how much it bobs up/down
#define JEWEL_HOVER_PERIOD_F  80.0f // ~80 frames for a full bob
#define JEWEL_SPIN_DEG_PER_F   5.0f // ~72 frames for a full spin
#define JEWEL_SCALE          0.12f  // markedly larger than the statue so it reads

static void Statue_Draw(Actor* thisx, PlayState* play) {
    s16 stone = STATUE_GET_STONE(thisx);
    if (stone < 0 || stone >= 3) {
        return;
    }

    // ----- Pass 1: opaque owl statue with vanilla colors. ----------------
    {
        OPEN_DISPS(play->state.gfxCtx);
        // Cast needed because this file gets pulled into the C++ translation
        // unit of spiritual_stones.cpp — C-style pointer→uintptr_t conversion
        // isn't implicit in C++.
        gSPSegment(POLY_OPA_DISP++, 0x0C, (uintptr_t)sStatueSegment0xC_Noop);
        Gfx_DrawDListOpa(play, (Gfx*)gOwlStatueOpenedDL_path);
        CLOSE_DISPS(play->state.gfxCtx);
    }

    // ----- Pass 2: floating + rotating spiritual stone gem above the statue.
    // Re-uses the same DL pair (Gem XLU, Setting OPA) and color setup that
    // z_draw.c uses for GetItem_DrawJewel{Kokiri,Goron,Zora}, so the floating
    // model matches what the player saw when they received the stone. Skipped
    // until the companion jewel DLs resolve (SpiritualStone_GemDL != NULL).
    Gfx* gemDL = SpiritualStone_GemDL(stone);
    Gfx* settingDL = SpiritualStone_SettingDL(stone);
    if ((gemDL != NULL) && (settingDL != NULL)) {
        const StoneJewel* j = &sStoneJewels[stone];
        f32 t = (f32)play->gameplayFrames;
        f32 bob = sinf(t * (2.0f * M_PI / JEWEL_HOVER_PERIOD_F)) * JEWEL_HOVER_AMPLITUDE;
        // BINANG units = 0x10000 / 360. Per-frame degree increment → BINANG.
        s16 spin = (s16)(t * (JEWEL_SPIN_DEG_PER_F * (0x10000 / 360.0f)));

        OPEN_DISPS(play->state.gfxCtx);

        // No segment-6 (object bank) rebind: the DLs were loaded standalone by OTR path, and the
        // resource manager resolves their internal Vtx/texture references (same as Din's Fire's
        // sModelDL). Only the animated texture-scroll segments below are still bound.

        // Texture-scroll segments the gem DLs reference. Without these, the
        // scintillation texture renders garbage. Same calls vanilla uses for
        // the get-item draw of the stones (see GetItem_DrawJewel in z_draw.c).
        // Casts: Gfx_*TexScrollEx returns Gfx*, gSPSegment wants uintptr_t —
        // C lets that decay implicitly, C++ (this TU is compiled as C++) does
        // not.
        gSPSegment(POLY_XLU_DISP++, 9,
                   (uintptr_t)Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, 255, 64, 64, 1, 0, 255, 16, 16, 0, 0, 0, 0));
        gSPSegment(POLY_OPA_DISP++, 8,
                   (uintptr_t)Gfx_TexScrollEx(play->state.gfxCtx, 0, 0, 16, 16, 0, 0));

        Matrix_Translate(thisx->world.pos.x,
                         thisx->world.pos.y + JEWEL_HOVER_Y_BASE + bob,
                         thisx->world.pos.z, MTXMODE_NEW);
        Matrix_RotateYF(BINANG_TO_RAD((f32)spin), MTXMODE_APPLY); // MM: RotateYF (float), not RotateY
        Matrix_Scale(JEWEL_SCALE, JEWEL_SCALE, JEWEL_SCALE, MTXMODE_APPLY);

        // Gem (XLU): cast const char[] → Gfx* for the C++ translation unit. MM names: Gfx_SetupDL25_Xlu
        // (not _25Xlu) and Matrix_NewMtx(gfxCtx, file, line) (MM has no MATRIX_NEWMTX macro).
        Gfx_SetupDL25_Xlu(play->state.gfxCtx);
        gSPMatrix(POLY_XLU_DISP++, Matrix_NewMtx(play->state.gfxCtx, "spiritual_stone_statue.c", 209),
                  G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 128, j->primXlu[0], j->primXlu[1], j->primXlu[2], 255);
        gDPSetEnvColor(POLY_XLU_DISP++, j->envXlu[0], j->envXlu[1], j->envXlu[2], 255);
        gSPDisplayList(POLY_XLU_DISP++, gemDL);

        // Setting (OPA): same matrix, different color pair.
        Gfx_SetupDL25_Opa(play->state.gfxCtx);
        gSPMatrix(POLY_OPA_DISP++, Matrix_NewMtx(play->state.gfxCtx, "spiritual_stone_statue.c", 217),
                  G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 128, j->primOpa[0], j->primOpa[1], j->primOpa[2], 255);
        gDPSetEnvColor(POLY_OPA_DISP++, j->envOpa[0], j->envOpa[1], j->envOpa[2], 255);
        gSPDisplayList(POLY_OPA_DISP++, settingDL);

        CLOSE_DISPS(play->state.gfxCtx);
    }
}

// ============================================================================
// ACTOR PROFILE + SPAWN
// ============================================================================

// Real MM actor registration (mirrors Oot_DinsFire_Profile in sw97_router.c). The extern
// declaration comes from DEFINE_ACTOR(SpiritualStoneStatue, ...) in actor_table.h, which also wires
// the gActorOverlayTable entry that points at this profile. This file is #included inside the
// extern "C" block of spiritual_stones.cpp, so the symbol has C linkage and matches that extern.
// ACTORCAT_PROP + flags 0: a static, non-targetable decorative prop. GAMEPLAY_KEEP because the owl
// and jewel DLs are loaded standalone by OTR path (draw), not from a per-actor object.
ActorProfile SpiritualStoneStatue_Profile = {
    /**/ ACTOR_SPIRITUAL_STONE_STATUE,
    /**/ ACTORCAT_PROP,
    /**/ 0,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(Actor),
    /**/ Statue_Init,
    /**/ Statue_Destroy,
    /**/ Statue_Update,
    /**/ Statue_Draw,
};

Actor* SpiritualStoneStatue_Spawn(PlayState* play, Vec3f* pos, s16 rotY, int stone) {
    if (stone < 0 || stone >= 3) {
        return NULL;
    }

    // Spawn the real actor; the stone index rides in params → read back in Statue_Init.
    return Actor_Spawn(&play->actorCtx, play, ACTOR_SPIRITUAL_STONE_STATUE,
                       pos->x, pos->y, pos->z, 0, rotY, 0, (s16)stone);
}

u8 SpiritualStoneStatue_IsStatue(Actor* actor) {
    if (actor == NULL || actor->update == NULL) {
        return 0;
    }
    return (actor->update == sStatueUpdateFunc);
}
