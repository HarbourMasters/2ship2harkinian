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
#include "FleetComboItems.h"     // FCI_MAX, FCI_NO_ITEM
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/BenJsonConversions.hpp"
#include "2s2h/Rando/Types.h"
#include "2s2h/Rando/Rando.h" // Rando::GiveItem

#include <libultraship/bridge/consolevariablebridge.h> // CVar: persist last-saved slot for auto-resume
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
// Nonzero while ApplyFcRegistryToNatives is materializing an FC deficit through the game's native
// give path. GiveItem.cpp's record hook checks this and skips recording those grants — otherwise a
// registry-driven grant would re-bump comboObtainedFc and feed itself an endless per-frame deficit.
int gFcCombo_SuppressRecord = 0;
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
    } catch (...) {
        SPDLOG_WARN("[FleetSync] temp file write failed");
    }
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
    if (wu & (1 << 2)) return 2;
    if (wu & (1 << 1)) return 1;
    return 0;
}

uint16_t ComputeShieldOwned() {
    NeiSaveData* nei = Nei_Save();
    uint16_t sh = nei->shieldOwned;
    int nibble = (MM_EQ.equipment >> 4) & 0xF; // 1 Hero, 2 Mirror(-MM)
    if (nibble >= 1) sh |= FC_SHIELD_HYLIAN;   // MM Hero == OoT Hylian
    if (nibble == 2) sh |= FC_SHIELD_IKANA;    // MM Mirror == OoT Shield of Ikana
    if (nei->extEquipOwnedBits & (1u << 19)) sh |= FC_SHIELD_DIVINE;
    if (nei->extEquipOwnedBits & (1u << 20)) sh |= FC_SHIELD_KITE;
    if (nei->extEquipOwnedBits & (1u << 21)) sh |= FC_SHIELD_IKANA;
    nei->shieldOwned = sh;
    return sh;
}

