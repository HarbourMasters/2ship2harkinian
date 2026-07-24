/**
 * spiritual_stones.cpp — Spiritual Stone passives + per-stone warp points.
 *
 * ── 2ship port status ───────────────────────────────────────────────────────
 * The Spiritual Stones (Kokiri Emerald / Goron's Ruby / Zora's Sapphire) are an
 * Ocarina of Time concept; they do not exist in Majora's Mask. The full summon-
 * warp feature is now ported to MM idioms:
 *   - Passive buffs: 3 flags in ShipSaveInfo, toggled by A on the OoT quest page.
 *   - C-button equip: done natively by z_kaleido_collect.c (BUTTON_ITEM_EQUIP),
 *     using the ITEM_SPIRITUAL_STONE_* sentinel ids (z64item.h).
 *   - In-game: SpiritualStone_TickHold (called from z_player.c Player_UpdateCommon)
 *     summons a warp statue on a >=60f C-button hold and opens a tap-to-warp prompt
 *     on a short release; the warp uses MM's respawn[]/respawnFlag/nextEntrance model
 *     (mirrors item_oot_spells.c Farore's Wind), and the prompt is a 2ship native
 *     CustomMessage two-choice box (2ship has no SoH CustomMessageManager).
 *   - The statue actor (mods/actors/spiritual_stone_statue.c) is a real registered MM
 *     actor (ACTOR_SPIRITUAL_STONE_STATUE) drawing the MM owl statue + a companion jewel DL.
 *
 * The following OoT-only API the SoH original used had to be replaced (not dropped):
 *   - OoT items / quest bits: ITEM_KOKIRI_EMERALD, QUEST_KOKIRI_EMERALD,
 *     CHECK_QUEST_ITEM(stone), Interface_LoadItemIcon1 — none exist in MM.
 *   - OoT equips layout: gSaveContext.equips.buttonItems[slot] /
 *     cButtonSlots[slot] — MM's equips are per-form ([4][4]), so the C-button
 *     equip path does not translate.
 *   - OoT warp API: play->nextEntranceIndex and OoT's RespawnData/playerParams
 *     semantics — MM uses a different entrance/respawn model.
 *   - The visible warp statue (spiritual_stone_statue.c) originally hijacked
 *     ACTOR_EN_LIGHTBOX and drew OBJECT_GI_JEWEL gems — neither exists in MM. It is
 *     now a real registered actor (ACTOR_SPIRITUAL_STONE_STATUE) and loads the jewel
 *     DLs by OTR path from the companion oot.o2r instead of the MM object system.
 *   - SoH-only services: SaveManager (AddInit/Save/Load) and CustomMessageManager
 *     (AddCustomMessageTable + the CustomMessage class API). 2ship has neither.
 *
 * What remains and ports cleanly:
 *   - Per-save state now lives INSIDE 2ship's per-save struct at
 *     gSaveContext.save.shipSaveInfo.spiritualStones (SpiritualStonesSaveData) and
 *     is serialized by 2s2h/BenJsonConversions.hpp — exactly the nei_save pattern.
 *     This is the SaveManager→ShipSaveInfo rewrite the original section needed.
 *   - The passive-toggle state (3 flags) and the per-stone warp records are kept,
 *     gated by the existing master CVar, and exposed through the public C API so
 *     an MM-native trigger path can be wired up later without touching save code.
 */

#include "spiritual_stones.h"

#include <libultraship/bridge.h> // CVarGetInteger
#include "soh/Enhancements/game-interactor/GameInteractor.h"
#include "soh/Enhancements/game-interactor/GameInteractor_Hooks.h"
#include "soh/ShipInit.hpp"
#include "2s2h/CustomMessage/CustomMessage.h" // warp prompt (2ship native two-choice box)
#include <string>

// OPEN_DISPS / CLOSE_DISPS (used by the statue .c pulled in below) redeclare these two symbols
// inline at every call site. In a C++ TU that in-block redeclaration takes C++ linkage unless a C
// declaration exists at file scope — force the C symbol so the macro's redeclaration matches and the
// link succeeds. (Same trick as PropHunt.cpp / the SoH original.)
extern "C" {
void FrameInterpolation_RecordOpenChild(const void* a, int b);
void FrameInterpolation_RecordCloseChild(void);
}

extern "C" {
#include "z64.h"
#include "global.h"
#include "functions.h"
#include "macros.h"
#include "variables.h"
extern PlayState* gPlayState;
extern SaveContext gSaveContext;
// Extended-button infra: stones are C-equipped via the u16 ext ids (buttonItems holds the
// ITEM_EXT_BUTTON marker), so held-button detection reads through ExtButton_GetItem, not the raw u8.
u16 ExtButton_GetItem(s32 form, s32 btn);
}

