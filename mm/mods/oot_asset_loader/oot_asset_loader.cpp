/*
 * oot_asset_loader.cpp — thin wrappers to load OoT-unique resources by OTR path.
 *
 * The oot.o2r / oot-mq.o2r archives are added to the resource manager (at lowest
 * priority) by mm_asset_loader's LoadMmO2r, which shares its single archive reorder
 * + message-table refresh. These wrappers resolve a resource by name; once an OoT
 * archive is loaded, ResourceMgr_*ByName finds OoT-unique paths there.
 */
#include "oot_asset_loader.h"
#include "soh/ResourceManagerHelpers.h" // ResourceMgr_LoadGfxByName / LoadTexOrDListByName (2s2h BenPort)
#include <libultraship/bridge.h>        // CVarGetInteger (SoH-custom mesh kill-switch)
#include <cstring>
#include <cstdint>
#include <string>
#include <unordered_map>

// Explicit forward declarations of the two ResourceMgr_* entry points we use.
// BenPort.h only exposes these decls inside a `#ifndef __cplusplus` block, so a
// C++ TU like this one never sees them through the shim (hence C3861). We
// re-declare them here with the EXACT signatures BenPort.cpp defines. BenPort.cpp
// defines them as `extern "C"` (2s2h BenPort.cpp lines ~1555/1590), so these MUST
// use C linkage too — plain C++ linkage would compile but fail to LINK with an
// unresolved external. The `Gfx` union typedef is already in scope here: the shim
// include above pulls in 2s2h/BenPort.h, which includes the z64 headers (and thus
// PR/gbi.h's `typedef union { ... } Gfx;`). We deliberately do NOT forward-declare
// Gfx ourselves — `struct Gfx;` would clash with that union typedef.
extern "C" {
Gfx* ResourceMgr_LoadGfxByName(const char* path);
unsigned char ResourceMgr_FileExists(const char* resName);
char* ResourceMgr_LoadTexOrDListByName(const char* filePath);
}

// Defined in mm_asset_loader.cpp — true once an oot.o2r / oot-mq.o2r is loaded.
extern "C" unsigned char MmAssets_OotArchivesLoaded(void);
// Defined in mm_asset_loader.cpp — archive-scoped load straight off the stored
// Ship::Archive handles (bypasses the priority chain, so it defeats shadowing).
extern "C" void* MmAssets_LoadFromOotArchive(const char* path, size_t* outSize);
// Defined in mm_asset_loader.cpp — CRC64 resource hash -> path string (global hash index).
extern "C" const char* MmAssets_HashToPath(unsigned long long hash);

extern "C" void* OotAssets_LoadGfx(const char* otrPath) {
    if (otrPath == nullptr || *otrPath == '\0') {
        return nullptr;
    }
    return (void*)ResourceMgr_LoadGfxByName(otrPath);
}

extern "C" void* OotAssets_LoadTexOrDList(const char* otrPath) {
    if (otrPath == nullptr || *otrPath == '\0') {
        return nullptr;
    }
    return (void*)ResourceMgr_LoadTexOrDListByName(otrPath);
}

extern "C" unsigned char OotAssets_Available(void) {
    return MmAssets_OotArchivesLoaded();
}

// ---------------------------------------------------------------------------------------------
// SoH-CUSTOM mesh kill-switch. SoH's custom-exported resources (object_key/keyring/bosskey,
// object_jabbernut, object_nei_*, object_somaria, object_gi_fishing_pole, ...) embed
// G_SETTIMG_OTR_FILEPATH + PUSHCD commands; 2ship's gfx interpreter crashes on that format
// (observed: "Texture is null" spam then 0xC0000005 in gfx_push_current_dir — and libultraship
// is off-limits). Until the format is verified compatible, those DLs are NEVER executed: with the
// CVar off (default) every load of a SoH-custom path returns NULL and the call sites' stand-in
// fallbacks draw instead. Torch/vanilla oot.o2r DLs are unaffected.
// BLOCKLIST — ONLY the custom families that verifiably crash 2ship's interpreter. The observed
// discriminator is PALETTIZED (ci-format) textures referenced by Path= ("LeafTex_pal_rgba16" —
// object_jabbernut): the palette load fails ("Texture is null") and the DL then executes garbage.
// Plain-texture families (keys/keyring/bosskey "Hilite_new" rgba16, fishing pole) and geometry-only
// families (nei meshes, ocarina buttons, triforce) render FINE and stay allowed — user-confirmed.
static int OotAssets_IsBlockedCustomPath(const char* p) {
    // Empty by default — jabbernut was exonerated by testing (crash persisted with it blocked).
    // Keep the mechanism: add a dir here (or investigate) if a custom family is proven to crash.
    (void)p;
    return 0;
}

