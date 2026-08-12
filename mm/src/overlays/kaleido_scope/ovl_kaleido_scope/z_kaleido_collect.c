/*
 * File: z_kaleido_collect.c
 * Overlay: ovl_kaleido_scope
 * Description: Pause Menu - Quest Status Page
 */

#include "z_kaleido_scope.h"
#include <libultraship/log/luslog.h> // 2S2H [Port] lusprintf (LUS 464 exports it via API_EXPORT)
#include "interface/parameter_static/parameter_static.h"
#include "archives/icon_item_static/icon_item_static_yar.h"
#include <libultraship/bridge/consolevariablebridge.h> // CVarGetInteger (OoT quest-page L-flip toggle)

s32 KaleidoScope_UpdateQuestStatusPoint(PauseContext* pauseCtx, s16 point);

// ============================================================================
// Skijer's NEI — OoT quest-status page (L-flip alternate of MM's quest page).
//
// A faithful port of OoT's KaleidoScope_DrawQuestStatus collect layout, reading a PARALLEL
// quest store (NeiSaveData.ootQuestItems / ootGsCount, OoT bit layout) so it never collides
// with MM's own gSaveContext.inventory.questItems. All icons come from the companion oot.o2r
// (textures/icon_item_24_static/gQuestIcon*Tex), loaded on demand. Toggled with L on the quest
// page (KaleidoScope_HandlePageToggles), persisted in a CVar. Self-contained: its own local
// Vtx grid (MM's pauseCtx->questVtx is sized for MM's 39-quad layout, too small for OoT's).
// ============================================================================
#include "mods/nei_save.h"
#include "mods/extended_inventory.h"
#include "mods/spiritual_stones/spiritual_stones.h" // SpiritualStone_IsPassiveActive (stone dimming)

extern void* OotAssets_LoadTexOrDList(const char* otrPath);
extern const char* sCounterTextures[]; // GS-count digit textures (I8 8x16), shared with MM's page

// CVar-persisted view toggle: 0 = MM quest page, 1 = OoT quest page.
#define CVAR_OOT_QUEST_PAGE "gEnhancements.Kaleido.OotQuestPage"

// OoT song-note colors — EXACT (soh z_kaleido_collect.c D_8082A164/17C/194).
static const s16 sOotSongR[12] = { 150, 255, 100, 255, 255, 255, 255, 255, 255, 255, 255, 255 };
static const s16 sOotSongG[12] = { 255, 80, 150, 160, 100, 240, 255, 255, 255, 255, 255, 255 };
static const s16 sOotSongB[12] = { 100, 40, 255, 0, 255, 100, 255, 255, 255, 255, 255, 255 };

// OoT medallion glow — env-color cycle targets (soh D_8082A090). Phase 0 = index sp218 (all 0 =
// no glow), phase 2 = index sp218+6 (the colored halo). Phases 1/3 hold. The env lerps toward the
// active phase's target each frame, giving the rotating colored glow behind the medallions.
static const s16 sOotMedGlowTargets[12][3] = {
    { 0, 0, 0 },  { 0, 0, 0 },  { 0, 0, 0 },    { 0, 0, 0 },   { 0, 0, 0 },   { 0, 0, 0 },
    { 0, 60, 0 }, { 90, 0, 0 }, { 0, 40, 110 }, { 80, 40, 0 }, { 70, 0, 90 }, { 90, 90, 0 },
};

// OoT quest-vertex layout tables (soh z_kaleido_scope_PAL.c D_8082B138 = X, D_8082B198 = Y,
// D_8082B1F8 = size). 47 quads: 0-5 medallions, 6-17 songs, 18-20 stones, 21 agony, 22 gerudo,
// 23 skulltula, 24 GS medal (48px), 25-40 ocarina staff, 41-46 GS-count digits.
static const s16 sOotVtxX[47] = {
    74,  74,  46,  18,  18,  46,  -108, -90, -72, -54, -36, -18, -108, -90, -72, -54,
    -36, -18, 20,  46,  72,  -110, -86,  -110, -54, -98, -86, -74, -62,  -50, -38, -26,
    -14, -98, -86, -74, -62, -50,  -38,  -26,  -14, -88, -81, -72, -90,  -83, -74,
};
static const s16 sOotVtxY[47] = {
    38, 6,   -12, 6,   38,  56,  -20, -20, -20, -20, -20, -20, 2,   2,   2,   2,   2,   2,  -46, -46, -46, 58, 58, 34,
    58, -52, -52, -52, -52, -52, -52, -52, -52, -52, -52, -52, -52, -52, -52, -52, -52, 34, 34,  34,  36,  36, 36,
};
static const s16 sOotVtxSize[47] = {
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    48, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
};

static const char* sOotMedallionIconPaths[6] = {
    "__OTR__textures/icon_item_24_static/gQuestIconMedallionForestTex",
    "__OTR__textures/icon_item_24_static/gQuestIconMedallionFireTex",
    "__OTR__textures/icon_item_24_static/gQuestIconMedallionWaterTex",
    "__OTR__textures/icon_item_24_static/gQuestIconMedallionSpiritTex",
    "__OTR__textures/icon_item_24_static/gQuestIconMedallionShadowTex",
    "__OTR__textures/icon_item_24_static/gQuestIconMedallionLightTex",
};
static const char* sOotStoneIconPaths[3] = {
    "__OTR__textures/icon_item_24_static/gQuestIconKokiriEmeraldTex",
    "__OTR__textures/icon_item_24_static/gQuestIconGoronRubyTex",
    "__OTR__textures/icon_item_24_static/gQuestIconZoraSapphireTex",
};
static const char* sOotAgonyIconPath = "__OTR__textures/icon_item_24_static/gQuestIconStoneOfAgonyTex";
static const char* sOotGerudoIconPath = "__OTR__textures/icon_item_24_static/gQuestIconGerudosCardTex";
static const char* sOotSkulltulaIconPath = "__OTR__textures/icon_item_24_static/gQuestIconGoldSkulltulaTex";

// OoT quest cursor points: 0-5 medallions, 6-0x11 songs, 0x12-0x14 stones, 0x15 agony, 0x16 gerudo,
// 0x17 skulltula, 0x18 heart-piece pie. Points map 1:1 onto the quad indices of the layout tables.
#define OOT_QUEST_POINT_HEART_PIECE 0x18
#define OOT_QUEST_POINT_MAX OOT_QUEST_POINT_HEART_PIECE

// Companion item-name textures (IA4 128x16) for the hovered slot — shown in the bottom name box, so
// hovering a song tells you WHICH song it is (its "preview"). Indexed by quest point 0..0x18.
// Entries 13/16/17 are NEI-generated (mm/assets/custom/textures/item_name_custom/generate_names.py);
// the rest come from the companion oot.o2r. Skijer's NEI.
static const char* sOotNamePaths[OOT_QUEST_POINT_MAX + 1] = {
    "__OTR__textures/item_name_static/gForestMedallionItemNameENGTex", // 0 Forest
    "__OTR__textures/item_name_static/gFireMedallionItemNameENGTex",   // 1 Fire
    "__OTR__textures/item_name_static/gWaterMedallionItemNameENGTex",  // 2 Water
    "__OTR__textures/item_name_static/gSpiritMedallionItemNameENGTex", // 3 Spirit
    "__OTR__textures/item_name_static/gShadowMedallionItemNameENGTex", // 4 Shadow
    "__OTR__textures/item_name_static/gLightMedallionItemNameENGTex",  // 5 Light
    "__OTR__textures/item_name_static/gMinuetOfForestItemNameENGTex",  // 6 Minuet
    "__OTR__textures/item_name_static/gBoleroOfFireItemNameENGTex",    // 7 Bolero
    "__OTR__textures/item_name_static/gSerenadeOfWaterItemNameENGTex", // 8 Serenade
    "__OTR__textures/item_name_static/gRequiemOfSpiritItemNameENGTex", // 9 Requiem
    "__OTR__textures/item_name_static/gNocturneOfShadowItemNameENGTex",// 10 Nocturne
    "__OTR__textures/item_name_static/gPreludeOfLightItemNameENGTex",  // 11 Prelude
    "__OTR__textures/item_name_static/gZeldasLullabyItemNameENGTex",   // 12 Lullaby
    "__OTR__textures/item_name_custom/gFugueOfHomeNameTex",            // 13 Fugue of Home (custom)
    "__OTR__textures/item_name_static/gSariasSongItemNameENGTex",      // 14 Saria
    "__OTR__textures/item_name_static/gSunsSongItemNameENGTex",        // 15 Sun
    "__OTR__textures/item_name_custom/gCommandMelodyNameTex",          // 16 Command Melody (custom)
    "__OTR__textures/item_name_custom/gBalladOfHeroNameTex",           // 17 Ballad of Hero (custom)
    "__OTR__textures/item_name_static/gKokiriEmeraldItemNameENGTex",   // 18 Kokiri Emerald
    "__OTR__textures/item_name_static/gGoronsRubyItemNameENGTex",      // 19 Goron's Ruby
    "__OTR__textures/item_name_static/gZorasSapphireItemNameENGTex",   // 20 Zora's Sapphire
    "__OTR__textures/item_name_static/gStoneofAgonyItemNameENGTex",    // 21 Stone of Agony
    "__OTR__textures/item_name_static/gGerudosCardItemNameENGTex",     // 22 Gerudo's Card
    "__OTR__textures/item_name_static/gGoldSkulltulaItemNameENGTex",   // 23 Gold Skulltula
    "__OTR__textures/item_name_static/gPieceofHeartItemNameENGTex",    // 0x18 Heart-piece pie
};

// Cache one companion texture per unique path (small fixed set; loaded once).
//
// MUST probe first: ResourceMgr_LoadTexOrDListByName (which OotAssets_LoadTexOrDList wraps) does
// `GetResourceByName(path)->GetInitData()` with NO null check, so a path that isn't in the mounted
// archives is a straight 0xC0000005 — not a NULL return. FileExists is the same guard the equipment
// page's KaleidoEquip_OotTex uses. Skijer 2026-07-30 (crash on opening the pause menu).
static void* OotQuest_Tex(const char* path) {
    extern u8 ResourceMgr_FileExists(const char* resName);

    if ((path == NULL) || !ResourceMgr_FileExists(path)) {
        return NULL;
    }
    return OotAssets_LoadTexOrDList(path);
}

// OoT-page song fingerings (OCARINA_BTN_* per note), song index 0..11 == quest point 6..17
// (Minuet, Bolero, Serenade, Requiem, Nocturne, Prelude, Lullaby, FUGUE OF HOME, Saria, Sun,
// COMMAND MELODY, BALLAD OF HERO). The 3 truly-doubled songs (Epona/Time/Storms — MM grants them
// natively) are REPLACED by the NEI custom songs on this page. Used to draw the notes on hover.
static const u8 sOotSongButtons[12][8] = {
    { OCARINA_BTN_A, OCARINA_BTN_C_UP, OCARINA_BTN_C_LEFT, OCARINA_BTN_C_RIGHT, OCARINA_BTN_C_LEFT, OCARINA_BTN_C_RIGHT },
    { OCARINA_BTN_C_DOWN, OCARINA_BTN_A, OCARINA_BTN_C_DOWN, OCARINA_BTN_A, OCARINA_BTN_C_RIGHT,
      OCARINA_BTN_C_DOWN, OCARINA_BTN_C_RIGHT, OCARINA_BTN_C_DOWN }, // Bolero (real OoT: Cd,A,Cd,A,Cr,Cd,Cr,Cd)
    { OCARINA_BTN_A, OCARINA_BTN_C_DOWN, OCARINA_BTN_C_RIGHT, OCARINA_BTN_C_RIGHT, OCARINA_BTN_C_LEFT },
    { OCARINA_BTN_A, OCARINA_BTN_C_DOWN, OCARINA_BTN_A, OCARINA_BTN_C_RIGHT, OCARINA_BTN_C_DOWN, OCARINA_BTN_A },
    { OCARINA_BTN_C_LEFT, OCARINA_BTN_C_RIGHT, OCARINA_BTN_C_RIGHT, OCARINA_BTN_A, OCARINA_BTN_C_LEFT,
      OCARINA_BTN_C_RIGHT, OCARINA_BTN_C_DOWN },
    { OCARINA_BTN_C_UP, OCARINA_BTN_C_RIGHT, OCARINA_BTN_C_UP, OCARINA_BTN_C_RIGHT, OCARINA_BTN_C_LEFT, OCARINA_BTN_C_UP },
    { OCARINA_BTN_C_LEFT, OCARINA_BTN_C_UP, OCARINA_BTN_C_RIGHT, OCARINA_BTN_C_LEFT, OCARINA_BTN_C_UP, OCARINA_BTN_C_RIGHT },
    { OCARINA_BTN_A, OCARINA_BTN_C_DOWN, OCARINA_BTN_C_RIGHT, OCARINA_BTN_C_UP, OCARINA_BTN_C_RIGHT,
      OCARINA_BTN_C_LEFT }, // Fugue of Home (replaces Epona's row)
    { OCARINA_BTN_C_DOWN, OCARINA_BTN_C_RIGHT, OCARINA_BTN_C_LEFT, OCARINA_BTN_C_DOWN, OCARINA_BTN_C_RIGHT,
      OCARINA_BTN_C_LEFT },
    { OCARINA_BTN_C_RIGHT, OCARINA_BTN_C_DOWN, OCARINA_BTN_C_UP, OCARINA_BTN_C_RIGHT, OCARINA_BTN_C_DOWN, OCARINA_BTN_C_UP },
    { OCARINA_BTN_A, OCARINA_BTN_C_LEFT, OCARINA_BTN_A, OCARINA_BTN_C_RIGHT, OCARINA_BTN_A,
      OCARINA_BTN_C_LEFT, OCARINA_BTN_A, OCARINA_BTN_C_RIGHT }, // Command Melody (replaces Song of Time's row)
    { OCARINA_BTN_A, OCARINA_BTN_C_DOWN, OCARINA_BTN_C_UP, OCARINA_BTN_C_LEFT, OCARINA_BTN_C_RIGHT,
      OCARINA_BTN_C_LEFT, OCARINA_BTN_C_RIGHT }, // Ballad of Hero (replaces Song of Storms' row)
};
static const u8 sOotSongButtonCount[12] = { 6, 8, 5, 6, 7, 6, 6, 6, 6, 6, 8, 7 };

// OoT quest song (0..11) -> MM OCARINA_SONG slot. The 6 warp songs have dedicated slots (24-29);
// Lullaby/Saria/Sun reuse the MM slots holding the same melody; the 3 doubled rows are the NEI custom
// songs (slots 30-32 — playback + side recognition; no repeats anywhere).
static const s8 sOotSongToMmOcarina[12] = {
    OCARINA_SONG_OOT_MINUET,     OCARINA_SONG_OOT_BOLERO,         OCARINA_SONG_OOT_SERENADE,
    OCARINA_SONG_OOT_REQUIEM,    OCARINA_SONG_OOT_NOCTURNE,       OCARINA_SONG_OOT_PRELUDE,
    OCARINA_SONG_ZELDAS_LULLABY, OCARINA_SONG_NEI_FUGUE_OF_HOME,  OCARINA_SONG_SARIAS,
    OCARINA_SONG_SUNS,           OCARINA_SONG_NEI_COMMAND_MELODY, OCARINA_SONG_NEI_BALLAD_OF_HERO,
};

// Note-button textures (IA8 16x16), OCARINA_BTN_* order — the same art MM's own quest-song staff uses.
static TexturePtr sOotOcarinaNoteTex[5] = {
    gOcarinaATex, gOcarinaCDownTex, gOcarinaCRightTex, gOcarinaCLeftTex, gOcarinaCUpTex,
};

// MM's own quest-song played-note accumulator + fade alphas (defined lower in this file, used by MM's
// vanilla quest draw) — reused by our OoT minigame's note-reveal + play-it-yourself.
extern s16 sQuestSongPlayedOcarinaButtonsNum;
extern u8 sQuestSongPlayedOcarinaButtons[];
extern s16 sQuestSongPlayedOcarinaButtonsAlpha[];

// ---- Interaction (cursor + equip + play-song), gated by the menu enhancements ----
#define CVAR_QUEST_INTERACT "gEnhancements.SkijerNEI.QuestPageInteract"
// "Pause Play": when ON (and the player holds an ocarina), A on a learned song plays it overworld-style
// (quick play + latched effect) INSTEAD of the learn-it minigame. OFF (default) = the minigame.
#define CVAR_PAUSE_PLAY "gEnhancements.SkijerNEI.PausePlay"

extern void SpiritualStone_TogglePassive(s32 stone); // A on a stone: flip its passive buff
void Interface_LoadItemIconImpl(PlayState* play, u8 btn);                    // refresh a C-button icon
void Interface_Dpad_LoadItemIcon(PlayState* play, u8 btn);                   // refresh a D-pad-button icon
Gfx* Gfx_DrawTexQuad4b(Gfx* gfx, TexturePtr texture, s32 fmt, s16 textureWidth, s16 textureHeight, u16 point);
Gfx* Gfx_DrawTexQuadIA8(Gfx* gfx, TexturePtr texture, s16 textureWidth, s16 textureHeight, u16 point); // note buttons

