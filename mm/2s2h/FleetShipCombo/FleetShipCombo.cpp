#include "FleetShipCombo.h"

#include <libultraship/libultraship.h>
#include <spdlog/spdlog.h>

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif
#endif

namespace {

// Candidate Ship executable file names, in priority order, per platform.
// Ship's CMake target is "soh" (-> soh.exe on Windows, soh.elf on Linux, soh on macOS).
const std::vector<std::string>& ShipExeNames() {
#ifdef _WIN32
    static const std::vector<std::string> names = { "soh.exe" };
#elif defined(__APPLE__)
    static const std::vector<std::string> names = { "soh", "soh-macos" };
#else
    static const std::vector<std::string> names = { "soh.elf", "soh" };
#endif
    return names;
}

// Directory containing the currently running 2ship executable.
std::filesystem::path SelfExeDir() {
#ifdef _WIN32
    wchar_t buf[MAX_PATH];
    DWORD len = GetModuleFileNameW(nullptr, buf, MAX_PATH);
    if (len == 0 || len == MAX_PATH) {
        return {};
    }
    return std::filesystem::path(std::wstring(buf, len)).parent_path();
#elif defined(__APPLE__)
    char buf[4096];
    uint32_t size = sizeof(buf);
    if (_NSGetExecutablePath(buf, &size) != 0) {
        return {};
    }
    return std::filesystem::weakly_canonical(std::filesystem::path(buf)).parent_path();
#else
    std::error_code ec;
    auto p = std::filesystem::read_symlink("/proc/self/exe", ec);
    if (ec) {
        return {};
    }
    return p.parent_path();
#endif
}

// 2ship lives at Ship/2ship/, so Ship's exe sits one level up (the "top level").
// Search the parent dir, then the parent's parent as a fallback.
std::filesystem::path LocateShipExe() {
    std::filesystem::path selfDir = SelfExeDir();
    if (selfDir.empty()) {
        return {};
    }

    std::vector<std::filesystem::path> searchDirs = { selfDir.parent_path() };
    if (!selfDir.parent_path().empty()) {
        searchDirs.push_back(selfDir.parent_path().parent_path());
    }

    std::error_code ec;
    for (const auto& dir : searchDirs) {
        if (dir.empty()) {
            continue;
        }
        for (const auto& name : ShipExeNames()) {
            std::filesystem::path candidate = dir / name;
            if (std::filesystem::exists(candidate, ec)) {
                return candidate;
            }
        }
    }
    return {};
}

bool HasArg(int argc, char** argv, const char* flag) {
    for (int i = 1; i < argc; ++i) {
        if (argv[i] != nullptr && std::string(argv[i]) == flag) {
            return true;
        }
    }
    return false;
}

// Launch Ship as the host, asking it to boot straight into MM. Returns true on
// success (caller should exit this 2ship process).
bool LaunchShip(const std::filesystem::path& shipExe) {
#ifdef _WIN32
    std::wstring cmd = L"\"" + shipExe.wstring() + L"\" --boot=mm";
    std::vector<wchar_t> mutableCmd(cmd.begin(), cmd.end());
    mutableCmd.push_back(L'\0');

    std::wstring workDir = shipExe.parent_path().wstring();

    STARTUPINFOW si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};

