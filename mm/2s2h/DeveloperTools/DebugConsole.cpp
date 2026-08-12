#include "DebugConsole.h"

#include <libultraship/bridge/consolevariablebridge.h>
#include <ship/window/Window.h>
#include <ship/window/gui/ConsoleWindow.h>
#include "2s2h/BenPort.h"
#include "2s2h/FleetShipCombo/FleetShipCombo.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/Rando/Rando.h"
#include "2s2h/CustomMessage/CustomMessage.h"
#include "2s2h/CustomItem/CustomItem.h"
#include "2s2h/BenGui/Notification.h"
#include <vector>
#include <string>
#include <utility>
#include <cctype>

extern "C" {
#include <z64.h>
#include "variables.h"
#include "functions.h"
#include "macros.h"

#include "overlays/gamestates/ovl_file_choose/z_file_select.h"
#include "overlays/gamestates/ovl_title/z_title.h"
}

#define CMD_REGISTER Ship::Context::GetRawInstance()->GetConsole()->AddCommand
// TODO: Commands should be using the output passed in.
#define ERROR_MESSAGE                                                                 \
    std::reinterpret_pointer_cast<Ship::ConsoleWindow>(                               \
        Ship::Context::GetRawInstance()->GetWindow()->GetGui()->GetGuiWindow("Console")) \
        ->SendErrorMessage
#define INFO_MESSAGE                                                                  \
    std::reinterpret_pointer_cast<Ship::ConsoleWindow>(                               \
        Ship::Context::GetRawInstance()->GetWindow()->GetGui()->GetGuiWindow("Console")) \
        ->SendInfoMessage

static bool ActorSpawnHandler(std::shared_ptr<Ship::Console> Console, const std::vector<std::string>& args,
                              std::string* output) {
    if ((args.size() != 9) && (args.size() != 3) && (args.size() != 6)) {
        ERROR_MESSAGE("Not enough arguments passed to actorspawn");
        return 1;
    }

    if (gPlayState == nullptr) {
        ERROR_MESSAGE("PlayState == nullptr");
        return 1;
    }

    Player* player = GET_PLAYER(gPlayState);
    PosRot spawnPoint;
    s16 actorId = 0;
    try {
        actorId = std::stoi(args[1]);
    } catch (std::invalid_argument const& ex) {
        ERROR_MESSAGE("Invalid actor ID");
        return 1;
    }

    const s16 params = std::stoi(args[2]);

    spawnPoint = player->actor.world;

    switch (args.size()) {
        case 9:
            if (args[6][0] != ',') {
                spawnPoint.rot.x = std::stoi(args[6]);
            }
            if (args[7][0] != ',') {
                spawnPoint.rot.y = std::stoi(args[7]);
            }
            if (args[8][0] != ',') {
                spawnPoint.rot.z = std::stoi(args[8]);
            }
        case 6:
            if (args[3][0] != ',') {
                spawnPoint.pos.x = std::stoi(args[3]);
            }
            if (args[4][0] != ',') {
                spawnPoint.pos.y = std::stoi(args[4]);
            }
            if (args[5][0] != ',') {
                spawnPoint.pos.z = std::stoi(args[5]);
            }
    }

    if (Actor_Spawn(&gPlayState->actorCtx, gPlayState, actorId, spawnPoint.pos.x, spawnPoint.pos.y, spawnPoint.pos.z,
                    spawnPoint.rot.x, spawnPoint.rot.y, spawnPoint.rot.z, params) == NULL) {
        ERROR_MESSAGE("Failed to spawn actor. Actor_Spawn returned NULL");
        return 1;
    }
    return 0;
}

static bool LoadSceneHandler(std::shared_ptr<Ship::Console> Console, const std::vector<std::string>&,
                             std::string* output) {
    gSaveContext.respawnFlag = 0;
    gSaveContext.seqId = NA_BGM_DISABLED;
    gSaveContext.gameMode = 0;

    return 0;
}

static bool SetPosHandler(std::shared_ptr<Ship::Console> Console, const std::vector<std::string> args,
                          std::string* output) {
    if (gPlayState == nullptr) {
        ERROR_MESSAGE("PlayState == nullptr");
        return 1;
    }

    Player* player = GET_PLAYER(gPlayState);

    if (args.size() == 1) {
        INFO_MESSAGE("Player position is [ %.2f, %.2f, %.2f ]", player->actor.world.pos.x, player->actor.world.pos.y,
                     player->actor.world.pos.z);
        return 0;
    }
    if (args.size() < 4)
        return 1;

    player->actor.world.pos.x = std::stof(args[1]);
    player->actor.world.pos.y = std::stof(args[2]);
    player->actor.world.pos.z = std::stof(args[3]);

    INFO_MESSAGE("Set player position to [ %.2f, %.2f, %.2f ]", player->actor.world.pos.x, player->actor.world.pos.y,
                 player->actor.world.pos.z);
    return 0;
}

