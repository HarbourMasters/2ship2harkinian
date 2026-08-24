// FleetSync.cpp (MM side) — cross-game save cache + shared player-state overlay.
//
// Mirror of the OoT implementation (soh/soh/FleetShipCombo/FleetSync.cpp) with MM accessors:
// - Anchor "mm" = the whole SaveContext via BenJsonConversions (to_json/from_json exist).
// - "shared" is canonical in the OoT item-id space: bottles/equips are translated through
//   FleetComboIds.h; souls/swim bridge to the rando's randoInf flags; triforce to
//   shipSaveInfo.rando.foundTriforcePieces.
// - Frozen-save (responder): replicates the AutoSave owl-save recipe; the sram flash pump runs in
//   Play_UpdateMain even while the game is combo-frozen, so the queued write completes.

#include "FleetSync.h"
#include "FleetShipCombo.h"
#include "FleetComboIds.h"
#include "FleetComboItemsGlue.h" // FcCombo_NativeForItem (fcId -> native RI)
#include "FleetComboOptions.h"   // FC_COMBO_OPTION_TABLE (shared NEI options)
#include "FleetComboItems.h"     // FCI_MAX, FCI_NO_ITEM
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/BenJsonConversions.hpp"
#include "2s2h/Rando/Types.h"
#include "2s2h/Rando/Rando.h" // Rando::GiveItem

#include <libultraship/bridge/consolevariablebridge.h> // CVar: persist last-saved slot for auto-resume
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#else
#include <unistd.h>
#endif

extern "C" {
#include "z64.h"
#include "macros.h"
#include "functions.h"
#include "variables.h"
#include "mods/nei_save.h"
extern PlayState* gPlayState;
extern SaveContext gSaveContext;
// The combo's single shared pictograph (FleetPicto.cpp) — pulled in on arrival.
void FleetPicto_Import(void);
// Nonzero while ApplyFcRegistryToNatives is materializing an FC deficit through the game's native
// give path. GiveItem.cpp's record hook checks this and skips recording those grants — otherwise a
// registry-driven grant would re-bump comboObtainedFc and feed itself an endless per-frame deficit.
int gFcCombo_SuppressRecord = 0;
// Bottle wheel reconcile (mods/items/custom_bottles.h). Declared here, INSIDE this extern "C"
// block, for the same reason as the ExtEquip accessors below. They must not be declared at the
// callsite: that sits in an anonymous namespace, so a local `extern void Bottle_...` picks up C++
// linkage in that namespace and the linker looks for `?Bottle_WheelPersist@?A0x...@@YAXEG@Z`,
// which nothing defines (custom_bottles.cpp defines them as extern "C").
void Bottle_WheelPersist(uint8_t wheel, uint16_t slotItem);
void Bottle_WheelRecordActive(uint8_t wheel, uint16_t slotItem);
// Upgrade-column equipment (mods/extended_equipment.h). Declared here rather than including that
// header, which pulls z64item.h/color.h into this TU for four accessors.
unsigned char ExtEquip_CapeOwned(void);
void ExtEquip_GiveCape(void);
unsigned char ExtEquip_PendantOwned(void);
void ExtEquip_GivePendant(void);
// Ownership of a page-2 equipment cell (extEquipOwnedBits). Needed by the fold below.
unsigned char ExtEquip_HasItem(short equipType, unsigned char index);
// Bottle wheel fold (custom_bottles.cpp) — declared HERE, in the extern "C" block: a declaration
// inside an anonymous namespace mangles as a local C++ symbol and fails to link (bit soh first).
void Bottle_WheelPersist(unsigned char wheel, unsigned short slotItem);
void Bottle_WheelRecordActive(unsigned char wheel, unsigned short slotItem);
// Elemental Wand / Sheikah Slate grants (mods/extended_inventory.c): place the cell item, light the
// rod/rune bit and pick the active mode -- the same call the native pickup makes.
void Wand_GrantMode(unsigned char mode);
void Slate_GrantRune(unsigned char rune);
}

// Cross-game restart: the raw reset of THIS game (defined in DebugConsole.cpp), called by the
// responder pump below WITHOUT signaling so a paired reset never ping-pongs.
extern "C" void FleetCombo_DoLocalReset(void);

namespace {

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
    return std::filesystem::canonical(buf).parent_path();
#else
    return std::filesystem::canonical("/proc/self/exe").parent_path();
#endif
}

// 2ship lives at <ShipDir>/2ship/ -> the shared files live in <ShipDir>/fleet/.
std::filesystem::path TempFilePath() {
    std::filesystem::path dir = SelfExeDir();
    if (dir.empty()) {
        return {};
    }
    std::error_code ec;
    std::filesystem::create_directories(dir.parent_path() / "fleet", ec);
    return dir.parent_path() / "fleet" / "temp_flags.json";
}

bool ReadTemp(nlohmann::json& out) {
    std::filesystem::path p = TempFilePath();
    if (p.empty() || !std::filesystem::exists(p)) {
        return false;
    }
    try {
        std::ifstream in(p);
        in >> out;
        return out.is_object();
    } catch (...) {
        SPDLOG_WARN("[FleetSync] temp file unreadable — treating as absent");
        return false;
    }
}

void WriteTemp(const nlohmann::json& j) {
    std::filesystem::path p = TempFilePath();
    if (p.empty()) {
        return;
    }
    try {
        std::filesystem::path tmp = p;
        tmp += ".tmp2"; // distinct from the host's .tmp so simultaneous writes don't collide
        {
            std::ofstream out(tmp);
            out << std::setw(1) << j << std::endl;
        }
        std::filesystem::rename(tmp, p);
    } catch (...) { SPDLOG_WARN("[FleetSync] temp file write failed"); }
}

void DeleteTemp() {
    std::filesystem::path p = TempFilePath();
    try {
        if (!p.empty() && std::filesystem::exists(p)) {
            std::filesystem::remove(p);
        }
    } catch (...) {}
}

// Handy roots
#define MM_INV gSaveContext.save.saveInfo.inventory
#define MM_PD gSaveContext.save.saveInfo.playerData
#define MM_EQ gSaveContext.save.saveInfo.equips

// nei->ootUpgrades 3-bit fields (mm/mods/nei_save.h): bulletBag@0, quiver@3, bombBag@6,
// strength@9, scale@12.
int OotUpgGet(int shift) {
    return (Nei_Save()->ootUpgrades >> shift) & 0x7;
}
void OotUpgSet(int shift, int v) {
    NeiSaveData* nei = Nei_Save();
    nei->ootUpgrades = (uint16_t)((nei->ootUpgrades & ~(0x7 << shift)) | ((v & 0x7) << shift));
}

// MM Kokiri-chain level from the shared weaponUpgrades byte (bits: 1<<1 razor, 1<<2 gilded).
int KokiriChainLevel() {
    uint8_t wu = Nei_Save()->weaponUpgrades;
    if (wu & (1 << 2))
        return 2;
    if (wu & (1 << 1))
        return 1;
    return 0;
}

uint16_t ComputeShieldOwned() {
    NeiSaveData* nei = Nei_Save();
    uint16_t sh = nei->shieldOwned;
    int nibble = (MM_EQ.equipment >> 4) & 0xF; // 1 Hero, 2 Mirror(-MM)
    if (nibble >= 1)
        sh |= FC_SHIELD_HYLIAN; // MM Hero == OoT Hylian
    if (nibble == 2)
        sh |= FC_SHIELD_IKANA; // MM Mirror == OoT Shield of Ikana
    if (nei->extEquipOwnedBits & (1u << 19))
        sh |= FC_SHIELD_DIVINE;
    if (nei->extEquipOwnedBits & (1u << 20))
        sh |= FC_SHIELD_KITE;
    if (nei->extEquipOwnedBits & (1u << 21))
        sh |= FC_SHIELD_IKANA;
    nei->shieldOwned = sh;
    return sh;
}

int GetEquippedShieldCanonical() {
    NeiSaveData* nei = Nei_Save();
    if (nei->extEquipShield >= 1 && nei->extEquipShield <= 3) {
        return 3 + nei->extEquipShield;
    }
    int nibble = (MM_EQ.equipment >> 4) & 0xF;
    if (nibble == 1)
        return 2; // Hero -> Hylian
    if (nibble == 2)
        return 6; // Mirror-MM -> Ikana
    return 0;
}

void SetEquippedShieldCanonical(int canon) {
    NeiSaveData* nei = Nei_Save();
    switch (canon) {
        case 2: // Hylian -> Hero
            nei->extEquipShield = 0;
            MM_EQ.equipment = (uint16_t)((MM_EQ.equipment & ~0xF0) | (1 << 4));
            break;
        case 6: // Ikana -> native MM Mirror
            nei->extEquipShield = 0;
            MM_EQ.equipment = (uint16_t)((MM_EQ.equipment & ~0xF0) | (2 << 4));
            break;
        case 4: // Divine (NEI ext 1 over Hero base)
        case 5: // Kite (NEI ext 2)
            nei->extEquipShield = (uint8_t)(canon - 3);
            MM_EQ.equipment = (uint16_t)((MM_EQ.equipment & ~0xF0) | (1 << 4));
            break;
        default:
            break; // deku / mirror-OoT / none: no MM relative -> keep current
    }
}

// MM inventory slot constants used below (mm/include/z64item.h InventorySlot).
enum {
    kSlotOcarina = 0x00,
    kSlotBow = 0x01,
    kSlotArrowFire = 0x02,
    kSlotArrowIce = 0x03,
    kSlotArrowLight = 0x04,
    kSlotBomb = 0x06,
    kSlotBombchu = 0x07,
    kSlotStick = 0x08,
    kSlotNut = 0x09,
    kSlotBeans = 0x0A,
    kSlotPowderKeg = 0x0C,
    kSlotPictograph = 0x0D,
    kSlotLens = 0x0E,
    kSlotHookshot = 0x0F,
    kSlotGreatFairySword = 0x10,
};

// ---- POST-FLIP TRACE (diagnostic, temporary — see FleetSync.h) ----
// Armed by the flip commit, counts down per frame. While armed, each step of the frozen game's work
// announces itself, so the LAST line in a truncated log names whatever killed the process. Disarmed
// it costs one int compare, and it expires on its own.
int sPostFlipTrace = 0;
constexpr int kPostFlipTraceFrames = 180; // ~3s; the observed crash lands within ~1.7s of the flip

// Flushed on every line ON PURPOSE. The whole point is to read the log of a process that DIED, and
// a buffered logger loses exactly the lines that matter — the last ones. Only while armed.
#define FS_TRACE(...)                                 \
    do {                                              \
        if (sPostFlipTrace > 0) {                     \
            SPDLOG_WARN("[FleetTrace] " __VA_ARGS__); \
            spdlog::default_logger()->flush();        \
        }                                             \
    } while (0)

void PutInvItem(nlohmann::json& inv, const char* key, int slot, bool withAmmo) {
    uint8_t item = MM_INV.items[slot];
    inv[key] = (item != 0xFF);
    if (withAmmo) {
        inv[std::string(key) + "Ammo"] = (int)MM_INV.ammo[slot];
    }
}

void ApplyInvItem(const nlohmann::json& inv, const char* key, int slot, uint8_t itemId, bool withAmmo) {
    if (!inv.contains(key)) {
        return;
    }
    if (inv[key].get<bool>()) {
        if (MM_INV.items[slot] == 0xFF) {
            MM_INV.items[slot] = itemId;
        }
    }
    if (withAmmo) {
        std::string ak = std::string(key) + "Ammo";
        if (inv.contains(ak) && MM_INV.items[slot] != 0xFF) {
            MM_INV.ammo[slot] = (int8_t)inv[ak].get<int>();
        }
    }
}

// Souls/abilities bridge: contiguous randoInf block <-> registry (index math, FleetComboIds.h).
// Page-2 equipment -> FC registry. THE bug behind "the equipment I got in one game never shows up in
// the other": ownership of these cells lives in extEquipOwnedBits, and that word never travels — it
// is not serialized as a word, and the fold below never looked at it. So the ONLY equipment that ever
// crossed was what happened to be granted through the randomizer hook (which records into
// comboObtainedFc by itself) plus the three shields, which survive by accident because
// ComputeShieldOwned reads their bits into the shieldOwned mask. Anything granted by the save editor,
// by an item behavior, or by ExtEquip_Init's migrations stayed local forever.
//
// Both counters are raised: `obtained` so the peer learns about it, and `applied` because the piece
// is ALREADY materialized here — without that, ApplyFcRegistryToNatives would see a deficit against
// our own fold and re-grant it locally every single frame, logging a [FleetGrant] each time.
//
// Trident, Climb Boots and Roc Boots are missing on purpose: they have no FCI_/RG_/RI_ identity yet,
// so there is no row to fold them into. Magic Cape and Pendant of Memories travel as their own
// booleans (capeOwned / the trade bit), not through this table. Skijer's NEI
// MM's native EquipmentType only goes SWORD=0/SHIELD=1/TUNIC=2; BOOTS is the NEI accessory tag.
// Same guarded mirror GiveItem.cpp keeps, so this TU doesn't have to pull extended_equipment.h.
#ifndef EQUIP_TYPE_BOOTS
#define EQUIP_TYPE_BOOTS 3
#endif
static void FoldExtEquipmentIntoRegistry(NeiSaveData* nei) {
    static const struct {
        short equipType;
        unsigned char index;
        int fcId;
    } kExtEquipRows[] = {
        { EQUIP_TYPE_SWORD, 1, FCI_EXT_CANE_OF_BYRNA },
        { EQUIP_TYPE_SWORD, 2, FCI_EXT_FOUR_SWORD },
        { EQUIP_TYPE_SHIELD, 1, FCI_EXT_DIVINE_SHIELD },
        { EQUIP_TYPE_SHIELD, 2, FCI_EXT_SHEIKAH_SHIELD },
        { EQUIP_TYPE_TUNIC, 1, FCI_EXT_CHAMPIONS_TUNIC },
        { EQUIP_TYPE_TUNIC, 2, FCI_EXT_SPIRIT_BREASTPLATE },
        { EQUIP_TYPE_TUNIC, 3, FCI_EXT_WATER_DRAGON_SCALE },
        { EQUIP_TYPE_BOOTS, 1, FCI_EXT_PEGASUS_ANKLET },
        { EQUIP_TYPE_SWORD, 3, FCI_EXT_TRIDENT },
        { EQUIP_TYPE_BOOTS, 2, FCI_EXT_CLIMB_BOOTS },
        { EQUIP_TYPE_BOOTS, 3, FCI_EXT_ROC_BOOTS },
    };
    for (size_t i = 0; i < sizeof(kExtEquipRows) / sizeof(kExtEquipRows[0]); i++) {
        const int fcId = kExtEquipRows[i].fcId;
        if (fcId < 0 || fcId >= FC_COMBO_OBTAINED_FC_SIZE) {
            continue;
        }
        if (!ExtEquip_HasItem(kExtEquipRows[i].equipType, kExtEquipRows[i].index)) {
            continue;
        }
        if (nei->comboObtainedFc[fcId] == 0) {
            nei->comboObtainedFc[fcId] = 1;
        }
        if (nei->comboAppliedFc[fcId] < nei->comboObtainedFc[fcId]) {
            nei->comboAppliedFc[fcId] = nei->comboObtainedFc[fcId];
        }
    }
}

