// Fleet Ship Combo - picture-in-picture PRODUCER (2ship side).
//
// Copies 2ship's rendered game image into a D3D11 shared texture each frame and
// publishes the OS shared handle into shared memory. Ship (consumer) opens it and
// draws it in an ImGui panel. Uses ONLY public libultraship APIs + D3D11 COM calls
// on objects we already hold, so there are NO changes to the libultraship submodule.
//
// How we get the game image without touching the submodule:
//   Window::GetGfxFrameBuffer() returns the game framebuffer's ID3D11ShaderResourceView
//   (as uintptr_t) BUT only when the game renders to an offscreen framebuffer, which
//   happens when the internal resolution multiplier != 1.0x. So we force a small
//   multiplier. From the SRV we reach the texture (GetResource), device (GetDevice)
//   and context (GetImmediateContext) purely via COM.

#include "FleetShipCombo.h"

#include <libultraship/libultraship.h>
#include <fast/Fast3dWindow.h> // Fast::Fast3dWindow / Fast::WindowBackend (renderer guard below)
#include "2s2h/BenGui/Notification.h" // on-screen warning when the renderer cannot share frames
#include <spdlog/spdlog.h>
#include <memory>
#include <cstdlib>
#include <cstring>

#ifndef _WIN32
#include <sys/types.h>
#include <signal.h> // kill()
#include <errno.h>  // errno / ESRCH
namespace {
// Host (Ship) PID to watch on POSIX; 0 = not a hosted child (standalone) -> watchdog no-ops.
unsigned long sPosixHostPid = 0;
} // namespace
#endif

#ifdef _WIN32
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <shobjidl.h> // ITaskbarList (force-remove the hidden window from the taskbar)
#include <imgui.h>
#include <imgui_impl_dx11.h> // second RenderDrawData pass into the UI-overlay shared texture