// The raw reset, callable WITHOUT signaling the combo (used by the responder pump so a paired reset
// never ping-pongs). extern "C" so FleetSync's cross-game restart pump can call it.
extern "C" void FleetCombo_DoLocalReset(void) {
    if (gGameState == nullptr) {
        return;
    }
    STOP_GAMESTATE(gGameState);
    SET_NEXT_GAMESTATE(gGameState, ConsoleLogo_Init, sizeof(ConsoleLogoState));
}

static bool ResetHandler(std::shared_ptr<Ship::Console> Console, std::vector<std::string> args, std::string* output) {
    if (gGameState == nullptr) {
        ERROR_MESSAGE("gGameState == nullptr");
        return 1;
    }
    FleetCombo_DoLocalReset();
    FleetShipCombo_SignalRestart(); // combo: restart the paired game too (no-op outside the combo)
    // ...and hand the combo back to OoT, so restarting FROM Majora's Mask still leaves the player
    // looking at Ocarina of Time's title screen instead of MM's (which the combo never shows).
    FleetShipCombo_YieldToOoT();
    return 0;
}

static bool BHandler(std::shared_ptr<Ship::Console> Console, const std::vector<std::string>& args,
                     std::string* output) {
    if (args.size() < 2) {
        ERROR_MESSAGE("[2S2H] Unexpected arguments passed");
        return 1;
    }

    u8 itemId;
    try {
        itemId = std::stoi(args[1]);
    } catch (std::invalid_argument const& ex) {
        ERROR_MESSAGE("[2S2H] Item ID must be an integer.");
        return 1;
    }

    if (gPlayState == nullptr) {
        ERROR_MESSAGE("gPlayState == nullptr");
        return 1;
    }

    BUTTON_ITEM_EQUIP(CUR_FORM, EQUIP_SLOT_B) = itemId;
    Interface_LoadItemIconImpl(gPlayState, EQUIP_SLOT_B);
    return 0;
}

static bool GiveItemHandler(std::shared_ptr<Ship::Console> Console, const std::vector<std::string> args,
                            std::string* output) {
    if (args.size() < 2) {
        ERROR_MESSAGE("[2S2H] Unexpected arguments passed");
        return 1;
    }

    GetItemId getItemId;
    try {
        getItemId = (GetItemId)std::stoi(args[1]);
    } catch (std::invalid_argument const& ex) {
        ERROR_MESSAGE("[2S2H] Item ID must be an integer.");
        return 1;
    }

    if (gPlayState == nullptr) {
        ERROR_MESSAGE("gPlayState == nullptr");
        return 1;
    }

    Player* player = GET_PLAYER(gPlayState);

    // Cheat by using Tatl to give the item
    if (player == nullptr || player->tatlActor == nullptr) {
        ERROR_MESSAGE("Unable to receive item");
        return 1;
    }

    if (!Actor_OfferGetItemFar(player->tatlActor, gPlayState, getItemId)) {
        ERROR_MESSAGE("Unable to receive item");
        return 1;
    }
    return 0;
}

// ── give <item name> ─────────────────────────────────────────────────────────────────────────────
// `give_item` above takes a raw GetItemId, so it can only hand over items that exist in the vanilla
// GI table — every custom item (page-2 gear, progressives, songs) is unreachable from it. The Save
// Editor's "Queue Randomizer Item Gives" list can give those, with the real presentation: get-item
// cutscene, textbox and the item's own 3D model. This is that exact code path, driven by name from
// the console, so testing an item is `give rocs feather` instead of hunting a button in a list of
// several hundred. Skijer's NEI
static std::string NormalizeItemName(const std::string& str) {
    std::string out;
    for (char c : str) {
        if (std::isalnum((unsigned char)c)) {
            out += (char)std::tolower((unsigned char)c);
        }
    }
    return out;
}