void FoldNativesIntoRegistry() {
    NeiSaveData* nei = Nei_Save();
    FoldExtEquipmentIntoRegistry(nei);
    for (int i = 0; i <= (int)FC_SOUL_LAST; i++) {
        if (Flags_GetRandoInf(RANDO_INF_OBTAINED_SOUL_OF_BOSS_GOHT + i)) {
            nei->comboObtained[FC_SOUL_FIRST + i] = 1;
        }
    }
    if (Flags_GetRandoInf(RANDO_INF_OBTAINED_SWIM)) {
        nei->comboObtained[FC_ABILITY_SWIM] = 1;
    }
    uint16_t tf = gSaveContext.save.shipSaveInfo.rando.foundTriforcePieces;
    if (tf > nei->comboTriforce) {
        nei->comboTriforce = tf;
    }
    // World-progress counters mirrored for the rando (info-only in OoT):
    uint32_t skulls = gSaveContext.save.saveInfo.skullTokenCount;
    nei->comboObtained[FC_MM_SKULLS_SWAMP] = (uint8_t)std::min<uint32_t>((skulls >> 16) & 0xFFFF, 255);
    nei->comboObtained[FC_MM_SKULLS_OCEAN] = (uint8_t)std::min<uint32_t>(skulls & 0xFFFF, 255);
    for (int i = 0; i < 4; i++) {
        int8_t f = MM_INV.strayFairies[i];
        if (f > 0 && (uint8_t)f > nei->comboObtained[FC_MM_FAIRIES_WOODFALL + i]) {
            nei->comboObtained[FC_MM_FAIRIES_WOODFALL + i] = (uint8_t)f;
        }
    }
}

void ApplyRegistryToNatives() {
    NeiSaveData* nei = Nei_Save();

    // OoT trade chain -> unified trade wheel (2026-08-07). soh's FoldNativesIntoRegistry flags the
    // held trade item into comboObtained[FC_OOT_TRADE_*], the array travels, MM merges it... and
    // NOTHING here ever read those indices — the classic registry cul-de-sac (same as the old
    // FC_OOT_STRENGTH). So a Weird Egg or Cojiro earned in OoT never reached MM's wheel, which is
    // the reported "child trade items don't arrive" bug. The mapping is the wheel's own index order
    // (trade_items.c sTradeAdultItems): 0-10 = OoT adult chain POCKET_EGG..CLAIM_CHECK, 20-22 =
    // OoT child WEIRD_EGG/CHICKEN/ZELDAS_LETTER. Additive only, like every registry apply.
    {
        static const struct {
            int fcId;
            int wheelIndex;
        } kOotTradeToWheel[] = {
            { FC_OOT_TRADE_POCKET_EGG, 0 },   { FC_OOT_TRADE_POCKET_CUCCO, 1 },   { FC_OOT_TRADE_COJIRO, 2 },
            { FC_OOT_TRADE_ODD_MUSHROOM, 3 }, { FC_OOT_TRADE_ODD_POTION, 4 },     { FC_OOT_TRADE_SAW, 5 },
            { FC_OOT_TRADE_BROKEN_SWORD, 6 }, { FC_OOT_TRADE_PRESCRIPTION, 7 },   { FC_OOT_TRADE_FROG, 8 },
            { FC_OOT_TRADE_EYEDROPS, 9 },     { FC_OOT_TRADE_CLAIM_CHECK, 10 },   { FC_OOT_TRADE_WEIRD_EGG, 20 },
            { FC_OOT_TRADE_CHICKEN, 21 },     { FC_OOT_TRADE_ZELDAS_LETTER, 22 },
        };
        for (size_t i = 0; i < sizeof(kOotTradeToWheel) / sizeof(kOotTradeToWheel[0]); i++) {
            if (nei->comboObtained[kOotTradeToWheel[i].fcId]) {
                nei->tradeAdultOwned |= (1u << kOotTradeToWheel[i].wheelIndex);
            }
        }
    }

    for (int i = 0; i <= (int)FC_SOUL_LAST; i++) {
        if (nei->comboObtained[FC_SOUL_FIRST + i] && !Flags_GetRandoInf(RANDO_INF_OBTAINED_SOUL_OF_BOSS_GOHT + i)) {
            Flags_SetRandoInf(RANDO_INF_OBTAINED_SOUL_OF_BOSS_GOHT + i);
        }
    }
    if (nei->comboObtained[FC_ABILITY_SWIM] && !Flags_GetRandoInf(RANDO_INF_OBTAINED_SWIM)) {
        Flags_SetRandoInf(RANDO_INF_OBTAINED_SWIM);
    }
    uint16_t& tf = gSaveContext.save.shipSaveInfo.rando.foundTriforcePieces;
    if (nei->comboTriforce > tf) {
        tf = nei->comboTriforce;
    }
}

// Generic fcId-indexed cross-item applier: for every FC-shared item with a native MM relative, grant
// the DEFICIT of copies (obtained - applied) that were recorded elsewhere (e.g. picked up in OoT) and
// synced into comboObtainedFc. comboAppliedFc is the local shadow of what's already materialized here,
// so this is idempotent: items obtained natively here were counted into BOTH stores by GiveItem's
// record hook and never re-granted. Deficit + shadow means it's safe to run every frame. Rando::GiveItem
// derefs gPlayState (Item_Give), so callers MUST gate on gPlayState != NULL.
void ApplyFcRegistryToNatives() {
    NeiSaveData* nei = Nei_Save();
    for (int fcId = 0; fcId < FCI_MAX && fcId < FC_COMBO_OBTAINED_FC_SIZE; fcId++) {
        int native = FcCombo_NativeForItem(fcId);
        if (native == FCI_NO_ITEM) {
            continue; // no MM relative — info-only in this game
        }
        uint8_t obtained = nei->comboObtainedFc[fcId];
        uint8_t applied = nei->comboAppliedFc[fcId];
        if (obtained > applied) {
            gFcCombo_SuppressRecord++; // don't let these grants re-record into comboObtainedFc
            for (int k = applied; k < (int)obtained; k++) {
                // Logged UNCONDITIONALLY, not just during the post-flip trace. This is the ONLY
                // place that hands the player an item nobody picked up in this game, so every
                // "why did MM just give me all of OoT's medallions/songs?" report is answered by
                // these lines: they name the fcId, the native item and how many copies. They are
                // rare by construction (only fired on a deficit), so they cost nothing.
                //
                // It is also the line most likely to kill the process: Rando::GiveItem walks into
                // Item_Give, which touches gPlayState, the player and the message system with an id
                // that came from the OTHER game's registry. A bad id is a raw memory fault, not an
                // exception, so no catch() can see it — but the log stops right here.
                SPDLOG_WARN("[FleetGrant] fcId={} -> native={} (copy {} of {}) — cross-game grant", fcId, native, k + 1,
                            (int)obtained);
                spdlog::default_logger()->flush();
                Rando::GiveItem((RandoItemId)native);
                FS_TRACE("3b. grant fcId={} OK", fcId);
            }
            gFcCombo_SuppressRecord--;
            nei->comboAppliedFc[fcId] = obtained;
        }
    }
}

// HEALING: earlier builds copied ownedItems RAW across the id spaces, leaking OoT ids into MM's
// NeiSaveData (page-2 ids 0x9E..0xB5, page-3 mask ids 0xB8..0xCF) — MM icon/name lookups on those
// ids read garbage pointers and crashed the pause menu. Translate leaked entries in place; runs on
// every arrival/save-signal so poisoned saves heal themselves.
void HealLeakedOwnedItems() {
    NeiSaveData* nei = Nei_Save();
    for (int i = 0; i < 48; i++) {
        uint16_t v = nei->ownedItems[i]; // u16 store (see NeiSaveData.ownedItems)
        if (v == 0xFF || v > 0xFF) {
            continue; // empty, or an EXT id (0x02xx) that never came from the OoT u8 space
        }
        if (v >= 0x9E && v <= 0xB5) {
            nei->ownedItems[i] = (uint16_t)(v + FC_PAGE2_MM_OFFSET); // leaked OoT page-2 id -> MM id
        } else if (i >= 24 && v >= FC_OOT_MM_MASK_ITEM_BASE && v < FC_OOT_MM_MASK_ITEM_BASE + FC_MM_MASK_COUNT) {
            // Leaked OoT page-3 mask id: masks are NATIVE in MM — grant the native mask and clear.
            if (MM_INV.items[FC_MM_MASK_SLOT_BASE + (v - FC_OOT_MM_MASK_ITEM_BASE)] == 0xFF) {
                MM_INV.items[FC_MM_MASK_SLOT_BASE + (v - FC_OOT_MM_MASK_ITEM_BASE)] =
                    kFcMmMaskItemBySlot[v - FC_OOT_MM_MASK_ITEM_BASE];
            }
            nei->ownedItems[i] = 0xFF;
        }
    }
}

// Cell repair (idempotent, both directions). Ownership of these cells is a FLAG/bitmask and the
// cell can be emptied by something other than the player (OoT's HealBogusOwnedItems used to wipe
// the wand cell on every extract; the Sheikah Slate cell was overwritten by the u16->u8 truncation
// loop). Once the bit is set, the incremental "gained" grants in ApplyShared never fire again, so
// the cell has to be re-seeded from the flags -- exactly what the kaleido's Page2Relayout_Heal does
// for the Shovel/Dominion wheel. Runs on every extract and every apply. Slots are
// extended_inventory.h SLOT_* (header not included in this TU).
static void RepairFlagOwnedCells(NeiSaveData* nei) {
    const uint8_t kSlotWand = 27, kSlotSlate = 39, kSlotShovel = 46;
    const uint16_t kExtSheikahSlate = 0x0220; // EXT_ITEM_SHEIKAH_SLATE (same id in both games)
    if (nei->wandRodsOwned != 0 && Nei_GetOwnedItem(kSlotWand) != ITEM_ELEMENTAL_WAND) {
        Nei_SetOwnedItem(kSlotWand, ITEM_ELEMENTAL_WAND);
    }
    if (nei->slateRunesOwned != 0 && Nei_GetOwnedItem(kSlotSlate) != kExtSheikahSlate) {
        Nei_SetOwnedItem(kSlotSlate, kExtSheikahSlate);
    }
    if (nei->shovelOwned || nei->dominionOwned) {
        const uint16_t cur = Nei_GetOwnedItem(kSlotShovel);
        if (cur != ITEM_SHOVEL && cur != ITEM_DOMINION_ROD) {
            Nei_SetOwnedItem(kSlotShovel, nei->shovelOwned ? ITEM_SHOVEL : ITEM_DOMINION_ROD);
        }
    }
}

