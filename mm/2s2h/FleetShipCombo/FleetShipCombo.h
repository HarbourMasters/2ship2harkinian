#ifndef FLEET_SHIP_COMBO_H
#define FLEET_SHIP_COMBO_H

#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Fleet Ship Combo bootstrap (Frente A).
//
// Ship (Ocarina of Time / Shipwright) is ALWAYS the host. When 2ship is launched
// directly with the combo enabled, it hands off to Ship and exits, so the user
// ends up inside Ship as the host (with Ship auto-booting into MM via --boot=mm).
//
// CVar namespace ("isFleetShipCombo[X]"):
//   isFleetShipCombo.Enabled    - master toggle for the combo / this bootstrap
//   isFleetShipCombo.ActiveGame - 0 = Ocarina of Time (Ship), 1 = Majora's Mask (2ship)
//
// Returns true if it launched Ship and the caller should EXIT this 2ship process.
// Returns false to continue booting 2ship standalone (combo off, hosted child, or
// Ship not found -> graceful fallback that also clears isFleetShipCombo.Enabled).
//
// Must be called early in boot, AFTER console variables are loaded but BEFORE the
// window / resource manager are created (so a bounce never flashes a 2ship window).
bool FleetShipCombo_BootstrapMaybeRelaunch(int argc, char** argv);

// Mirror mm.o2r + oot.o2r so both sit next to BOTH exes (combo layout: root + /2ship). Call around the
// extractor: an o2r extracted by either game is copied into the sibling dir, and a missing one that's
// present in the sibling is pulled in. No-op with no sibling (standalone).
void FleetShipCombo_ProvisionO2rBothDirs(void);

// ---- Shared-memory coordination (Frente B) ----
// A named shared-memory region carries the active-game flag (and later the D3D11
// shared texture handle + per-frame sync) between Ship and 2ship.
//   activeGame: 0 = Ocarina of Time (Ship), 1 = Majora's Mask (2ship)
//
// Opens (or creates) the shared region. Safe to call more than once. Only meaningful
// in combo mode; standalone 2ship never calls it, so IsThisGameActive() stays true.
// instanceKey (the host Ship's PID) makes the region name UNIQUE per combo, so several
// combos can run on one machine without colliding. Pass 0 to use the legacy unsuffixed name.
void FleetShipCombo_SharedInit(unsigned long instanceKey);

// Active game stored in shared memory, or -1 if the region is unavailable.
int FleetShipCombo_GetActiveGame(void);

// Write the active game to shared memory (used by the Switch button).
void FleetShipCombo_SetActiveGame(int game);

// ---- Cross-game loading-zone WARP (the world connector) ----
// Trigger side: record the target (in the TARGET game's scene-id + world coords), bump the warp
// seq, and flip activeGame so the target game becomes active and applies it. targetGame: 0 = OoT
// (Ship), 1 = MM (2ship). rotY is the s16 binary-angle Link should face on arrival. saveFile is the
// save SLOT the trigger is in (e.g. gSaveContext.fileNum) so the target game lands in its own same slot.
void FleetShipCombo_RequestWarp(int targetGame, int scene, float x, float y, float z, int rotY, int saveFile);

// Applied by whichever game just became active: returns 1 ONCE per new request when a warp is
// addressed to THIS game, filling the target scene + land position/rotation. The receiver then
// loads `scene` and overrides Link's pos/rot to (x,y,z,rotY). Any out-param may be null.
int FleetShipCombo_ConsumePendingWarp(int* scene, float* x, float* y, float* z, int* rotY);

// The save SLOT the pending warp came from (set by RequestWarp). The receiver loads its own save at
// this slot so OoT file_1 <-> MM file_1. Returns -1 if unset/unavailable.
int FleetShipCombo_GetWarpSaveFile(void);

// Cross-game arrival blackout: paint THIS game's screen black for `frames` frames so the stale frame
// of the other game + the warp scene-load are hidden during a flip. The render path queries
// ...Active() once per frame (it decrements and returns 1 while still blacking out).
void FleetShipCombo_BeginArrivalBlackout(int frames);
int FleetShipCombo_ArrivalBlackoutActive(void);

// Sending-side fade overlay alpha (0..255): the active game ramps it while Link walks into the door
// (no scene transition); the host PiP consumer draws black at this alpha over the scene for a real
// fade-out, then flips at full black. Same-process (host) read/write.
void FleetShipCombo_SetSendFadeAlpha(int alpha);
int FleetShipCombo_GetSendFadeAlpha(void);

// DEV: index of the Lost Woods room display list the MM door-tunnel tool is currently showing, shared
// so Ship's on-screen overlay can display it while you cycle to find the tunnel piece.
void FleetShipCombo_SetDoorDLIndex(int index);
int FleetShipCombo_GetDoorDLIndex(void);

// True if THIS process is the active game, OR if shared memory is unavailable
// (standalone). Drives the FrameAdvance freeze of the inactive game.
bool FleetShipCombo_IsThisGameActive(void);

// ---- Picture-in-picture: shared D3D11 game texture (Frente B B2-B4) ----
// 2ship (producer) copies its rendered game image into a D3D11 shared texture and
// publishes the OS shared handle here; Ship (consumer) opens it and draws it in an
// ImGui panel. All via public libultraship APIs + COM (no submodule changes).