namespace {
ID3D11Texture2D* sSharedTex = nullptr;
IDXGIKeyedMutex* sSharedMutex = nullptr;
HANDLE sSharedHandle = nullptr;
HANDLE sHostProcess = nullptr; // Ship's process; when it exits, we exit too (no orphan)
DWORD sHostPid = 0;            // Ship's PID, to find its window for seamless overlay
RECT sHostRect = {};           // Ship's window rect (filled before showing 2ship on top of it)
bool sHaveHostRect = false;
HWND sHostHwnd = nullptr;      // Ship's window handle, to hand OS focus back when OoT becomes active
uint32_t sSharedW = 0;
uint32_t sSharedH = 0;
uint32_t sSharedFmt = 0;
uint32_t sFrameIndex = 0;

void ReleaseSharedTex() {
    if (sSharedMutex) {
        sSharedMutex->Release();
        sSharedMutex = nullptr;
    }
    if (sSharedTex) {
        sSharedTex->Release();
        sSharedTex = nullptr;
    }
    sSharedHandle = nullptr;
    sSharedW = sSharedH = sSharedFmt = 0;
}

// ---------------- UI-overlay capture (2nd shared texture: ImGui windows ONLY) ----------------
// Ship's consumer draws the GAME framebuffer, which is pre-ImGui, so 2ship's trackers were
// invisible in the combo no matter the active game. Here we replay this frame's ImGui draw
// data (already built by the normal render; valid until the next NewFrame) into a second
// shared texture with a TRANSPARENT clear, skipping the "Main Game" image and the "Main Menu"
// (BenGui) windows, so Ship can overlay just the floating UI (trackers etc.) on either game.
ID3D11Texture2D* sUiTex = nullptr;
ID3D11RenderTargetView* sUiRtv = nullptr;
IDXGIKeyedMutex* sUiMutex = nullptr;
HANDLE sUiHandle = nullptr;
uint32_t sUiW = 0;
uint32_t sUiH = 0;
uint32_t sUiFrameIndex = 0;

void ReleaseUiTex() {
    if (sUiRtv) {
        sUiRtv->Release();
        sUiRtv = nullptr;
    }
    if (sUiMutex) {
        sUiMutex->Release();
        sUiMutex = nullptr;
    }
    if (sUiTex) {
        sUiTex->Release();
        sUiTex = nullptr;
    }
    sUiHandle = nullptr;
    sUiW = sUiH = 0;
}

bool UiListIsExcluded(const ImDrawList* list) {
    const char* name = list->_OwnerName;
    if (name == nullptr) {
        return false; // viewport background/foreground lists: keep (notifications etc.)
    }
    // Prefix match also catches child windows ("Main Menu/Menu Block_XXXX").
    return strncmp(name, "Main Game", 9) == 0 || strncmp(name, "Main Menu", 9) == 0;
}

void PublishUiOverlay(ID3D11Device* dev, ID3D11DeviceContext* ctx) {
    ImDrawData* src = ImGui::GetDrawData();
    if (src == nullptr || !src->Valid) {
        return;
    }
    uint32_t w = (uint32_t)src->DisplaySize.x;
    uint32_t h = (uint32_t)src->DisplaySize.y;
    if (w == 0 || h == 0) {
        return;
    }

    if (!sUiTex || sUiW != w || sUiH != h) {
        ReleaseUiTex();
        D3D11_TEXTURE2D_DESC td = {};
        td.Width = w;
        td.Height = h;
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        td.MiscFlags = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;
        if (SUCCEEDED(dev->CreateTexture2D(&td, nullptr, &sUiTex)) && sUiTex &&
            SUCCEEDED(dev->CreateRenderTargetView(sUiTex, nullptr, &sUiRtv)) && sUiRtv) {
            IDXGIResource* dxgiRes = nullptr;
            if (SUCCEEDED(sUiTex->QueryInterface(__uuidof(IDXGIResource), reinterpret_cast<void**>(&dxgiRes))) &&
                dxgiRes) {
                dxgiRes->GetSharedHandle(&sUiHandle);
                dxgiRes->Release();
            }
            sUiTex->QueryInterface(__uuidof(IDXGIKeyedMutex), reinterpret_cast<void**>(&sUiMutex));
            sUiW = w;
            sUiH = h;
            SPDLOG_INFO("[FleetShipCombo] Producer UI-overlay texture {}x{} handle {}", w, h, (void*)sUiHandle);
        } else {
            ReleaseUiTex();
            return;
        }
    }
    if (!sUiTex || !sUiHandle || !sUiMutex || !sUiRtv) {
        return;
    }

    // Shallow struct copy duplicates the CmdLists pointer-array (ImVector copy), NOT the lists;
    // we then swap in the filtered set. The lists themselves stay owned by ImGui.
    static ImVector<ImDrawList*> sFiltered;
    sFiltered.resize(0);
    int totalVtx = 0;
    int totalIdx = 0;
    for (int i = 0; i < src->CmdLists.Size; i++) {
        ImDrawList* list = src->CmdLists[i];
        if (list == nullptr || UiListIsExcluded(list)) {
            continue;
        }
        sFiltered.push_back(list);
        totalVtx += list->VtxBuffer.Size;
        totalIdx += list->IdxBuffer.Size;
    }
    ImDrawData ui = *src;
    ui.CmdLists = sFiltered;
    ui.CmdListsCount = sFiltered.Size;
    ui.TotalVtxCount = totalVtx;
    ui.TotalIdxCount = totalIdx;

    // Save the bound render target + viewport; the ImGui DX11 backend renders into whatever
    // target is currently bound and does not restore state.
    ID3D11RenderTargetView* prevRtv = nullptr;
    ID3D11DepthStencilView* prevDsv = nullptr;
    ctx->OMGetRenderTargets(1, &prevRtv, &prevDsv);
    UINT prevVpCount = 1;
    D3D11_VIEWPORT prevVp = {};
    ctx->RSGetViewports(&prevVpCount, &prevVp);

    if (sUiMutex->AcquireSync(0, 8) == S_OK) {
        const float kClear[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; // transparent: only the UI pixels land
        ctx->OMSetRenderTargets(1, &sUiRtv, nullptr);
        ctx->ClearRenderTargetView(sUiRtv, kClear);
        if (ui.CmdListsCount > 0) {
            ImGui_ImplDX11_RenderDrawData(&ui);
        }
        ctx->Flush();
        sUiMutex->ReleaseSync(0);
        sUiFrameIndex++;
        FleetShipCombo_PublishUiTexture(static_cast<unsigned long long>(reinterpret_cast<uintptr_t>(sUiHandle)), w, h,
                                        (uint32_t)DXGI_FORMAT_R8G8B8A8_UNORM, sUiFrameIndex);
    }

    ctx->OMSetRenderTargets(1, &prevRtv, prevDsv);
    if (prevVpCount > 0) {
        ctx->RSSetViewports(1, &prevVp);
    }
    if (prevRtv) {
        prevRtv->Release();
    }
    if (prevDsv) {
        prevDsv->Release();
    }
}

WNDPROC sOrigWndProc = nullptr;
HWND sSubclassedHwnd = nullptr;

// Subclassed window proc for 2ship's game window while in combo mode. Clicking the X (or
// Alt+F4) on the config window would otherwise post WM_CLOSE -> SDL quits -> the whole
// 2ship session dies and the combo breaks. Instead we swallow the close and just drop
// uiFocus, so the window hides and Ship comes back (the user's instinctive "close" =
// "return to Ship", reliable since a click on the X never depends on keyboard focus).
LRESULT CALLBACK FleetGuestWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_CLOSE) {
        FleetShipCombo_SetUiFocus(0);
        return 0;
    }
    if (msg == WM_SYSCOMMAND && (wParam & 0xFFF0) == SC_CLOSE) {
        FleetShipCombo_SetUiFocus(0);
        return 0;
    }
    return CallWindowProc(sOrigWndProc, hwnd, msg, wParam, lParam);
}