void ExtractShared(nlohmann::json& sh) {
    NeiSaveData* nei = Nei_Save();
    HealLeakedOwnedItems();
    FoldNativesIntoRegistry();

    sh["schema"] = 1;
    sh["vitals"] = { { "health", MM_PD.health },
                     { "healthCapacity", MM_PD.healthCapacity },
                     { "doubleDefense", MM_PD.doubleDefense },
                     { "defenseHearts", MM_INV.defenseHearts },
                     { "magic", MM_PD.magic },
                     // magicLevel is NOT published: it is the game's own meter-build handshake, not
                     // shared state. See the magic block in ApplyShared.
                     { "isMagic", MM_PD.isMagicAcquired },
                     { "isDoubleMagic", MM_PD.isDoubleMagicAcquired },
                     { "rupees", MM_PD.rupees } };
    sh["upgrades"] = { { "wallet", CUR_UPG_VALUE(UPG_WALLET) },
                       { "quiver", CUR_UPG_VALUE(UPG_QUIVER) },
                       { "bombBag", CUR_UPG_VALUE(UPG_BOMB_BAG) },
                       { "sticks", CUR_UPG_VALUE(UPG_DEKU_STICKS) },
                       { "nuts", CUR_UPG_VALUE(UPG_DEKU_NUTS) },
                       { "strength", std::max<int>(CUR_UPG_VALUE(UPG_STRENGTH), OotUpgGet(9)) },
                       { "scale", std::max<int>(CUR_UPG_VALUE(UPG_SCALE), OotUpgGet(12)) },
                       { "bulletBag", OotUpgGet(0) } };
    // OoT "Grab" skill (RAND_INF_CAN_GRAB on the soh side). Not an upgrade LEVEL — it is the first
    // link of the Progressive Strength chain when Shuffle Grab is on, and MM can receive that copy.
    // Latched, never cleared, so it cannot walk backwards. Skijer's NEI
    sh["canGrab"] = (int)nei->ootCanGrab;
    // GFS: keep the native inventory item and the shared weaponUpgrades bit coherent both ways.
    if (MM_INV.items[kSlotGreatFairySword] != 0xFF) {
        nei->weaponUpgrades |= (1 << 4); // WEAPON_UPGRADE_BGS_GREAT_FAIRY
    }
    sh["weaponUpgrades"] = nei->weaponUpgrades;
    sh["shieldOwned"] = ComputeShieldOwned();
    sh["equippedShield"] = GetEquippedShieldCanonical();
    // Upgrade-column equipment (Skijer 2026-07-31): the Magic Cape and the Pendant of Memories live
    // outside both extEquipOwnedBits and the equipment word, so nothing was carrying them across —
    // obtaining either in one game left the other game's equipment page empty. Ownership only; the
    // capeHidden / pendantEffectOff toggles stay per-game (they are view/moveset preferences, and
    // OR-merging a toggle would make it impossible to turn off). ExtEquip_PendantOwned() is called
    // rather than read raw so the adult-trade-slot grant latches before we publish.
    sh["capeOwned"] = ExtEquip_CapeOwned() != 0;
    sh["pendantOwned"] = ExtEquip_PendantOwned() != 0;
    // Sword flags: kokiri chain is native; master/bgs base ownership rides the registry.
    int swordNibble = MM_EQ.equipment & 0xF; // 1 kokiri, 2 razor, 3 gilded, 4 deity
    sh["swordFlags"] = { { "kokiri", swordNibble >= 1 || KokiriChainLevel() > 0 },
                         { "master", nei->comboObtained[FC_OOT_SWORD_MASTER] != 0 },
                         { "biggoron", nei->comboObtained[FC_OOT_SWORD_BIGGORON] != 0 } };
    sh["equippedSword"] = (swordNibble >= 1 && swordNibble <= 3) ? 1 : (swordNibble == 4 ? 4 : 0);

    nlohmann::json inv = sh.contains("inv") ? sh["inv"] : nlohmann::json::object();
    PutInvItem(inv, "stick", kSlotStick, true);
    PutInvItem(inv, "nut", kSlotNut, true);
    PutInvItem(inv, "bomb", kSlotBomb, true);
    PutInvItem(inv, "bow", kSlotBow, true);
    PutInvItem(inv, "bombchu", kSlotBombchu, true);
    PutInvItem(inv, "fireArrow", kSlotArrowFire, false);
    PutInvItem(inv, "iceArrow", kSlotArrowIce, false);
    PutInvItem(inv, "lightArrow", kSlotArrowLight, false);
    PutInvItem(inv, "lens", kSlotLens, false);
    PutInvItem(inv, "beans", kSlotBeans, true);
    inv["ocarinaFairy"] = MM_INV.items[kSlotOcarina] != 0xFF;
    inv["ocarinaTime"] = MM_INV.items[kSlotOcarina] == ITEM_OCARINA_OF_TIME;
    inv["boomerang"] = nei->ootBoomerangOwned != 0;
    inv["hammer"] = nei->ootHammerOwned != 0;
    inv["dins"] = (nei->ootSpellsOwned & (1 << 0)) != 0;
    inv["farores"] = (nei->ootSpellsOwned & (1 << 1)) != 0;
    inv["nayrus"] = (nei->ootSpellsOwned & (1 << 2)) != 0;
    // Bit 3 is the SHIP-VANILLA Roc's Feather, which shares the Nayru's Love cell. It is a separate
    // item from Skijer's feather in SLOT_ROCS (synced with the other page-2 items), and it was the
    // one bit of ootSpellsOwned this block forgot — so it never crossed. Skijer's NEI
    inv["rocsFeatherVanilla"] = (nei->ootSpellsOwned & (1 << 3)) != 0;
    inv["slingshot"] = nei->slingshotOwned != 0;
    inv["slingshotAmmo"] = nei->slingshotSeeds;
    int hookLevel = nei->ootHookshotLevel;
    if (hookLevel == 0 && MM_INV.items[kSlotHookshot] != 0xFF) {
        hookLevel = 1; // native MM hookshot with no OoT-chain level recorded
    }
    inv["hookshotLevel"] = hookLevel;
    inv["clawshot"] = nei->clawshotOwned != 0;
    // Pictograph Box: MM-native slot is the single source of truth (the NEI pictobox flag is gone).
    inv["pictobox"] = MM_INV.items[kSlotPictograph] != 0xFF;
    bool kegOwned = MM_INV.items[kSlotPowderKeg] != 0xFF || nei->powerKegOwned;
    inv["powderKeg"] = kegOwned;
    inv["powderKegCount"] = std::max<int>(MM_INV.ammo[kSlotPowderKeg], nei->powerKegCount);
    inv["net"] = nei->netEquipped != 0;
    inv["bottomlessMode"] = nei->bottomlessBottleMode;
    {
        uint8_t bt = FcBottle_MmToOot(nei->bottomlessContent); // canonical = OoT ids
        inv["bottomlessContent"] = (bt == FC_BOTTLE_UNMAPPED) ? 0x14 : bt;
    }
    inv["bottomlessCount"] = nei->bottomlessCount;
    sh["inv"] = inv;

    // Fold the LIVE bottles into the shared store before publishing (2026-08-07). The wheel's
    // Persist/RecordActive pair only ran from the kaleido, so a fairy caught (or potion drunk)
    // in-game sat ONLY in the native slot until the player happened to open the pause item page —
    // and FleetSync kept publishing the stale wheel array the whole time. That is the reported
    // "bottles don't share correctly". Same pair the kaleido uses, driven by the two native cells.
    {
        // (Bottle_WheelPersist / Bottle_WheelRecordActive are declared in the extern "C" block at
        // the top of this file — declaring them here would give them anonymous-namespace linkage.)
        uint16_t nativeA = gSaveContext.save.saveInfo.inventory.items[SLOT_BOTTLE_1];
        uint16_t nativeB = gSaveContext.save.saveInfo.inventory.items[SLOT_BOTTLE_2];
        Bottle_WheelPersist(0, nativeA); // BOTTLE_WHEEL_A
        Bottle_WheelRecordActive(0, nativeA);
        Bottle_WheelPersist(1, nativeB); // BOTTLE_WHEEL_B
        Bottle_WheelRecordActive(1, nativeB);
    }

    // Bottles: MM stores MM content ids -> translate to the canonical OoT space.
    nlohmann::json bottles = nlohmann::json::array();
    for (int i = 0; i < 8; i++) {
        uint8_t t = FcBottle_MmToOot(nei->bottleSlots[i]);
        if (t == FC_BOTTLE_UNMAPPED) {
            t = 0x14; // canonical (OoT) empty bottle — never leak a foreign id
        }
        bottles.push_back(t);
    }
    sh["bottleSlots"] = bottles;
    // Canonical ownedItems = OoT id space: translate our page-2 entries; mask entries [24..47]
    // come from MM's NATIVE mask inventory (MM never stores masks in ownedItems).
    nlohmann::json ownedItems = nlohmann::json::array();
    for (int i = 0; i < 48; i++) {
        uint8_t canon = 0xFF;
        if (i >= 24) {
            if (MM_INV.items[FC_MM_MASK_SLOT_BASE + (i - 24)] != 0xFF) {
                canon = (uint8_t)(FC_OOT_MM_MASK_ITEM_BASE + (i - 24));
            }
            // 0x00 must be treated as EMPTY here, exactly like 0xFF. An uninitialized ownedItems
            // slot reads as a raw 0x00, and 0x00 is MM's Ocarina of Time, so FcEquip_MmToOot(0x00)
            // returns 0x08 (OoT's ocarina) and every empty slot publishes as a real Ocarina of Time
            // -- which OoT then writes into its own ownedItems. That is the "half my items turned
            // into Ocarinas of Time" bug. Same guard as the cEquips/dEquips extract above and as
            // this array's own apply path; it was the one spot that was missed.
        } else if (nei->ownedItems[i] != 0xFF && nei->ownedItems[i] != 0x00 && nei->ownedItems[i] <= 0xFF) {
            // The > 0xFF guard is the EXT id space (0x02xx): those items do not exist on the OoT side
            // yet, so there is nothing to translate to — they stay local until they have a peer.
            // Only page-2 customs (0xB6..0xCF + wand 0xD0) are legitimate here; a cell poisoned with
            // anything else (see the apply side) must not be echoed to the peer.
            const uint8_t v = (uint8_t)nei->ownedItems[i];
            if ((v >= FC_OOT_PAGE2_FIRST + FC_PAGE2_MM_OFFSET && v <= FC_OOT_PAGE2_LAST + FC_PAGE2_MM_OFFSET) ||
                v == ITEM_ELEMENTAL_WAND) {
                canon = FcEquip_MmToOot(v); // 0xFF if unmappable
            }
        }
        ownedItems.push_back(canon);
    }
    sh["ownedItems"] = ownedItems;
    sh["tradeAdultOwned"] = nei->tradeAdultOwned;
    // Elemental Wand rods + Sheikah Slate runes: the cell item travels in ownedItems, but WHICH rods
    // / runes you own lives in these bitmasks, and nothing carried them -- the peer got an empty
    // wand. OR-merged both ways (one-way unlocks); the fcId deficit path still grants the items
    // natively, this just guarantees the bits arrive even if a grant is missed.
    sh["wandRodsOwned"] = (int)nei->wandRodsOwned;
    sh["slateRunesOwned"] = (int)nei->slateRunesOwned;
    RepairFlagOwnedCells(nei);
    // Shared NEI options/flags (FleetComboOptions.h). Table-driven so a future
    // option is one row there, not new code here. MAX rows never lose a value;
    // NEWEST rows let either game re-author the player's preference.
#define FCO_EXTRACT(key, field, mode) \
    sh[key] = (mode == FCO_MERGE_MAX) ? (uint8_t)std::max<int>(sh.value(key, 0), nei->field) : (uint8_t)nei->field;
    FC_COMBO_OPTION_TABLE(FCO_EXTRACT)
#undef FCO_EXTRACT
    // Merge with previous shared value (one-way unlocks, authored by both games via
    // cross-placement) so bits OoT published that we haven't applied yet aren't clobbered.
    // Publish the LIVE store, never "snapshot | store": in a resync `sh` IS the running snapshot, so
    // ORing against it rebroadcast every bit forever and the two games kept re-infecting each other
    // (a quest page filling itself with medallions the save never had). Anchor's model: publish
    // facts, and let ApplyShared OR incoming bits into the live store.
    sh["ootQuestItems"] = nei->ootQuestItems;
    sh["gsTokens"] = nei->ootGsCount;
    // mmQuestItems: authored from MM's NATIVE questItems (remains/songs/notebook). Plain OR-merge
    // with the previous shared value: every bit is a one-way unlock and BOTH games author bits via
    // cross-placement (OoT sets FC_MMQ bits for remains/songs found there), so never drop prev bits.
    sh["mmQuestItems"] = (MM_INV.questItems & (uint32_t)FC_MMQ_NATIVE_MASK); // live only — see above
    sh["comboObtained"] = nei->comboObtained;
    sh["comboObtainedFc"] = nei->comboObtainedFc; // fcId-indexed cross store (counts); comboAppliedFc is LOCAL only
    sh["comboTriforce"] = nei->comboTriforce;
    // Goal state (Beat Both Bosses). OR-merged both ways: a boss that fell stays fallen, and
    // neither world may end the run until it sees BOTH bits.
    sh["comboGoalFlags"] = nei->comboGoalFlags;
    sh["ootMasksOwned"] = nei->ootMasksOwned; // MM authors this (OoT echoes)

    // NOTE: cEquips/dEquips are deliberately NOT here. Button equips travel only at a game change —
    // see the ExtractEquips/ApplyEquips block below for why.

    sh["form"] = (int)gSaveContext.save.playerForm; // 0 FD .. 4 Human (same space as canonical)

    // Time Gate "adult mode" (OoT adult Link visual). Last-writer-wins toggle — do NOT OR-merge.
    // Bridges to OoT's gSaveContext.linkAge (adult == LINK_AGE_ADULT == 0) via the shared "adult" key.
    sh["adult"] = Nei_Save()->timeGateAdultMode != 0;
}