// Pull the statue actor into this translation unit (the .c is NOT in CMake; consumers #include it,
// exactly like somaria_cubes.c). extern "C" so Statue_Draw's OPEN_DISPS / CLOSE_DISPS inherit C
// linkage on their inline FrameInterpolation_* redeclarations.
extern "C" {
#include "../actors/spiritual_stone_statue.h"
#include "../actors/spiritual_stone_statue.c"
}

// ============================================================================
// State accessors — the live per-save state lives in ShipSaveInfo.
// ============================================================================

namespace {

inline SpiritualStonesSaveData& State() {
    return gSaveContext.save.shipSaveInfo.spiritualStones;
}

// Master toggle (NEI "Spells" tab). Default ON. When OFF, the spiritual stones
// behave VANILLA. Gated at runtime in the public API so toggling takes effect
// without a restart.
inline bool StonesEnabled() {
    return CVarGetInteger("gMods.SpiritualStones.Enabled", 1) != 0;
}

// ============================================================================
// Warp-point state (transient runtime) + helpers
// ============================================================================

// Set while a two-choice warp prompt is open; the per-frame tick polls the choice.
s32 sPendingWarpStone = -1;
// Per-stone C-button hold counter. -1 == "already summoned this hold" (fires once).
s16 sHoldFrames[SPIRITUAL_STONE_COUNT] = { 0, 0, 0 };

// Extended (u16) sentinel item ids (same ids the kaleido equip writes via ExtButton_SetItem).
// Indexed by stone. > 0xFF, so they only ever compare equal through ExtButton_GetItem.
inline s32 StoneItemId(int stone) {
    switch (stone) {
        case SPIRITUAL_STONE_KOKIRI: return EXT_ITEM_SPIRITUAL_STONE_KOKIRI;
        case SPIRITUAL_STONE_GORON:  return EXT_ITEM_SPIRITUAL_STONE_GORON;
        case SPIRITUAL_STONE_ZORA:   return EXT_ITEM_SPIRITUAL_STONE_ZORA;
    }
    return ITEM_NONE;
}

const char* StoneNameEnglish(int stone) {
    switch (stone) {
        case SPIRITUAL_STONE_KOKIRI: return "Kokiri Emerald";
        case SPIRITUAL_STONE_GORON:  return "Goron's Ruby";
        case SPIRITUAL_STONE_ZORA:   return "Zora's Sapphire";
    }
    return "Spiritual Stone";
}

// ── Warp execution — MM respawn model (mirrors item_oot_spells.c's Farore's Wind Return branch).
// MM uses respawn[]/respawnFlag/nextEntrance, NOT OoT's nextEntranceIndex/playerParams semantics.
void ExecuteWarp(PlayState* play, int stone) {
    SpiritualStoneWarpSave& w = State().warp[stone];
    if (w.entranceId < 0) {
        return;
    }

    // Use RESPAWN_MODE_TOP (Farore's Wind slot), NOT RESPAWN_MODE_DOWN: DOWN is the VOID-OUT respawn
    // and MM's Player_Init inflicts fall/void damage when reviving through it (that was the "me quita
    // vida" bug). TOP is a plain placement with no damage — same as item_oot_spells.c Farore's Wind.
    RespawnData* top = &gSaveContext.respawn[RESPAWN_MODE_TOP];
    top->pos.x = w.posX;
    top->pos.y = w.posY;
    top->pos.z = w.posZ;
    top->yaw = w.rotY;
    top->playerParams = PLAYER_PARAMS(0xFF, PLAYER_START_MODE_D); // stationary; plain re-enter
    top->entrance = (u16)w.entranceId;
    top->roomIndex = (u8)w.roomNum;
    top->data = 1;
    top->tempSwitchFlags = 0;
    top->unk_18 = 0;
    top->tempCollectFlags = 0;

    // respawnFlag = 3 → Player_Init uses respawn[respawnFlag - 1] = respawn[2] = RESPAWN_MODE_TOP.
    gSaveContext.respawnFlag = 3;
    play->nextEntrance = (u16)w.entranceId;
    play->transitionTrigger = TRANS_TRIGGER_START;
    play->transitionType = TRANS_TYPE_FADE_BLACK;
}

// ── Statue summon — record the warp point and spawn the visible statue at Link's spot.
void SummonStatueHere(PlayState* play, Player* player, int stone) {
    SpiritualStoneWarpSave& w = State().warp[stone];
    w.entranceId = gSaveContext.save.entrance; // MM: current entrance ("scene_no")
    w.sceneId = play->sceneId;
    w.roomNum = (s8)play->roomCtx.curRoom.num;
    w.posX = player->actor.world.pos.x;
    w.posY = player->actor.world.pos.y;
    w.posZ = player->actor.world.pos.z;
    w.rotY = player->actor.shape.rot.y;

    Vec3f pos = { w.posX, w.posY, w.posZ };
    SpiritualStoneStatue_Spawn(play, &pos, w.rotY, stone);
    Actor_PlaySfx(&player->actor, NA_SE_SY_GET_ITEM);
}

// ── Re-spawn statues that belong to the freshly-loaded scene (port of SpawnStatuesForCurrentScene).
void SpawnStatuesForCurrentScene() {
    if (!StonesEnabled()) {
        return;
    }
    if (gPlayState == nullptr) {
        return;
    }
    for (int i = 0; i < SPIRITUAL_STONE_COUNT; ++i) {
        SpiritualStoneWarpSave& w = State().warp[i];
        if (w.entranceId < 0) {
            continue;
        }
        if (w.sceneId != gPlayState->sceneId) {
            continue;
        }
        Vec3f pos = { w.posX, w.posY, w.posZ };
        SpiritualStoneStatue_Spawn(gPlayState, &pos, w.rotY, i);
    }
}

// ── Warp prompt — 2ship native two-choice box (choiceIndex 0 = Yes/warp, 1 = No).
void OpenWarpPrompt(PlayState* play, Player* player, int stone) {
    (void)play;
    sPendingWarpStone = stone;
    if (player != nullptr) {
        player->stateFlags1 |= PLAYER_STATE1_20000000; // freeze during the prompt (OoT IN_CUTSCENE analog)
    }
    // OCTAL control codes (a \x hex escape greedily eats following hex letters):
    //   \002 = CTRL_COLOR_GREEN, \021 = CTRL_NEWLINE (0x11), \302 = CTRL_TWO_CHOICE (0xC2).
    // The first option after the choice code is choiceIndex 0 (Yes).
    std::string body =
        std::string("Warp to your ") + StoneNameEnglish(stone) + " waypoint?\002\021\302Yes\021No";
    CustomMessage::Entry entry;
    entry.textboxType = 0;
    CustomMessage::StartTextbox(body, entry);
}

// ============================================================================
// Boot registration
// ============================================================================

// Scene-init: reset transient hold/prompt state and re-spawn statues for any warp point that lives
// in the freshly loaded scene (mirrors the SoH OnSceneSpawnActors re-spawn). The persisted
// SpiritualStonesSaveData itself is owned by the save file (BenJsonConversions).
void OnSceneInit(s16 sceneId, s8 spawnNum) {
    (void)sceneId;
    (void)spawnNum;
    sPendingWarpStone = -1;
    for (int i = 0; i < SPIRITUAL_STONE_COUNT; ++i) {
        sHoldFrames[i] = 0;
    }
    SpawnStatuesForCurrentScene();
}

void Register() {
    static bool registered = false;
    if (registered) {
        return;
    }
    registered = true;

    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSceneInit>(OnSceneInit);
}

static RegisterShipInitFunc gSpiritualStonesInit(Register);

} // namespace

