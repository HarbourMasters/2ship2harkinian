// FleetOracle.cpp (MM side) — Oráculo lógico del Combo Randomizer. Ver FleetOracle.h.
//
// Diseño (espejo deliberado de ApplyGlitchlessLogicToSaveContext, SIN colocación):
//   1. Backup completo de gSaveContext (heap; SaveContext es grande).
//   2. Baseline = Sram_InitNewSave() + el bloque pre-lógica de Rando::MiscBehavior::OnFileCreate
//      (SAVETYPE_RANDO, zero del struct rando, South Clock Town humano, sin espada/escudo,
//      opciones desde CVars, starting items + GrantStartingItems). Sram_InitNewSave NO dispara
//      hooks (OnSaveInit lo dispara file select, no la función), así que no se genera nada.
//   3. GiveItem(ConvertItem(...)) por cada item del inventario pedido.
//   4. Crawl fixed-point de regiones/eventos/checks con FindReachableRegions (idéntico al del
//      Glitchless pero sin tocar pools) -> checksInLogic.
//   5. Restore de gSaveContext. El estado del jugador nunca se ve afectado.
//
// gPlayState puede ser null (igual que en OnFileCreate, que corre en file select): la lógica del
// rando solo evalúa estado de save, así que es seguro responder desde el title screen o congelado.

#include "FleetOracle.h"
#include "FleetShipCombo.h"
#include "FleetComboItems.h"
#include "FleetComboItemsGlue.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/Rando/Rando.h"
#include "2s2h/Rando/Logic/Logic.h"
#include "2s2h/Rando/Spoiler/Spoiler.h"
#include "2s2h/SaveManager/SaveManager.h"
#include "2s2h/BenJsonConversions.hpp"
#include "2s2h/BenPort.h" // appShortName (randomizer/ folder of THIS 2ship's app dir)
#include <libultraship/libultraship.h>
#include <libultraship/bridge/consolevariablebridge.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <set>
#include <random>    // mt19937 seeded from the seed (deterministic fillTurn)
#include <algorithm> // std::shuffle
#include <thread>    // sleep_for on the response-rename retry
#include <chrono>
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
#include "sequence.h"                          // SEQ_PLAYER_* (shared audio volumes)
#include "overlays/actors/ovl_En_Si/z_en_si.h" // SPIDER_HOUSE_TOKENS_REQUIRED
uint64_t GetUnixTimestamp();
extern SaveContext gSaveContext;
}

// Toast/cross-sync-record suppression flag, DEFINED in FleetSync.cpp with C linkage (inside its
// extern "C" block, same as GiveItem.cpp's declaration). Must be declared extern "C" here too, else
// the C++-mangled name wouldn't resolve to the C definition (LNK2001). Global scope on purpose —
// inside the anonymous namespace below it would get internal linkage instead (C7631).
extern "C" int gFcCombo_SuppressRecord;