void ApplyShared(const nlohmann::json& sh) {
    NeiSaveData* nei = Nei_Save();
    HealLeakedOwnedItems();

    if (sh.contains("vitals")) {
        const auto& v = sh["vitals"];
        MM_PD.healthCapacity = (int16_t)v.value("healthCapacity", (int)MM_PD.healthCapacity);
        MM_PD.health = (int16_t)std::min<int>(v.value("health", (int)MM_PD.health), MM_PD.healthCapacity);
        // EVERY default here MUST be the current value, never 0: this block also runs for PARTIAL
        // deltas (FleetNet sends one leaf at a time), so a default of 0 would wipe magic and defense
        // hearts every time an unrelated vital -- a single rupee -- changed.
        // Double Defense is a one-way unlock: MAX-merge, so a peer snapshot taken before it received
        // its copy (or a resync from the inactive game) can never strip it again.
        MM_PD.doubleDefense = (uint8_t)std::max<int>(MM_PD.doubleDefense, v.value("doubleDefense", 0));
        MM_INV.defenseHearts = (int8_t)std::max<int>(MM_INV.defenseHearts, v.value("defenseHearts", 0));

        // Magic syncs as the two OWNERSHIP FLAGS only -- magicLevel is deliberately not copied.
        // magicLevel is not "how much magic you have", it is the handshake the game uses to build
        // the meter: z_parameter.c waits for (isMagicAcquired && magicLevel == 0), then sets
        // magicLevel = isDoubleMagicAcquired + 1 and steps magicCapacity up from zero. Copying the
        // peer's magicLevel = 1 skips that init, magicCapacity stays 0, and the bar never appears
        // even though you own the magic. So: take the flags, and whenever they GAIN something, clear
        // magicLevel so this game runs its own init next frame -- exactly what a native grant does.
        const bool hadMagic = MM_PD.isMagicAcquired != 0;
        const bool hadDouble = MM_PD.isDoubleMagicAcquired != 0;
        // MAX-merge (never lose): magic is never un-obtained in either game, so a peer snapshot that
        // still says "no magic" (it simply hasn't been granted its copy yet) must not strip the
        // meter this game just earned — that left MM "waiting" for a magic it already had.
        const bool hasMagic = hadMagic || v.value("isMagic", 0) != 0;
        const bool hasDouble = hadDouble || v.value("isDoubleMagic", 0) != 0;
        MM_PD.isMagicAcquired = hasMagic;
        MM_PD.isDoubleMagicAcquired = hasDouble;
        if ((hasMagic && !hadMagic) || (hasDouble && !hadDouble)) {
            MM_PD.magicLevel = 0; // re-run the native meter init (grows magicCapacity)
        }
        MM_PD.magic = (int8_t)v.value("magic", (int)MM_PD.magic);
        MM_PD.rupees = (int16_t)v.value("rupees", (int)MM_PD.rupees);
    }
    if (sh.contains("upgrades")) {
        const auto& u = sh["upgrades"];
        // These are UNCONDITIONAL writes, so the default matters twice over. This block also runs
        // for PARTIAL deltas, and a default of 0 would reset every upgrade the delta did not happen
        // to mention -- a wallet change alone would wipe the quiver, bomb bag, strength and scale.
        // Take the max as well: none of these ever decrease in either game, so a stale or
        // differently-scaled reading from the peer can never walk an upgrade backwards.
        auto upg = [&u](const char* key, int cur) { return std::max(cur, u.value(key, cur)); };
        Inventory_ChangeUpgrade(UPG_WALLET, upg("wallet", CUR_UPG_VALUE(UPG_WALLET)));
        // Wallet level 3 means 999 in OoT (its table) but only 500 in MM's vanilla table, or 5000 when
        // MM's own Tycoon option is on -- so the same shared wallet showed three different capacities
        // depending on where you looked. In the combo the wallet is ONE item: make MM's level 3 the
        // same 999 OoT uses, so the capacity (and the synced rupee count) mean the same thing on both
        // sides. Idempotent; only while a combo is running.
        if (FleetShipCombo_GetActiveGame() >= 0) {
            gUpgradeCapacities[UPG_WALLET][3] = 999; // variables.h
        }
        Inventory_ChangeUpgrade(UPG_QUIVER, upg("quiver", CUR_UPG_VALUE(UPG_QUIVER)));
        Inventory_ChangeUpgrade(UPG_BOMB_BAG, upg("bombBag", CUR_UPG_VALUE(UPG_BOMB_BAG)));
        Inventory_ChangeUpgrade(UPG_DEKU_STICKS, upg("sticks", CUR_UPG_VALUE(UPG_DEKU_STICKS)));
        Inventory_ChangeUpgrade(UPG_DEKU_NUTS, upg("nuts", CUR_UPG_VALUE(UPG_DEKU_NUTS)));
        Inventory_ChangeUpgrade(UPG_STRENGTH, upg("strength", CUR_UPG_VALUE(UPG_STRENGTH)));
        Inventory_ChangeUpgrade(UPG_SCALE, upg("scale", CUR_UPG_VALUE(UPG_SCALE)));
        // Mirror store for the OoT-side levels MM has no native slot for (nei->ootUpgrades).
        OotUpgSet(9, upg("strength", OotUpgGet(9)));
        OotUpgSet(12, upg("scale", OotUpgGet(12)));
        OotUpgSet(0, std::min(upg("bulletBag", OotUpgGet(0)), 3)); // cap 3: raw 7 reads capacities OOB
    }
    // Grab skill — latch on, never off (OR, not assign), so a peer snapshot taken before the pickup
    // cannot take it away again.
    if (sh.contains("canGrab") && sh["canGrab"].get<int>() != 0) {
        nei->ootCanGrab = 1;
    }
    if (sh.contains("weaponUpgrades")) {
        nei->weaponUpgrades |= (uint8_t)sh["weaponUpgrades"].get<int>();
        if (nei->weaponUpgrades & (1 << 4)) { // GFS bit -> native MM item
            if (MM_INV.items[kSlotGreatFairySword] == 0xFF) {
                MM_INV.items[kSlotGreatFairySword] = ITEM_SWORD_GREAT_FAIRY;
            }
        }
    }
    if (sh.contains("shieldOwned")) {
        uint16_t owned = (uint16_t)sh["shieldOwned"].get<int>();
        nei->shieldOwned |= owned;
        if (owned & FC_SHIELD_DIVINE)
            nei->extEquipOwnedBits |= (1u << 19);
        if (owned & FC_SHIELD_KITE)
            nei->extEquipOwnedBits |= (1u << 20);
        if (owned & FC_SHIELD_IKANA)
            nei->extEquipOwnedBits |= (1u << 21);
        int nibble = (MM_EQ.equipment >> 4) & 0xF;
        if ((owned & FC_SHIELD_IKANA) && nibble < 2) {
            MM_EQ.equipment = (uint16_t)((MM_EQ.equipment & ~0xF0) | (2 << 4)); // own MM Mirror
        } else if ((owned & FC_SHIELD_HYLIAN) && nibble < 1) {
            MM_EQ.equipment = (uint16_t)((MM_EQ.equipment & ~0xF0) | (1 << 4)); // own Hero
        }
    }
    if (sh.contains("equippedShield")) {
        SetEquippedShieldCanonical(sh["equippedShield"].get<int>());
    }
    // Upgrade-column equipment — additive, never cleared (mirror of the extract side above).
    if (sh.value("capeOwned", false)) {
        ExtEquip_GiveCape();
    }
    if (sh.value("pendantOwned", false)) {
        ExtEquip_GivePendant();
    }
    if (sh.contains("swordFlags")) {
        const auto& s = sh["swordFlags"];
        if (s.value("master", false))
            nei->comboObtained[FC_OOT_SWORD_MASTER] = 1;
        if (s.value("biggoron", false))
            nei->comboObtained[FC_OOT_SWORD_BIGGORON] = 1;
        if (s.value("kokiri", false)) {
            int nibble = MM_EQ.equipment & 0xF;
            int target = 1 + KokiriChainLevel(); // 1 kokiri / 2 razor / 3 gilded
            if (nibble < target) {
                MM_EQ.equipment = (uint16_t)((MM_EQ.equipment & ~0xF) | target);
            }
        }
    }
    if (sh.contains("equippedSword")) {
        int sw = sh["equippedSword"].get<int>();
        if (sw == 1) {
            int target = 1 + KokiriChainLevel();
            MM_EQ.equipment = (uint16_t)((MM_EQ.equipment & ~0xF) | target);
        } else if (sw == 4) {
            MM_EQ.equipment = (uint16_t)((MM_EQ.equipment & ~0xF) | 4);
        }
        // 2 (master) / 3 (bgs): no MM equip nibble -> keep current
    }

    if (sh.contains("inv")) {
        const auto& inv = sh["inv"];
        ApplyInvItem(inv, "stick", kSlotStick, ITEM_DEKU_STICK, true);
        ApplyInvItem(inv, "nut", kSlotNut, ITEM_DEKU_NUT, true);
        ApplyInvItem(inv, "bomb", kSlotBomb, ITEM_BOMB, true);
        ApplyInvItem(inv, "bow", kSlotBow, ITEM_BOW, true);
        ApplyInvItem(inv, "bombchu", kSlotBombchu, ITEM_BOMBCHU, true);
        ApplyInvItem(inv, "fireArrow", kSlotArrowFire, ITEM_ARROW_FIRE, false);
        ApplyInvItem(inv, "iceArrow", kSlotArrowIce, ITEM_ARROW_ICE, false);
        ApplyInvItem(inv, "lightArrow", kSlotArrowLight, ITEM_ARROW_LIGHT, false);
        ApplyInvItem(inv, "lens", kSlotLens, ITEM_LENS_OF_TRUTH, false);
        ApplyInvItem(inv, "beans", kSlotBeans, ITEM_MAGIC_BEANS, true);
        if (inv.value("ocarinaTime", false)) {
            MM_INV.items[kSlotOcarina] = ITEM_OCARINA_OF_TIME;
        } else if (inv.value("ocarinaFairy", false) && MM_INV.items[kSlotOcarina] == 0xFF) {
            MM_INV.items[kSlotOcarina] = ITEM_OCARINA_FAIRY;
        }
        int hookLevel = inv.value("hookshotLevel", 0);
        if (hookLevel > nei->ootHookshotLevel) {
            nei->ootHookshotLevel = (uint8_t)hookLevel;
        }
        if (hookLevel >= 1 && MM_INV.items[kSlotHookshot] == 0xFF) {
            MM_INV.items[kSlotHookshot] = ITEM_HOOKSHOT;
        }
        if (inv.value("clawshot", false)) {
            nei->clawshotOwned = 1;
            nei->twilightUpgrade |= 0x1; // TWILIGHT_UPGRADE_CLAWSHOT: gates the L-tap selector
                                         // (same companion flag Nei_GiveAllOotItems sets)
        }
        if (inv.value("boomerang", false))
            nei->ootBoomerangOwned = 1;
        if (inv.value("hammer", false))
            nei->ootHammerOwned = 1;
        if (inv.value("dins", false))
            nei->ootSpellsOwned |= (1 << 0);
        if (inv.value("farores", false))
            nei->ootSpellsOwned |= (1 << 1);
        if (inv.value("nayrus", false))
            nei->ootSpellsOwned |= (1 << 2);
        if (inv.value("rocsFeatherVanilla", false))
            nei->ootSpellsOwned |= (1 << 3);
        if (inv.value("slingshot", false))
            nei->slingshotOwned = 1;
        if (inv.contains("slingshotAmmo"))
            nei->slingshotSeeds = (uint8_t)inv["slingshotAmmo"].get<int>();
        if (inv.value("pictobox", false)) {
            if (MM_INV.items[kSlotPictograph] == 0xFF) {
                MM_INV.items[kSlotPictograph] = ITEM_PICTOGRAPH_BOX;
            }
        }
        if (inv.value("powderKeg", false)) {
            nei->powerKegOwned = 1;
            if (MM_INV.items[kSlotPowderKeg] == 0xFF) {
                MM_INV.items[kSlotPowderKeg] = ITEM_POWDER_KEG;
            }
        }
        if (inv.contains("powderKegCount")) {
            int c = std::min(inv["powderKegCount"].get<int>(), 5);
            if (c > nei->powerKegCount)
                nei->powerKegCount = (uint8_t)c;
            if (MM_INV.items[kSlotPowderKeg] != 0xFF && c > MM_INV.ammo[kSlotPowderKeg]) {
                MM_INV.ammo[kSlotPowderKeg] = (int8_t)c;
            }
        }
        if (inv.value("net", false))
            nei->netEquipped = 1;
        if (inv.contains("bottomlessMode"))
            nei->bottomlessBottleMode = (uint8_t)inv["bottomlessMode"].get<int>();
        if (inv.contains("bottomlessContent")) {
            uint8_t bt = FcBottle_OotToMm((uint8_t)inv["bottomlessContent"].get<int>());
            nei->bottomlessContent = (bt == FC_BOTTLE_UNMAPPED) ? 0x12 : bt;
        }
        if (inv.contains("bottomlessCount"))
            nei->bottomlessCount = (uint8_t)inv["bottomlessCount"].get<int>();
    }

    if (sh.contains("bottleSlots") && sh["bottleSlots"].is_array()) {
        for (int i = 0; i < 8 && i < (int)sh["bottleSlots"].size(); i++) {
            uint8_t t = FcBottle_OotToMm((uint8_t)sh["bottleSlots"][i].get<int>());
            if (t == FC_BOTTLE_UNMAPPED) {
                t = 0x12; // MM ITEM_BOTTLE: keep an empty bottle rather than a foreign id
            }
            // Applied VERBATIM, empties included. The old "an empty slot never clears a full one"
            // guard was what resurrected consumed contents: the game that drank refused to let the
            // peer's stale copy clear it -- but then its own state disagreed with the snapshot it had
            // folded, so it republished the full bottle and the drinker got the potion back. Bottles
            // are now a VOLATILE leaf (NetIsVolatileLeaf): only the active game publishes them, so
            // there is no stale echo left to guard against.
            nei->bottleSlots[i] = t;
        }
    }
    if (sh.contains("ownedItems") && sh["ownedItems"].is_array()) {
        for (int i = 0; i < 48 && i < (int)sh["ownedItems"].size(); i++) {
            const int raw = sh["ownedItems"][i].get<int>();
            // 0xFF = empty; 0x00 = uninitialized slot (no custom item / MM mask is ever id 0). Both
            // must be skipped, else a raw 0x00 translates to a spurious page-2/mask item. Anything
            // above 0xFF is an EXT (u16) id: those never travel through this u8 canonical array (the
            // fcId registry carries them), and truncating one to a byte here used to turn the
            // Sheikah Slate into a random bottled content. Skip it whole.
            if (raw == 0xFF || raw == 0x00 || raw > 0xFF || raw < 0) {
                continue;
            }
            const uint8_t v = (uint8_t)raw;
            if (i >= 24) {
                // Canonical mask entry -> grant MM's NATIVE mask (never store in ownedItems).
                if (v >= FC_OOT_MM_MASK_ITEM_BASE && v < FC_OOT_MM_MASK_ITEM_BASE + FC_MM_MASK_COUNT &&
                    MM_INV.items[FC_MM_MASK_SLOT_BASE + (v - FC_OOT_MM_MASK_ITEM_BASE)] == 0xFF) {
                    MM_INV.items[FC_MM_MASK_SLOT_BASE + (v - FC_OOT_MM_MASK_ITEM_BASE)] =
                        kFcMmMaskItemBySlot[v - FC_OOT_MM_MASK_ITEM_BASE];
                }
            } else {
                // Page-2 cells only ever hold page-2 customs (0xB6..0xCF, + the wand at 0xD0). A
                // translation that lands anywhere else (FcEquip_OotToMm also maps page-1 items and
                // BOTTLE CONTENTS, which is how a truncated Slate id became a bottled Poe in a cell)
                // is garbage for this array and must not be stored.
                uint8_t mm = FcEquip_OotToMm(v); // page-2 translate (+0x18); 0xFF if unmappable
                const bool page2 =
                    (mm >= FC_OOT_PAGE2_FIRST + FC_PAGE2_MM_OFFSET && mm <= FC_OOT_PAGE2_LAST + FC_PAGE2_MM_OFFSET) ||
                    mm == ITEM_ELEMENTAL_WAND;
                if (page2)
                    nei->ownedItems[i] = mm; // peer ids are u8; EXT ids never arrive here
            }
        }
    }
    // Diagnostic for "the Title Deed obtained in OoT never shows up in MM": tells us in ONE run
    // whether this is a transport problem (key absent / null after unflatten) or a display problem
    // (bit arrives fine and the kaleido is at fault). Logged once per apply, cheap. Skijer 2026-07-30
    // A partial delta legitimately carries only the keys that changed, so an ABSENT key is normal and
    // is not logged — warning on it floods the log at delta rate and buries the real events.
    // Shared NEI options/flags (FleetComboOptions.h) — mirror of the extract above.
#define FCO_APPLY(key, field, mode)                        \
    if (sh.contains(key) && sh[key].is_number_integer()) { \
        uint8_t v = (uint8_t)sh[key].get<int>();           \
        if (mode == FCO_MERGE_MAX) {                       \
            if (v > nei->field) {                          \
                nei->field = v;                            \
            }                                              \
        } else {                                           \
            nei->field = v;                                \
        }                                                  \
    }
    FC_COMBO_OPTION_TABLE(FCO_APPLY)
#undef FCO_APPLY

    // Wand rods / slate runes: OR the bits in, and hand the game every rod/rune it did not have yet
    // through its own grant function (which also places the cell item and picks the active mode).
    if (sh.contains("wandRodsOwned") && sh["wandRodsOwned"].is_number_integer()) {
        const uint8_t incoming = (uint8_t)sh["wandRodsOwned"].get<int>();
        const uint8_t gained = (uint8_t)(incoming & ~nei->wandRodsOwned);
        for (uint8_t m = 0; m < 6 && gained != 0; m++) {
            if (gained & (1 << m)) {
                Wand_GrantMode(m);
            }
        }
        nei->wandRodsOwned |= incoming;
    }
    if (sh.contains("slateRunesOwned") && sh["slateRunesOwned"].is_number_integer()) {
        const uint8_t incoming = (uint8_t)sh["slateRunesOwned"].get<int>();
        const uint8_t gained = (uint8_t)(incoming & ~nei->slateRunesOwned);
        for (uint8_t r = 0; r < 4 && gained != 0; r++) {
            if (gained & (1 << r)) {
                Slate_GrantRune(r);
            }
        }
        nei->slateRunesOwned |= incoming;
    }

    RepairFlagOwnedCells(nei);
    if (sh.contains("tradeAdultOwned")) {
        if (sh["tradeAdultOwned"].is_null()) {
            // unflatten() fills gaps in a sparse index set with null (see the note further down); the
            // null guard is what keeps .get<uint32_t>() from throwing here — and a throw is not local,
            // it is caught way out at the delta level and ABORTS THE WHOLE REMAINING APPLY.
            SPDLOG_WARN("[FleetSync] trade: key present but NULL (own=0x{:08X})", nei->tradeAdultOwned);
        } else {
            uint32_t incoming = sh["tradeAdultOwned"].get<uint32_t>();
            SPDLOG_INFO("[FleetSync] trade: incoming=0x{:08X} own=0x{:08X} -> 0x{:08X}", incoming, nei->tradeAdultOwned,
                        nei->tradeAdultOwned | incoming);
            nei->tradeAdultOwned |= incoming;
        }
    }

    // Reconcile the two stores that both describe "you own this MM trade item".
    //
    //   tradeAdultOwned  — the wheel bitmask, set by TradeAdult_GiveItem in OoT.
    //   comboObtainedFc  — the fcId counter, set by Randomizer_Item_Give's record hook, and the ONLY
    //                      thing ApplyFcRegistryToNatives reads to actually grant the native MM item.
    //
    // They are written by different code paths and can disagree. They DID disagree in every save made
    // before 2026-07-30: comboObtainedFc is a 512-entry array, so a delta touching one index unflattens
    // to a mostly-null array, ApplyShared threw on the first null and aborted — the counter never
    // arrived, while tradeAdultOwned (a plain scalar) did. Result: the wheel bit was set, the native
    // item was never granted, and a Title Deed earned in OoT was invisible in MM.
    //
    // NetFillNullGaps fixes the transport going forward; this heals saves already in that state and
    // keeps the two stores from drifting again. Only the 8 MM-native trade items — the OoT-side entries
    // have no MM relative and are handled by the wheel's index cursor. Skijer 2026-07-30
    {
        static const struct {
            int tradeIndex;
            int fcId;
        } kMmTradeFc[] = {
            { 11, FCI_MM_MOONS_TEAR },      { 12, FCI_MM_DEED_LAND },      { 13, FCI_MM_DEED_SWAMP },
            { 14, FCI_MM_DEED_MOUNTAIN },   { 15, FCI_MM_DEED_OCEAN },     { 16, FCI_MM_ROOM_KEY },
            { 17, FCI_MM_LETTER_TO_KAFEI }, { 18, FCI_MM_LETTER_TO_MAMA },
        };
        for (const auto& e : kMmTradeFc) {
            bool ownedByWheel = (nei->tradeAdultOwned & (1u << e.tradeIndex)) != 0;
            if (ownedByWheel && e.fcId < FC_COMBO_OBTAINED_FC_SIZE && nei->comboObtainedFc[e.fcId] == 0) {
                nei->comboObtainedFc[e.fcId] = 1; // ApplyFcRegistryToNatives grants the native item next tick
                SPDLOG_INFO("[FleetSync] trade: healed fcId={} from wheel bit {} (native grant pending)", e.fcId,
                            e.tradeIndex);
            }
        }
    }
    if (sh.contains("ootQuestItems"))
        nei->ootQuestItems |= sh["ootQuestItems"].get<uint32_t>();
    if (sh.contains("gsTokens")) {
        int gs = sh["gsTokens"].get<int>();
        if (gs > nei->ootGsCount)
            nei->ootGsCount = (uint16_t)gs;
    }
    if (sh.contains("mmQuestItems")) {
        MM_INV.questItems |= (sh["mmQuestItems"].get<uint32_t>() & (uint32_t)FC_MMQ_NATIVE_MASK);
    }
    if (sh.contains("ootMasksOwned"))
        nei->ootMasksOwned |= (uint16_t)sh["ootMasksOwned"].get<int>();
    if (sh.contains("comboObtained") && sh["comboObtained"].is_array()) {
        for (int i = 0; i < FC_COMBO_OBTAINED_SIZE && i < (int)sh["comboObtained"].size(); i++) {
            uint8_t v = (uint8_t)sh["comboObtained"][i].get<int>();
            if (v > nei->comboObtained[i])
                nei->comboObtained[i] = v;
        }
    }
    if (sh.contains("comboObtainedFc") && sh["comboObtainedFc"].is_array()) {
        for (int i = 0; i < FC_COMBO_OBTAINED_FC_SIZE && i < (int)sh["comboObtainedFc"].size(); i++) {
            uint8_t v = (uint8_t)sh["comboObtainedFc"][i].get<int>();
            if (v > nei->comboObtainedFc[i])
                nei->comboObtainedFc[i] = v;
        }
    }
    if (sh.contains("comboGoalFlags")) {
        nei->comboGoalFlags |= (uint8_t)sh["comboGoalFlags"].get<int>();
    }
    if (sh.contains("comboTriforce")) {
        uint16_t tf = (uint16_t)sh["comboTriforce"].get<int>();
        if (tf > nei->comboTriforce)
            nei->comboTriforce = tf;
    }
    ApplyRegistryToNatives();

    // NOTE: cEquips/dEquips are deliberately NOT applied here — see ApplyEquips below.

    if (sh.contains("form")) {
        int form = sh["form"].get<int>();
        if (form >= 0 && form <= 4) {
            gSaveContext.save.playerForm = (s8)form;
            switch (form) {
                case PLAYER_FORM_FIERCE_DEITY:
                    gSaveContext.save.equippedMask = PLAYER_MASK_FIERCE_DEITY;
                    break;
                case PLAYER_FORM_GORON:
                    gSaveContext.save.equippedMask = PLAYER_MASK_GORON;
                    break;
                case PLAYER_FORM_ZORA:
                    gSaveContext.save.equippedMask = PLAYER_MASK_ZORA;
                    break;
                case PLAYER_FORM_DEKU:
                    gSaveContext.save.equippedMask = PLAYER_MASK_DEKU;
                    break;
                default:
                    gSaveContext.save.equippedMask = PLAYER_MASK_NONE;
                    break;
            }
        }
    }

    // Time Gate "adult mode" — mirror OoT's linkAge into our persistent flag (last-writer-wins). The
    // next Player_Draw calls AdultLink_ShouldHide (lazy setup), so no explicit re-init is needed here.
    if (sh.contains("adult")) {
        Nei_Save()->timeGateAdultMode = sh["adult"].get<bool>() ? 1 : 0;
    }

    // [FleetSyncAudit] one line per full-state apply: what the peer SAID and what we HAVE now for
    // the fields people report as "not crossing". Compare this line on both sides of a hand-over.
    if (sh.contains("vitals") && sh.contains("upgrades") && sh.contains("inv")) {
        const auto& v = sh["vitals"];
        const auto& u = sh["upgrades"];
        SPDLOG_INFO("[FleetSyncAudit] in: hp={}/{} magic={} isMagic={} dbl={} rupees={} wallet={} str={} wand={:#x} "
                    "slate={:#x} | now: hp={}/{} isMagic={} dbl={} wallet={} wand={:#x} slate={:#x} "
                    "bottles=[{},{},{},{},{},{},{},{}]",
                    v.value("health", -1), v.value("healthCapacity", -1), v.value("magic", -1), v.value("isMagic", -1),
                    v.value("isDoubleMagic", -1), v.value("rupees", -1), u.value("wallet", -1), u.value("strength", -1),
                    sh.value("wandRodsOwned", -1), sh.value("slateRunesOwned", -1), (int)MM_PD.health,
                    (int)MM_PD.healthCapacity, (int)MM_PD.isMagicAcquired, (int)MM_PD.isDoubleMagicAcquired,
                    (int)CUR_UPG_VALUE(UPG_WALLET), (int)nei->wandRodsOwned, (int)nei->slateRunesOwned,
                    (int)nei->bottleSlots[0], (int)nei->bottleSlots[1], (int)nei->bottleSlots[2],
                    (int)nei->bottleSlots[3], (int)nei->bottleSlots[4], (int)nei->bottleSlots[5],
                    (int)nei->bottleSlots[6], (int)nei->bottleSlots[7]);
    }
}