// OoT medallion item IDs, in quest-point order (0..5). NOT contiguous in MM's item enum
// (Shadow=0xF9, Light=0xFA skip 0xF7/0xF8), so equip must index this table, never do arithmetic.
static const u8 sOotMedallionItemIds[6] = {
    ITEM_MEDALLION_FOREST, ITEM_MEDALLION_FIRE,   ITEM_MEDALLION_WATER,
    ITEM_MEDALLION_SPIRIT, ITEM_MEDALLION_SHADOW, ITEM_MEDALLION_LIGHT,
};

// OoT quest cursor point (0..0x19: 0-5 medallions, 6-0x11 songs, 0x12-0x14 stones, 0x15 agony,
// 0x16 gerudo, 0x17 skulltula, 0x18 heart-piece). Drives the highlight + selection on the OoT page.
// The cursor itself is MM's gPauseMenuCursorTex (from the included icon_item_static_yar.h) drawn as
// 4 spinning circles in the PAGE view (Math_SinS/CosS from z64math.h) so it aligns with the icons —
// MM's own DrawCursor runs in a different flat view (SetView 0,0,64) and can't be aligned blind.
static s16 sOotQuestCursorPoint = 0;

// ---- Quartz of Motion: category-tracking list on the Stone of Agony slot ----
// Level 2 of the progressive Stone of Agony. Pressing A on point 0x15 opens a
// modal list of tracking categories (Bombers'-Notebook-style, but implemented
// like the item page's cycle wheels: a small modal that OWNS the stick until it
// closes, so it costs nothing outside the kaleido). Confirming spends one heart
// container and runs the sensor for 5 minutes — see 2s2h/Rando/DesireCompass.h.
#include "2s2h/Rando/DesireCompass.h"

#define QUARTZ_POINT 0x15

static s8 sQuartzMenuOpen = 0;
static s8 sQuartzMenuIndex = 0;
static s8 sQuartzStickHeld = 0;

// Read by the ImGui overlay (2s2h/Rando/DesireCompassHud.cpp), which renders the
// list itself — the kaleido owns the INPUT, the overlay owns the PIXELS. Drawing
// it here would mean hand-rolling text with the N64 renderer for no benefit.
u8 Quartz_IsListOpen(void) {
    return (u8)sQuartzMenuOpen;
}
s32 Quartz_GetListIndex(void) {
    return sQuartzMenuIndex;
}

// ---- "Pause Play": close the pause menu and auto-play the selected song on the REAL ocarina ----
// Works for ALL songs (MM's quest page + the OoT page). Phase machine driven from Player_Update
// (NeiPausePlay_Update): the menu closes, Link pulls out the ocarina (Player_UseItem — same as
// pressing its item button), the free-play staff opens, the song auto-plays, and for MM songs
// AudioOcarina_ForceSongPlayed hands it to the NATIVE recognition flow → the song's real effect
// (Soaring menu, Song of Time prompt, Storms rain, ...). OoT warp songs (slots 24-29) skip the force
// (the message handler only knows songs <= SCARECROW_SPAWN); their effect stays latched in
// gOotQuestSongToPlay for the warp/effect pass. Skijer's NEI.
void Player_UseItem(PlayState* play, Player* player, ItemId item); // z_player.c:4778 (no header decl)

static s16 sNeiPausePlaySong = -1; // MM ocarina slot to auto-play in-world, or -1
static u8 sNeiPausePlayPhase = 0;  // 0 idle, 1 pull ocarina, 2 wait free-play staff (then instant success)
static s16 sNeiPausePlayTimer = 0; // watchdog so a blocked state (cutscene, water...) can't wedge it

// Deterministic forced-success handoff, consumed by z_message.c's MSGMODE_OCARINA_PLAYING handler.
// NEEDED because the audio-side played-song latch (sPlayedOcarinaSongIndexPlusOne → playing staff)
// only survives ONE AudioOcarina_Update tick, and Audio_Update runs more than once per game frame
// (game.c:252 + graph.c:249) — the message's poll usually missed it. The message reads THIS instead,
// synchronously, so the success can never be dropped. -1 = none pending.
s16 gNeiPausePlayForcedSong = -1;

static void NeiPausePlay_Start(PlayState* play, s16 mmSongIndex) {
    PauseContext* pauseCtx = &play->pauseCtx;
    extern f32 sPauseMenuVerticalOffset; // z_kaleido_scope_NES.c:322 (file-global, not static)

    sNeiPausePlaySong = mmSongIndex;
    sNeiPausePlayPhase = 1;
    sNeiPausePlayTimer = 200; // ~10s of gameplay frames
    gNeiPausePlayForcedSong = -1;

    // Close the pause menu exactly like the B/Start close path (z_kaleido_scope_NES.c:3609-3621).
    AudioOcarina_SetInstrument(OCARINA_INSTRUMENT_OFF);
    Interface_SetAButtonDoAction(play, DO_ACTION_NONE);
    pauseCtx->state = PAUSE_STATE_UNPAUSE_SETUP;
    sPauseMenuVerticalOffset = -6240.0f;
    Audio_PlaySfx_PauseMenuOpenOrClose(SFX_PAUSE_MENU_CLOSE);
    pauseCtx->mainState = PAUSE_MAIN_STATE_IDLE;
}

// NEI-DBG: pause-play tracing (remove after diagnosis)

// Called every frame from Player_Update (gameplay only). Skijer's NEI.
void NeiPausePlay_Update(PlayState* play, Player* player) {
    MessageContext* msgCtx = &play->msgCtx;

    if (sNeiPausePlayPhase == 0) {
        return;
    }
    if (--sNeiPausePlayTimer <= 0) {
        lusprintf(__FILE__, __LINE__, 2, "NEI-PP: WATCHDOG abort phase=%d song=%d msgMode=%d action=%d",
                  sNeiPausePlayPhase, sNeiPausePlaySong, msgCtx->msgMode, msgCtx->ocarinaAction);
        sNeiPausePlayPhase = 0;
        sNeiPausePlaySong = -1;
        return;
    }

    switch (sNeiPausePlayPhase) {
        case 1: // gameplay resumed → pull out the ocarina (same pipeline as pressing its item button)
            if ((play->pauseCtx.state == PAUSE_STATE_OFF) && (msgCtx->msgMode == MSGMODE_NONE)) {
                lusprintf(__FILE__, __LINE__, 2, "NEI-PP: phase1 UseItem(ocarina) song=%d", sNeiPausePlaySong);
                Player_UseItem(play, player, ITEM_OCARINA_OF_TIME);
                sNeiPausePlayPhase = 2;
            }
            break;

        case 2: // the free-play staff is up → INSTANT success: count the song as played right away.
            // The native success flow then does everything itself — correct-chime textbox, the melody
            // replay (SetPlaybackSong in MSGMODE_SETUP_DISPLAY_SONG_PLAYED), the song-effect VFX, and
            // the real effect (Soaring menu, etc.). No slow note-by-note pre-play.
            if ((msgCtx->ocarinaAction == OCARINA_ACTION_FREE_PLAY) && (msgCtx->msgMode == MSGMODE_OCARINA_PLAYING)) {
                lusprintf(__FILE__, __LINE__, 2, "NEI-PP: phase2 staff up, forcing song=%d", sNeiPausePlaySong);
                // Deterministic handoff consumed by z_message.c (see gNeiPausePlayForcedSong); the
                // ForceSongPlayed call just shuts the ocarina input/flags down cleanly.
                gNeiPausePlayForcedSong = sNeiPausePlaySong;
                AudioOcarina_ForceSongPlayed((u8)sNeiPausePlaySong);
                sNeiPausePlayPhase = 0;
                sNeiPausePlaySong = -1;
            }
            break;

        default:
            sNeiPausePlayPhase = 0;
            break;
    }
}

// OoT quest cursor navigation (soh z_kaleido_collect.c D_8082A1AC): [point][dir] dir 0=up 1=down
// 2=left 3=right. Faithful hexagon/grid navigation.
//
// Skijer 2026-07-31 FIX: this table used to have 0xFF in EVERY slot soh spells 0xFE/0xFD, i.e. all
// six page-switch edges were flattened into dead ends — with the D-pad/stick-edge scroll also gated
// off in KaleidoScope_HandlePageToggles, the only way off this page was R/Z. Restored 1:1 from soh,
// with the sentinels spelled out instead of relying on the s8 wraparound of 0xFF/0xFE/0xFD.
#define OQ_NAV_NONE (-1)       // soh 0xFF: no neighbour in that direction, cursor stays put
#define OQ_NAV_PAGE_RIGHT (-2) // soh 0xFE
#define OQ_NAV_PAGE_LEFT (-3)  // soh 0xFD
static const s8 sOotQuestNav[26][4] = {
    { 0x05, 0x01, 0x05, OQ_NAV_PAGE_RIGHT },
    { 0x00, 0x02, 0x02, OQ_NAV_PAGE_RIGHT },
    { OQ_NAV_NONE, 0x13, 0x03, 0x01 },
    { 0x04, 0x02, 0x11, 0x02 },
    { 0x05, 0x03, 0x18, 0x05 },
    { OQ_NAV_NONE, OQ_NAV_NONE, 0x04, 0x00 },
    { 0x0C, OQ_NAV_NONE, OQ_NAV_PAGE_LEFT, 0x07 },
    { 0x0D, OQ_NAV_NONE, 0x06, 0x08 },
    { 0x0E, OQ_NAV_NONE, 0x07, 0x09 },
    { 0x0F, OQ_NAV_NONE, 0x08, 0x0A },
    { 0x10, OQ_NAV_NONE, 0x09, 0x0B },
    { 0x11, OQ_NAV_NONE, 0x0A, 0x12 },
    { 0x17, 0x06, OQ_NAV_PAGE_LEFT, 0x0D },
    { 0x17, 0x07, 0x0C, 0x0E },
    { 0x17, 0x08, 0x0D, 0x0F },
    { 0x18, 0x09, 0x0E, 0x10 },
    { 0x18, 0x0A, 0x0F, 0x11 },
    { 0x18, 0x0B, 0x10, 0x03 },
    { 0x02, OQ_NAV_NONE, 0x0B, 0x13 },
    { 0x02, OQ_NAV_NONE, 0x12, 0x14 },
    { 0x02, OQ_NAV_NONE, 0x13, OQ_NAV_PAGE_RIGHT },
    { OQ_NAV_NONE, 0x17, OQ_NAV_PAGE_LEFT, 0x16 },
    { OQ_NAV_NONE, 0x17, 0x15, 0x18 },
    { 0x15, 0x0C, OQ_NAV_PAGE_LEFT, 0x18 },
    { OQ_NAV_NONE, 0x10, 0x16, 0x04 },
    { 0x00, 0x00, 0x00, 0x00 },
};

// OOT_QUEST_POINT_HEART_PIECE (0x18) is reachable from six rows of the table above — it is the
// heart-piece pie, quad 24. See the point map next to sOotNamePaths.

static s32 OotQuest_Has(s32 bit); // defined below

// Is this OoT quest point currently owned? Used for the label/notes/A-action, NOT for navigation —
// OoT (and MM) both let the cursor rest on an empty slot, it just shows nothing.
static s32 OotQuest_PointOwned(s16 point) {
    if (point <= 0x17) {
        return OotQuest_Has(point); // ootQuestItems bit index == quest point for 0..0x17
    }
    if (point == OOT_QUEST_POINT_HEART_PIECE) {
        return (GET_SAVE_INVENTORY_QUEST_ITEMS >> QUEST_HEART_PIECE_COUNT) != 0;
    }
    return 0;
}

static s32 OotQuest_Has(s32 bit) {
    return (Nei_Save()->ootQuestItems & (1u << bit)) != 0;
}

// Build the 47-quad (188-vertex) OoT quest layout — 1:1 port of soh KaleidoScope_InitVertices'
// quest portion (D_8082B138/198/1F8). Medallions/upgrades keep the size table; songs + the middle
// icons (6..40) shrink by 4 and songs force width 16 (the note-shaped slot), which is why they must
// NOT be drawn as plain 24px squares.
static void OotQuest_SetupVtx(Vtx* vtx, s16 offsetY, u8 alpha, s16 cursorQuad) {
    s16 i;
    s16 j;
    for (i = 0, j = 0; i < 47; i++, j += 4) {
        s16 w = sOotVtxSize[i]; // texture-S width (tc), differs from the on-screen quad below
        if ((i < 6) || (i >= 41)) {
            vtx[j + 0].v.ob[0] = vtx[j + 2].v.ob[0] = sOotVtxX[i];
            vtx[j + 1].v.ob[0] = vtx[j + 3].v.ob[0] = vtx[j + 0].v.ob[0] + sOotVtxSize[i];
            vtx[j + 0].v.ob[1] = vtx[j + 1].v.ob[1] = sOotVtxY[i] + offsetY;
            vtx[j + 2].v.ob[1] = vtx[j + 3].v.ob[1] = vtx[j + 0].v.ob[1] - sOotVtxSize[i];
            if (i >= 41) { // GS-count digits: 8 wide, 16 tall
                vtx[j + 1].v.ob[0] = vtx[j + 3].v.ob[0] = vtx[j + 0].v.ob[0] + 8;
                vtx[j + 0].v.ob[1] = vtx[j + 1].v.ob[1] = sOotVtxY[i] + offsetY - 6;
                vtx[j + 2].v.ob[1] = vtx[j + 3].v.ob[1] = vtx[j + 0].v.ob[1] - 16;
                w = 8;
            }
        } else {
            if ((i >= 6) && (i <= 17)) {
                w = 16; // songs: force the note-texture width
            }
            vtx[j + 0].v.ob[0] = vtx[j + 2].v.ob[0] = sOotVtxX[i] + 2;
            vtx[j + 1].v.ob[0] = vtx[j + 3].v.ob[0] = vtx[j + 0].v.ob[0] + w - 4;
            vtx[j + 0].v.ob[1] = vtx[j + 1].v.ob[1] = sOotVtxY[i] + offsetY - 2;
            vtx[j + 2].v.ob[1] = vtx[j + 3].v.ob[1] = vtx[j + 0].v.ob[1] - sOotVtxSize[i] + 4;
        }
        vtx[j + 0].v.ob[2] = vtx[j + 1].v.ob[2] = vtx[j + 2].v.ob[2] = vtx[j + 3].v.ob[2] = 0;
        vtx[j + 0].v.flag = vtx[j + 1].v.flag = vtx[j + 2].v.flag = vtx[j + 3].v.flag = 0;
        vtx[j + 0].v.tc[0] = vtx[j + 0].v.tc[1] = vtx[j + 1].v.tc[1] = vtx[j + 2].v.tc[0] = 0;
        vtx[j + 1].v.tc[0] = vtx[j + 3].v.tc[0] = w << 5;
        vtx[j + 2].v.tc[1] = vtx[j + 3].v.tc[1] = sOotVtxSize[i] << 5;
        for (s16 k = 0; k < 4; k++) {
            vtx[j + k].v.cn[0] = vtx[j + k].v.cn[1] = vtx[j + k].v.cn[2] = 255;
            vtx[j + k].v.cn[3] = alpha;
        }
        // Skijer's NEI: grow the hovered quad a few px so it reads as "selected" (OoT scales the
        // cursor'd item up). tc is left alone → the texture just stretches to fill the bigger quad.
        if ((cursorQuad >= 0) && (i == cursorQuad)) {
            vtx[j + 0].v.ob[0] -= 3;
            vtx[j + 2].v.ob[0] -= 3;
            vtx[j + 1].v.ob[0] += 3;
            vtx[j + 3].v.ob[0] += 3;
            vtx[j + 0].v.ob[1] += 3;
            vtx[j + 1].v.ob[1] += 3;
            vtx[j + 2].v.ob[1] -= 3;
            vtx[j + 3].v.ob[1] -= 3;
        }
    }
}

// Draw one texture quad (RGBA32 icon) at a computed quad index — binds its 4 verts then draws.
static void OotQuest_DrawIcon(GraphicsContext* gfxCtx, Vtx* vtx, s16 quad, void* tex, u16 w, u16 h) {
    OPEN_DISPS(gfxCtx);
    gSPVertex(POLY_OPA_DISP++, &vtx[quad * 4], 4, 0);
    KaleidoScope_DrawTexQuadRGBA32(gfxCtx, tex, w, h, 0);
    CLOSE_DISPS(gfxCtx);
}

static void OotQuest_DrawSongStaff(PlayState* play); // hovered song's notes, on the staff (phased)

