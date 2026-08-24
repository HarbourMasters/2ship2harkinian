// FleetWarpArrival.cpp — cross-game warps, MM side (UNIFIED arrival pipeline + sending triggers).
//
// ARRIVALS (any warp addressed to MM) run ONE pipeline, cold-boot or in-gameplay:
//   1. force-open the paired save slot (file-select loader on cold boot; direct flashrom reload
//      in gameplay),
//   2. FleetSync overlays (own full anchor + shared player state),
//   3. EXPLICIT destination overrides LAST (entrance/cutscene/respawn) — applied AFTER the load so
//      nothing stomps them (the old Room -96 / "Lost Woods Intro" crash was FileSelect_LoadGame's
//      state machine overwriting overrides set too early),
//   4. fresh Play_Init.
// Cold boot: the overrides are applied from the OnSaveLoad hook, which fires at the very END of
// FileSelect_LoadGame (after Sram_OpenSave + all vanilla defaults, before Play_Init runs).
//
// SENDING: Lost Woods door proximity trigger + the South Clock Town fleet hole (Door_Ana, spawned
// by FleetWarpDoor.cpp; fall intercepted in z_door_ana.c -> FleetSync_OnHoleFall). Both ramp the
// manual send-fade and flip to OoT at full black, writing the FleetSync departure first.

#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "FleetShipCombo.h"
#include "FleetSync.h"
#include "FleetOracle.h"                  // FleetOracle_RecreateSaveSlot (rebuild a rejected MM slot)
#include "2s2h/BenGui/Notification.h"     // tell the player their MM file was rebuilt
#include "2s2h/SaveManager/SaveManager.h" // read a slot's seed off disk without loading it
#include <nlohmann/json.hpp>
#include <string>
#include <libultraship/bridge/consolevariablebridge.h> // CVar: persist the combo slot for auto-resume
#include <spdlog/spdlog.h>                             // [FleetArrive] diagnostics

extern "C" {
#include "z64.h"
#include "macros.h"
#include "functions.h"
#include "variables.h"
#include "src/overlays/gamestates/ovl_file_choose/z_file_select.h"
extern PlayState* gPlayState;
extern SaveContext gSaveContext;
extern FileSelectState* gFileSelectState;          // non-null only while at the file-select screen
extern void FileSelect_LoadGame(GameState* thisx); // SM_LOAD_GAME handler (non-static; not in the header)
extern void Play_Init(GameState* thisx);           // last-resort scene reload (watchdog recovery)
extern SceneEntranceTableEntry sSceneEntranceTable[ENTR_SCENE_MAX]; // z_scene_table.c (non-static, see BetterMapSelect)
u8 ResourceMgr_FileExists(const char* resName); // BenPort.cpp (C-side prototype only; see DrawItem.cpp)
}