// =================================================================================================
// BUTTON EQUIPS — game-change only, never through FleetNet
// =================================================================================================
// C / D-pad equips used to be ordinary shared state: published by BOTH games ~3x a second and re-sent
// whole on every resync. That made them a one-way ratchet in OoT's favour, and it is exactly what the
// "no matter what I equip in MM, it forces an OoT item onto the button" report was. The loop:
//   - We publish 0xFF for anything OoT cannot represent — any ITEM_EXT_BUTTON custom (0xFB, i.e. every
//     NEI item), the pictobox, the Great Fairy's Sword, and even the Ocarina of Time, whose MM id is
//     the 0x00 that the old extract had to treat as "empty" because it could not tell it from a zeroed
//     byte.
//   - OoT reads that 0xFF as "keep mine", so it never changes — and keeps publishing its own id.
//   - We obey that id and stamp it over the button the player had just set.
// The two snapshots could then never agree either, so the hash verifier kept firing full resyncs,
// which re-stamped the whole set at moments that looked random to the player.
//
// So they are OUT of ExtractShared/ApplyShared entirely: FleetNet neither sends nor applies them, and
// NetResetBaseline strips them from the snapshot so they don't even reach the hash. They travel ONCE,
// in the departure temp file, and the arriving game inherits them under three rules:
//   1. 0xFF from the peer means "this button held something I could not express" -> keep ours.
//   2. A local button holding something the PEER cannot express was put there by the player, in this
//      game, on purpose -> keep it. Only empty or translatable buttons may be replaced.
//   3. Never equip an item this save does not own, and always write the button's SLOT next to its id.
//      An id without its cButtonSlots/dpadSlots entry is a phantom button the rest of the game reads
//      inconsistently — that mismatch is the other half of "it changed the item I had equipped".

// Canonical (OoT) id for one MM button, or 0xFF when OoT has no way to hold what is on it.
// 0x00 is BOTH MM's Ocarina of Time and the value any zeroed byte reads as, so the button's SLOT is
// what disambiguates it: a real ocarina equip points at kSlotOcarina and that slot is filled.
uint8_t MmButtonCanonical(uint8_t item, uint8_t slot) {
    if (item == 0xFF || item == ITEM_EXT_BUTTON) {
        return 0xFF; // empty, or a custom (u16) item that only exists on this side
    }
    if (item == ITEM_OCARINA_OF_TIME) { // 0x00
        return (slot == kSlotOcarina && MM_INV.items[kSlotOcarina] != 0xFF) ? FcEquip_MmToOot(item) : 0xFF;
    }
    return FcEquip_MmToOot(item);
}

void ExtractEquips(nlohmann::json& sh) {
    nlohmann::json c = nlohmann::json::array();
    for (int b = 1; b <= 3; b++) {
        c.push_back(MmButtonCanonical(MM_EQ.buttonItems[CUR_FORM][b], MM_EQ.cButtonSlots[CUR_FORM][b]));
    }
    sh["cEquips"] = c;
    nlohmann::json d = nlohmann::json::array();
    for (int b = 0; b < 4; b++) {
        d.push_back(MmButtonCanonical(gSaveContext.save.shipSaveInfo.dpadEquips.dpadItems[CUR_FORM][b],
                                      gSaveContext.save.shipSaveInfo.dpadEquips.dpadSlots[CUR_FORM][b]));
    }
    sh["dEquips"] = d;
}

// Inventory slot holding `item`, or -1 if this save does not have it. The ocarina is looked up by its
// own slot rather than by scanning: its id is 0x00, so a scan would match the first zeroed byte.
int FindInvSlot(uint8_t item) {
    if (item == ITEM_OCARINA_OF_TIME) {
        return (MM_INV.items[kSlotOcarina] != 0xFF) ? (int)kSlotOcarina : -1;
    }
    for (int s = 0; s < FC_MM_MASK_SLOT_BASE + FC_MM_MASK_COUNT; s++) {
        if (MM_INV.items[s] == item) {
            return s;
        }
    }
    return -1;
}

// itemDst / slotDst: the button's id byte and its inventory-slot byte (vanilla C or Ship D-pad).
// extDst: the parallel u16 that only means anything while itemDst holds ITEM_EXT_BUTTON (NULL for the
// D-pad, which has no such shadow).
void ApplyOneButton(uint8_t* itemDst, uint8_t* slotDst, uint16_t* extDst, uint8_t canon) {
    if (canon == 0xFF) {
        return; // rule 1
    }
    const uint8_t cur = *itemDst;
    // A raw 0x00 sitting on a button that does NOT point at the ocarina slot is a zeroed byte, not an
    // equip — treat it as empty so it can be replaced instead of being protected as if it were loot.
    const bool curEmpty = (cur == 0xFF) || (cur == ITEM_OCARINA_OF_TIME && *slotDst != kSlotOcarina);
    if (!curEmpty && MmButtonCanonical(cur, *slotDst) == 0xFF) {
        return; // rule 2: the player put something OoT cannot hold here — don't take it away
    }
    const uint8_t mm = FcEquip_OotToMm(canon);
    if (mm == 0xFF) {
        return; // no MM relative for what they had
    }
    const int slot = FindInvSlot(mm);
    if (slot < 0) {
        return; // rule 3: we don't own it
    }
    *itemDst = mm;
    *slotDst = (uint8_t)slot;
    if (extDst != NULL) {
        *extDst = 0; // the u16 shadow is meaningless now that this button holds a plain u8 id
    }
}

void ApplyEquips(const nlohmann::json& sh) {
    if (sh.contains("cEquips") && sh["cEquips"].is_array()) {
        for (int i = 0; i < 3 && i < (int)sh["cEquips"].size(); i++) {
            if (!sh["cEquips"][i].is_number_integer()) {
                continue;
            }
            const int b = 1 + i;
            ApplyOneButton(&MM_EQ.buttonItems[CUR_FORM][b], &MM_EQ.cButtonSlots[CUR_FORM][b],
                           &gSaveContext.save.shipSaveInfo.extButtons.items[CUR_FORM][b],
                           (uint8_t)sh["cEquips"][i].get<int>());
        }
    }
    if (sh.contains("dEquips") && sh["dEquips"].is_array()) {
        for (int i = 0; i < 4 && i < (int)sh["dEquips"].size(); i++) {
            if (!sh["dEquips"][i].is_number_integer()) {
                continue;
            }
            ApplyOneButton(&gSaveContext.save.shipSaveInfo.dpadEquips.dpadItems[CUR_FORM][i],
                           &gSaveContext.save.shipSaveInfo.dpadEquips.dpadSlots[CUR_FORM][i], NULL,
                           (uint8_t)sh["dEquips"][i].get<int>());
        }
    }
}

// ---------------------------------------------------------------------------------------------
// Save sync + registration
// ---------------------------------------------------------------------------------------------
unsigned long long sLastSeenSyncSeq = 0;
bool sSyncSeqInit = false;
unsigned long long sWaitingAckSeq = 0;
bool sTitleDeleteDone = false;

void RefreshSharedInTemp() {
    nlohmann::json temp;
    ReadTemp(temp);
    nlohmann::json sh = temp.contains("shared") ? temp["shared"] : nlohmann::json::object();
    try {
        ExtractShared(sh); // may throw on rando/FC fields a fresh check populated (guarded so no terminate)
    } catch (const std::exception& e) {
        SPDLOG_ERROR("[FleetSync] ExtractShared threw on save: {}", e.what());
    } catch (...) { SPDLOG_ERROR("[FleetSync] ExtractShared threw a non-std exception on save"); }
    temp["version"] = 1;
    temp["slot"] = gSaveContext.fileNum;
    temp["shared"] = sh;
    WriteTemp(temp);
}