static void QueueRandoItemGive(RandoItemId randoItemId) {
    GameInteractor::Instance->events.emplace_back(GIEventGiveItem{
        .showGetItemCutscene = Rando::StaticData::ShouldShowGetItemCutscene(Rando::ConvertItem(randoItemId)),
        .param = (int16_t)randoItemId,
        .giveItem =
            [](Actor* actor, PlayState* play) {
                RandoItemId randoItemId = Rando::ConvertItem((RandoItemId)CUSTOM_ITEM_PARAM);
                std::string prefix = "You found";
                std::string message = Rando::StaticData::GetItemName(randoItemId);

                CustomMessage::Entry entry = {
                    .textboxType = 2,
                    .icon = Rando::StaticData::GetIconForZMessage(randoItemId),
                    .msg = prefix + " " + message + "!",
                };

                if (CUSTOM_ITEM_FLAGS & CustomItem::GIVE_ITEM_CUTSCENE) {
                    CustomMessage::SetActiveCustomMessage(entry.msg, entry);
                } else if (Rando::StaticData::ShouldShowGetItemCutscene(
                               Rando::ConvertItem((RandoItemId)CUSTOM_ITEM_PARAM))) {
                    CustomMessage::StartTextbox(entry.msg + "\x1C\x02\x10", entry);
                } else {
                    Notification::Emit({
                        .itemIcon = Rando::StaticData::GetIconTexturePath(randoItemId),
                        .message = prefix,
                        .suffix = message,
                    });
                }
                Rando::GiveItem(randoItemId);
                CUSTOM_ITEM_PARAM = randoItemId;
            },
        .drawItem =
            [](Actor* actor, PlayState* play) {
                RandoItemId randoItemId;

                // If the item has been given, the CUSTOM_ITEM_PARAM is set to the RI, prior to that it's the RC
                if (CUSTOM_ITEM_FLAGS & CustomItem::CALLED_ACTION) {
                    randoItemId = (RandoItemId)CUSTOM_ITEM_PARAM;
                } else {
                    randoItemId = Rando::ConvertItem((RandoItemId)CUSTOM_ITEM_PARAM);
                }

                Matrix_Scale(30.0f, 30.0f, 30.0f, MTXMODE_APPLY);
                Rando::DrawItem(randoItemId, RC_UNKNOWN, actor);
            } });
}

static bool GiveHandler(std::shared_ptr<Ship::Console> Console, const std::vector<std::string>& args,
                        std::string* output) {
    if (args.size() < 2) {
        ERROR_MESSAGE("[2S2H] Usage: give <item name>  (e.g. \"give rocs feather\", \"give list roc\")");
        return 1;
    }

    // `give list [filter]` prints what a name would match instead of giving anything.
    bool listOnly = args[1] == "list";
    std::string query;
    for (size_t i = listOnly ? 2 : 1; i < args.size(); i++) {
        query += args[i];
    }
    std::string needle = NormalizeItemName(query);

    if (!listOnly && needle.empty()) {
        ERROR_MESSAGE("[2S2H] No item name passed");
        return 1;
    }

    RandoItemId exact = RI_UNKNOWN;
    std::vector<std::pair<RandoItemId, std::string>> matches;

    for (auto& [randoItemId, randoStaticItem] : Rando::StaticData::Items) {
        if (randoItemId == RI_UNKNOWN || randoItemId == RI_NONE || randoItemId == RI_TRIFORCE_PIECE_PREVIOUS) {
            continue;
        }

        std::string spoilerName = NormalizeItemName(randoStaticItem.spoilerName); // "RI_OOT_NEI_WHIP"
        std::string displayName = NormalizeItemName(randoStaticItem.name);        // "Whip"

        if (!needle.empty() && (needle == spoilerName || needle == displayName)) {
            exact = randoItemId;
            break;
        }
        if (needle.empty() || spoilerName.find(needle) != std::string::npos ||
            displayName.find(needle) != std::string::npos) {
            matches.emplace_back(randoItemId,
                                 std::string(randoStaticItem.name) + "  [" + randoStaticItem.spoilerName + "]");
        }
    }

    if (listOnly) {
        if (matches.empty()) {
            ERROR_MESSAGE("[2S2H] No item matches \"%s\"", query.c_str());
            return 1;
        }
        INFO_MESSAGE("[2S2H] %d item(s) match:", (int)matches.size());
        for (auto& [randoItemId, label] : matches) {
            INFO_MESSAGE("  %s", label.c_str());
        }
        return 0;
    }

    if (exact == RI_UNKNOWN) {
        if (matches.empty()) {
            ERROR_MESSAGE("[2S2H] No item matches \"%s\"", query.c_str());
            return 1;
        }
        if (matches.size() > 1) {
            ERROR_MESSAGE("[2S2H] \"%s\" is ambiguous, %d matches:", query.c_str(), (int)matches.size());
            for (size_t i = 0; i < matches.size() && i < 20; i++) {
                ERROR_MESSAGE("  %s", matches[i].second.c_str());
            }
            if (matches.size() > 20) {
                ERROR_MESSAGE("  ...and %d more (use `give list %s`)", (int)matches.size() - 20, query.c_str());
            }
            return 1;
        }
        exact = matches[0].first;
    }

    // Only the give itself needs gameplay; the check is here so a typo still gets a name error first.
    if (gPlayState == nullptr) {
        ERROR_MESSAGE("gPlayState == nullptr");
        return 1;
    }

    QueueRandoItemGive(exact);
    INFO_MESSAGE("[2S2H] Giving %s", Rando::StaticData::Items[exact].name);
    return 0;
}