// Producer: capture this frame's game image to the shared texture + publish the handle.
// Call once per frame (2ship). No-op when not in combo.
void FleetShipCombo_ProducerPublishFrame(void);

// Hide 2ship's own OS window(s) so the combo shows only ONE window (Ship). Idempotent;
// call early (right after the window is created) and each frame. Windows-only / no-op
// elsewhere. 2ship keeps rendering while hidden.
void FleetShipCombo_HideGuestWindow(void);

// Watch the host (Ship) process by PID; once it exits, 2ship exits too (no orphan).
// Called from the bootstrap with the --fleet-host-pid value.
void FleetShipCombo_WatchHost(unsigned long hostPid);

// Host-death watchdog: if the host (Ship) has exited, exit this process too (no orphan).
// Cheap (one 0-timeout WaitForSingleObject); a strict no-op when not a hosted child (standalone
// has no host handle) or while the host is alive. Call EVERY frame, independent of any render
// gating, so it still runs when this game is inactive and skips ProducerPublishFrame.
void FleetShipCombo_PollHostAlive(void);

// UI focus: which game's window is in front for CONFIG (0 = Ship, 1 = 2ship). Independent
// of the active game. The Ship NEI "View" selector sets it; 2ship reads it to show/hide
// its own window (so the user can reach 2ship's BenGui).
int FleetShipCombo_GetUiFocus(void);
void FleetShipCombo_SetUiFocus(int focus);

// ---- FleetSync save-sync handshake (reservedU[1..3]) ----
// The game that just SAVED signals; the other (frozen) exe applies the shared overlay from the
// temp file, saves its own slot, and acks with the seq it processed. See FleetSync.cpp.
void FleetShipCombo_SignalSyncSave(int slot);
unsigned long long FleetShipCombo_GetSyncSaveSeq(void);
int FleetShipCombo_GetSyncSaveSlot(void);
void FleetShipCombo_AckSyncSave(unsigned long long seq);
unsigned long long FleetShipCombo_GetSyncSaveAck(void);

// ---- Fleet Oracle (combo randomizer generation) handshake (reservedU[4..5]) ----
// The HOST (Ship) writes fleet_oracle_req.json in its own dir and bumps the request seq; the MM
// oracle (FleetOracle.cpp) answers into fleet_oracle_resp.json and acks the seq. See FleetOracle.cpp.
void FleetShipCombo_SignalOracleRequest(void);
unsigned long long FleetShipCombo_GetOracleRequestSeq(void);
void FleetShipCombo_AckOracleResponse(unsigned long long seq);
unsigned long long FleetShipCombo_GetOracleResponseAck(void);

// ---- Shared-window open request (reservedU[6]) ----
// El tab "Shared" de 2ship bumpea el contador; Ship abre su ventana Fleet Shared al verlo.
void FleetShipCombo_RequestSharedWindowOpen(void);
unsigned long long FleetShipCombo_GetSharedWindowOpenSeq(void);

// ---- Cross-game RESTART (reservedU[10]) ----
// A reset in one game signals; the other game's per-frame pump consumes it and resets itself too,
// so "restart one, restart both". SignalRestart marks the bump as ours (we never respond to our own
// reset); ConsumeRestartRequest returns 1 ONCE when the OTHER game reset.
void FleetShipCombo_SignalRestart(void);
int FleetShipCombo_ConsumeRestartRequest(void);

// Combo active save slot published to shared memory by OoT (0..2), so MM auto-loads the same slot
// when it becomes the active game. -1 when unset. (reservedU[11].)
void FleetShipCombo_SetComboSlot(int slot);
int FleetShipCombo_GetComboSlot(void);

// Per-frame: show 2ship's window on-screen+front when uiFocus==1 (so its BenGui is usable),
// otherwise park it off-screen as a tool window. Windows-only / no-op elsewhere.
void FleetShipCombo_UpdateGuestWindow(void);

// Store the shared-texture descriptor in shared memory (called by the producer).
void FleetShipCombo_PublishSharedTexture(unsigned long long handle, unsigned int width, unsigned int height,
                                         unsigned int dxgiFormat, unsigned int frameIndex);

// Read the shared-texture descriptor (called by the consumer). Returns 1 if a handle
// is present, 0 otherwise. Any out-param may be null.
int FleetShipCombo_GetSharedTexture(unsigned long long* handle, unsigned int* width, unsigned int* height,
                                    unsigned int* dxgiFormat, unsigned int* frameIndex);

// ---- UI-overlay texture (reservedU[7..9]) ----
// SECOND shared texture with ONLY 2ship's ImGui windows (trackers etc.) on a transparent
// background — excludes the game image and the BenGui main menu. Ship draws it on top of
// whichever game is active so both games' trackers can be visible at once. The 2ship side
// publishes it every frame it renders (gated by gFleetShipCombo.UiOverlay, default on);
// Ship's display of it is gated by gFleetCombo.ShowMmUiOverlay.
void FleetShipCombo_PublishUiTexture(unsigned long long handle, unsigned int width, unsigned int height,
                                     unsigned int dxgiFormat, unsigned int frameIndex);
int FleetShipCombo_GetUiTexture(unsigned long long* handle, unsigned int* width, unsigned int* height,
                                unsigned int* dxgiFormat, unsigned int* frameIndex);

#ifdef __cplusplus
}
#endif

#endif // FLEET_SHIP_COMBO_H
