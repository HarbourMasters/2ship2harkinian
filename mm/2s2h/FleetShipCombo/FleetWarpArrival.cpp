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
#include <libultraship/bridge/consolevariablebridge.h> // CVar: persist the combo slot for auto-resume
#include <spdlog/spdlog.h> // [FleetArrive] diagnostics

extern "C" {
#include "z64.h"
#include "macros.h"
#include "functions.h"
#include "variables.h"
#include "src/overlays/gamestates/ovl_file_choose/z_file_select.h"
extern PlayState* gPlayState;
extern SaveContext gSaveContext;
extern FileSelectState* gFileSelectState;     // non-null only while at the file-select screen
extern void FileSelect_LoadGame(GameState* thisx); // SM_LOAD_GAME handler (non-static; not in the header)
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

bool sArmed = false;    // Lost Woods door: don't re-trigger while standing on it
bool sHoleArmed = false; // Clock Town fleet hole: re-arms when Link steps off the hole spot
s16 sCooldown = 0;   // suppress the trigger right after any warp (bridges the scene reload)
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

    FleetSync_ApplyArrival(slot);

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
    sArmed = true;     // Lost Woods door: don't instantly flip back
    sHoleArmed = true; // Clock Town hole: Link pops OUT on the spot -> suppress until he steps off
    sCooldown = 40;
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

    // (0) CRASH GUARD — kill any transition primed with a GARBAGE entrance before
    // Play_UpdateTransition dereferences it. Valid MM entrances have scene index < 0x71.
    if (gPlayState->transitionTrigger != TRANS_TRIGGER_OFF &&
        ((((u16)gPlayState->nextEntrance) >> 9) & 0x7F) >= 0x71) {
        gPlayState->transitionTrigger = TRANS_TRIGGER_OFF;
        gPlayState->transitionMode = TRANS_MODE_OFF;
    }
    // (0b) FROZEN GUARD — while we are the INACTIVE game, squash any freshly-set transition
    // trigger (e.g. the hole-fall completion landing AFTER the flip): a frozen game must never
    // start scene transitions on its own. Only the trigger — never a live transition mode.
    if (!FleetShipCombo_IsThisGameActive() && gPlayState->transitionTrigger != TRANS_TRIGGER_OFF &&
        gPlayState->transitionMode == TRANS_MODE_OFF) {
        gPlayState->transitionTrigger = TRANS_TRIGGER_OFF;
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
            if (gPlayState->transitionMode != TRANS_MODE_OFF) {
                FleetShipCombo_SetSendFadeAlpha(255); // hold full black; sSendFlip stays true -> retry next frame
                return;
            }
            sSendFlip = false;
            FleetSync_WriteDeparture(gSaveContext.fileNum); // anchor + shared BEFORE the flip
            FleetShipCombo_SetSendFadeAlpha(0); // OoT (now active) owns the black from here
            FleetShipCombo_RequestWarp(kOoTGame, sSendScene, sSendX, sSendY, sSendZ, sSendRotY,
                                       gSaveContext.fileNum);
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
        SPDLOG_INFO("[FleetArrive] MM heartbeat: play={} fsel={} activeGame={} thisActive={}",
                    (int)(gPlayState != NULL), (int)(gFileSelectState != NULL), FleetShipCombo_GetActiveGame(),
                    (int)FleetShipCombo_IsThisGameActive());
    }
    // One-shot per process: reaching the file select after launch wipes the temp file.
    if (gPlayState == NULL && gFileSelectState != NULL) {
        FleetSync_OnTitleScreen();
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
            // In-game transition to the destination (respawn[TOP] survives to Play_Init). The OUT is
            // TRANS_TYPE_INSTANT — NOT FADE_BLACK — on PURPOSE: a FADE_BLACK out fades the OLD (frozen,
            // just-flipped-to) MM scene over ~30-60 NEW frames, which the PiP consumer shows (the
            // "veo un segundo la scene anterior" glimpse) because those frames still advance so its
            // stall-detection misses them and the 10-frame postFlipHold expires mid-fade. INSTANT
            // skips the fade-out -> MM goes straight to Play_Init and STALLS during the load, so the
            // consumer's stall-detection + postFlipHold keep the window black through the whole
            // handoff. The reveal at the destination still fades in via nextTransitionType=FADE_BLACK.
            gPlayState->nextEntrance = (u16)gSaveContext.save.entrance;
            gPlayState->transitionTrigger = TRANS_TRIGGER_START;
            gPlayState->transitionType = TRANS_TYPE_INSTANT;
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
        return;
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
        gFileSelectState->buttonIndex = (u16)slot;
        FileSelect_LoadGame(&gFileSelectState->state); // -> Sram_OpenSave -> defaults -> OnSaveLoad hook
        return;
    }

    // No manual warp pending. COMBO file-select BYPASS (OoT drives everything): when MM is the
    // ACTIVE game, auto-load the designated combo slot so entering MM (start-in-MM at boot, or the
    // Switch button) drops STRAIGHT into the paired save — MM never shows a pickable file select.
    // When MM is INACTIVE it just sits here frozen/off-screen (OoT is on screen), so no auto-load.
    if (!FleetShipCombo_IsThisGameActive() || sComboAutoLoaded) {
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
    if (slot < 0 || slot > 2 || !SLOT_OCCUPIED(gFileSelectState, slot)) {
        return; // no valid combo save in the slot -> don't boot garbage (leave the menu frozen)
    }
    sComboAutoLoaded = true;
    gFileSelectState->buttonIndex = (u16)slot;
    SPDLOG_INFO("[FleetArrive] MM COMBO auto-load (active, fleet mode): slot={} -> FileSelect_LoadGame", slot);
    FileSelect_LoadGame(&gFileSelectState->state); // loads the slot at its OWN entrance (resume or fresh)
}

// Fired at the END of FileSelect_LoadGame (z_file_choose_NES.c) — the exact moment the loaded save
// is fully in gSaveContext and nothing else will touch it before Play_Init.
void FleetWarp_OnSaveLoad(s16 fileNum) {
    (void)fileNum;
    if (FleetShipCombo_GetActiveGame() < 0 || !sArrivalPending) {
        return;
    }
    SPDLOG_INFO("[FleetArrive] MM OnSaveLoad fired (fileNum={}) -> ApplyDestinationOverrides", fileNum);
    ApplyDestinationOverrides();
}

} // namespace

static void RegisterFleetWarpArrival() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnPlayDrawWorldEnd>(FleetWarp_Tick);
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnGameStateUpdate>(FleetWarp_FileSelectTick);
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSaveLoad>(FleetWarp_OnSaveLoad);
}

static RegisterShipInitFunc initFunc(RegisterFleetWarpArrival, {});