// Frozen-save: write our slot to disk using the AutoSave owl-save recipe. The queued flash write
// is pumped by Play_UpdateMain (which runs even while combo-frozen).
void SaveOwnSlotFrozen() {
    if (gPlayState == NULL) {
        return; // not in gameplay (e.g. still at file select): nothing to persist
    }
    bool prevOwl = gSaveContext.save.isOwlSave;
    gSaveContext.save.isOwlSave = true;
    Play_SaveCycleSceneFlags(gPlayState);
    gSaveContext.save.saveInfo.playerData.savedSceneId = gPlayState->sceneId;
    // Parked in the waiting room, the file must still say where the player REALLY is: swap the
    // real entrance/scene in for the copy into the flash buffer, then put the room back (we are
    // still parked). No-op when not parked.
    FleetShipCombo_LimboSaveShadowBegin();
    func_8014546C(&gPlayState->sramCtx); // copies gSaveContext into the flash buffer
    FleetShipCombo_LimboSaveShadowEnd();
    Sram_SetFlashPagesOwlSave(&gPlayState->sramCtx,
                              gFlashOwlSaveStartPages[gSaveContext.fileNum * FLASH_SAVE_MAIN_MULTIPLIER],
                              gFlashOwlSaveNumPages[gSaveContext.fileNum * FLASH_SAVE_MAIN_MULTIPLIER]);
    Sram_StartWriteToFlashOwlSave(&gPlayState->sramCtx);
    gSaveContext.save.isOwlSave = prevOwl;
}

// =================================================================================================
// FleetNet - continuous Anchor-style state sync over the shared-memory packet rings
// =================================================================================================
// The temp-file handshake above only fires on a SAVE or a game CHANGE. FleetNet keeps the two games
// agreeing the whole time, and it does it WITHOUT a second translator: ExtractShared/ApplyShared
// already map live state <-> the canonical "shared" json, so we just run them continuously and send
// what moved.
//
//   scan  -> ExtractShared into our running snapshot, flatten() it to JSON-pointer leaves
//            ("/vitals/health" -> 16), diff against what we last published, send the changed leaves.
//   apply -> unflatten() the leaves into a PARTIAL shared object and hand it to ApplyShared, which
//            is already partial-safe (every field is contains()/value() guarded with the current
//            value as default), then fold them into our snapshot so we never echo them back.
//   verify-> every few seconds each side sends a hash of its snapshot. Deltas can be lost (ring
//            overrun while a game is mid scene-load), so this is what guarantees convergence:
//            a repeated mismatch triggers a full resend. Without it a single dropped packet would
//            desync the two games permanently.
//
// This block is TEXTUALLY IDENTICAL in Ship and 2ship -- both sides speak the canonical schema, so
// neither needs to know which game it is.

constexpr int kNetScanPeriod = 20;       // frames between delta scans (~3x/sec)
constexpr int kNetVerifyPeriod = 300;    // frames between hash reports (~5s)
constexpr int kNetResyncCooldown = 900;  // min frames between full resends (~15s), anti-loop
constexpr size_t kNetBatchBytes = 3800;  // leave room for the {"op":"delta","d":{}} envelope in 4095
constexpr int kNetMaxDrainPerFrame = 64; // hard cap on packets applied per frame (anti hang: the
                                         // peer can refill the ring while we drain it)

nlohmann::json sNetShared = nlohmann::json::object(); // running canonical state (unflattened)

// Repair the null gaps that unflatten() leaves behind, BEFORE ApplyShared ever sees them.
//
// A delta carries only the leaves that changed ("/x/3": 7). unflatten() has to materialise the whole
// container to place index 3, so it emits [null, null, null, 7] — the untouched indices become JSON
// null. ApplyShared then calls .get<>() on one and nlohmann throws type_error.302. That throw is NOT
// local: it is caught out at the delta level, which ABORTS THE ENTIRE REMAINING APPLY and silently
// drops every field after it. The 2026-07-30 logs show 69 of these in a single session, i.e. the
// second half of the shared state was being thrown away over and over.
//
// A null here means "this leaf was not in the packet" = UNCHANGED, not "clear it". So fill it from the
// running canonical state. Anything still null afterwards (no canonical value yet) is erased from
// objects; array slots keep their null, since erasing would shift every later index, and callers must
// treat a null array slot as "no value". Skijer 2026-07-30
static void NetFillNullGaps(nlohmann::json& dst, const nlohmann::json& base) {
    if (dst.is_array()) {
        for (size_t i = 0; i < dst.size(); i++) {
            bool haveBase = base.is_array() && (i < base.size());
            if (dst[i].is_null()) {
                if (haveBase) {
                    dst[i] = base[i];
                }
            } else if (haveBase) {
                NetFillNullGaps(dst[i], base[i]);
            }
        }
    } else if (dst.is_object()) {
        for (auto it = dst.begin(); it != dst.end();) {
            bool haveBase = base.is_object() && base.contains(it.key());
            if (it.value().is_null()) {
                if (haveBase && !base[it.key()].is_null()) {
                    it.value() = base[it.key()];
                    ++it;
                } else {
                    it = dst.erase(it); // no canonical value — drop the key so .get<>() is never reached
                }
            } else {
                if (haveBase) {
                    NetFillNullGaps(it.value(), base[it.key()]);
                }
                ++it;
            }
        }
    }
}
nlohmann::json sNetFlat = nlohmann::json::object(); // its flattened form = what the peer has
int sNetScanTick = 0;
int sNetVerifyTick = 0;
int sNetResyncCooldownLeft = 0;
int sNetMismatchStreak = 0;
int sNetFutileResyncs = 0; // consecutive resyncs that did NOT make the hashes agree
bool sNetPrimed = false;   // first scan publishes nothing: it only establishes the baseline

// ---- DIAGNOSTIC: "everything turned into an Ocarina of Time" watcher ----
// In MM, ITEM_OCARINA_OF_TIME is 0x00 -- the same value an uninitialized or zeroed byte reads as.
// That means ANY bug that writes a stray zero into an item slot shows up in game as an Ocarina of
// Time, which is why the symptom keeps reappearing from different causes. Rather than guess again,
// this watcher snapshots the slots that matter and reports the FIRST moment one of them turns into
// an ocarina, tagged with the phase that did it. Cheap (72 byte compares) and silent until it
// fires; remove once the culprit is fixed.
uint8_t sPoisonPrev[FC_MM_MASK_SLOT_BASE + FC_MM_MASK_COUNT];
uint16_t sPoisonPrevOwned[48]; // u16: ownedItems widened
bool sPoisonInit = false;

// Companion to NetPoisonScan: dumps the raw slot bytes instead of only reporting transitions.
// The watcher can only see a CHANGE to 0x00, so if the slots were already zero before its first
// call it stays silent forever -- which is exactly what happened. This shows the actual state at
// each step of the arrival, so we can see the precise moment the inventory becomes all-ocarina.
void NetPoisonDump(const char* phase) {
    const int nItems = FC_MM_MASK_SLOT_BASE + FC_MM_MASK_COUNT;
    std::string items;
    int zeros = 0;
    for (int i = 0; i < nItems; i++) {
        const uint8_t v = MM_INV.items[i];
        if (v == 0x00) {
            zeros++;
        }
        char b[8];
        snprintf(b, sizeof(b), "%02X ", v);
        items += b;
    }
    SPDLOG_WARN("[Poison] DUMP {} | zeros(=ocarina)={} | items: {}", phase, zeros, items);
}

void NetPoisonScan(const char* phase) {
    const int nItems = FC_MM_MASK_SLOT_BASE + FC_MM_MASK_COUNT; // vanilla slots + the 24 mask slots
    NeiSaveData* nei = Nei_Save();
    if (!sPoisonInit) {
        sPoisonInit = true;
        for (int i = 0; i < nItems; i++) {
            sPoisonPrev[i] = MM_INV.items[i];
        }
        for (int i = 0; i < 48; i++) {
            sPoisonPrevOwned[i] = nei->ownedItems[i];
        }
        return;
    }
    for (int i = 0; i < nItems; i++) {
        const uint8_t now = MM_INV.items[i];
        const uint8_t was = sPoisonPrev[i];
        // kSlotOcarina is the ONE slot where 0x00 is legitimate.
        if (now != was && now == ITEM_OCARINA_OF_TIME && i != kSlotOcarina) {
            SPDLOG_ERROR("[Poison] {}: MM_INV.items[{}] {:#04x} -> OCARINA (0x00){}", phase, i, was,
                         i >= FC_MM_MASK_SLOT_BASE ? "  [MASK SLOT]" : "  [vanilla slot]");
        }
        sPoisonPrev[i] = now;
    }
    for (int i = 0; i < 48; i++) {
        const uint16_t now = nei->ownedItems[i];
        const uint16_t was = sPoisonPrevOwned[i];
        if (now != was && now == 0x00) {
            SPDLOG_ERROR("[Poison] {}: nei->ownedItems[{}] {:#04x} -> 0x00", phase, i, was);
        }
        sPoisonPrevOwned[i] = now;
    }
}

// FNV-1a over the dumped snapshot. nlohmann objects are key-sorted, so the dump -- and therefore
// the hash -- is order-independent on both sides.
uint64_t NetHash(const nlohmann::json& flat) {
    const std::string s = flat.dump();
    uint64_t h = 1469598103934665603ull;
    for (unsigned char c : s) {
        h ^= c;
        h *= 1099511628211ull;
    }
    return h;
}

void NetSend(const nlohmann::json& j) {
    FleetShipCombo_PushPacket(j.dump().c_str());
}

// A delta carries two kinds of change, and the split is load-bearing.
//
// Scalars travel as flattened JSON-pointer leaves ("/vitals/rupees": 40), which ApplyShared can
// take partially because every field there is contains()/value() guarded.
//
// ARRAYS travel as WHOLE SUBTREES ("/ownedItems": [ ... ]), never as leaves. Two independent
// reasons, and missing either one corrupts the save:
//   1. unflatten() of a sparse index set fills the gaps with null, and ApplyShared then reads a
//      null as a value (json type_error.302) or writes a bogus item.
//   2. Even when every leaf of the array is queued, the packet batching below would split a large
//      array across two packets, and the second packet unflattens to exactly that sparse, mostly
//      null array. ownedItems alone is 48 entries at ~21 bytes per pointer-keyed leaf, so it does
//      not fit in one packet -- this is what made the bug survive the first fix.
// Sending an array as a single JSON value is also about 5x smaller than one leaf per entry.
struct NetDelta {
    nlohmann::json leaves; // {"/vitals/rupees": 40}
    nlohmann::json arrays; // {"/ownedItems": [ ... ]}
    bool empty() const {
        return leaves.empty() && arrays.empty();
    }
};

// VOLATILE leaves: live meters the PLAYER moves, as opposed to one-way unlocks. Only the ACTIVE
// game may author these. Without that rule the two games fight over rupees: the wallet scales are
// not the same on both sides (OoT can shuffle the child wallet, MM has no "no wallet" state), so
// the frozen game clamps the value to ITS capacity, republishes the clamped number, and the active
// game's real rupees get dragged down -- money visibly draining on its own. Unlocks stay two-way;
// only these follow whoever is actually being played.
bool NetIsVolatileLeaf(const std::string& key) {
    // Live meters AND live contents: anything the PLAYER changes by playing. The inactive game must
    // never author these -- it can only hold a stale copy, and a stale copy echoed back is exactly
    // how a drunk potion came back, ammo walked backwards and a heart container vanished. The active
    // game is the single author; the inactive one takes what it is sent and keeps the peer's value
    // as its own baseline (see NetRescan).
    if (key.rfind("/vitals/health", 0) == 0 || key.rfind("/vitals/magic", 0) == 0 ||
        key.rfind("/vitals/rupees", 0) == 0) {
        return true; // health, healthCapacity, magic, rupees
    }
    if (key.rfind("/bottleSlots", 0) == 0) {
        return true; // the bottle wheel: contents are consumed and caught in the active game only
    }
    // WHAT IS EQUIPPED/WORN is a live choice too, not an unlock: sword, shield, MM form, adult/child.
    // These apply by OVERWRITE, so a parked game's stale copy (any full resync sends the whole
    // snapshot) used to force the active player's equipment right back.
    if (key.rfind("/equippedShield", 0) == 0 || key.rfind("/equippedSword", 0) == 0 || key.rfind("/form", 0) == 0 ||
        key.rfind("/adult", 0) == 0) {
        return true;
    }
    if (key.rfind("/inv/", 0) == 0) {
        const std::string leaf = key.substr(5);
        if (leaf.size() > 4 && leaf.compare(leaf.size() - 4, 4, "Ammo") == 0) {
            return true; // stickAmmo, bombAmmo, bowAmmo, ... slingshotAmmo
        }
        if (leaf == "powderKegCount" || leaf == "bottomlessContent" || leaf == "bottomlessCount") {
            return true;
        }
    }
    return false;
}

// If `key` points inside an array, return that array's root pointer ("/ownedItems"); else "".
std::string NetArrayRootOf(const std::string& key) {
    size_t pos = 0;
    while (true) {
        const size_t next = key.find('/', pos + 1);
        if (next == std::string::npos) {
            return "";
        }
        const std::string path = key.substr(0, next);
        try {
            if (sNetShared.at(nlohmann::json::json_pointer(path)).is_array()) {
                return path;
            }
        } catch (...) {
            return ""; // not a real path in our snapshot
        }
        pos = next;
    }
}

// Send a delta. Scalars are batched to fill packets; every array goes in a packet of its own so it
// can never be split. An array too big even for one packet is dropped with a loud log rather than
// sent half-formed -- a half-formed one is precisely what corrupts the save.
void NetSendDelta(const NetDelta& delta) {
    nlohmann::json batch = nlohmann::json::object();
    auto flush = [&]() {
        if (!batch.empty()) {
            NetSend({ { "op", "delta" }, { "d", batch } });
            batch = nlohmann::json::object();
        }
    };
    for (auto it = delta.leaves.begin(); it != delta.leaves.end(); ++it) {
        batch[it.key()] = it.value();
        if (batch.dump().size() > kNetBatchBytes) {
            // This leaf overflowed the batch: pull it back out, ship the rest, restart with it.
            nlohmann::json held = batch[it.key()];
            batch.erase(it.key());
            flush();
            batch[it.key()] = held;
        }
    }
    flush();

    for (auto it = delta.arrays.begin(); it != delta.arrays.end(); ++it) {
        nlohmann::json pkt = { { "op", "delta" }, { "a", { { it.key(), it.value() } } } };
        const size_t size = pkt.dump().size();
        if (size > kNetBatchBytes) {
            SPDLOG_ERROR("[FleetNet] array {} is {} bytes and does not fit a packet -- not sent", it.key(), size);
            continue;
        }
        NetSend(pkt);
    }
}