// Install the close-interceptor on 2ship's game window exactly once. Everything except
// WM_CLOSE/SC_CLOSE is forwarded to SDL's original proc, so input/resize still work.
void SubclassGuestWindowOnce(HWND hwnd) {
    if (hwnd == nullptr || sSubclassedHwnd == hwnd) {
        return;
    }
    sOrigWndProc = reinterpret_cast<WNDPROC>(
        SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(FleetGuestWndProc)));
    sSubclassedHwnd = hwnd;
}

// Forcibly drop 2ship's hidden window from the Windows taskbar. WS_EX_TOOLWINDOW alone can
// miss when the shell creates the taskbar button slightly AFTER we apply the style (and our
// once-guard then never re-flushes it), so we also call ITaskbarList::DeleteTab for a short
// burst of frames after boot / after returning from the config view.
int sTaskbarKillFrames = 300;
void RemoveFromTaskbar(HWND hwnd) {
    static ITaskbarList* tbl = nullptr;
    static bool tried = false;
    if (!tried) {
        tried = true;
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED); // harmless if COM is already initialized
        if (FAILED(CoCreateInstance(__uuidof(TaskbarList), nullptr, CLSCTX_INPROC_SERVER,
                                    __uuidof(ITaskbarList), reinterpret_cast<void**>(&tbl))) ||
            tbl == nullptr || FAILED(tbl->HrInit())) {
            if (tbl) {
                tbl->Release();
                tbl = nullptr;
            }
        }
    }
    if (tbl) {
        tbl->DeleteTab(hwnd); // idempotent: harmless if there is no tab
    }
}