void KaleidoScope_DrawOotQuestStatus(PlayState* play) {
    // Medallion glow cycle state (soh D_8082A0D8/E4/F0 current RGB + D_8082A0FC timer + D_8082A100
    // phase). The env color lerps toward the active phase's target each frame → rotating halo.
    static s16 sMedR[6] = { 255, 255, 255, 255, 255, 255 };
    static s16 sMedG[6] = { 255, 255, 255, 255, 255, 255 };
    static s16 sMedB[6] = { 150, 150, 150, 150, 150, 150 };
    static s16 sMedTimer = 20;
    static s16 sMedPhase = 0;

    PauseContext* pauseCtx = &play->pauseCtx;
    GraphicsContext* gfxCtx = play->state.gfxCtx;
    Vtx* vtx = GRAPH_ALLOC(gfxCtx, 188 * sizeof(Vtx));
    s16 i;

    OPEN_DISPS(gfxCtx);

    // Hovered quad grows when interaction is on (point 0..0x18 maps 1:1 to quad index 0..24).
    s16 cursorQuad = (CVarGetInteger(CVAR_QUEST_INTERACT, 1) && (sOotQuestCursorPoint <= OOT_QUEST_POINT_MAX))
                         ? sOotQuestCursorPoint
                         : -1;
    OotQuest_SetupVtx(vtx, pauseCtx->offsetY, pauseCtx->alpha, cursorQuad);

    // Suppress MM's own info-panel name at draw time too (this runs before DrawInfoPanel this frame),
    // so its stale item-page name never bleeds under our name box regardless of update ordering.
    // itemDescriptionOn OR namedItem!=NONE both trigger MM's name draw (z_kaleido_scope_NES.c:1395), so
    // clear both.
    //
    // Skijer 2026-07-31 FIX: gated on pageIndex. namedItem/itemDescriptionOn are GLOBAL, and
    // KaleidoScope_DrawPages also draws the quest page as the adjacent page whenever the player is on
    // the map or the equipment page (z_kaleido_scope_NES.c:910) — before DrawInfoPanel runs. So this
    // pair was wiping the NAME and cancelling the item DESCRIPTION on those pages every frame.
    if (pauseCtx->pageIndex == PAUSE_QUEST) {
        pauseCtx->namedItem = PAUSE_ITEM_NONE;
        pauseCtx->itemDescriptionOn = false;
    }

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);
    gDPSetCombineLERP(POLY_OPA_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, PRIMITIVE,
                      ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);

    // Medallions (0..5) — env-color glow lerp (soh loop), skip the hold phases 1 & 3.
    sMedTimer--;
    for (i = 0; i < 6; i++) {
        if ((sMedPhase != 1) && (sMedPhase != 3)) {
            s16 t = (sMedPhase != 0) ? (i + 6) : i; // phase 0 → black target, phase 2 → colored halo
            if (sMedTimer != 0) {
                sMedR[i] += (ABS_ALT(sMedR[i] - sOotMedGlowTargets[t][0]) / sMedTimer) *
                            ((sMedR[i] >= sOotMedGlowTargets[t][0]) ? -1 : 1);
                sMedG[i] += (ABS_ALT(sMedG[i] - sOotMedGlowTargets[t][1]) / sMedTimer) *
                            ((sMedG[i] >= sOotMedGlowTargets[t][1]) ? -1 : 1);
                sMedB[i] += (ABS_ALT(sMedB[i] - sOotMedGlowTargets[t][2]) / sMedTimer) *
                            ((sMedB[i] >= sOotMedGlowTargets[t][2]) ? -1 : 1);
            } else {
                sMedR[i] = sOotMedGlowTargets[t][0];
                sMedG[i] = sOotMedGlowTargets[t][1];
                sMedB[i] = sOotMedGlowTargets[t][2];
            }
        }
        if (OotQuest_Has(OOT_QUEST_MEDALLION_FOREST + i)) {
            void* tex = OotQuest_Tex(sOotMedallionIconPaths[i]);
            if (tex != NULL) {
                gDPPipeSync(POLY_OPA_DISP++);
                gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);
                gDPSetEnvColor(POLY_OPA_DISP++, sMedR[i], sMedG[i], sMedB[i], 0);
                OotQuest_DrawIcon(gfxCtx, vtx, i, tex, 24, 24);
            }
        }
    }
    if (sMedTimer == 0) {
        sMedTimer = 20;
        if (++sMedPhase >= 4) {
            sMedPhase = 0;
        }
    }

    // Songs (6..17) — the note texture, one per learned song, tinted with its OoT color.
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);
    gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 255);
    gDPLoadTextureBlock(POLY_OPA_DISP++, gItemIconSongNoteTex, G_IM_FMT_IA, G_IM_SIZ_8b, 16, 24, 0,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                        G_TX_NOLOD);
    for (i = 0; i < 12; i++) {
        if (OotQuest_Has(OOT_QUEST_SONG_MINUET + i)) {
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, sOotSongR[i], sOotSongG[i], sOotSongB[i], pauseCtx->alpha);
            gSPVertex(POLY_OPA_DISP++, &vtx[(6 + i) * 4], 4, 0);
            gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);
        }
    }

    // Spiritual stones (18..20) — dim to 50% when the passive buff is off.
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);
    gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 255);
    for (i = 0; i < 3; i++) {
        if (OotQuest_Has(OOT_QUEST_KOKIRI_EMERALD + i)) {
            void* tex = OotQuest_Tex(sOotStoneIconPaths[i]);
            if (tex != NULL) {
                u8 a = SpiritualStone_IsPassiveActive(i) ? pauseCtx->alpha : (pauseCtx->alpha >> 1);
                gDPPipeSync(POLY_OPA_DISP++);
                gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, a);
                OotQuest_DrawIcon(gfxCtx, vtx, 18 + i, tex, 24, 24);
            }
        }
    }

    // Stone of Agony (21) + Gerudo Card (22) + Gold Skulltula icon (23).
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);
    if (OotQuest_Has(OOT_QUEST_STONE_OF_AGONY)) {
        void* tex = OotQuest_Tex(sOotAgonyIconPath);
        if (tex != NULL) {
            OotQuest_DrawIcon(gfxCtx, vtx, 21, tex, 24, 24);
        }
    }
    if (OotQuest_Has(OOT_QUEST_GERUDO_CARD)) {
        void* tex = OotQuest_Tex(sOotGerudoIconPath);
        if (tex != NULL) {
            OotQuest_DrawIcon(gfxCtx, vtx, 22, tex, 24, 24);
        }
    }
    if (OotQuest_Has(OOT_QUEST_SKULL_TOKEN)) {
        void* tex = OotQuest_Tex(sOotSkulltulaIconPath);
        if (tex != NULL) {
            OotQuest_DrawIcon(gfxCtx, vtx, 23, tex, 24, 24);
        }

        // GS-count digits — soh 1:1: bind the 6 digit quads at vtx[164] (quads 41-46), shadow pass
        // then colored pass, leading zeros suppressed. Digit textures are I8 8x16 (sCounterTextures).
        s16 c = Nei_Save()->ootGsCount;
        s16 digits[3];
        digits[0] = digits[1] = 0;
        digits[2] = c;
        while (digits[2] >= 100) {
            digits[0]++;
            digits[2] -= 100;
        }
        while (digits[2] >= 10) {
            digits[1]++;
            digits[2] -= 10;
        }

        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetCombineLERP(POLY_OPA_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0,
                          PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
        gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 0);
        gSPVertex(POLY_OPA_DISP++, &vtx[164], 24, 0); // quads 41..46 (3 shadow + 3 colored)
        for (s32 pass = 0, base = 0; pass < 2; pass++) {
            s32 drawn = 0;
            if (pass == 0) {
                gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 0, 0, 0, pauseCtx->alpha);
            } else {
                gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, (c == 100) ? 200 : 255, (c == 100) ? 50 : 255,
                                (c == 100) ? 50 : 255, pauseCtx->alpha);
            }
            for (s32 d = 0; d < 3; d++, base += 4) {
                if ((d >= 2) || (digits[d] != 0) || drawn) {
                    gDPLoadTextureBlock(POLY_OPA_DISP++, sCounterTextures[digits[d]], G_IM_FMT_I, G_IM_SIZ_8b, 8, 16, 0,
                                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                                        G_TX_NOLOD, G_TX_NOLOD);
                    gSP1Quadrangle(POLY_OPA_DISP++, base, base + 2, base + 3, base + 1, 0);
                    drawn = 1;
                }
            }
        }
    }

    // --- Heart-piece count (quad 24, 48x48 IA8 pie) ---
    // Skijer 2026-07-29 FIX: this quad was in the layout table but never drawn, so flipping to the
    // OoT layout lost the piece-of-heart indicator that MM's own page shows. The count is the SAME
    // save field in both layouts (questItems bits 28+), and MM's yar icon set has the pie textures,
    // so this is MM's native draw (Gfx_DrawTexQuadIA8, icons 0x7A..) on the OoT quad.
    {
        u32 hpCount = GET_SAVE_INVENTORY_QUEST_ITEMS >> QUEST_HEART_PIECE_COUNT;

        if (hpCount != 0) {
            gDPPipeSync(POLY_OPA_DISP++);
            gDPSetCombineLERP(POLY_OPA_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0,
                              PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 70, 50, pauseCtx->alpha);
            gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 255);
            gSPVertex(POLY_OPA_DISP++, &vtx[24 * 4], 4, 0);
            POLY_OPA_DISP = Gfx_DrawTexQuadIA8(POLY_OPA_DISP, gItemIcons[0x7A + hpCount], 48, 48, 0);
        }
    }

    // Hovered song's OoT note fingering, drawn on the parchment's music staff (bottom-left).
    OotQuest_DrawSongStaff(play);

    // Cursor — MM's spinning-circles cursor, drawn HERE in the page view (same as the icons) so it
    // stays aligned. 4 copies of gPauseMenuCursorTex orbit the selected slot's exact center, exactly
    // like KaleidoScope_DrawCursor (Math_SinS/CosS(spin + i*0x4000) * radius) but in our space.
    // cursorSpecialPos != 0 means the cursor has moved onto a page-scroll arrow, which MM draws
    // itself through pauseCtx->cursorVtx — drawing ours too would leave a second cursor stuck on the
    // last hovered slot.
    if (CVarGetInteger(CVAR_QUEST_INTERACT, 1) && (pauseCtx->cursorSpecialPos == 0) &&
        (sOotQuestCursorPoint <= OOT_QUEST_POINT_MAX)) {
        static s16 sSpin = 0;
        s16 q = sOotQuestCursorPoint * 4;
        f32 cx = (vtx[q].v.ob[0] + vtx[q + 1].v.ob[0]) * 0.5f;
        f32 cy = (vtx[q].v.ob[1] + vtx[q + 2].v.ob[1]) * 0.5f;
        f32 rx = 14.0f; // orbit radii (a touch wider than the icon)
        f32 ry = 14.0f;

        sSpin += 0x300;
        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetCombineLERP(POLY_OPA_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0,
                          PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);
        gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 255);
        gDPLoadTextureBlock(POLY_OPA_DISP++, gPauseMenuCursorTex, G_IM_FMT_IA, G_IM_SIZ_8b, 16, 16, 0,
                            G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                            G_TX_NOLOD);
        for (i = 0; i < 4; i++) {
            s16 ccx = (s16)(cx + Math_SinS(sSpin + i * 0x4000) * rx);
            s16 ccy = (s16)(cy + Math_CosS(sSpin + i * 0x4000) * ry);
            Vtx* cv = GRAPH_ALLOC(gfxCtx, 4 * sizeof(Vtx));
            cv[0].v.ob[0] = cv[2].v.ob[0] = ccx - 8;
            cv[1].v.ob[0] = cv[3].v.ob[0] = ccx + 8;
            cv[0].v.ob[1] = cv[1].v.ob[1] = ccy + 8;
            cv[2].v.ob[1] = cv[3].v.ob[1] = ccy - 8;
            cv[0].v.tc[0] = cv[2].v.tc[0] = 0;
            cv[1].v.tc[0] = cv[3].v.tc[0] = 16 << 5;
            cv[0].v.tc[1] = cv[1].v.tc[1] = 0;
            cv[2].v.tc[1] = cv[3].v.tc[1] = 16 << 5;
            for (s32 k = 0; k < 4; k++) {
                cv[k].v.ob[2] = 0;
                cv[k].v.flag = 0;
                cv[k].v.cn[0] = cv[k].v.cn[1] = cv[k].v.cn[2] = cv[k].v.cn[3] = 255;
            }
            gSPVertex(POLY_OPA_DISP++, cv, 4, 0);
            gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);
        }
        // The hovered item's NAME (its "preview") is drawn separately in KaleidoScope_DrawOotQuestName,
        // in the flat info-panel view at the bottom of the screen — the page perspective view here
        // projects a bottom name box off-screen, which is why it never showed.
    }

    CLOSE_DISPS(gfxCtx);
}

// Bottom name box (companion item_name_static IA4 128x16) for the hovered slot — its "preview", so
// hovering a song shows WHICH song. Drawn in the FLAT info-panel view (SetView 0,0,64), 1:1 with how
// MM draws pauseCtx->nameSegment (z_kaleido_scope_NES.c:1854): same infoPanelVtx position + combiner +
// Gfx_DrawTexQuad4b. Called from KaleidoScope_Draw right after KaleidoScope_DrawInfoPanel. Skijer's NEI.
void KaleidoScope_DrawOotQuestName(PlayState* play) {
    PauseContext* pauseCtx = &play->pauseCtx;
    GraphicsContext* gfxCtx = play->state.gfxCtx;
    void* nameTex;
    Vtx* nv;
    s16 y;

    // Skijer 2026-07-31 FIX: this used to draw whenever the quest page was the active page, with no
    // check on pauseCtx->state, mainState or cursorSpecialPos — and it runs AFTER
    // KaleidoScope_DrawInfoPanel (z_kaleido_scope_NES.c:3317 then :3322) onto the exact same strip
    // (infoPanelVtx[16..19] is also y = infoPanelOffsetY - 80, x -63..+65). So every time the player
    // scrolled off this page the info panel's "to Map"/"to Equipment" label was drawn first and this
    // name landed on top of it — two texts stacked. Same during the save prompt / game over.
    // Only draw where MM itself would draw an item name: main state, cursor on the page, slot owned.
    if (!CVarGetInteger(CVAR_OOT_QUEST_PAGE, 0) || !CVarGetInteger(CVAR_QUEST_INTERACT, 1) ||
        (pauseCtx->pageIndex != PAUSE_QUEST) || (pauseCtx->state != PAUSE_STATE_MAIN) ||
        (pauseCtx->cursorSpecialPos != 0) || (sOotQuestCursorPoint > OOT_QUEST_POINT_MAX) ||
        !OotQuest_PointOwned(sOotQuestCursorPoint)) {
        return;
    }

    // Existence probe only — the DRAW below passes the OTR PATH, not this pointer, so texture packs
    // still get to substitute the name by name (see the HD-path rule used across the kaleido).
    nameTex = OotQuest_Tex(sOotNamePaths[sOotQuestCursorPoint]);
    if (nameTex == NULL) {
        return;
    }

    OPEN_DISPS(gfxCtx);

    nv = GRAPH_ALLOC(gfxCtx, 4 * sizeof(Vtx));
    y = pauseCtx->infoPanelOffsetY - 80; // same bottom strip MM uses for the item name
    nv[0].v.ob[0] = nv[2].v.ob[0] = -63;
    nv[1].v.ob[0] = nv[3].v.ob[0] = -63 + 128;
    nv[0].v.ob[1] = nv[1].v.ob[1] = y;
    nv[2].v.ob[1] = nv[3].v.ob[1] = y - 16;
    nv[0].v.tc[0] = nv[2].v.tc[0] = 0;
    nv[1].v.tc[0] = nv[3].v.tc[0] = 128 << 5;
    nv[0].v.tc[1] = nv[1].v.tc[1] = 0;
    nv[2].v.tc[1] = nv[3].v.tc[1] = 16 << 5;
    for (s32 k = 0; k < 4; k++) {
        nv[k].v.ob[2] = 0;
        nv[k].v.flag = 0;
        nv[k].v.cn[0] = nv[k].v.cn[1] = nv[k].v.cn[2] = nv[k].v.cn[3] = 255;
    }

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetCombineLERP(POLY_OPA_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, PRIMITIVE,
                      ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
    gDPSetEnvColor(POLY_OPA_DISP++, 20, 30, 40, 0);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);
    gSPVertex(POLY_OPA_DISP++, nv, 4, 0);
    POLY_OPA_DISP =
        Gfx_DrawTexQuad4b(POLY_OPA_DISP, sOotNamePaths[sOotQuestCursorPoint], G_IM_FMT_IA, 128, 16, 0);

    CLOSE_DISPS(gfxCtx);
}