    BOOL ok = CreateProcessW(shipExe.wstring().c_str(), mutableCmd.data(), nullptr, nullptr, FALSE, 0, nullptr,
                             workDir.empty() ? nullptr : workDir.c_str(), &si, &pi);
    if (!ok) {
        SPDLOG_ERROR("[FleetShipCombo] CreateProcessW failed for '{}' (error {})", shipExe.string(), GetLastError());
        return false;
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;
#else
    // execv replaces this process with Ship, which cleanly "closes" 2ship.
    std::string exe = shipExe.string();
    char* args[] = { exe.data(), const_cast<char*>("--boot=mm"), nullptr };
    execv(exe.c_str(), args);
    // Only reached if execv failed.
    SPDLOG_ERROR("[FleetShipCombo] execv failed for '{}'", exe);
    return false;
#endif
}

// ===================== Shared-memory coordination (Frente B) =====================

// THIS_GAME identity for this build: 1 = Majora's Mask (2ship).
constexpr int kThisGame = 1;
constexpr uint32_t kFscMagic = 0x46534331u;   // 'FSC1'
constexpr uint32_t kFscVersion = 1u;

// Layout shared between Ship and 2ship. MUST stay byte-identical to the Ship copy.
struct FscShared {
    uint32_t magic;
    uint32_t version;
    int32_t activeGame; // 0 = Ocarina of Time (Ship), 1 = Majora's Mask (2ship)
    // Picture-in-picture: 2ship publishes its game image as a D3D11 shared texture.
    uint64_t texHandle;     // OS shared handle from IDXGIResource::GetSharedHandle (0 = none)
    uint32_t texWidth;
    uint32_t texHeight;
    uint32_t texFormat;     // DXGI_FORMAT value
    uint32_t texFrameIndex; // bumped each publish (lets the consumer detect new frames)
    int32_t uiFocus;        // which window is front for CONFIG: 0 = Ship, 1 = 2ship
    // Cross-game loading-zone WARP request. The trigger side writes the target (in the TARGET
    // game's id/space), bumps warpSeq, and flips activeGame; whichever game BECOMES active applies
    // it once per new seq (load scene + override Link pos/rot). MUST stay byte-identical to Ship.
    int32_t warpSeq;        // bumped per request (0 = none yet)
    int32_t warpScene;      // target scene id in the target game's space
    float warpX;            // land position override (target game world coords)
    float warpY;
    float warpZ;
    int32_t warpRotY;        // land Y rotation (s16 binary angle stored in int32)
    int32_t warpSaveFileNum; // save SLOT the warp came from; the target game loads its own same slot (-1 = unset)
    int32_t sendFadeAlpha;   // 0..255 sending-fade overlay, written by the ACTIVE (sending) game, drawn by the host consumer
    int32_t doorDLIndex;     // DEV: which Lost Woods room-DL the MM door tunnel tool is showing (for the on-screen readout)
    uint64_t reservedU[12];
};

#ifdef _WIN32
std::wstring sShmName = L"Local\\FleetShipComboShared";
HANDLE sShmHandle = nullptr;
#else
std::string sShmName = "/FleetShipComboShared";
int sShmFd = -1;
#endif
FscShared* sShared = nullptr;
bool sLazyOpenTried = false;

// Make the shared-memory region name UNIQUE per combo instance, keyed by the host Ship's PID,
// so SEVERAL combos can run on one machine without colliding on a single region. The Ship host
// keys it by its own PID and launches 2ship with --fleet-host-pid=<that PID>, so the child
// derives the SAME name and the pair is matched. Key 0 keeps the legacy unsuffixed name.
void SetInstanceKey(unsigned long key) {
    if (key == 0) {
        return;
    }
#ifdef _WIN32
    sShmName = L"Local\\FleetShipComboShared_" + std::to_wstring(key);
#else
    sShmName = "/FleetShipComboShared_" + std::to_string(key);
#endif
}

void InitFreshRegion() {
    if (!sShared) {
        return;
    }
    sShared->magic = kFscMagic;
    sShared->version = kFscVersion;
    sShared->activeGame = 0; // default to OoT until host/child sets it
    sShared->texHandle = 0;
    sShared->texWidth = 0;
    sShared->texHeight = 0;
    sShared->texFormat = 0;
    sShared->texFrameIndex = 0;
    sShared->uiFocus = 0;
    sShared->warpSeq = 0;
    sShared->warpScene = 0;
    sShared->warpX = sShared->warpY = sShared->warpZ = 0.0f;
    sShared->warpRotY = 0;
    sShared->warpSaveFileNum = -1;
    sShared->sendFadeAlpha = 0;
    sShared->doorDLIndex = 0;
    for (int i = 0; i < 12; ++i) {
        sShared->reservedU[i] = 0;
    }
}

// create=true: create-or-open (FleetShipCombo_SharedInit). create=false: open an
// EXISTING region only (lazy), so standalone play never allocates one.
FscShared* MapShared(bool create) {
    if (sShared) {
        return sShared;
    }
#ifdef _WIN32
    if (create) {
        sShmHandle = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(FscShared), sShmName.c_str());
        bool existed = (sShmHandle != nullptr && GetLastError() == ERROR_ALREADY_EXISTS);
        if (sShmHandle != nullptr) {
            sShared = (FscShared*)MapViewOfFile(sShmHandle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(FscShared));
            if (sShared != nullptr && !existed) {
                InitFreshRegion();
            }
        }
    } else {
        sShmHandle = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, sShmName.c_str());
        if (sShmHandle != nullptr) {
            sShared = (FscShared*)MapViewOfFile(sShmHandle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(FscShared));
        }
    }