namespace {

// ---- rutas (mismo criterio que FleetSync::TempFilePath: 2ship vive en <ShipDir>/2ship/) ----

std::filesystem::path SelfExeDir() {
#ifdef _WIN32
    wchar_t buf[MAX_PATH];
    DWORD len = GetModuleFileNameW(nullptr, buf, MAX_PATH);
    if (len == 0 || len == MAX_PATH) {
        return {};
    }
    return std::filesystem::canonical(buf).parent_path();
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

// 2ship lives at <ShipDir>/2ship/ -> fleet files live in <ShipDir>/fleet/.
std::filesystem::path OracleFilePath(const char* name) {
    std::filesystem::path dir = SelfExeDir();
    if (dir.empty()) {
        return {};
    }
    std::error_code ec;
    std::filesystem::create_directories(dir.parent_path() / "fleet", ec);
    return dir.parent_path() / "fleet" / name;
}

std::filesystem::path ReqPath() {
    return OracleFilePath("oracle_req.json");
}
std::filesystem::path RespPath() {
    return OracleFilePath("oracle_resp.json");
}

bool ReadJson(const std::filesystem::path& p, nlohmann::json& out) {
    if (p.empty() || !std::filesystem::exists(p)) {
        return false;
    }
    try {
        std::ifstream in(p);
        in >> out;
        return out.is_object();
    } catch (...) { return false; }
}

void WriteJsonAtomic(const std::filesystem::path& p, const nlohmann::json& j) {
    if (p.empty()) {
        return;
    }
    try {
        std::filesystem::path tmp = p;
        tmp += ".tmp2"; // 2ship's own suffix, as in FleetSync, so it never clashes with the host's
        {
            std::ofstream out(tmp);
            out << j << std::endl;
        }
        // Windows: renaming onto oracle_resp.json fails (sharing violation) when the host happens to
        // have it open for reading at that exact moment. Dropping the response there is fatal for the
        // host: it blocks for the full 45s OracleReachable timeout, the pre-placement aborts and the
        // whole fill attempt is retried. With thousands of requests per generation this fired often
        // enough that seeds essentially never finished generating.
        //
        // The host side already retries its own rename for exactly this reason (FleetOracleClient's
        // SendRequest); this mirrors it so the fix covers BOTH directions. Skijer's NEI
        std::error_code ec;
        for (int attempt = 0; attempt < 100; attempt++) {
            std::filesystem::rename(tmp, p, ec);
            if (!ec) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
        if (ec) {
            std::error_code rmec;
            std::filesystem::remove(tmp, rmec); // do not leave the .tmp2 behind
            SPDLOG_WARN("[FleetOracle] failed writing the response (rename: {}) - the host will time out",
                        ec.message());
        }
    } catch (...) { SPDLOG_WARN("[FleetOracle] failed writing the response"); }
}

// ---- shop prices (5.0.0) ----
// GeneratePools rolls a RANDOM price for every shuffled shop / Tingle check, and the wallet gate in
// Logic.h reads it. The manifest freezes the prices rolled at manifest time (they travel to the host,
// which writes them into the combo spoiler as {randoItemId, price}); every later crawl (reachable,
// fillTurn) re-applies THESE prices so the logic the fill was validated against is the logic the
// player gets. Without this the spoiler carried no price at all (every shop item cost 0).
std::map<RandoCheckId, int> sComboPrices;

void ApplyComboPrices() {
    for (auto& [checkId, price] : sComboPrices) {
        RANDO_SAVE_CHECKS[checkId].price = (uint16_t)price;
    }
}

// ---- baseline: replica el bloque pre-lógica de OnFileCreate sobre gSaveContext ----

void BuildRandoBaseline() {
    Sram_InitNewSave();

    gSaveContext.save.shipSaveInfo.saveType = SAVETYPE_RANDO;
    memset(&gSaveContext.save.shipSaveInfo.rando, 0, sizeof(gSaveContext.save.shipSaveInfo.rando));
    memcpy(&gSaveContext.save.shipSaveInfo.rando.foundDungeonKeys, &gSaveContext.save.saveInfo.inventory.dungeonKeys,
           sizeof(gSaveContext.save.saveInfo.inventory.dungeonKeys));

    gSaveContext.save.entrance = ENTRANCE(SOUTH_CLOCK_TOWN, 0);
    gSaveContext.save.cutsceneIndex = 0;
    gSaveContext.save.hasTatl = true;
    gSaveContext.save.playerForm = PLAYER_FORM_HUMAN;
    gSaveContext.save.saveInfo.playerData.threeDayResetCount = 1;
    gSaveContext.save.isFirstCycle = true;
    SET_WEEKEVENTREG(WEEKEVENTREG_59_04);
    SET_WEEKEVENTREG(WEEKEVENTREG_31_04);
    gSaveContext.save.saveInfo.permanentSceneFlags[SCENE_INSIDETOWER].switch0 |= (1 << 0);

    SET_EQUIP_VALUE(EQUIP_TYPE_SWORD, EQUIP_VALUE_SWORD_NONE);
    BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_B) = ITEM_NONE;
    SET_EQUIP_VALUE(EQUIP_TYPE_SHIELD, EQUIP_VALUE_SHIELD_NONE);

    // Opciones desde CVars (idéntico a OnFileCreate; el host configura los CVars de 2ship antes)
    for (auto& [randoOptionId, randoStaticOption] : Rando::StaticData::Options) {
        RANDO_SAVE_OPTIONS[randoOptionId] =
            (uint32_t)CVarGetInteger(randoStaticOption.cvar, randoStaticOption.defaultValue);
    }
    if (!RANDO_SAVE_OPTIONS[RO_SHUFFLE_GOLD_SKULLTULAS]) {
        RANDO_SAVE_OPTIONS[RO_SKULLTULA_TOKENS_REQUIRED] = SPIDER_HOUSE_TOKENS_REQUIRED;
    }
    // 5.0.0: starting with every Stray Fairy means the Great Fairies always have their full set
    // (mirrors OnFileCreate; without it the oracle judged Great Fairy gates against the vanilla
    // requirement while the real save uses the full set).
    if (RANDO_SAVE_OPTIONS[RO_PLACEMENT_STRAY_FAIRIES] == RO_DUNGEON_ITEM_START_WITH) {
        RANDO_SAVE_OPTIONS[RO_STRAY_FAIRIES_REQUIRED] = STRAY_FAIRY_SCATTERED_TOTAL;
    }

    auto startingItems = Rando::GetStartingItemsFromConfig();
    Rando::SetStartingItemsInSave(gSaveContext.save.shipSaveInfo.rando, startingItems);
    Rando::GrantStartingItems();
    ApplyComboPrices();
}

// Is this an ocarina song? MM has no RITYPE_SONG, and the song ids sit in one contiguous alphabetical
// block, so this is the same test Menu.cpp uses for the starting-items song group. RI_PROGRESSIVE_LULLABY
// lives outside the block (it is a progressive chain) and has to be named explicitly. Skijer's NEI
static bool FleetOracle_IsSongItem(RandoItemId itemId) {
    return (itemId >= RI_SONG_DOUBLE_TIME && itemId <= RI_SONG_TIME) || itemId == RI_PROGRESSIVE_LULLABY;
}

// Is this a dungeon reward? MM's four boss remains — the counterpart of OoT's 6 medallions and 3
// spiritual stones. The ids are contiguous but the block is alphabetical and could grow, so they are
// named one by one rather than range-tested. Skijer's NEI
static bool FleetOracle_IsRewardItem(RandoItemId itemId) {
    return itemId == RI_REMAINS_ODOLWA || itemId == RI_REMAINS_GOHT || itemId == RI_REMAINS_GYORG ||
           itemId == RI_REMAINS_TWINMOLD;
}

// SHARED RESTRICTED CATEGORIES. Each one is a set of items that belongs to a matching set of spots in
// both games; the host asks for MM's half through the manifest and then places across both. `key`
// must match gFcCategories in the host's FleetComboRando.cpp — it is the wire contract. Adding a
// category is one row here and one row there. Skijer's NEI
struct FleetOracleCategory {
    const char* key;
    bool (*isItem)(RandoItemId);
};
static const FleetOracleCategory sOracleCategories[] = {
    { "songs", FleetOracle_IsSongItem },
    { "rewards", FleetOracle_IsRewardItem },
};

// ---- ops ----

nlohmann::json HandleManifest() {
    nlohmann::json resp;
    BuildRandoBaseline();

    std::vector<RandoCheckId> checkPool;
    std::vector<RandoItemId> itemPool;
    Rando::Logic::GeneratePools(gSaveContext.save.shipSaveInfo.rando, checkPool, itemPool);

    // Freeze this generation's shop prices (see sComboPrices) and publish them, plus the checks
    // 5.0.0's "excluded checks" turned into skipped junk (they are NOT in the check pool, but their
    // vanilla item WAS put back into the item pool — the host must write them into the spoiler as
    // skipped junk, or the spoiler-apply gives them their vanilla item AGAIN = duplicate).
    sComboPrices.clear();
    nlohmann::json checkPrices = nlohmann::json::object();
    nlohmann::json skippedChecks = nlohmann::json::array();
    for (auto& [checkId, staticCheck] : Rando::StaticData::Checks) {
        if (staticCheck.randoCheckId == RC_UNKNOWN) {
            continue;
        }
        if (staticCheck.randoCheckType == RCTYPE_SHOP || staticCheck.randoCheckType == RCTYPE_TINGLE_SHOP) {
            sComboPrices[checkId] = RANDO_SAVE_CHECKS[checkId].price;
            checkPrices[staticCheck.name] = RANDO_SAVE_CHECKS[checkId].price;
        }
        if (RANDO_SAVE_CHECKS[checkId].skipped) {
            skippedChecks.push_back(staticCheck.name);
        }
    }

    nlohmann::json checks = nlohmann::json::array();
    // Readable area name per check (RC_name -> "Woodfall Temple"). OoT needs this to generate hints
    // pointing at items placed in MM: without it its generator resolves RC_UNKNOWN_CHECK and emits
    // "Invalid Location", which ends up leaving raw [[N]] tokens on the altar. Skijer's NEI
    nlohmann::json checkAreas = nlohmann::json::object();
    // MM's own song spots, for the combo's shared "Song Spots" mode.
    //
    // Identified by the check's VANILLA ITEM, not by randoCheckType: the two Goron Lullaby checks
    // (RC_GORON_SHRINE_FULL_LULLABY and RC_PATH_TO_GORON_VILLAGE_LULLABY_INTRO) are typed RCTYPE_NPC,
    // so filtering on RCTYPE_SONG silently found 9 spots instead of 11. There is no RITYPE_SONG in
    // MM; the range test below is the idiom the codebase already uses (Menu.cpp). Skijer's NEI
    nlohmann::json songChecks = nlohmann::json::array();
    nlohmann::json categoryChecks = nlohmann::json::object();
    for (auto& cat : sOracleCategories) {
        categoryChecks[cat.key] = nlohmann::json::array();
    }
    for (RandoCheckId checkId : checkPool) {
        checks.push_back({ (int)checkId, Rando::StaticData::Checks[checkId].name });
        for (auto& cat : sOracleCategories) {
            if (cat.isItem(Rando::StaticData::Checks[checkId].randoItemId)) {
                categoryChecks[cat.key].push_back(Rando::StaticData::Checks[checkId].name);
            }
        }
        if (FleetOracle_IsSongItem(Rando::StaticData::Checks[checkId].randoItemId)) {
            songChecks.push_back(Rando::StaticData::Checks[checkId].name);
        }

        // GetLocationNameForHint returns "in <Area>"; the host only wants the area.
        std::string area = Rando::StaticData::GetLocationNameForHint(checkId, false);
        if (area.rfind("in ", 0) == 0) {
            area = area.substr(3);
        }
        checkAreas[Rando::StaticData::Checks[checkId].name] = area;
    }
    // Pool con categoría: [spoilerName, "prog"|"junk"|"health"|"trap"] — el host clasifica
    // progresión vs relleno con esto (strings estables, no ints de enum que pueden driftear).
    nlohmann::json pool = nlohmann::json::array();
    for (RandoItemId itemId : itemPool) {
        auto& staticItem = Rando::StaticData::Items[itemId];
        const char* category = "prog";
        if (itemId == RI_TRAP) { // no existe RITYPE_TRAP; el trap se identifica por item
            category = "trap";
        } else if (staticItem.randoItemType == RITYPE_JUNK) {
            category = "junk";
        } else if (staticItem.randoItemType == RITYPE_HEALTH) {
            category = "health";
        }
        pool.push_back({ staticItem.spoilerName, category });
    }
    nlohmann::json starting = nlohmann::json::array();
    for (RandoItemId itemId : Rando::GetStartingItemsFromSave(gSaveContext.save.shipSaveInfo.rando)) {
        starting.push_back(Rando::StaticData::Items[itemId].spoilerName);
    }
    nlohmann::json options = nlohmann::json::array();
    for (auto& [randoOptionId, randoStaticOption] : Rando::StaticData::Options) {
        options.push_back({ randoStaticOption.name, randoStaticOption.cvar, RANDO_SAVE_OPTIONS[randoOptionId] });
    }

    resp["checks"] = checks;
    resp["checkAreas"] = checkAreas;
    resp["checkPrices"] = checkPrices;     // shop/Tingle: RC name -> rupee price rolled for this seed
    resp["skippedChecks"] = skippedChecks; // 5.0.0 excluded checks the spoiler must mark as skipped junk
    // songChecks is the old songs-only field, kept so a host built before categoryChecks still gets
    // its song spots instead of silently placing every song inside Hyrule. Skijer's NEI
    resp["songChecks"] = songChecks;
    resp["categoryChecks"] = categoryChecks;
    resp["pool"] = pool;
    resp["startingItems"] = starting;
    resp["options"] = options;
    SPDLOG_INFO("[FleetOracle] manifest: {} checks, {} items en pool", checks.size(), pool.size());
    return resp;
}

// op "setOptions": el host empuja valores a CVars de 2ship y persiste. Whitelist de prefijos
// (rando + contenido compartido NEI/enhancements/cheats) para que el canal no pueda tocar
// cualquier cosa. Así el tab Shared de Ship edita las variables de MM sin tocar su GUI.
bool SetOptionsCvarAllowed(const std::string& cvar) {
    static const char* kAllowedPrefixes[] = { "gRando.",  "gMods.",     "gEnhancements.",
                                              "gCheats.", "gSettings.", "gNotifications." };
    for (const char* prefix : kAllowedPrefixes) {
        if (cvar.rfind(prefix, 0) == 0) {
            return true;
        }
    }
    // LUS graphics/nav cvars que NO llevan prefijo gSettings (shared menu los vincula).
    static const char* kAllowedExact[] = {
        "gInternalResolution", "gMSAAValue",  "gVsyncEnabled",     "gSdlWindowedFullscreen", "gEnableMultiViewports",
        "gTextureFilter",      "gControlNav", "gInterpolationFPS", "gMatchRefreshRate"
    };
    for (const char* exact : kAllowedExact) {
        if (cvar == exact) {
            return true;
        }
    }
    return false;
}

nlohmann::json HandleSetOptions(const nlohmann::json& req) {
    nlohmann::json resp;
    int applied = 0;
    if (req.contains("cvars") && req["cvars"].is_array()) {
        for (auto& pair : req["cvars"]) {
            std::string cvar = pair[0].get<std::string>();
            int value = pair[1].get<int>();
            if (!SetOptionsCvarAllowed(cvar)) {
                SPDLOG_WARN("[FleetOracle] setOptions rechaza cvar fuera del whitelist: {}", cvar);
                continue;
            }
            CVarSetInteger(cvar.c_str(), value);
            applied++;
        }
    }
    if (req.contains("cvarsFloat") && req["cvarsFloat"].is_array()) {
        for (auto& pair : req["cvarsFloat"]) {
            std::string cvar = pair[0].get<std::string>();
            float value = pair[1].get<float>();
            if (!SetOptionsCvarAllowed(cvar)) {
                SPDLOG_WARN("[FleetOracle] setOptions rechaza cvar float fuera del whitelist: {}", cvar);
                continue;
            }
            CVarSetFloat(cvar.c_str(), value);
            // Master se lee cada frame del cvar; los per-player deben aplicarse al sequencer
            // (mismo apply que el callback del slider de BenMenu).
            if (cvar == "gSettings.Audio.MainMusicVolume") {
                AudioSeq_SetPortVolumeScale(SEQ_PLAYER_BGM_MAIN, value);
            } else if (cvar == "gSettings.Audio.SubMusicVolume") {
                AudioSeq_SetPortVolumeScale(SEQ_PLAYER_BGM_SUB, value);
            } else if (cvar == "gSettings.Audio.SoundEffectsVolume") {
                AudioSeq_SetPortVolumeScale(SEQ_PLAYER_SFX, value);
            } else if (cvar == "gSettings.Audio.FanfareVolume") {
                AudioSeq_SetPortVolumeScale(SEQ_PLAYER_FANFARE, value);
            }
            applied++;
        }
    }
    CVarSave();
    resp["applied"] = applied;
    SPDLOG_INFO("[FleetOracle] setOptions: {} cvars aplicados", applied);
    return resp;
}

// op "prepareSeed" (Fase 3): el host escribió el spoiler combo en <ShipDir>/fleet_oracle_spoiler.json;
// lo guardamos con NUESTRO SaveToFile (resuelve el appdir real de 2ship: <appdir>/randomizer/<file>),
// activamos el rando y dejamos gRando.SpoilerFile apuntando al archivo. Al crear el save pareado,
// OnFileCreate toma la rama SpoilerFileIndex != 0 y aplica el spoiler (mecanismo 100% vanilla).
nlohmann::json HandlePrepareSeed(const nlohmann::json& req) {
    nlohmann::json resp;
    std::string fileName = req.value("file", "");
    if (fileName.empty()) {
        throw std::runtime_error("prepareSeed: missing file name");
    }

    nlohmann::json spoiler;
    std::filesystem::path bridge = OracleFilePath("oracle_spoiler.json");
    if (!ReadJson(bridge, spoiler)) {
        throw std::runtime_error("prepareSeed: could not read fleet/oracle_spoiler.json");
    }
    if (!spoiler.contains("type") || spoiler["type"] != "2S2H_RANDO_SPOILER") {
        throw std::runtime_error("prepareSeed: spoiler is not 2S2H_RANDO_SPOILER");
    }
    // 5.0.0's ApplyToSaveContext reads spoiler["sariaPriorityItems"] with no guard (a null there is
    // a json type_error -> OnFileCreate's catch turns the slot into an INVALID vanilla file). Older
    // hosts (and hand-made spoilers) don't emit the key: default it here so a combo seed can never
    // fail to apply over a field the combo doesn't even use.
    if (!spoiler.contains("sariaPriorityItems") || !spoiler["sariaPriorityItems"].is_array()) {
        spoiler["sariaPriorityItems"] = nlohmann::json::array();
    }

    Rando::Spoiler::SaveToFile(fileName, spoiler);
    CVarSetInteger("gRando.Enabled", 1);
    CVarSetString("gRando.SpoilerFile", fileName.c_str());
    Rando::Spoiler::RefreshOptions(); // sincroniza gRando.SpoilerFileIndex con el archivo
    CVarSave();

    int index = CVarGetInteger("gRando.SpoilerFileIndex", 0);
    if (index == 0) {
        throw std::runtime_error("prepareSeed: RefreshOptions did not find the saved spoiler");
    }
    resp["spoilerIndex"] = index;
    resp["file"] = fileName;
    SPDLOG_INFO("[FleetOracle] prepareSeed OK: {} (index {})", fileName, index);
    return resp;
}

// finalSeed of a spoiler on disk in <appdir>/randomizer/, 0 if unreadable / not a spoiler.
unsigned int SpoilerFinalSeed(const std::string& fileName) {
    try {
        nlohmann::json s = Rando::Spoiler::LoadFromFile(fileName);
        return s.value("finalSeed", 0u);
    } catch (...) { return 0; }
}

// Make sure the spoiler 2ship will APPLY on the next createSave is the one for `expected` (the
// combo seed OoT published). The prepared spoiler (gRando.SpoilerFile) is simply "the last one
// prepareSeed installed", which is the last seed GENERATED or LOADED — not necessarily the seed of
// the OoT file the player just picked. Loading an older combo file therefore rebuilt MM's slot on
// the wrong fill and looped on seedMismatch forever ("MM no pudo parear el slot"). Every combo seed
// ever prepared here still sits in randomizer/ as combo_<inputSeed>.json, so look for one carrying
// this finalSeed and select it. Returns true when the prepared spoiler now matches.
bool PrepareSpoilerForComboSeed(unsigned int expected) {
    if (expected == 0) {
        return true; // no seed published: nothing to match against
    }
    std::string current = CVarGetString("gRando.SpoilerFile", "");
    if (!current.empty() && CVarGetInteger("gRando.SpoilerFileIndex", 0) != 0 &&
        SpoilerFinalSeed(current) == expected) {
        return true;
    }
    std::error_code ec;
    std::filesystem::path dir = Ship::Context::GetPathRelativeToAppDirectory("randomizer", appShortName);
    for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".json") {
            continue;
        }
        std::string fileName = entry.path().filename().string();
        if (fileName.rfind("combo_ootareas_", 0) == 0) {
            continue; // hint sidecar, not a spoiler
        }
        if (SpoilerFinalSeed(fileName) != expected) {
            continue;
        }
        CVarSetInteger("gRando.Enabled", 1);
        CVarSetString("gRando.SpoilerFile", fileName.c_str());
        Rando::Spoiler::RefreshOptions();
        CVarSave();
        bool ok = CVarGetInteger("gRando.SpoilerFileIndex", 0) != 0;
        SPDLOG_INFO("[FleetOracle] combo seed {}: selected spoiler {} from randomizer/ (index {})", expected, fileName,
                    CVarGetInteger("gRando.SpoilerFileIndex", 0));
        return ok;
    }
    SPDLOG_WARN("[FleetOracle] combo seed {}: no spoiler in randomizer/ carries it (prepared: '{}')", expected,
                current);
    return false;
}