int GetEquippedShieldCanonical() {
    NeiSaveData* nei = Nei_Save();
    if (nei->extEquipShield >= 1 && nei->extEquipShield <= 3) {
        return 3 + nei->extEquipShield;
    }
    int nibble = (MM_EQ.equipment >> 4) & 0xF;
    if (nibble == 1) return 2; // Hero -> Hylian
    if (nibble == 2) return 6; // Mirror-MM -> Ikana
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
void FoldNativesIntoRegistry() {
    NeiSaveData* nei = Nei_Save();
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
                Rando::GiveItem((RandoItemId)native);
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
        uint8_t v = nei->ownedItems[i];
        if (v == 0xFF) {
            continue;
        }
        if (v >= 0x9E && v <= 0xB5) {
            nei->ownedItems[i] = (uint8_t)(v + FC_PAGE2_MM_OFFSET); // leaked OoT page-2 id -> MM id
        } else if (i >= 24 && v >= FC_OOT_MM_MASK_ITEM_BASE &&
                   v < FC_OOT_MM_MASK_ITEM_BASE + FC_MM_MASK_COUNT) {
            // Leaked OoT page-3 mask id: masks are NATIVE in MM — grant the native mask and clear.
            if (MM_INV.items[FC_MM_MASK_SLOT_BASE + (v - FC_OOT_MM_MASK_ITEM_BASE)] == 0xFF) {
                MM_INV.items[FC_MM_MASK_SLOT_BASE + (v - FC_OOT_MM_MASK_ITEM_BASE)] =
                    kFcMmMaskItemBySlot[v - FC_OOT_MM_MASK_ITEM_BASE];
            }
            nei->ownedItems[i] = 0xFF;
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
                     { "magicLevel", MM_PD.magicLevel },
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
    // GFS: keep the native inventory item and the shared weaponUpgrades bit coherent both ways.
    if (MM_INV.items[kSlotGreatFairySword] != 0xFF) {
        nei->weaponUpgrades |= (1 << 4); // WEAPON_UPGRADE_BGS_GREAT_FAIRY
    }
    sh["weaponUpgrades"] = nei->weaponUpgrades;
    sh["shieldOwned"] = ComputeShieldOwned();
    sh["equippedShield"] = GetEquippedShieldCanonical();
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
    inv["slingshot"] = nei->slingshotOwned != 0;
    inv["slingshotAmmo"] = nei->slingshotSeeds;
    int hookLevel = nei->ootHookshotLevel;
    if (hookLevel == 0 && MM_INV.items[kSlotHookshot] != 0xFF) {
        hookLevel = 1; // native MM hookshot with no OoT-chain level recorded
    }
    inv["hookshotLevel"] = hookLevel;
    inv["clawshot"] = nei->clawshotOwned != 0;
    inv["pictobox"] = MM_INV.items[kSlotPictograph] != 0xFF || nei->pictoboxOwned;
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
        } else if (nei->ownedItems[i] != 0xFF) {
            canon = FcEquip_MmToOot(nei->ownedItems[i]); // 0xFF if unmappable
        }
        ownedItems.push_back(canon);
    }
    sh["ownedItems"] = ownedItems;
    sh["tradeAdultOwned"] = nei->tradeAdultOwned;
    // Merge with previous shared value (one-way unlocks, authored by both games via
    // cross-placement) so bits OoT published that we haven't applied yet aren't clobbered.
    sh["ootQuestItems"] = sh.value("ootQuestItems", 0u) | nei->ootQuestItems;
    sh["gsTokens"] = nei->ootGsCount;
    // mmQuestItems: authored from MM's NATIVE questItems (remains/songs/notebook). Plain OR-merge
    // with the previous shared value: every bit is a one-way unlock and BOTH games author bits via
    // cross-placement (OoT sets FC_MMQ bits for remains/songs found there), so never drop prev bits.
    sh["mmQuestItems"] = sh.value("mmQuestItems", 0u) | (MM_INV.questItems & (uint32_t)FC_MMQ_NATIVE_MASK);
    sh["comboObtained"] = nei->comboObtained;
    sh["comboObtainedFc"] = nei->comboObtainedFc; // fcId-indexed cross store (counts); comboAppliedFc is LOCAL only
    sh["comboTriforce"] = nei->comboTriforce;
    sh["ootMasksOwned"] = nei->ootMasksOwned; // MM authors this (OoT echoes)

    // Button equips: translate MM ids -> canonical OoT ids (0xFF = unmappable, keeps destination).
    // CRITICAL: MM's Ocarina of Time is id 0x00 and MM's "empty" button is 0xFF, but any zeroed/
    // uninitialized equip region reads as a raw 0x00. Left alone, FcEquip_MmToOot(0x00)=0x08 (OoT
    // ocarina) bakes into the shared block and the return trip FcEquip_OotToMm(0x08)=0x00 stamps an
    // MM Ocarina into EVERY empty slot (the reported bug). Since 0x00 can't be disambiguated on a
    // button, treat it as empty (0xFF) here; the ocarina's ownership still travels via inv.ocarinaTime.
    nlohmann::json c = nlohmann::json::array();
    for (int i = 1; i <= 3; i++) {
        uint8_t v = MM_EQ.buttonItems[CUR_FORM][i];
        c.push_back(v == 0x00 ? 0xFF : FcEquip_MmToOot(v));
    }
    sh["cEquips"] = c;
    nlohmann::json d = nlohmann::json::array();
    for (int i = 0; i < 4; i++) {
        uint8_t v = gSaveContext.save.shipSaveInfo.dpadEquips.dpadItems[CUR_FORM][i];
        d.push_back(v == 0x00 ? 0xFF : FcEquip_MmToOot(v));
    }
    sh["dEquips"] = d;

    sh["form"] = (int)gSaveContext.save.playerForm; // 0 FD .. 4 Human (same space as canonical)
}