extern "C" unsigned char OotAssets_PathAllowed(const char* p) {
    // gRando.EnableSohCustomMeshes=1 re-enables the blocked families for experiments.
    return !OotAssets_IsBlockedCustomPath(p) || CVarGetInteger("gRando.EnableSohCustomMeshes", 0) != 0;
}

extern "C" unsigned char OotAssets_SohCustomEnabled(void) {
    return 1; // legacy shim for older call sites — per-path gating happens in OotAssets_PathAllowed
}

// ---------------------------------------------------------------------------------------------
// DEEP loader — the crash/wrong-model fix for direct-loaded OoT display lists.
//
// An archive-scoped load only redirects the TOP display list. Its CHILD references
// (G_VTX_OTR_FILEPATH / G_DL_OTR_FILEPATH / G_SETTIMG_OTR_FILEPATH) still resolve through the
// normal priority chain AT DRAW TIME, inside the gfx interpreter:
//   - shadowed paths (same name exists in mm.o2r/2ship.o2r) return MM's data: wrong model, or an
//     out-of-bounds vertex read -> 0xC0000005 in Fast::Interpreter::GfxSpVertex (crash log
//     classes gfx_vtx_otr_filepath_handler_custom / gfx_vtx_handler_f3dex2);
//   - companion-only paths fail LoadResourceProcess ("G_SETTIMG_OTR_FILEPATH: Texture is null")
//     and the stream can end up executing string/garbage data as commands (the "Unhandled OP
//     code: 0x54 0x63 ..." ASCII spam right before several crashes).
// libultraship is off-limits, so instead of fixing the interpreter we patch the loaded
// instruction stream IN PLACE: every child is resolved archive-scoped here, at load time, and
// the command is rewritten to its raw-pointer form. The buffers live in mm_asset_loader's
// private sOotResourceCache (never shared with the resource-manager cache), so mutating them
// cannot affect MM's own draws of same-named assets.
//
// oot.o2r display lists reference their children TWO ways, and ALL children (geometry AND textures)
// must be patched, because the companion archive is NOT in the resource system at draw time:
// ArchiveManager::AddArchive REJECTS oot.o2r (its OoT game version isn't in the MM build's
// mValidGameVersions), so oot.o2r's files never enter the global hash/path index. At draw time the
// interpreter therefore resolves NEITHER a child hash NOR a child path for an oot.o2r asset — a
// vertex ref reads garbage (crash / wrong shape) and a texture ref logs "Texture is null" (dark
// surface). So the walker resolves every child HERE, at load time, straight off the oot.o2r archive
// handle (MmAssets_LoadFromOotArchive / MmAssets_HashToPath, which reads oot.o2r's own file list),
// and rewrites the command to a raw-pointer form the interpreter can execute without any lookup.
//   - by PATH  (G_*_OTR_FILEPATH) — the SoH-custom-export style; w1 is a char* path.
//   - by HASH  (G_*_OTR_HASH)     — the OTRExporter style oot.o2r actually uses; MmAssets_HashToPath
//     turns the CRC64 back into its path via the archive's own ListFiles() map.
//
// Command encodings (verified against libultraship/src/fast/interpreter.cpp, READ-only):
//   G_VTX_OTR_FILEPATH  (0x24, 2 Gfx): w1=char* path; next: w0=count, w1=(dstIdx<<16)|vtxOff.
//     -> plain F3DEX2 G_VTX (0x01): w0=(op<<24)|(count<<12)|(((dstIdx+count)&0x7F)<<1),
//        w1=raw F3DVtx* (+vtxOff*16; GBI_FLOATS off -> sizeof(F3DVtx)==16). 2nd Gfx -> G_NOOP.
//   G_VTX_OTR_HASH      (0x32, 2 Gfx): w0 ALREADY in f3dex2 G_VTX bit layout (count=C0(12,8),
//        end=C0(1,7)); w1=byte offset; 2nd Gfx w0<<32|w1 = hash. -> flip opcode 0x32->0x01,
//        w1 = raw F3DVtx* + offset; 2nd Gfx -> G_NOOP.
//   G_DL_OTR_FILEPATH   (0x27, 1 Gfx): w1=char* path; C0(16,1): 0=call, 1=branch.
//     -> plain G_DL (0xDE) keeping the flag bits, w1=raw child Gfx* (patched recursively).
//   G_DL_OTR_HASH       (0x31, 2 Gfx): w0 flag C0(16,1); 2nd Gfx = hash. -> flip opcode 0x31->0xDE
//        keeping the flag bit, w1=raw child Gfx* (patched recursively); 2nd Gfx -> G_NOOP.
//   G_SETTIMG_OTR_FILEPATH (0x25, 1 Gfx) / G_SETTIMG_OTR_HASH (0x20, 2 Gfx): fmt/size/width bits are
//     identical to the plain RDP G_SETTIMG (0xFD) -> flip the opcode, w1 = texture ImageData (raw
//     pixels from MmAssets_LoadFromOotArchive). RawTexMetadata defaults h_byte_scale/v_pixel_scale to
//     1 so the empty-metadata RDP path renders at native resolution (no HD-pack support for these,
//     unavoidable since oot.o2r isn't in the resource system). 2nd Gfx (hash) -> G_NOOP.
namespace {

struct DeepGfx {
    uintptr_t w0;
    uintptr_t w1;
};

constexpr uint8_t OP_VTX_FILEPATH = 0x24;
constexpr uint8_t OP_SETTIMG_FILEPATH = 0x25;
constexpr uint8_t OP_DL_FILEPATH = 0x27;
constexpr uint8_t OP_PUSHCD = 0x28;
constexpr uint8_t OP_MTX_FILEPATH = 0x29;
constexpr uint8_t OP_SETTIMG_HASH = 0x20;
constexpr uint8_t OP_DL_HASH = 0x31;
constexpr uint8_t OP_VTX_HASH = 0x32;
constexpr uint8_t OP_MARKER = 0x33;
constexpr uint8_t OP_BRANCH_Z = 0x35;
constexpr uint8_t OP_VTX_F3DEX2 = 0x01;
constexpr uint8_t OP_DL_F3DEX2 = 0xDE;
constexpr uint8_t OP_ENDDL_F3DEX2 = 0xDF;
constexpr uint8_t OP_SETTIMG_RDP = 0xFD;
constexpr size_t kVtxStride = 16; // sizeof(F3DVtx) with GBI_FLOATS off (lus_gbi.h)
constexpr size_t kMaxCmds = 16384;
constexpr int kMaxDepth = 8;

// path -> patched DL (nullptr = remembered failure). Also serves as the cycle guard: the entry
// is inserted BEFORE the walk, so a self/looping reference resolves to the in-progress buffer.
std::unordered_map<std::string, void*> sDeepDlCache;

void* DeepLoadPatchDl(const char* otrPath, int depth) {
    if (otrPath == nullptr || *otrPath == '\0' || depth > kMaxDepth) {
        return nullptr;
    }
    std::string key(otrPath);
    auto it = sDeepDlCache.find(key);
    if (it != sDeepDlCache.end()) {
        return it->second;
    }

    DeepGfx* dl = (DeepGfx*)MmAssets_LoadFromOotArchive(otrPath, nullptr);
    if (dl == nullptr) {
        // Not in the companions. Do NOT negative-cache (archives may not be mounted yet — the
        // boot-order rule MmAssets_LoadFromOotArchive itself follows).
        return nullptr;
    }
    sDeepDlCache[key] = dl; // insert before walking: cycle guard + idempotency

    DeepGfx* c = dl;
    for (size_t n = 0; n < kMaxCmds; n++, c++) {
        uint8_t op = (uint8_t)((c->w0 >> 24) & 0xFF);
        if (op == OP_ENDDL_F3DEX2) {
            break;
        }
        switch (op) {
            case OP_VTX_FILEPATH: {
                const char* vp = (const char*)c->w1;
                size_t cnt = c[1].w0;
                size_t dstIdx = c[1].w1 >> 16;
                size_t vtxOff = c[1].w1 & 0xFFFF;
                void* vtx = (vp != nullptr) ? MmAssets_LoadFromOotArchive(vp, nullptr) : nullptr;
                if (vtx != nullptr) {
                    c->w0 = ((uintptr_t)OP_VTX_F3DEX2 << 24) | ((cnt & 0xFF) << 12) | (((dstIdx + cnt) & 0x7F) << 1);
                    c->w1 = (uintptr_t)((char*)vtx + vtxOff * kVtxStride);
                    c[1].w0 = 0; // G_NOOP
                    c[1].w1 = 0;
                } else if (vp == nullptr || !ResourceMgr_FileExists(vp)) {
                    // Unresolvable ANYWHERE: executing it would feed garbage to GfxSpVertex.
                    // Fail the whole load; the call site draws its stand-in instead.
                    sDeepDlCache[key] = nullptr;
                    return nullptr;
                } // else: companion-absent but chain-resolvable (no shadow possible) — leave it.
                n++;
                c++; // both layouts consume 2 Gfx here
                break;
            }
            case OP_DL_FILEPATH: {
                const char* dp = (const char*)c->w1;
                int branch = (int)((c->w0 >> 16) & 1);
                void* child = DeepLoadPatchDl(dp, depth + 1);
                if (child != nullptr) {
                    c->w0 = (c->w0 & 0x00FFFFFF) | ((uintptr_t)OP_DL_F3DEX2 << 24);
                    c->w1 = (uintptr_t)child;
                } else if (dp == nullptr || !ResourceMgr_FileExists(dp)) {
                    sDeepDlCache[key] = nullptr; // dead child DL == guaranteed garbage execution
                    return nullptr;
                }
                if (branch) {
                    n = kMaxCmds; // unconditional branch: nothing after this is reachable
                }
                break;
            }
            case OP_SETTIMG_FILEPATH: {
                // Textures MUST be patched to a raw ImageData pointer. Leaving the command as a
                // filepath/hash does NOT work for oot.o2r: the companion archive is game-version-
                // rejected from the global ArchiveManager index, so at draw time neither the path nor
                // the hash resolves (the interpreter logs "Texture is null" and the surface goes dark).
                // RawTexMetadata defaults h_byte_scale/v_pixel_scale to 1, so the empty-metadata RDP
                // path renders correctly. (Downside: a patched texture bypasses HD packs — unavoidable
                // here since oot.o2r isn't in the resource system at all.)
                const char* tp = (const char*)c->w1;
                void* img = (tp != nullptr) ? MmAssets_LoadFromOotArchive(tp, nullptr) : nullptr;
                if (img != nullptr) {
                    c->w0 = (c->w0 & 0x00FFFFFF) | ((uintptr_t)OP_SETTIMG_RDP << 24);
                    c->w1 = (uintptr_t)img; // Texture::GetRawPointer() == ImageData (raw pixels)
                }
                // Miss: leave it (nothing better to do); the interpreter skips a null texture safely.
                break;
            }
            case OP_VTX_HASH: {
                uintptr_t offset = c->w1; // byte offset added to the resolved vtx base
                uint64_t hash = ((uint64_t)c[1].w0 << 32) | (uint32_t)c[1].w1;
                const char* vp = MmAssets_HashToPath(hash);
                void* vtx = (vp != nullptr) ? MmAssets_LoadFromOotArchive(vp, nullptr) : nullptr;
                if (vtx != nullptr) {
                    // w0 already carries the f3dex2 G_VTX count/index bits — just swap the opcode.
                    c->w0 = (c->w0 & 0x00FFFFFF) | ((uintptr_t)OP_VTX_F3DEX2 << 24);
                    c->w1 = (uintptr_t)((char*)vtx + offset);
                    c[1].w0 = 0; // G_NOOP
                    c[1].w1 = 0;
                } else if (vp == nullptr || !ResourceMgr_FileExists(vp)) {
                    sDeepDlCache[key] = nullptr;
                    return nullptr;
                }
                n++;
                c++;
                break;
            }
            case OP_DL_HASH: {
                int branch = (int)((c->w0 >> 16) & 1);
                uint64_t hash = ((uint64_t)c[1].w0 << 32) | (uint32_t)c[1].w1;
                const char* dp = MmAssets_HashToPath(hash);
                void* child = (dp != nullptr) ? DeepLoadPatchDl(dp, depth + 1) : nullptr;
                if (child != nullptr) {
                    c->w0 = (c->w0 & 0x00FFFFFF) | ((uintptr_t)OP_DL_F3DEX2 << 24);
                    c->w1 = (uintptr_t)child;
                    c[1].w0 = 0; // G_NOOP
                    c[1].w1 = 0;
                } else if (dp == nullptr || !ResourceMgr_FileExists(dp)) {
                    sDeepDlCache[key] = nullptr;
                    return nullptr;
                } else {
                    n++;
                    c++;
                    break; // left as hash (chain-resolvable, no shadow) — skip its payload word
                }
                if (branch) {
                    n = kMaxCmds; // unconditional branch: rest unreachable
                    break;
                }
                n++;
                c++;
                break;
            }
            case OP_SETTIMG_HASH: {
                // Patch the texture to a raw ImageData pointer (see OP_SETTIMG_FILEPATH rationale —
                // oot.o2r hashes don't resolve at draw time, so an unpatched texture goes dark).
                uint64_t hash = ((uint64_t)c[1].w0 << 32) | (uint32_t)c[1].w1;
                const char* tp = MmAssets_HashToPath(hash);
                void* img = (tp != nullptr) ? MmAssets_LoadFromOotArchive(tp, nullptr) : nullptr;
                if (img != nullptr) {
                    c->w0 = (c->w0 & 0x00FFFFFF) | ((uintptr_t)OP_SETTIMG_RDP << 24);
                    c->w1 = (uintptr_t)img;
                    c[1].w0 = 0; // G_NOOP
                    c[1].w1 = 0;
                }
                n++; // 2-Gfx command — skip the hash payload word regardless
                c++;
                break;
            }
            // Multi-word commands we don't patch — skip their parameter words so the walk
            // never misreads a payload word as an opcode.
            case OP_MARKER:
            case OP_BRANCH_Z:
                n++;
                c++;
                break;
            case OP_PUSHCD:
            case OP_MTX_FILEPATH:
            default:
                break; // 1-Gfx command
        }
    }
    return dl;
}

} // namespace