// ASCII -> charset del file select (dígitos 0-9, A-Z=10..35, a-z=36..61, espacio=62)
void EncodeSaveName(const std::string& name, u8 out[8]) {
    for (int i = 0; i < 8; i++) {
        out[i] = 62; // espacio
        if (i >= (int)name.size()) {
            continue;
        }
        char c = name[i];
        if (c >= '0' && c <= '9') {
            out[i] = (u8)(c - '0');
        } else if (c >= 'A' && c <= 'Z') {
            out[i] = (u8)(10 + c - 'A');
        } else if (c >= 'a' && c <= 'z') {
            out[i] = (u8)(36 + c - 'a');
        }
    }
}

// op "createSave" (botón "Crear saves combo"): crea el save de MM en el slot con el spoiler combo
// aplicado y lo escribe a disco DIRECTO vía SaveManager_WriteSaveFile (overwrite total de
// <AppDir>/saves/file{slot+1}.json). Funciona en CUALQUIER estado (title/frozen/gameplay); no
// depende de PlayState ni del mapeo de páginas de flash. El estado vivo del jugador se respalda y
// restaura: solo cambia el archivo del slot.
nlohmann::json HandleCreateSave(const nlohmann::json& req) {
    int slot = req.value("slot", -1);
    std::string name = req.value("name", "Link");
    if (slot < 0 || slot > 2) {
        throw std::runtime_error("createSave: invalid slot");
    }

    // The slot is only useful if OnFileCreate APPLIES THE COMBO SPOILER. That takes gRando.Enabled
    // (the player can switch it off in 2ship's own menu -> a VANILLA MM file would be written and
    // "File N isn't a rando") and a prepared spoiler (SpoilerFileIndex == 0 makes OnFileCreate roll a
    // brand-new RANDOM seed -> "MM plays another seed"). Refuse loudly instead of writing either.
    CVarSetInteger("gRando.Enabled", 1);
    if (CVarGetInteger("gRando.SpoilerFileIndex", 0) == 0) {
        Rando::Spoiler::RefreshOptions(); // re-sync the index with gRando.SpoilerFile
    }
    if (CVarGetInteger("gRando.SpoilerFileIndex", 0) == 0) {
        throw std::runtime_error("createSave: no combo spoiler prepared in this 2ship (prepareSeed first)");
    }
    // And it has to be THIS combo's seed when one is published (see PrepareSpoilerForComboSeed).
    if (!PrepareSpoilerForComboSeed(FleetShipCombo_GetComboSeed())) {
        throw std::runtime_error("createSave: the prepared spoiler is for another seed and no spoiler for combo "
                                 "seed " +
                                 std::to_string(FleetShipCombo_GetComboSeed()) +
                                 " exists in randomizer/ (reload this combo's .fleet from OoT)");
    }

    auto backup = std::make_unique<SaveContext>();
    memcpy(backup.get(), &gSaveContext, sizeof(SaveContext));

    nlohmann::json resp;
    try {
        gSaveContext.fileNum = slot;
        Sram_InitNewSave();
        u8 encoded[8];
        EncodeSaveName(name, encoded);
        memcpy(gSaveContext.save.saveInfo.playerData.playerName, encoded, 8);
        // IS_VALID_FILE exige newf == "ZELDA3" o SaveManager BORRA el archivo en vez de escribirlo
        gSaveContext.save.saveInfo.playerData.newf[0] = 'Z';
        gSaveContext.save.saveInfo.playerData.newf[1] = 'E';
        gSaveContext.save.saveInfo.playerData.newf[2] = 'L';
        gSaveContext.save.saveInfo.playerData.newf[3] = 'D';
        gSaveContext.save.saveInfo.playerData.newf[4] = 'A';
        gSaveContext.save.saveInfo.playerData.newf[5] = '3';

        // OnSaveInit dispara Rando::MiscBehavior::OnFileCreate -> aplica el spoiler combo
        GameInteractor::Instance->ExecuteHooks<GameInteractor::OnSaveInit>((s16)slot);
        bool randoApplied = gSaveContext.save.shipSaveInfo.saveType == SAVETYPE_RANDO;
        // OnFileCreate's own catch (spoiler failed to apply) leaves saveType VANILLA and newf[0]=0.
        // Writing that would give the player an invalid or vanilla "File N" — refuse; the host logs
        // it as a failed save op and the next ensureSave retries.
        if (!randoApplied || gSaveContext.save.saveInfo.playerData.newf[0] != 'Z') {
            throw std::runtime_error("createSave: the combo spoiler did not apply (see 'Seed Failure' above)");
        }
        // Belt and braces: the file must carry the seed OoT published, or the two games desync.
        unsigned int expected = FleetShipCombo_GetComboSeed();
        if (expected != 0 && gSaveContext.save.shipSaveInfo.rando.finalSeed != expected) {
            throw std::runtime_error("createSave: applied spoiler finalSeed " +
                                     std::to_string(gSaveContext.save.shipSaveInfo.rando.finalSeed) +
                                     " != combo seed " + std::to_string(expected));
        }

        gSaveContext.save.saveInfo.checksum = 0;
        gSaveContext.save.saveInfo.checksum = Sram_CalcChecksum(&gSaveContext.save, sizeof(Save));

        // Escritura directa por SaveManager (mismo JSON que SaveManager_SysFlashrom_WriteData arma
        // para un NEW_CYCLE_SAVE). Overwrite total: no preservamos owl save previo.
        nlohmann::json j;
        j["newCycleSave"]["save"] = gSaveContext.save;
        j["version"] = 7; // CURRENT_SAVE_VERSION (SaveManager.cpp); un save viejo se migra al cargar
        j["type"] = "2S2H_SAVE";
        SaveManager_WriteSaveFile(SaveManager_GetFileName(slot + 1), j);

        resp["randoApplied"] = randoApplied;
        resp["slot"] = slot;
        SPDLOG_INFO("[FleetOracle] createSave OK: file{}.json (rando={})", slot + 1, randoApplied);
    } catch (...) {
        memcpy(&gSaveContext, backup.get(), sizeof(SaveContext));
        throw;
    }
    memcpy(&gSaveContext, backup.get(), sizeof(SaveContext));
    return resp;
}