// Draw the hovered song's OoT note fingering ON the music staff baked into the OoT parchment (bottom
// left). Drawn in the PAGE (perspective) view so it lands on the staff, using MM's per-pitch staff
// heights (soh ocarinaButtonsY) + soh's staff-slot X. Phased like MM's own quest-song staff:
//   • hovering (IDLE)      -> the whole fingering, full brightness (read the song).
//   • SONG_PLAYBACK        -> notes reveal one-by-one as ocarinaStaff->pos advances (auto-play).
//   • SONG_PROMPT          -> the whole fingering dimmed (the target), the notes you've played lit.
// A button is blue, C buttons yellow. Skijer's NEI.
static void OotQuest_DrawSongStaff(PlayState* play) {
    // Per-button vertical position on the staff (soh z_kaleido_collect.c:1576 ocarinaButtonsY).
    static const s16 sStaffNoteY[5] = { -62, -56, -49, -46, -41 }; // A, Cdown, Cright, Cleft, Cup
    PauseContext* pauseCtx = &play->pauseCtx;
    GraphicsContext* gfxCtx = play->state.gfxCtx;
    u16 mainState = pauseCtx->mainState;
    s16 song;
    u8 n;
    u8 revealed; // notes drawn bright (progressive during play)
    u8 dimAll;   // also draw the not-yet-revealed notes, dimmed (prompt target)
    u8 bi;

    // Ownership matters now that the cursor is allowed to rest on empty slots (vanilla nav): without
    // this the staff happily previewed the fingering of a song the player has not learned.
    if ((sOotQuestCursorPoint < 6) || (sOotQuestCursorPoint > 0x11) || !OotQuest_Has(sOotQuestCursorPoint)) {
        return;
    }
    song = sOotQuestCursorPoint - 6;
    n = sOotSongButtonCount[song];

    if ((mainState == PAUSE_MAIN_STATE_SONG_PLAYBACK) || (mainState == PAUSE_MAIN_STATE_SONG_PROMPT)) {
        revealed = (pauseCtx->ocarinaStaff != NULL) ? pauseCtx->ocarinaStaff->pos : 0;
        if (revealed > n) {
            revealed = n;
        }
        dimAll = (mainState == PAUSE_MAIN_STATE_SONG_PROMPT); // show the whole target while you play
    } else {
        revealed = n; // hovering: show the full fingering
        dimAll = false;
    }

    OPEN_DISPS(gfxCtx);
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    for (bi = 0; bi < n; bi++) {
        u8 btn = sOotSongButtons[song][bi];
        u8 bright = (bi < revealed);
        u8 a;
        s16 nx, ny;
        Vtx* bv;

        if (!bright && !dimAll) {
            continue; // not revealed yet and not in the prompt -> don't draw it
        }
        a = bright ? pauseCtx->alpha : (pauseCtx->alpha >> 2); // dim target = quarter alpha

        nx = sOotVtxX[25 + bi]; // exact soh staff-slot X positions (quads 25..32 = -98..-14)
        ny = sStaffNoteY[btn];
        bv = GRAPH_ALLOC(gfxCtx, 4 * sizeof(Vtx));
        bv[0].v.ob[0] = bv[2].v.ob[0] = nx;
        bv[1].v.ob[0] = bv[3].v.ob[0] = nx + 14;
        bv[0].v.ob[1] = bv[1].v.ob[1] = ny;
        bv[2].v.ob[1] = bv[3].v.ob[1] = ny - 12;
        bv[0].v.tc[0] = bv[2].v.tc[0] = 0;
        bv[1].v.tc[0] = bv[3].v.tc[0] = 16 << 5;
        bv[0].v.tc[1] = bv[1].v.tc[1] = 0;
        bv[2].v.tc[1] = bv[3].v.tc[1] = 16 << 5;
        for (s32 kk = 0; kk < 4; kk++) {
            bv[kk].v.ob[2] = 0;
            bv[kk].v.flag = 0;
            bv[kk].v.cn[0] = bv[kk].v.cn[1] = bv[kk].v.cn[2] = bv[kk].v.cn[3] = 255;
        }
        if (btn == OCARINA_BTN_A) {
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 80, 150, 255, a); // A = blue
        } else {
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 50, a); // C = yellow
        }
        gSPVertex(POLY_OPA_DISP++, bv, 4, 0);
        POLY_OPA_DISP = Gfx_DrawTexQuadIA8(POLY_OPA_DISP, sOotOcarinaNoteTex[btn], 16, 16, 0);
    }
    CLOSE_DISPS(gfxCtx);
}

// ---------------------------------------------------------------------------
// Interaction — isolated cursor layer, invoked from KaleidoScope_UpdateQuestCursor with an early
// return so MM's own quest cursor logic never runs on the OoT page. Skijer's NEI.
// ---------------------------------------------------------------------------
// Skijer 2026-07-31 FIX: this used to WALK the nav chain until it found an owned slot, which is not
// what the table encodes. In soh the walk condition is KaleidoScope_UpdateQuestStatusPoint, and that
// function unconditionally `return 1` — so vanilla stops at the FIRST neighbour, owned or not, and
// simply shows nothing for an empty slot. Walking past empties with a table built for "stop
// immediately" is why most directions did nothing on a half-filled page: e.g. from 4 (Shadow) down ->
// 3 (Spirit) -> 2 (Water) -> 0x13 (Goron's Ruby) -> 0xFF, cursor never moved. Now 1:1 with vanilla.
static void OotQuest_MoveCursor(PlayState* play, s16 dir) {
    s16 next = sOotQuestNav[sOotQuestCursorPoint][dir];

    if (next == OQ_NAV_PAGE_LEFT) {
        KaleidoScope_MoveCursorToSpecialPos(play, PAUSE_CURSOR_PAGE_LEFT);
        return;
    }
    if (next == OQ_NAV_PAGE_RIGHT) {
        KaleidoScope_MoveCursorToSpecialPos(play, PAUSE_CURSOR_PAGE_RIGHT);
        return;
    }
    if (next == OQ_NAV_NONE) {
        return;
    }
    if (next != sOotQuestCursorPoint) {
        sOotQuestCursorPoint = next;
        Audio_PlaySfx(NA_SE_SY_CURSOR);
    }
}

// Play a learned song from the pause menu. For now: SFX confirmation + a latched request the
// in-world / combo-rando warp pass will consume later (per the design). Skijer's NEI.
s16 gOotQuestSongToPlay = -1; // song index 0..11 requested this pause, or -1

static void OotQuest_HandleSelect(PlayState* play, Input* input) {
    s16 point = sOotQuestCursorPoint;

    // Medallions (0..5): equip to a C button (C-left/down/right), or to a D-pad slot when the
    // DpadEquips enhancement is on. Item id comes from the table (medallions are NOT contiguous).
    if (point <= 5 && OotQuest_Has(point)) {
        u8 item = sOotMedallionItemIds[point];

        // C-button items live at form index 0 (shared across forms; only B differs per form) — same
        // idiom as the item page's BUTTON_ITEM_EQUIP(0, i) + Interface_LoadItemIconImpl.
        s16 cBtn = -1;
        if (CHECK_BTN_ALL(input->press.button, BTN_CLEFT)) {
            cBtn = EQUIP_SLOT_C_LEFT;
        } else if (CHECK_BTN_ALL(input->press.button, BTN_CDOWN)) {
            cBtn = EQUIP_SLOT_C_DOWN;
        } else if (CHECK_BTN_ALL(input->press.button, BTN_CRIGHT)) {
            cBtn = EQUIP_SLOT_C_RIGHT;
        }
        if (cBtn != -1) {
            BUTTON_ITEM_EQUIP(0, cBtn) = item;
            C_SLOT_EQUIP(0, cBtn) = 0xFF; // not from an inventory slot
            Interface_LoadItemIconImpl(play, (u8)cBtn);
            Audio_PlaySfx(NA_SE_SY_DECIDE);
            return;
        }

        // D-pad equip (same DPAD_BUTTON_ITEM_EQUIP(0, ...) + Interface_Dpad_LoadItemIcon idiom the
        // item page uses). Only when the enhancement is enabled, so the D-pad keeps its normal role
        // otherwise. On this page the D-pad no longer switches kaleido pages (HandlePageToggles gate).
        if (CVarGetInteger("gEnhancements.Dpad.DpadEquips", 0)) {
            s16 dBtn = -1;
            if (CHECK_BTN_ALL(input->press.button, BTN_DRIGHT)) {
                dBtn = EQUIP_SLOT_D_RIGHT;
            } else if (CHECK_BTN_ALL(input->press.button, BTN_DLEFT)) {
                dBtn = EQUIP_SLOT_D_LEFT;
            } else if (CHECK_BTN_ALL(input->press.button, BTN_DDOWN)) {
                dBtn = EQUIP_SLOT_D_DOWN;
            } else if (CHECK_BTN_ALL(input->press.button, BTN_DUP)) {
                dBtn = EQUIP_SLOT_D_UP;
            }
            if (dBtn != -1) {
                DPAD_BUTTON_ITEM_EQUIP(0, dBtn) = item;
                // Same 0xFF "not from an inventory slot" sentinel the C-button branch sets above —
                // otherwise this D-pad slot keeps the previous item's slot and the toggle/outline
                // logic that compares slots misreads the button.
                DPAD_SLOT_EQUIP(0, dBtn) = 0xFF;
                Interface_Dpad_LoadItemIcon(play, (u8)dBtn);
                Audio_PlaySfx(NA_SE_SY_DECIDE);
                return;
            }
        }
    }

    // Spiritual stones (0x12..0x14): A toggles the passive buff (swim/climb/walk speed);
    // C-left/down/right (or a D-pad slot with DpadEquips) equips the stone to a button, so the
    // in-game hold=summon / tap=warp path (SpiritualStone_TickHold) can read the held button.
    // The equip idiom mirrors the medallion block above 1:1.
    if (point >= 0x12 && point <= 0x14 && OotQuest_Has(point)) {
        // Extended (u16) sentinel ids (indexed by point - 0x12: 0=Kokiri, 1=Goron, 2=Zora). These are
        // above the u8 item space, so they can't live in buttonItems directly: they're equipped via
        // the extended-button infra — ExtButton_SetItem parks ITEM_EXT_BUTTON in buttonItems and the
        // real u16 in extButtons, and the ext-aware icon sites (Interface_LoadItemIconImpl / the HUD
        // draw) resolve it. Stones are C-button-only: the D-pad's dpadItems array is u8 and can't hold
        // a u16 ext id, so the D-pad equip branch (present in the medallion block above) is omitted.
        extern void ExtButton_SetItem(s32 form, s32 btn, u16 extId);
        static const u16 sSpiritualStoneItemIds[3] = {
            EXT_ITEM_SPIRITUAL_STONE_KOKIRI,
            EXT_ITEM_SPIRITUAL_STONE_GORON,
            EXT_ITEM_SPIRITUAL_STONE_ZORA,
        };
        u16 item = sSpiritualStoneItemIds[point - 0x12];

        s16 cBtn = -1;
        if (CHECK_BTN_ALL(input->press.button, BTN_CLEFT)) {
            cBtn = EQUIP_SLOT_C_LEFT;
        } else if (CHECK_BTN_ALL(input->press.button, BTN_CDOWN)) {
            cBtn = EQUIP_SLOT_C_DOWN;
        } else if (CHECK_BTN_ALL(input->press.button, BTN_CRIGHT)) {
            cBtn = EQUIP_SLOT_C_RIGHT;
        }
        if (cBtn != -1) {
            ExtButton_SetItem(0, cBtn, item);
            C_SLOT_EQUIP(0, cBtn) = 0xFF; // not from an inventory slot
            Interface_LoadItemIconImpl(play, (u8)cBtn);
            Audio_PlaySfx(NA_SE_SY_DECIDE);
            return;
        }

        if (CHECK_BTN_ALL(input->press.button, BTN_A)) {
            SpiritualStone_TogglePassive(point - 0x12); // 0=Kokiri 1=Goron 2=Zora
            Audio_PlaySfx(NA_SE_SY_DECIDE);
        }
        return;
    }

    // Stone of Agony (0x15): with the Quartz of Motion (level 2 of the progressive) owned, A opens
    // the tracking-category list. Without it the stone is a passive sense and A does nothing.
    if (point == QUARTZ_POINT && OotQuest_Has(OOT_QUEST_STONE_OF_AGONY)) {
        if (CHECK_BTN_ALL(input->press.button, BTN_A)) {
            if (Rando_DesireCompass_IsOwned()) {
                sQuartzMenuOpen = 1;
                sQuartzStickHeld = 1; // swallow the stick until it recenters
                sQuartzMenuIndex = (s8)Nei_Save()->quartzCategory;
                if (sQuartzMenuIndex < 0 || sQuartzMenuIndex >= (s8)DCOMPASS_CAT_MAX) {
                    sQuartzMenuIndex = 0;
                }
                Audio_PlaySfx(NA_SE_SY_DECIDE);
            } else {
                Audio_PlaySfx(NA_SE_SY_ERROR);
            }
        }
        return;
    }

    // Songs (6..0x11): press A to play the melody, driven through MM's REAL ocarina playback minigame
    // (same path the vanilla quest page uses), so it actually sounds + the staff can animate. The 6
    // songs shared with MM (Lullaby/Epona/Saria/Sun/Time/Storms) use MM's own note data via
    // AudioOcarina_SetPlaybackSong; the 6 OoT-only warp songs use our custom note buffer. mainState is
    // set to SONG_PLAYBACK exactly like vanilla; HandleCursor pulls it back to IDLE once it finishes.
    if (point >= 6 && point <= 0x11 && OotQuest_Has(point)) {
        if (CHECK_BTN_ALL(input->press.button, BTN_A)) {
            PauseContext* pauseCtx = &play->pauseCtx;
            s16 song = point - 6; // 0..11
            s8 mmIdx = sOotSongToMmOcarina[song];
            // "Pause play": with the enhancement on AND an ocarina in the ocarina slot (OoT or Fairy),
            // pressing A plays the song like the overworld — a quick playback + its latched effect —
            // instead of the learn-it minigame. The in-world / combo warp pass consumes gOotQuestSongToPlay.
            u8 pausePlay = CVarGetInteger(CVAR_PAUSE_PLAY, 0) && (INV_CONTENT(ITEM_OCARINA_OF_TIME) != ITEM_NONE);

            // Reset the played-note accumulator so the staff redraws the notes one-by-one this play.
            sQuestSongPlayedOcarinaButtonsNum = 0;
            for (u8 z = 0; z < 8; z++) {
                sQuestSongPlayedOcarinaButtons[z] = OCARINA_BTN_INVALID;
                sQuestSongPlayedOcarinaButtonsAlpha[z] = 0;
            }

            gOotQuestSongToPlay = song;

            if (pausePlay) {
                // "Pause Play": leave the menu — Link pulls out the ocarina and the song auto-plays
                // in-world (NeiPausePlay_Update drives it; MM songs fire their native effect).
                NeiPausePlay_Start(play, mmIdx);
                return;
            }

            // Learn-it minigame (default): auto-playback in the menu, then "play it yourself".
            // Every OoT song now has a real MM ocarina slot, so all 12 play through MM's own playback.
            AudioOcarina_SetInstrument(OCARINA_INSTRUMENT_DEFAULT);
            pauseCtx->ocarinaSongIndex = mmIdx;
            AudioOcarina_SetPlaybackSong(mmIdx + 1, 1);
            pauseCtx->mainState = PAUSE_MAIN_STATE_SONG_PLAYBACK;
            pauseCtx->ocarinaStaff = AudioOcarina_GetPlaybackStaff();
            pauseCtx->ocarinaStaff->pos = 0;
        }
    }
}