// Split a whole flattened snapshot into scalars + whole arrays (used by the full resync).
NetDelta NetSplitAll(const nlohmann::json& flat) {
    NetDelta out;
    out.leaves = nlohmann::json::object();
    out.arrays = nlohmann::json::object();
    for (auto it = flat.begin(); it != flat.end(); ++it) {
        const std::string root = NetArrayRootOf(it.key());
        if (root.empty()) {
            out.leaves[it.key()] = it.value();
        } else if (!out.arrays.contains(root)) {
            out.arrays[root] = sNetShared.at(nlohmann::json::json_pointer(root));
        }
    }
    return out;
}

// Refresh the snapshot from live state. Returns what changed since the last publish.
NetDelta NetRescan() {
    // ExtractShared merges INTO sNetShared, which is what keeps the one-way-unlock bitfields
    // (ootQuestItems / mmQuestItems) from clobbering bits the peer published: same contract the
    // temp-file path relies on, just held in memory instead of re-read from disk 3x a second.
    FS_TRACE("5a. ExtractShared enter (rescan)");
    ExtractShared(sNetShared);
    FS_TRACE("5b. ExtractShared OK");
    NetPoisonScan("extract");
    nlohmann::json flat = sNetShared.flatten();
    const bool active = FleetShipCombo_IsThisGameActive();

    NetDelta out;
    out.leaves = nlohmann::json::object();
    out.arrays = nlohmann::json::object();
    std::vector<std::string> changedArrays;

    for (auto it = flat.begin(); it != flat.end(); ++it) {
        auto prev = sNetFlat.find(it.key());
        if (prev != sNetFlat.end() && *prev == it.value()) {
            continue;
        }
        if (!active && NetIsVolatileLeaf(it.key())) {
            // Not ours to publish right now. Keep the PEER's value as our published baseline so
            // that when we become active again we diff against what they last said, not against
            // our own frozen reading -- otherwise becoming active would replay a stale meter.
            if (prev != sNetFlat.end()) {
                it.value() = *prev;
            }
            continue;
        }
        const std::string root = NetArrayRootOf(it.key());
        if (root.empty()) {
            out.leaves[it.key()] = it.value();
        } else if (std::find(changedArrays.begin(), changedArrays.end(), root) == changedArrays.end()) {
            changedArrays.push_back(root);
        }
    }
    sNetFlat = flat;
    for (const std::string& root : changedArrays) {
        out.arrays[root] = sNetShared.at(nlohmann::json::json_pointer(root));
    }
    return out;
}

void NetHandleDelta(const nlohmann::json& p) {
    const nlohmann::json d = p.value("d", nlohmann::json::object()); // scalar leaves
    const nlohmann::json a = p.value("a", nlohmann::json::object()); // whole arrays
    if (d.empty() && a.empty()) {
        return;
    }
    nlohmann::json partial = nlohmann::json::object();
    if (!d.empty()) {
        nlohmann::json flat = nlohmann::json::object();
        for (auto it = d.begin(); it != d.end(); ++it) {
            flat[it.key()] = it.value();
        }
        partial = flat.unflatten();
        // Repair unflatten()'s null gaps against the canonical state — without this a single null
        // aborts the whole apply via the catch below. See NetFillNullGaps. Skijer 2026-07-30
        NetFillNullGaps(partial, sNetShared);
    }
    // Arrays are set WHOLE, by pointer -- no unflatten, so null gaps are structurally impossible.
    for (auto it = a.begin(); it != a.end(); ++it) {
        partial[nlohmann::json::json_pointer(it.key())] = it.value();
    }
    NetPoisonScan("before delta apply");
    FS_TRACE("4a. ApplyShared(delta) enter — {} leaves, {} arrays", d.size(), a.size());
    try {
        ApplyShared(partial);
    } catch (const std::exception& e) {
        SPDLOG_ERROR("[FleetNet] ApplyShared threw on delta: {}", e.what());
        return;
    } catch (...) {
        SPDLOG_ERROR("[FleetNet] ApplyShared threw a non-std exception on delta");
        return;
    }
    NetPoisonScan("delta apply");
    FS_TRACE("4b. ApplyShared(delta) OK");
    // Fold the peer's change into our snapshot BEFORE our next scan, so applying it does not read
    // back as a local change and bounce straight back to them.
    for (auto it = d.begin(); it != d.end(); ++it) {
        sNetFlat[it.key()] = it.value();
    }
    for (auto it = a.begin(); it != a.end(); ++it) {
        // Flatten just this subtree so the snapshot keeps its leaf-wise form.
        nlohmann::json one = nlohmann::json::object();
        one[nlohmann::json::json_pointer(it.key())] = it.value();
        const nlohmann::json oneFlat = one.flatten();
        for (auto lf = oneFlat.begin(); lf != oneFlat.end(); ++lf) {
            sNetFlat[lf.key()] = lf.value();
        }
    }
    sNetShared = sNetFlat.unflatten();
}

void NetSendFullState() {
    NetRescan(); // make sure the snapshot is current before we declare it authoritative
    const NetDelta all = NetSplitAll(sNetFlat);
    SPDLOG_INFO("[FleetNet] full resync: {} scalar leaves + {} arrays", all.leaves.size(), all.arrays.size());
    NetSendDelta(all);
    sNetResyncCooldownLeft = kNetResyncCooldown;
}

// Same call, but it can NEVER throw at its caller. Every warp step calls this (departure publish,
// arrival re-baseline, save handshake), and those callers must complete even if the snapshot is
// unserialisable: a state sync that fails is a desync FleetNet will repair on its next hash round,
// while a warp that fails is a player stuck on a black screen. NetRescan -> ExtractShared and
// NetSplitAll's json_pointer lookups are the throwing parts.
void NetSendFullStateSafe(const char* where) {
    try {
        NetSendFullState();
    } catch (const std::exception& e) {
        SPDLOG_ERROR("[FleetNet] full-state publish threw at {}: {} — skipped (hash validation will retry)", where,
                     e.what());
    } catch (...) { SPDLOG_ERROR("[FleetNet] full-state publish threw a non-std exception at {} — skipped", where); }
}

void NetHandlePacket(const nlohmann::json& p) {
    const std::string op = p.value("op", "");
    FS_TRACE("4. packet op={}", op);
    if (op == "delta") {
        NetHandleDelta(p);
    } else if (op == "hash") {
        // The peer told us what it thinks the state is. Agreeing is the common case and costs
        // nothing; disagreeing once is usually just a delta still in flight, so we only act on a
        // SECOND consecutive mismatch.
        const uint64_t theirs = std::strtoull(p.value("h", "0").c_str(), nullptr, 10);
        if (theirs == NetHash(sNetFlat)) {
            sNetMismatchStreak = 0;
            sNetFutileResyncs = 0;
        } else if (++sNetMismatchStreak >= 2 && sNetResyncCooldownLeft == 0) {
            // A resync that does NOT restore agreement means the two hashes can never match: the
            // translator is asymmetric somewhere (a field one game extracts and the other cannot
            // reproduce). Resending forever would be a silent 15-second storm, so back off hard and
            // say so once -- the fix belongs in ExtractShared, not here.
            if (++sNetFutileResyncs >= 3) {
                if (sNetFutileResyncs == 3) {
                    SPDLOG_ERROR("[FleetNet] repeated resyncs did not reconcile the snapshots -- the shared "
                                 "schema is asymmetric between the two games. Backing off; deltas keep working.");
                }
                sNetResyncCooldownLeft = kNetResyncCooldown * 20; // ~5 min
                sNetMismatchStreak = 0;
                return;
            }
            SPDLOG_WARN("[FleetNet] state hash mismatch twice in a row -- requesting full resync");
            sNetMismatchStreak = 0;
            NetSend({ { "op", "resync" } });
            sNetResyncCooldownLeft = kNetResyncCooldown; // don't ask again while one is inbound
        }
    } else if (op == "resync") {
        NetSendFullStateSafe("resync request");
    } else if (op == "saveRequest") {
        // The peer is about to hand over (game change) or just saved: publish everything we have so
        // its snapshot is complete before it writes its own file. See FleetNet_RequestPeerSave.
        NetSendFullStateSafe("peer saveRequest");
        NetSend({ { "op", "saveAck" } });
    } else if (op == "saveAck") {
        SPDLOG_INFO("[FleetNet] peer acknowledged the save request");
    }
}

void NetPump() {
    if (FleetShipCombo_GetActiveGame() < 0 || gPlayState == NULL) {
        return; // no combo, or no live save context to read/write
    }
    // NO REAL FILE, NO SYNC. gPlayState alone is not "a game is loaded": MM's TITLE DEMO runs inside
    // a PlayState too, with fileNum 0xFF and a throwaway save. Publishing from there sends demo
    // state to OoT as if it were the player's, and applying INTO it writes the player's real
    // inventory over a save nobody will keep. The same guard the warp triggers already use.
    if (gSaveContext.fileNum < 0 || gSaveContext.fileNum > 2) {
        return;
    }
    if (sPostFlipTrace > 0) {
        // One line per frame while armed: gives the crash a frame number to sit on, so we can tell
        // "died on the first frozen frame" from "survived a while, then died".
        SPDLOG_WARN("[FleetTrace] === frozen frame {} (active={}) ===", kPostFlipTraceFrames - sPostFlipTrace,
                    (int)FleetShipCombo_IsThisGameActive());
        spdlog::default_logger()->flush();
        sPostFlipTrace--;
    }
    if (sNetResyncCooldownLeft > 0) {
        sNetResyncCooldownLeft--;
    }

    // Drain first: apply what the peer sent before scanning, so their changes land in this frame's
    // snapshot instead of racing our own diff.
    //
    // BOUNDED on purpose. The ring holds kFscRingSlots packets and the peer can refill it while we
    // drain (a resync storm, a peer running many frames per frame of ours), so an unbounded `while`
    // is a loop the game can never leave — a hard freeze with the process still "running". Anything
    // left over is drained next frame; the packets are idempotent and the hash round repairs drops.
    char buf[4200];
    int drained = 0;
    while (drained < kNetMaxDrainPerFrame && FleetShipCombo_PopPacket(buf, (int)sizeof(buf))) {
        drained++;
        try {
            NetHandlePacket(nlohmann::json::parse(buf));
        } catch (const std::exception& e) { SPDLOG_ERROR("[FleetNet] bad packet dropped: {}", e.what()); } catch (...) {
            SPDLOG_ERROR("[FleetNet] packet handler threw a non-std exception — dropped");
        }
    }
    if (drained >= kNetMaxDrainPerFrame) {
        SPDLOG_WARN("[FleetNet] drain cap hit ({} packets this frame) — the rest waits for the next frame", drained);
    }

    // The scan reads the whole live save through ExtractShared, so it throws on exactly the kind of
    // state a fresh check can introduce. It must never take the frame — or the warp logic that runs
    // in the same hook — down with it.
    try {
        if (++sNetScanTick >= kNetScanPeriod) {
            sNetScanTick = 0;
            const NetDelta changed = NetRescan();
            if (!sNetPrimed) {
                // First scan after boot/arrival: the "changes" are just the entire existing state, and
                // the peer already has it from the arrival overlay. Publishing it would be a pointless
                // storm, so we only record the baseline.
                sNetPrimed = true;
            } else if (!changed.empty()) {
                NetSendDelta(changed);
            }
        }

        if (++sNetVerifyTick >= kNetVerifyPeriod) {
            sNetVerifyTick = 0;
            NetSend({ { "op", "hash" }, { "h", std::to_string(NetHash(sNetFlat)) } });
        }
    } catch (const std::exception& e) {
        SPDLOG_ERROR("[FleetNet] scan/verify threw: {} — skipped this round", e.what());
    } catch (...) { SPDLOG_ERROR("[FleetNet] scan/verify threw a non-std exception — skipped this round"); }
    FS_TRACE("5. pump done ({} packets drained)", drained);
}

// Re-seed the snapshot from a known-agreed state (the temp-file overlay at a departure/arrival) and
// re-prime, so the first scan after a game change does not report the whole save as "changed".
void NetResetBaseline(const nlohmann::json& sh) {
    sNetShared = sh.is_object() ? sh : nlohmann::json::object();
    // Button equips are a game-change-only payload (see ExtractEquips). The departure block we are
    // re-baselining from carries them, so strip them here: leaving them in would put them back in the
    // diff and in the verify hash, which is the whole bug this split exists to kill.
    sNetShared.erase("cEquips");
    sNetShared.erase("dEquips");
    sNetFlat = sNetShared.flatten();
    sNetPrimed = false;
    sNetMismatchStreak = 0;
    sNetScanTick = 0;
}

void HandleOwnSave(s16 fileNum) {
    if (FleetShipCombo_GetActiveGame() < 0 || !FleetShipCombo_IsThisGameActive()) {
        return; // combo off, or we're the frozen responder (its own save must not re-signal)
    }
    // Remember the slot we last saved in, so a boot-into-MM auto-resume loads THIS file (not File 1).
    if (fileNum >= 0 && fileNum <= 2) {
        CVarSetInteger("gFleetCombo.LastSlot", fileNum);
        CVarSave();
    }
    RefreshSharedInTemp();
    // Publish everything BEFORE signalling: the responder saves its own slot the moment it sees the
    // signal, so any delta still queued behind it would land in the peer's file one save too late.
    NetSendFullStateSafe("own save");
    FleetShipCombo_SignalSyncSave(fileNum);
    sWaitingAckSeq = FleetShipCombo_GetSyncSaveSeq();
}

int sHoleGrabCooldown = 0; // frames the fleet hole may not GRAB after an arrival (visible, inert)

