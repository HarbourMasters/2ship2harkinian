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
//
// VERSION-AWARE: an archive is only mirrored when its "portVersion" major matches its OWNER build
// (mm.o2r <-> this 2ship build, oot.o2r <-> the soh.o2r at the root), and then it OVERWRITES a copy
// that doesn't match. Existence-only mirroring bounced an outdated archive straight back into the dir
// the game had just deleted it from (re-deleted every boot / crash on the new resource format).
void FleetShipCombo_ProvisionO2rBothDirs(void);

// True when this process was launched by Ship as `2ship.exe --fleet-extract`: the ONLY job is to run
// the ROM extractor VISIBLY (window on-screen, no shared memory, no bounce) so mm.o2r gets built, then
// exit. Ship waits for us and runs its own extractor afterwards.
bool FleetShipCombo_IsExtractOnly(void);

// True when a mm.o2r matching THIS build sits next to 2ship.exe (after ProvisionO2rBothDirs pulled it
// in from the root if needed). The hosted child only parks its window off-screen when this holds:
// otherwise the extractor's popups must stay visible.
bool FleetShipCombo_HaveValidMmArchive(void);

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

// Scene sentinel for a RESUME warp: "hand the player to the other game AT ITS OWN SAVE", instead of
// at a portal. Used when a combo file whose last save was made in the other game is loaded — the
// arrival keeps the entrance/respawn the save itself carries (owl save included) and only takes the
// shared-state overlay. Every real scene id is >= 0, so -1 can never collide with one.
#define FC_WARP_SCENE_RESUME (-1)

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

// ---- "this frame actually rendered the game" ----
// The producer captures the game image through Window::GetGfxFrameBuffer(), which is a raw
// uintptr_t holding the renderer's CACHED ID3D11ShaderResourceView for the internal framebuffer.
// Two facts make that dangerous together:
//   1. The renderer only writes it while processing a display list, and it does NOT clear it when a
//      frame is skipped — so it keeps pointing at the previous frame's view.
//   2. That view is destroyed and recreated whenever the framebuffers are resized (window shown or
//      hidden, resolution multiplier changed).
// The frozen game renders an EMPTY display list (or nothing at all), and becoming frozen is what
// hides the window — so the producer could dereference a view the resize had already freed. Using a
// released COM object corrupts the D3D runtime's heap, and the process then dies somewhere else
// entirely, inside ntdll, with no exception and nothing in the log. That is the reported crash.
//
// So the render path marks the frames where the game display list really went through, and the
// producer only touches the framebuffer handle on those. Marking is per frame: the producer
// consumes the flag.
void FleetShipCombo_MarkGameFrameRendered(void);

// Sending-side fade overlay alpha (0..255): the active game ramps it while Link walks into the door
// (no scene transition); the host PiP consumer draws black at this alpha over the scene for a real
// fade-out, then flips at full black. Same-process (host) read/write.
void FleetShipCombo_SetSendFadeAlpha(int alpha);
int FleetShipCombo_GetSendFadeAlpha(void);

// DEV: index of the Lost Woods room display list the MM door-tunnel tool is currently showing, shared
// so Ship's on-screen overlay can display it while you cycle to find the tunnel piece.
void FleetShipCombo_SetDoorDLIndex(int index);
int FleetShipCombo_GetDoorDLIndex(void);

// ---- Combo seed identity ----
// The Rando finalSeed OoT generated for this combo. OoT publishes it; MM validates the finalSeed
// baked into its paired save against it and rebuilds the slot when they disagree, so an MM file
// left over from an older seed can never be played against a newer OoT seed. 0 = unset.
void FleetShipCombo_SetComboSeed(unsigned int seed);
unsigned int FleetShipCombo_GetComboSeed(void);

// ---- Anchor-style packet channel (shared-memory rings, region version 2) ----
// The transport under FleetNet: one JSON message per call, same shape as an Anchor packet, but
// through shared memory instead of a socket. Two one-way rings mean no lock and no file, so a
// delta costs microseconds and cannot hit the oracle's "sharing violation" retry loop.
// Push returns 1 if the packet was queued (0 = no combo, peer too old, or payload > 1023 bytes).
// Pop fills `out` with ONE pending packet and returns 1, or returns 0 when the queue is empty --
// call it in a loop from the per-frame pump until it returns 0.
int FleetShipCombo_PushPacket(const char* json);
int FleetShipCombo_PopPacket(char* out, int cap);

// True if THIS process is the active game, OR if shared memory is unavailable
// (standalone). Drives input blocking, audio mute and the warp triggers.
bool FleetShipCombo_IsThisGameActive(void);

// ---- Waiting room ("limbo") ----
// The inactive game is no longer frozen: before handing over it parks Link in a sealed custom
// scene ("fleet_scene") and keeps RUNNING there. These are what the two engine gates ask instead
// of IsThisGameActive():
//   IsGameSuspended  -> 1 only for an inactive game that is NOT parked (limbo unavailable, or a
//                       title/file-select state with nothing to park). That is the old freeze,
//                       kept purely as the fallback. 0 = tick and draw normally.
//   IsParkedInLimbo  -> 1 while the loaded scene is the waiting room.
// The save-shadow pair wraps a save write done while parked so the file records the player's real
// place (entrance / cutscene / scene) and never the waiting room. Implemented in FleetWarpArrival.cpp.
int FleetShipCombo_IsGameSuspended(void);
int FleetShipCombo_IsParkedInLimbo(void);
void FleetShipCombo_LimboSaveShadowBegin(void);
void FleetShipCombo_LimboSaveShadowEnd(void);

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

// Read at menu-REGISTRATION time (boot), so "isFleetShipCombo.DevUi" needs a restart to take effect.
bool FleetShipCombo_ShowMenuUi(void);

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

// Hand the combo back to Ocarina of Time: active game 0, front window 0, isPlayerIn2Ship 0.
// MM's OWN title screen / file select must never be somewhere the player can act, so every path
// that lands MM back on them (a restart, ours or OoT's) ends by yielding: MM freezes on its logo
// off-screen and the player sees OoT's title instead. Idempotent; no-op outside a combo.
void FleetShipCombo_YieldToOoT(void);

// ---- Guest heartbeat (reservedU[0]) ----
// Bumped by 2ship every frame from PollHostAlive, which runs OUTSIDE the render gating, so it keeps
// ticking while MM is the frozen/inactive game. Ship watches it: a heartbeat that stopped while
// MM's process is still alive is a HANG, and a hung guest leaves Ship staring at a stale shared
// texture forever (the black screen), so Ship tears the whole combo down instead.
void FleetShipCombo_BeatHeartbeat(void);
unsigned long long FleetShipCombo_GetGuestHeartbeat(void);

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
