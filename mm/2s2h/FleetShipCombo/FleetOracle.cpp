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
#include <libultraship/bridge/consolevariablebridge.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <set>
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
    } catch (...) {
        return false;
    }
}

void WriteJsonAtomic(const std::filesystem::path& p, const nlohmann::json& j) {
    if (p.empty()) {
        return;
    }
    try {
        std::filesystem::path tmp = p;
        tmp += ".tmp2"; // sufijo propio de 2ship, como en FleetSync, para no chocar con el host
        {
            std::ofstream out(tmp);
            out << j << std::endl;
        }
        std::filesystem::rename(tmp, p);
    } catch (...) {
        SPDLOG_WARN("[FleetOracle] fallo escribiendo la respuesta");
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

    auto startingItems = Rando::GetStartingItemsFromConfig();
    Rando::SetStartingItemsInSave(gSaveContext.save.shipSaveInfo.rando, startingItems);
    Rando::GrantStartingItems();
}

// ---- ops ----

nlohmann::json HandleManifest() {
    nlohmann::json resp;
    BuildRandoBaseline();

    std::vector<RandoCheckId> checkPool;
    std::vector<RandoItemId> itemPool;
    Rando::Logic::GeneratePools(gSaveContext.save.shipSaveInfo.rando, checkPool, itemPool);

    nlohmann::json checks = nlohmann::json::array();
    for (RandoCheckId checkId : checkPool) {
        checks.push_back({ (int)checkId, Rando::StaticData::Checks[checkId].name });
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
    static const char* kAllowedPrefixes[] = { "gRando.",        "gMods.",     "gEnhancements.",
                                              "gCheats.",       "gSettings.", "gNotifications." };
    for (const char* prefix : kAllowedPrefixes) {
        if (cvar.rfind(prefix, 0) == 0) {
            return true;
        }
    }
    // LUS graphics/nav cvars que NO llevan prefijo gSettings (shared menu los vincula).
    static const char* kAllowedExact[] = { "gInternalResolution",  "gMSAAValue",       "gVsyncEnabled",
                                           "gSdlWindowedFullscreen", "gEnableMultiViewports", "gTextureFilter",
                                           "gControlNav",          "gInterpolationFPS", "gMatchRefreshRate" };
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
    SPDLOG_INFO("[FleetOracle] reachable: {} checks alcanzables", reachable.size());
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
    bool suppressToast = (op == "manifest" || op == "reachable");
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
        } else if (op == "prepareSeed") {
            resp = HandlePrepareSeed(req);
        } else if (op == "createSave") {
            resp = HandleCreateSave(req);
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