#else
    int flags = create ? (O_CREAT | O_RDWR) : O_RDWR;
    sShmFd = shm_open(sShmName.c_str(), flags, 0666);
    if (sShmFd >= 0) {
        if (create) {
            (void)ftruncate(sShmFd, sizeof(FscShared));
        }
        void* p = mmap(nullptr, sizeof(FscShared), PROT_READ | PROT_WRITE, MAP_SHARED, sShmFd, 0);
        if (p != MAP_FAILED) {
            sShared = (FscShared*)p;
            if (create && sShared->magic != kFscMagic) {
                InitFreshRegion();
            }
        }
    }
#endif
    return sShared;
}

// Attach to an existing region once (used by the per-frame freeze check). If none
// exists (standalone), leaves sShared null so IsThisGameActive() returns true.
FscShared* LazyOpen() {
    if (sShared) {
        return sShared;
    }
    if (sLazyOpenTried) {
        return nullptr;
    }
    sLazyOpenTried = true;
    return MapShared(false);
}

// Copy an .o2r from whichever combo dir has it into the one that doesn't (best-effort, never throws).
void MirrorO2r(const std::filesystem::path& dirA, const std::filesystem::path& dirB, const char* name) {
    std::error_code ec;
    std::filesystem::path a = dirA / name;
    std::filesystem::path b = dirB / name;
    bool ae = std::filesystem::exists(a, ec);
    bool be = std::filesystem::exists(b, ec);
    if (ae && !be) {
        std::filesystem::copy_file(a, b, std::filesystem::copy_options::overwrite_existing, ec);
    } else if (be && !ae) {
        std::filesystem::copy_file(b, a, std::filesystem::copy_options::overwrite_existing, ec);
    }
}

} // namespace

void FleetShipCombo_ProvisionO2rBothDirs(void) {
    // Combo layout: <root>/soh.exe + <root>/2ship/2ship.exe. Both mm.o2r and oot.o2r should sit next to
    // BOTH exes, so mirror whichever exists into the dir missing it — an extraction done by EITHER game
    // (soh -> oot.o2r, 2ship -> mm.o2r) then provisions both. No-op when there's no sibling (standalone).
    std::filesystem::path selfDir = SelfExeDir();
    std::filesystem::path shipExe = LocateShipExe();
    if (selfDir.empty() || shipExe.empty()) {
        return;
    }
    std::filesystem::path rootDir = shipExe.parent_path();
    if (rootDir.empty() || rootDir == selfDir) {
        return;
    }
    MirrorO2r(selfDir, rootDir, "mm.o2r");
    MirrorO2r(selfDir, rootDir, "oot.o2r");
}