// Returns true if the OoT quest page owns input this frame (caller must skip MM's cursor logic).
s32 OotQuest_HandleCursor(PlayState* play) {
    PauseContext* pauseCtx = &play->pauseCtx;
    Input* input = &play->state.input[0];

    // Cursor works whenever the OoT quest page is shown and interaction is enabled (DEFAULT ON —
    // the earlier default-off was why the cursor "didn't move"). A on a song runs the learn-it minigame
    // by default; the "Pause Play" enhancement switches that to overworld-style play (with an ocarina).
    if (!CVarGetInteger(CVAR_OOT_QUEST_PAGE, 0) || !CVarGetInteger(CVAR_QUEST_INTERACT, 1) ||
        (pauseCtx->pageIndex != PAUSE_QUEST)) {
        // Not our page: make sure the Quartz modal can't stay latched across a page change.
        sQuartzMenuOpen = 0;
        sQuartzStickHeld = 0;
        return false;
    }

    // Suppress MM's own info-panel name on this page: its quest cursor is frozen here, so its
    // namedItem is left pointing at whatever was last hovered on the item page — which then bleeds
    // under our own name box. Force it to NONE so only KaleidoScope_DrawOotQuestName's name shows.
    pauseCtx->namedItem = PAUSE_ITEM_NONE;

    // Skijer 2026-07-31 FIX: KaleidoScope_UpdateQuestCursor is driven for the WHOLE
    // PAUSE_STATE_OPENING_3..PAUSE_STATE_SAVEPROMPT range (z_kaleido_scope_NES.c), and MM's own quest
    // handler re-filters on PAUSE_STATE_MAIN. This one never did, so the cursor moved, items equipped
    // and songs started during the open/close animation and while the save prompt was up (where the
    // same A press also answers "save?"). We still own the page — just take no input.
    //
    // This is also where the Quartz modal gets released: closing the menu with START used to leave
    // sQuartzMenuOpen latched, and the next time the player opened the pause menu the cursor was dead
    // because the modal swallowed everything and returned true.
    if (pauseCtx->state != PAUSE_STATE_MAIN) {
        sQuartzMenuOpen = 0;
        sQuartzStickHeld = 0;
        return true;
    }

    // Defensive clamp: every write to sOotQuestCursorPoint goes through the nav table, but a stale
    // CVar/save could still leave it out of range and index sOotQuestNav OOB.
    if ((sOotQuestCursorPoint < 0) || (sOotQuestCursorPoint > OOT_QUEST_POINT_MAX)) {
        sOotQuestCursorPoint = 0;
    }

    // --- Song minigame states -------------------------------------------------------------------
    // MM's own quest-page handler for these is skipped on our page, so we run the bits we need; the
    // top-level pause switch (z_kaleido_scope_NES.c:3589) still drives PLAYBACK->finish and the
    // PROMPT check/done for us. While a song is auto-playing or being played back, the ocarina engine
    // owns the controller — we must NOT run nav/equip/select (which would grab A / the C buttons).
    if (pauseCtx->mainState == PAUSE_MAIN_STATE_SONG_PROMPT_INIT) {
        // Auto-playback finished. Every OoT song now has a real MM ocarina slot (warp songs included),
        // so start the "play it yourself" prompt for all of them (1:1 with vanilla's SONG_PROMPT_INIT).
        if ((sOotQuestCursorPoint >= 6) && (sOotQuestCursorPoint <= 0x11)) {
            u8 z;
            for (z = 0; z < 8; z++) {
                sQuestSongPlayedOcarinaButtons[z] = OCARINA_BTN_INVALID;
                sQuestSongPlayedOcarinaButtonsAlpha[z] = 0;
            }
            sQuestSongPlayedOcarinaButtonsNum = 0;
            AudioOcarina_SetInstrument(OCARINA_INSTRUMENT_DEFAULT);
            if (pauseCtx->ocarinaSongIndex >= OCARINA_SONG_NEI_CUSTOM_FIRST) {
                // Custom songs (slots 30-32) have no bitmask bit (1<<30+ hits the mode flags) — use
                // the without-staff mode + the side availability mask instead (side recognition in
                // AudioOcarina_CheckSongsWithoutMusicStaff detects them).
                gNeiCustomSongsAvailable = (1 << (pauseCtx->ocarinaSongIndex - OCARINA_SONG_NEI_CUSTOM_FIRST));
                AudioOcarina_StartDefault(0xC0000000);
            } else {
                AudioOcarina_StartDefault((1 << pauseCtx->ocarinaSongIndex) | 0x80000000);
            }
            pauseCtx->ocarinaStaff = AudioOcarina_GetPlaybackStaff();
            pauseCtx->ocarinaStaff->pos = 0;
            pauseCtx->ocarinaStaff->state = 0xFE;
            pauseCtx->mainState = PAUSE_MAIN_STATE_SONG_PROMPT;
        } else {
            pauseCtx->mainState = PAUSE_MAIN_STATE_IDLE;
        }
        return true;
    }
    // Skijer 2026-07-31 FIX: the "play it yourself" prompt had NO way out. MM's own quest handler
    // aborts it as soon as the stick moves (KaleidoScope_UpdateQuestCursor below); this page only
    // returned true, so once you pressed A on a song you were pinned: no cursor, and no page change
    // either, because KaleidoScope_HandlePageToggles is only called while mainState is IDLE /
    // IDLE_CURSOR_ON_SONG. Worse for the NEI custom songs (slots 30-32): the prompt only ends when
    // ocarinaStaff->state == ocarinaSongIndex, and those start through the without-staff path
    // (AudioOcarina_StartDefault(0xC0000000)), so the state may never match and the prompt never
    // finished at all. Same stick-abort as vanilla.
    if (pauseCtx->mainState == PAUSE_MAIN_STATE_SONG_PROMPT) {
        if ((pauseCtx->stickAdjX != 0) || (pauseCtx->stickAdjY != 0)) {
            pauseCtx->mainState = PAUSE_MAIN_STATE_IDLE;
            AudioOcarina_SetInstrument(OCARINA_INSTRUMENT_OFF);
        }
        return true;
    }
    if ((pauseCtx->mainState == PAUSE_MAIN_STATE_SONG_PLAYBACK) ||
        (pauseCtx->mainState == PAUSE_MAIN_STATE_SONG_PROMPT_DONE)) {
        return true;
    }

    // --- Quartz of Motion list (modal) ---------------------------------------------------------
    // While open it OWNS the stick and A/B, exactly like the item page's cycle wheels
    // (z_kaleido_item.c: "an open wheel OWNS the stick ... skip all pause-cursor movement").
    // Returning true keeps MM's cursor logic from running underneath.
    if (sQuartzMenuOpen) {
        // Vertical nav with debounce so one tilt = one row.
        if ((pauseCtx->stickAdjY > 30) || (pauseCtx->stickAdjY < -30)) {
            if (!sQuartzStickHeld) {
                s8 count = (s8)DCOMPASS_CAT_MAX;
                sQuartzMenuIndex += (pauseCtx->stickAdjY > 30) ? -1 : 1;
                if (sQuartzMenuIndex < 0) {
                    sQuartzMenuIndex = (s8)(count - 1);
                } else if (sQuartzMenuIndex >= count) {
                    sQuartzMenuIndex = 0;
                }
                Audio_PlaySfx(NA_SE_SY_CURSOR);
                sQuartzStickHeld = 1;
            }
        } else {
            sQuartzStickHeld = 0;
        }

        if (CHECK_BTN_ALL(input->press.button, BTN_A)) {
            // Confirm: queue the activation and leave the pause menu. The heart
            // is charged in-world at the end of the attuning animation.
            if (Rando_DesireCompass_RequestActivation((DesireCompassCategory)sQuartzMenuIndex,
                                                       DCOMPASS_SUBCAT_ANY)) {
                Audio_PlaySfx(NA_SE_SY_DECIDE);
                sQuartzMenuOpen = 0;
                sQuartzStickHeld = 0;
                // Close the menu the same way the B/START path does.
                pauseCtx->state = PAUSE_STATE_UNPAUSE_SETUP;
                pauseCtx->mainState = PAUSE_MAIN_STATE_IDLE;
            } else {
                // Refused (not enough heart capacity, or not owned) — keep the
                // list up so the player can see nothing happened on purpose.
                Audio_PlaySfx(NA_SE_SY_ERROR);
            }
            return true;
        }

        if (CHECK_BTN_ALL(input->press.button, BTN_B)) {
            sQuartzMenuOpen = 0;
            sQuartzStickHeld = 0;
            Audio_PlaySfx(NA_SE_SY_CANCEL);
            return true;
        }
        return true;
    }

    // --- Page-scroll positions ------------------------------------------------------------------
    // The nav table can now park the cursor on PAUSE_CURSOR_PAGE_LEFT/RIGHT (see sOotQuestNav).
    // Pushing further in the same direction is KaleidoScope_HandlePageToggles' job (it scrolls the
    // kaleido); pushing back returns to the page, mirroring soh's own return points — 0x15 (Stone of
    // Agony) from the left edge, 0 (Forest Medallion) from the right.
    if (pauseCtx->cursorSpecialPos != 0) {
        // KaleidoScope_MoveCursorFromSpecialPos already plays NA_SE_SY_CURSOR and restores the
        // button statuses — don't double up the sfx here.
        if ((pauseCtx->cursorSpecialPos == PAUSE_CURSOR_PAGE_LEFT) && (pauseCtx->stickAdjX > 30)) {
            KaleidoScope_MoveCursorFromSpecialPos(play);
            sOotQuestCursorPoint = 0x15;
        } else if ((pauseCtx->cursorSpecialPos == PAUSE_CURSOR_PAGE_RIGHT) && (pauseCtx->stickAdjX < -30)) {
            KaleidoScope_MoveCursorFromSpecialPos(play);
            sOotQuestCursorPoint = 0;
        }
        return true;
    }

    // Stick / DPad navigation (thresholds + directions match MM's kaleido cursor).
    //
    // NOTE: no "snap to the first owned slot" here any more. It ran every frame, so the cursor was
    // yanked back the instant it rested on an empty slot — which the vanilla table expects it to do.
    if (pauseCtx->stickAdjX < -30) {
        OotQuest_MoveCursor(play, 2);
    } else if (pauseCtx->stickAdjX > 30) {
        OotQuest_MoveCursor(play, 3);
    }
    if (pauseCtx->stickAdjY < -30) {
        OotQuest_MoveCursor(play, 1);
    } else if (pauseCtx->stickAdjY > 30) {
        OotQuest_MoveCursor(play, 0);
    }

    // A left/right step may have moved us onto a page-scroll position; don't also run select there.
    if (pauseCtx->cursorSpecialPos != 0) {
        return true;
    }

    OotQuest_HandleSelect(play, input);
    return true;
}