namespace {

// MM-side door anchor (Lost Woods) and the OoT-side door we flip back to.
constexpr float kMmDoorX = -1092.578f;
constexpr float kMmDoorY = 0.0f;
constexpr float kMmDoorZ = 487.082f;
constexpr float kDoorRadius = 120.0f;

constexpr int kOoTGame = 0;     // FleetShipCombo target id for Ocarina of Time (Ship)
constexpr int kOoTScene = 0x5B; // OoT SCENE_LOST_WOODS
constexpr float kOoTDoorX = 772.033f;
constexpr float kOoTDoorY = 0.0f;
constexpr float kOoTDoorZ = 322.431f;
constexpr int kOoTDoorRotY = -367;

constexpr int kOoTTotScene = 0x23; // ToT signal id; OoT maps it to the Temple of Time INTERIOR
                                   // (pedestal chamber, room 1) where the paired hole lives

// The MM fleet hole (South Clock Town) — must match the spawner in FleetWarpDoor.cpp. The pop-out
// spawns AT the hole (known-good ground; a position offset landed inside geometry and trapped the
// player under the hole). Re-entry is prevented by a HOLE SPAWN DELAY after arrival: the hole
// simply doesn't exist for a few seconds (see FleetWarpDoor.cpp).
constexpr float kMmHoleX = -527.0f;
constexpr float kMmHoleY = 100.0f;
constexpr float kMmHoleZ = -1173.719f;
constexpr s16 kMmHoleExitYaw = -16384; // facing -X, leaping away from the hole

bool sArmed = false;     // Lost Woods door: don't re-trigger while standing on it
bool sHoleArmed = false; // Clock Town fleet hole: re-arms when Link steps off the hole spot
s16 sCooldown = 0;       // suppress the trigger right after any warp (bridges the scene reload)
bool sSendFlip = false;
s16 sSendAlpha = 0;
int sSendScene = 0x5B;
float sSendX = 0.0f, sSendY = 0.0f, sSendZ = 0.0f;
int sSendRotY = 0;

// Pending ARRIVAL cache: consumed warp waiting for the destination overrides to be applied at the
// right moment (cold boot: after FileSelect_LoadGame finishes, via OnSaveLoad).
bool sArrivalPending = false;
int sArrivalScene = 0;
float sArrivalX = 0.0f, sArrivalY = 0.0f, sArrivalZ = 0.0f;
int sArrivalRotY = 0;

// =================================================================================================
// WATCHDOGS — "a warp must never leave the player stuck"
// =================================================================================================
// Every step of the warp is a handshake with something that can silently not happen: a hook that
// doesn't fire (OnSaveLoad), a transition FSM that never settles, a trigger another guard squashed,
// a fade the peer never cleared. Any ONE of those leaves the player looking at a frozen black
// screen with no error, and we cannot reproduce it here — so instead of guessing which one it is,
// EVERY step now carries a deadline and a recovery, and each recovery logs a distinct
// [FleetWatchdog] line. Whichever line shows up in a stuck player's log names the culprit.
//
// The escalation is always the same shape: wait -> retry the normal path -> force the destination
// the blunt way (Play_Init on the entrance we already computed). The blunt way is not pretty, but a
// scene reload at the right entrance is never worse than a frozen screen.
constexpr s16 kSendHoldMaxFrames = 300;      // 5s held at full black waiting for transitionMode==OFF
constexpr s16 kArrivalHookFrames = 240;      // 4s waiting for OnSaveLoad after FileSelect_LoadGame
constexpr s16 kArrivalLoadRetryFrames = 180; // 3s waiting for FileSelect_LoadGame to do anything
constexpr s16 kArrivalArmFrames = 40;        // frames an armed arrival transition may fail to start
constexpr s16 kArrivalMaxRetries = 3;        // re-arms before we force Play_Init outright
constexpr s16 kTransStuckFrames = 600;       // 10s of a transition FSM that never completes
constexpr s16 kFadeStuckFrames = 120;        // 2s of full-black send fade with no flip in progress
constexpr s16 kArrivalGraceFrames = 30;      // our own trigger is protected from the squash guards
constexpr s16 kArrivalWindowFrames = 1800;   // 30s after a warp: the blunt recoveries stay allowed. A
                                             // vanilla transition that hangs LONG after a warp is not
                                             // ours to teleport out of — it only gets cancelled.

s16 sSendHoldFrames = 0;
s16 sArrivalPendingFrames = 0;
s16 sArrivalLoadRetries = 0;
s16 sArrivalWatchFrames = 0;
s16 sArrivalRetries = 0;
bool sArrivalArmed = false; // an in-game arrival transition is armed and being watched
u16 sArrivalEntrance = 0;   // the entrance it was armed with (re-arm / forced reload use it)
s16 sArrivalGrace = 0;      // >0: don't let the squash guards touch our own transition
s16 sArrivalWindow = 0;     // >0: a warp arrival is still in flight (bounds the blunt recoveries)
s16 sTransStuckFrames = 0;
s16 sFadeStuckFrames = 0;

// DEFERRED FLIP — set by the draw hook when the fade reaches full black, executed by the UPDATE
// hook. See the note where it is committed: handing the game over from inside Play_Draw is what the
// 2026-07-31 logs show killing 2ship, so the draw hook now only decides, and the flip itself
// happens with no render in progress.
bool sFlipCommitPending = false;

// =================================================================================================
// LIMBO — the inactive game is PARKED, not frozen
// =================================================================================================
// A frozen game is a half-alive one: FrameAdvance never ticks it, so it never finishes the
// transition it was in, never completes the flash write it started, never refreshes the framebuffer
// the producer captures — every one of those was a crash or a hang this month. So the inactive game
// no longer freezes. Instead, BEFORE handing over, it walks Link into a sealed room with no exits,
// no actors, no music and time speed 0 ("fleet_scene", a custom scene in 2ship.o2r), and only THEN
// flips. It keeps running normally in there — a real scene, drawn every frame — with input blocked
// (BenPort already blocks game input for the inactive process). Becoming active again is an
// ordinary in-game transition out of the room to wherever the peer sent us.
//
// This also gives every portal a FIXED exit: leaving is always "go to limbo", and the DESTINATION is
// whatever the other game says. Adding a portal is one exit actor, not a pair of agreed coordinates.
//
// The room is a vanilla-format scene at a scene id the game never used (SCENE_UNSET_01) reached
// through an entrance-table slot the game never used (ENTR_SCENE_UNSET_08); both tables are patched
// at init below, so no engine file changes.
constexpr s32 kLimboSceneId = SCENE_UNSET_01;                       // 0x01, a hole in the scene table
constexpr s32 kLimboEntrSceneIdx = ENTR_SCENE_UNSET_08;             // 0x08, a hole in the entrance table
constexpr u16 kLimboEntrance = (u16)(kLimboEntrSceneIdx << 9);      // spawn 0, layer 0 -> 0x1000
constexpr s16 kLimboWaitMaxFrames = 300;                            // 5s to reach the room before we
                                                                    // flip anyway (old behaviour)
constexpr u16 kLimboTransFlags = (u16)(TRANS_TYPE_FADE_BLACK << 7); // Play_Init: (flags >> 7) & 0x7F

EntranceTableEntry sLimboEntries[4] = { { kLimboSceneId, 0, kLimboTransFlags },
                                        { kLimboSceneId, 0, kLimboTransFlags },
                                        { kLimboSceneId, 0, kLimboTransFlags },
                                        { kLimboSceneId, 0, kLimboTransFlags } };
EntranceTableEntry* sLimboSpawnTable[] = { sLimboEntries }; // one spawn, four scene layers

// The state Link had BEFORE being parked, so a RESUME (or the save written while parked) describes
// the real save and not the waiting room.
struct LimboReturnState {
    bool valid = false;
    u16 entrance = 0;
    s32 cutsceneIndex = 0;
    u16 nextCutsceneIndex = 0xFFEF;
    u8 isOwlSave = 0;
    s32 respawnFlag = 0;
    s16 savedSceneId = 0;
    u16 time = 0; // MM's clock: frozen while parked (room time speed 0) and restored on the way out
    s32 day = 0;
};
LimboReturnState sLimboReturn;

// "Heading into the room": set when the limbo transition starts, cleared once the room is loaded
// (or after kLimboWaitMaxFrames). While set, the game is NOT suspended even though it is already
// inactive (the flip happens the same frame the transition starts), and the frozen-game guards
// leave its transition trigger alone -- otherwise the handover would freeze it mid-transition, the
// exact half-alive state the waiting room exists to abolish.
bool sLimboInFlight = false;
s16 sLimboInFlightFrames = 0;
bool sBootIntoLimbo = false; // cold boot as the INACTIVE game: land straight in the room

bool LimboInRoom() {
    return gPlayState != NULL && gPlayState->sceneId == kLimboSceneId;
}

// Is the waiting room actually in an archive? 2ship's OTRPlay_SpawnScene does NOT null-check the
// scene it loads (soh does), so booting a scene whose file is missing is a hard crash inside
// OTRScene_ExecuteCommands — which is exactly what a 2ship.o2r built before the asset existed
// produces. Checked once, on first use, and if the room is missing the whole limbo feature turns
// itself off: every caller falls back to the old freeze-in-place behaviour and says why.
bool sLimboAvailable = false;
bool sLimboChecked = false;
bool LimboAvailable() {
    if (!sLimboChecked) {
        sLimboChecked = true;
        sLimboAvailable = ResourceMgr_FileExists("scenes/nonmq/fleet_scene/fleet_scene") &&
                          ResourceMgr_FileExists("scenes/nonmq/fleet_scene/fleet_scene_room_0") &&
                          ResourceMgr_FileExists("scenes/nonmq/fleet_scene/fleet_scene_col");
        if (!sLimboAvailable) {
            SPDLOG_ERROR("[FleetLimbo] fleet_scene is NOT in any archive — 2ship.o2r was packed before the asset "
                         "existed (run the Generate2ShipOtr target). Waiting room disabled; inactive MM will "
                         "freeze in place as before.");
        }
    }
    return sLimboAvailable;
}

void LimboStashReturnState() {
    sLimboReturn.valid = true;
    sLimboReturn.entrance = (u16)gSaveContext.save.entrance;
    sLimboReturn.cutsceneIndex = gSaveContext.save.cutsceneIndex;
    sLimboReturn.nextCutsceneIndex = gSaveContext.nextCutsceneIndex;
    sLimboReturn.isOwlSave = gSaveContext.save.isOwlSave;
    sLimboReturn.respawnFlag = gSaveContext.respawnFlag;
    sLimboReturn.savedSceneId = gSaveContext.save.saveInfo.playerData.savedSceneId;
    sLimboReturn.time = gSaveContext.save.time;
    sLimboReturn.day = gSaveContext.save.day;
}

// Point the save at the waiting room. Only touches what Play_Init reads to pick a scene; the stash
// above is what brings the real values back.
void LimboSetSaveToRoom() {
    gSaveContext.save.entrance = kLimboEntrance;
    gSaveContext.save.cutsceneIndex = 0;
    gSaveContext.nextCutsceneIndex = 0xFFEF;
    gSaveContext.sceneLayer = 0;
    gSaveContext.save.isOwlSave = false;
    gSaveContext.respawnFlag = 0; // spawn from the room's own spawn point, never a stale respawn
    gSaveContext.nextTransitionType = TRANS_TYPE_FADE_BLACK;
    gSaveContext.seqId = (u8)NA_BGM_DISABLED;
    gSaveContext.ambienceId = AMBIENCE_ID_DISABLED;
    gSaveContext.nextDayTime = NEXT_TIME_NONE;
}

// Bring back what the stash holds (the save's own place). Used by RESUME arrivals and by the
// frozen-save writer, which must never persist the waiting room as the player's location.
void LimboRestoreReturnState() {
    if (!sLimboReturn.valid) {
        return;
    }
    gSaveContext.save.entrance = sLimboReturn.entrance;
    gSaveContext.save.cutsceneIndex = sLimboReturn.cutsceneIndex;
    gSaveContext.nextCutsceneIndex = sLimboReturn.nextCutsceneIndex;
    gSaveContext.save.isOwlSave = sLimboReturn.isOwlSave;
    gSaveContext.respawnFlag = sLimboReturn.respawnFlag;
    gSaveContext.save.saveInfo.playerData.savedSceneId = sLimboReturn.savedSceneId;
    gSaveContext.save.time = sLimboReturn.time;
    gSaveContext.save.day = sLimboReturn.day;
}

// Start walking into the room from live gameplay (the departing game, still ACTIVE at this point).
// The transition is INSTANT because the send fade is already at full black.
void LimboEnterFromGameplay() {
    if (gPlayState == NULL || !LimboAvailable()) {
        return; // no room to go to: the caller's wait loop expires and flips in place (old behaviour)
    }
    LimboStashReturnState();
    LimboSetSaveToRoom();
    gPlayState->nextEntrance = kLimboEntrance;
    gPlayState->transitionTrigger = TRANS_TRIGGER_START;
    gPlayState->transitionType = TRANS_TYPE_INSTANT;
    sLimboInFlight = true;
    sLimboInFlightFrames = 0;
    SPDLOG_INFO("[FleetLimbo] MM heading into the waiting room (was at entrance {:#06x})", (int)sLimboReturn.entrance);
}

// Patch the two engine tables so the waiting room is a real, bootable scene. Runs once at init.
void LimboInstallScene() {
    SceneTableEntry* entry = &gSceneTable[kLimboSceneId];
    entry->segment.vromStart = 0;
    entry->segment.vromEnd = 0;
    entry->segment.fileName = (char*)"fleet_scene"; // -> scenes/nonmq/fleet_scene/fleet_scene (2ship.o2r)
    entry->titleTextId = 0;
    entry->unk_A = 0;
    entry->drawConfig = SCENE_DRAW_CFG_DEFAULT;
    entry->unk_C = 0;
    entry->unk_D = 0;

    sSceneEntranceTable[kLimboEntrSceneIdx].tableCount = 1;
    sSceneEntranceTable[kLimboEntrSceneIdx].table = sLimboSpawnTable;
    sSceneEntranceTable[kLimboEntrSceneIdx].name = (char*)"FLEET_LIMBO";
    SPDLOG_INFO("[FleetLimbo] waiting room installed: scene {:#x}, entrance {:#06x}", kLimboSceneId,
                (int)kLimboEntrance);
}

// Emergency destination reload: gSaveContext already holds the destination (save.entrance +
// respawn[TOP] + respawnFlag=3 from ApplyDestinationOverrides), so a plain Play_Init lands exactly
// where the transition was supposed to take us. Used only after the normal path missed its
// deadline — it costs a scene load, never a stuck screen.
void ForceDestinationReload(const char* why) {
    if (gPlayState == NULL) {
        return;
    }
    SPDLOG_ERROR("[FleetWatchdog] {} -> forcing Play_Init on entrance {:#06x}", why,
                 (int)(u16)gSaveContext.save.entrance);
    Notification::Emit({
        .prefix = "[Fleet] ",
        .message = "warp recovered by the watchdog",
        .suffix = " (please send your log)",
    });
    gPlayState->transitionTrigger = TRANS_TRIGGER_OFF;
    gPlayState->transitionMode = TRANS_MODE_OFF;
    sArrivalArmed = false;
    sArrivalWatchFrames = 0;
    sArrivalRetries = 0;
    STOP_GAMESTATE(&gPlayState->state);
    SET_NEXT_GAMESTATE(&gPlayState->state, Play_Init, sizeof(PlayState));
}

// In fleet mode MM's save is DERIVED: OoT holds the combo seed, so a missing or REJECTED MM file is
// not data loss -- it is something we can rebuild. This matters because of what vanilla does when a
// slot's save AND its backup both fail to load (z_sram_NES.c, Sram_OpenSave):
//
//     memset(sramCtx->saveBuf, 0, SAVE_BUFFER_SIZE);
//     memcpy(&gSaveContext, sramCtx->saveBuf, ...);
//
// gSaveContext comes back ALL ZEROES -- and 0x00 is MM's ITEM_OCARINA_OF_TIME, so the inventory
// fills with Ocarinas of Time. Worse, every "is this slot empty?" test in the codebase compares
// against 0xFF, so with 0x00 sitting there they all read "occupied" and FleetSync silently refuses
// to write anything: the cross-game item sync stops working too. Both symptoms, one cause.
//
// The usual way a valid file gets rejected is the rando version gate (BenJsonConversions: a rando
// save whose commitHash differs from this build is refused, and SaveManager then renames it to
// file<N>_invalid_<time>.json). That is expected after a rebuild -- so rebuild the slot instead of
// booting a zeroed save.
// Does the MM file in `slot` carry the seed OoT published for this combo? Read straight off disk:
// the save is not loaded yet when we need to know, and this avoids a load-then-reload dance.
// Returns true when we can PROVE it is the wrong seed; false when it matches, or when we cannot
// tell (no seed published, unreadable file, non-rando save) -- never rebuild on a guess.
bool ComboSlotHasWrongSeed(int slot) {
    const unsigned int expected = FleetShipCombo_GetComboSeed();
    if (expected == 0) {
        return false; // OoT has not generated/loaded a combo seed yet: nothing to validate against
    }
    nlohmann::json j;
    if (SaveManager_ReadSaveFile(SaveManager_GetFileName(slot + 1), j) != 0) {
        return false; // unreadable: the SLOT_OCCUPIED path deals with that case
    }
    try {
        const auto& rando = j.at("newCycleSave").at("save").at("shipSaveInfo").at("rando");
        const unsigned int actual = rando.at("finalSeed").get<unsigned int>();
        if (actual == expected) {
            return false;
        }
        SPDLOG_WARN("[FleetArrive] MM slot {} was built for seed {} but this combo is seed {}", slot, actual, expected);
        return true;
    } catch (...) {
        return false; // not a rando save, or a shape we do not understand: leave it alone
    }
}

// The file select's occupancy cache is filled ONCE at menu init, so a slot whose file vanished since
// then (the save pairing deletes MM's half when OoT erases its file) still reads "occupied" — and
// loading a slot with no file behind it is exactly what hands back an all-zeroes gSaveContext. Ask
// the disk instead of the cache.
bool ComboSlotFileExists(int slot) {
    nlohmann::json j;
    return SaveManager_ReadSaveFile(SaveManager_GetFileName(slot + 1), j) == 0;
}

bool EnsureComboSlot(int slot) {
    if (slot < 0 || slot > 2 || gFileSelectState == NULL) {
        return false;
    }
    const bool wrongSeed = ComboSlotHasWrongSeed(slot);
    const bool onDisk = ComboSlotFileExists(slot);
    if (SLOT_OCCUPIED(gFileSelectState, slot) && onDisk && !wrongSeed) {
        return true;
    }
    static const char kNewf[6] = { 'Z', 'E', 'L', 'D', 'A', '3' };
    if (onDisk && !wrongSeed) {
        // A good file that the menu simply hasn't seen: it was written AFTER this file select was
        // built (OoT's oracle created or rebuilt it while MM sat here — the normal order now that MM
        // auto-loads its slot as the inactive game). Refresh the cached listing and use it as is;
        // rebuilding would throw away a file that is already right.
        for (int i = 0; i < 6; i++) {
            gFileSelectState->newf[slot][i] = kNewf[i];
        }
        SPDLOG_INFO("[FleetArrive] MM slot {} exists on disk with the right seed -> refreshed the file-select listing",
                    slot);
        return true;
    }
    SPDLOG_WARN("[FleetArrive] MM slot {} unusable ({}) -> rebuilding it from the combo seed", slot,
                wrongSeed ? "built for a different seed"
                : !onDisk ? "no file on disk (erased with OoT's half)"
                          : "missing, or rejected and backed up");
    if (!FleetOracle_RecreateSaveSlot(slot, "Link")) {
        SPDLOG_ERROR("[FleetArrive] rebuild of MM slot {} FAILED -- refusing to load a zeroed save", slot);
        return false;
    }
    // Refresh the file-select's cached listing: it is only read at menu init, and SLOT_OCCUPIED (and
    // FileSelect_LoadGame) consult it rather than the disk. The file itself is now on disk, so
    // Sram_OpenSave will read the real thing.
    for (int i = 0; i < 6; i++) {
        gFileSelectState->newf[slot][i] = kNewf[i];
    }
    Notification::Emit({
        .prefix = "[Fleet] ",
        .message = "Majora's Mask file " + std::to_string(slot + 1) + " was rebuilt from the combo seed",
        .suffix = wrongSeed ? " (it belonged to a different seed)" : " (the previous one could not be loaded)",
    });
    return true;
}

// Fleet-mode combo file-select BYPASS: MM's title/file-select are OoT-driven. When MM is the ACTIVE
// game it auto-loads the designated combo slot (no manual pick). One-shot until MM leaves the file
// select (reset when a PlayState / title comes up).
bool sComboAutoLoaded = false;

// Destination overrides + FleetSync overlays. MUST run after ANY save loading is complete and
// before Play_Init consumes gSaveContext.
void ApplyDestinationOverrides() {
    int slot = FleetShipCombo_GetWarpSaveFile();
    if (slot < 0 || slot > 2) {
        slot = 0;
    }
    gSaveContext.fileNum = slot;

    // Remember the combo slot so a later boot-into-MM (fresh process, no warp yet) can auto-resume THIS
    // slot instead of guessing File 1. Persisted across sessions.
    CVarSetInteger("gFleetCombo.LastSlot", slot);
    CVarSave();

    // The item overlay must never be able to cancel the ARRIVAL. Everything below this line is what
    // actually puts the player somewhere valid; the overlay is "what you're carrying when you get
    // there". A throw here (bad JSON in the temp file, an item ApplyShared can't map) used to abort
    // the whole function, so the destination overrides never ran: the warp was consumed, no entrance
    // was set, and MM sat wherever it was — frozen from the player's point of view. Arrive first,
    // sync second; FleetNet reconciles the state afterwards anyway.
    try {
        FleetSync_ApplyArrival(slot);
    } catch (const std::exception& e) {
        SPDLOG_ERROR("[FleetWatchdog] arrival overlay threw: {} — continuing to the destination anyway", e.what());
    } catch (...) {
        SPDLOG_ERROR("[FleetWatchdog] arrival overlay threw a non-std exception — continuing to the destination");
    }

    // RESUME ARRIVAL — "carry on where you saved", not "come out of a portal".
    // Sent by OoT when a combo file whose last save was made in MM is loaded: the player is meant to
    // land back in their own save, wherever that is (an owl save, a fresh file's opening, the dawn
    // of a cycle), NOT at the Clock Town hole. FileSelect_LoadGame has already put exactly that in
    // gSaveContext, so the correct thing to do here is NOTHING — every override below would drag
    // the player to a portal they never used. Note this is also why isOwlSave must be left alone:
    // an owl save legitimately re-derives its own entrance.
    if (sArrivalScene == FC_WARP_SCENE_RESUME) {
        // Parked in the waiting room, the save's "own place" is the one we stashed on the way in —
        // gSaveContext currently says "waiting room", which is the one place a resume must NOT
        // land. (Not parked = the file-select load just ran and gSaveContext is already right.)
        if (LimboInRoom()) {
            LimboRestoreReturnState();
        }
        SPDLOG_INFO("[FleetArrive] MM RESUME arrival: keeping the save's own entrance {:#06x} (fileNum={})",
                    (int)(u16)gSaveContext.save.entrance, gSaveContext.fileNum);
        sArrivalPending = false;
        sArrivalPendingFrames = 0;
        sArrivalLoadRetries = 0;
        sArrivalEntrance = (u16)gSaveContext.save.entrance;
        sArmed = true; // don't let a door/hole we happen to spawn next to flip us straight back
        sHoleArmed = true;
        sCooldown = 40;
        FleetShipCombo_SetSendFadeAlpha(0); // destination settled: lower the curtain the peer left up
        return;
    }

    // WARP RECIPE — copied EXACTLY from mm/mods/spiritual_stones/spiritual_stones.cpp ExecuteWarp
    // (a PROVEN MM warp). Two things it gets right that the old code got wrong:
    //   (1) RESPAWN_MODE_TOP (Farore's Wind slot), NOT RESPAWN_MODE_DOWN. DOWN is the VOID-OUT
    //       respawn that MM's Player_Update CONTINUOUSLY overwrites with Link's current safe
    //       position — so a DOWN warp point gets clobbered back to the DEPARTURE spot before
    //       Play_Init reads it ("no me mueve" / lands where you left MM). TOP is never touched.
    //   (2) respawnFlag = 3  ->  Player_Init reads respawn[respawnFlag - 1] = respawn[2] = TOP,
    //       with NO void/fall damage (respawnFlag 1/-1 would inflict it).
    RespawnData* top = &gSaveContext.respawn[RESPAWN_MODE_TOP];
    s32 entrance;
    s16 startMode;
    switch (sArrivalScene) {
        case SCENE_CLOCKTOWER: // 0x6F South Clock Town: pop OUT of the fleet hole
            entrance = ENTRANCE(SOUTH_CLOCK_TOWN, 0);
            top->pos.x = kMmHoleX;
            top->pos.y = kMmHoleY;
            top->pos.z = kMmHoleZ;
            top->yaw = kMmHoleExitYaw;
            startMode = PLAYER_START_MODE_GROTTO; // rise out of the ground
            break;
        case SCENE_LOST_WOODS: // 0x65 Lost Woods door: land at the door spot walking out
        default:
            entrance = ENTRANCE(LOST_WOODS, 0);
            top->pos.x = (sArrivalX != 0.0f || sArrivalZ != 0.0f) ? sArrivalX : kMmDoorX;
            top->pos.y = sArrivalY;
            top->pos.z = (sArrivalX != 0.0f || sArrivalZ != 0.0f) ? sArrivalZ : kMmDoorZ;
            top->yaw = (s16)sArrivalRotY;
            startMode = PLAYER_START_MODE_E; // walk forward out
            break;
    }
    top->playerParams = PLAYER_PARAMS(0xFF, startMode);
    top->entrance = (u16)entrance;
    top->roomIndex = 0;
    top->data = 1;
    top->tempSwitchFlags = 0;
    top->unk_18 = 0;
    top->tempCollectFlags = 0;
    gSaveContext.respawnFlag = 3; // -> respawn[RESPAWN_MODE_TOP], no void damage

    gSaveContext.save.entrance = entrance;
    gSaveContext.save.isOwlSave = false; // arriving is a fresh boot, never an owl-save resume (an
                                         // owl resume re-derives entrance from pauseSaveEntrance /
                                         // owlWarpId — e.g. Inverted Stone Tower)
    gSaveContext.save.cutsceneIndex = 0; // never inherit a stale scene-setup/cutscene selector
    gSaveContext.nextCutsceneIndex = 0xFFEF;
    gSaveContext.sceneLayer = 0;
    gSaveContext.nextTransitionType = TRANS_TYPE_FADE_BLACK; // slow fade-in reveal
    gSaveContext.seqId = (u8)NA_BGM_DISABLED;
    gSaveContext.ambienceId = AMBIENCE_ID_DISABLED;
    gSaveContext.nextDayTime = NEXT_TIME_NONE;

    SPDLOG_INFO("[FleetArrive] MM ApplyDestinationOverrides: sArrivalScene={:#x} entrance={:#06x} respawnFlag={} "
                "fileNum={} save.entrance={:#06x}",
                sArrivalScene, entrance, gSaveContext.respawnFlag, gSaveContext.fileNum, gSaveContext.save.entrance);

    sArrivalPending = false;
    sArrivalPendingFrames = 0;
    sArrivalLoadRetries = 0;
    sArrivalEntrance = (u16)entrance;
    sArmed = true;     // Lost Woods door: don't instantly flip back
    sHoleArmed = true; // Clock Town hole: Link pops OUT on the spot -> suppress until he steps off
    sCooldown = 40;
    FleetShipCombo_SetSendFadeAlpha(0); // destination settled: lower the curtain the peer left up
}

// Arm the in-game arrival transition (gameplay arrivals) and put it under watch. The OUT is
// TRANS_TYPE_INSTANT — NOT FADE_BLACK — on PURPOSE: a FADE_BLACK out fades the OLD (frozen,
// just-flipped-to) MM scene over ~30-60 NEW frames, which the PiP consumer shows (the "veo un
// segundo la scene anterior" glimpse) because those frames still advance so its stall-detection
// misses them and the 10-frame postFlipHold expires mid-fade. INSTANT skips the fade-out -> MM goes
// straight to Play_Init and STALLS during the load, so the consumer's stall-detection +
// postFlipHold keep the window black through the whole handoff. The reveal at the destination still
// fades in via nextTransitionType=FADE_BLACK.
void ArmArrivalTransition() {
    if (gPlayState == NULL) {
        return;
    }
    sLimboReturn.valid = false; // leaving the waiting room (or never in it): the stash is spent
    sArrivalEntrance = (u16)gSaveContext.save.entrance;
    gPlayState->nextEntrance = sArrivalEntrance;
    gPlayState->transitionTrigger = TRANS_TRIGGER_START;
    gPlayState->transitionType = TRANS_TYPE_INSTANT;
    // Under watch from here: if the FSM hasn't picked this up in kArrivalArmFrames we re-arm, and
    // after kArrivalMaxRetries we load the destination outright. See the watchdog in FleetWarp_Tick.
    sArrivalArmed = true;
    sArrivalWatchFrames = 0;
    sArrivalGrace = kArrivalGraceFrames;   // the squash guards must not eat our own trigger
    sArrivalWindow = kArrivalWindowFrames; // blunt recoveries are in scope for the next 30s
    // Destination armed: the black curtain the DEPARTING game left up can come down now. The
    // transition out of the waiting room is INSTANT and the destination fades in from black, so
    // nothing but black is ever on screen between the portal and the new scene.
    FleetShipCombo_SetSendFadeAlpha(0);
}

void FleetWarp_Tick() {
    if (FleetShipCombo_GetActiveGame() < 0) {
        return; // combo not running -> no-op (standalone MM unaffected)
    }
    if (sCooldown > 0) {
        sCooldown--;
    }
    if (gPlayState == NULL) {
        return;
    }

    if (sArrivalGrace > 0) {
        sArrivalGrace--;
    }
    if (sArrivalWindow > 0) {
        sArrivalWindow--;
    }

    // (0) CRASH GUARD — kill any transition primed with a GARBAGE entrance before
    // Play_UpdateTransition dereferences it. Valid MM entrances have scene index < 0x71.
    if (gPlayState->transitionTrigger != TRANS_TRIGGER_OFF && ((((u16)gPlayState->nextEntrance) >> 9) & 0x7F) >= 0x71) {
        gPlayState->transitionTrigger = TRANS_TRIGGER_OFF;
        gPlayState->transitionMode = TRANS_MODE_OFF;
        // If it was OUR arrival that got killed, the entrance we computed is known-good: put it
        // back and re-arm rather than dropping the player where they were. (A garbage nextEntrance
        // during an armed arrival means something else overwrote it between frames.)
        if (sArrivalArmed && sArrivalEntrance != 0) {
            SPDLOG_ERROR("[FleetWatchdog] arrival entrance was clobbered to {:#06x} -> restoring {:#06x}",
                         (int)(u16)gPlayState->nextEntrance, (int)sArrivalEntrance);
            gSaveContext.save.entrance = sArrivalEntrance;
            ArmArrivalTransition();
        }
    }
    // (0b) FROZEN GUARD — while we are the INACTIVE game, squash any freshly-set transition
    // trigger (e.g. the hole-fall completion landing AFTER the flip): a frozen game must never
    // start scene transitions on its own. Only the trigger — never a live transition mode.
    // EXCEPTION: our own just-armed arrival (sArrivalGrace). The active-game flag and the arrival
    // are set from different frames/hooks, so a single frame where we still read as inactive would
    // otherwise eat the arrival transition and strand the player in the departure scene — with the
    // warp already consumed, i.e. frozen with nothing pending.
    if (!FleetShipCombo_IsThisGameActive() && gPlayState->transitionTrigger != TRANS_TRIGGER_OFF &&
        gPlayState->transitionMode == TRANS_MODE_OFF && sArrivalGrace == 0 && !sLimboInFlight) {
        gPlayState->transitionTrigger = TRANS_TRIGGER_OFF;
    }

    // (0c) ARRIVAL-TRANSITION WATCHDOG — we armed a transition; make sure it actually starts.
    if (sArrivalArmed) {
        if (gPlayState->transitionMode != TRANS_MODE_OFF) {
            sArrivalArmed = false; // the FSM took it: the scene load is under way
            sArrivalWatchFrames = 0;
            sArrivalRetries = 0;
        } else if (++sArrivalWatchFrames > kArrivalArmFrames) {
            sArrivalWatchFrames = 0;
            if (++sArrivalRetries > kArrivalMaxRetries) {
                ForceDestinationReload("armed arrival transition never started");
            } else {
                SPDLOG_WARN("[FleetWatchdog] arrival transition did not start (try {}/{}) -> re-arming",
                            (int)sArrivalRetries, (int)kArrivalMaxRetries);
                ArmArrivalTransition();
            }
        }
    }

    // (0d) STUCK-FSM WATCHDOG — a transition that never completes IS a frozen game (the screen
    // holds whatever the fade left). Cancel it; if it was our arrival, land the destination the
    // blunt way instead of leaving the player in limbo.
    if (gPlayState->transitionMode != TRANS_MODE_OFF) {
        if (++sTransStuckFrames > kTransStuckFrames) {
            sTransStuckFrames = 0;
            if (sArrivalWindow > 0 && sArrivalEntrance != 0 && FleetShipCombo_IsThisGameActive()) {
                ForceDestinationReload("transition FSM stuck (never completed)");
            } else {
                SPDLOG_ERROR("[FleetWatchdog] transition FSM stuck (mode {}) -> cancelled",
                             (int)gPlayState->transitionMode);
                gPlayState->transitionTrigger = TRANS_TRIGGER_OFF;
                gPlayState->transitionMode = TRANS_MODE_OFF;
            }
        }
    } else {
        sTransStuckFrames = 0;
    }

    // (0e) STUCK-FADE WATCHDOG — the send fade is a black overlay the HOST draws. If it is up while
    // no send is in progress, the player is staring at a black screen that will never clear on its
    // own and reads as a hard freeze (input works, nothing is visible). Only the active game may
    // clear it: the frozen one is not the author of that value.
    if (FleetShipCombo_IsThisGameActive() && !sSendFlip && !sFlipCommitPending &&
        FleetShipCombo_GetSendFadeAlpha() != 0) {
        if (++sFadeStuckFrames > kFadeStuckFrames) {
            sFadeStuckFrames = 0;
            SPDLOG_ERROR("[FleetWatchdog] send-fade left at alpha {} with no warp in progress -> cleared",
                         FleetShipCombo_GetSendFadeAlpha());
            FleetShipCombo_SetSendFadeAlpha(0);
        }
    } else {
        sFadeStuckFrames = 0;
    }

    // (1) ARRIVAL is handled in FleetWarp_FileSelectTick (both the gameplay and file-select cases)
    // using the spiritual_stones respawn[TOP] recipe — see ApplyDestinationOverrides. It's NOT here
    // to avoid a double-consume (this draw hook runs before OnGameStateUpdate in the frame).

    // (2) FLEET-HOLE FALL (South Clock Town Door_Ana): falling INTO the hole (z_door_ana.c disabled
    // Link's floor and called FleetSync_OnHoleFall) starts the sending fade. Gated on a real file.
    if (FleetSync_HoleFallPending() && gSaveContext.fileNum > 2) {
        FleetSync_ClearHoleFall();
    }
    if (FleetSync_HoleFallPending() && !sSendFlip && sCooldown == 0) {
        FleetSync_ClearHoleFall();
        FleetSync_BeginPostFlipTrace(); // arm from the FIRST step of the swap, not just the flip
        FleetSync_PostFlipTrace("0. hole fall: MM -> OoT, fade starting");
        sSendFlip = true;
        sSendAlpha = 0;
        sSendScene = kOoTTotScene; // OoT Temple of Time
        sSendX = sSendY = sSendZ = 0.0f;
        sSendRotY = 0;
    }

    // (3) SENDING FADE (MM->OoT): squash only a freshly-set exit trigger (mode still OFF — never
    // stomp a live transition FSM: that was the Play_UpdateTransition crash), ramp the shared fade,
    // flip at full black with the FleetSync departure written first.
    if (sSendFlip) {
        if (gPlayState->transitionMode == TRANS_MODE_OFF) {
            gPlayState->transitionTrigger = TRANS_TRIGGER_OFF;
        }
        sSendAlpha += 9; // ~1.5s ramp (matches the OoT side)
        if (sSendAlpha >= 255) {
            sSendAlpha = 255;
            // WARP-OUT SAFETY: never flip to OoT while a scene-transition FSM is still LIVE
            // (transitionMode != OFF). The flip freezes MM mid-transition, and the inactive game keeps
            // running Play_UpdateTransition on scene state that OoT's takeover tears down / reallocates
            // -> the 2ship process dies right after the warp with NO C++ throw (that's why the existing
            // FrameGuard catch(...) can't stop it, and there's no crash trace: it's the documented
            // Play_UpdateTransition corruption, not an exception). The South Clock Town fleet hole
            // (Door_Ana) can leave a grotto transition live exactly at full black. Stay black and keep
            // waiting until the transition settles to OFF, THEN flip. The trigger-squash above keeps
            // any freshly re-armed exit trigger from starting a new transition while we hold.
            //
            // WATCHDOG: this hold has no natural end — if the FSM never settles (a transition that
            // can't complete, a scene that won't finish loading) we would sit at full black
            // FOREVER, which is exactly what a "the warp froze the game" report looks like. So the
            // hold is bounded: past the deadline, cancel the transition ourselves and flip anyway.
            // Flipping into a cancelled transition is survivable (the destination does a fresh
            // Play_Init); holding black forever is not.
            if (gPlayState->transitionMode != TRANS_MODE_OFF) {
                if (++sSendHoldFrames <= kSendHoldMaxFrames) {
                    FleetShipCombo_SetSendFadeAlpha(255); // hold full black; sSendFlip stays true -> retry next frame
                    return;
                }
                SPDLOG_ERROR("[FleetWatchdog] send fade held {} frames waiting for transitionMode (still {}) "
                             "-> cancelling it and flipping anyway",
                             (int)sSendHoldFrames, (int)gPlayState->transitionMode);
                gPlayState->transitionTrigger = TRANS_TRIGGER_OFF;
                gPlayState->transitionMode = TRANS_MODE_OFF;
            }
            sSendHoldFrames = 0;
            sSendFlip = false;
            // HAND OFF FROM THE UPDATE HOOK, NOT FROM HERE. We are running inside Play_Draw
            // (OnPlayDrawWorldEnd), so committing the flip here flips the game to INACTIVE halfway
            // through building this frame's display list — the render path then throws the whole
            // half-built DL away and swaps in an empty one under itself. The 2026-07-31 tester logs
            // end EXACTLY on this line, twice, with no C++ throw and no crash trace: 2ship dies
            // right after handing over. Deferring the commit to the update hook costs at most one
            // frame of black and takes the render out of the equation entirely.
            sFlipCommitPending = true;
            FleetShipCombo_SetSendFadeAlpha(255); // stay black until the commit lands
        } else {
            FleetShipCombo_SetSendFadeAlpha((int)sSendAlpha);
        }
        return;
    }

    // (4) LOST WOODS DOOR TRIGGER — only while MM is the active game, near our door.
    if (!FleetShipCombo_IsThisGameActive() || gPlayState->sceneId != SCENE_LOST_WOODS) {
        return; // DON'T reset armed here -> survives the scene-reload transition (no re-trigger loop)
    }
    if (gSaveContext.fileNum > 2) {
        return; // no REAL file loaded: the TITLE DEMO plays in Lost Woods with fileNum 0xFF and its
                // demo Link walks near the door -> a ghost flip that stomps the shared state
    }
    Player* player = GET_PLAYER(gPlayState);
    if (player == NULL) {
        return;
    }
    float dx = player->actor.world.pos.x - kMmDoorX;
    float dz = player->actor.world.pos.z - kMmDoorZ;
    if ((dx * dx + dz * dz) < (kDoorRadius * kDoorRadius)) {
        if (gPlayState->transitionMode == TRANS_MODE_OFF) {
            gPlayState->transitionTrigger = TRANS_TRIGGER_OFF; // squash the vanilla exit -> no reload
        }
        if (!sArmed && sCooldown == 0) {
            sArmed = true;
            FleetSync_BeginPostFlipTrace();
            FleetSync_PostFlipTrace("0. Lost Woods door: MM -> OoT, fade starting");
            sSendFlip = true;
            sSendAlpha = 0;
            sSendScene = kOoTScene; // Lost Woods door -> OoT Lost Woods at the door spot
            sSendX = kOoTDoorX;
            sSendY = kOoTDoorY;
            sSendZ = kOoTDoorZ;
            sSendRotY = kOoTDoorRotY;
        }
    } else if (gPlayState->transitionTrigger == TRANS_TRIGGER_OFF && gPlayState->transitionMode == TRANS_MODE_OFF) {
        // Link clearly OUTSIDE the zone in stable gameplay -> re-arm.
        sArmed = false;
    }
}

// COLD-BOOT / FILE-SELECT arrival: consume the warp, kick the file-select's REAL loader for the
// paired slot, and let the OnSaveLoad hook (fired at the very END of FileSelect_LoadGame, after
// Sram_OpenSave and every vanilla default) apply the FleetSync overlays + destination overrides.
// Applying them here (before the load completes) is what corrupted the arrival (crash C3).
void FleetWarp_FileSelectTick() {
    if (FleetShipCombo_GetActiveGame() < 0) {
        return; // combo not running
    }
    // [FleetArrive] heartbeat: fires on EVERY gamestate update (gameplay / file-select / title), so
    // if NO "arrival" line appears after a warp, this shows which state MM was stuck in.
    static int sHb = 0;
    if ((sHb++ % 180) == 0) {
        // Everything a stuck report needs, in one line: which state MM is in, whether it thinks it
        // is the active game, whether a warp is waiting to be applied, what the transition FSM is
        // doing, and whether the host is being told to paint the screen black. A "frozen" MM is
        // always one of those five being wrong.
        SPDLOG_INFO("[FleetArrive] MM heartbeat: play={} fsel={} activeGame={} thisActive={} pending={}({}f) "
                    "trig={} mode={} fadeAlpha={} entrance={:#06x} sendFlip={}",
                    (int)(gPlayState != NULL), (int)(gFileSelectState != NULL), FleetShipCombo_GetActiveGame(),
                    (int)FleetShipCombo_IsThisGameActive(), (int)sArrivalPending, (int)sArrivalPendingFrames,
                    gPlayState ? (int)gPlayState->transitionTrigger : -1,
                    gPlayState ? (int)gPlayState->transitionMode : -1, FleetShipCombo_GetSendFadeAlpha(),
                    (int)(u16)gSaveContext.save.entrance, (int)sSendFlip);
    }
    // ---- WAITING-ROOM UPKEEP (runs every frame, in or out of the room) ----
    if (LimboInRoom()) {
        // Time stands still while parked. The room's own time speed is 0, so this is only a
        // backstop against anything else that moves the clock (a song, a debug tool, a peer delta).
        if (sLimboReturn.valid) {
            gSaveContext.save.time = sLimboReturn.time;
            gSaveContext.save.day = sLimboReturn.day;
        }
        // Parked with the WRONG file: OoT loaded a different combo slot after we parked (it went
        // back to its file select and picked another file). Everything we hold belongs to the old
        // slot, so go back through the file select and park again with the right one — the same
        // path a cold boot takes, so it needs no second loader.
        if (!FleetShipCombo_IsThisGameActive() && gPlayState->transitionMode == TRANS_MODE_OFF) {
            const int comboSlot = FleetShipCombo_GetComboSlot();
            if (comboSlot >= 0 && comboSlot <= 2 && comboSlot != gSaveContext.fileNum) {
                SPDLOG_WARN("[FleetLimbo] parked with slot {} but OoT is on slot {} -> re-parking with the right file",
                            (int)gSaveContext.fileNum, comboSlot);
                sComboAutoLoaded = false;
                sLimboReturn.valid = false;
                STOP_GAMESTATE(&gPlayState->state);
                SET_NEXT_GAMESTATE(&gPlayState->state, FileSelect_Init, sizeof(FileSelectState));
                return;
            }
        }
    }

    // ---- DEFERRED FLIP COMMIT (MM -> OoT) ----
    // The send fade reached full black in the draw hook; the actual handover happens HERE, where no
    // display list is being built. Everything about it is ordered for one reason: the flip must
    // happen, and nothing before it may be able to prevent it.
    //   - WriteDeparture is best-effort bookkeeping (whole save -> JSON): guarded, and its failure
    //     only costs the peer a state sync, which FleetNet reconciles.
    //   - RequestWarp is the only thing that hands the player over. It runs unconditionally, last.
    // Before this was reordered, a throw in the departure write took the flip with it: the fade
    // stayed at 255 (the host paints that as a black screen) and OoT was never activated — a frozen
    // black MM with no crash to point at.
    if (sFlipCommitPending) {
        // Re-check the transition FSM one last time. The draw hook already waited for it to settle,
        // but a frame passed since then, and flipping mid-transition is the OTHER documented way to
        // kill 2ship (the frozen game keeps running Play_UpdateTransition on scene state OoT is
        // tearing down). Bounded by the same deadline as the fade hold: never wait forever.
        if (gPlayState != NULL && gPlayState->transitionMode != TRANS_MODE_OFF &&
            ++sSendHoldFrames <= kSendHoldMaxFrames) {
            FleetShipCombo_SetSendFadeAlpha(255); // hold black; retry next frame
            return;
        }
        sSendHoldFrames = 0;
        sFlipCommitPending = false;
        FleetSync_PostFlipTrace("1. commit: WriteDeparture next");
        try {
            FleetSync_WriteDeparture(gSaveContext.fileNum); // anchor + shared BEFORE the flip
        } catch (const std::exception& e) {
            SPDLOG_ERROR("[FleetWatchdog] departure write threw: {} — flipping anyway (the peer keeps its "
                         "last known state; FleetNet will reconcile)",
                         e.what());
        } catch (...) { SPDLOG_ERROR("[FleetWatchdog] departure write threw a non-std exception — flipping anyway"); }
        // Departure written. Start walking Link into the waiting room AND hand over in the same
        // frame: the room finishes loading in the background (sLimboInFlight keeps this game ticking
        // although it is already inactive), while OoT gets the player right away. The black curtain
        // (send fade) stays UP across the flip on purpose -- it is the ARRIVING game that lowers it,
        // the moment its destination transition is armed, so the player never sees the waiting room
        // on either side: black from the portal until the destination fades in.
        LimboEnterFromGameplay(); // no-op if the room is unavailable -> plain flip in place (old behaviour)
        FleetShipCombo_SetSendFadeAlpha(255);
        SPDLOG_INFO("[FleetArrive] MM committing flip to OoT (scene={:#x}) -- heading into the waiting room",
                    sSendScene);
        FleetSync_BeginPostFlipTrace();
        FleetShipCombo_RequestWarp(kOoTGame, sSendScene, sSendX, sSendY, sSendZ, sSendRotY, gSaveContext.fileNum);
        SPDLOG_INFO("[FleetArrive] MM flip committed -- OoT is active, MM is parking");
        return;
    }

    // ---- IN-FLIGHT BOOKKEEPING (MM -> waiting room, already inactive) ----
    // Clear the in-flight window once the room is up; bounded so a room that never loads cannot
    // keep an inactive game ticking forever (it then falls back to the old freeze).
    if (sLimboInFlight) {
        if (LimboInRoom() && gPlayState->transitionMode == TRANS_MODE_OFF) {
            sLimboInFlight = false;
            sLimboInFlightFrames = 0;
            SPDLOG_INFO("[FleetLimbo] MM parked in the waiting room");
        } else if (++sLimboInFlightFrames > kLimboWaitMaxFrames) {
            sLimboInFlight = false;
            sLimboInFlightFrames = 0;
            SPDLOG_ERROR("[FleetLimbo] waiting room not reached after {} frames (scene={:#x} mode={}) -- giving up, "
                         "inactive MM falls back to the freeze",
                         (int)kLimboWaitMaxFrames, gPlayState ? (int)gPlayState->sceneId : -1,
                         gPlayState ? (int)gPlayState->transitionMode : -1);
        }
    }

    // One-shot per process: reaching the file select after launch wipes the temp file.
    if (gPlayState == NULL && gFileSelectState != NULL) {
        FleetSync_OnTitleScreen();
    }
    // Single age counter for a consumed-but-not-yet-applied arrival. Every watchdog below reads it;
    // ApplyDestinationOverrides resets it. A warp that has been "pending" for thousands of frames is
    // the signature of a lost handshake, and it is the ONE thing we can always detect.
    if (sArrivalPending) {
        sArrivalPendingFrames++;
    } else {
        sArrivalPendingFrames = 0;
    }
    // GAMEPLAY arrival (repeat warps): the save is ALREADY loaded, so apply the destination
    // overrides (respawn[TOP] recipe) and start the spiritual_stones-style in-game FADE_BLACK
    // transition. respawn[TOP] survives the fade (unlike respawn[DOWN], which Player_Update
    // overwrites → the "lands where I left MM" bug).
    if (gPlayState != NULL) {
        // MM's TITLE DEMO also runs inside a PlayState (fileNum 0xFF). Consuming the warp here would run
        // a broken in-game transition on the demo and BURN the warp -> MM then hangs at the file select
        // with nothing pending. Only do the in-game arrival for a REAL loaded file (0..2); during the
        // demo do nothing and leave the warp PENDING for the file-select branch below.
        if (gSaveContext.fileNum > 2) {
            return;
        }
        // WATCHDOG — OnSaveLoad never fired. A file-select arrival hands the destination overrides to
        // the OnSaveLoad hook; if that hook doesn't run (it is fired from inside FileSelect_LoadGame,
        // so anything that makes that function return early loses it), the warp is already CONSUMED
        // and nothing is left to apply it: MM boots the save at its own entrance, or sits there. We
        // are now in gameplay with the load finished, which is the exact state the gameplay arrival
        // wants — so apply the overrides here instead. The player takes one extra scene load; the
        // alternative is a frozen or wrong-place arrival with the warp already spent.
        if (sArrivalPending && sArrivalPendingFrames > kArrivalHookFrames) {
            SPDLOG_ERROR("[FleetWatchdog] OnSaveLoad never fired after the file-select arrival "
                         "({} frames) -> applying the destination overrides in gameplay instead",
                         (int)sArrivalPendingFrames);
            ApplyDestinationOverrides(); // clears sArrivalPending
            ArmArrivalTransition();
            return;
        }
        // Never take a warp while a scene transition is still running (typically: we are parked
        // and the room's own fade-in has not finished, or the room is still loading). Consuming it
        // now would arm a second transition on top of a live one; the FSM then reads our START as
        // the completion of ITS fade and loads whatever nextEntrance holds. The warp is not lost —
        // it stays pending in shared memory until the FSM is idle a frame or two later.
        if (gPlayState->transitionMode != TRANS_MODE_OFF) {
            return;
        }
        int scene = 0, rotY = 0;
        float wx = 0.0f, wy = 0.0f, wz = 0.0f;
        if (FleetShipCombo_ConsumePendingWarp(&scene, &wx, &wy, &wz, &rotY)) {
            int slot = FleetShipCombo_GetWarpSaveFile();
            if (slot >= 0 && slot <= 2) {
                gSaveContext.fileNum = slot;
            }
            sArrivalScene = scene;
            sArrivalX = wx;
            sArrivalY = wy;
            sArrivalZ = wz;
            sArrivalRotY = rotY;
            ApplyDestinationOverrides(); // respawn[TOP] + respawnFlag=3 + save.entrance
            if (scene == FC_WARP_SCENE_RESUME) {
                if (LimboInRoom()) {
                    // Parked: "resume" means walk out of the waiting room back to the save's own
                    // place, which ApplyDestinationOverrides just restored into gSaveContext.
                    SPDLOG_INFO("[FleetArrive] MM RESUME warp while parked — leaving the waiting room to {:#06x}",
                                (int)(u16)gSaveContext.save.entrance);
                    ArmArrivalTransition();
                    return;
                }
                // A resume warp aimed at a game that is ALREADY running its save: there is nowhere
                // to travel to. Take the state overlay (done above) and stay put — reloading the
                // scene here would yank the player out of whatever they were doing.
                SPDLOG_INFO("[FleetArrive] MM RESUME warp while already in gameplay — no transition needed");
                FleetShipCombo_SetSendFadeAlpha(0); // nothing to wait for: lower the curtain
                return;
            }
            ArmArrivalTransition(); // in-game transition, under watchdog from here
            SPDLOG_INFO("[FleetArrive] MM GAMEPLAY warp: nextEntrance={:#06x} respawnFlag={} (spiritual_stones recipe)",
                        (int)(u16)gPlayState->nextEntrance, (int)gSaveContext.respawnFlag);
        }
        return; // gameplay arrival handled here
    }
    if (gFileSelectState == NULL) {
        sComboAutoLoaded = false; // MM left its file select (gameplay/title) -> re-arm the combo auto-load
        return;                   // not at the file-select screen yet (console logo / opening)
    }

    // Only act from the STABLE main file-select menu (not mid fade-in / copy / erase / name entry /
    // options): calling FileSelect_LoadGame from those sub-states is unsafe, and consuming a warp we
    // cannot yet apply would LOSE it. The warp stays pending in shared memory until the menu settles.
    if (gFileSelectState->menuMode != FS_MENU_MODE_CONFIG || gFileSelectState->configMode != CM_MAIN_MENU) {
        // WATCHDOG — the menu never settles. With a warp already consumed that is a permanent stall
        // (MM sits on a file select the player cannot even see, OoT waits for a game that never
        // arrives). Force the stable state so the retry below can run: a menu we shoved into its own
        // main state is recoverable, a warp stuck forever is not.
        if (sArrivalPending && sArrivalPendingFrames > kArrivalHookFrames) {
            SPDLOG_ERROR("[FleetWatchdog] file select stuck in menuMode={} configMode={} with a warp "
                         "pending -> forcing the main menu state",
                         (int)gFileSelectState->menuMode, (int)gFileSelectState->configMode);
            gFileSelectState->menuMode = FS_MENU_MODE_CONFIG;
            gFileSelectState->configMode = CM_MAIN_MENU;
        }
        return;
    }

    // WATCHDOG — FileSelect_LoadGame was called and nothing happened (we are still sitting in the
    // file select with the arrival pending). Kick it again; after kArrivalMaxRetries, give up on the
    // loader and say so loudly rather than retrying forever in silence.
    if (sArrivalPending && sArrivalPendingFrames > kArrivalLoadRetryFrames) {
        sArrivalPendingFrames = 0;
        if (++sArrivalLoadRetries > kArrivalMaxRetries) {
            SPDLOG_ERROR("[FleetWatchdog] FileSelect_LoadGame failed {} times for the arrival slot — "
                         "MM cannot boot the paired save (check the [FleetArrive] slot lines above)",
                         (int)sArrivalLoadRetries);
            sArrivalLoadRetries = 0;
            sArrivalPending = false; // stop the loop; the combo auto-load below can still try
            // LAST RESORT — hand the player back to OoT. MM cannot boot, and with the active game
            // pointed here BOTH games sit waiting: OoT is frozen off-screen and MM is on a file
            // select it can't load, which is the worst possible outcome — the player has no game at
            // all. Returning the focus leaves them exactly where they left OoT, still playable,
            // with a log line explaining why.
            SPDLOG_ERROR("[FleetWatchdog] returning control to OoT so the player is not left with a "
                         "frozen screen in either game");
            Notification::Emit({
                .prefix = "[Fleet] ",
                .message = "could not enter Majora's Mask — sent you back to Ocarina of Time",
                .suffix = " (please send your log)",
            });
            FleetShipCombo_SetActiveGame(0);
            FleetShipCombo_SetUiFocus(0);
            FleetShipCombo_SetSendFadeAlpha(0); // never leave the host painting black
        } else {
            int slot = FleetShipCombo_GetWarpSaveFile();
            if (slot < 0 || slot > 2) {
                slot = 0;
            }
            SPDLOG_WARN("[FleetWatchdog] arrival load did not start (try {}/{}) -> calling "
                        "FileSelect_LoadGame again for slot {}",
                        (int)sArrivalLoadRetries, (int)kArrivalMaxRetries, slot);
            if (EnsureComboSlot(slot)) {
                gFileSelectState->buttonIndex = (u16)slot;
                FileSelect_LoadGame(&gFileSelectState->state);
            }
            return;
        }
    }

    // Make the slot bootable BEFORE consuming the warp. ConsumePendingWarp is one-shot: consuming it
    // and then discovering we cannot boot the slot would BURN the warp and strand MM at the file
    // select with nothing pending. GetWarpSaveFile is a plain read, so the slot is known up front.
    // Gated on being the active game: ConsumePendingWarp only ever fires for the active side, so
    // this matches its precondition and keeps us from touching save files while MM sits frozen.
    if (FleetShipCombo_IsThisGameActive()) {
        int peekSlot = FleetShipCombo_GetWarpSaveFile();
        if (peekSlot < 0 || peekSlot > 2) {
            peekSlot = 0;
        }
        if (!EnsureComboSlot(peekSlot)) {
            return; // leave the warp PENDING; we'll retry next tick rather than lose it
        }
    }

    int scene = 0, rotY = 0;
    float wx = 0.0f, wy = 0.0f, wz = 0.0f;
    if (FleetShipCombo_ConsumePendingWarp(&scene, &wx, &wy, &wz, &rotY)) {
        sArrivalPending = true;
        sArrivalScene = scene;
        sArrivalX = wx;
        sArrivalY = wy;
        sArrivalZ = wz;
        sArrivalRotY = rotY;

        int slot = FleetShipCombo_GetWarpSaveFile();
        if (slot < 0 || slot > 2) {
            slot = 0;
        }
        SPDLOG_INFO("[FleetArrive] MM FILE-SELECT arrival: scene={:#x} slot={} -> FileSelect_LoadGame", scene, slot);
        gFileSelectState->buttonIndex = (u16)slot;     // slot already ensured before the consume above
        FileSelect_LoadGame(&gFileSelectState->state); // -> Sram_OpenSave -> defaults -> OnSaveLoad hook
        return;
    }

    // No manual warp pending. COMBO file-select BYPASS (OoT drives everything): auto-load the
    // designated combo slot so MM never shows a pickable file select.
    //   ACTIVE   -> load the slot at its own entrance (start-in-MM / Switch button), as before.
    //   INACTIVE -> load the slot too, but land in the WAITING ROOM: both saves are then live from
    //               the moment OoT loads a combo file, and the first portal is an ordinary in-game
    //               transition out of the room instead of a cold boot. Only once OoT has actually
    //               published a combo slot (a combo file is loaded over there) — before that MM
    //               waits on the file select as it always did.
    if (sComboAutoLoaded) {
        return;
    }
    const bool activeNow = FleetShipCombo_IsThisGameActive();
    if (!activeNow && FleetShipCombo_GetComboSlot() < 0) {
        return;
    }
    // Prefer the slot OoT published for this combo file (cross-process); fall back to the last warp
    // slot, then this process's remembered slot.
    int slot = FleetShipCombo_GetComboSlot();
    if (slot < 0 || slot > 2) {
        slot = FleetShipCombo_GetWarpSaveFile();
    }
    if (slot < 0 || slot > 2) {
        slot = CVarGetInteger("gFleetCombo.LastSlot", 0);
    }
    if (slot < 0 || slot > 2) {
        return;
    }
    // Empty/rejected slot: rebuild it rather than leaving the menu frozen (or booting a zeroed save).
    if (!EnsureComboSlot(slot)) {
        return;
    }
    sComboAutoLoaded = true;
    sBootIntoLimbo = !activeNow; // consumed by FleetWarp_OnSaveLoad: redirect the boot into the room
    gFileSelectState->buttonIndex = (u16)slot;
    SPDLOG_INFO("[FleetArrive] MM COMBO auto-load ({}, fleet mode): slot={} -> FileSelect_LoadGame{}",
                activeNow ? "active" : "inactive", slot, sBootIntoLimbo ? " -> waiting room" : "");
    FileSelect_LoadGame(&gFileSelectState->state); // loads the slot at its OWN entrance (resume or fresh)
}

// Fired at the END of FileSelect_LoadGame (z_file_choose_NES.c) — the exact moment the loaded save
// is fully in gSaveContext and nothing else will touch it before Play_Init.
void FleetWarp_OnSaveLoad(s16 fileNum) {
    (void)fileNum;
    if (FleetShipCombo_GetActiveGame() < 0) {
        return;
    }
    // Cold boot as the INACTIVE game: the file-select loader has just filled gSaveContext with the
    // save's own place. Remember it, then point the boot at the waiting room instead. Play_Init
    // (next gamestate) loads the room; a later warp/RESUME walks Link out of it.
    if (sBootIntoLimbo) {
        sBootIntoLimbo = false;
        if (!LimboAvailable()) {
            return; // boot the save at its own entrance instead; the freeze fallback covers it
        }
        LimboStashReturnState();
        LimboSetSaveToRoom();
        SPDLOG_INFO("[FleetLimbo] MM booting into the waiting room (fileNum={}, save's own entrance {:#06x})", fileNum,
                    (int)sLimboReturn.entrance);
        return;
    }
    if (!sArrivalPending) {
        return;
    }
    SPDLOG_INFO("[FleetArrive] MM OnSaveLoad fired (fileNum={}) -> ApplyDestinationOverrides", fileNum);
    ApplyDestinationOverrides();
}

// Hook wrapper: a throw out of ANY of these three would leave the warp half-applied (see the
// unconditional-flip note above) and, from the game loop's point of view, skip work with no trace.
// Everything they do is best-effort by nature, so swallow + log and let the watchdogs recover on the
// next frame instead of letting one bad frame decide the player is stuck.
template <typename Fn> void GuardedTick(const char* what, Fn&& fn) {
    try {
        fn();
    } catch (const std::exception& e) {
        SPDLOG_ERROR("[FleetWatchdog] {} threw: {} — frame skipped, watchdogs still armed", what, e.what());
    } catch (...) { SPDLOG_ERROR("[FleetWatchdog] {} threw a non-std exception — frame skipped", what); }
}

} // namespace