// ── Recorrido de validación: give_next / give_all ────────────────────────────────────────────────
// Para comprobar que CADA item se da, se dibuja, y tiene icono y descripción, hace falta pasar por
// todos en orden y mirarlos. `give_next` avanza uno por invocación con la presentación real;
// `give_again` repite el actual (así se ven los niveles de una cadena progresiva, que es el mismo
// item dado N veces); `give_all` llena de golpe una categoría sin cutscene. Skijer's NEI
// ─── Walk classification (Skijer's 13 categories, 2026-08-07 — MISMOS nombres que en soh) ────────
// Una WalkSpec por RI: categoría del walk + cuántas copias encola un give_all (una copia POR NIVEL
// de cadena progresiva). category == NULL excluye del walk: win conditions, la Hylia retirada, y
// las filas que solo existen como target discreto de una cadena progresiva (la cadena ya reparte
// esos niveles). La lista completa con razones vive en GIVE_CATEGORIES.md (raíz de Shipwright).
struct WalkSpec {
    const char* category;
    int copies;
};

static WalkSpec WalkSpecOf(RandoItemId randoItemId, const Rando::StaticData::RandoStaticItem& item) {
    std::string id = item.spoilerName;
    auto pre = [&](const char* p) { return id.rfind(p, 0) == 0; };
    auto in = [&](std::initializer_list<const char*> l) {
        for (const char* s : l) {
            if (id == s) {
                return true;
            }
        }
        return false;
    };
    static const WalkSpec kExcluded = { nullptr, 0 };

    // Exclusiones: win conditions, retirada, targets discretos de progresivos, alias progresivos.
    if (in({ "RI_TRIFORCE_PIECE", "RI_TRIFORCE_PIECE_PREVIOUS", "RI_OOT_NEI_HYLIAS_GRACE",
             "RI_SWORD_KOKIRI", "RI_SWORD_RAZOR", "RI_SWORD_GILDED", "RI_BOW", "RI_QUIVER_40",
             "RI_QUIVER_50", "RI_BOMB_BAG_20", "RI_BOMB_BAG_30", "RI_BOMB_BAG_40", "RI_SINGLE_MAGIC",
             "RI_DOUBLE_MAGIC", "RI_WALLET_ADULT", "RI_WALLET_GIANT", "RI_WALLET_TYCOON",
             "RI_PROGRESSIVE_LULLABY", "RI_TIME_PROGRESSIVE",
             // skills sueltos del Dual Cane: los reparte RI_OOT_NEI_CANE_OF_SOMARIA x6
             "RI_OOT_NEI_CANE_SOMARIA_BLOCK", "RI_OOT_NEI_CANE_SOMARIA_PLATFORM",
             "RI_OOT_NEI_CANE_PACCI_FLIP", "RI_OOT_NEI_CANE_PACCI_STONE",
             "RI_OOT_NEI_CANE_PACCI_ULTRAHAND" })) {
        return kExcluded;
    }

    // ── lado OoT (ports) ──
    if (pre("RI_OOT_NEI_")) {
        if (id == "RI_OOT_NEI_CANE_OF_SOMARIA") {
            return { "nei_items", 6 }; // 6 skills en 1 slot (el record alias cruza por la genérica)
        }
        if (id == "RI_OOT_NEI_SHEIKAH_SLATE") {
            return kExcluded; // lo representan sus 4 RI_OOT_NEI_SLATE_RUNE_* (como la wand y sus rods)
        }
        return { "nei_items", 1 }; // incluye las 4 runas del slate (items hermanos, como los rods)
    }
    if (pre("RI_OOT_EXT_")) {
        return { "nei_items", 1 };
    }
    if (in({ "RI_OOT_SONG_FUGUE_OF_HOME", "RI_OOT_SONG_COMMAND_MELODY", "RI_OOT_SONG_BALLAD_OF_THE_HERO",
             "RI_OOT_BOTTLE_MAGIC_MUSHROOM" })) {
        return { "nei_items", 1 }; // las 3 canciones custom de Skijer + la botella custom
    }
    if (id == "RI_OOT_STONE_OF_AGONY") {
        return { "oot_collectables", 2 }; // L1 piedra, L2 Quartz of Motion (mismo RI)
    }
    if (pre("RI_OOT_SONG_") || pre("RI_OOT_MEDALLION_") || pre("RI_OOT_STONE_") ||
        in({ "RI_OOT_GS_TOKEN", "RI_OOT_GREG", "RI_OOT_GERUDO_MEMBERSHIP_CARD" })) {
        return { "oot_collectables", 1 };
    }
    if (pre("RI_OOT_ABILITY_") || pre("RI_OOT_SPEAK_") || pre("RI_SOUL_OOT_BEAN_")) {
        return { "oot_skills", 1 }; // climb/crawl/chests, jabber nuts, bean souls (habilitan algo)
    }
    if (pre("RI_SOUL_OOT_BOSS_")) {
        return { "oot_dungeons", 1 };
    }
    if (pre("RI_OOT_MAP_") || pre("RI_OOT_COMPASS_") || pre("RI_OOT_SMALL_KEY_") ||
        pre("RI_OOT_BOSS_KEY_") || pre("RI_OOT_KEY_RING_") || id == "RI_OOT_SKELETON_KEY") {
        return { "oot_dungeons", 1 };
    }
    if (pre("RI_OOT_PROGRESSIVE_")) {
        // Cadenas: una copia por nivel. Hammer/MS/BGS son upgrades NEI (su L1 = arma vanilla va
        // incluida en la primera copia); Roc es el item de Skijer (pluma→capa).
        if (in({ "RI_OOT_PROGRESSIVE_HAMMER", "RI_OOT_PROGRESSIVE_MASTER_SWORD",
                 "RI_OOT_PROGRESSIVE_BGS" })) {
            return { "oot_nei_upgrades", 2 };
        }
        if (id == "RI_OOT_PROGRESSIVE_ROC") {
            return { "nei_items", 2 };
        }
        if (id == "RI_OOT_PROGRESSIVE_STRENGTH") {
            return { "oot_items", 3 }; // Goron -> Silver -> Golden
        }
        return { "oot_items", 2 }; // stick/nut capacity: 20->30 / 30->40
    }
    if (pre("RI_OOT_")) {
        return { "oot_items", 1 }; // spells, boomerang, tunics/boots/shields, bottles, trade, masks...
    }

    // ── lado MM (nativo) ──
    if (item.randoItemType == RITYPE_MASK) {
        return { "mm_masks", 1 };
    }
    if (item.randoItemType == RITYPE_JUNK || in({ "RI_JUNK", "RI_TRAP", "RI_MAX_TRAP" })) {
        return { "mm_junk", 1 };
    }
    if (pre("RI_SOUL_BOSS_")) {
        return { "mm_dungeons", 1 }; // boss souls van con el relleno de mazmorra
    }
    if (pre("RI_SOUL_ENEMY_") || in({ "RI_ABILITY_SWIM", "RI_GREAT_SPIN_ATTACK" })) {
        return { "mm_skills", 1 }; // enemy souls habilitan enemigos, como las skills
    }
    if (pre("RI_SONG_") || pre("RI_REMAINS_") || pre("RI_OWL_") || pre("RI_TINGLE_MAP_") ||
        pre("RI_GS_TOKEN_") || pre("RI_FROG_") || pre("RI_TIME_") ||
        in({ "RI_HEART_PIECE", "RI_HEART_CONTAINER", "RI_DOUBLE_DEFENSE" })) {
        return { "mm_collectables", 1 };
    }
    if (id.find("_STRAY_FAIRY") != std::string::npos || id.find("_SMALL_KEY") != std::string::npos ||
        id.find("_BOSS_KEY") != std::string::npos || id.find("_MAP") != std::string::npos ||
        id.find("_COMPASS") != std::string::npos) {
        return { "mm_dungeons", 1 };
    }
    // Custom NEI que vive en filas "MM" (sin prefijo OOT): botellas custom y el clawshot.
    if (in({ "RI_NET", "RI_BOTTOMLESS_BOTTLE", "RI_CLAWSHOT" })) {
        return { "nei_items", 1 };
    }
    // Progresivos nativos de MM: una copia por nivel (ConvertItem escala en cada give).
    if (id == "RI_PROGRESSIVE_MAGIC") {
        return { "mm_items", 2 };
    }
    if (in({ "RI_PROGRESSIVE_BOW", "RI_PROGRESSIVE_BOMB_BAG", "RI_PROGRESSIVE_SWORD",
             "RI_PROGRESSIVE_WALLET" })) {
        return { "mm_items", 3 };
    }
    return { "mm_items", 1 }; // pictobox, keg, bottles, ocarina, trade quest, notebook...
}