s16 sQuestRemainsColorTimerInit[] = { 120, 60, 2, 80 };
s16 sQuestHpColorTimerInits[] = { 20, 4, 20, 10 };
s16 sQuestSongPlayedOcarinaButtonsNum = 0;
u8 sQuestSongPlayedOcarinaButtons[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
s16 sQuestSongPlayedOcarinaButtonsAlpha[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
s16 sQuestHpPrimColorTargets[][4] = {
    { 255, 0, 0, 255 },
    { 255, 70, 0, 150 },
    { 255, 70, 0, 150 },
    { 255, 0, 0, 255 },
};
s16 sQuestRemainsEnvTargets[][3] = {
    { 0, 0, 0 },    // QUEST_REMAINS_ODOLWA Target 1
    { 0, 0, 0 },    // QUEST_REMAINS_GOHT Target 1
    { 0, 0, 0 },    // QUEST_REMAINS_GYORG Target 1
    { 0, 0, 0 },    // QUEST_REMAINS_TWINMOLD Target 1
    { 0, 0, 0 },    // Unused
    { 0, 0, 0 },    // Unused
    { 0, 60, 0 },   // QUEST_REMAINS_ODOLWA Target 2
    { 90, 0, 0 },   // QUEST_REMAINS_GOHT Target 2
    { 0, 40, 110 }, // QUEST_REMAINS_GYORG Target 2
    { 80, 40, 0 },  // QUEST_REMAINS_TWINMOLD Target 2
    { 70, 0, 90 },  // Unused
    { 90, 90, 0 },  // Unused
};
s16 sQuestRemainsEnvRed[] = {
    255, // QUEST_REMAINS_ODOLWA
    255, // QUEST_REMAINS_GOHT
    255, // QUEST_REMAINS_GYORG
    255, // QUEST_REMAINS_TWINMOLD
    255, // Unused
    255, // Unused
};
s16 sQuestRemainsEnvGreen[] = {
    255, // QUEST_REMAINS_ODOLWA
    255, // QUEST_REMAINS_GOHT
    255, // QUEST_REMAINS_GYORG
    255, // QUEST_REMAINS_TWINMOLD
    255, // Unused
    255, // Unused
};
s16 sQuestRemainsEnvBlue[] = {
    150, // QUEST_REMAINS_ODOLWA
    150, // QUEST_REMAINS_GOHT
    150, // QUEST_REMAINS_GYORG
    150, // QUEST_REMAINS_TWINMOLD
    150, // Unused
    150, // Unused
};

// 2S2H [Port] (and in the function) don't do pointer math and access the list of digits directly.
extern const char* sCounterTextures[];

void KaleidoScope_DrawOotQuestStatus(PlayState* play);

void KaleidoScope_DrawQuestStatus(PlayState* play) {
    // Skijer's NEI: L on the quest page flips to the OoT collect layout (its own quest store).
    if (CVarGetInteger(CVAR_OOT_QUEST_PAGE, 0)) {
        KaleidoScope_DrawOotQuestStatus(play);
        return;
    }

    static s16 sQuestRemainsColorTimer = 20;
    static s16 sQuestRemainsColorTimerIndex = 0;
    static s16 sQuestHpPrimRed = 0;
    static s16 sQuestHpPrimGreen = 0;
    static s16 sQuestHpPrimBlue = 0;
    static s16 sQuestHpPrimAlpha = 0;
    static s16 sQuestHpColorTimer = 20;
    static s16 sQuestHpPrimColorTargetIndex = 0;
    static TexturePtr sOcarinaButtonTextures[] = {
        gOcarinaATex, gOcarinaCDownTex, gOcarinaCRightTex, gOcarinaCLeftTex, gOcarinaCUpTex,
    };
    static s16 sQuestSongsPrimRed[] = {
        150, // QUEST_SONG_SONATA
        255, // QUEST_SONG_LULLABY
        100, // QUEST_SONG_BOSSA_NOVA
        255, // QUEST_SONG_ELEGY
        255, // QUEST_SONG_OATH
        255, // QUEST_SONG_SARIA
        255, // QUEST_SONG_TIME
        255, // QUEST_SONG_HEALING
        255, // QUEST_SONG_EPONA
        255, // QUEST_SONG_SOARING
        255, // QUEST_SONG_STORMS
        255, // QUEST_SONG_SUN
    };
    static s16 sQuestSongsPrimGreen[] = {
        255, // QUEST_SONG_SONATA
        80,  // QUEST_SONG_LULLABY
        150, // QUEST_SONG_BOSSA_NOVA
        160, // QUEST_SONG_ELEGY
        100, // QUEST_SONG_OATH
        240, // QUEST_SONG_SARIA
        255, // QUEST_SONG_TIME
        255, // QUEST_SONG_HEALING
        255, // QUEST_SONG_EPONA
        255, // QUEST_SONG_SOARING
        255, // QUEST_SONG_STORMS
        255, // QUEST_SONG_SUN
    };
    static s16 sQuestSongsPrimBlue[] = {
        100, // QUEST_SONG_SONATA
        40,  // QUEST_SONG_LULLABY
        255, // QUEST_SONG_BOSSA_NOVA
        0,   // QUEST_SONG_ELEGY
        255, // QUEST_SONG_OATH
        100, // QUEST_SONG_SARIA
        255, // QUEST_SONG_TIME
        255, // QUEST_SONG_HEALING
        255, // QUEST_SONG_EPONA
        255, // QUEST_SONG_SOARING
        255, // QUEST_SONG_STORMS
        255, // QUEST_SONG_SUN
    };
    static TexturePtr sQuestUpgradeTextures[][3] = {
        { gItemIconQuiver30Tex, gItemIconQuiver40Tex, gItemIconQuiver50Tex },    // UPG_QUIVER
        { gItemIconBombBag20Tex, gItemIconBombBag30Tex, gItemIconBombBag40Tex }, // UPG_BOMB_BAG
    };
    static u8 sQuestUpgrades[] = { UPG_QUIVER, UPG_BOMB_BAG };
    PauseContext* pauseCtx = &play->pauseCtx;
    s16 sp1CA; // colorSetR and numOcarinaButtons
    s16 sp1C8; // colorSetG and ocarinaButtonIndex
    s16 sp1C6;
    s16 sp1C4;
    s16 var_v1;
    s16 i;
    s16 j;
    s16 k;
    s16 skullTokenDigits[3];
    u16 isDigitDrawn;
    u32* questItemsPtr;

    OPEN_DISPS(play->state.gfxCtx);

    KaleidoScope_SetCursorVtxPos(pauseCtx, pauseCtx->cursorSlot[PAUSE_QUEST] * 4, pauseCtx->questVtx);

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);
    gDPSetCombineLERP(POLY_OPA_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0, PRIMITIVE,
                      ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);

    // Draw the Boss Remains icons
    sQuestRemainsColorTimer--;
    for (i = 0, j = 0; i < 4; i++, j += 4) {
        if ((sQuestRemainsColorTimerIndex != 1) && (sQuestRemainsColorTimerIndex != 3)) {
            var_v1 = (sQuestRemainsColorTimerIndex != 0) ? (i + 6) : i;

            if (sQuestRemainsColorTimer != 0) {
                sp1CA = ABS_ALT(sQuestRemainsEnvRed[i] - sQuestRemainsEnvTargets[var_v1][0]) / sQuestRemainsColorTimer;
                sp1C8 =
                    ABS_ALT(sQuestRemainsEnvGreen[i] - sQuestRemainsEnvTargets[var_v1][1]) / sQuestRemainsColorTimer;
                sp1C6 = ABS_ALT(sQuestRemainsEnvBlue[i] - sQuestRemainsEnvTargets[var_v1][2]) / sQuestRemainsColorTimer;

                if (sQuestRemainsEnvRed[i] >= sQuestRemainsEnvTargets[var_v1][0]) {
                    sQuestRemainsEnvRed[i] -= sp1CA;
                } else {
                    sQuestRemainsEnvRed[i] += sp1CA;
                }
                if (sQuestRemainsEnvGreen[i] >= sQuestRemainsEnvTargets[var_v1][1]) {
                    sQuestRemainsEnvGreen[i] -= sp1C8;
                } else {
                    sQuestRemainsEnvGreen[i] += sp1C8;
                }
                if (sQuestRemainsEnvBlue[i] >= sQuestRemainsEnvTargets[var_v1][2]) {
                    sQuestRemainsEnvBlue[i] -= sp1C6;
                } else {
                    sQuestRemainsEnvBlue[i] += sp1C6;
                }
            } else {
                sQuestRemainsEnvRed[i] = sQuestRemainsEnvTargets[var_v1][0];
                sQuestRemainsEnvGreen[i] = sQuestRemainsEnvTargets[var_v1][1];
                sQuestRemainsEnvBlue[i] = sQuestRemainsEnvTargets[var_v1][2];
            }
        }

        if (CHECK_QUEST_ITEM(i)) {
            gDPPipeSync(POLY_OPA_DISP++);
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);
            gDPSetEnvColor(POLY_OPA_DISP++, sQuestRemainsEnvRed[i], sQuestRemainsEnvGreen[i], sQuestRemainsEnvBlue[i],
                           0);
            gSPVertex(POLY_OPA_DISP++, &pauseCtx->questVtx[j], 4, 0);
            KaleidoScope_DrawTexQuadRGBA32(play->state.gfxCtx, gItemIcons[ITEM_REMAINS_ODOLWA + i], 32, 32, 0);
        }
    }

    if (sQuestRemainsColorTimer == 0) {
        sQuestRemainsColorTimer = sQuestRemainsColorTimerInit[sQuestRemainsColorTimerIndex];
        if (++sQuestRemainsColorTimerIndex > 3) {
            sQuestRemainsColorTimerIndex = 0;
        }
    }

    // STRENGTH / SWIM upgrades (Skijer 2026-07-29 kaleido re-layout).
    // These two quads used to mirror the equipped shield and sword. Both live on the equipment page,
    // so the quads now carry the two capacity upgrades that moved OFF that page's left column:
    // quad QUEST_SHIELD = strength (bracelet/gauntlets), quad QUEST_SWORD = swim (silver/golden
    // scale). Values come from the NEI parallel OoT-upgrade store (ootUpgrades: strength@9 scale@12)
    // and the icons from the player's OoT archive, exactly like the equipment page drew them.
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);
    gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);

    {
        static const char* sStrengthIcons[3] = {
            "__OTR__textures/icon_item_static/gItemIconGoronsBraceletTex",
            "__OTR__textures/icon_item_static/gItemIconSilverGauntletsTex",
            "__OTR__textures/icon_item_static/gItemIconGoldenGauntletsTex",
        };
        static const char* sScaleIcons[2] = {
            // NOTE the spelling: oot.o2r has ScaleSilver/ScaleGolden, NOT SilverScale/GoldenScale
            // (verified against the archive; the old equipment-page code had it backwards and silently
            // drew nothing because it was FileExists-gated).
            "__OTR__textures/icon_item_static/gItemIconScaleSilverTex",
            "__OTR__textures/icon_item_static/gItemIconScaleGoldenTex",
        };
        s16 upg = (Nei_Save()->ootUpgrades >> 9) & 7; // strength
        void* tex;

        if (upg > 0) {
            tex = OotQuest_Tex(sStrengthIcons[((upg > 3) ? 3 : upg) - 1]);
            if (tex != NULL) {
                gSPVertex(POLY_OPA_DISP++, &pauseCtx->questVtx[j], 4, 0);
                KaleidoScope_DrawTexQuadRGBA32(play->state.gfxCtx, tex, 32, 32, 0);
            }
        }

        j += 4;

        upg = (Nei_Save()->ootUpgrades >> 12) & 7; // swim (scale)
        if (upg > 0) {
            tex = OotQuest_Tex(sScaleIcons[((upg > 2) ? 2 : upg) - 1]);
            if (tex != NULL) {
                gSPVertex(POLY_OPA_DISP++, &pauseCtx->questVtx[j], 4, 0);
                KaleidoScope_DrawTexQuadRGBA32(play->state.gfxCtx, tex, 32, 32, 0);
            }
        }

        j += 4;
    }

    // Draw Songs
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);
    gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 255);

    gDPLoadTextureBlock(POLY_OPA_DISP++, gItemIconSongNoteTex, G_IM_FMT_IA, G_IM_SIZ_8b, 16, 24, 0,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                        G_TX_NOLOD);

    for (i = 0; i < 12; i++, j += 4) {
        if (CHECK_QUEST_ITEM(i + QUEST_SONG_SONATA) ||
            ((i == (QUEST_SONG_LULLABY - QUEST_SONG_SONATA)) && !CHECK_QUEST_ITEM(i + QUEST_SONG_SONATA) &&
             CHECK_QUEST_ITEM(QUEST_SONG_LULLABY_INTRO))) {
            if ((i + QUEST_SONG_SONATA) == pauseCtx->cursorSlot[PAUSE_QUEST]) {
                pauseCtx->questVtx[j + 0].v.ob[0] = pauseCtx->questVtx[j + 2].v.ob[0] =
                    pauseCtx->questVtx[j + 0].v.ob[0] - 2;

                pauseCtx->questVtx[j + 1].v.ob[0] = pauseCtx->questVtx[j + 3].v.ob[0] =
                    pauseCtx->questVtx[j + 1].v.ob[0] + 4;

                pauseCtx->questVtx[j + 0].v.ob[1] = pauseCtx->questVtx[j + 1].v.ob[1] =
                    pauseCtx->questVtx[j + 0].v.ob[1] + 2;

                pauseCtx->questVtx[j + 2].v.ob[1] = pauseCtx->questVtx[j + 3].v.ob[1] =
                    pauseCtx->questVtx[j + 2].v.ob[1] - 4;
            }

            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, sQuestSongsPrimRed[i], sQuestSongsPrimGreen[i],
                            sQuestSongsPrimBlue[i], pauseCtx->alpha);
            gSPVertex(POLY_OPA_DISP++, &pauseCtx->questVtx[j], 4, 0);
            gSP1Quadrangle(POLY_OPA_DISP++, 0, 2, 3, 1, 0);
        }
    }

    // Draw Bombers Notebook
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);
    gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 255);

    if (CHECK_QUEST_ITEM(QUEST_BOMBERS_NOTEBOOK)) {
        gSPVertex(POLY_OPA_DISP++, &pauseCtx->questVtx[j], 4, 0);
        KaleidoScope_DrawTexQuadRGBA32(play->state.gfxCtx, gItemIconBombersNotebookTex, 32, 32, 0);
    }

    j += 4;

    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);

    // Loop over quest item upgrades
    for (i = 0; i < 2; i++, j += 4) {
        gSPVertex(POLY_OPA_DISP++, &pauseCtx->questVtx[j], 4, 0);
        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);

        if (GET_CUR_UPG_VALUE(sQuestUpgrades[i]) != 0) {
            KaleidoScope_DrawTexQuadRGBA32(
                play->state.gfxCtx, sQuestUpgradeTextures[i][GET_CUR_UPG_VALUE(sQuestUpgrades[i]) - 1], 32, 32, 0);
        }
    }

    // Skip over &pauseCtx->questVtx[84], which should be `QUEST_SKULL_TOKEN`
    j += 4;

    sp1CA = ABS_ALT(sQuestHpPrimRed - sQuestHpPrimColorTargets[sQuestHpPrimColorTargetIndex][0]) / sQuestHpColorTimer;
    sp1C8 = ABS_ALT(sQuestHpPrimGreen - sQuestHpPrimColorTargets[sQuestHpPrimColorTargetIndex][1]) / sQuestHpColorTimer;
    sp1C6 = ABS_ALT(sQuestHpPrimBlue - sQuestHpPrimColorTargets[sQuestHpPrimColorTargetIndex][2]) / sQuestHpColorTimer;
    sp1C4 = ABS_ALT(sQuestHpPrimAlpha - sQuestHpPrimColorTargets[sQuestHpPrimColorTargetIndex][3]) / sQuestHpColorTimer;

    if (sQuestHpPrimRed >= sQuestHpPrimColorTargets[sQuestHpPrimColorTargetIndex][0]) {
        sQuestHpPrimRed -= sp1CA;
    } else {
        sQuestHpPrimRed += sp1CA;
    }

    if (sQuestHpPrimGreen >= sQuestHpPrimColorTargets[sQuestHpPrimColorTargetIndex][1]) {
        sQuestHpPrimGreen -= sp1C8;
    } else {
        sQuestHpPrimGreen += sp1C8;
    }

    if (sQuestHpPrimBlue >= sQuestHpPrimColorTargets[sQuestHpPrimColorTargetIndex][2]) {
        sQuestHpPrimBlue -= sp1C6;
    } else {
        sQuestHpPrimBlue += sp1C6;
    }

    if (sQuestHpPrimAlpha >= sQuestHpPrimColorTargets[sQuestHpPrimColorTargetIndex][3]) {
        sQuestHpPrimAlpha -= sp1C4;
    } else {
        sQuestHpPrimAlpha += sp1C4;
    }

    sQuestHpColorTimer--;
    if (sQuestHpColorTimer == 0) {
        sQuestHpPrimRed = sQuestHpPrimColorTargets[sQuestHpPrimColorTargetIndex][0];
        sQuestHpPrimGreen = sQuestHpPrimColorTargets[sQuestHpPrimColorTargetIndex][1];
        sQuestHpPrimBlue = sQuestHpPrimColorTargets[sQuestHpPrimColorTargetIndex][2];
        sQuestHpPrimAlpha = sQuestHpPrimColorTargets[sQuestHpPrimColorTargetIndex][3];
        sQuestHpColorTimer = sQuestHpColorTimerInits[sQuestHpPrimColorTargetIndex];
        if (++sQuestHpPrimColorTargetIndex > 3) {
            sQuestHpPrimColorTargetIndex = 0;
        }
    }

    //! FAKE: Used to load `0xF0000000` early
    if ((GET_SAVE_INVENTORY_QUEST_ITEMS & 0xF0000000) != 0) {}
    questItemsPtr = &gSaveContext.save.saveInfo.inventory.questItems;
    if (1) {}

    if ((*questItemsPtr >> QUEST_HEART_PIECE_COUNT) != 0) {
        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetCombineLERP(POLY_OPA_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0,
                          PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);

        if ((pauseCtx->state == PAUSE_STATE_OPENING_3) || (pauseCtx->state == PAUSE_STATE_UNPAUSE_SETUP)) {
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, sQuestHpPrimColorTargets[0][0], sQuestHpPrimColorTargets[0][1],
                            sQuestHpPrimColorTargets[0][2], pauseCtx->alpha);
        } else {
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, sQuestHpPrimRed, sQuestHpPrimGreen, sQuestHpPrimBlue,
                            sQuestHpPrimAlpha);
        }

        gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 255);
        gSPVertex(POLY_OPA_DISP++, &pauseCtx->questVtx[j], 4, 0);

        POLY_OPA_DISP = Gfx_DrawTexQuadIA8(
            POLY_OPA_DISP,
            gItemIcons[(0x7A + ((GET_SAVE_INVENTORY_QUEST_ITEMS & 0xF0000000) >> QUEST_HEART_PIECE_COUNT))], 48, 48, 0);
    }

    j += 4;

    if (pauseCtx->state == PAUSE_STATE_MAIN) {
        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);

        if (pauseCtx->mainState == PAUSE_MAIN_STATE_SONG_PLAYBACK) {
            // Draw ocarina buttons as they are played back
            pauseCtx->ocarinaStaff = AudioOcarina_GetPlaybackStaff();

            if (pauseCtx->ocarinaStaff->pos != 0) {
                if (sQuestSongPlayedOcarinaButtonsNum == (pauseCtx->ocarinaStaff->pos - 1)) {
                    sQuestSongPlayedOcarinaButtonsNum++;
                    sQuestSongPlayedOcarinaButtons[pauseCtx->ocarinaStaff->pos - 1] =
                        pauseCtx->ocarinaStaff->buttonIndex;
                }

                for (i = 0, k = 0; i < 8; i++, k += 4, j += 4) {
                    if (sQuestSongPlayedOcarinaButtons[i] == OCARINA_BTN_INVALID) {
                        break;
                    }

                    if (sQuestSongPlayedOcarinaButtonsAlpha[i] != 255) {
                        sQuestSongPlayedOcarinaButtonsAlpha[i] += 50;
                        if (sQuestSongPlayedOcarinaButtonsAlpha[i] >= 255) {
                            sQuestSongPlayedOcarinaButtonsAlpha[i] = 255;
                        }
                    }

                    pauseCtx->questVtx[j + 0].v.ob[1] = pauseCtx->questVtx[j + 1].v.ob[1] =
                        pauseCtx->ocarinaButtonsY[sQuestSongPlayedOcarinaButtons[i]];

                    pauseCtx->questVtx[j + 2].v.ob[1] = pauseCtx->questVtx[j + 3].v.ob[1] =
                        pauseCtx->questVtx[j + 0].v.ob[1] - 12;

                    gDPPipeSync(POLY_OPA_DISP++);

                    if (sQuestSongPlayedOcarinaButtons[i] == OCARINA_BTN_A) {
                        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 80, 150, 255, sQuestSongPlayedOcarinaButtonsAlpha[i]);
                    } else {
                        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 50, sQuestSongPlayedOcarinaButtonsAlpha[i]);
                    }

                    gSPVertex(POLY_OPA_DISP++, &pauseCtx->questVtx[j], 4, 0);

                    POLY_OPA_DISP = Gfx_DrawTexQuadIA8(
                        POLY_OPA_DISP, sOcarinaButtonTextures[sQuestSongPlayedOcarinaButtons[i]], 16, 16, 0);
                }
            }
        } else if (IS_PAUSE_MAIN_STATE_SONG_PROMPT(pauseCtx) ||
                   (pauseCtx->mainState == PAUSE_MAIN_STATE_IDLE_CURSOR_ON_SONG)) {
            // Draw the buttons for playing a song
            sp1C8 = pauseCtx->ocarinaSongIndex;
            sp1CA = gOcarinaSongButtons[sp1C8].numButtons;

            i = j;

            for (k = 0; k < sp1CA; k++, j += 4) {
                pauseCtx->questVtx[j + 0].v.ob[1] = pauseCtx->questVtx[j + 1].v.ob[1] =
                    pauseCtx->ocarinaButtonsY[gOcarinaSongButtons[sp1C8].buttonIndex[k]];

                pauseCtx->questVtx[j + 2].v.ob[1] = pauseCtx->questVtx[j + 3].v.ob[1] =
                    pauseCtx->questVtx[j + 0].v.ob[1] - 12;

                gDPPipeSync(POLY_OPA_DISP++);

                if (pauseCtx->mainState == PAUSE_MAIN_STATE_IDLE_CURSOR_ON_SONG) {
                    // Draw ocarina buttons colored
                    if (gOcarinaSongButtons[sp1C8].buttonIndex[k] == OCARINA_BTN_A) {
                        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 80, 150, 255, 200);
                    } else {
                        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 50, 200);
                    }
                } else {
                    // Gray out buttons while reading ocarina song inputs
                    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 150, 150, 150, 150);
                }

                gSPVertex(POLY_OPA_DISP++, &pauseCtx->questVtx[j], 4, 0);

                POLY_OPA_DISP = Gfx_DrawTexQuadIA8(
                    POLY_OPA_DISP, sOcarinaButtonTextures[gOcarinaSongButtons[sp1C8].buttonIndex[k]], 16, 16, 0);
            }

            if (pauseCtx->mainState != PAUSE_MAIN_STATE_IDLE_CURSOR_ON_SONG) {
                pauseCtx->ocarinaStaff = AudioOcarina_GetPlayingStaff();

                // Update ocarina song inputs
                if (pauseCtx->ocarinaStaff->pos != 0) {
                    if (sQuestSongPlayedOcarinaButtonsNum == (pauseCtx->ocarinaStaff->pos - 1)) {
                        if (pauseCtx->ocarinaStaff->buttonIndex <= OCARINA_BTN_C_UP) {
                            sQuestSongPlayedOcarinaButtons[pauseCtx->ocarinaStaff->pos - 1] =
                                pauseCtx->ocarinaStaff->buttonIndex;
                            sQuestSongPlayedOcarinaButtons[pauseCtx->ocarinaStaff->pos] = OCARINA_BTN_INVALID;
                            sQuestSongPlayedOcarinaButtonsNum++;
                        }
                    }
                }

                // Draw the buttons colored as the ocarina song inputs are read from
                j = i + 32;
                k = 0;
                for (; k < 8; k++, j += 4) {
                    if (sQuestSongPlayedOcarinaButtons[k] == OCARINA_BTN_INVALID) {
                        continue;
                    }

                    if (sQuestSongPlayedOcarinaButtonsAlpha[k] != 255) {
                        sQuestSongPlayedOcarinaButtonsAlpha[k] += 50;
                        if (sQuestSongPlayedOcarinaButtonsAlpha[k] >= 255) {
                            sQuestSongPlayedOcarinaButtonsAlpha[k] = 255;
                        }
                    }

                    pauseCtx->questVtx[j + 0].v.ob[1] = pauseCtx->questVtx[j + 1].v.ob[1] =
                        pauseCtx->ocarinaButtonsY[sQuestSongPlayedOcarinaButtons[k]];

                    pauseCtx->questVtx[j + 2].v.ob[1] = pauseCtx->questVtx[j + 3].v.ob[1] =
                        pauseCtx->questVtx[j + 0].v.ob[1] - 12;

                    gDPPipeSync(POLY_OPA_DISP++);

                    if (sQuestSongPlayedOcarinaButtons[k] == OCARINA_BTN_A) {
                        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 80, 150, 255, sQuestSongPlayedOcarinaButtonsAlpha[k]);
                    } else {
                        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 50, sQuestSongPlayedOcarinaButtonsAlpha[k]);
                    }

                    gSPVertex(POLY_OPA_DISP++, &pauseCtx->questVtx[j], 4, 0);

                    POLY_OPA_DISP = Gfx_DrawTexQuadIA8(
                        POLY_OPA_DISP, sOcarinaButtonTextures[sQuestSongPlayedOcarinaButtons[k]], 16, 16, 0);
                }

                if (pauseCtx->mainState == PAUSE_MAIN_STATE_SONG_PROMPT_INIT) {
                    for (k = 0; k < 8; k++) {
                        sQuestSongPlayedOcarinaButtons[k] = OCARINA_BTN_INVALID;
                        sQuestSongPlayedOcarinaButtonsAlpha[k] = 0;
                    }

                    sQuestSongPlayedOcarinaButtonsNum = 0;
                    AudioOcarina_SetInstrument(OCARINA_INSTRUMENT_DEFAULT);
                    AudioOcarina_StartDefault((1 << pauseCtx->ocarinaSongIndex) | 0x80000000);
                    pauseCtx->ocarinaStaff = AudioOcarina_GetPlaybackStaff();
                    pauseCtx->ocarinaStaff->pos = 0;
                    pauseCtx->ocarinaStaff->state = 0xFE;
                    pauseCtx->mainState = PAUSE_MAIN_STATE_SONG_PROMPT;
                }
            }
        }
    }

    // Draw Skull Token Count
    // QUEST_SKULL_TOKEN never properly set, see Item_Give(),
    // Vertices not well placed, digits are not aligned and placed in unintended positions
    if (CHECK_QUEST_ITEM(QUEST_SKULL_TOKEN) && ((play->sceneId == SCENE_KINSTA1) || (play->sceneId == SCENE_KINDAN2))) {
        gDPPipeSync(POLY_OPA_DISP++);
        gDPSetCombineLERP(POLY_OPA_DISP++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0,
                          PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
        gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 0);

        skullTokenDigits[0] = skullTokenDigits[1] = 0;
        skullTokenDigits[2] = Inventory_GetSkullTokenCount(play->sceneId);

        while (skullTokenDigits[2] >= 100) {
            skullTokenDigits[0]++;
            skullTokenDigits[2] -= 100;
        }

        while (skullTokenDigits[2] >= 10) {
            skullTokenDigits[1]++;
            skullTokenDigits[2] -= 10;
        }

        //! @bug: &pauseCtx->questVtx[84] is the questVtx for skull tokens
        gSPVertex(POLY_OPA_DISP++, &pauseCtx->questVtx[152], 24, 0);

        // Loop over two sets of digits, the first is shadowed, the second is colored
        for (k = 0, i = 0; k < 2; k++) {
            if (k == 0) {
                // shadow
                gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 0, 0, 0, pauseCtx->alpha);
            } else {
                if (Inventory_GetSkullTokenCount(play->sceneId) == 100) {
                    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 200, 50, 50, pauseCtx->alpha);
                } else {
                    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);
                }
            }

            isDigitDrawn = false;
            for (j = 0; j < 3; j++, i += 4) {
                if ((j >= 2) || (skullTokenDigits[j] != 0) || isDigitDrawn) {
                    // 2S2H [Port] Don't do pointer math and access the texture directly.
                    gDPLoadTextureBlock(POLY_OPA_DISP++, sCounterTextures[skullTokenDigits[j]], G_IM_FMT_I, G_IM_SIZ_8b,
                                        8, 16, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK,
                                        G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

                    gSP1Quadrangle(POLY_OPA_DISP++, i, i + 2, i + 3, i + 1, 0);

                    isDigitDrawn = true;
                }
            }
        }
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