bool FleetShipCombo_BootstrapMaybeRelaunch(int argc, char** argv) {
    // Ship launches us with --fleet-child; never bounce again or we'd loop forever.
    if (HasArg(argc, argv, "--fleet-child")) {
        SPDLOG_INFO("[FleetShipCombo] Hosted by Ship (--fleet-child); continuing as 2ship child.");
#ifdef _WIN32
        // We run hidden behind Ship's single window, so our own console (created by
        // main.c's AllocConsole) must never show as a stray terminal. main.c only hides
        // it in Release builds (#ifndef _DEBUG); hide it unconditionally for the child so
        // a Debug build (or any later re-show) can't leak a terminal next to Ship.
        if (HWND con = GetConsoleWindow()) {
            ShowWindow(con, SW_HIDE);
        }
#endif
        // The host PID (from --fleet-host-pid) keys BOTH the per-instance shared-memory region
        // name (so multiple combos on one PC don't collide) AND the host-death watchdog, so
        // parse it first, before attaching to the region.
        unsigned long hostPid = 0;
        const std::string pidPrefix = "--fleet-host-pid=";
        for (int i = 1; i < argc; ++i) {
            std::string a = argv[i] ? argv[i] : "";
            if (a.rfind(pidPrefix, 0) == 0) {
                hostPid = strtoul(a.c_str() + pidPrefix.size(), nullptr, 10);
            }
        }
        // Attach to THIS host's shared region so the freeze / active-game stays in sync.
        FleetShipCombo_SharedInit(hostPid);
        // Watch the host PID so we exit when Ship closes (no orphan process).
        if (hostPid != 0) {
            FleetShipCombo_WatchHost(hostPid);
        }
        return false;
    }

    if (!CVarGetInteger("isFleetShipCombo.Enabled", 0)) {
        return false;
    }

    std::filesystem::path shipExe = LocateShipExe();
    if (shipExe.empty()) {
        // Graceful fallback: Ship isn't where we expect, so disable the combo and
        // boot 2ship standalone instead of trapping the user.
        SPDLOG_WARN("[FleetShipCombo] Ship executable not found at top level; disabling combo and booting standalone.");
        CVarSetInteger("isFleetShipCombo.Enabled", 0);
        Ship::Context::GetInstance()->GetConsoleVariables()->Save();
        return false;
    }

    // Only bounce to Ship if mm.o2r is OBTAINABLE (present next to 2ship OR next to Ship — the combo
    // mirrors it into both). If it's missing EVERYWHERE, DON'T bounce: stay in 2ship so its extractor
    // (RunExtract, later in boot) can create mm.o2r. Bouncing first exits before the extractor ever runs,
    // so mm.o2r could never be generated by launching 2ship.exe directly.
    {
        std::error_code ec;
        std::filesystem::path selfDir = SelfExeDir();
        bool here = !selfDir.empty() && std::filesystem::exists(selfDir / "mm.o2r", ec);
        bool atShip = std::filesystem::exists(shipExe.parent_path() / "mm.o2r", ec);
        if (!here && !atShip) {
            SPDLOG_INFO("[FleetShipCombo] mm.o2r missing everywhere; staying in 2ship to run the extractor (no bounce).");
            return false;
        }
    }

    // Opening 2ship directly means the player wants MM: remember that so the host
    // (and a future plain Ship launch) resumes into 2ship.
    CVarSetInteger("isPlayerIn2Ship", 1);
    Ship::Context::GetInstance()->GetConsoleVariables()->Save();

    SPDLOG_INFO("[FleetShipCombo] Combo enabled; handing off to host Ship at '{}' (--boot=mm).", shipExe.string());
    if (LaunchShip(shipExe)) {
        return true; // caller should exit; Ship is now the host.
    }

    // Launch failed: fall back to standalone rather than leaving the user with nothing.
    SPDLOG_ERROR("[FleetShipCombo] Failed to launch Ship; booting 2ship standalone.");
    return false;
}

void FleetShipCombo_SharedInit(unsigned long instanceKey) {
    SetInstanceKey(instanceKey);
    MapShared(true);
}

int FleetShipCombo_GetActiveGame(void) {
    FscShared* s = LazyOpen();
    return s ? s->activeGame : -1;
}

void FleetShipCombo_SetActiveGame(int game) {
    FscShared* s = LazyOpen();
    if (s) {
        s->activeGame = game;
    }
}

// Per-process seq of the last warp we issued/consumed, so the REQUESTER never re-consumes its own.
static int sLastWarpSeq = 0;