static const char* kWalkUsage = "all|progressive|oot_items|oot_nei_upgrades|oot_collectables|oot_skills|"
                                "oot_dungeons|oot_junk|nei_items|mm_masks|mm_items|mm_collectables|"
                                "mm_dungeons|mm_skills|mm_junk";

// Categoría VIRTUAL "progressive": SOLO las cadenas, agrupadas por cadena y de nivel más bajo a más
// alto — para verlas subir una tras otra in-game (give_next progressive + give_again por nivel, o
// give_all progressive en masivo). Los items también viven en su categoría normal.
static const RandoItemId kProgressiveWalk[] = {
    RI_PROGRESSIVE_SWORD,                                          // Kokiri -> Razor -> Gilded (x3)
    RI_OOT_PROGRESSIVE_MASTER_SWORD,                               // Master -> True Master (x2)
    RI_OOT_PROGRESSIVE_BGS,                                        // Biggoron -> GFS (x2)
    RI_OOT_PROGRESSIVE_HAMMER,                                     // Hammer -> IK Axe (x2)
    RI_PROGRESSIVE_BOW, RI_PROGRESSIVE_BOMB_BAG,                   // x3 c/u
    RI_PROGRESSIVE_MAGIC,                                          // x2
    RI_PROGRESSIVE_WALLET,                                         // x3
    RI_OOT_PROGRESSIVE_STRENGTH,                                   // x3
    RI_OOT_PROGRESSIVE_STICK_CAPACITY, RI_OOT_PROGRESSIVE_NUT_CAPACITY, // x2 c/u
    RI_OOT_PROGRESSIVE_ROC,                                        // pluma -> capa (x2)
    RI_OOT_STONE_OF_AGONY,                                         // piedra -> Quartz (x2)
    RI_OOT_NEI_CANE_OF_SOMARIA,                                    // las 6 skills (x6)
    // Los 6 rods de la Elemental Wand: items hermanos sobre un slot (cada uno con su textbox).
    RI_OOT_NEI_WAND_SAND_ROD, RI_OOT_NEI_WAND_TORNADO_ROD, RI_OOT_NEI_WAND_WATER_ROD,
    RI_OOT_NEI_WAND_METEOR_ROD, RI_OOT_NEI_WAND_STORM_ROD, RI_OOT_NEI_WAND_SHADOW_SCEPTER,
    // Las 4 runas del Sheikah Slate: items hermanos sobre un slot (cada una con su textbox).
    RI_OOT_NEI_SLATE_RUNE_BOMB, RI_OOT_NEI_SLATE_RUNE_MASTER_CYCLE,
    RI_OOT_NEI_SLATE_RUNE_STASIS, RI_OOT_NEI_SLATE_RUNE_CRYONIS,
    RI_SONG_LULLABY_INTRO, RI_SONG_LULLABY,                        // la "cadena" de la Lullaby
};