// ---- Save pairing (OoT is the owner, MM's files are DERIVED) ----------------------------------
// A combo file is ONE file living in two processes: OoT's slot N and MM's slot N are the same save.
// So MM must never keep a slot OoT no longer has, and must never play a slot built for a different
// seed than the one OoT is running -- that is how you end up with two games handing out items from
// two different fills, which looks like nothing at all until hours later.

// Delete MM's file for `slot` (and its backup). Called for the pair when OoT erases its own file.
void DeleteSlotFiles(int slot) {
    SaveManager_DeleteSaveFile(SaveManager_GetFileName(slot + 1));
    SaveManager_DeleteSaveFile(SaveManager_GetFileName(slot + 1, true)); // backup copy
}

// Does the file on disk carry the seed OoT published for this combo? Reads the slot straight off
// disk (nothing is loaded at the point we ask) and only reports a rebuild when it can PROVE the
// slot is unusable: missing, unreadable, or stamped with another seed. "We cannot tell" (no seed
// published yet, a non-rando save) is never a rebuild -- guessing here destroys real progress.
bool SlotNeedsRebuild(int slot) {
    nlohmann::json j;
    if (SaveManager_ReadSaveFile(SaveManager_GetFileName(slot + 1), j) != 0) {
        return true; // no file at all, or garbage: OoT has a combo file, MM must have its half
    }
    const unsigned int expected = FleetShipCombo_GetComboSeed();
    if (expected == 0) {
        return false; // no combo seed published yet: nothing to validate against
    }
    try {
        const auto& rando = j.at("newCycleSave").at("save").at("shipSaveInfo").at("rando");
        return rando.at("finalSeed").get<unsigned int>() != expected;
    } catch (...) {
        // With a combo seed published, MM's half of the slot MUST be a rando save carrying it. A
        // vanilla save (or any shape without a finalSeed) sitting there is simply not the other
        // half of OoT's file, so it gets rebuilt rather than played as if it were.
        return true;
    }
}