void FleetShipCombo_RequestWarp(int targetGame, int scene, float x, float y, float z, int rotY, int saveFile) {
    FscShared* s = LazyOpen();
    if (!s) {
        return;
    }
    s->warpScene = scene;
    s->warpX = x;
    s->warpY = y;
    s->warpZ = z;
    s->warpRotY = rotY;
    s->warpSaveFileNum = saveFile; // tell the target game which save slot to be in (set before the seq bump)
    s->warpSeq += 1;            // mark a new request
    sLastWarpSeq = s->warpSeq;  // we issued it; don't let THIS process consume its own warp
    s->activeGame = targetGame; // flip: the target game becomes active (unfrozen) and applies it
    s->uiFocus = targetGame;    // front window follows the active game (0=OoT, 1=MM); a peek tab may
                                // override it afterwards without touching activeGame
}

int FleetShipCombo_ConsumePendingWarp(int* scene, float* x, float* y, float* z, int* rotY) {
    FscShared* s = LazyOpen();
    if (!s) {
        return 0;
    }
    if (s->warpSeq == sLastWarpSeq) {
        return 0; // nothing new
    }
    if (s->activeGame != kThisGame) {
        return 0; // not addressed to this game yet
    }
    sLastWarpSeq = s->warpSeq;
    if (scene) {
        *scene = s->warpScene;
    }
    if (x) {
        *x = s->warpX;
    }
    if (y) {
        *y = s->warpY;
    }
    if (z) {
        *z = s->warpZ;
    }
    if (rotY) {
        *rotY = s->warpRotY;
    }
    return 1;
}

int FleetShipCombo_GetWarpSaveFile(void) {
    FscShared* s = LazyOpen();
    return s ? s->warpSaveFileNum : -1;
}

// Cross-game arrival blackout: while > 0, THIS game's render path paints the screen BLACK (empty DL)
// so the stale frame of the OTHER game and the warp scene-load are never shown during a flip. Set on
// warp arrival; the render path calls ...Active() exactly once per frame (it decrements).
static int sArrivalBlackout = 0;
void FleetShipCombo_BeginArrivalBlackout(int frames) {
    sArrivalBlackout = frames;
}
int FleetShipCombo_ArrivalBlackoutActive(void) {
    if (sArrivalBlackout > 0) {
        sArrivalBlackout--;
        return 1;
    }
    return 0;
}

// Sending-side fade overlay (0..255). The active game's warp logic ramps this up while Link keeps
// walking into the door (no scene transition -> no reload, not frozen); the host PiP consumer draws a
// black overlay at this alpha over the scene, giving a real fade-out, then we flip at full black.
void FleetShipCombo_SetSendFadeAlpha(int alpha) {
    FscShared* s = LazyOpen();
    if (!s) {
        return;
    }
    s->sendFadeAlpha = alpha < 0 ? 0 : (alpha > 255 ? 255 : alpha);
}
int FleetShipCombo_GetSendFadeAlpha(void) {
    FscShared* s = LazyOpen();
    return s ? s->sendFadeAlpha : 0;
}

void FleetShipCombo_SetDoorDLIndex(int index) {
    FscShared* s = LazyOpen();
    if (s) {
        s->doorDLIndex = index;
    }
}
int FleetShipCombo_GetDoorDLIndex(void) {
    FscShared* s = LazyOpen();
    return s ? s->doorDLIndex : 0;
}

bool FleetShipCombo_IsThisGameActive(void) {
    FscShared* s = LazyOpen();
    if (!s) {
        return true; // standalone / no combo: always active, never freeze
    }
    return s->activeGame == kThisGame;
}

int FleetShipCombo_GetUiFocus(void) {
    FscShared* s = LazyOpen();
    return s ? s->uiFocus : -1;
}

void FleetShipCombo_SetUiFocus(int focus) {
    FscShared* s = LazyOpen();
    if (s) {
        s->uiFocus = focus;
    }
}