// The walk order is the table order, which is also the order the audit script reports in.
// (RI_TRIFORCE_PIECE(_PREVIOUS) quedan fuera vía WalkSpecOf: al llegar a REQUIRED disparan la
// transición de game-completion — créditos. Siguen accesibles con `give triforce piece`.)
static std::vector<RandoItemId> BuildWalkList(const std::string& category) {
    std::vector<RandoItemId> list;
    if (category == "progressive") {
        // Orden explícito por cadena (L1 primero) en vez del orden de la tabla.
        for (RandoItemId randoItemId : kProgressiveWalk) {
            if (Rando::StaticData::Items.count(randoItemId) != 0) {
                list.push_back(randoItemId);
            }
        }
        return list;
    }
    for (auto& [randoItemId, randoStaticItem] : Rando::StaticData::Items) {
        if (randoItemId == RI_UNKNOWN || randoItemId == RI_NONE) {
            continue;
        }
        WalkSpec spec = WalkSpecOf(randoItemId, randoStaticItem);
        if (spec.category == nullptr) {
            continue; // excluido del walk (triforce, targets de resolución, Hylia retirada...)
        }
        if (!category.empty() && category != "all" && category != spec.category) {
            continue;
        }
        list.push_back(randoItemId);
    }
    return list;
}

static std::string sWalkCategory = "all";
static std::vector<RandoItemId> sWalkList;
static int sWalkIndex = -1; // -1 = nothing given yet; give_next moves to 0 first

static void AnnounceWalkPosition(RandoItemId randoItemId) {
    auto& item = Rando::StaticData::Items[randoItemId];
    WalkSpec spec = WalkSpecOf(randoItemId, item);
    INFO_MESSAGE("[2S2H] %d/%d  %s  (%s, %s)", sWalkIndex + 1, (int)sWalkList.size(), item.name,
                 spec.category != nullptr ? spec.category : "?", item.spoilerName);
}