// ---- C-callable limbo queries (declared in FleetShipCombo.h) ----
extern "C" {

// "Should this process stop ticking its game?" — the freeze/render gates ask this instead of
// IsThisGameActive(). Parked in the waiting room the game keeps RUNNING (that is the whole point);
// the freeze only remains as the fallback for an inactive game that is NOT parked (limbo failed to
// load, or a title/file-select state where there is nothing to park).
int FleetShipCombo_IsGameSuspended(void) {
    if (FleetShipCombo_IsThisGameActive()) {
        return 0;
    }
    if (sLimboInFlight) {
        return 0; // still walking into the room: the transition must be allowed to finish
    }
    return LimboInRoom() ? 0 : 1;
}

int FleetShipCombo_IsParkedInLimbo(void) {
    return LimboInRoom() ? 1 : 0;
}

// Wrap a save write done while parked: the file must record the player's REAL place, never the
// waiting room. Begin swaps the stash in, End swaps the room back. Nesting is not supported (and
// not needed: the only caller is FleetSync's frozen-slot writer).
void FleetShipCombo_LimboSaveShadowBegin(void) {
    if (!LimboInRoom() || !sLimboReturn.valid) {
        return;
    }
    gSaveContext.save.entrance = sLimboReturn.entrance;
    gSaveContext.save.cutsceneIndex = sLimboReturn.cutsceneIndex;
    // The scene the file select shows / an owl resume lands in: the return entrance's scene, not
    // whatever the room set (the frozen writer stamps gPlayState->sceneId first).
    gSaveContext.save.saveInfo.playerData.savedSceneId = (s16)Entrance_GetSceneIdAbsolute(sLimboReturn.entrance);
}
void FleetShipCombo_LimboSaveShadowEnd(void) {
    if (!LimboInRoom() || !sLimboReturn.valid) {
        return;
    }
    gSaveContext.save.entrance = kLimboEntrance;
    gSaveContext.save.cutsceneIndex = 0;
    gSaveContext.save.saveInfo.playerData.savedSceneId = kLimboSceneId;
}

} // extern "C"

static void RegisterFleetWarpArrival() {
    LimboInstallScene(); // patch the scene + entrance tables before anything can boot a scene
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnPlayDrawWorldEnd>(
        []() { GuardedTick("FleetWarp_Tick", FleetWarp_Tick); });
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnGameStateUpdate>(
        []() { GuardedTick("FleetWarp_FileSelectTick", FleetWarp_FileSelectTick); });
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSaveLoad>(
        [](s16 fileNum) { GuardedTick("FleetWarp_OnSaveLoad", [fileNum]() { FleetWarp_OnSaveLoad(fileNum); }); });
}

static RegisterShipInitFunc initFunc(RegisterFleetWarpArrival, {});