void ApplyShared(const nlohmann::json& sh) {
    NeiSaveData* nei = Nei_Save();
    HealLeakedOwnedItems();

    if (sh.contains("vitals")) {
        const auto& v = sh["vitals"];
        MM_PD.healthCapacity = (int16_t)v.value("healthCapacity", (int)MM_PD.healthCapacity);
        MM_PD.health = (int16_t)std::min<int>(v.value("health", (int)MM_PD.health), MM_PD.healthCapacity);
        MM_PD.doubleDefense = (uint8_t)v.value("doubleDefense", 0);
        MM_INV.defenseHearts = (int8_t)v.value("defenseHearts", 0);
        MM_PD.magicLevel = (int8_t)v.value("magicLevel", 0);
        MM_PD.magic = (int8_t)v.value("magic", 0);
        MM_PD.isMagicAcquired = (uint8_t)v.value("isMagic", 0);
        MM_PD.isDoubleMagicAcquired = (uint8_t)v.value("isDoubleMagic", 0);
        MM_PD.rupees = (int16_t)v.value("rupees", (int)MM_PD.rupees);
    }
    if (sh.contains("upgrades")) {
        const auto& u = sh["upgrades"];
        Inventory_ChangeUpgrade(UPG_WALLET, u.value("wallet", 0));
        Inventory_ChangeUpgrade(UPG_QUIVER, u.value("quiver", 0));
        Inventory_ChangeUpgrade(UPG_BOMB_BAG, u.value("bombBag", 0));
        Inventory_ChangeUpgrade(UPG_DEKU_STICKS, u.value("sticks", 0));
        Inventory_ChangeUpgrade(UPG_DEKU_NUTS, u.value("nuts", 0));
        Inventory_ChangeUpgrade(UPG_STRENGTH, u.value("strength", 0));
        Inventory_ChangeUpgrade(UPG_SCALE, u.value("scale", 0));
        OotUpgSet(9, u.value("strength", 0));
        OotUpgSet(12, u.value("scale", 0));
        OotUpgSet(0, std::min(u.value("bulletBag", 0), 3)); // cap 3: raw 7 reads capacities OOB
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
        if (owned & FC_SHIELD_DIVINE) nei->extEquipOwnedBits |= (1u << 19);
        if (owned & FC_SHIELD_KITE) nei->extEquipOwnedBits |= (1u << 20);
        if (owned & FC_SHIELD_IKANA) nei->extEquipOwnedBits |= (1u << 21);
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
    if (sh.contains("swordFlags")) {
        const auto& s = sh["swordFlags"];
        if (s.value("master", false)) nei->comboObtained[FC_OOT_SWORD_MASTER] = 1;
        if (s.value("biggoron", false)) nei->comboObtained[FC_OOT_SWORD_BIGGORON] = 1;
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
        if (inv.value("boomerang", false)) nei->ootBoomerangOwned = 1;
        if (inv.value("hammer", false)) nei->ootHammerOwned = 1;
        if (inv.value("dins", false)) nei->ootSpellsOwned |= (1 << 0);
        if (inv.value("farores", false)) nei->ootSpellsOwned |= (1 << 1);
        if (inv.value("nayrus", false)) nei->ootSpellsOwned |= (1 << 2);
        if (inv.value("slingshot", false)) nei->slingshotOwned = 1;
        if (inv.contains("slingshotAmmo")) nei->slingshotSeeds = (uint8_t)inv["slingshotAmmo"].get<int>();
        if (inv.value("pictobox", false)) {
            nei->pictoboxOwned = 1;
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
            if (c > nei->powerKegCount) nei->powerKegCount = (uint8_t)c;
            if (MM_INV.items[kSlotPowderKeg] != 0xFF && c > MM_INV.ammo[kSlotPowderKeg]) {
                MM_INV.ammo[kSlotPowderKeg] = (int8_t)c;
            }
        }
        if (inv.value("net", false)) nei->netEquipped = 1;
        if (inv.contains("bottomlessMode")) nei->bottomlessBottleMode = (uint8_t)inv["bottomlessMode"].get<int>();
        if (inv.contains("bottomlessContent")) {
            uint8_t bt = FcBottle_OotToMm((uint8_t)inv["bottomlessContent"].get<int>());
            nei->bottomlessContent = (bt == FC_BOTTLE_UNMAPPED) ? 0x12 : bt;
        }
        if (inv.contains("bottomlessCount")) nei->bottomlessCount = (uint8_t)inv["bottomlessCount"].get<int>();
    }

    if (sh.contains("bottleSlots") && sh["bottleSlots"].is_array()) {
        for (int i = 0; i < 8 && i < (int)sh["bottleSlots"].size(); i++) {
            uint8_t t = FcBottle_OotToMm((uint8_t)sh["bottleSlots"][i].get<int>());
            if (t == FC_BOTTLE_UNMAPPED) {
                t = 0x12; // MM ITEM_BOTTLE: keep an empty bottle rather than a foreign id
            }
            nei->bottleSlots[i] = t;
        }
    }
    if (sh.contains("ownedItems") && sh["ownedItems"].is_array()) {
        for (int i = 0; i < 48 && i < (int)sh["ownedItems"].size(); i++) {
            uint8_t v = (uint8_t)sh["ownedItems"][i].get<int>();
            // 0xFF = empty; 0x00 = uninitialized slot (no custom item / MM mask is ever id 0). Both
            // must be skipped, else a raw 0x00 translates to a spurious page-2/mask item.
            if (v == 0xFF || v == 0x00) {
                continue;
            }
            if (i >= 24) {
                // Canonical mask entry -> grant MM's NATIVE mask (never store in ownedItems).
                if (v >= FC_OOT_MM_MASK_ITEM_BASE && v < FC_OOT_MM_MASK_ITEM_BASE + FC_MM_MASK_COUNT &&
                    MM_INV.items[FC_MM_MASK_SLOT_BASE + (v - FC_OOT_MM_MASK_ITEM_BASE)] == 0xFF) {
                    MM_INV.items[FC_MM_MASK_SLOT_BASE + (v - FC_OOT_MM_MASK_ITEM_BASE)] =
                        kFcMmMaskItemBySlot[v - FC_OOT_MM_MASK_ITEM_BASE];
                }
            } else {
                uint8_t mm = FcEquip_OotToMm(v); // page-2 translate (+0x18); 0xFF if unmappable
                if (mm != 0xFF) nei->ownedItems[i] = mm;
            }
        }
    }
    if (sh.contains("tradeAdultOwned")) nei->tradeAdultOwned |= sh["tradeAdultOwned"].get<uint32_t>();
    if (sh.contains("ootQuestItems")) nei->ootQuestItems |= sh["ootQuestItems"].get<uint32_t>();
    if (sh.contains("gsTokens")) {
        int gs = sh["gsTokens"].get<int>();
        if (gs > nei->ootGsCount) nei->ootGsCount = (uint16_t)gs;
    }
    if (sh.contains("mmQuestItems")) {
        MM_INV.questItems |= (sh["mmQuestItems"].get<uint32_t>() & (uint32_t)FC_MMQ_NATIVE_MASK);
    }
    if (sh.contains("ootMasksOwned")) nei->ootMasksOwned |= (uint16_t)sh["ootMasksOwned"].get<int>();
    if (sh.contains("comboObtained") && sh["comboObtained"].is_array()) {
        for (int i = 0; i < FC_COMBO_OBTAINED_SIZE && i < (int)sh["comboObtained"].size(); i++) {
            uint8_t v = (uint8_t)sh["comboObtained"][i].get<int>();
            if (v > nei->comboObtained[i]) nei->comboObtained[i] = v;
        }
    }
    if (sh.contains("comboObtainedFc") && sh["comboObtainedFc"].is_array()) {
        for (int i = 0; i < FC_COMBO_OBTAINED_FC_SIZE && i < (int)sh["comboObtainedFc"].size(); i++) {
            uint8_t v = (uint8_t)sh["comboObtainedFc"][i].get<int>();
            if (v > nei->comboObtainedFc[i]) nei->comboObtainedFc[i] = v;
        }
    }
    if (sh.contains("comboTriforce")) {
        uint16_t tf = (uint16_t)sh["comboTriforce"].get<int>();
        if (tf > nei->comboTriforce) nei->comboTriforce = tf;
    }
    ApplyRegistryToNatives();

    if (sh.contains("cEquips") && sh["cEquips"].is_array()) {
        for (int i = 0; i < 3 && i < (int)sh["cEquips"].size(); i++) {
            uint8_t mm = FcEquip_OotToMm((uint8_t)sh["cEquips"][i].get<int>());
            if (mm != 0xFF) MM_EQ.buttonItems[CUR_FORM][1 + i] = mm;
        }
    }
    if (sh.contains("dEquips") && sh["dEquips"].is_array()) {
        for (int i = 0; i < 4 && i < (int)sh["dEquips"].size(); i++) {
            uint8_t mm = FcEquip_OotToMm((uint8_t)sh["dEquips"][i].get<int>());
            if (mm != 0xFF) gSaveContext.save.shipSaveInfo.dpadEquips.dpadItems[CUR_FORM][i] = mm;
        }
    }

    if (sh.contains("form")) {
        int form = sh["form"].get<int>();
        if (form >= 0 && form <= 4) {
            gSaveContext.save.playerForm = (s8)form;
            switch (form) {
                case PLAYER_FORM_FIERCE_DEITY: gSaveContext.save.equippedMask = PLAYER_MASK_FIERCE_DEITY; break;
                case PLAYER_FORM_GORON: gSaveContext.save.equippedMask = PLAYER_MASK_GORON; break;
                case PLAYER_FORM_ZORA: gSaveContext.save.equippedMask = PLAYER_MASK_ZORA; break;
                case PLAYER_FORM_DEKU: gSaveContext.save.equippedMask = PLAYER_MASK_DEKU; break;
                default: gSaveContext.save.equippedMask = PLAYER_MASK_NONE; break;
            }
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
    } catch (...) {
        SPDLOG_ERROR("[FleetSync] ExtractShared threw a non-std exception on save");
    }
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
    func_8014546C(&gPlayState->sramCtx);
    Sram_SetFlashPagesOwlSave(&gPlayState->sramCtx,
                              gFlashOwlSaveStartPages[gSaveContext.fileNum * FLASH_SAVE_MAIN_MULTIPLIER],
                              gFlashOwlSaveNumPages[gSaveContext.fileNum * FLASH_SAVE_MAIN_MULTIPLIER]);
    Sram_StartWriteToFlashOwlSave(&gPlayState->sramCtx);
    gSaveContext.save.isOwlSave = prevOwl;
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
    FleetShipCombo_SignalSyncSave(fileNum);
    sWaitingAckSeq = FleetShipCombo_GetSyncSaveSeq();
}

int sHoleGrabCooldown = 0; // frames the fleet hole may not GRAB after an arrival (visible, inert)

void ProcessSignals() {
    if (FleetShipCombo_GetActiveGame() < 0) {
        return;
    }
    // Cross-game restart: the OTHER game reset -> reset ourselves too. DoLocalReset does NOT re-signal,
    // so this never ping-pongs.
    if (FleetShipCombo_ConsumeRestartRequest()) {
        FleetCombo_DoLocalReset();
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
        sLastSeenSyncSeq = seq;
        if (!FleetShipCombo_IsThisGameActive()) {
            nlohmann::json temp;
            if (ReadTemp(temp) && temp.contains("shared")) {
                try {
                    ApplyShared(temp["shared"]);
                } catch (const std::exception& e) {
                    SPDLOG_ERROR("[FleetSync] ApplyShared threw (responder): {}", e.what());
                } catch (...) {
                    SPDLOG_ERROR("[FleetSync] ApplyShared threw a non-std exception (responder)");
                }
            }
            int slot = FleetShipCombo_GetSyncSaveSlot();
            if (slot >= 0 && slot <= 2 && gSaveContext.fileNum == slot) {
                SaveOwnSlotFrozen();
            }
            FleetShipCombo_AckSyncSave(seq);
        }
    }
    if (sWaitingAckSeq != 0 && FleetShipCombo_GetSyncSaveAck() >= sWaitingAckSeq) {
        sWaitingAckSeq = 0;
        DeleteTemp();
    }
}

void RegisterFleetSync() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSaveFile>(HandleOwnSave);
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnGameStateUpdate>(ProcessSignals);
    // Generic FC cross-item applier: grant the native deficit for fcIds obtained elsewhere. Deficit +
    // shadow (comboAppliedFc) makes it idempotent, so it's safe every frame. Gate on gPlayState != NULL
    // because Rando::GiveItem -> Item_Give derefs gPlayState.
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnGameStateUpdate>([]() {
        if (gPlayState != NULL) {
            // Guard: granting the just-obtained check's cross-item (Rando::GiveItem -> ConvertItem) can
            // throw on a bad/missing item; uncaught it would std::terminate 2ship on the frame after a
            // check. Catch + log so 2ship survives and the culprit item is recorded.
            try {
                ApplyFcRegistryToNatives();
            } catch (const std::exception& e) {
                SPDLOG_ERROR("[FleetSync] ApplyFcRegistryToNatives threw: {}", e.what());
            } catch (...) {
                SPDLOG_ERROR("[FleetSync] ApplyFcRegistryToNatives threw a non-std exception");
            }
        }
    });
    // Heal ownedItems poisoned by the pre-fix raw copy the moment ANY save is loaded (the crash
    // was in the pause draw, which can happen before any FleetSync event would run the healer).
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSaveLoad>(
        [](s16 fileNum) { (void)fileNum; HealLeakedOwnedItems(); });
}

void* sFleetHole = nullptr;
bool sHoleFallPending = false;

} // namespace