typedef enum {
    /* -3 */ CURSOR_TO_LEFT = -3, // Cursor on the "scroll to left page" position
    /* -2 */ CURSOR_TO_RIGHT,     // Cursor on the "scroll to right page" position
    /* -1 */ CURSOR_NONE          // No position in that direction, cursor stays where it is
} CursorPointNext;

typedef struct {
    /* 0x0 */ s16 up;
    /* 0x2 */ s16 down;
    /* 0x4 */ s16 left;
    /* 0x6 */ s16 right;
} CursorPointDirection; // size = 0x8

void KaleidoScope_UpdateQuestCursor(PlayState* play) {
    static s16 sQuestSongPlaybackDelayTimer = 0;
    static CursorPointDirection sCursorPointLinks[] = {
        { CURSOR_NONE, QUEST_REMAINS_TWINMOLD, QUEST_REMAINS_GYORG, QUEST_REMAINS_GOHT },     // QUEST_REMAINS_ODOLWA
        { QUEST_REMAINS_ODOLWA, QUEST_SHIELD, QUEST_REMAINS_TWINMOLD, CURSOR_TO_RIGHT },      // QUEST_REMAINS_GOHT
        { QUEST_REMAINS_ODOLWA, QUEST_SWORD, QUEST_HEART_PIECE, QUEST_REMAINS_TWINMOLD },     // QUEST_REMAINS_GYORG
        { QUEST_REMAINS_ODOLWA, QUEST_SHIELD, QUEST_REMAINS_GYORG, QUEST_REMAINS_GOHT },      // QUEST_REMAINS_TWINMOLD
        { QUEST_REMAINS_GOHT, QUEST_BOMB_BAG, QUEST_SWORD, CURSOR_TO_RIGHT },                 // QUEST_SHIELD
        { QUEST_REMAINS_GYORG, QUEST_QUIVER, QUEST_SONG_STORMS, QUEST_SHIELD },               // QUEST_SWORD
        { QUEST_SONG_TIME, CURSOR_NONE, CURSOR_TO_LEFT, QUEST_SONG_LULLABY },                 // QUEST_SONG_SONATA
        { QUEST_SONG_HEALING, CURSOR_NONE, QUEST_SONG_SONATA, QUEST_SONG_BOSSA_NOVA },        // QUEST_SONG_LULLABY
        { QUEST_SONG_EPONA, CURSOR_NONE, QUEST_SONG_LULLABY, QUEST_SONG_ELEGY },              // QUEST_SONG_BOSSA_NOVA
        { QUEST_SONG_SOARING, CURSOR_NONE, QUEST_SONG_BOSSA_NOVA, QUEST_SONG_OATH },          // QUEST_SONG_ELEGY
        { QUEST_SONG_STORMS, CURSOR_NONE, QUEST_SONG_ELEGY, QUEST_QUIVER },                   // QUEST_SONG_OATH
        { QUEST_SONG_SUN, CURSOR_NONE, QUEST_SONG_OATH, QUEST_SWORD },                        // QUEST_SONG_SARIA
        { QUEST_BOMBERS_NOTEBOOK, QUEST_SONG_SONATA, CURSOR_TO_LEFT, QUEST_SONG_HEALING },    // QUEST_SONG_TIME
        { QUEST_BOMBERS_NOTEBOOK, QUEST_SONG_LULLABY, QUEST_SONG_TIME, QUEST_SONG_EPONA },    // QUEST_SONG_HEALING
        { QUEST_HEART_PIECE, QUEST_SONG_BOSSA_NOVA, QUEST_SONG_HEALING, QUEST_SONG_SOARING }, // QUEST_SONG_EPONA
        { QUEST_HEART_PIECE, QUEST_SONG_ELEGY, QUEST_SONG_EPONA, QUEST_SONG_STORMS },         // QUEST_SONG_SOARING
        { QUEST_HEART_PIECE, QUEST_SONG_OATH, QUEST_SONG_SOARING, QUEST_SWORD },              // QUEST_SONG_STORMS
        { QUEST_HEART_PIECE, QUEST_SONG_SARIA, QUEST_SONG_STORMS, QUEST_SWORD },              // QUEST_SONG_SUN
        { CURSOR_NONE, QUEST_SONG_TIME, CURSOR_TO_LEFT, QUEST_HEART_PIECE },                  // QUEST_BOMBERS_NOTEBOOK
        { QUEST_SWORD, CURSOR_NONE, QUEST_SONG_OATH, QUEST_BOMB_BAG },                        // QUEST_QUIVER
        { QUEST_SHIELD, CURSOR_NONE, QUEST_QUIVER, CURSOR_TO_RIGHT },                         // QUEST_BOMB_BAG
        { QUEST_QUIVER, QUEST_SONG_TIME, CURSOR_TO_LEFT, QUEST_HEART_PIECE },                 // QUEST_SKULL_TOKEN
        { CURSOR_NONE, QUEST_SONG_STORMS, QUEST_BOMBERS_NOTEBOOK, QUEST_REMAINS_GYORG },      // QUEST_HEART_PIECE
    };
    PauseContext* pauseCtx = &play->pauseCtx;
    MessageContext* msgCtx = &play->msgCtx;
    InterfaceContext* interfaceCtx = &play->interfaceCtx;
    s32 pad;
    s16 nextCursorPoint;
    s16 oldCursorPoint;
    s16 i;
    u16 cursor;
    u16 cursorItem;

    // Skijer's NEI: on the OoT quest page (L-flip) with interaction enabled, our own cursor layer
    // fully owns input — MM's quest cursor logic below never runs (no state bleed on the OoT page).
    if (OotQuest_HandleCursor(play)) {
        return;
    }

    pauseCtx->nameColorSet = PAUSE_NAME_COLOR_SET_WHITE;
    pauseCtx->cursorColorSet = PAUSE_CURSOR_COLOR_SET_WHITE;

    // != PAUSE_MAIN_STATE_IDLE
    if ((pauseCtx->state == PAUSE_STATE_MAIN) &&
        (!pauseCtx->mainState || (pauseCtx->mainState == PAUSE_MAIN_STATE_SONG_PROMPT) ||
         (pauseCtx->mainState == PAUSE_MAIN_STATE_IDLE_CURSOR_ON_SONG)) &&
        (pauseCtx->pageIndex == PAUSE_QUEST) && !pauseCtx->itemDescriptionOn) {
        if (pauseCtx->cursorSpecialPos == 0) {
            oldCursorPoint = pauseCtx->cursorPoint[PAUSE_QUEST];

            if (pauseCtx->stickAdjX < -30) {
                // Move cursor left
                if (pauseCtx->mainState == PAUSE_MAIN_STATE_SONG_PROMPT) {
                    AudioOcarina_SetInstrument(OCARINA_INSTRUMENT_OFF);
                }
                pauseCtx->cursorShrinkRate = 4.0f;

                nextCursorPoint = sCursorPointLinks[oldCursorPoint].left;
                if (nextCursorPoint == CURSOR_TO_LEFT) {
                    KaleidoScope_MoveCursorToSpecialPos(play, PAUSE_CURSOR_PAGE_LEFT);
                    pauseCtx->mainState = PAUSE_MAIN_STATE_IDLE;
                    if (interfaceCtx->aButtonDoActionDelayed == DO_ACTION_DECIDE) {
                        Interface_SetAButtonDoAction(play, DO_ACTION_INFO);
                    }
                    return;
                } else {
                    while (nextCursorPoint > CURSOR_NONE) {
                        if (KaleidoScope_UpdateQuestStatusPoint(pauseCtx, nextCursorPoint)) {
                            break;
                        }
                        nextCursorPoint = sCursorPointLinks[nextCursorPoint].left;
                    }
                }
            } else if (pauseCtx->stickAdjX > 30) {
                // Move cursor right
                if (pauseCtx->mainState == PAUSE_MAIN_STATE_SONG_PROMPT) {
                    AudioOcarina_SetInstrument(OCARINA_INSTRUMENT_OFF);
                }
                pauseCtx->cursorShrinkRate = 4.0f;
                nextCursorPoint = sCursorPointLinks[oldCursorPoint].right;

                if (nextCursorPoint == CURSOR_TO_RIGHT) {
                    KaleidoScope_MoveCursorToSpecialPos(play, PAUSE_CURSOR_PAGE_RIGHT);
                    pauseCtx->mainState = PAUSE_MAIN_STATE_IDLE;
                    return;
                }

                while (nextCursorPoint > CURSOR_NONE) {
                    if (KaleidoScope_UpdateQuestStatusPoint(pauseCtx, nextCursorPoint)) {
                        break;
                    }
                    nextCursorPoint = sCursorPointLinks[nextCursorPoint].right;
                }
            }

            if (pauseCtx->stickAdjY < -30) {
                // Move cursor down
                if (pauseCtx->mainState == PAUSE_MAIN_STATE_SONG_PROMPT) {
                    AudioOcarina_SetInstrument(OCARINA_INSTRUMENT_OFF);
                }
                nextCursorPoint = sCursorPointLinks[oldCursorPoint].down;

                while (nextCursorPoint > CURSOR_NONE) {
                    pauseCtx->cursorShrinkRate = 4.0f;
                    if (KaleidoScope_UpdateQuestStatusPoint(pauseCtx, nextCursorPoint)) {
                        break;
                    }
                    nextCursorPoint = sCursorPointLinks[nextCursorPoint].down;
                }
            } else if (pauseCtx->stickAdjY > 30) {
                // Move cursor up
                if (pauseCtx->mainState == PAUSE_MAIN_STATE_SONG_PROMPT) {
                    AudioOcarina_SetInstrument(OCARINA_INSTRUMENT_OFF);
                }
                nextCursorPoint = sCursorPointLinks[oldCursorPoint].up;
                while (nextCursorPoint > CURSOR_NONE) {
                    pauseCtx->cursorShrinkRate = 4.0f;
                    if (KaleidoScope_UpdateQuestStatusPoint(pauseCtx, nextCursorPoint)) {
                        break;
                    }
                    nextCursorPoint = sCursorPointLinks[nextCursorPoint].up;
                }
            }

            // if the cursor point changed
            if (oldCursorPoint != pauseCtx->cursorPoint[PAUSE_QUEST]) {
                pauseCtx->mainState = PAUSE_MAIN_STATE_IDLE;
                Audio_PlaySfx(NA_SE_SY_CURSOR);
            }

            // Update cursor item and slot
            if (pauseCtx->cursorPoint[PAUSE_QUEST] != QUEST_HEART_PIECE) {
                if (pauseCtx->cursorPoint[PAUSE_QUEST] <= QUEST_REMAINS_TWINMOLD) {
                    // Boss Remains
                    if (CHECK_QUEST_ITEM(pauseCtx->cursorPoint[PAUSE_QUEST])) {
                        cursorItem = ITEM_REMAINS_ODOLWA + pauseCtx->cursorPoint[PAUSE_QUEST];
                    } else {
                        cursorItem = PAUSE_ITEM_NONE;
                    }
                } else if (pauseCtx->cursorPoint[PAUSE_QUEST] == QUEST_BOMBERS_NOTEBOOK) {
                    // Bombers Notebook
                    if (CHECK_QUEST_ITEM(pauseCtx->cursorPoint[PAUSE_QUEST])) {
                        cursorItem = ITEM_BOMBERS_NOTEBOOK;
                    } else {
                        cursorItem = PAUSE_ITEM_NONE;
                    }
                } else if ((pauseCtx->cursorPoint[PAUSE_QUEST] == QUEST_SHIELD) ||
                           (pauseCtx->cursorPoint[PAUSE_QUEST] == QUEST_SWORD)) {
                    // Skijer 2026-07-29: these two quads are the STRENGTH and SWIM upgrades now (they
                    // moved off the equipment page's left column). MM has no name texture for the
                    // bracelet/gauntlets/scales, so the cell shows no label — same as it did there.
                    cursorItem = PAUSE_ITEM_NONE;
                } else if (pauseCtx->cursorPoint[PAUSE_QUEST] <= QUEST_SONG_SUN) {
                    // Songs
                    if (CHECK_QUEST_ITEM(pauseCtx->cursorPoint[PAUSE_QUEST])) {
                        cursorItem = ITEM_WALLET_GIANT + pauseCtx->cursorPoint[PAUSE_QUEST];
                    } else if ((pauseCtx->cursorPoint[PAUSE_QUEST] == QUEST_SONG_LULLABY) &&
                               CHECK_QUEST_ITEM(QUEST_SONG_LULLABY_INTRO)) {
                        cursorItem = ITEM_SONG_LULLABY_INTRO;
                    } else {
                        cursorItem = PAUSE_ITEM_NONE;
                    }
                } else if (pauseCtx->cursorPoint[PAUSE_QUEST] == QUEST_QUIVER) {
                    // Quiver Upgrade
                    if (GET_CUR_UPG_VALUE(UPG_QUIVER) != 0) {
                        cursorItem = (ITEM_QUIVER_30 - 1) + GET_CUR_UPG_VALUE(UPG_QUIVER);
                    } else {
                        cursorItem = PAUSE_ITEM_NONE;
                    }
                } else if (pauseCtx->cursorPoint[PAUSE_QUEST] == QUEST_BOMB_BAG) {
                    // Bomb Bag Upgrade
                    if (GET_CUR_UPG_VALUE(UPG_BOMB_BAG) != 0) {
                        cursorItem = (ITEM_BOMB_BAG_20 - 1) + GET_CUR_UPG_VALUE(UPG_BOMB_BAG);
                    } else {
                        cursorItem = PAUSE_ITEM_NONE;
                    }
                } else {
                    cursorItem = PAUSE_ITEM_NONE;
                }
            } else {
                // Heart Piece Count
                if ((GET_SAVE_INVENTORY_QUEST_ITEMS >> QUEST_HEART_PIECE_COUNT) != 0) {
                    cursorItem = ITEM_HEART_CONTAINER;
                } else {
                    cursorItem = PAUSE_ITEM_NONE;
                }
            }

            cursor = pauseCtx->cursorPoint[PAUSE_QUEST];
            pauseCtx->cursorItem[pauseCtx->pageIndex] = cursorItem;
            pauseCtx->cursorSlot[pauseCtx->pageIndex] = cursor;

            if ((pauseCtx->debugEditor == DEBUG_EDITOR_NONE) && (pauseCtx->state == PAUSE_STATE_MAIN) &&
                (pauseCtx->mainState == PAUSE_MAIN_STATE_IDLE) && (pauseCtx->cursorSpecialPos == 0)) {
                if ((cursor >= QUEST_SONG_SONATA) && (cursor <= QUEST_SONG_SUN)) {
                    // Handle part of the ocarina songs playback
                    if ((CHECK_QUEST_ITEM(pauseCtx->cursorPoint[PAUSE_QUEST]) ||
                         ((cursor == QUEST_SONG_LULLABY) && !CHECK_QUEST_ITEM(pauseCtx->cursorPoint[PAUSE_QUEST]) &&
                          CHECK_QUEST_ITEM(QUEST_SONG_LULLABY_INTRO))) &&
                        (msgCtx->msgLength == 0)) {
                        // The cursor is on a learned song
                        // Set some things up for song playback

                        if (CHECK_QUEST_ITEM(pauseCtx->cursorPoint[PAUSE_QUEST])) {
                            cursor = pauseCtx->cursorSlot[PAUSE_QUEST];
                        } else {
                            cursor = QUEST_BOMB_BAG;
                        }

                        pauseCtx->ocarinaSongIndex = gOcarinaSongItemMap[cursor - QUEST_SONG_SONATA];
                        sQuestSongPlaybackDelayTimer = 10;

                        for (i = 0; i < 8; i++) {
                            sQuestSongPlayedOcarinaButtons[i] = OCARINA_BTN_INVALID;
                            sQuestSongPlayedOcarinaButtonsAlpha[i] = 0;
                        }

                        sQuestSongPlayedOcarinaButtonsNum = 0;

                        // Setup the song to receive user input, immediately cancelled below
                        AudioOcarina_SetInstrument(OCARINA_INSTRUMENT_DEFAULT);
                        AudioOcarina_StartDefault((1 << pauseCtx->ocarinaSongIndex) | 0x80000000);

                        // Clear the playback staff
                        pauseCtx->ocarinaStaff = AudioOcarina_GetPlaybackStaff();
                        pauseCtx->ocarinaStaff->pos = 0;
                        pauseCtx->ocarinaStaff->state = 0xFF;

                        pauseCtx->ocarinaButtonsY[OCARINA_BTN_A] = -62;
                        pauseCtx->ocarinaButtonsY[OCARINA_BTN_C_DOWN] = -56;
                        pauseCtx->ocarinaButtonsY[OCARINA_BTN_C_RIGHT] = -49;
                        pauseCtx->ocarinaButtonsY[OCARINA_BTN_C_LEFT] = -46;
                        pauseCtx->ocarinaButtonsY[OCARINA_BTN_C_UP] = -41;

                        pauseCtx->mainState = PAUSE_MAIN_STATE_IDLE_CURSOR_ON_SONG;

                        if (interfaceCtx->aButtonDoActionDelayed != DO_ACTION_DECIDE) {
                            Interface_SetAButtonDoAction(play, DO_ACTION_DECIDE);
                        }

                        // Stop receiving input to play a song as mentioned above
                        AudioOcarina_SetInstrument(OCARINA_INSTRUMENT_OFF);

                        if (gSaveContext.buttonStatus[EQUIP_SLOT_A] == BTN_DISABLED) {
                            gSaveContext.buttonStatus[EQUIP_SLOT_A] = BTN_ENABLED;
                            gSaveContext.hudVisibility = HUD_VISIBILITY_IDLE;
                            Interface_SetHudVisibility(HUD_VISIBILITY_ALL);
                        }
                    } else {
                        if (interfaceCtx->aButtonDoActionDelayed != DO_ACTION_DECIDE) {
                            Interface_SetAButtonDoAction(play, DO_ACTION_DECIDE);
                        }
                        if (gSaveContext.buttonStatus[EQUIP_SLOT_A] != BTN_DISABLED) {
                            gSaveContext.buttonStatus[EQUIP_SLOT_A] = BTN_DISABLED;
                            gSaveContext.hudVisibility = HUD_VISIBILITY_IDLE;
                            Interface_SetHudVisibility(HUD_VISIBILITY_ALL);
                        }
                    }
                } else {
                    if ((cursor == QUEST_BOMBERS_NOTEBOOK) && (pauseCtx->cursorItem[PAUSE_QUEST] != PAUSE_ITEM_NONE)) {
                        if (interfaceCtx->aButtonDoActionDelayed != DO_ACTION_DECIDE) {
                            Interface_SetAButtonDoAction(play, DO_ACTION_DECIDE);
                        }
                        pauseCtx->cursorColorSet = PAUSE_CURSOR_COLOR_SET_BLUE;
                    } else if (interfaceCtx->aButtonDoActionDelayed == DO_ACTION_DECIDE) {
                        Interface_SetAButtonDoAction(play, DO_ACTION_INFO);
                    }

                    if ((pauseCtx->cursorItem[PAUSE_QUEST] != PAUSE_ITEM_NONE) && (msgCtx->msgLength == 0)) {
                        if (gSaveContext.buttonStatus[EQUIP_SLOT_A] == BTN_DISABLED) {
                            gSaveContext.buttonStatus[EQUIP_SLOT_A] = BTN_ENABLED;
                            gSaveContext.hudVisibility = HUD_VISIBILITY_IDLE;
                            Interface_SetHudVisibility(HUD_VISIBILITY_ALL);
                        }

                        // Skijer's NEI: if the cursor is on a boss remains, C-left/down/right (or a
                        // D-pad slot with DpadEquips) equips it to that button as a wearable "mask".
                        // No-op for any other quest item; C/D-pad presses don't collide with the A
                        // (info/decide) handling just below.
                        {
                            extern s32 BossRemains_TryEquipAtCursor(PlayState * play, Input * input);
                            BossRemains_TryEquipAtCursor(play, CONTROLLER1(&play->state));
                        }

                        if (CHECK_BTN_ALL(CONTROLLER1(&play->state)->press.button, BTN_A) && (msgCtx->msgLength == 0)) {
                            if (pauseCtx->cursorPoint[PAUSE_QUEST] == QUEST_BOMBERS_NOTEBOOK) {
                                play->pauseCtx.bombersNotebookOpen = true;
                                pauseCtx->mainState = PAUSE_MAIN_STATE_BOMBERS_NOTEBOOK_OPEN;
                                Audio_PlaySfx(NA_SE_SY_DECIDE);
                            } else {
                                pauseCtx->itemDescriptionOn = true;
                                if (pauseCtx->cursorYIndex[PAUSE_QUEST] < 2) {
                                    if (pauseCtx->cursorItem[PAUSE_QUEST] < ITEM_REMAINS_ODOLWA) {
                                        func_801514B0(play, 0x1737 + pauseCtx->cursorItem[PAUSE_QUEST], 1);
                                    } else {
                                        func_801514B0(play, 0x173B + pauseCtx->cursorItem[PAUSE_QUEST], 3);
                                    }
                                } else {
                                    if (pauseCtx->cursorItem[PAUSE_QUEST] < ITEM_REMAINS_ODOLWA) {
                                        func_801514B0(play, 0x1737 + pauseCtx->cursorItem[PAUSE_QUEST], 1);
                                    } else {
                                        func_801514B0(play, 0x173B + pauseCtx->cursorItem[PAUSE_QUEST], 1);
                                    }
                                }
                            }
                        }
                    } else if (gSaveContext.buttonStatus[EQUIP_SLOT_A] != BTN_DISABLED) {
                        gSaveContext.buttonStatus[EQUIP_SLOT_A] = BTN_DISABLED;
                        gSaveContext.hudVisibility = HUD_VISIBILITY_IDLE;
                        Interface_SetHudVisibility(HUD_VISIBILITY_ALL);
                    }
                }
            } else if (pauseCtx->mainState == PAUSE_MAIN_STATE_SONG_PROMPT) {
                // Abort reading ocarina song input if the stick is moved
                if ((pauseCtx->stickAdjX != 0) || (pauseCtx->stickAdjY != 0)) {
                    pauseCtx->mainState = PAUSE_MAIN_STATE_IDLE;
                    AudioOcarina_SetInstrument(OCARINA_INSTRUMENT_OFF);
                }
            } else if ((pauseCtx->mainState == PAUSE_MAIN_STATE_IDLE_CURSOR_ON_SONG) &&
                       CHECK_BTN_ALL(CONTROLLER1(&play->state)->press.button, BTN_A) && (msgCtx->msgLength == 0) &&
                       (cursor >= QUEST_SONG_SONATA) && (cursor <= QUEST_SONG_SUN)) {
                // Skijer's NEI "Pause Play": with the enhancement on and an ocarina owned, A leaves the
                // menu and auto-plays the song on the real ocarina (native effect flow), instead of the
                // in-menu playback. Applies to MM's songs here just like the OoT page's.
                if (CVarGetInteger("gEnhancements.SkijerNEI.PausePlay", 0) &&
                    (INV_CONTENT(ITEM_OCARINA_OF_TIME) != ITEM_NONE)) {
                    NeiPausePlay_Start(play, pauseCtx->ocarinaSongIndex);
                } else {
                    pauseCtx->mainState = PAUSE_MAIN_STATE_SONG_PLAYBACK_INIT;
                    sQuestSongPlaybackDelayTimer = 10;
                }
            }

            if (pauseCtx->cursorSpecialPos == 0) {
                if ((pauseCtx->cursorSlot[PAUSE_QUEST] >= 6) && (pauseCtx->cursorSlot[PAUSE_QUEST] < 0x12) &&
                    ((pauseCtx->mainState <= PAUSE_MAIN_STATE_SONG_PLAYBACK) ||
                     (pauseCtx->mainState == PAUSE_MAIN_STATE_SONG_PROMPT) ||
                     (pauseCtx->mainState == PAUSE_MAIN_STATE_IDLE_CURSOR_ON_SONG)) &&
                    (pauseCtx->cursorItem[pauseCtx->pageIndex] != PAUSE_ITEM_NONE)) {
                    pauseCtx->cursorColorSet = PAUSE_CURSOR_COLOR_SET_BLUE;
                    if ((pauseCtx->mainState >= PAUSE_MAIN_STATE_SONG_PLAYBACK) &&
                        (pauseCtx->mainState <= PAUSE_MAIN_STATE_SONG_PROMPT_DONE)) {
                        pauseCtx->cursorColorSet = PAUSE_CURSOR_COLOR_SET_WHITE;
                    }
                }
            }
        } else if (pauseCtx->cursorSpecialPos == PAUSE_CURSOR_PAGE_LEFT) {
            if (pauseCtx->stickAdjX > 30) {
                if (pauseCtx->mainState == PAUSE_MAIN_STATE_SONG_PROMPT) {
                    AudioOcarina_SetInstrument(OCARINA_INSTRUMENT_OFF);
                }

                KaleidoScope_MoveCursorFromSpecialPos(play);

                pauseCtx->cursorPoint[PAUSE_QUEST] = QUEST_BOMBERS_NOTEBOOK;
                if (CHECK_QUEST_ITEM(QUEST_BOMBERS_NOTEBOOK)) {
                    cursorItem = ITEM_BOMBERS_NOTEBOOK;
                } else {
                    cursorItem = PAUSE_ITEM_NONE;
                }

                cursor = pauseCtx->cursorPoint[PAUSE_QUEST];
                pauseCtx->cursorItem[pauseCtx->pageIndex] = cursorItem;
                pauseCtx->cursorSlot[pauseCtx->pageIndex] = cursor;
            }
        } else if (pauseCtx->stickAdjX < -30) {
            if (pauseCtx->mainState == PAUSE_MAIN_STATE_SONG_PROMPT) {
                AudioOcarina_SetInstrument(OCARINA_INSTRUMENT_OFF);
            }

            KaleidoScope_MoveCursorFromSpecialPos(play);

            pauseCtx->cursorPoint[PAUSE_QUEST] = QUEST_REMAINS_GOHT;
            if (CHECK_QUEST_ITEM(pauseCtx->cursorPoint[PAUSE_QUEST])) {
                cursorItem = (ITEM_REMAINS_GOHT - 1) + pauseCtx->cursorPoint[PAUSE_QUEST];
                if (pauseCtx->cursorPoint[PAUSE_QUEST] != QUEST_REMAINS_ODOLWA) {
                    // This condition is always true as `cursorPoint` is set three lines above
                    cursorItem = ITEM_MASK_GIANT;
                }
            } else {
                cursorItem = PAUSE_ITEM_NONE;
            }

            cursor = pauseCtx->cursorPoint[PAUSE_QUEST];
            pauseCtx->cursorItem[pauseCtx->pageIndex] = cursorItem;
            pauseCtx->cursorSlot[pauseCtx->pageIndex] = cursor;
        }
    } else if (pauseCtx->mainState == PAUSE_MAIN_STATE_SONG_PLAYBACK_INIT) {
        // After a short delay, start the playback of the selected song

        pauseCtx->cursorColorSet = PAUSE_CURSOR_COLOR_SET_BLUE;
        sQuestSongPlaybackDelayTimer--;
        if (sQuestSongPlaybackDelayTimer == 0) {
            for (i = 0; i < 8; i++) {
                sQuestSongPlayedOcarinaButtons[i] = OCARINA_BTN_INVALID;
                sQuestSongPlayedOcarinaButtonsAlpha[i] = 0;
            }
            sQuestSongPlayedOcarinaButtonsNum = 0;

            pauseCtx->ocarinaButtonsY[OCARINA_BTN_A] = -62;
            pauseCtx->ocarinaButtonsY[OCARINA_BTN_C_DOWN] = -56;
            pauseCtx->ocarinaButtonsY[OCARINA_BTN_C_RIGHT] = -49;
            pauseCtx->ocarinaButtonsY[OCARINA_BTN_C_LEFT] = -46;
            pauseCtx->ocarinaButtonsY[OCARINA_BTN_C_UP] = -41;

            if (CHECK_QUEST_ITEM(pauseCtx->cursorPoint[PAUSE_QUEST])) {
                cursor = pauseCtx->cursorSlot[PAUSE_QUEST];
            } else {
                cursor = QUEST_BOMB_BAG;
            }

            AudioOcarina_SetInstrument(OCARINA_INSTRUMENT_DEFAULT);
            AudioOcarina_SetInstrument(OCARINA_INSTRUMENT_DEFAULT);
            pauseCtx->ocarinaSongIndex = gOcarinaSongItemMap[cursor - QUEST_SONG_SONATA];
            AudioOcarina_SetPlaybackSong(pauseCtx->ocarinaSongIndex + 1, 1);
            pauseCtx->mainState = PAUSE_MAIN_STATE_SONG_PLAYBACK;
            pauseCtx->ocarinaStaff = AudioOcarina_GetPlaybackStaff();
            pauseCtx->ocarinaStaff->pos = 0;
        }
    }
}

s32 KaleidoScope_UpdateQuestStatusPoint(PauseContext* pauseCtx, s16 point) {
    pauseCtx->cursorPoint[PAUSE_QUEST] = point;

    return true;
}