// step: +1 next, -1 previous, 0 repeat the current one (next level of a progressive chain).
static bool WalkStep(int step, const std::vector<std::string>& args, size_t categoryArg) {
    std::string requested = (args.size() > categoryArg) ? args[categoryArg] : "";
    if (!requested.empty() && requested != sWalkCategory) {
        sWalkCategory = requested;
        sWalkList.clear();
        sWalkIndex = -1;
    }
    if (sWalkList.empty()) {
        sWalkList = BuildWalkList(sWalkCategory);
        if (sWalkList.empty()) {
            ERROR_MESSAGE("[2S2H] No items in category \"%s\"", sWalkCategory.c_str());
            return false;
        }
    }
    if (gPlayState == nullptr) {
        ERROR_MESSAGE("gPlayState == nullptr");
        return false;
    }

    int next = sWalkIndex + step;
    if (step == 0 && sWalkIndex < 0) {
        next = 0; // nothing given yet: "again" behaves like "next"
    }
    if (next < 0) {
        ERROR_MESSAGE("[2S2H] Already at the start of \"%s\"", sWalkCategory.c_str());
        return false;
    }
    if (next >= (int)sWalkList.size()) {
        INFO_MESSAGE("[2S2H] End of \"%s\" (%d items). give_reset to start over.", sWalkCategory.c_str(),
                     (int)sWalkList.size());
        return false;
    }

    sWalkIndex = next;
    QueueRandoItemGive(sWalkList[sWalkIndex]);
    AnnounceWalkPosition(sWalkList[sWalkIndex]);
    return true;
}

static bool GiveNextHandler(std::shared_ptr<Ship::Console> Console, const std::vector<std::string>& args,
                            std::string* output) {
    return WalkStep(1, args, 1) ? 0 : 1;
}

static bool GivePrevHandler(std::shared_ptr<Ship::Console> Console, const std::vector<std::string>& args,
                            std::string* output) {
    return WalkStep(-1, args, 1) ? 0 : 1;
}

static bool GiveAgainHandler(std::shared_ptr<Ship::Console> Console, const std::vector<std::string>& args,
                             std::string* output) {
    return WalkStep(0, args, 1) ? 0 : 1;
}

static bool GiveResetHandler(std::shared_ptr<Ship::Console> Console, const std::vector<std::string>& args,
                             std::string* output) {
    sWalkCategory = (args.size() > 1) ? args[1] : "all";
    sWalkList = BuildWalkList(sWalkCategory);
    sWalkIndex = -1;
    if (sWalkList.empty()) {
        ERROR_MESSAGE("[2S2H] No items in category \"%s\". Categories: %s", sWalkCategory.c_str(), kWalkUsage);
        return 1;
    }
    INFO_MESSAGE("[2S2H] Walk reset: \"%s\", %d items. give_next to start.", sWalkCategory.c_str(),
                 (int)sWalkList.size());
    return 0;
}

// Masivo: sin cutscene ni textbox — encolar cientos de cutscenes sería inusable. Va directo a
// Rando::GiveItem, que es el mismo camino que usa el arranque de una seed.
static bool GiveAllHandler(std::shared_ptr<Ship::Console> Console, const std::vector<std::string>& args,
                           std::string* output) {
    if (args.size() < 2) {
        ERROR_MESSAGE("[2S2H] Usage: give_all <%s>", kWalkUsage);
        return 1;
    }
    if (gPlayState == nullptr) {
        ERROR_MESSAGE("gPlayState == nullptr");
        return 1;
    }

    std::vector<RandoItemId> list = BuildWalkList(args[1]);
    if (list.empty()) {
        ERROR_MESSAGE("[2S2H] No items in category \"%s\"", args[1].c_str());
        return 1;
    }
    int given = 0;
    for (RandoItemId randoItemId : list) {
        // Una copia por NIVEL de cadena (WalkSpecOf): las cadenas progresivas convierten al
        // siguiente nivel en cada give (ConvertItem / los arms RI_OOT_PROGRESSIVE leen estado), y
        // una copia sobre el tope re-otorga el máximo sin efecto visible (give_all es silencioso).
        for (int i = 0; i < WalkSpecOf(randoItemId, Rando::StaticData::Items[randoItemId]).copies; i++) {
            Rando::GiveItem(randoItemId);
            given++;
        }
    }
    INFO_MESSAGE("[2S2H] Gave %d items from \"%s\" (cadenas progresivas repetidas hasta su tope)", given,
                 args[1].c_str());
    return 0;
}

static bool EntranceHandler(std::shared_ptr<Ship::Console> Console, const std::vector<std::string>& args,
                            std::string* output) {
    if (args.size() < 2) {
        ERROR_MESSAGE("[2S2H] Unexpected arguments passed");
        return 1;
    }

    unsigned int entrance;

    try {
        entrance = std::stoi(args[1], nullptr, 16);
    } catch (std::invalid_argument const& ex) {
        ERROR_MESSAGE("[2S2H] Entrance value must be a Hex number.");
        return 1;
    }

    gPlayState->nextEntrance = entrance;
    gPlayState->transitionTrigger = TRANS_TRIGGER_START;
    gPlayState->transitionType = TRANS_TYPE_INSTANT;
    gSaveContext.nextTransitionType = TRANS_TYPE_INSTANT;
    return 0;
}

static bool VoidHandler(std::shared_ptr<Ship::Console> Console, const std::vector<std::string>& args,
                        std::string* output) {
    if (gPlayState != nullptr) {
        func_80169EFC(gPlayState);
    } else {
        ERROR_MESSAGE("gPlayState == nullptr");
        return 1;
    }
    return 0;
}