// op "deleteSave": OoT erased its file in this slot -> drop MM's half of the pair.
nlohmann::json HandleDeleteSave(const nlohmann::json& req) {
    int slot = req.value("slot", -1);
    if (slot < 0 || slot > 2) {
        throw std::runtime_error("deleteSave: invalid slot");
    }
    DeleteSlotFiles(slot);
    SPDLOG_INFO("[FleetOracle] deleteSave OK: MM file{}.json removed (OoT erased its half)", slot + 1);
    nlohmann::json resp;
    resp["slot"] = slot;
    return resp;
}

// op "wipeSaves": the combo was just switched ON. Every MM save predating the combo belongs to a
// game OoT knows nothing about, so none of them has a valid OoT counterpart -- wipe the lot and let
// the pairing rebuild them from OoT's seed. Ship only sends this on the OFF->ON transition.
nlohmann::json HandleWipeSaves(const nlohmann::json& req) {
    (void)req;
    for (int slot = 0; slot < 3; slot++) {
        DeleteSlotFiles(slot);
    }
    SPDLOG_INFO("[FleetOracle] wipeSaves OK: MM file1/2/3 removed (combo just enabled)");
    nlohmann::json resp;
    resp["wiped"] = 3;
    return resp;
}

// op "ensureSave": OoT is about to play its combo file in this slot -> guarantee MM's half exists
// and belongs to the SAME seed, rebuilding it when it does not. This is the "no match, no file"
// repair: the alternative is OoT running seed A while MM runs seed B.
nlohmann::json HandleEnsureSave(const nlohmann::json& req) {
    int slot = req.value("slot", -1);
    if (slot < 0 || slot > 2) {
        throw std::runtime_error("ensureSave: invalid slot");
    }
    nlohmann::json resp;
    resp["slot"] = slot;
    if (!SlotNeedsRebuild(slot)) {
        resp["rebuilt"] = false;
        return resp;
    }
    SPDLOG_WARN("[FleetOracle] ensureSave: MM slot {} is missing or built for another seed -> rebuilding", slot);
    // The rebuild must use THIS combo's spoiler, not "whatever prepareSeed installed last". Select the
    // spoiler carrying the published seed (any combo seed ever prepared here is still in randomizer/);
    // if none exists, keep the old file rather than replacing it with a wrong-seed one, and say so
    // — silent is exactly how the two games end up on different fills.
    if (!PrepareSpoilerForComboSeed(FleetShipCombo_GetComboSeed())) {
        resp["rebuilt"] = false;
        resp["seedMismatch"] = true;
        SPDLOG_ERROR("[FleetOracle] ensureSave: no spoiler for combo seed {} in this 2ship -> slot {} NOT rebuilt "
                     "(OoT must send prepareSeed from this combo's .fleet first)",
                     FleetShipCombo_GetComboSeed(), slot);
        return resp;
    }
    DeleteSlotFiles(slot); // drop the stale half first: createSave overwrites the file, not the backup
    nlohmann::json createReq;
    createReq["slot"] = slot;
    createReq["name"] = req.value("name", "LINK");
    HandleCreateSave(createReq); // throws (-> error response) if the spoiler didn't apply
    resp["rebuilt"] = true;
    if (SlotNeedsRebuild(slot)) {
        resp["seedMismatch"] = true;
        SPDLOG_ERROR("[FleetOracle] ensureSave: MM slot {} STILL does not match combo seed {} after the rebuild", slot,
                     FleetShipCombo_GetComboSeed());
    }
    return resp;
}