// ---- FleetSync save-sync handshake over reservedU (layout unchanged — MUST match Ship) ----
// reservedU[0] is reserved (legacy bottles-sync plan). [1] = syncSaveSeq (bumped by the game that
// just SAVED), [2] = syncSaveAck (set by the OTHER game once it applied the shared overlay and
// wrote its own file), [3] = syncSaveSlot.
void FleetShipCombo_SignalSyncSave(int slot) {
    FscShared* s = LazyOpen();
    if (!s) {
        return;
    }
    s->reservedU[3] = (uint64_t)(uint32_t)slot;
    s->reservedU[1] = s->reservedU[1] + 1;
}

unsigned long long FleetShipCombo_GetSyncSaveSeq(void) {
    FscShared* s = LazyOpen();
    return s ? s->reservedU[1] : 0;
}

int FleetShipCombo_GetSyncSaveSlot(void) {
    FscShared* s = LazyOpen();
    return s ? (int)(uint32_t)s->reservedU[3] : -1;
}

void FleetShipCombo_AckSyncSave(unsigned long long seq) {
    FscShared* s = LazyOpen();
    if (s) {
        s->reservedU[2] = seq;
    }
}

unsigned long long FleetShipCombo_GetSyncSaveAck(void) {
    FscShared* s = LazyOpen();
    return s ? s->reservedU[2] : 0;
}

// ---- Fleet Oracle (combo randomizer) handshake over reservedU[4..5] (layout unchanged — MUST match Ship) ----
// [4] = oracleReqSeq (bumped by the HOST after writing fleet_oracle_req.json), [5] = oracleRespAck
// (set by the MM oracle to the req seq it answered, after writing fleet_oracle_resp.json).
void FleetShipCombo_SignalOracleRequest(void) {
    FscShared* s = LazyOpen();
    if (s) {
        s->reservedU[4] = s->reservedU[4] + 1;
    }
}

unsigned long long FleetShipCombo_GetOracleRequestSeq(void) {
    FscShared* s = LazyOpen();
    return s ? s->reservedU[4] : 0;
}

void FleetShipCombo_AckOracleResponse(unsigned long long seq) {
    FscShared* s = LazyOpen();
    if (s) {
        s->reservedU[5] = seq;
    }
}

unsigned long long FleetShipCombo_GetOracleResponseAck(void) {
    FscShared* s = LazyOpen();
    return s ? s->reservedU[5] : 0;
}

// ---- Shared-window open request over reservedU[6] (layout unchanged — MUST match Ship) ----
// 2ship's "Shared" tab can't open Ship's window directly; it bumps this counter and Ship's
// client pump opens the Fleet Shared window when it sees the change.
void FleetShipCombo_RequestSharedWindowOpen(void) {
    FscShared* s = LazyOpen();
    if (s) {
        s->reservedU[6] = s->reservedU[6] + 1;
    }
}

unsigned long long FleetShipCombo_GetSharedWindowOpenSeq(void) {
    FscShared* s = LazyOpen();
    return s ? s->reservedU[6] : 0;
}

// ---- Cross-game RESTART over reservedU[10] (layout unchanged — MUST match the other repo) ----
// A reset in one game bumps reservedU[10]; the other game's per-frame pump sees the new value and
// resets itself too. sRestartSelfSeq marks the value we ourselves bumped/observed so we never respond
// to our own reset (which would ping-pong forever).
static unsigned long long sRestartSelfSeq = 0;
static bool sRestartSeqInit = false;

void FleetShipCombo_SignalRestart(void) {
    FscShared* s = LazyOpen();
    if (s) {
        s->reservedU[10] = s->reservedU[10] + 1;
        sRestartSelfSeq = s->reservedU[10]; // our own bump; our pump must not respond to it
        sRestartSeqInit = true;
    }
}

int FleetShipCombo_ConsumeRestartRequest(void) {
    FscShared* s = LazyOpen();
    if (!s) {
        return 0;
    }
    if (!sRestartSeqInit) {
        sRestartSeqInit = true;
        sRestartSelfSeq = s->reservedU[10]; // don't fire on any pre-existing value at attach
        return 0;
    }
    if (s->reservedU[10] == sRestartSelfSeq) {
        return 0; // nothing new, or our own reset
    }
    sRestartSelfSeq = s->reservedU[10];
    return 1;
}