extern "C" void* OotAssets_LoadGfxDirect(const char* otrPath) {
    if (otrPath == nullptr || *otrPath == '\0') {
        return nullptr;
    }
    if (!OotAssets_PathAllowed(otrPath)) {
        return nullptr; // blocked family: caller falls back to its stand-in draw
    }
    // Archive-scoped DEEP load first: resolves the path AND every child reference from the
    // OoT/soh companion archives, patching the instruction stream to raw pointers. This is the
    // only way to correctly draw an OoT DL whose paths are SHADOWED by same-named MM assets
    // (companions mount at LOWEST priority) — and it prevents the child-miss crashes.
    void* gfx = DeepLoadPatchDl(otrPath, 0);
    if (gfx != nullptr) {
        return gfx;
    }
    // Fallback: the NORMAL chain. Covers SoH custom assets copied into mm/assets/custom (packed
    // into 2ship.o2r — e.g. object_key/object_keyring/object_bosskey/object_gi_fishing_pole), so
    // those resolve even when soh.o2r isn't shipped. Safe ordering: a shadowed path already
    // returned above; a path reaching here is either companion-absent or 2ship-local.
    if (ResourceMgr_FileExists(otrPath)) {
        return (void*)ResourceMgr_LoadGfxByName(otrPath);
    }
    return nullptr;
}