void GiveOneItem(RandoItemId randoItemId) {
    if (randoItemId <= RI_UNKNOWN || randoItemId >= RI_MAX) {
        return;
    }
    Rando::GiveItem(Rando::ConvertItem(randoItemId));
}

nlohmann::json HandleReachable(const nlohmann::json& req) {
    nlohmann::json resp;
    BuildRandoBaseline();

    // Inventario asumido: items FC compartidos (resueltos a RI vía la tabla) + items nativos MM
    if (req.contains("fcItems") && req["fcItems"].is_array()) {
        for (auto& pair : req["fcItems"]) {
            int fcId = pair[0].get<int>();
            int count = pair[1].get<int>();
            int ri = FcCombo_NativeForItem(fcId);
            if (ri == FCI_NO_ITEM) {
                continue; // item compartido sin relative MM todavía: no aporta lógica local
            }
            for (int i = 0; i < count; i++) {
                GiveOneItem((RandoItemId)ri);
            }
        }
    }
    if (req.contains("mmItems") && req["mmItems"].is_array()) {
        for (auto& pair : req["mmItems"]) {
            std::string name = pair[0].get<std::string>();
            int count = pair[1].get<int>();
            RandoItemId ri = Rando::StaticData::GetItemIdFromName(name.c_str());
            if (ri == RI_UNKNOWN) {
                SPDLOG_WARN("[FleetOracle] item MM desconocido en request: {}", name);
                continue;
            }
            for (int i = 0; i < count; i++) {
                GiveOneItem(ri);
            }
        }
    }

    // PRE-PLACED ITEMS (restricted stages: shared songs, dungeon rewards). They are physically in
    // MM's checks already, but nothing here knew that — so MM's logic could never reach anything
    // gated behind them, and could never tell OoT it had got there either. They are NOT part of the
    // assumed inventory: an item in a check is only yours once you can reach the check, which is
    // exactly how a plando placement behaves. The crawl below hands each one over on arrival and
    // keeps iterating, so one of them opening the way to the next resolves on its own. Skijer's NEI
    std::map<std::string, RandoItemId> prePlaced;
    if (req.contains("prePlaced") && req["prePlaced"].is_object()) {
        for (auto& [checkName, itemName] : req["prePlaced"].items()) {
            RandoItemId ri = Rando::StaticData::GetItemIdFromName(itemName.get<std::string>().c_str());
            if (ri == RI_UNKNOWN) {
                SPDLOG_WARN("[FleetOracle] pre-placed item desconocido: {}", itemName.get<std::string>());
                continue;
            }
            prePlaced[checkName] = ri;
        }
    }
    std::set<std::string> prePlacedReached;

    // Crawl fixed-point (mismo esqueleto que GlitchlessLogic, sin colocación ni junk-swap).
    // Los time states se inicializan DESPUÉS de dar los items, igual que en OnFileCreate
    // (starting items primero, InitializeRegionTimeStates ya refleja el tiempo poseído).
    uint64_t tick = GetUnixTimestamp();
    std::set<RandoRegionId> regionsInLogic = { RR_MAX };
    std::set<RandoCheckId> checksInLogic;
    std::set<std::pair<RandoEvent, std::function<bool()>>*> eventsInLogic;
    auto regionTimeStates = Rando::Logic::InitializeRegionTimeStates(RR_MAX);

    while (true) {
        if (GetUnixTimestamp() - tick > 10000) {
            throw std::runtime_error("Oracle crawl took too long, aborting");
        }

        bool changed = false;

        auto prevRegionsInLogicSize = regionsInLogic.size();
        for (RandoRegionId regionId : regionsInLogic) {
            Rando::Logic::FindReachableRegions(regionId, regionsInLogic, regionTimeStates);
        }
        if (regionsInLogic.size() != prevRegionsInLogicSize) {
            changed = true;
        }

        for (RandoRegionId regionId : regionsInLogic) {
            auto& randoRegion = Rando::Logic::Regions[regionId];
            Rando::Logic::SetCurrentRegionTime(regionTimeStates, regionId);

            for (auto& randoEvent : randoRegion.events) {
                if (!eventsInLogic.contains(&randoEvent) && randoEvent.second()) {
                    RANDO_EVENTS[randoEvent.first]++;
                    eventsInLogic.insert(&randoEvent);
                    changed = true;
                }
            }
            for (auto& [randoCheckId, checkLogic] : randoRegion.checks) {
                if (!checksInLogic.contains(randoCheckId) && checkLogic.first()) {
                    checksInLogic.insert(randoCheckId);
                    changed = true;
                    // Reached a check that already holds something: collect it. `changed` is already
                    // true, so the loop runs again and whatever this item unlocks is picked up.
                    auto pre = prePlaced.find(Rando::StaticData::Checks[randoCheckId].name);
                    if (pre != prePlaced.end() && !prePlacedReached.contains(pre->first)) {
                        prePlacedReached.insert(pre->first);
                        GiveOneItem(pre->second);
                    }
                }
            }
        }

        if (!changed) {
            break;
        }
    }

    nlohmann::json reachable = nlohmann::json::array();
    for (RandoCheckId checkId : checksInLogic) {
        reachable.push_back((int)checkId);
    }
    resp["reachable"] = reachable;
    // Whether MM's end of the portal (the fleet hole in South Clock Town) is standable with this
    // inventory. The cross-game playthrough verifier needs it to know when Hyrule opens up.
    resp["portalReachable"] = regionsInLogic.contains(RR_CLOCK_TOWN_SOUTH);
    // Which pre-placed spots MM actually stood on. This is MM saying "I got to this one" — the host
    // turns it into an announcement so OoT may assume that item from now on, and not a moment sooner.
    nlohmann::json reachedPre = nlohmann::json::array();
    for (auto& name : prePlacedReached) {
        reachedPre.push_back(name);
    }
    resp["prePlacedReached"] = reachedPre;
    SPDLOG_INFO("[FleetOracle] reachable: {} checks alcanzables, {} de {} pre-colocados alcanzados", reachable.size(),
                prePlacedReached.size(), prePlaced.size());
    return resp;
}