// Move EVERY top-level window of this process fully OFF-SCREEN (instead of SW_HIDE).
// A hidden/minimized window can stop rendering or report a 0 size, which would make
// GetGfxFrameBuffer() return 0 and freeze the captured image. An off-screen-but-shown
// window keeps rendering normally; the user just never sees it. Ship composites our image.
BOOL CALLBACK HideOwnWindowsProc(HWND hwnd, LPARAM /*lparam*/) {
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    // Skip our console window: it must stay hidden. Otherwise Show/Hide would drag the
    // terminal on-screen with the game window (only the game window should ever surface).
    if (pid == GetCurrentProcessId() && hwnd != GetConsoleWindow() && GetWindow(hwnd, GW_OWNER) == nullptr &&
        GetWindowTextLengthW(hwnd) > 0) {
        SubclassGuestWindowOnce(hwnd); // X / Alt+F4 -> return to Ship, never kill the session
        // Make it a tool window once (removes it from the taskbar AND alt-tab) so there's
        // no second entry. Apply the ex-style while off-screen so the required hide/show
        // doesn't flash on screen.
        LONG_PTR ex = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
        if (!(ex & WS_EX_TOOLWINDOW)) {
            ShowWindow(hwnd, SW_HIDE);
            SetWindowPos(hwnd, HWND_BOTTOM, -32000, -32000, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
            SetWindowLongPtrW(hwnd, GWL_EXSTYLE, (ex | WS_EX_TOOLWINDOW) & ~WS_EX_APPWINDOW);
            ShowWindow(hwnd, SW_SHOWNA); // shown (off-screen) so it keeps rendering, no activation
        }
        RECT r;
        if (GetWindowRect(hwnd, &r) && r.left > -20000) {
            SetWindowPos(hwnd, HWND_BOTTOM, -32000, -32000, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
        }
        // Belt-and-suspenders: explicitly remove the taskbar button for a short burst, in
        // case the shell created it after the tool-window style was applied.
        if (sTaskbarKillFrames > 0) {
            RemoveFromTaskbar(hwnd);
            sTaskbarKillFrames--;
        }
    }
    return TRUE; // keep enumerating
}

// Find Ship's (host) window rect by its PID, so we can overlay 2ship exactly on top of it
// (seamless: looks like the same window changed content rather than a new window popping up).
BOOL CALLBACK FindHostWindowProc(HWND hwnd, LPARAM /*lparam*/) {
    if (sHostPid == 0) {
        return FALSE;
    }
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == sHostPid && GetWindow(hwnd, GW_OWNER) == nullptr && GetWindowTextLengthW(hwnd) > 0 &&
        IsWindowVisible(hwnd)) {
        RECT r;
        if (GetWindowRect(hwnd, &r)) {
            sHostRect = r;
            sHaveHostRect = true;
            sHostHwnd = hwnd; // remember it so we can hand OS focus back to Ship on OoT switch
            return FALSE;      // found Ship's window
        }
    }
    return TRUE;
}

// Bring 2ship's window ON-SCREEN to the front (so its BenGui is usable for config),
// overlaying Ship's window exactly. The window stays a tool window the whole time so it
// never adds a taskbar/alt-tab entry (tool windows can still be shown on-screen and focused).
BOOL CALLBACK ShowOwnWindowsProc(HWND hwnd, LPARAM /*lparam*/) {
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    // Skip our console window: it must stay hidden. Otherwise Show/Hide would drag the
    // terminal on-screen with the game window (only the game window should ever surface).
    if (pid == GetCurrentProcessId() && hwnd != GetConsoleWindow() && GetWindow(hwnd, GW_OWNER) == nullptr &&
        GetWindowTextLengthW(hwnd) > 0) {
        SubclassGuestWindowOnce(hwnd); // X / Alt+F4 -> return to Ship, never kill the session
        // Keep it a permanent tool window (do NOT switch to WS_EX_APPWINDOW) so showing it for
        // config never spawns a taskbar button. Re-arm the cleanup for the next hide just in case.
        sTaskbarKillFrames = 300;
        if (sHaveHostRect) {
            // Overlay Ship's window exactly (seamless).
            SetWindowPos(hwnd, HWND_TOP, sHostRect.left, sHostRect.top, sHostRect.right - sHostRect.left,
                         sHostRect.bottom - sHostRect.top, SWP_NOACTIVATE);
        } else {
            RECT r;
            if (GetWindowRect(hwnd, &r) && r.left < -10000) {
                SetWindowPos(hwnd, HWND_TOP, 80, 80, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
            }
        }
        ShowWindow(hwnd, SW_SHOW);
        // Reliable cross-process foreground: attach to the current foreground thread's input
        // queue to bypass Windows' focus-stealing prevention, so the keyboard reaches 2ship's
        // BenGui without the user clicking the window first.
        HWND fg = GetForegroundWindow();
        DWORD fgTid = (fg != nullptr) ? GetWindowThreadProcessId(fg, nullptr) : 0;
        DWORD myTid = GetCurrentThreadId();
        bool attached = (fgTid != 0 && fgTid != myTid && AttachThreadInput(myTid, fgTid, TRUE));
        // Briefly mark topmost to win the z-order, then drop back to a normal window.
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        BringWindowToTop(hwnd);
        SetForegroundWindow(hwnd);
        SetActiveWindow(hwnd);
        SetFocus(hwnd);
        if (attached) {
            AttachThreadInput(myTid, fgTid, FALSE);
        }
    }
    return TRUE;
}

int sLastUiShown = -1; // -1 unknown, 0 hidden, 1 shown-for-config

// Frames to skip capturing after the window is shown or hidden.
//
// Showing/hiding/moving the window makes the swapchain resize, which DESTROYS AND RECREATES the
// framebuffer textures. The capture below takes the framebuffer handle from the renderer and
// reinterpret_casts it to an ID3D11ShaderResourceView, then calls a method on it — so if it grabs
// the handle in the same frame the window changed, it can dereference a view the resize already
// released. That is a raw access violation: no exception, no C++ throw, nothing in the log.
//
// It is exactly the 2026-08-06 tester trace: "8. PollHostAlive enter", "9. done — host still
// alive", "10. ProducerPublishFrame enter", and then the log ends. The first frame after a flip is
// the ONE frame where this window goes from visible to hidden, which is why the swap looked guilty
// for so long — the swap was only what made the window change.
//
// Two frames of margin: the resize is processed by the message pump between frames, so one is
// enough in theory and two costs nothing (the peer just keeps showing the previous frame).
int sSkipCaptureFrames = 0;
} // namespace
#endif

void FleetShipCombo_HideGuestWindow(void) {
#ifdef _WIN32
    EnumWindows(HideOwnWindowsProc, 0);
#endif
}

void FleetShipCombo_WatchHost(unsigned long hostPid) {
#ifdef _WIN32
    sHostPid = static_cast<DWORD>(hostPid);
    if (hostPid != 0 && sHostProcess == nullptr) {
        sHostProcess = OpenProcess(SYNCHRONIZE, FALSE, static_cast<DWORD>(hostPid));
    }
#else
    sPosixHostPid = hostPid;
#endif
}

void FleetShipCombo_UpdateGuestWindow(void) {
#ifdef _WIN32
    // The ACTIVE game must own OS keyboard focus, because ESC / typing / a game's own BenGui only
    // reach the process whose window is focused. So bring 2ship's real window to the front (focused,
    // overlaying Ship's window exactly) whenever MM (this process) is the ACTIVE game. We ALSO honor
    // the config-peek path (uiFocus==1, the "2 Ship 2 Harkinian" tab) so the user can reach 2ship's
    // BenGui while OoT is still the active game.
    const bool mmActive = FleetShipCombo_IsThisGameActive();
    // uiFocus is the single "front window" flag (1 = MM window front, 0 = OoT/Ship front). It tracks
    // the active game on every flip (set at the flip source), so normal MM play keeps MM front — but
    // the "Ship of Harkinian" tab can set uiFocus=0 to peek OoT's menu EVEN while MM is the active
    // game, and then MM parks so Ship's window (SohMenu) surfaces. So show MM's real window IFF the UI
    // wants MM front; do NOT force it just because MM is active (that was the one-directional bug).
    const bool uiWantsMm = (FleetShipCombo_GetUiFocus() == 1);
    const bool configPeek = uiWantsMm && !mmActive; // peeking MM's BenGui while OoT is the active game
    const int wantShown = uiWantsMm ? 1 : 0;
    if (wantShown) {
        if (sLastUiShown != 1) {
            sHaveHostRect = false;
            EnumWindows(FindHostWindowProc, 0); // find Ship's rect so we overlay it exactly
            EnumWindows(ShowOwnWindowsProc, 0); // bring on-screen + front once (focus -> keyboard/ESC)
            // Config-peek (reaching MM's BenGui WHILE playing OoT) lands the user directly in the
            // menu. Switching to PLAY MM does NOT force the menu open — 2ship now has focus, so ESC
            // opens BenGui on demand and the keyboard drives MM.
            if (configPeek && !mmActive) {
                if (auto window = Ship::Context::GetRawInstance()->GetWindow()) {
                    if (auto gui = window->GetGui()) {
                        if (auto menu = gui->GetMenu()) {
                            if (!menu->IsVisible()) {
                                menu->ToggleVisibility();
                            }
                        }
                    }
                }
            }
            sLastUiShown = 1;
            sSkipCaptureFrames = 2; // the swapchain just resized — see sSkipCaptureFrames
        } else if (configPeek && !mmActive) {
            // Config-peek ONLY: the user came here just to use 2ship's BenGui while OoT is active;
            // once they close it (ESC / X), hand focus back to Ship. When MM is the ACTIVE game we
            // KEEP focus here (they're playing MM) — closing BenGui just closes the menu.
            bool hasMenu = false, menuOpen = false;
            if (auto window = Ship::Context::GetRawInstance()->GetWindow()) {
                if (auto gui = window->GetGui()) {
                    if (auto menu = gui->GetMenu()) {
                        hasMenu = true;
                        menuOpen = menu->IsVisible();
                    }
                }
            }
            if (hasMenu && !menuOpen) {
                FleetShipCombo_SetUiFocus(0); // 2ship window hides on the next frame -> back to Ship
            }
        }
    } else {
        // Transition to hidden (OoT just became active, or the config-peek ended): moving our
        // window off-screen does NOT transfer OS foreground, so keyboard/ESC would go nowhere.
        // We currently HOLD the foreground, so we can legitimately hand it to Ship (a background
        // process foregrounding itself would be blocked by focus-stealing prevention — doing it
        // from here, the current foreground holder, is allowed).
        if (sLastUiShown == 1) {
            sHostHwnd = nullptr;
            EnumWindows(FindHostWindowProc, 0);
            if (sHostHwnd != nullptr) {
                BringWindowToTop(sHostHwnd);
                SetForegroundWindow(sHostHwnd);
                SetActiveWindow(sHostHwnd);
            }
        }
        if (sLastUiShown != 0) {
            sSkipCaptureFrames = 2; // the swapchain just resized — see sSkipCaptureFrames
        }
        EnumWindows(HideOwnWindowsProc, 0); // keep off-screen / tool window (idempotent)
        sLastUiShown = 0;
    }
#endif
}

void FleetShipCombo_PollHostAlive(void) {
    // Tell Ship we are still turning frames. This is the ONE place guaranteed to run every frame
    // whatever MM is doing (the frame loop calls it outside the render gating), which is exactly
    // what makes it a usable liveness signal: an inactive MM renders nothing, so "did the shared
    // texture update?" cannot distinguish idle from hung. A stopped heartbeat can.
    FleetShipCombo_BeatHeartbeat();
#ifdef _WIN32
    // If the host (Ship) has exited, don't linger as an orphan process. No-op when not a hosted
    // child (sHostProcess stays null in standalone) or while the host is still alive.
    //
    // SAY SO FIRST. This exit(0) is why "2ship crashes" reports have no crash in them: the process
    // ends normally, with no exception, no C++ throw and not one line in the log — it just stops
    // mid-frame, in whatever state it happened to be (idle on the file select, in gameplay, during a
    // flip), which is exactly the pattern the 2026-08-05 logs show. It is not a crash here at all;
    // it is US shutting down because SHIP died.
    //
    // The host's exit code is the payoff: a clean close reads 0, while a crash reads its exception
    // code (0xC0000005 for an access violation, 0xC0000409 for a stack check, ...). So this single
    // line says both "2ship stopped on purpose" and "here is how the host went down" — which moves
    // the investigation to the process that actually failed.
    if (sHostProcess != nullptr && WaitForSingleObject(sHostProcess, 0) == WAIT_OBJECT_0) {
        DWORD hostExit = 0;
        GetExitCodeProcess(sHostProcess, &hostExit);
        SPDLOG_ERROR("[FleetExit] host (Ship) is GONE — exit code {:#010x} ({}). 2ship is closing itself so it "
                     "does not orphan; this is NOT a 2ship crash.",
                     (uint32_t)hostExit, hostExit == 0 ? "clean shutdown" : "abnormal — that is the crash to chase");
        spdlog::default_logger()->flush();
        exit(0);
    }
#else
    // POSIX: a 0-signal probe reports whether the host PID still exists. ESRCH = it's gone, so
    // exit so we don't orphan. EPERM (alive but not ours) and success both mean alive. No-op when
    // not a hosted child (sPosixHostPid == 0). Runs every frame, so host death is caught quickly.
    if (sPosixHostPid != 0 && kill(static_cast<pid_t>(sPosixHostPid), 0) != 0 && errno == ESRCH) {
        exit(0);
    }
#endif
}

void FleetShipCombo_ProducerPublishFrame(void) {
#ifdef _WIN32
    // Host-death watchdog. Also polled unconditionally from the frame loop (Graph_ProcessGfxCommands)
    // so it still runs when this function is gated out for the inactive game.
    FleetShipCombo_PollHostAlive();

    // Only when in combo (shared region exists).
    if (FleetShipCombo_GetActiveGame() < 0) {
        return;
    }

    // Single-window combo: park 2ship off-screen, or bring it to front when the user
    // selects "2Ship" in Ship's View selector (uiFocus) to reach its BenGui.
    FleetShipCombo_UpdateGuestWindow();

    auto window = Ship::Context::GetRawInstance()->GetWindow();
    if (!window) {
        return;
    }

    // Frame sharing is DirectX 11 only: below, GetGfxFrameBuffer() is reinterpret_cast to an
    // ID3D11ShaderResourceView and dereferenced. Under OpenGL that same call returns a GL texture
    // NAME — a small integer like 3 — so the cast produces a bogus pointer and the first method
    // call is an access violation. The non-zero check further down does not catch it, because a
    // texture name is perfectly non-zero.
    //
    // Bail out instead of crashing: the combo simply stops sharing frames until the user switches
    // back to DirectX. Warned once so the reason is visible in the log rather than silent.
    if (auto fast3d = std::dynamic_pointer_cast<Fast::Fast3dWindow>(window)) {
        if (fast3d->GetWindowBackend() != Fast::WindowBackend::FAST3D_DXGI_DX11) {
            // Warn once per renderer change rather than once per process: if the player switches
            // back to DirectX and away again, they should be told again.
            static int32_t lastWarnedBackend = -1;
            const int32_t backend = fast3d->GetWindowBackend();
            if (backend != lastWarnedBackend) {
                lastWarnedBackend = backend;
                SPDLOG_WARN("[FleetShipCombo] Frame sharing needs the DirectX 11 renderer; the "
                            "current backend is {}. Frames will not be shared until it is changed "
                            "back (Settings -> Renderer API).",
                            fast3d->GetWindowBackendName());
                Notification::Emit({
                    .prefix = "Combo",
                    .message = "needs the DirectX 11 renderer - the combo image will not show on " +
                               fast3d->GetWindowBackendName(),
                    .suffix = "change it in Settings > Renderer API",
                    .remainingTime = 15.0f,
                });
            }
            return;
        }
    } else {
        return; // not a Fast3D window at all — nothing to capture
    }

    // Force render-to-fb EVERY frame so GetGfxFrameBuffer() stays non-zero (it is only
    // non-zero when internal resolution != 1.0x). Forcing once isn't enough: if anything
    // resets the multiplier to 1.0x, capture would silently stop and the image would
    // freeze. Slightly above 1.0 keeps it ~native. Configurable via gFleetShipCombo.RenderScale.
    {
        float scale = CVarGetFloat("gFleetShipCombo.RenderScale", 1.0f);
        if (scale > 0.99f && scale < 1.01f) {
            scale = 1.01f; // exactly 1.0x renders straight to the swapchain (no grabbable texture)
        }
        window->SetResolutionMultiplier(scale);
    }

    // Don't touch the framebuffer on a frame where the window just changed: the swapchain resize
    // that follows a show/hide releases the very view we are about to dereference below, and the
    // handle we would read is a plain uintptr_t with no way to tell a live view from a dead one.
    // See sSkipCaptureFrames — this is the 2026-08-06 crash ("10. ProducerPublishFrame enter" and
    // then nothing), which happens on the first frame after a flip because that is when this
    // window goes from visible to hidden.
    if (sSkipCaptureFrames > 0) {
        sSkipCaptureFrames--;
        return; // the peer keeps showing the previous frame for one or two frames
    }

    uintptr_t fb = window->GetGfxFrameBuffer();
    if (fb == 0) {
        return; // not rendering to an fb yet; try again next frame
    }

    ID3D11ShaderResourceView* srv = reinterpret_cast<ID3D11ShaderResourceView*>(fb);
    ID3D11Resource* res = nullptr;
    srv->GetResource(&res); // borrowed SRV; do NOT release srv
    if (!res) {
        return;
    }

    ID3D11Texture2D* gameTex = nullptr;
    res->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&gameTex));
    res->Release();
    if (!gameTex) {
        return;
    }

    D3D11_TEXTURE2D_DESC desc;
    gameTex->GetDesc(&desc);

    ID3D11Device* dev = nullptr;
    gameTex->GetDevice(&dev);
    ID3D11DeviceContext* ctx = nullptr;
    if (dev) {
        dev->GetImmediateContext(&ctx);
    }

    if (dev && ctx) {
        // (Re)create the shared texture when size/format changes.
        if (!sSharedTex || sSharedW != desc.Width || sSharedH != desc.Height ||
            sSharedFmt != static_cast<uint32_t>(desc.Format)) {
            ReleaseSharedTex();

            D3D11_TEXTURE2D_DESC sd = {};
            sd.Width = desc.Width;
            sd.Height = desc.Height;
            sd.MipLevels = 1;
            sd.ArraySize = 1;
            sd.Format = desc.Format;
            sd.SampleDesc.Count = 1;
            sd.SampleDesc.Quality = 0;
            sd.Usage = D3D11_USAGE_DEFAULT;
            sd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            // Keyed mutex = cross-process shareable + GPU sync so the consumer never reads
            // a half-written / stale frame (the cause of a "stuck" image without sync).
            sd.MiscFlags = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;

            if (SUCCEEDED(dev->CreateTexture2D(&sd, nullptr, &sSharedTex)) && sSharedTex) {
                IDXGIResource* dxgiRes = nullptr;
                if (SUCCEEDED(sSharedTex->QueryInterface(__uuidof(IDXGIResource),
                                                         reinterpret_cast<void**>(&dxgiRes))) &&
                    dxgiRes) {
                    dxgiRes->GetSharedHandle(&sSharedHandle);
                    dxgiRes->Release();
                }
                sSharedTex->QueryInterface(__uuidof(IDXGIKeyedMutex), reinterpret_cast<void**>(&sSharedMutex));
                sSharedW = desc.Width;
                sSharedH = desc.Height;
                sSharedFmt = static_cast<uint32_t>(desc.Format);
                SPDLOG_INFO("[FleetShipCombo] Producer shared texture {}x{} fmt {} handle {}", sSharedW, sSharedH,
                            sSharedFmt, (void*)sSharedHandle);
            } else {
                SPDLOG_ERROR("[FleetShipCombo] Producer failed to create shared texture");
            }
        }

        if (sSharedTex && sSharedHandle && sSharedMutex) {
            // Key 0 for both sides; short wait, skip this frame if the consumer holds it.
            if (sSharedMutex->AcquireSync(0, 8) == S_OK) {
                ctx->CopyResource(sSharedTex, gameTex);
                ctx->Flush();
                sSharedMutex->ReleaseSync(0);
                sFrameIndex++;
                FleetShipCombo_PublishSharedTexture(
                    static_cast<unsigned long long>(reinterpret_cast<uintptr_t>(sSharedHandle)), sSharedW, sSharedH,
                    sSharedFmt, sFrameIndex);
            }
        }

        // UI-only overlay (trackers etc.) so Ship can show 2ship's UI over EITHER game.
        if (CVarGetInteger("gFleetShipCombo.UiOverlay", 1)) {
            PublishUiOverlay(dev, ctx);
        }
    }

    if (ctx) {
        ctx->Release();
    }
    if (dev) {
        dev->Release();
    }
    gameTex->Release();
#endif // _WIN32
}