// ============================================================================
// Public API (extern "C") — passive flags + the per-frame hold/tap warp driver.
// The C-button EQUIP is done natively by the kaleido OoT quest page
// (z_kaleido_collect.c), so the two OoT-cursor shims below stay no-ops.
// ============================================================================

extern "C" s32 SpiritualStone_TryToggleAtCursor(PlayState* play, Input* input) {
    // The passive A-toggle is driven directly by the kaleido OoT quest page
    // (OotQuest_HandleSelect → SpiritualStone_TogglePassive). No-op here.
    (void)play;
    (void)input;
    return false;
}

extern "C" s32 SpiritualStone_TryEquipAtCursor(PlayState* play, Input* input) {
    // The C-button / D-pad equip is driven directly by the kaleido OoT quest page
    // (OotQuest_HandleSelect, BUTTON_ITEM_EQUIP idiom). No-op here.
    (void)play;
    (void)input;
    return false;
}

extern "C" void SpiritualStone_TickHold(PlayState* play, Player* player) {
    if (!StonesEnabled()) {
        return;
    }
    if (play == nullptr || player == nullptr) {
        return;
    }

    // A 2ship choice box does NOT self-close on A (see item_oot_spells.c Player_Action_OotFwMenu):
    // the opener must poll TEXT_STATE_CHOICE + Message_ShouldAdvance, read choiceIndex, act, then
    // call Message_CloseTextbox itself.
    if (sPendingWarpStone >= 0) {
        if ((Message_GetState(&play->msgCtx) == TEXT_STATE_CHOICE) && Message_ShouldAdvance(play)) {
            s32 stone = sPendingWarpStone;
            s32 choice = play->msgCtx.choiceIndex;
            sPendingWarpStone = -1;
            Message_CloseTextbox(play);
            player->stateFlags1 &= ~PLAYER_STATE1_20000000;
            if (choice == 0) { // Yes → warp
                ExecuteWarp(play, stone);
            }
        }
        return; // don't process holds while the prompt is up
    }

    // Don't process hold input while a textbox is up.
    if (play->msgCtx.msgMode != MSGMODE_NONE) {
        return;
    }

    // Held-button detection: build a per-stone C-button mask from the equips and watch
    // input[0].cur.button. CRITICAL: MM equips.buttonItems is [form][button] ([4][4]); C-button items
    // are stored at FORM 0 for every form (see z64save.h GET_CUR_FORM_BTN_ITEM — only B is per-form),
    // and that is exactly where the kaleido equips them (BUTTON_ITEM_EQUIP(0, cBtn)). So read
    // buttonItems[0][1..3] — NOT buttonItems[1..3], which would be a whole row, not an item.
    // EQUIP_SLOT_B=0, C_LEFT=1, C_DOWN=2, C_RIGHT=3.
    u16 cur = play->state.input[0].cur.button;

    for (int stone = 0; stone < SPIRITUAL_STONE_COUNT; ++stone) {
        s32 itemId = StoneItemId(stone);
        u16 mask = 0;
        // Read through ExtButton_GetItem: the slot's u8 is ITEM_EXT_BUTTON, the real u16 id (compared
        // here) lives in extButtons. Comparing the raw u8 buttonItems would never match the u16 id.
        if (ExtButton_GetItem(0, EQUIP_SLOT_C_LEFT) == itemId) {
            mask |= BTN_CLEFT;
        }
        if (ExtButton_GetItem(0, EQUIP_SLOT_C_DOWN) == itemId) {
            mask |= BTN_CDOWN;
        }
        if (ExtButton_GetItem(0, EQUIP_SLOT_C_RIGHT) == itemId) {
            mask |= BTN_CRIGHT;
        }
        if (mask == 0) {
            sHoldFrames[stone] = 0;
            continue;
        }

        bool held = (cur & mask) != 0;
        if (held) {
            if (sHoldFrames[stone] >= 0) {
                sHoldFrames[stone]++;
                // Immediate feedback the FIRST frame the equipped stone's C button is held, so the
                // hold is visibly/audibly detected (also a diagnostic: if you hear this, detection
                // works and only the hold duration matters).
                if (sHoldFrames[stone] == 1) {
                    Actor_PlaySfx(&player->actor, NA_SE_SY_CURSOR);
                }
                if (sHoldFrames[stone] >= SPIRITUAL_STONE_SUMMON_HOLD_FRAMES) {
                    SummonStatueHere(play, player, stone);
                    sHoldFrames[stone] = -1; // sentinel: summoned this hold
                }
            }
        } else {
            // Release: a short tap on a stone with a warp set opens the yes/no warp prompt.
            s16 frames = sHoldFrames[stone];
            sHoldFrames[stone] = 0;
            if ((frames > 0) && (frames < SPIRITUAL_STONE_SUMMON_HOLD_FRAMES) &&
                (State().warp[stone].entranceId >= 0) && (sPendingWarpStone < 0)) {
                OpenWarpPrompt(play, player, stone);
                break; // one prompt per frame
            }
        }
    }
}

extern "C" s32 SpiritualStone_IsPassiveActive(s32 stone) {
    if (!StonesEnabled()) {
        return 0;
    }
    if (stone < 0 || stone >= SPIRITUAL_STONE_COUNT) {
        return 0;
    }
    return State().passive[stone];
}

extern "C" void SpiritualStone_TogglePassive(s32 stone) {
    if (!StonesEnabled()) {
        return;
    }
    if (stone < 0 || stone >= SPIRITUAL_STONE_COUNT) {
        return;
    }
    State().passive[stone] = !State().passive[stone];
}

extern "C" s32 SpiritualStone_KokiriWalkActive(void) {
    if (!StonesEnabled()) {
        return 0;
    }
    return State().passive[SPIRITUAL_STONE_KOKIRI];
}

extern "C" s32 SpiritualStone_GoronClimbActive(void) {
    if (!StonesEnabled()) {
        return 0;
    }
    return State().passive[SPIRITUAL_STONE_GORON];
}

extern "C" s32 SpiritualStone_ZoraSwimActive(void) {
    if (!StonesEnabled()) {
        return 0;
    }
    return State().passive[SPIRITUAL_STONE_ZORA];
}