// ---- fillTurn: ONE TURN of the delegated fill ----
//
// The combo no longer places MM's items: it asks MM to place them. Each turn MM receives (a) the
// inventory it may ASSUME (what OoT already placed and is reachable — "kept promises" — plus what MM
// itself has placed so far), (b) the items still assigned to it, and (c) the checks already taken in
// previous turns. It places everything that fits in checks reachable RIGHT NOW and returns.
//
// Why placing everything reachable at once is safe: putting an item into an ALREADY reachable
// location can never make that location unreachable, so there is no need to re-crawl between items.
// Spheres advance on their own — each turn starts from a larger assumed inventory.
//
// Stateless across calls: the request carries `used` and the RNG is seeded from the seed, so the same
// seed always produces the same game. Skijer's NEI
nlohmann::json HandleFillTurn(const nlohmann::json& req) {
    nlohmann::json resp;
    BuildRandoBaseline();

    // (a) what OoT has ANNOUNCED it already placed, as [RI name, count]. Facts, not promises: every
    // entry is a copy already sitting in a slot OoT could reach, so MM is entitled to assume it. The
    // host never sends the shared pool wholesale — an item MM has not been told about does not exist
    // as far as this turn is concerned. Skijer's NEI
    std::vector<std::pair<std::string, int>> assumed;
    if (req.contains("assumed") && req["assumed"].is_array()) {
        for (auto& pair : req["assumed"]) {
            assumed.push_back({ pair[0].get<std::string>(), pair[1].get<int>() });
        }
    }

    // (b) items MM still owes, expanded one entry per copy
    std::vector<std::string> toPlace;
    if (req.contains("toPlace") && req["toPlace"].is_array()) {
        for (auto& pair : req["toPlace"]) {
            for (int i = 0, n = pair[1].get<int>(); i < n; i++) {
                toPlace.push_back(pair[0].get<std::string>());
            }
        }
    }

    // (c) checks already taken in previous turns
    std::set<std::string> used;
    if (req.contains("used") && req["used"].is_array()) {
        for (auto& name : req["used"]) {
            used.insert(name.get<std::string>());
        }
    }

    // Only MM's SHUFFLED checks are valid (the pool depends on MM's own options).
    std::vector<RandoCheckId> checkPool;
    std::vector<RandoItemId> itemPool;
    Rando::Logic::GeneratePools(gSaveContext.save.shipSaveInfo.rando, checkPool, itemPool);
    ApplyComboPrices(); // GeneratePools re-rolls shop prices; the crawl must see the manifest's
    std::set<RandoCheckId> shuffled(checkPool.begin(), checkPool.end());

    std::mt19937 rng((uint32_t)req.value("rngSeed", 0u));
    std::shuffle(toPlace.begin(), toPlace.end(), rng);

    // Nothing needs ordering last any more: restricted-category items (songs, dungeon rewards) are
    // placed by the host's own stage before the split and never reach a fill turn. Skijer's NEI

    // Same pre-placed handling as the reachable op: items the restricted stages already dropped into
    // MM's checks are collected ON ARRIVAL, not assumed. Without this the fill turn is blind to them
    // and reports slots unreachable that a player could open. Skijer's NEI
    std::map<std::string, RandoItemId> prePlaced;
    if (req.contains("prePlaced") && req["prePlaced"].is_object()) {
        for (auto& [checkName, itemName] : req["prePlaced"].items()) {
            RandoItemId ri = Rando::StaticData::GetItemIdFromName(itemName.get<std::string>().c_str());
            if (ri != RI_UNKNOWN) {
                prePlaced[checkName] = ri;
            }
        }
    }
    std::set<std::string> prePlacedReached;

    // Reachability crawl under a given assumed inventory. Rebuilds the baseline every call because
    // GiveOneItem mutates the save; the whole thing is restored by the caller afterwards.
    auto crawlReachable = [&](const std::vector<std::string>& inventory) {
        BuildRandoBaseline();
        for (auto& name : inventory) {
            RandoItemId ri = Rando::StaticData::GetItemIdFromName(name.c_str());
            if (ri != RI_UNKNOWN) {
                GiveOneItem(ri);
            }
        }
        std::set<std::string> reachedHere;
        uint64_t tick = GetUnixTimestamp();
        std::set<RandoRegionId> regionsInLogic = { RR_MAX };
        std::set<RandoCheckId> checksInLogic;
        std::set<std::pair<RandoEvent, std::function<bool()>>*> eventsInLogic;
        auto regionTimeStates = Rando::Logic::InitializeRegionTimeStates(RR_MAX);
        while (true) {
            if (GetUnixTimestamp() - tick > 10000) {
                throw std::runtime_error("fillTurn crawl took too long, aborting");
            }
            bool changed = false;
            auto prevSize = regionsInLogic.size();
            for (RandoRegionId regionId : regionsInLogic) {
                Rando::Logic::FindReachableRegions(regionId, regionsInLogic, regionTimeStates);
            }
            if (regionsInLogic.size() != prevSize) {
                changed = true;
            }
            for (RandoRegionId regionId : regionsInLogic) {
                auto& randoRegion = Rando::Logic::Regions[regionId];
                Rando::Logic::SetCurrentRegionTime(regionTimeStates, regionId);
                for (auto& randoEvent : randoRegion.events) {
                    if (!eventsInLogic.contains(&randoEvent) && randoEvent.second()) {
                        RANDO_EVENTS[randoEvent.first]++;
                        eventsInLogic.insert(&randoEvent);
                        changed = true;
                    }
                }
                for (auto& [randoCheckId, checkLogic] : randoRegion.checks) {
                    if (!checksInLogic.contains(randoCheckId) && checkLogic.first()) {
                        checksInLogic.insert(randoCheckId);
                        changed = true;
                        auto pre = prePlaced.find(Rando::StaticData::Checks[randoCheckId].name);
                        if (pre != prePlaced.end() && !reachedHere.contains(pre->first)) {
                            reachedHere.insert(pre->first);
                            GiveOneItem(pre->second);
                        }
                    }
                }
            }
            if (!changed) {
                break;
            }
        }
        prePlacedReached.insert(reachedHere.begin(), reachedHere.end());
        return std::make_pair(checksInLogic, regionsInLogic.contains(RR_CLOCK_TOWN_SOUTH));
    };

    // ASSUMED FILL, ONE ITEM AT A TIME. The copy being placed is left OUT of the assumed inventory,
    // so its slot is guaranteed reachable WITHOUT it. Placing a whole batch against a single crawl
    // (which is what this did at first) lets items end up behind themselves and produces seeds that
    // cannot be completed — the exact bug that had to be fixed on the OoT side too. Skijer's NEI
    nlohmann::json placed = nlohmann::json::object();
    std::vector<std::string> leftover;
    std::set<std::string> takenThisTurn;
    bool portalReachable = false;

    for (size_t idx = 0; idx < toPlace.size(); idx++) {
        // Assumed = OoT's announcements + every copy MM still owes EXCEPT the one being placed now.
        std::vector<std::string> inventory;
        for (auto& [name, count] : assumed) {
            for (int i = 0; i < count; i++) {
                inventory.push_back(name);
            }
        }
        for (size_t j = idx + 1; j < toPlace.size(); j++) {
            inventory.push_back(toPlace[j]);
        }
        for (auto& name : leftover) {
            inventory.push_back(name);
        }

        auto [reachable, portal] = crawlReachable(inventory);
        portalReachable = portalReachable || portal;

        // 5.0.0 placement constraints (dungeon-item confinement: keys/boss keys/stray fairies to
        // "own dungeon"): the native fill refuses such an item outside its dungeon, so must we, or
        // the delegated fill hands MM a placement its own logic never accepts.
        RandoItemId placingRi = Rando::StaticData::GetItemIdFromName(toPlace[idx].c_str());

        std::vector<RandoCheckId> candidates;
        for (RandoCheckId checkId : reachable) {
            if (!shuffled.contains(checkId)) {
                continue;
            }
            const char* name = Rando::StaticData::Checks[checkId].name;
            if (used.contains(name) || takenThisTurn.contains(name)) {
                continue;
            }
            if (placingRi != RI_UNKNOWN && !Rando::Logic::IsItemAllowedAtCheck(placingRi, checkId)) {
                continue;
            }
            candidates.push_back(checkId);
        }
        if (candidates.empty()) {
            leftover.push_back(toPlace[idx]); // no reachable slot: the host retries it next turn
            continue;
        }
        RandoCheckId pick = candidates[std::uniform_int_distribution<size_t>(0, candidates.size() - 1)(rng)];
        const char* pickName = Rando::StaticData::Checks[pick].name;
        placed[pickName] = toPlace[idx];
        takenThisTurn.insert(pickName);
    }

    nlohmann::json remaining = nlohmann::json::array();
    for (auto& riName : leftover) {
        remaining.push_back(riName);
    }

    resp["placed"] = placed;
    resp["remaining"] = remaining;
    // MM's side of the cross-game portal is the fleet hole in South Clock Town. Reporting whether
    // MM's own logic can stand there is what lets the host open OoT when the seed STARTS in MM: the
    // portal is one bidirectional edge, and whichever world you begin in unlocks the other by
    // reaching its end of it. Skijer's NEI
    resp["portalReachable"] = portalReachable;
    // Pre-placed spots MM stood on during this turn: "I got to this one". The host announces each to
    // OoT, which may assume that item from then on and not before.
    nlohmann::json reachedPre = nlohmann::json::array();
    for (auto& name : prePlacedReached) {
        reachedPre.push_back(name);
    }
    resp["prePlacedReached"] = reachedPre;
    // Blocked = items remain and this turn could not place a SINGLE one. If it placed something, the
    // next turn starts from a larger inventory and may unblock itself.
    resp["blocked"] = !leftover.empty() && placed.empty();
    resp["reachable"] = (int)placed.size();
    SPDLOG_INFO("[FleetOracle] fillTurn: {} placed, {} pending, portal={}", placed.size(), leftover.size(),
                portalReachable);
    return resp;
}