static bool ReloadHandler(std::shared_ptr<Ship::Console> Console, const std::vector<std::string>& args,
                          std::string* output) {
    if (gPlayState != nullptr) {
        gPlayState->nextEntrance = gSaveContext.save.entrance;
        gPlayState->transitionTrigger = TRANS_TRIGGER_START;
        gPlayState->transitionType = TRANS_TYPE_INSTANT;
        gSaveContext.nextTransitionType = TRANS_TYPE_INSTANT;
    } else {
        ERROR_MESSAGE("gPlayState == nullptr");
        return 1;
    }
    return 0;
}

static bool FileSelectHandler(std::shared_ptr<Ship::Console> Console, const std::vector<std::string>& args,
                              std::string* output) {
    if (gPlayState != nullptr) {
        STOP_GAMESTATE(&gPlayState->state);
        SET_NEXT_GAMESTATE(&gPlayState->state, FileSelect_Init, sizeof(FileSelectState));
    } else {
        ERROR_MESSAGE("gPlayState == nullptr");
        return 1;
    }
    return 0;
}

static bool QuitHandler(std::shared_ptr<Ship::Console> Console, const std::vector<std::string>& args,
                        std::string* output) {
    Ship::Context::GetRawInstance()->GetWindow()->Close();
    return 0;
}

void DebugConsole_Init(void) {
    // Console
    CMD_REGISTER("file_select", { FileSelectHandler, "Returns to the file select." });
    CMD_REGISTER("reset", { ResetHandler, "Resets the game." });
    CMD_REGISTER("quit", { QuitHandler, "Quits the game." });

    // Map & Location
    CMD_REGISTER("void", { VoidHandler, "Voids out of the current map." });
    CMD_REGISTER("reload", { ReloadHandler, "Reloads the current map." });
    CMD_REGISTER("entrance", { EntranceHandler,
                               "Sends player to the entered entrance (hex)",
                               { { "entrance", Ship::ArgumentType::NUMBER } } });

    // Gameplay
    CMD_REGISTER("give_item", { GiveItemHandler,
                                "Gives an item to the player as if it was given from an actor",
                                { { "giveItemID", Ship::ArgumentType::NUMBER } } });

    CMD_REGISTER("give", { GiveHandler,
                           "Gives an item by name, with its get-item cutscene and model. `give list <filter>` "
                           "lists matches. Skijer's NEI",
                           { { "item name", Ship::ArgumentType::TEXT } } });

    // Recorrido de validación (Skijer's NEI). Las 13 categorías (idénticas en soh) y la lista
    // completa de items por categoría viven en GIVE_CATEGORIES.md (raíz de Shipwright).
    CMD_REGISTER("give_next", { GiveNextHandler,
                                "Gives the next item of the walk, with cutscene and model.",
                                { { "category", Ship::ArgumentType::TEXT, true } } });
    CMD_REGISTER("give_prev", { GivePrevHandler,
                                "Goes back one item in the walk.",
                                { { "category", Ship::ArgumentType::TEXT, true } } });
    CMD_REGISTER("give_again", { GiveAgainHandler,
                                 "Gives the CURRENT item again — next level of a progressive chain.",
                                 { { "category", Ship::ArgumentType::TEXT, true } } });
    CMD_REGISTER("give_reset", { GiveResetHandler,
                                 "Restarts the walk, optionally on another category.",
                                 { { "category", Ship::ArgumentType::TEXT, true } } });
    CMD_REGISTER("give_all", { GiveAllHandler,
                               "Gives every item of a category at once, no cutscene.",
                               { { "category", Ship::ArgumentType::TEXT } } });

    CMD_REGISTER("bItem", { BHandler, "Set an item to the B button.", { { "Item ID", Ship::ArgumentType::NUMBER } } });

    CMD_REGISTER("spawn", { ActorSpawnHandler,
                            "Spawn an actor.",
                            { { "actor id", Ship::ArgumentType::NUMBER },
                              { "data", Ship::ArgumentType::NUMBER },
                              { "x", Ship::ArgumentType::NUMBER, true },
                              { "y", Ship::ArgumentType::NUMBER, true },
                              { "z", Ship::ArgumentType::NUMBER, true },
                              { "rx", Ship::ArgumentType::NUMBER, true },
                              { "ry", Ship::ArgumentType::NUMBER, true },
                              { "rz", Ship::ArgumentType::NUMBER, true } } });

    CMD_REGISTER("pos", { SetPosHandler,
                          "Sets the position of the player.",
                          { { "x", Ship::ArgumentType::NUMBER, true },
                            { "y", Ship::ArgumentType::NUMBER, true },
                            { "z", Ship::ArgumentType::NUMBER, true } } });
}