void FleetShipCombo_PublishSharedTexture(unsigned long long handle, unsigned int width, unsigned int height,
                                         unsigned int dxgiFormat, unsigned int frameIndex) {
    FscShared* s = LazyOpen();
    if (!s) {
        return;
    }
    s->texHandle = handle;
    s->texWidth = width;
    s->texHeight = height;
    s->texFormat = dxgiFormat;
    s->texFrameIndex = frameIndex;
}

int FleetShipCombo_GetSharedTexture(unsigned long long* handle, unsigned int* width, unsigned int* height,
                                    unsigned int* dxgiFormat, unsigned int* frameIndex) {
    FscShared* s = LazyOpen();
    if (!s) {
        return 0;
    }
    if (handle) {
        *handle = s->texHandle;
    }
    if (width) {
        *width = s->texWidth;
    }
    if (height) {
        *height = s->texHeight;
    }
    if (dxgiFormat) {
        *dxgiFormat = s->texFormat;
    }
    if (frameIndex) {
        *frameIndex = s->texFrameIndex;
    }
    return s->texHandle != 0 ? 1 : 0;
}

// ---- UI-overlay texture over reservedU[7..9] (layout unchanged — MUST match Ship) ----
// A SECOND shared texture with ONLY 2ship's ImGui windows (trackers etc.) over a transparent
// background, so Ship can draw MM's UI on top of whichever game is active (both trackers at once).
// [7] = D3D11 shared handle (0 = none), [8] = (width<<32)|height, [9] = (dxgiFormat<<32)|frameIndex.
void FleetShipCombo_PublishUiTexture(unsigned long long handle, unsigned int width, unsigned int height,
                                     unsigned int dxgiFormat, unsigned int frameIndex) {
    FscShared* s = LazyOpen();
    if (!s) {
        return;
    }
    s->reservedU[8] = ((unsigned long long)width << 32) | height;
    s->reservedU[9] = ((unsigned long long)dxgiFormat << 32) | frameIndex;
    s->reservedU[7] = handle; // handle last: the consumer keys re-opens off it
}

int FleetShipCombo_GetUiTexture(unsigned long long* handle, unsigned int* width, unsigned int* height,
                                unsigned int* dxgiFormat, unsigned int* frameIndex) {
    FscShared* s = LazyOpen();
    if (!s) {
        return 0;
    }
    if (handle) {
        *handle = s->reservedU[7];
    }
    if (width) {
        *width = (unsigned int)(s->reservedU[8] >> 32);
    }
    if (height) {
        *height = (unsigned int)(s->reservedU[8] & 0xFFFFFFFFu);
    }
    if (dxgiFormat) {
        *dxgiFormat = (unsigned int)(s->reservedU[9] >> 32);
    }
    if (frameIndex) {
        *frameIndex = (unsigned int)(s->reservedU[9] & 0xFFFFFFFFu);
    }
    return s->reservedU[7] != 0 ? 1 : 0;
}

// ---- Combo active SLOT over reservedU[11] (layout unchanged — MUST match the other repo) ----
// OoT publishes which save slot the current combo file is (0..2 stored as 1..3; 0 = unset) when a
// combo file is created/loaded, so MM auto-loads the SAME slot when it becomes active (its own
// per-process gFleetCombo.LastSlot may not match on a fresh combo).
void FleetShipCombo_SetComboSlot(int slot) {
    FscShared* s = LazyOpen();
    if (s && slot >= 0 && slot <= 2) {
        s->reservedU[11] = (uint64_t)(slot + 1);
    }
}

int FleetShipCombo_GetComboSlot(void) {
    FscShared* s = LazyOpen();
    if (!s || s->reservedU[11] == 0) {
        return -1;
    }
    return (int)s->reservedU[11] - 1;
}