// ---- bomba por frame ----

unsigned long long sLastProcessedSeq = 0;

void ProcessOracle() {
    if (FleetShipCombo_GetActiveGame() < 0) {
        return; // sin combo, sin oráculo
    }
    unsigned long long seq = FleetShipCombo_GetOracleRequestSeq();
    if (seq == 0 || seq == sLastProcessedSeq) {
        return;
    }

    nlohmann::json req;
    if (!ReadJson(ReqPath(), req) || !req.contains("seq")) {
        return; // el archivo todavía no está: reintenta el próximo frame
    }
    if (req["seq"].get<unsigned long long>() != seq) {
        return; // request vieja o a medio escribir: espera a que el archivo alcance al seq
    }

    std::string op = req.value("op", "reachable");
    SPDLOG_INFO("[FleetOracle] request #{} op={}", seq, op);

    // Backup completo del save del jugador; TODAS las ops mutan gSaveContext y se restaura siempre.
    auto backup = std::make_unique<SaveContext>();
    memcpy(backup.get(), &gSaveContext, sizeof(SaveContext));

    // Silencia el toast "You found X" (y el registro cross-sync) de Rando::GiveItem SOLO en las ops
    // de SIMULACIÓN (manifest/reachable), que dan miles de items y se restauran por completo: nadie ve
    // esos toasts (title/file select) y floodean la cola de notificaciones. createSave crea el save
    // real y sí debe registrar, así que NO se suprime ahí (el guard de gPlayState en CurrentJunkItem
    // evita el crash en cualquier caso). Balanceado en el restore.
    bool suppressToast = (op == "manifest" || op == "reachable" || op == "fillTurn");
    if (suppressToast) {
        gFcCombo_SuppressRecord++;
    }

    nlohmann::json resp;
    try {
        if (op == "manifest") {
            resp = HandleManifest();
        } else if (op == "reachable") {
            resp = HandleReachable(req);
        } else if (op == "setOptions") {
            resp = HandleSetOptions(req);
        } else if (op == "fillTurn") {
            resp = HandleFillTurn(req);
        } else if (op == "prepareSeed") {
            resp = HandlePrepareSeed(req);
        } else if (op == "createSave") {
            resp = HandleCreateSave(req);
        } else if (op == "deleteSave") {
            resp = HandleDeleteSave(req);
        } else if (op == "wipeSaves") {
            resp = HandleWipeSaves(req);
        } else if (op == "ensureSave") {
            resp = HandleEnsureSave(req);
        } else {
            resp["error"] = "unknown op: " + op;
        }
    } catch (const std::exception& e) {
        SPDLOG_ERROR("[FleetOracle] error procesando request #{}: {}", seq, e.what());
        resp = nlohmann::json{};
        resp["error"] = e.what();
    }
    if (suppressToast) {
        gFcCombo_SuppressRecord--;
    }
    memcpy(&gSaveContext, backup.get(), sizeof(SaveContext));

    resp["seq"] = seq;
    resp["op"] = op;
    WriteJsonAtomic(RespPath(), resp);
    FleetShipCombo_AckOracleResponse(seq);
    sLastProcessedSeq = seq;
}

void RegisterFleetOracle() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnGameStateUpdate>(ProcessOracle);
}

static RegisterShipInitFunc initFleetOracle(RegisterFleetOracle, {});

} // namespace

bool FleetOracle_RecreateSaveSlot(int slot, const char* name) {
    try {
        nlohmann::json req;
        req["slot"] = slot;
        req["name"] = (name != nullptr && name[0] != '\0') ? name : "Link";
        HandleCreateSave(req);
        return true;
    } catch (const std::exception& e) {
        SPDLOG_ERROR("[FleetOracle] could not rebuild MM slot {}: {}", slot, e.what());
        return false;
    } catch (...) {
        SPDLOG_ERROR("[FleetOracle] could not rebuild MM slot {}: unknown exception", slot);
        return false;
    }
}