extern "C" {

void FleetSync_WriteDeparture(int slot) {
    if (FleetShipCombo_GetActiveGame() < 0) {
        return;
    }
    if (slot < 0 || slot > 2) {
        SPDLOG_WARN("[FleetSync] departure with no real file loaded (slot {}) — ignored", slot);
        return; // never anchor/extract an unloaded save (title demo etc.)
    }
    nlohmann::json temp;
    ReadTemp(temp);
    temp["version"] = 1;
    temp["slot"] = slot;
    // Serialize the anchor + shared inside a guard: a check picked up in MM populates rando/FC fields
    // whose (de)serialization can throw (nlohmann .at()/version-mismatch), and an UNCAUGHT throw here
    // was calling std::terminate -> 2ship silently closed on the warp-out. Catch + log so 2ship
    // survives and the exact exception is recorded for a root fix.
    try {
        temp["mm"] = gSaveContext; // full anchor via BenJsonConversions to_json
        nlohmann::json sh = temp.contains("shared") ? temp["shared"] : nlohmann::json::object();
        ExtractShared(sh);
        temp["shared"] = sh;
    } catch (const std::exception& e) {
        temp.erase("mm"); // never persist a half-serialized anchor
        SPDLOG_ERROR("[FleetSync] departure serialization threw: {} (slot {}) — wrote without mm/shared", e.what(),
                     slot);
    } catch (...) {
        temp.erase("mm");
        SPDLOG_ERROR("[FleetSync] departure serialization threw a non-std exception (slot {})", slot);
    }
    WriteTemp(temp);
    SPDLOG_INFO("[FleetSync] MM departure written (slot {})", slot);
}

void FleetSync_ApplyArrival(int slot) {
    if (FleetShipCombo_GetActiveGame() < 0) {
        return;
    }
    nlohmann::json temp;
    if (!ReadTemp(temp)) {
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
            } catch (...) {
                SPDLOG_WARN("[FleetSync] MM anchor unreadable — skipped");
            }
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
            ApplyShared(temp["shared"]);
            SPDLOG_INFO("[FleetSync] shared overlay applied (MM)");
        } catch (const std::exception& e) {
            SPDLOG_ERROR("[FleetSync] ApplyShared threw on arrival: {}", e.what());
        } catch (...) {
            SPDLOG_ERROR("[FleetSync] ApplyShared threw a non-std exception on arrival");
        }
    }
    if (dirty) {
        WriteTemp(temp);
    }
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