void ProcessSignals() {
    if (FleetShipCombo_GetActiveGame() < 0) {
        return;
    }
    // Runs BEFORE any gPlayState gate: the quest page fills itself while the arrival scene is still
    // loading, and NetPump (which needs a live PlayState) would only notice one frame later, after
    // the culprit had already run.
    // Cross-game restart: the OTHER game reset -> reset ourselves too. DoLocalReset does NOT re-signal,
    // so this never ping-pongs.
    if (FleetShipCombo_ConsumeRestartRequest()) {
        FleetCombo_DoLocalReset();
        // A restart always lands the player on OCARINA OF TIME's title screen: MM's own title/file
        // select are not screens this combo ever shows, and leaving MM active here would put the
        // player on one (its file select would even auto-load a slot behind OoT's back).
        FleetShipCombo_YieldToOoT();
        return;
    }
    if (sHoleGrabCooldown > 0) {
        sHoleGrabCooldown--;
    }
    unsigned long long seq = FleetShipCombo_GetSyncSaveSeq();
    if (!sSyncSeqInit) {
        sSyncSeqInit = true;
        sLastSeenSyncSeq = seq;
    }
    if (seq != sLastSeenSyncSeq) {
        // Step 2 of the post-swap trace: the peer saved, so the FROZEN game writes its own slot to
        // disk from here. This is the heaviest thing a frozen MM ever does (JSON of the whole save +
        // a flash write started while nothing is updating it), and it lands right after a swap.
        FS_TRACE("2. peer save signal seen (seq {}) — responder path", seq);
        sLastSeenSyncSeq = seq;
        // Same "no real file, no sync" rule as the pump: MM's title demo also runs inside a
        // PlayState (fileNum 0xFF), and overlaying the peer's inventory onto it — then saving —
        // writes the player's state into a file nobody will keep.
        const bool haveRealFile = gPlayState != NULL && gSaveContext.fileNum >= 0 && gSaveContext.fileNum <= 2;
        if (!FleetShipCombo_IsThisGameActive() && !haveRealFile) {
            SPDLOG_WARN("[FleetSync] save signal ignored: MM has no file loaded (fileNum={})",
                        (int)gSaveContext.fileNum);
            FleetShipCombo_AckSyncSave(seq); // ack anyway: the peer must not wait on us
        } else if (!FleetShipCombo_IsThisGameActive()) {
            nlohmann::json temp;
            if (ReadTemp(temp) && temp.contains("shared")) {
                try {
                    ApplyShared(temp["shared"]);
                } catch (const std::exception& e) {
                    SPDLOG_ERROR("[FleetSync] ApplyShared threw (responder): {}", e.what());
                } catch (...) { SPDLOG_ERROR("[FleetSync] ApplyShared threw a non-std exception (responder)"); }
            }
            int slot = FleetShipCombo_GetSyncSaveSlot();
            if (slot >= 0 && slot <= 2 && gSaveContext.fileNum == slot) {
                FS_TRACE("2a. SaveOwnSlotFrozen enter (flash write while frozen)");
                SaveOwnSlotFrozen();
                FS_TRACE("2b. SaveOwnSlotFrozen done");
            }
            FleetShipCombo_AckSyncSave(seq);
        }
    }
    if (sWaitingAckSeq != 0 && FleetShipCombo_GetSyncSaveAck() >= sWaitingAckSeq) {
        sWaitingAckSeq = 0;
        DeleteTemp();
    }
    // Continuous state sync. Runs in BOTH exes, active or frozen -- the frozen one still gets this
    // hook, which is exactly what makes the responder path above work.
    NetPump();
}

void RegisterFleetSync() {
    // Both of these run every frame and touch JSON built from live save state, so both can throw on
    // data a fresh check introduced. Uncaught, that propagates out of the game loop — and it is the
    // same hook the warp logic lives in, so one bad frame could swallow a warp step. Swallow + log.
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSaveFile>([](s16 fileNum) {
        try {
            HandleOwnSave(fileNum);
        } catch (const std::exception& e) {
            SPDLOG_ERROR("[FleetSync] save handshake threw: {}", e.what());
        } catch (...) { SPDLOG_ERROR("[FleetSync] save handshake threw a non-std exception"); }
    });
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnGameStateUpdate>([]() {
        try {
            ProcessSignals();
        } catch (const std::exception& e) { SPDLOG_ERROR("[FleetSync] signal pump threw: {}", e.what()); } catch (...) {
            SPDLOG_ERROR("[FleetSync] signal pump threw a non-std exception");
        }
    });
    // Generic FC cross-item applier: grant the native deficit for fcIds obtained elsewhere. Deficit +
    // shadow (comboAppliedFc) makes it idempotent, so it's safe every frame. Gate on gPlayState != NULL
    // because Rando::GiveItem -> Item_Give derefs gPlayState.
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnGameStateUpdate>([]() {
        if (gPlayState != NULL) {
            // Guard: granting the just-obtained check's cross-item (Rando::GiveItem -> ConvertItem) can
            // throw on a bad/missing item; uncaught it would std::terminate 2ship on the frame after a
            // check. Catch + log so 2ship survives and the culprit item is recorded.
            FleetSync_PostFlipTrace("3. ApplyFcRegistryToNatives enter");
            try {
                ApplyFcRegistryToNatives();
            } catch (const std::exception& e) {
                SPDLOG_ERROR("[FleetSync] ApplyFcRegistryToNatives threw: {}", e.what());
            } catch (...) { SPDLOG_ERROR("[FleetSync] ApplyFcRegistryToNatives threw a non-std exception"); }
            FleetSync_PostFlipTrace("3c. ApplyFcRegistryToNatives done");
        }
    });
    // Heal ownedItems poisoned by the pre-fix raw copy the moment ANY save is loaded (the crash
    // was in the pause draw, which can happen before any FleetSync event would run the healer).
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSaveLoad>([](s16 fileNum) {
        (void)fileNum;
        HealLeakedOwnedItems();
        // A new file starts from a blank snapshot: it only records "what did I last publish", so
        // carrying it across a load makes it describe a DIFFERENT save. (Anchor's equivalent packet
        // assigns the joined state outright instead of merging into what was there.)
        NetResetBaseline(nlohmann::json::object());
        SPDLOG_INFO("[FleetNet] file loaded — snapshot reset");
    });
}

void* sFleetHole = nullptr;
bool sHoleFallPending = false;

} // namespace

extern "C" {

// Absolute path of a file inside the shared fleet folder (<ShipDir>/fleet/<name>), created on demand.
// Ship resolves it from its own exe dir, 2ship from the parent of its own — the same physical folder,
// which is what makes it the place for anything the two games must literally SHARE rather than copy,
// like the combo pictograph. Returns "" if the exe dir can't be resolved; the returned pointer stays
// valid until the next call. Skijer's NEI
const char* FleetSync_SharedFilePath(const char* name) {
    static std::string sPath;
    sPath.clear();
    std::filesystem::path dir = SelfExeDir();
    if (dir.empty() || name == nullptr) {
        return "";
    }
    std::error_code ec;
    std::filesystem::create_directories(dir.parent_path() / "fleet", ec);
    sPath = (dir.parent_path() / "fleet" / name).string();
    return sPath.c_str();
}

void FleetSync_WriteDeparture(int slot) {
    if (FleetShipCombo_GetActiveGame() < 0) {
        return;
    }
    if (slot < 0 || slot > 2) {
        SPDLOG_WARN("[FleetSync] departure with no real file loaded (slot {}) — ignored", slot);
        return; // never anchor/extract an unloaded save (title demo etc.)
    }
    FS_TRACE("1a. WriteDeparture: ReadTemp");
    nlohmann::json temp;
    ReadTemp(temp);
    temp["version"] = 1;
    temp["slot"] = slot;
    // Serialize the anchor + shared inside a guard: a check picked up in MM populates rando/FC fields
    // whose (de)serialization can throw (nlohmann .at()/version-mismatch), and an UNCAUGHT throw here
    // was calling std::terminate -> 2ship silently closed on the warp-out. Catch + log so 2ship
    // survives and the exact exception is recorded for a root fix.
    try {
        FS_TRACE("1b. serializing anchor (whole save -> json)");
        temp["mm"] = gSaveContext; // full anchor via BenJsonConversions to_json
        FS_TRACE("1c. anchor done — ExtractShared next");
        nlohmann::json sh = temp.contains("shared") ? temp["shared"] : nlohmann::json::object();
        ExtractShared(sh);
        FS_TRACE("1d. ExtractShared done — ExtractEquips next");
        ExtractEquips(sh); // game-change payload: the ONLY moment button equips are published
        temp["shared"] = sh;
        FS_TRACE("1e. shared block built");
    } catch (const std::exception& e) {
        temp.erase("mm"); // never persist a half-serialized anchor
        SPDLOG_ERROR("[FleetSync] departure serialization threw: {} (slot {}) — wrote without mm/shared", e.what(),
                     slot);
    } catch (...) {
        temp.erase("mm");
        SPDLOG_ERROR("[FleetSync] departure serialization threw a non-std exception (slot {})", slot);
    }
    FS_TRACE("1f. WriteTemp (json -> disk)");
    WriteTemp(temp);
    FS_TRACE("1g. WriteTemp done");
    // We are about to freeze: hand the peer everything, so whatever it does while we are asleep is
    // built on our final state rather than on deltas that may still have been in flight. Read the
    // shared block back off `temp` -- the extract above may have been skipped by the catch.
    //
    // Guarded: this whole block is a state-sync nicety, and it runs INSIDE the departure, one line
    // before the game flips. If it throws it must not reach the caller — the flip is what the player
    // is waiting for. (The caller flips regardless now, but a departure that also finishes its
    // bookkeeping is strictly better.)
    try {
        FS_TRACE("1h. NetResetBaseline + full-state publish");
        NetResetBaseline(temp.contains("shared") ? temp["shared"] : nlohmann::json::object());
        NetSendFullStateSafe("departure");
        FS_TRACE("1i. full-state published");
    } catch (const std::exception& e) {
        SPDLOG_ERROR("[FleetSync] departure re-baseline threw: {} — departure still committed", e.what());
    } catch (...) {
        SPDLOG_ERROR("[FleetSync] departure re-baseline threw a non-std exception — departure still committed");
    }
    NetPoisonDump("departure written");
    SPDLOG_INFO("[FleetSync] MM departure written (slot {})", slot);
}

void FleetSync_ApplyArrival(int slot) {
    if (FleetShipCombo_GetActiveGame() < 0) {
        return;
    }
    NetPoisonDump("arrival:entry (save just loaded from disk)");
    nlohmann::json temp;
    if (!ReadTemp(temp)) {
        NetPoisonDump("arrival:no temp file");
        return;
    }
    bool dirty = false;
    // The temp's "slot" is written together with the "mm" anchor by WriteDeparture, so it identifies
    // which slot the anchor belongs to. Only restore the anchor when it matches the slot we're arriving
    // at: the FileSelect load already put THIS slot's correct rando seed in gSaveContext, and splatting
    // an anchor written for a DIFFERENT slot would overwrite shipSaveInfo.rando with the wrong seed
    // (part of the "MM loaded a different seed" bug). If unsure (no slot field), keep the old behavior.
    int anchorSlot = (temp.contains("slot") && temp["slot"].is_number_integer()) ? temp["slot"].get<int>() : slot;
    if (temp.contains("mm")) {
        if (anchorSlot == slot) {
            try {
                // ZERO first: BenJson's SaveContext from_json only fills the serialized fields — a raw
                // stack temporary would leave every unserialized field (cycleSceneFlags, respawn[],
                // timers, ...) as GARBAGE that the memcpy below would splat into the live save.
                static SaveContext restored;
                memset(&restored, 0, sizeof(restored));
                from_json(temp["mm"], restored);
                // Persistent region only (same limit the SaveManager flashrom reader uses).
                memcpy(&gSaveContext, &restored, offsetof(SaveContext, fileNum));
                SPDLOG_INFO("[FleetSync] MM anchor restored (slot {})", slot);
                NetPoisonDump("arrival:after anchor memcpy");
            } catch (...) { SPDLOG_WARN("[FleetSync] MM anchor unreadable — skipped"); }
        } else {
            SPDLOG_WARN("[FleetSync] MM anchor is for slot {} but arriving at {} — kept the loaded save's seed",
                        anchorSlot, slot);
        }
        temp.erase("mm");
        dirty = true;
    }
    if (temp.contains("shared")) {
        // Guard the shared overlay: the other game's block after a cross-game check can carry rando/FC
        // data whose parse throws (nlohmann .at()/.get type); an uncaught throw here would std::terminate
        // MM on arrival. Catch + log so MM survives.
        try {
            NetPoisonScan("before arrival overlay");
            ApplyShared(temp["shared"]);
            // Button equips: arrival only, and ONE-SHOT. Consuming them here means a stale temp block
            // (one whose departure we already absorbed) can never re-stamp the player's buttons later.
            ApplyEquips(temp["shared"]);
            temp["shared"].erase("cEquips");
            temp["shared"].erase("dEquips");
            dirty = true;
            NetPoisonScan("arrival overlay");
            NetPoisonDump("arrival:after shared overlay");
            SPDLOG_INFO("[FleetSync] shared overlay applied (MM)");
        } catch (const std::exception& e) {
            SPDLOG_ERROR("[FleetSync] ApplyShared threw on arrival: {}", e.what());
        } catch (...) { SPDLOG_ERROR("[FleetSync] ApplyShared threw a non-std exception on arrival"); }
    }
    // Re-baseline on the state we just arrived with, then ASK the peer for its full state (Anchor's
    // request/response shape). The temp file only carries what the peer knew when it wrote the
    // departure; anything it changed afterwards -- or any delta lost while we were frozen -- comes
    // back through this. Its answer is a normal resync, applied by the pump.
    // Guarded for the same reason as the departure: the arrival pipeline continues into the
    // destination overrides right after this returns, and none of it may be skipped because a
    // snapshot could not be flattened.
    try {
        NetResetBaseline(temp.contains("shared") ? temp["shared"] : nlohmann::json::object());
        NetSend({ { "op", "saveRequest" } });
    } catch (const std::exception& e) {
        SPDLOG_ERROR("[FleetSync] arrival re-baseline threw: {} — arrival continues", e.what());
    } catch (...) { SPDLOG_ERROR("[FleetSync] arrival re-baseline threw a non-std exception — arrival continues"); }
    // The combo has ONE pictograph, and this has to be the LAST word on it. Everything above splats
    // saved state over the live one — the anchor memcpy alone rewrites the whole persistent region,
    // pictoPhotoI5 and QUEST_PICTOGRAPH included — so an import done any earlier is simply undone,
    // which is exactly how a picture deleted in MM came back and a picture taken in OoT never
    // arrived. Here, what the run currently holds wins: the shared picture if there is one, and no
    // picture at all if the other game threw it away. Skijer's NEI
    FleetPicto_Import();

    if (dirty) {
        WriteTemp(temp);
    }
}

void FleetSync_BeginPostFlipTrace(void) {
    sPostFlipTrace = kPostFlipTraceFrames;
    SPDLOG_WARN("[FleetTrace] post-flip trace armed for {} frames", kPostFlipTraceFrames);
}

int FleetSync_PostFlipTraceActive(void) {
    return sPostFlipTrace > 0 ? 1 : 0;
}

void FleetSync_PostFlipTrace(const char* what) {
    FS_TRACE("{}", what ? what : "?");
}

void FleetSync_OnTitleScreen(void) {
    if (sTitleDeleteDone || FleetShipCombo_GetActiveGame() < 0) {
        return;
    }
    sTitleDeleteDone = true;
    DeleteTemp();
    SPDLOG_INFO("[FleetSync] title screen -> temp file deleted");
}

void FleetSync_RegisterFleetHole(void* actor) {
    sFleetHole = actor;
}
int FleetSync_IsFleetHole(void* actor) {
    return actor != nullptr && actor == sFleetHole;
}
void FleetSync_OnHoleFall(void) {
    sHoleFallPending = true;
}
int FleetSync_HoleFallPending(void) {
    return sHoleFallPending ? 1 : 0;
}
void FleetSync_ClearHoleFall(void) {
    sHoleFallPending = false;
}
void FleetSync_SetHoleGrabCooldown(int frames) {
    sHoleGrabCooldown = frames;
}
int FleetSync_HoleGrabInert(void) {
    return sHoleGrabCooldown > 0 ? 1 : 0;
}

} // extern "C"

static RegisterShipInitFunc initFleetSync(RegisterFleetSync, {});
