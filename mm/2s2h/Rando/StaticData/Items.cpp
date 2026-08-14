#include "StaticData.h"
#include <cstring>
#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/ShipUtils.h"
#include "2s2h/Rando/Rando.h"
#include "2s2h_assets.h"

extern "C" {
extern s16 D_801CFF94[250];
// 2S2H [Rando] z_message.c — stages a custom rgba32 textbox icon consumed by sentinel icon byte 0xF5
void Message_StageCustomItemIcon(void* tex, s16 size);
#include "assets/interface/parameter_static/parameter_static.h"
#include "assets/interface/icon_item_dungeon_static/icon_item_dungeon_static.h"
#include "interface/icon_item_field_static/icon_item_field_static.h"
#include "assets/archives/icon_item_static/icon_item_static_yar.h"
}

namespace Rando {

namespace StaticData {

#define RI(id, article, name, type, itemId, getItemId, drawId)      \
    {                                                               \
        id, {                                                       \
            id, #id, article, name, type, itemId, getItemId, drawId \
        }                                                           \
    }

// clang-format off
std::map<RandoItemId, RandoStaticItem> Items = {
    RI(RI_UNKNOWN,                    "",     "Unknown",                    RITYPE_JUNK,            ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_ABILITY_SWIM,               "the",  "Ability to Swim",            RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_ARROW_FIRE,                 "",     "Fire Arrows",                RITYPE_MAJOR,           ITEM_ARROW_FIRE,                 GI_ARROW_FIRE,               GID_ARROW_FIRE),
    RI(RI_ARROW_ICE,                  "",     "Ice Arrows",                 RITYPE_MAJOR,           ITEM_ARROW_ICE,                  GI_ARROW_ICE,                GID_ARROW_ICE),
    RI(RI_ARROW_LIGHT,                "",     "Light Arrows",               RITYPE_MAJOR,           ITEM_ARROW_LIGHT,                GI_ARROW_LIGHT,              GID_ARROW_LIGHT),
    RI(RI_ARROWS_10,                  "",     "10 Arrows",                  RITYPE_JUNK,            ITEM_ARROWS_10,                  GI_ARROWS_10,                GID_ARROWS_SMALL),
    RI(RI_ARROWS_30,                  "",     "30 Arrows",                  RITYPE_JUNK,            ITEM_ARROWS_30,                  GI_ARROWS_30,                GID_ARROWS_MEDIUM),
    RI(RI_ARROWS_50,                  "",     "50 Arrows",                  RITYPE_JUNK,            ITEM_ARROWS_50,                  GI_ARROWS_50,                GID_ARROWS_LARGE),
    RI(RI_BLUE_POTION_REFILL,         "a",    "Blue Potion Refill",         RITYPE_JUNK,            ITEM_POTION_BLUE,                GI_POTION_BLUE,              GID_POTION_BLUE),
    RI(RI_BOMB_BAG_20,                "a",    "Bomb Bag",                   RITYPE_MAJOR,           ITEM_BOMB_BAG_20,                GI_BOMB_BAG_20,              GID_BOMB_BAG_20),
    RI(RI_BOMB_BAG_30,                "a",    "Big Bomb Bag",               RITYPE_LESSER,          ITEM_BOMB_BAG_30,                GI_BOMB_BAG_30,              GID_BOMB_BAG_30),
    RI(RI_BOMB_BAG_40,                "the",  "Biggest Bomb Bag",           RITYPE_LESSER,          ITEM_BOMB_BAG_40,                GI_BOMB_BAG_40,              GID_BOMB_BAG_40),
    RI(RI_BOMBCHU_10,                 "",     "10 Bombchus",                RITYPE_JUNK,            ITEM_BOMBCHUS_10,                GI_BOMBCHUS_10,              GID_BOMBCHU),
    RI(RI_BOMBCHU_5,                  "",     "5 Bombchus",                 RITYPE_JUNK,            ITEM_BOMBCHUS_5,                 GI_BOMBCHUS_5,               GID_BOMBCHU),
    RI(RI_BOMBCHU,                    "a",    "Bombchu",                    RITYPE_JUNK,            ITEM_BOMBCHUS_1,                 GI_BOMBCHUS_1,               GID_BOMBCHU), // not sure about this
    RI(RI_BOMBERS_NOTEBOOK,           "the",  "Bomber's Notebook",          RITYPE_LESSER,          ITEM_BOMBERS_NOTEBOOK,           GI_BOMBERS_NOTEBOOK,         GID_BOMBERS_NOTEBOOK),
    RI(RI_BOMBS_10,                   "",     "10 Bombs",                   RITYPE_JUNK,            ITEM_BOMBS_10,                   GI_BOMBS_10,                 GID_BOMB),
    RI(RI_BOMBS_5,                    "",     "5 Bombs",                    RITYPE_JUNK,            ITEM_BOMBS_5,                    GI_BOMBS_5,                  GID_BOMB),
    RI(RI_BOTTLE_CHATEAU_ROMANI,      "a",    "Bottle of Chateau Romani",   RITYPE_MAJOR,           ITEM_CHATEAU,                    GI_CHATEAU,                  GID_CHATEAU),
    RI(RI_BOTTLE_EMPTY,               "an",   "Empty Bottle",               RITYPE_MAJOR,           ITEM_BOTTLE,                     GI_BOTTLE,                   GID_BOTTLE),
    RI(RI_BOTTLE_GOLD_DUST,           "a",    "Bottle With Gold Dust",      RITYPE_MAJOR,           ITEM_GOLD_DUST,                  GI_GOLD_DUST,                GID_SEAHORSE), // bottle of gold dust
    RI(RI_BOTTLE_MILK,                "a",    "Bottle of Milk",             RITYPE_MAJOR,           ITEM_MILK_BOTTLE,                GI_MILK_BOTTLE,              GID_MILK),
    RI(RI_BOTTLE_RED_POTION,          "a",    "Bottle with Red Potion",     RITYPE_MAJOR,           ITEM_POTION_RED,                 GI_POTION_RED_BOTTLE,        GID_57), // bottle of red potion
    RI(RI_BOW,                        "a",    "Bow",                        RITYPE_MAJOR,           ITEM_BOW,                        GI_QUIVER_30,                GID_BOW),
    RI(RI_CHATEAU_ROMANI_REFILL,      "a",    "Chateau Romani Refill",      RITYPE_JUNK,            ITEM_CHATEAU_2,                  GI_CHATEAU,                  GID_CHATEAU),
    RI(RI_CLOCK_TOWN_STRAY_FAIRY,     "a",    "Clock Town Stray Fairy",     RITYPE_STRAY_FAIRY,     ITEM_STRAY_FAIRIES,              GI_STRAY_FAIRY,              GID_NONE),
    RI(RI_DEED_LAND,                  "the",  "Land Title Deed",            RITYPE_MAJOR,           ITEM_DEED_LAND,                  GI_DEED_LAND,                GID_DEED_LAND),
    RI(RI_DEED_MOUNTAIN,              "the",  "Mountain Title Deed",        RITYPE_MAJOR,           ITEM_DEED_MOUNTAIN,              GI_DEED_MOUNTAIN,            GID_DEED_MOUNTAIN),
    RI(RI_DEED_OCEAN,                 "the",  "Ocean Title Deed",           RITYPE_MAJOR,           ITEM_DEED_OCEAN,                 GI_DEED_OCEAN,               GID_DEED_OCEAN),
    RI(RI_DEED_SWAMP,                 "the",  "Swamp Title Deed",           RITYPE_MAJOR,           ITEM_DEED_SWAMP,                 GI_DEED_SWAMP,               GID_DEED_SWAMP),
    RI(RI_DEKU_NUT,                   "a",    "Deku Nut",                   RITYPE_JUNK,            ITEM_DEKU_NUT,                   GI_DEKU_NUTS_1,              GID_DEKU_NUTS),
    RI(RI_DEKU_NUTS_10,               "",     "10 Deku Nuts",               RITYPE_JUNK,            ITEM_DEKU_NUTS_10,               GI_DEKU_NUTS_10,             GID_DEKU_NUTS),
    RI(RI_DEKU_NUTS_5,                "",     "5 Deku Nuts",                RITYPE_JUNK,            ITEM_DEKU_NUTS_5,                GI_DEKU_NUTS_5,              GID_DEKU_NUTS),
    RI(RI_DEKU_SEEDS,                 "",     "Deku Seeds",                 RITYPE_JUNK,            ITEM_NONE,                       GI_NONE,                     GID_NONE), // OoT slingshot ammo (custom draw/give) — Skijer's NEI
    RI(RI_DEKU_STICK,                 "a",    "Deku Stick",                 RITYPE_JUNK,            ITEM_DEKU_STICK,                 GI_DEKU_STICKS_1,            GID_DEKU_STICK),
    RI(RI_DEKU_STICKS_5,              "",     "5 Deku Sticks",              RITYPE_JUNK,            ITEM_DEKU_STICKS_5,              GI_NONE,                     GID_DEKU_STICK),
    RI(RI_DOUBLE_DEFENSE,             "",     "Double Defense",             RITYPE_HEALTH,          ITEM_NONE,                       GI_NONE,                     GID_HEART_CONTAINER),
    RI(RI_DOUBLE_MAGIC,               "a",    "Magic Upgrade",              RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_MAGIC_JAR_BIG),
    RI(RI_FAIRY_REFILL,               "a",    "Fairy",                      RITYPE_JUNK,            ITEM_FAIRY,                      GI_FAIRY,                    GID_FAIRY_2),
    RI(RI_FAIRY_SLINGSHOT,            "a",    "Fairy Slingshot",            RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE), // OoT Fairy Slingshot (custom draw/give) — Skijer's NEI
    RI(RI_NET,                        "the",  "Bug-Catching Net",           RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE), // Bottle rando Net (custom draw/give) — Skijer's NEI
    RI(RI_BOTTOMLESS_BOTTLE,          "the",  "Bottomless Bottle",          RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE), // Bottle rando Bottomless (custom draw/give) — Skijer's NEI
    RI(RI_FROG_BLUE,                  "a",    "Blue Frog",                  RITYPE_MAJOR,           ITEM_NONE,                       GI_MASK_DON_GERO,            GID_NONE),
    RI(RI_FROG_CYAN,                  "a",    "Cyan Frog",                  RITYPE_MAJOR,           ITEM_NONE,                       GI_MASK_DON_GERO,            GID_NONE),
    RI(RI_FROG_PINK,                  "a",    "Pink Frog",                  RITYPE_MAJOR,           ITEM_NONE,                       GI_MASK_DON_GERO,            GID_NONE),
    RI(RI_FROG_WHITE,                 "a",    "White Frog",                 RITYPE_MAJOR,           ITEM_NONE,                       GI_MASK_DON_GERO,            GID_NONE),
    RI(RI_GOLD_DUST_REFILL,           "a",    "Gold Dust Refill",           RITYPE_LESSER,          ITEM_GOLD_DUST_2,                GI_GOLD_DUST_2,              GID_GOLD_DUST),
    RI(RI_GREAT_BAY_BOSS_KEY,         "the",  "Great Bay Boss Key",         RITYPE_BOSS_KEY,        ITEM_KEY_BOSS,                   GI_KEY_BOSS,                 GID_KEY_BOSS),
    RI(RI_GREAT_BAY_COMPASS,          "the",  "Great Bay Compass",          RITYPE_LESSER,          ITEM_COMPASS,                    GI_COMPASS,                  GID_COMPASS),
    RI(RI_GREAT_BAY_MAP,              "the",  "Great Bay Map",              RITYPE_LESSER,          ITEM_DUNGEON_MAP,                GI_MAP,                      GID_DUNGEON_MAP),
    RI(RI_GREAT_BAY_SMALL_KEY,        "a",    "Great Bay Small Key",        RITYPE_SMALL_KEY,       ITEM_KEY_SMALL,                  GI_KEY_SMALL,                GID_KEY_SMALL),
    RI(RI_GREAT_BAY_STRAY_FAIRY,      "a",    "Great Bay Stray Fairy",      RITYPE_STRAY_FAIRY,     ITEM_STRAY_FAIRIES,              GI_STRAY_FAIRY,              GID_NONE),
    RI(RI_GREAT_FAIRY_SWORD,          "the",  "Great Fairy's Sword",        RITYPE_LESSER,          ITEM_SWORD_GREAT_FAIRY,          GI_SWORD_GREAT_FAIRY,        GID_SWORD_GREAT_FAIRY),
    RI(RI_GREAT_SPIN_ATTACK,          "the",  "Great Spin Attack",          RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_SWORD_KOKIRI),
    RI(RI_GREEN_POTION_REFILL,        "a",    "Green Potion Refill",        RITYPE_JUNK,            ITEM_POTION_GREEN,               GI_POTION_GREEN,             GID_POTION_GREEN),
    RI(RI_GS_TOKEN_OCEAN,             "an",   "Ocean Gold Skulltula Token", RITYPE_SKULLTULA_TOKEN, ITEM_SKULL_TOKEN,                GI_SKULL_TOKEN,              GID_SKULL_TOKEN_2),
    RI(RI_GS_TOKEN_SWAMP,             "a",    "Swamp Gold Skulltula Token", RITYPE_SKULLTULA_TOKEN, ITEM_SKULL_TOKEN,                GI_SKULL_TOKEN,              GID_SKULL_TOKEN_2),
    RI(RI_HEART_CONTAINER,            "a",    "Heart Container",            RITYPE_HEALTH,          ITEM_HEART_CONTAINER,            GI_HEART_CONTAINER,          GID_HEART_CONTAINER),
    RI(RI_HEART_PIECE,                "a",    "Heart Piece",                RITYPE_HEALTH,          ITEM_HEART_PIECE,                GI_HEART_PIECE,              GID_HEART_PIECE),
    RI(RI_HOOKSHOT,                   "the",  "Hookshot",                   RITYPE_MAJOR,           ITEM_HOOKSHOT,                   GI_HOOKSHOT,                 GID_HOOKSHOT), // kept "Hookshot": this token = OoT progressive-chain tier 1 (FCI_HOOKSHOT). The MM kaleido cell for the native item reads "Clawshot" (extended_inventory.c name override); RI_CLAWSHOT below owns the "Clawshot" rando name (FCI_CLAWSHOT)
    RI(RI_CLAWSHOT,                   "the",  "Clawshot",                   RITYPE_MAJOR,           ITEM_HOOKSHOT,                   GI_HOOKSHOT,                 GID_HOOKSHOT),
    RI(RI_JUNK,                       "",     "Junk",                       RITYPE_JUNK,            ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_LENS,                       "the",  "Lens of Truth",              RITYPE_MAJOR,           ITEM_LENS_OF_TRUTH,              GI_LENS_OF_TRUTH,            GID_LENS),
    RI(RI_LETTER_TO_KAFEI,            "the",  "Letter to Kafei",            RITYPE_MAJOR,           ITEM_LETTER_TO_KAFEI,            GI_LETTER_TO_KAFEI,          GID_LETTER_TO_KAFEI),
    RI(RI_LETTER_TO_MAMA,             "the",  "Letter to Mama",             RITYPE_MAJOR,           ITEM_LETTER_MAMA,                GI_LETTER_TO_MAMA,           GID_LETTER_MAMA),
    RI(RI_MAGIC_BEAN,                 "a",    "Magic Bean",                 RITYPE_LESSER,          ITEM_MAGIC_BEANS,                GI_MAGIC_BEANS,              GID_MAGIC_BEANS),
    RI(RI_MAGIC_JAR_BIG,              "a",    "Large Magic Refill",         RITYPE_JUNK,            ITEM_MAGIC_JAR_BIG,              GI_MAGIC_JAR_BIG,            GID_MAGIC_JAR_BIG),
    RI(RI_MAGIC_JAR_SMALL,            "a",    "Small Magic Refill",         RITYPE_JUNK,            ITEM_MAGIC_JAR_SMALL,            GI_MAGIC_JAR_SMALL,          GID_MAGIC_JAR_SMALL),
    RI(RI_MASK_ALL_NIGHT,             "the",  "All-Night Mask",             RITYPE_MASK,            ITEM_MASK_ALL_NIGHT,             GI_MASK_ALL_NIGHT,           GID_MASK_ALL_NIGHT),
    RI(RI_MASK_BLAST,                 "the",  "Blast Mask",                 RITYPE_MASK,            ITEM_MASK_BLAST,                 GI_MASK_BLAST,               GID_MASK_BLAST),
    RI(RI_MASK_BREMEN,                "the",  "Bremen Mask",                RITYPE_MASK,            ITEM_MASK_BREMEN,                GI_MASK_BREMEN,              GID_MASK_BREMEN),
    RI(RI_MASK_BUNNY,                 "the",  "Bunny Hood",                 RITYPE_MASK,            ITEM_MASK_BUNNY,                 GI_MASK_BUNNY,               GID_MASK_BUNNY),
    RI(RI_MASK_CAPTAIN,               "the",  "Captain's Hat",              RITYPE_MASK,            ITEM_MASK_CAPTAIN,               GI_MASK_CAPTAIN,             GID_MASK_CAPTAIN),
    RI(RI_MASK_CIRCUS_LEADER,         "the",  "Circus Leader's Mask",       RITYPE_MASK,            ITEM_MASK_CIRCUS_LEADER,         GI_MASK_CIRCUS_LEADER,       GID_MASK_CIRCUS_LEADER),
    RI(RI_MASK_COUPLE,                "the",  "Couples Mask",               RITYPE_MASK,            ITEM_MASK_COUPLE,                GI_MASK_COUPLE,              GID_MASK_COUPLE),
    RI(RI_MASK_DEKU,                  "the",  "Deku Mask",                  RITYPE_MASK,            ITEM_MASK_DEKU,                  GI_MASK_DEKU,                GID_MASK_DEKU),
    RI(RI_MASK_DON_GERO,              "the",  "Don Gero Mask",              RITYPE_MASK,            ITEM_MASK_DON_GERO,              GI_MASK_DON_GERO,            GID_MASK_DON_GERO),
    RI(RI_MASK_FIERCE_DEITY,          "the",  "Fierce Deity Mask",          RITYPE_MASK,            ITEM_MASK_FIERCE_DEITY,          GI_MASK_FIERCE_DEITY,        GID_MASK_FIERCE_DEITY),
    RI(RI_MASK_GARO,                  "",     "Garo's Mask",                RITYPE_MASK,            ITEM_MASK_GARO,                  GI_MASK_GARO,                GID_MASK_GARO),
    RI(RI_MASK_GIANT,                 "the",  "Giant's Mask",               RITYPE_MASK,            ITEM_MASK_GIANT,                 GI_MASK_GIANT,               GID_MASK_GIANT),
    RI(RI_MASK_GIBDO,                 "the",  "Gibdo Mask",                 RITYPE_MASK,            ITEM_MASK_GIBDO,                 GI_MASK_GIBDO,               GID_MASK_GIBDO),
    RI(RI_MASK_GORON,                 "the",  "Goron Mask",                 RITYPE_MASK,            ITEM_MASK_GORON,                 GI_MASK_GORON,               GID_MASK_GORON),
    RI(RI_MASK_GREAT_FAIRY,           "the",  "Great Fairy Mask",           RITYPE_MASK,            ITEM_MASK_GREAT_FAIRY,           GI_MASK_GREAT_FAIRY,         GID_MASK_GREAT_FAIRY),
    RI(RI_MASK_KAFEIS_MASK,           "",     "Kafei's Mask",               RITYPE_MASK,            ITEM_MASK_KAFEIS_MASK,           GI_MASK_KAFEIS_MASK,         GID_MASK_KAFEIS_MASK),
    RI(RI_MASK_KAMARO,                "",     "Kamaro's Mask",              RITYPE_MASK,            ITEM_MASK_KAMARO,                GI_MASK_KAMARO,              GID_MASK_KAMARO),
    RI(RI_MASK_KEATON,                "the",  "Keaton Mask",                RITYPE_MASK,            ITEM_MASK_KEATON,                GI_MASK_KEATON,              GID_MASK_KEATON),
    RI(RI_MASK_POSTMAN,               "the",  "Postman's Hat",              RITYPE_MASK,            ITEM_MASK_POSTMAN,               GI_MASK_POSTMAN,             GID_MASK_POSTMAN),
    RI(RI_MASK_ROMANI,                "",     "Romani's Mask",              RITYPE_MASK,            ITEM_MASK_ROMANI,                GI_MASK_ROMANI,              GID_MASK_ROMANI),
    RI(RI_MASK_SCENTS,                "the",  "Mask of Scents",             RITYPE_MASK,            ITEM_MASK_SCENTS,                GI_MASK_SCENTS,              GID_MASK_SCENTS),
    RI(RI_MASK_STONE,                 "the",  "Stone Mask",                 RITYPE_MASK,            ITEM_MASK_STONE,                 GI_MASK_STONE,               GID_MASK_STONE),
    RI(RI_MASK_TRUTH,                 "the",  "Mask of Truth",              RITYPE_MASK,            ITEM_MASK_TRUTH,                 GI_MASK_TRUTH,               GID_MASK_TRUTH),
    RI(RI_MASK_ZORA,                  "the",  "Zora Mask",                  RITYPE_MASK,            ITEM_MASK_ZORA,                  GI_MASK_ZORA,                GID_MASK_ZORA),
    RI(RI_MILK_REFILL,                "a",    "Milk Refill",                RITYPE_JUNK,            ITEM_MILK,                       GI_MILK,                     GID_MILK),
    RI(RI_MOONS_TEAR,                 "the",  "Moon's Tear",                RITYPE_MAJOR,           ITEM_MOONS_TEAR,                 GI_MOONS_TEAR,               GID_MOONS_TEAR),
    RI(RI_MUSHROOM,                   "a",    "Magic Mushroom",             RITYPE_MAJOR,           ITEM_MUSHROOM,                   GI_MUSHROOM,                 GID_MUSHROOM),
    RI(RI_NONE,                       "",     "literally nothing",          RITYPE_JUNK,            ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OCARINA_BUTTON_A,           "the",  "A Button",                   RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OCARINA_BUTTON_C_DOWN,      "the",  "C Down Button",              RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OCARINA_BUTTON_C_RIGHT,     "the",  "C Right Button",             RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OCARINA_BUTTON_C_LEFT,      "the",  "C Left Button",              RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OCARINA_BUTTON_C_UP,        "the",  "C Up Button",                RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OCARINA,                    "the",  "Ocarina of Time",            RITYPE_MAJOR,           ITEM_OCARINA_OF_TIME,            GI_OCARINA_OF_TIME,          GID_OCARINA),
    // Skijer's NEI — OoT (SoH) per-dungeon items ported into MM as get-items (custom OoT-model draw, no-op give).
    // Distinct token+name per (dungeon,type); model shared per type. Names are the exact OoT English names.
    // Second wave (gear/spells/masks + NEI page-2 + ext equipment + NEI songs) is interleaved alphabetically.
    // Third wave (final cross items): SoH abilities / jabber nuts / GS token / bottled contents.
    // ITEM_NONE keeps the give a no-op (it is in GiveItem's no-op group); the GI/GID are only what the
    // default GetItem_Draw path renders, so it shows MM's own small key rather than nothing.
    RI(RI_OOT_ABILITY_CHESTS,         "",     "Open Chests",                RITYPE_MAJOR,           ITEM_NONE,                       GI_KEY_SMALL,                GID_KEY_SMALL), // SoH RG_OPEN_CHEST — no MM gate, FC record only
    RI(RI_OOT_ABILITY_CLIMB,          "",     "Climb",                      RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE), // SoH RG_CLIMB — ladder model draw
    RI(RI_OOT_ABILITY_CRAWL,          "",     "Crawl",                      RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE), // SoH RG_CRAWL — knee-pads (2 deku shields)
    RI(RI_OOT_BOMBCHU_BAG,            "a",    "Bombchu Bag",                RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_BOOMERANG,              "the",  "Boomerang",                  RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_BOSS_KEY_FIRE_TEMPLE,   "the",  "Fire Temple Boss Key",       RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_BOSS_KEY_FOREST_TEMPLE, "the",  "Forest Temple Boss Key",     RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_BOSS_KEY_GANONS_CASTLE, "the",  "Ganon's Castle Boss Key",    RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_BOSS_KEY_SHADOW_TEMPLE, "the",  "Shadow Temple Boss Key",     RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_BOSS_KEY_SPIRIT_TEMPLE, "the",  "Spirit Temple Boss Key",     RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_BOSS_KEY_WATER_TEMPLE,  "the",  "Water Temple Boss Key",      RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    // OoT bottled contents (SoH names). Where MM ships the same content the row carries MM's native
    // GID (DrawItem default GetItem_Draw path) and the give routes through MM's own bottle system.
    RI(RI_OOT_BOTTLE_BIG_POE,         "a",    "Bottle with Big Poe",        RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_BIG_POE),
    RI(RI_OOT_BOTTLE_BLUE_FIRE,       "a",    "Bottle with Blue Fire",      RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE), // no MM analog — OoT blue-fire model (custom draw)
    RI(RI_OOT_BOTTLE_BLUE_POTION,     "a",    "Bottle with Blue Potion",    RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_POTION_BLUE),
    RI(RI_OOT_BOTTLE_BUGS,            "a",    "Bottle with Bugs",           RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_BUG),
    RI(RI_OOT_BOTTLE_FAIRY,           "a",    "Bottle with Fairy",          RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_FAIRY),
    RI(RI_OOT_BOTTLE_FISH,            "a",    "Bottle with Fish",           RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_FISH),
    RI(RI_OOT_BOTTLE_GREEN_POTION,    "a",    "Bottle with Green Potion",   RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_POTION_GREEN),
    RI(RI_OOT_BOTTLE_MAGIC_MUSHROOM,  "a",    "Bottle with Magic Mushroom", RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_MUSHROOM),
    RI(RI_OOT_BOTTLE_POE,             "a",    "Bottle with Poe",            RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_POE),
    RI(RI_OOT_COMPASS_BOTTOM_OF_THE_WELL, "the", "Bottom of the Well Compass", RITYPE_LESSER,       ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_COMPASS_DEKU_TREE,      "the",  "Great Deku Tree Compass",    RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_COMPASS_DODONGOS_CAVERN,"the",  "Dodongo's Cavern Compass",   RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_COMPASS_FIRE_TEMPLE,    "the",  "Fire Temple Compass",        RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_COMPASS_FOREST_TEMPLE,  "the",  "Forest Temple Compass",      RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_COMPASS_ICE_CAVERN,     "the",  "Ice Cavern Compass",         RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_COMPASS_JABU_JABUS_BELLY,"the", "Jabu-Jabu's Belly Compass",  RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_COMPASS_SHADOW_TEMPLE,  "the",  "Shadow Temple Compass",      RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_COMPASS_SPIRIT_TEMPLE,  "the",  "Spirit Temple Compass",      RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_COMPASS_WATER_TEMPLE,   "the",  "Water Temple Compass",       RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_DEKU_SHIELD,            "the",  "Deku Shield",                RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE), // OoT Deku Shield (FC_SHIELD_DEKU); real object_gi_shield_1 mesh via direct load
    RI(RI_OOT_DINS_FIRE,              "",     "Din's Fire",                 RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_EXT_CANE_OF_BYRNA,      "the",  "Cane of Byrna",              RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_EXT_CHAMPIONS_TUNIC,    "the",  "Champion's Tunic",           RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_EXT_DIVINE_SHIELD,      "the",  "Goddess Shield",             RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_EXT_FOUR_SWORD,         "the",  "Four Sword",                 RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_EXT_MAGIC_CAPE,         "the",  "Magic Cape",                 RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_EXT_PEGASUS_ANKLET,     "the",  "Pegasus Boots",              RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_EXT_SHEIKAH_SHIELD,     "the",  "Kite Shield",                RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_EXT_SPIRIT_BREASTPLATE, "the",  "Magic Tunic",                RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_EXT_WATER_DRAGON_SCALE, "the",  "Sage's Tunic",            RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_EXT_TRIDENT,            "the",  "Trident",                    RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_EXT_CLIMB_BOOTS,        "the",  "Climb Boots",                RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_EXT_ROC_BOOTS,          "the",  "Roc's Boots",               RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_SHEIKAH_SLATE,      "the",  "Sheikah Slate",              RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_PHANTOM_HOURGLASS,  "the",  "Phantom Hourglass",          RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_SHADOW_CRYSTAL,     "the",  "Shadow Crystal",             RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_ROD_OF_SEASONS,     "the",  "Rod of Seasons",             RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    // Sheikah Slate runes — sibling items over the slate cell (wand idiom: any order, no levels).
    RI(RI_OOT_NEI_SLATE_RUNE_BOMB,     "the", "Rune: Remote Bomb",          RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_SLATE_RUNE_MASTER_CYCLE, "the", "Rune: Master Cycle",             RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_SLATE_RUNE_STASIS,   "the", "Rune: Stasis",               RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_SLATE_RUNE_CRYONIS,  "the", "Rune: Cryonis",              RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_FARORES_WIND,           "",     "Farore's Wind",              RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_FISHING_POLE,           "the",  "Fishing Pole",               RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_GERUDO_MEMBERSHIP_CARD, "the",  "Gerudo Membership Card",     RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_GORON_TUNIC,            "the",  "Goron Tunic",                RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_GREG,                   "",     "Greg the Green Rupee",       RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_GS_TOKEN,               "a",    "Gold Skulltula Token",       RITYPE_SKULLTULA_TOKEN, ITEM_NONE,                       GI_NONE,                     GID_NONE), // real OoT object_gi_sutaru mesh (direct load; MM shadows the path)
    RI(RI_OOT_HOVER_BOOTS,            "the",  "Hover Boots",                RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_IRON_BOOTS,             "the",  "Iron Boots",                 RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_KEY_RING_BOTTOM_OF_THE_WELL, "the", "Bottom of the Well Key Ring", RITYPE_MAJOR,      ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_KEY_RING_FIRE_TEMPLE,   "the",  "Fire Temple Key Ring",       RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_KEY_RING_FOREST_TEMPLE, "the",  "Forest Temple Key Ring",     RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_KEY_RING_GANONS_CASTLE, "the",  "Ganon's Castle Key Ring",    RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_KEY_RING_GERUDO_FORTRESS, "the", "Gerudo Fortress Key Ring",  RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_KEY_RING_GERUDO_TRAINING_GROUND, "the", "Training Ground Key Ring", RITYPE_MAJOR,     ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_KEY_RING_SHADOW_TEMPLE, "the",  "Shadow Temple Key Ring",     RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_KEY_RING_SPIRIT_TEMPLE, "the",  "Spirit Temple Key Ring",     RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_KEY_RING_TREASURE_GAME, "the",  "Chest Game Key Ring",        RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_KEY_RING_WATER_TEMPLE,  "the",  "Water Temple Key Ring",      RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_MAP_BOTTOM_OF_THE_WELL, "the",  "Bottom of the Well Map",     RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_MAP_DEKU_TREE,          "the",  "Great Deku Tree Map",        RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_MAP_DODONGOS_CAVERN,    "the",  "Dodongo's Cavern Map",       RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_MAP_FIRE_TEMPLE,        "the",  "Fire Temple Map",            RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_MAP_FOREST_TEMPLE,      "the",  "Forest Temple Map",          RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_MAP_ICE_CAVERN,         "the",  "Ice Cavern Map",             RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_MAP_JABU_JABUS_BELLY,   "the",  "Jabu-Jabu's Belly Map",      RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_MAP_SHADOW_TEMPLE,      "the",  "Shadow Temple Map",          RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_MAP_SPIRIT_TEMPLE,      "the",  "Spirit Temple Map",          RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_MAP_WATER_TEMPLE,       "the",  "Water Temple Map",           RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_MASK_GERUDO,            "the",  "Gerudo Mask",                RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_MASK_SKULL,             "the",  "Skull Mask",                 RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_MASK_SPOOKY,            "the",  "Spooky Mask",                RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    // Skijer's NEI — OoT (SoH) medallions, warp songs, and spiritual stones ported into MM as get-items
    // (custom OoT-model draw, no-op give). The 6 non-warp OoT songs already exist as MM's native RI_SONG_*.
    RI(RI_OOT_MEDALLION_FIRE,         "the",  "Fire Medallion",             RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_MEDALLION_FOREST,       "the",  "Forest Medallion",           RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_MEDALLION_LIGHT,        "the",  "Light Medallion",            RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_MEDALLION_SHADOW,       "the",  "Shadow Medallion",           RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_MEDALLION_SPIRIT,       "the",  "Spirit Medallion",           RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_MEDALLION_WATER,        "the",  "Water Medallion",            RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_MIRROR_SHIELD,          "the",  "Mirror Shield",              RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE), // OoT Mirror Shield (FC "Mirror Shield (OoT)"); draws MM's mirror shield model per FC vanillaShieldSkin note
    RI(RI_OOT_NAYRUS_LOVE,            "",     "Nayru's Love",               RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_BALL_AND_CHAIN,     "the",  "Ball and Chain",             RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_BEETLE,             "the",  "Beetle",                     RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_BOMB_ARROWS,        "",     "Bomb Arrows",                RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_CANE_OF_SOMARIA,    "the",  "Cane of Somaria",            RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    // The other five Dual Cane skills. Each is its own check; whichever one is found
    // first is what hands the player the cane itself (see Cane_GiveSkill).
    RI(RI_OOT_NEI_CANE_SOMARIA_BLOCK,    "the", "Somaria Block",            RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_CANE_SOMARIA_PLATFORM, "the", "Somaria Platform",         RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_CANE_PACCI_FLIP,       "the", "Cane of Pacci",            RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_CANE_PACCI_STONE,      "the", "Magic Powder",              RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_CANE_PACCI_ULTRAHAND,  "the", "Pacci Ultrahand",          RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_DEKU_LEAF,          "the",  "Deku Leaf",                  RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_DEMISE_DESTRUCTION, "",     "Demise Destruction",         RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_DESIRE_SENSOR,      "the",  "Quartz of Motion",           RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_DOMINION_ROD,       "the",  "Dominion Rod",               RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_ELEMENTAL_WAND,     "the",  "Elemental Wand",             RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_WAND_SAND_ROD,      "the",  "Sand Rod",                   RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_WAND_TORNADO_ROD,   "the",  "Tornado Rod",                RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_WAND_WATER_ROD,     "the",  "Water Rod",                  RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_WAND_METEOR_ROD,    "the",  "Meteor Rod",                 RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_WAND_STORM_ROD,     "the",  "Storm Rod",                  RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_WAND_SHADOW_SCEPTER,"the",  "Shadow Scepter",             RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_FIRE_ROD,           "the",  "Fire Rod",                   RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_GUST_JAR,           "the",  "Gust Jar",                   RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_HYLIAS_GRACE,       "",     "Hylia's Grace",              RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_ICE_ROD,            "the",  "Ice Rod",                    RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_LANTERN,            "the",  "Lantern",                    RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_LIGHT_ROD,          "the",  "Light Rod",                  RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_MINISH_CAP,         "",     "The Minish Cap",             RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_MOGMA_MITTS,        "the",  "Mogma Mitts",                RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_POKE_BALL,          "a",    "Poke Ball",                  RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE), // soh English is "Poké Ball"; é dropped (MM charmap-safe)
    RI(RI_OOT_NEI_SHOVEL,             "the",  "Shovel",                     RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_SPINNER,            "the",  "Spinner",                    RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_SWITCH_HOOK,        "the",  "Switch Hook",                RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_TIME_GATE,          "the",  "Time Gate",                  RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_WHIP,               "the",  "Whip",                       RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_NEI_ZONAI_PERMAFROST,   "the",  "Zonai Timer",                RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE), // renamed from "Zonai Permafrost" (user 2026-08-06); internal ids unchanged
    RI(RI_OOT_PROGRESSIVE_HAMMER,     "a",    "Progressive Hammer",         RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_PROGRESSIVE_BGS,        "a",    "Progressive Biggoron's Sword", RITYPE_MAJOR,        ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_PROGRESSIVE_STRENGTH,   "a",    "Progressive Strength Upgrade", RITYPE_MAJOR,        ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_PROGRESSIVE_MASTER_SWORD, "a",  "Progressive Master Sword",   RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_PROGRESSIVE_NUT_CAPACITY,  "a", "Progressive Nut Capacity",   RITYPE_MAJOR,           ITEM_NONE,                       GI_DEKU_NUTS_1,              GID_DEKU_NUTS),
    RI(RI_OOT_PROGRESSIVE_ROC,        "a",    "Progressive Roc",            RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_PROGRESSIVE_STICK_CAPACITY, "a", "Progressive Stick Capacity", RITYPE_MAJOR,          ITEM_NONE,                       GI_DEKU_STICKS_1,            GID_DEKU_STICK),
    RI(RI_OOT_ROCS_FEATHER,           "a",    "Roc's Feather",              RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE), // SoH's ship-vanilla feather (Nayru's Love slot), NOT the progressive Skijer Roc above
    RI(RI_OOT_RUTOS_LETTER,           "a",    "Bottle with Ruto's Letter",  RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE), // OoT object_gi_bottle_letter (custom draw)
    RI(RI_OOT_SKELETON_KEY,           "the",  "Skeleton Key",               RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_SMALL_KEY_BOTTOM_OF_THE_WELL, "a", "Bottom of the Well Small Key", RITYPE_MAJOR,      ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_SMALL_KEY_FIRE_TEMPLE,  "a",    "Fire Temple Small Key",      RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_SMALL_KEY_FOREST_TEMPLE,"a",    "Forest Temple Small Key",    RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_SMALL_KEY_GANONS_CASTLE,"a",    "Ganon's Castle Small Key",   RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_SMALL_KEY_GERUDO_FORTRESS, "a", "Gerudo Fortress Small Key",  RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_SMALL_KEY_GERUDO_TRAINING_GROUND, "a", "Training Ground Small Key", RITYPE_MAJOR,     ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_SMALL_KEY_SHADOW_TEMPLE,"a",    "Shadow Temple Small Key",    RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_SMALL_KEY_SPIRIT_TEMPLE,"a",    "Spirit Temple Small Key",    RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_SMALL_KEY_TREASURE_GAME,"a",    "Chest Game Small Key",       RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_SMALL_KEY_WATER_TEMPLE, "a",    "Water Temple Small Key",     RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_SONG_BALLAD_OF_THE_HERO,"the",  "Ballad of the Hero",         RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE), // NEI custom song
    RI(RI_OOT_SONG_BOLERO_OF_FIRE,    "the",  "Bolero of Fire",             RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_SONG_COMMAND_MELODY,    "the",  "Command Melody",             RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE), // NEI custom song
    RI(RI_OOT_SONG_FUGUE_OF_HOME,     "the",  "Fugue of Home",              RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE), // NEI custom song
    RI(RI_OOT_SONG_MINUET_OF_FOREST,  "the",  "Minuet of Forest",           RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_SONG_NOCTURNE_OF_SHADOW,"the",  "Nocturne of Shadow",         RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_SONG_PRELUDE_OF_LIGHT,  "the",  "Prelude of Light",           RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_SONG_REQUIEM_OF_SPIRIT, "the",  "Requiem of Spirit",          RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_SONG_SERENADE_OF_WATER, "the",  "Serenade of Water",          RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_SONG_ZELDAS_LULLABY,    "",     "Zelda's Lullaby",            RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    // SoH Skijer jabber nuts (exact SoH English names) — per-race nut mesh from soh.o2r (direct load).
    RI(RI_OOT_SPEAK_DEKU,             "a",    "Deku Jabber Nut",            RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_SPEAK_GERUDO,           "a",    "Gerudo Jabber Nut",          RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_SPEAK_GORON,            "a",    "Goron Jabber Nut",           RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_SPEAK_HYLIAN,           "a",    "Hylian Jabber Nut",          RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_SPEAK_KOKIRI,           "a",    "Kokiri Jabber Nut",          RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_SPEAK_ZORA,             "a",    "Zora Jabber Nut",            RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_STONE_GORON_RUBY,       "the",  "Goron's Ruby",               RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_STONE_KOKIRI_EMERALD,   "the",  "Kokiri's Emerald",           RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_STONE_OF_AGONY,         "the",  "Stone of Agony",             RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_STONE_ZORA_SAPPHIRE,    "the",  "Zora's Sapphire",            RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    // Skijer's NEI — OoT (SoH) trade-chain items ported into MM as get-items (custom OoT-model draw, no-op give).
    RI(RI_OOT_TRADE_BROKEN_GORONS_SWORD, "the", "Broken Goron's Sword",     RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_TRADE_CLAIM_CHECK,      "the",  "Claim Check",                RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_TRADE_COJIRO,           "",     "Cojiro",                     RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_TRADE_EYEBALL_FROG,     "the",  "Eyeball Frog",               RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_TRADE_EYEDROPS,         "the",  "World's Finest Eyedrops",    RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_TRADE_ODD_MUSHROOM,     "the",  "Odd Mushroom",               RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_TRADE_ODD_POTION,       "the",  "Odd Potion",                 RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_TRADE_POACHERS_SAW,     "the",  "Poacher's Saw",              RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_TRADE_POCKET_EGG,       "the",  "Pocket Egg",                 RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_TRADE_PRESCRIPTION,     "the",  "Prescription",               RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_TRADE_WEIRD_EGG,        "the",  "Weird Egg",                  RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_TRADE_ZELDAS_LETTER,    "",     "Zelda's Letter",             RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OOT_ZORA_TUNIC,             "the",  "Zora Tunic",                 RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OWL_CLOCK_TOWN_SOUTH,       "the",  "Clock Town Owl Statue",      RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OWL_GREAT_BAY_COAST,        "the",  "Great Bay Coast Owl Statue", RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OWL_IKANA_CANYON,           "the",  "Ikana Canyon Owl Statue",    RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OWL_MILK_ROAD,              "the",  "Milk Road Owl Statue",       RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OWL_MOUNTAIN_VILLAGE,       "the",  "Mountain Village Owl Statue",RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OWL_SNOWHEAD,               "the",  "Snowhead Owl Statue",        RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OWL_SOUTHERN_SWAMP,         "the",  "Southern Swamp Owl Statue",  RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OWL_STONE_TOWER,            "the",  "Stone Tower Owl Statue",     RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OWL_WOODFALL,               "the",  "Woodfall Owl Statue",        RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OWL_ZORA_CAPE,              "the",  "Zora Cape Owl Statue",       RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_PENDANT_OF_MEMORIES,        "the",  "Pendant of Memories",        RITYPE_MAJOR,           ITEM_PENDANT_OF_MEMORIES,        GI_PENDANT_OF_MEMORIES,      GID_PENDANT_OF_MEMORIES),
    RI(RI_PICTOGRAPH_BOX,             "a",    "Pictograph Box",             RITYPE_MAJOR,           ITEM_PICTOGRAPH_BOX,             GI_PICTOGRAPH_BOX,           GID_PICTOGRAPH_BOX),
    RI(RI_POWDER_KEG,                 "a",    "Powder Keg",                 RITYPE_MAJOR,           ITEM_POWDER_KEG,                 GI_POWDER_KEG,               GID_POWDER_KEG),
    RI(RI_PROGRESSIVE_BOMB_BAG,       "a",    "Progressive Bomb Bag",       RITYPE_MAJOR,           ITEM_BOMB_BAG_20,                GI_BOMB_BAG_20,              GID_BOMB_BAG_20),
    RI(RI_PROGRESSIVE_BOW,            "a",    "Progressive Bow",            RITYPE_MAJOR,           ITEM_BOW,                        GI_QUIVER_30,                GID_BOW),
    RI(RI_PROGRESSIVE_LULLABY,        "",     "Progressive Goron Lullaby",  RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_PROGRESSIVE_MAGIC,          "",     "Progressive Magic",          RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_MAGIC_JAR_SMALL),
    RI(RI_PROGRESSIVE_SWORD,          "a",    "Progressive Sword",          RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_SWORD_KOKIRI),
    RI(RI_PROGRESSIVE_WALLET,         "a",    "Progressive Wallet",         RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_WALLET_ADULT),
    RI(RI_QUIVER_40,                  "the",  "Large Quiver",               RITYPE_LESSER,          ITEM_QUIVER_40,                  GI_QUIVER_40,                GID_QUIVER_40),
    RI(RI_QUIVER_50,                  "the",  "Largest Quiver",             RITYPE_LESSER,          ITEM_QUIVER_50,                  GI_QUIVER_50,                GID_QUIVER_50),
    RI(RI_RECOVERY_HEART,             "a",    "Recovery Heart",             RITYPE_JUNK,            ITEM_RECOVERY_HEART,             GI_RECOVERY_HEART,           GID_RECOVERY_HEART),
    RI(RI_RED_POTION_REFILL,          "a",    "Red Potion Refill",          RITYPE_JUNK,            ITEM_POTION_RED,                 GI_POTION_RED,               GID_POTION_RED),
    RI(RI_REMAINS_GOHT,               "",     "Goht's Remains",             RITYPE_MAJOR,           ITEM_REMAINS_GOHT,               GI_REMAINS_GOHT,             GID_REMAINS_GOHT),
    RI(RI_REMAINS_GYORG,              "",     "Gyorg's Remains",            RITYPE_MAJOR,           ITEM_REMAINS_GYORG,              GI_REMAINS_GYORG,            GID_REMAINS_GYORG),
    RI(RI_REMAINS_ODOLWA,             "",     "Odolwa's Remains",           RITYPE_MAJOR,           ITEM_REMAINS_ODOLWA,             GI_REMAINS_ODOLWA,           GID_REMAINS_ODOLWA),
    RI(RI_REMAINS_TWINMOLD,           "",     "Twinmold's Remains",         RITYPE_MAJOR,           ITEM_REMAINS_TWINMOLD,           GI_REMAINS_TWINMOLD,         GID_REMAINS_TWINMOLD),
    RI(RI_ROOM_KEY,                   "the",  "Room Key",                   RITYPE_MAJOR,           ITEM_ROOM_KEY,                   GI_ROOM_KEY,                 GID_ROOM_KEY),
    RI(RI_RUPEE_BLUE,                 "a",    "Blue Rupee",                 RITYPE_JUNK,            ITEM_RUPEE_BLUE,                 GI_RUPEE_BLUE,               GID_RUPEE_BLUE),
    RI(RI_RUPEE_GREEN,                "a",    "Green Rupee",                RITYPE_JUNK,            ITEM_RUPEE_GREEN,                GI_RUPEE_GREEN,              GID_RUPEE_GREEN),
    RI(RI_RUPEE_HUGE,                 "a",    "Huge Rupee",                 RITYPE_JUNK,            ITEM_RUPEE_HUGE,                 GI_RUPEE_HUGE,               GID_RUPEE_HUGE),
    RI(RI_RUPEE_PURPLE,               "a",    "Purple Rupee",               RITYPE_JUNK,            ITEM_RUPEE_PURPLE,               GI_RUPEE_PURPLE,             GID_RUPEE_PURPLE),
    RI(RI_RUPEE_RED,                  "a",    "Red Rupee",                  RITYPE_JUNK,            ITEM_RUPEE_RED,                  GI_RUPEE_RED,                GID_RUPEE_RED),
    RI(RI_RUPEE_SILVER,               "a",    "Silver Rupee",               RITYPE_JUNK,            ITEM_RUPEE_SILVER,               GI_RUPEE_SILVER,             GID_RUPEE_SILVER),
    RI(RI_SHIELD_HERO,                "the",  "Hero's Shield",              RITYPE_MAJOR,           ITEM_SHIELD_HERO,                GI_SHIELD_HERO,              GID_SHIELD_HERO),
    RI(RI_SHIELD_MIRROR,              "the",  "Shield of Ikana",            RITYPE_MAJOR,           ITEM_SHIELD_MIRROR,              GI_SHIELD_MIRROR,            GID_SHIELD_MIRROR), // MM's vanilla Mirror Shield — displays "Shield of Ikana" (RI_OOT_MIRROR_SHIELD keeps "Mirror Shield"); spoiler name (RI_SHIELD_MIRROR) untouched
    RI(RI_SINGLE_MAGIC,               "the",  "Power of Magic",             RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_MAGIC_JAR_SMALL),
    RI(RI_SKELETON_KEY,               "the",  "Skeleton Key",               RITYPE_SMALL_KEY,       ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SNOWHEAD_BOSS_KEY,          "the",  "Snowhead Boss Key",          RITYPE_BOSS_KEY,        ITEM_KEY_BOSS,                   GI_KEY_BOSS,                 GID_KEY_BOSS),
    RI(RI_SNOWHEAD_COMPASS,           "the",  "Snowhead Compass",           RITYPE_LESSER,          ITEM_COMPASS,                    GI_COMPASS,                  GID_COMPASS),
    RI(RI_SNOWHEAD_MAP,               "the",  "Snowhead Map",               RITYPE_LESSER,          ITEM_DUNGEON_MAP,                GI_MAP,                      GID_DUNGEON_MAP),
    RI(RI_SNOWHEAD_SMALL_KEY,         "a",    "Snowhead Small Key",         RITYPE_SMALL_KEY,       ITEM_KEY_SMALL,                  GI_KEY_SMALL,                GID_KEY_SMALL),
    RI(RI_SNOWHEAD_STRAY_FAIRY,       "a",    "Snowhead Stray Fairy",       RITYPE_STRAY_FAIRY,     ITEM_STRAY_FAIRIES,              GI_STRAY_FAIRY,              GID_NONE),
    RI(RI_SONG_DOUBLE_TIME,           "the",  "Song of Double Time",        RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SONG_ELEGY,                 "the",  "Elegy of Emptiness",         RITYPE_MAJOR,           ITEM_SONG_ELEGY,                 GI_NONE,                     GID_NONE),
    RI(RI_SONG_EPONA,                 "",     "Epona's Song",               RITYPE_MAJOR,           ITEM_SONG_EPONA,                 GI_NONE,                     GID_NONE),
    RI(RI_SONG_HEALING,               "the",  "Song of Healing",            RITYPE_MAJOR,           ITEM_SONG_HEALING,               GI_NONE,                     GID_NONE),
    RI(RI_SONG_INVERTED_TIME,         "the",  "Inverted Song of Time",      RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SONG_LULLABY_INTRO,         "the",  "Goron Lullaby Intro",        RITYPE_MAJOR,           ITEM_SONG_LULLABY_INTRO,         GI_NONE,                     GID_NONE),
    RI(RI_SONG_LULLABY,               "the",  "Goron Lullaby",              RITYPE_MAJOR,           ITEM_SONG_LULLABY,               GI_NONE,                     GID_NONE),
    RI(RI_SONG_NOVA,                  "the",  "New Wave Bossa Nova",        RITYPE_MAJOR,           ITEM_SONG_NOVA,                  GI_NONE,                     GID_NONE),
    RI(RI_SONG_OATH,                  "the",  "Oath to Order",              RITYPE_MAJOR,           ITEM_SONG_OATH,                  GI_NONE,                     GID_NONE),
    RI(RI_SONG_SARIA,                 "",     "Saria's Song",               RITYPE_MAJOR,           ITEM_SONG_SARIA,                 GI_NONE,                     GID_NONE),
    RI(RI_SONG_SOARING,               "the",  "Song of Soaring",            RITYPE_MAJOR,           ITEM_SONG_SOARING,               GI_NONE,                     GID_NONE),
    RI(RI_SONG_SONATA,                "the",  "Sonata of Awakening",        RITYPE_MAJOR,           ITEM_SONG_SONATA,                GI_NONE,                     GID_NONE),
    RI(RI_SONG_STORMS,                "the",  "Song of Storms",             RITYPE_MAJOR,           ITEM_SONG_STORMS,                GI_NONE,                     GID_NONE),
    RI(RI_SONG_SUN,                   "the",  "Sun's Song",                 RITYPE_MAJOR,           ITEM_SONG_SUN,                   GI_NONE,                     GID_NONE),
    RI(RI_SONG_TIME,                  "the",  "Song of Time",               RITYPE_MAJOR,           ITEM_SONG_TIME,                  GI_NONE,                     GID_NONE),
    RI(RI_SOUL_BOSS_GOHT,             "the",  "Soul of Goht",               RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_BOSS_GYORG,            "the",  "Soul of Gyorg",              RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_BOSS_MAJORA,           "the",  "Soul of Majora",             RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_BOSS_ODOLWA,           "the",  "Soul of Odolwa",             RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_BOSS_TWINMOLD,         "the",  "Soul of Twinmold",           RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_ALIEN,           "the",  "Soul of Aliens",             RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_ARMOS,           "the",  "Soul of Armos",              RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_BAD_BAT,         "the",  "Soul of Bad Bats",           RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_BEAMOS,          "the",  "Soul of Beamos",             RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_BOE,             "the",  "Soul of Boes",               RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_BUBBLE,          "the",  "Soul of Bubbles",            RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_CAPTAIN_KEETA,   "the",  "Soul of Captain Keeta",      RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_CHUCHU,          "the",  "Soul of Chuchus",            RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_DEATH_ARMOS,     "the",  "Soul of Death Armos",        RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_DEEP_PYTHON,     "the",  "Soul of Deep Pythons",       RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_DEKU_BABA,       "the",  "Soul of Deku Babas",         RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_DEXIHAND,        "the",  "Soul of Dexihands",          RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_DINOLFOS,        "the",  "Soul of Dinolfos",           RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_DODONGO,         "the",  "Soul of Dodongos",           RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_DRAGONFLY,       "the",  "Soul of Dragonflies",        RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_EENO,            "the",  "Soul of Eenos",              RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_EYEGORE,         "the",  "Soul of Eyegores",           RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_FREEZARD,        "the",  "Soul of Freezards",          RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_GARO,            "the",  "Soul of Garos",              RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_GEKKO,           "the",  "Soul of Gekkos",             RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_GIANT_BEE,       "the",  "Soul of Giant Bees",         RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_GOMESS,          "the",  "Soul of Gomess",             RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_GUAY,            "the",  "Soul of Guays",              RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_HIPLOOP,         "the",  "Soul of Hiploops",           RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_IGOS_DU_IKANA,   "the",  "Soul of Igos du Ikana",      RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_IRON_KNUCKLE,    "the",  "Soul of Iron Knuckles",      RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_KEESE,           "the",  "Soul of Keese",              RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_LEEVER,          "the",  "Soul of Leevers",            RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_LIKE_LIKE,       "the",  "Soul of Like Likes",         RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_MAD_SCRUB,       "the",  "Soul of Mad Scrubs",         RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_NEJIRON,         "the",  "Soul of Nejirons",           RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_OCTOROK,         "the",  "Soul of Octoroks",           RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_PEAHAT,          "the",  "Soul of Peahats",            RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_PIRATE,          "the",  "Soul of Pirates",            RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_POE,             "the",  "Soul of Poes",               RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_REDEAD,          "the",  "Soul of Redeads",            RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_SHELLBLADE,      "the",  "Soul of Shellblades",        RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_SKULLFISH,       "the",  "Soul of Skullfish",          RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_SKULLTULA,       "the",  "Soul of Skulltulas",         RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_SNAPPER,         "the",  "Soul of Snappers",           RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_STALCHILD,       "the",  "Soul of Stalchildren",       RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_TAKKURI,         "the",  "Soul of Takkuri",            RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_TEKTITE,         "the",  "Soul of Tektites",           RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_WALLMASTER,      "the",  "Soul of Wallmasters",        RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_WART,            "the",  "Soul of Warts",              RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_WIZROBE,         "the",  "Soul of Wizrobes",           RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ENEMY_WOLFOS,          "the",  "Soul of Wolfos",             RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    // Skijer's NEI — OoT (SoH) rando souls ported as MM get-items. Custom OoT-drawn (no MM native model),
    // no MM gameplay effect (give is a no-op). Bean souls draw the OoT bean sprout; boss souls draw the
    // OoT tinted blue-fire flame — see DrawItem.cpp DrawOotBeanSoul/DrawOotBossSoul.
    RI(RI_SOUL_OOT_BEAN_DEATH_MOUNTAIN_CRATER, "the", "Death Mountain Crater Bean Soul", RITYPE_MAJOR, ITEM_NONE,          GI_NONE,                     GID_NONE),
    RI(RI_SOUL_OOT_BEAN_DEATH_MOUNTAIN_TRAIL,  "the", "Death Mountain Trail Bean Soul",  RITYPE_MAJOR, ITEM_NONE,          GI_NONE,                     GID_NONE),
    RI(RI_SOUL_OOT_BEAN_DESERT_COLOSSUS,       "the", "Desert Colossus Bean Soul",       RITYPE_MAJOR, ITEM_NONE,          GI_NONE,                     GID_NONE),
    RI(RI_SOUL_OOT_BEAN_GERUDO_VALLEY,         "the", "Gerudo Valley Bean Soul",         RITYPE_MAJOR, ITEM_NONE,          GI_NONE,                     GID_NONE),
    RI(RI_SOUL_OOT_BEAN_GRAVEYARD,             "the", "Graveyard Bean Soul",             RITYPE_MAJOR, ITEM_NONE,          GI_NONE,                     GID_NONE),
    RI(RI_SOUL_OOT_BEAN_KOKIRI_FOREST,         "the", "Kokiri Forest Bean Soul",         RITYPE_MAJOR, ITEM_NONE,          GI_NONE,                     GID_NONE),
    RI(RI_SOUL_OOT_BEAN_LAKE_HYLIA,            "the", "Lake Hylia Bean Soul",            RITYPE_MAJOR, ITEM_NONE,          GI_NONE,                     GID_NONE),
    RI(RI_SOUL_OOT_BEAN_LOST_WOODS,            "the", "Lost Woods Bean Soul",            RITYPE_MAJOR, ITEM_NONE,          GI_NONE,                     GID_NONE),
    RI(RI_SOUL_OOT_BEAN_LOST_WOODS_BRIDGE,     "the", "Lost Woods Bridge Bean Soul",     RITYPE_MAJOR, ITEM_NONE,          GI_NONE,                     GID_NONE),
    RI(RI_SOUL_OOT_BEAN_ZORAS_RIVER,           "the", "Zora's River Bean Soul",          RITYPE_MAJOR, ITEM_NONE,          GI_NONE,                     GID_NONE),
    RI(RI_SOUL_OOT_BOSS_BARINADE,              "",    "Barinade's Soul",                 RITYPE_MAJOR, ITEM_NONE,          GI_NONE,                     GID_NONE),
    RI(RI_SOUL_OOT_BOSS_BONGO_BONGO,           "",    "Bongo Bongo's Soul",              RITYPE_MAJOR, ITEM_NONE,          GI_NONE,                     GID_NONE),
    RI(RI_SOUL_OOT_BOSS_GANON,                 "",    "Ganon's Soul",                    RITYPE_MAJOR, ITEM_NONE,          GI_NONE,                     GID_NONE),
    RI(RI_SOUL_OOT_BOSS_GOHMA,                 "",    "Gohma's Soul",                    RITYPE_MAJOR, ITEM_NONE,          GI_NONE,                     GID_NONE),
    RI(RI_SOUL_OOT_BOSS_KING_DODONGO,          "",    "King Dodongo's Soul",             RITYPE_MAJOR, ITEM_NONE,          GI_NONE,                     GID_NONE),
    RI(RI_SOUL_OOT_BOSS_MORPHA,                "",    "Morpha's Soul",                   RITYPE_MAJOR, ITEM_NONE,          GI_NONE,                     GID_NONE),
    RI(RI_SOUL_OOT_BOSS_PHANTOM_GANON,         "",    "Phantom Ganon's Soul",            RITYPE_MAJOR, ITEM_NONE,          GI_NONE,                     GID_NONE),
    RI(RI_SOUL_OOT_BOSS_TWINROVA,              "",    "Twinrova's Soul",                 RITYPE_MAJOR, ITEM_NONE,          GI_NONE,                     GID_NONE),
    RI(RI_SOUL_OOT_BOSS_VOLVAGIA,              "",    "Volvagia's Soul",                 RITYPE_MAJOR, ITEM_NONE,          GI_NONE,                     GID_NONE),
    RI(RI_STONE_TOWER_BOSS_KEY,       "the",  "Stone Tower Boss Key",       RITYPE_BOSS_KEY,        ITEM_KEY_BOSS,                   GI_KEY_BOSS,                 GID_KEY_BOSS),
    RI(RI_STONE_TOWER_COMPASS,        "the",  "Stone Tower Compass",        RITYPE_LESSER,          ITEM_COMPASS,                    GI_COMPASS,                  GID_COMPASS),
    RI(RI_STONE_TOWER_MAP,            "the",  "Stone Tower Map",            RITYPE_LESSER,          ITEM_DUNGEON_MAP,                GI_MAP,                      GID_DUNGEON_MAP),
    RI(RI_STONE_TOWER_SMALL_KEY,      "a",    "Stone Tower Small Key",      RITYPE_SMALL_KEY,       ITEM_KEY_SMALL,                  GI_KEY_SMALL,                GID_KEY_SMALL),
    RI(RI_STONE_TOWER_STRAY_FAIRY,    "a",    "Stone Tower Stray Fairy",    RITYPE_STRAY_FAIRY,     ITEM_STRAY_FAIRIES,              GI_STRAY_FAIRY,              GID_NONE),
    RI(RI_SWORD_GILDED,               "the",  "Gilded Sword",               RITYPE_LESSER,          ITEM_SWORD_GILDED,               GI_SWORD_GILDED,             GID_SWORD_GILDED),
    RI(RI_SWORD_KOKIRI,               "the",  "Kokiri Sword",               RITYPE_MAJOR,           ITEM_SWORD_KOKIRI,               GI_SWORD_KOKIRI,             GID_SWORD_KOKIRI),
    RI(RI_SWORD_RAZOR,                "the",  "Razor Sword",                RITYPE_LESSER,          ITEM_SWORD_RAZOR,                GI_SWORD_RAZOR,              GID_SWORD_RAZOR),
    RI(RI_TIME_DAY_1,                 "",     "Time (Day 1)",               RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_TIME_DAY_2,                 "",     "Time (Day 2)",               RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_TIME_DAY_3,                 "",     "Time (Day 3)",               RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_TIME_NIGHT_1,               "",     "Time (Night 1)",             RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_TIME_NIGHT_2,               "",     "Time (Night 2)",             RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_TIME_NIGHT_3,               "",     "Time (Night 3)",             RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_TIME_PROGRESSIVE,           "",     "Progressive Time",           RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_TINGLE_MAP_CLOCK_TOWN,      "",     "Tingle's Clock Town Map",    RITYPE_LESSER,          ITEM_TINGLE_MAP,                 GI_TINGLE_MAP_CLOCK_TOWN,    GID_TINGLE_MAP),
    RI(RI_TINGLE_MAP_GREAT_BAY,       "",     "Tingle's Great Bay Map",     RITYPE_LESSER,          ITEM_TINGLE_MAP,                 GI_TINGLE_MAP_GREAT_BAY,     GID_TINGLE_MAP),
    RI(RI_TINGLE_MAP_ROMANI_RANCH,    "",     "Tingle's Romani Ranch Map",  RITYPE_LESSER,          ITEM_TINGLE_MAP,                 GI_TINGLE_MAP_ROMANI_RANCH,  GID_TINGLE_MAP),
    RI(RI_TINGLE_MAP_SNOWHEAD,        "",     "Tingle's Snowhead Map",      RITYPE_LESSER,          ITEM_TINGLE_MAP,                 GI_TINGLE_MAP_SNOWHEAD,      GID_TINGLE_MAP),
    RI(RI_TINGLE_MAP_STONE_TOWER,     "",     "Tingle's Stone Tower Map",   RITYPE_LESSER,          ITEM_TINGLE_MAP,                 GI_TINGLE_MAP_STONE_TOWER,   GID_TINGLE_MAP),
    RI(RI_TINGLE_MAP_WOODFALL,        "",     "Tingle's Woodfall Map",      RITYPE_LESSER,          ITEM_TINGLE_MAP,                 GI_TINGLE_MAP_WOODFALL,      GID_TINGLE_MAP),
    RI(RI_TRAP,                       "a",    "Knockoff Item",              RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_TRIFORCE_PIECE_PREVIOUS,    "a",    "Piece of the Triforce",      RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE), // This only exists to aid in the drawing of unique models, it has no use outside of that.
    RI(RI_TRIFORCE_PIECE,             "a",    "Piece of the Triforce",      RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_WALLET_ADULT,               "the",  "Adult's Wallet",             RITYPE_MAJOR,           ITEM_WALLET_ADULT,               GI_WALLET_ADULT,             GID_WALLET_ADULT),
    RI(RI_WALLET_GIANT,               "the",  "Giant's Wallet",             RITYPE_LESSER,          ITEM_WALLET_GIANT,               GI_WALLET_GIANT,             GID_WALLET_GIANT),
    RI(RI_WALLET_TYCOON,              "the",  "Tycoon's Wallet",            RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_WALLET_GIANT),
    RI(RI_WOODFALL_BOSS_KEY,          "the",  "Woodfall Boss Key",          RITYPE_BOSS_KEY,        ITEM_KEY_BOSS,                   GI_KEY_BOSS,                 GID_KEY_BOSS),
    RI(RI_WOODFALL_COMPASS,           "the",  "Woodfall Compass",           RITYPE_LESSER,          ITEM_COMPASS,                    GI_COMPASS,                  GID_COMPASS),
    RI(RI_WOODFALL_MAP,               "the",  "Woodfall Map",               RITYPE_LESSER,          ITEM_DUNGEON_MAP,                GI_MAP,                      GID_DUNGEON_MAP),
    RI(RI_WOODFALL_SMALL_KEY,         "a",    "Woodfall Small Key",         RITYPE_SMALL_KEY,       ITEM_KEY_SMALL,                  GI_KEY_SMALL,                GID_KEY_SMALL),
    RI(RI_WOODFALL_STRAY_FAIRY,       "a",    "Woodfall Stray Fairy",       RITYPE_STRAY_FAIRY,     ITEM_STRAY_FAIRIES,              GI_STRAY_FAIRY,              GID_NONE),
};

std::map<StartingItemCategory, std::vector<RandoItemId>> StartingItemsMap = {
    { STARTING_ITEMS_INVENTORY, 
        { RI_OCARINA,               RI_PROGRESSIVE_BOW, RI_ARROW_FIRE,  RI_ARROW_ICE,   RI_ARROW_LIGHT,
          RI_PROGRESSIVE_BOMB_BAG,  RI_BOMBCHU,         RI_DEKU_STICK,  RI_DEKU_NUT,    RI_MAGIC_BEAN,
          RI_POWDER_KEG,            RI_PICTOGRAPH_BOX,  RI_LENS,        RI_HOOKSHOT,    RI_GREAT_FAIRY_SWORD,
          RI_BOTTLE_EMPTY
        } },
    { STARTING_ITEMS_MASK,
        { RI_MASK_POSTMAN,  RI_MASK_ALL_NIGHT,      RI_MASK_BLAST,          RI_MASK_STONE,      RI_MASK_GREAT_FAIRY,    RI_MASK_DEKU,
          RI_MASK_KEATON,   RI_MASK_BREMEN,         RI_MASK_BUNNY,          RI_MASK_DON_GERO,   RI_MASK_SCENTS,         RI_MASK_GORON,
          RI_MASK_ROMANI,   RI_MASK_CIRCUS_LEADER,  RI_MASK_KAFEIS_MASK,    RI_MASK_COUPLE,     RI_MASK_TRUTH,          RI_MASK_ZORA,
          RI_MASK_KAMARO,   RI_MASK_GIBDO,          RI_MASK_GARO,           RI_MASK_CAPTAIN,    RI_MASK_GIANT,          RI_MASK_FIERCE_DEITY
        } },
    { STARTING_ITEMS_QUEST,
        { RI_BOMBERS_NOTEBOOK,  RI_REMAINS_ODOLWA,      RI_REMAINS_GOHT,        RI_REMAINS_GYORG,       RI_REMAINS_TWINMOLD,
          RI_PROGRESSIVE_SWORD, RI_SHIELD_HERO,         RI_SHIELD_MIRROR,       RI_PROGRESSIVE_MAGIC,   RI_DOUBLE_DEFENSE,  
          RI_PROGRESSIVE_WALLET,
          RI_SONG_TIME,         RI_SONG_HEALING,        RI_SONG_EPONA,          RI_SONG_SOARING,    RI_SONG_STORMS,
          RI_SONG_SONATA,       RI_PROGRESSIVE_LULLABY, RI_SONG_NOVA,           RI_SONG_ELEGY,      RI_SONG_OATH,
          RI_SONG_DOUBLE_TIME,  RI_SONG_INVERTED_TIME,  RI_SONG_SUN,            RI_SONG_SARIA,
        } },
    { STARTING_ITEMS_TRADE,
        { RI_MOONS_TEAR, RI_DEED_LAND, RI_DEED_SWAMP, RI_DEED_MOUNTAIN, RI_DEED_OCEAN, RI_ROOM_KEY, RI_LETTER_TO_MAMA,
          RI_LETTER_TO_KAFEI, RI_PENDANT_OF_MEMORIES
        } },
    { STARTING_ITEMS_MISC, 
        { RI_SOUL_BOSS_GOHT, RI_SOUL_BOSS_GYORG,  RI_SOUL_BOSS_MAJORA, RI_SOUL_BOSS_ODOLWA, RI_SOUL_BOSS_TWINMOLD,
          RI_FROG_BLUE, RI_FROG_CYAN,   RI_FROG_PINK,   RI_FROG_WHITE,
          RI_TIME_DAY_1, RI_TIME_NIGHT_1, RI_TIME_DAY_2, RI_TIME_NIGHT_2, RI_TIME_DAY_3, RI_TIME_NIGHT_3, RI_TIME_PROGRESSIVE,
          RI_OCARINA_BUTTON_A, RI_OCARINA_BUTTON_C_DOWN, RI_OCARINA_BUTTON_C_RIGHT, RI_OCARINA_BUTTON_C_LEFT, RI_OCARINA_BUTTON_C_UP,
        } }, 
};

std::map<RandoItemId, u8> MaxStartingItemsMap = {
    { RI_PROGRESSIVE_SWORD, 3 }, { RI_PROGRESSIVE_BOMB_BAG, 3 }, { RI_PROGRESSIVE_WALLET, 2 },
    { RI_PROGRESSIVE_BOW, 3 },   { RI_PROGRESSIVE_LULLABY, 2 },  { RI_PROGRESSIVE_MAGIC, 2 },
    { RI_TIME_PROGRESSIVE, 6 },
};
// clang-format on

RandoItemId GetItemIdFromName(const char* name) {
    for (auto& [randoItemId, randoStaticItem] : Items) {
        if (strcmp(name, randoStaticItem.spoilerName) == 0) {
            return randoItemId;
        }
    }
    return RI_UNKNOWN;
}

RandoItemId GetItemIdFromVanillaItemId(u32 itemId) {
    for (auto& [randoItemId, randoStaticItem] : Items) {
        if (randoStaticItem.itemId == itemId) {
            return randoItemId;
        }
    }
    return RI_UNKNOWN;
}

// This exists because of nintendo being nintendo
u8 GetIconForZMessage(RandoItemId randoItemId) {
    switch (randoItemId) {
        case RI_MASK_CAPTAIN:
            return GI_MASK_TRUTH;
        case RI_MASK_TRUTH:
            return GI_MASK_CAPTAIN;
        case RI_MASK_GIANT:
            return GI_MASK_KAFEIS_MASK;
        case RI_MASK_KAFEIS_MASK:
            return GI_MASK_GIANT;
        case RI_BOMBCHU:
        case RI_BOMBCHU_5:
            return GI_BOMBCHUS_10;
        case RI_BOW:
            return GI_ARROWS_10;
        case RI_DEKU_STICKS_5:
            return GI_DEKU_STICKS_1;
        case RI_DOUBLE_DEFENSE:
            return GI_HEART_CONTAINER;
        case RI_SINGLE_MAGIC:
            return GI_MAGIC_JAR_SMALL;
        case RI_DOUBLE_MAGIC:
            return GI_MAGIC_JAR_BIG;
        case RI_GREAT_SPIN_ATTACK:
            return GI_SWORD_KOKIRI;
        // 2S2H [Rando] Stray fairies use sentinel icon bytes 0xF1-0xF4 so z_message.c knows WHICH
        // fairy icon/colors to draw. The vanilla path picks the texture from
        // gSaveContext.dungeonSceneSharedIndex, which is garbage outside the four dungeons and
        // rendered a corrupted textbox icon (e.g. a Woodfall Stray Fairy found in the overworld).
        // Clock Town's fairy shares Stone Tower's icon (parity with GetIconTexturePath below).
        case RI_WOODFALL_STRAY_FAIRY:
            return 0xF1;
        case RI_SNOWHEAD_STRAY_FAIRY:
            return 0xF2;
        case RI_GREAT_BAY_STRAY_FAIRY:
            return 0xF3;
        case RI_STONE_TOWER_STRAY_FAIRY:
        case RI_CLOCK_TOWN_STRAY_FAIRY:
            return 0xF4;
        default:
            break;
    }

    // The icon byte indexes D_801CFF94[250] in z_message.c, and 0xF1+ are our sentinels — only pass
    // through getItemIds that stay inside the vanilla-meaningful range AND map to a real native icon
    // (9999 == z_message.c's MESSAGE_ITEM_NONE). Anything else (custom items with GI_NONE, extended
    // GIs, GIs in unused table slots) falls through to the custom-texture path below instead of
    // producing an OOB/garbage native icon.
    {
        s16 getItemId = Rando::StaticData::Items[randoItemId].getItemId;
        if (getItemId != GI_NONE && getItemId > 0 && getItemId < 0xF1 && D_801CFF94[getItemId] != 9999) {
            return (u8)getItemId;
        }
    }

    // 2S2H [Rando] Textbox-only overrides for items whose toast texture is format-unsafe for the
    // textbox blit: enemy + OoT boss souls (toast uses gDungeonMapSkullTex, a map-screen texture)
    // get the Captain's Hat keeta-skull icon; clock items (toast uses the HUD sun/moon hour texes)
    // get the Moon's Tear icon. Both are rgba32 32x32 gItemIcons — the toast keeps its themed art
    // via GetIconTexturePath. MM boss souls already stage their remains icons (safe family).
    if ((randoItemId >= RI_SOUL_ENEMY_ALIEN && randoItemId <= RI_SOUL_ENEMY_WOLFOS) ||
        (randoItemId >= RI_SOUL_OOT_BOSS_BARINADE && randoItemId <= RI_SOUL_OOT_BOSS_VOLVAGIA)) {
        Message_StageCustomItemIcon((void*)gItemIcons[ITEM_MASK_CAPTAIN], 32);
        return 0xF5;
    }
    switch (randoItemId) {
        case RI_TIME_DAY_1:
        case RI_TIME_DAY_2:
        case RI_TIME_DAY_3:
        case RI_TIME_NIGHT_1:
        case RI_TIME_NIGHT_2:
        case RI_TIME_NIGHT_3:
        case RI_TIME_PROGRESSIVE:
            Message_StageCustomItemIcon((void*)gItemIcons[ITEM_MOONS_TEAR], 32);
            return 0xF5;
        default:
            break;
    }

    // 2S2H [Rando] Items with no native MM get-item icon (Net, Bottomless Bottle, the RI_OOT_* waves,
    // NEI customs, ...): reuse their notification icon in the textbox when it belongs to a family with
    // a known rgba32 square layout, staged for the 0xF5 sentinel handled in z_message.c. Textures with
    // other formats/sizes (song notes, owl face, clock sun/moon, ...) safely show no icon instead.
    const char* texturePath = GetIconTexturePath(randoItemId);
    if (texturePath != nullptr) {
        s16 size = 0;
        if (strstr(texturePath, "icon_item_24_static") != nullptr) {
            size = 24; // OoT + MM 24x24 rgba32 quest icons
        } else if (strstr(texturePath, "icon_item_custom") != nullptr ||
                   strstr(texturePath, "/gItemIcon") != nullptr) {
            size = 32; // custom (2ship.o2r) + OoT/MM native item icons, rgba32 32x32
        }
        if (size != 0) {
            Message_StageCustomItemIcon((void*)texturePath, size);
            return 0xF5;
        }
    }

    return 0xFE;
}

const char* GetIconTexturePath(RandoItemId randoItemId) {
    switch (randoItemId) {
        case RI_BOMBCHU:
        case RI_BOMBCHU_5:
            return (const char*)gItemIcons[ITEM_BOMBCHU];
        case RI_DEKU_STICKS_5:
            return (const char*)gItemIcons[ITEM_DEKU_STICK];
        case RI_DOUBLE_DEFENSE:
            return (const char*)gItemIcons[ITEM_HEART_CONTAINER];
        case RI_SINGLE_MAGIC:
            return (const char*)gItemIcons[ITEM_MAGIC_JAR_SMALL];
        case RI_DOUBLE_MAGIC:
            return (const char*)gItemIcons[ITEM_MAGIC_JAR_BIG];
        case RI_GREAT_SPIN_ATTACK:
            return (const char*)gItemIcons[ITEM_SWORD_KOKIRI];
        case RI_CLOCK_TOWN_STRAY_FAIRY:
        case RI_STONE_TOWER_STRAY_FAIRY:
            return (const char*)gStrayFairyStoneTowerIconTex;
        case RI_SNOWHEAD_STRAY_FAIRY:
            return (const char*)gStrayFairySnowheadIconTex;
        case RI_GREAT_BAY_STRAY_FAIRY:
            return (const char*)gStrayFairyGreatBayIconTex;
        case RI_WOODFALL_STRAY_FAIRY:
            return (const char*)gStrayFairyWoodfallIconTex;
        case RI_SNOWHEAD_COMPASS:
        case RI_GREAT_BAY_COMPASS:
        case RI_WOODFALL_COMPASS:
        case RI_STONE_TOWER_COMPASS:
            return (const char*)gItemIcons[ITEM_DUNGEON_MAP];
        case RI_SNOWHEAD_MAP:
        case RI_GREAT_BAY_MAP:
        case RI_WOODFALL_MAP:
        case RI_STONE_TOWER_MAP:
            return (const char*)gItemIcons[ITEM_COMPASS];
        case RI_PROGRESSIVE_BOMB_BAG:
            return (const char*)gItemIcons[ITEM_BOMB_BAG_20];
        case RI_PROGRESSIVE_BOW:
            return (const char*)gItemIcons[ITEM_BOW];
        case RI_PROGRESSIVE_SWORD:
            return (const char*)gItemIcons[ITEM_SWORD_KOKIRI];
        case RI_PROGRESSIVE_WALLET:
            return (const char*)gItemIcons[ITEM_WALLET_ADULT];
        case RI_WALLET_TYCOON:
            return (const char*)gItemIcons[ITEM_WALLET_GIANT];
        case RI_PROGRESSIVE_LULLABY:
            return (const char*)gItemIcons[ITEM_SONG_LULLABY];
        case RI_PROGRESSIVE_MAGIC:
            return (const char*)gItemIcons[ITEM_MAGIC_JAR_SMALL];
        case RI_SOUL_BOSS_GOHT:
            return (const char*)gItemIcons[ITEM_REMAINS_GOHT];
        case RI_SOUL_BOSS_GYORG:
            return (const char*)gItemIcons[ITEM_REMAINS_GYORG];
        case RI_SOUL_BOSS_ODOLWA:
            return (const char*)gItemIcons[ITEM_REMAINS_ODOLWA];
        case RI_SOUL_BOSS_TWINMOLD:
            return (const char*)gItemIcons[ITEM_REMAINS_TWINMOLD];
        case RI_SOUL_BOSS_MAJORA:
        case RI_SOUL_ENEMY_ALIEN:
        case RI_SOUL_ENEMY_ARMOS:
        case RI_SOUL_ENEMY_BAD_BAT:
        case RI_SOUL_ENEMY_BEAMOS:
        case RI_SOUL_ENEMY_BOE:
        case RI_SOUL_ENEMY_BUBBLE:
        case RI_SOUL_ENEMY_CAPTAIN_KEETA:
        case RI_SOUL_ENEMY_CHUCHU:
        case RI_SOUL_ENEMY_DEATH_ARMOS:
        case RI_SOUL_ENEMY_DEEP_PYTHON:
        case RI_SOUL_ENEMY_DEKU_BABA:
        case RI_SOUL_ENEMY_DEXIHAND:
        case RI_SOUL_ENEMY_DINOLFOS:
        case RI_SOUL_ENEMY_DODONGO:
        case RI_SOUL_ENEMY_DRAGONFLY:
        case RI_SOUL_ENEMY_EENO:
        case RI_SOUL_ENEMY_EYEGORE:
        case RI_SOUL_ENEMY_FREEZARD:
        case RI_SOUL_ENEMY_GARO:
        case RI_SOUL_ENEMY_GEKKO:
        case RI_SOUL_ENEMY_GIANT_BEE:
        case RI_SOUL_ENEMY_GOMESS:
        case RI_SOUL_ENEMY_GUAY:
        case RI_SOUL_ENEMY_HIPLOOP:
        case RI_SOUL_ENEMY_IGOS_DU_IKANA:
        case RI_SOUL_ENEMY_IRON_KNUCKLE:
        case RI_SOUL_ENEMY_KEESE:
        case RI_SOUL_ENEMY_LEEVER:
        case RI_SOUL_ENEMY_LIKE_LIKE:
        case RI_SOUL_ENEMY_MAD_SCRUB:
        case RI_SOUL_ENEMY_NEJIRON:
        case RI_SOUL_ENEMY_OCTOROK:
        case RI_SOUL_ENEMY_PEAHAT:
        case RI_SOUL_ENEMY_PIRATE:
        case RI_SOUL_ENEMY_POE:
        case RI_SOUL_ENEMY_REDEAD:
        case RI_SOUL_ENEMY_SHELLBLADE:
        case RI_SOUL_ENEMY_SKULLFISH:
        case RI_SOUL_ENEMY_SKULLTULA:
        case RI_SOUL_ENEMY_SNAPPER:
        case RI_SOUL_ENEMY_STALCHILD:
        case RI_SOUL_ENEMY_TAKKURI:
        case RI_SOUL_ENEMY_TEKTITE:
        case RI_SOUL_ENEMY_WALLMASTER:
        case RI_SOUL_ENEMY_WART:
        case RI_SOUL_ENEMY_WIZROBE:
        case RI_SOUL_ENEMY_WOLFOS:
        // Skijer's NEI — OoT boss souls reuse the generic skull soul icon (like MM's own souls above).
        case RI_SOUL_OOT_BOSS_BARINADE:
        case RI_SOUL_OOT_BOSS_BONGO_BONGO:
        case RI_SOUL_OOT_BOSS_GANON:
        case RI_SOUL_OOT_BOSS_GOHMA:
        case RI_SOUL_OOT_BOSS_KING_DODONGO:
        case RI_SOUL_OOT_BOSS_MORPHA:
        case RI_SOUL_OOT_BOSS_PHANTOM_GANON:
        case RI_SOUL_OOT_BOSS_TWINROVA:
        case RI_SOUL_OOT_BOSS_VOLVAGIA:
            return (const char*)gDungeonMapSkullTex;
        // Skijer's NEI — OoT bean souls use MM's native magic-bean item icon.
        case RI_SOUL_OOT_BEAN_DEATH_MOUNTAIN_CRATER:
        case RI_SOUL_OOT_BEAN_DEATH_MOUNTAIN_TRAIL:
        case RI_SOUL_OOT_BEAN_DESERT_COLOSSUS:
        case RI_SOUL_OOT_BEAN_GERUDO_VALLEY:
        case RI_SOUL_OOT_BEAN_GRAVEYARD:
        case RI_SOUL_OOT_BEAN_KOKIRI_FOREST:
        case RI_SOUL_OOT_BEAN_LAKE_HYLIA:
        case RI_SOUL_OOT_BEAN_LOST_WOODS:
        case RI_SOUL_OOT_BEAN_LOST_WOODS_BRIDGE:
        case RI_SOUL_OOT_BEAN_ZORAS_RIVER:
            return (const char*)gItemIcons[ITEM_MAGIC_BEANS];
        case RI_FROG_BLUE:
        case RI_FROG_CYAN:
        case RI_FROG_PINK:
        case RI_FROG_WHITE:
            return (const char*)gItemIcons[ITEM_MASK_DON_GERO];
        case RI_OWL_CLOCK_TOWN_SOUTH:
        case RI_OWL_GREAT_BAY_COAST:
        case RI_OWL_IKANA_CANYON:
        case RI_OWL_MILK_ROAD:
        case RI_OWL_MOUNTAIN_VILLAGE:
        case RI_OWL_SNOWHEAD:
        case RI_OWL_SOUTHERN_SWAMP:
        case RI_OWL_STONE_TOWER:
        case RI_OWL_WOODFALL:
        case RI_OWL_ZORA_CAPE:
            return (const char*)gWorldMapOwlFaceTex;
        case RI_TINGLE_MAP_CLOCK_TOWN:
        case RI_TINGLE_MAP_GREAT_BAY:
        case RI_TINGLE_MAP_ROMANI_RANCH:
        case RI_TINGLE_MAP_SNOWHEAD:
        case RI_TINGLE_MAP_STONE_TOWER:
        case RI_TINGLE_MAP_WOODFALL:
            return (const char*)gItemIconTingleMapTex;
        case RI_SKELETON_KEY:
            return (const char*)gItemIcons[ITEM_KEY_SMALL];
        case RI_TRIFORCE_PIECE:
            return (const char*)gTriforcePieceTex;
        case RI_OCARINA_BUTTON_A:
            return (const char*)gOcarinaATex;
        case RI_OCARINA_BUTTON_C_DOWN:
            return (const char*)gOcarinaCDownTex;
        case RI_OCARINA_BUTTON_C_LEFT:
            return (const char*)gOcarinaCLeftTex;
        case RI_OCARINA_BUTTON_C_RIGHT:
            return (const char*)gOcarinaCRightTex;
        case RI_OCARINA_BUTTON_C_UP:
            return (const char*)gOcarinaCUpTex;
        case RI_TIME_DAY_1:
        case RI_TIME_DAY_2:
        case RI_TIME_DAY_3:
            return (const char*)gThreeDayClockSunHourTex;
        case RI_TIME_NIGHT_1:
        case RI_TIME_NIGHT_2:
        case RI_TIME_NIGHT_3:
            return (const char*)gThreeDayClockMoonHourTex;
        case RI_TIME_PROGRESSIVE:
            return (const char*)gThreeDayClockSunHourTex;
        case RI_ABILITY_SWIM:
            return (const char*)gFlippersTex;
        case RI_SONG_DOUBLE_TIME:
        case RI_SONG_INVERTED_TIME:
            return (const char*)gItemIcons[ITEM_SONG_TIME];
        case RI_SONG_SARIA:
            return (const char*)gItemIcons[ITEM_SONG_SARIA];
        // Skijer's NEI — OoT-only items with no MM native item icon. Deku Seeds pulls the OoT
        // slingshot-ammo icon from oot.o2r; the Fairy Slingshot uses MM's own slingshot icon.
        case RI_DEKU_SEEDS:
            return "__OTR__textures/icon_item_static/gItemIconDekuSeedsTex";
        case RI_FAIRY_SLINGSHOT:
            return (const char*)gItemIconSlingshotTex;
        // Skijer's NEI bottle rando — Net + Bottomless Bottle custom icons (2ship.o2r icon_item_custom).
        case RI_NET:
            return "__OTR__textures/icon_item_custom/gItemIconNetTex";
        case RI_BOTTOMLESS_BOTTLE:
            return "__OTR__textures/icon_item_custom/gItemIconBottomlessBottleTex";
        // Skijer's NEI — OoT (SoH) trade-chain items pull their icons from oot.o2r (no MM native icon).
        case RI_OOT_TRADE_BROKEN_GORONS_SWORD:
            return "__OTR__textures/icon_item_static/gItemIconBrokenGoronsSwordTex";
        case RI_OOT_TRADE_CLAIM_CHECK:
            return "__OTR__textures/icon_item_static/gItemIconClaimCheckTex";
        case RI_OOT_TRADE_COJIRO:
            return "__OTR__textures/icon_item_static/gItemIconCojiroTex";
        case RI_OOT_TRADE_EYEBALL_FROG:
            return "__OTR__textures/icon_item_static/gItemIconEyeballFrogTex";
        case RI_OOT_TRADE_EYEDROPS:
            return "__OTR__textures/icon_item_static/gItemIconEyeDropsTex";
        case RI_OOT_TRADE_ODD_MUSHROOM:
            return "__OTR__textures/icon_item_static/gItemIconOddMushroomTex";
        case RI_OOT_TRADE_ODD_POTION:
            return "__OTR__textures/icon_item_static/gItemIconOddPotionTex";
        case RI_OOT_TRADE_POACHERS_SAW:
            return "__OTR__textures/icon_item_static/gItemIconPoachersSawTex";
        case RI_OOT_TRADE_POCKET_EGG:
            return "__OTR__textures/icon_item_static/gItemIconPocketEggTex";
        case RI_OOT_TRADE_PRESCRIPTION:
            return "__OTR__textures/icon_item_static/gItemIconPrescriptionTex";
        case RI_OOT_TRADE_WEIRD_EGG:
            return "__OTR__textures/icon_item_static/gItemIconWeirdEggTex";
        case RI_OOT_TRADE_ZELDAS_LETTER:
            return "__OTR__textures/icon_item_static/gItemIconZeldasLetterTex";
        // Skijer's NEI — OoT (SoH) per-dungeon items share one OoT 24x24 quest icon per type (all pulled from
        // oot.o2r by OTR path). Key rings reuse the small-key icon (OoT has no distinct key-ring quest icon).
        case RI_OOT_SMALL_KEY_BOTTOM_OF_THE_WELL:
        case RI_OOT_SMALL_KEY_FIRE_TEMPLE:
        case RI_OOT_SMALL_KEY_FOREST_TEMPLE:
        case RI_OOT_SMALL_KEY_GANONS_CASTLE:
        case RI_OOT_SMALL_KEY_GERUDO_FORTRESS:
        case RI_OOT_SMALL_KEY_GERUDO_TRAINING_GROUND:
        case RI_OOT_SMALL_KEY_SHADOW_TEMPLE:
        case RI_OOT_SMALL_KEY_SPIRIT_TEMPLE:
        case RI_OOT_SMALL_KEY_TREASURE_GAME:
        case RI_OOT_SMALL_KEY_WATER_TEMPLE:
        case RI_OOT_KEY_RING_BOTTOM_OF_THE_WELL:
        case RI_OOT_KEY_RING_FIRE_TEMPLE:
        case RI_OOT_KEY_RING_FOREST_TEMPLE:
        case RI_OOT_KEY_RING_GANONS_CASTLE:
        case RI_OOT_KEY_RING_GERUDO_FORTRESS:
        case RI_OOT_KEY_RING_GERUDO_TRAINING_GROUND:
        case RI_OOT_KEY_RING_SHADOW_TEMPLE:
        case RI_OOT_KEY_RING_SPIRIT_TEMPLE:
        case RI_OOT_KEY_RING_TREASURE_GAME:
        case RI_OOT_KEY_RING_WATER_TEMPLE:
            return "__OTR__textures/icon_item_24_static/gQuestIconSmallKeyTex";
        case RI_OOT_BOSS_KEY_FIRE_TEMPLE:
        case RI_OOT_BOSS_KEY_FOREST_TEMPLE:
        case RI_OOT_BOSS_KEY_GANONS_CASTLE:
        case RI_OOT_BOSS_KEY_SHADOW_TEMPLE:
        case RI_OOT_BOSS_KEY_SPIRIT_TEMPLE:
        case RI_OOT_BOSS_KEY_WATER_TEMPLE:
            return "__OTR__textures/icon_item_24_static/gQuestIconDungeonBossKeyTex";
        case RI_OOT_MAP_BOTTOM_OF_THE_WELL:
        case RI_OOT_MAP_DEKU_TREE:
        case RI_OOT_MAP_DODONGOS_CAVERN:
        case RI_OOT_MAP_FIRE_TEMPLE:
        case RI_OOT_MAP_FOREST_TEMPLE:
        case RI_OOT_MAP_ICE_CAVERN:
        case RI_OOT_MAP_JABU_JABUS_BELLY:
        case RI_OOT_MAP_SHADOW_TEMPLE:
        case RI_OOT_MAP_SPIRIT_TEMPLE:
        case RI_OOT_MAP_WATER_TEMPLE:
            return "__OTR__textures/icon_item_24_static/gQuestIconDungeonMapTex";
        case RI_OOT_COMPASS_BOTTOM_OF_THE_WELL:
        case RI_OOT_COMPASS_DEKU_TREE:
        case RI_OOT_COMPASS_DODONGOS_CAVERN:
        case RI_OOT_COMPASS_FIRE_TEMPLE:
        case RI_OOT_COMPASS_FOREST_TEMPLE:
        case RI_OOT_COMPASS_ICE_CAVERN:
        case RI_OOT_COMPASS_JABU_JABUS_BELLY:
        case RI_OOT_COMPASS_SHADOW_TEMPLE:
        case RI_OOT_COMPASS_SPIRIT_TEMPLE:
        case RI_OOT_COMPASS_WATER_TEMPLE:
            return "__OTR__textures/icon_item_24_static/gQuestIconDungeonCompassTex";
        // Skijer's NEI — OoT (SoH) medallions/stones use the OoT 24x24 quest icons; warp songs share the OoT
        // ocarina-note icon. All pulled from oot.o2r by OTR path (no MM native icon).
        case RI_OOT_MEDALLION_FIRE:
            return "__OTR__textures/icon_item_24_static/gQuestIconMedallionFireTex";
        case RI_OOT_MEDALLION_FOREST:
            return "__OTR__textures/icon_item_24_static/gQuestIconMedallionForestTex";
        case RI_OOT_MEDALLION_LIGHT:
            return "__OTR__textures/icon_item_24_static/gQuestIconMedallionLightTex";
        case RI_OOT_MEDALLION_SHADOW:
            return "__OTR__textures/icon_item_24_static/gQuestIconMedallionShadowTex";
        case RI_OOT_MEDALLION_SPIRIT:
            return "__OTR__textures/icon_item_24_static/gQuestIconMedallionSpiritTex";
        case RI_OOT_MEDALLION_WATER:
            return "__OTR__textures/icon_item_24_static/gQuestIconMedallionWaterTex";
        case RI_OOT_STONE_GORON_RUBY:
            return "__OTR__textures/icon_item_24_static/gQuestIconGoronRubyTex";
        case RI_OOT_STONE_KOKIRI_EMERALD:
            return "__OTR__textures/icon_item_24_static/gQuestIconKokiriEmeraldTex";
        case RI_OOT_STONE_ZORA_SAPPHIRE:
            return "__OTR__textures/icon_item_24_static/gQuestIconZoraSapphireTex";
        case RI_OOT_SONG_BOLERO_OF_FIRE:
        case RI_OOT_SONG_MINUET_OF_FOREST:
        case RI_OOT_SONG_NOCTURNE_OF_SHADOW:
        case RI_OOT_SONG_PRELUDE_OF_LIGHT:
        case RI_OOT_SONG_REQUIEM_OF_SPIRIT:
        case RI_OOT_SONG_SERENADE_OF_WATER:
        case RI_OOT_SONG_ZELDAS_LULLABY:
        // Skijer's NEI — the 3 NEI custom songs share the same OoT note icon (no icon of their own anywhere).
        case RI_OOT_SONG_BALLAD_OF_THE_HERO:
        case RI_OOT_SONG_COMMAND_MELODY:
        case RI_OOT_SONG_FUGUE_OF_HOME:
            return "__OTR__textures/icon_item_static/gSongNoteTex";
        // Skijer's NEI — OoT vanilla gear/spells/masks: OoT item/quest icons pulled from oot.o2r (symbols
        // only exist there; same pattern as the trade/medallion icons above).
        case RI_OOT_BOOMERANG:
            return "__OTR__textures/icon_item_static/gItemIconBoomerangTex";
        case RI_OOT_DINS_FIRE:
            return "__OTR__textures/icon_item_static/gItemIconDinsFireTex";
        case RI_OOT_FARORES_WIND:
            return "__OTR__textures/icon_item_static/gItemIconFaroresWindTex";
        case RI_OOT_NAYRUS_LOVE:
            return "__OTR__textures/icon_item_static/gItemIconNayrusLoveTex";
        case RI_OOT_IRON_BOOTS:
            return "__OTR__textures/icon_item_static/gItemIconBootsIronTex";
        case RI_OOT_HOVER_BOOTS:
            return "__OTR__textures/icon_item_static/gItemIconBootsHoverTex";
        case RI_OOT_GORON_TUNIC:
            return "__OTR__textures/icon_item_static/gItemIconTunicGoronTex";
        case RI_OOT_ZORA_TUNIC:
            return "__OTR__textures/icon_item_static/gItemIconTunicZoraTex";
        case RI_OOT_DEKU_SHIELD: // OoT's own deku-shield icon (oot.o2r; OoT-unique symbol, same pattern as below)
            return "__OTR__textures/icon_item_static/gItemIconShieldDekuTex";
        case RI_OOT_MIRROR_SHIELD: // OoT's own mirror-shield icon (distinct from MM's RI_SHIELD_MIRROR icon)
            return "__OTR__textures/icon_item_static/gItemIconShieldMirrorTex";
        case RI_OOT_PROGRESSIVE_HAMMER:
            return "__OTR__textures/icon_item_static/gItemIconHammerTex";
        case RI_OOT_PROGRESSIVE_MASTER_SWORD:
            return "__OTR__textures/icon_item_static/gItemIconSwordMasterTex";
        case RI_OOT_MASK_SKULL:
            return "__OTR__textures/icon_item_static/gItemIconMaskSkullTex";
        case RI_OOT_MASK_SPOOKY:
            return "__OTR__textures/icon_item_static/gItemIconMaskSpookyTex";
        case RI_OOT_MASK_GERUDO:
            return "__OTR__textures/icon_item_static/gItemIconMaskGerudoTex";
        case RI_OOT_STONE_OF_AGONY:
            return "__OTR__textures/icon_item_24_static/gQuestIconStoneOfAgonyTex";
        case RI_OOT_GERUDO_MEMBERSHIP_CARD:
            return "__OTR__textures/icon_item_24_static/gQuestIconGerudosCardTex";
        case RI_OOT_SKELETON_KEY: // SoH's skeleton-key icon is custom (soh.o2r only) — reuse the OoT small-key quest icon
            return "__OTR__textures/icon_item_24_static/gQuestIconSmallKeyTex";
        case RI_OOT_BOMBCHU_BAG: // SoH's bombchu-bag icon is custom — MM's native bombchu icon stands in
            return (const char*)gItemIcons[ITEM_BOMBCHU];
        case RI_OOT_FISHING_POLE: // SoH's fishing-pole icon is custom — MM's native fishing-rod icon stands in
            return (const char*)gItemIconFishingRodTex;
        case RI_OOT_GREG: // parity with SoH, which hides Greg behind the Goron Mask icon (MM's own goron mask here)
            return (const char*)gItemIcons[ITEM_MASK_GORON];
        // Third wave (final cross items) — icons:
        case RI_OOT_GS_TOKEN: // OoT's own 24x24 GS quest icon (oot.o2r)
            return "__OTR__textures/icon_item_24_static/gQuestIconGoldSkulltulaTex";
        case RI_OOT_RUTOS_LETTER: // OoT bottle-with-letter icon (oot.o2r)
            return "__OTR__textures/icon_item_static/gItemIconBottleRutosLetterTex";
        // OoT bottled contents — OoT's own bottle icons (oot.o2r; OoT-unique symbols).
        case RI_OOT_BOTTLE_BIG_POE:
            return "__OTR__textures/icon_item_static/gItemIconBottleBigPoeTex";
        case RI_OOT_BOTTLE_BLUE_FIRE:
            return "__OTR__textures/icon_item_static/gItemIconBottleBlueFireTex";
        case RI_OOT_BOTTLE_BLUE_POTION:
            return "__OTR__textures/icon_item_static/gItemIconBottlePotionBlueTex";
        case RI_OOT_BOTTLE_BUGS:
            return "__OTR__textures/icon_item_static/gItemIconBottleBugTex";
        case RI_OOT_BOTTLE_FAIRY:
            return "__OTR__textures/icon_item_static/gItemIconBottleFairyTex";
        case RI_OOT_BOTTLE_FISH:
            return "__OTR__textures/icon_item_static/gItemIconBottleFishTex";
        case RI_OOT_BOTTLE_GREEN_POTION:
            return "__OTR__textures/icon_item_static/gItemIconBottlePotionGreenTex";
        case RI_OOT_BOTTLE_POE:
            return "__OTR__textures/icon_item_static/gItemIconBottlePoeTex";
        case RI_OOT_BOTTLE_MAGIC_MUSHROOM: // SoH's icon is soh.o2r-custom — MM's native mushroom icon stands in
            return (const char*)gItemIcons[ITEM_MUSHROOM];
        // Jabber nuts: SoH has no dedicated icons either — MM's native deku-nut icon stands in for all 6.
        case RI_OOT_SPEAK_DEKU:
        case RI_OOT_SPEAK_GERUDO:
        case RI_OOT_SPEAK_GORON:
        case RI_OOT_SPEAK_HYLIAN:
        case RI_OOT_SPEAK_KOKIRI:
        case RI_OOT_SPEAK_ZORA:
            return (const char*)gItemIcons[ITEM_DEKU_NUT];
        case RI_OOT_ABILITY_CHESTS: // no icon in SoH either; MM's chest-y stand-in is the small key
            return (const char*)gItemIcons[ITEM_KEY_SMALL];
        case RI_OOT_ABILITY_CLIMB: // no icon exists anywhere (SoH included) — MM hookshot icon stands in (climbing aid)
            return (const char*)gItemIcons[ITEM_HOOKSHOT];
        case RI_OOT_ABILITY_CRAWL: // parity with SoH's knee-pads draw (two deku shields) — OoT deku-shield icon
            return "__OTR__textures/icon_item_static/gItemIconShieldDekuTex";
        // Skijer's NEI — NEI page-2 / ext-equipment icons: 2ship carries its OWN copies of these custom
        // textures (mm/assets/custom/textures/icon_item_custom → 2ship.o2r), so they always resolve natively.
        case RI_OOT_NEI_BALL_AND_CHAIN:
            return "__OTR__textures/icon_item_custom/gItemIconBallAndChainTex";
        case RI_OOT_NEI_BEETLE:
            return "__OTR__textures/icon_item_custom/gItemIconBeetleTex";
        case RI_OOT_NEI_BOMB_ARROWS:
            return "__OTR__textures/icon_item_custom/gItemIconBombArrowsTex";
        // All six Dual Cane skills share the cane's icon — they are one item in the
        // inventory, so a distinct icon per skill would misrepresent the slot.
        case RI_OOT_NEI_CANE_OF_SOMARIA:
        case RI_OOT_NEI_CANE_SOMARIA_BLOCK:
        case RI_OOT_NEI_CANE_SOMARIA_PLATFORM:
        case RI_OOT_NEI_CANE_PACCI_FLIP:
        case RI_OOT_NEI_CANE_PACCI_STONE:
        case RI_OOT_NEI_CANE_PACCI_ULTRAHAND:
            return "__OTR__textures/icon_item_custom/gItemIconCaneOfSomariaTex";
        case RI_OOT_NEI_DEKU_LEAF:
            return "__OTR__textures/icon_item_custom/gItemIconDekuLeafTex";
        case RI_OOT_NEI_DEMISE_DESTRUCTION:
            return "__OTR__textures/icon_item_custom/gItemIconDemiseDestructionTex";
        case RI_OOT_NEI_DESIRE_SENSOR:
            return "__OTR__textures/icon_item_custom/gItemIconDesireSensorTex";
        case RI_OOT_NEI_DOMINION_ROD:
            return "__OTR__textures/icon_item_custom/gItemIconDominionRodTex";
        // Elemental Wand: per-ROD icons, so the check tracker and the get-item textbox say which of
        // the six you found even though they all land on one inventory cell.
        case RI_OOT_NEI_ELEMENTAL_WAND:
        case RI_OOT_NEI_WAND_SAND_ROD:
            return "__OTR__textures/icon_item_custom/gItemIconSandRodTex";
        case RI_OOT_NEI_WAND_TORNADO_ROD:
            return "__OTR__textures/icon_item_custom/gItemIconTornadoRodTex";
        case RI_OOT_NEI_WAND_WATER_ROD:
            return "__OTR__textures/icon_item_custom/gItemIconWaterRodTex";
        case RI_OOT_NEI_WAND_METEOR_ROD:
            return "__OTR__textures/icon_item_custom/gItemIconMeteorRodTex";
        case RI_OOT_NEI_WAND_STORM_ROD:
            return "__OTR__textures/icon_item_custom/gItemIconStormRodTex";
        case RI_OOT_NEI_WAND_SHADOW_SCEPTER:
            return "__OTR__textures/icon_item_custom/gItemIconShadowScepterTex";
        case RI_OOT_NEI_FIRE_ROD:
            return "__OTR__textures/icon_item_custom/gItemIconFireRodTex";
        case RI_OOT_NEI_GUST_JAR:
            return "__OTR__textures/icon_item_custom/gItemIconGustJarTex";
        case RI_OOT_NEI_HYLIAS_GRACE:
            return "__OTR__textures/icon_item_custom/gItemIconHyliaGraceTex";
        case RI_OOT_NEI_ICE_ROD:
            return "__OTR__textures/icon_item_custom/gItemIconIceRodTex";
        case RI_OOT_NEI_LANTERN:
            return "__OTR__textures/icon_item_custom/gItemIconLanternTex";
        case RI_OOT_NEI_LIGHT_ROD:
            return "__OTR__textures/icon_item_custom/gItemIconLightRodTex";
        case RI_OOT_NEI_MINISH_CAP:
            return "__OTR__textures/icon_item_custom/gItemIconMinishCapTex";
        case RI_OOT_NEI_MOGMA_MITTS:
            return "__OTR__textures/icon_item_custom/gItemIconMogmaMittsTex";
        case RI_OOT_NEI_POKE_BALL:
            return "__OTR__textures/icon_item_custom/gItemIconPokeballTex";
        case RI_OOT_NEI_SHOVEL:
            return "__OTR__textures/icon_item_custom/gItemIconShovelTex";
        case RI_OOT_NEI_SPINNER:
            return "__OTR__textures/icon_item_custom/gItemIconSpinnerTex";
        case RI_OOT_NEI_SWITCH_HOOK:
            return "__OTR__textures/icon_item_custom/gItemIconSwitchHookTex";
        case RI_OOT_NEI_TIME_GATE:
            return "__OTR__textures/icon_item_custom/gItemIconTimeGateTex";
        case RI_OOT_NEI_WHIP:
            return "__OTR__textures/icon_item_custom/gItemIconWhipTex";
        case RI_OOT_NEI_ZONAI_PERMAFROST:
            return "__OTR__textures/icon_item_custom/gItemIconZonaiPermafrostTex";
        case RI_OOT_PROGRESSIVE_ROC:
            return "__OTR__textures/icon_item_custom/gItemIconRocsFeatherTex";
        case RI_OOT_PROGRESSIVE_STICK_CAPACITY: // MM's own stick/nut icons; no upgrade art exists
            return (const char*)gItemIcons[ITEM_DEKU_STICK];
        case RI_OOT_PROGRESSIVE_NUT_CAPACITY:
            return (const char*)gItemIcons[ITEM_DEKU_NUT];
        case RI_OOT_ROCS_FEATHER:
            // Ship-vanilla art, the same texture extended_inventory.c loads for ITEM_ROCS_FEATHER.
            return "__OTR__textures/icon_item_static/gRocsFeatherTex";
        case RI_OOT_EXT_CANE_OF_BYRNA:
            return "__OTR__textures/icon_item_custom/gItemIconCaneOfByrnaTex";
        case RI_OOT_EXT_CHAMPIONS_TUNIC:
            return "__OTR__textures/icon_item_custom/gItemIconChampionsTunicTex";
        case RI_OOT_EXT_DIVINE_SHIELD: // = the GODDESS SHIELD (renamed 2026-07-29; same item/behavior)
            return "__OTR__textures/icon_item_custom/gItemIconGoddessShieldTex";
        case RI_OOT_EXT_FOUR_SWORD:
            return "__OTR__textures/icon_item_custom/gItemIconFourSwordTex";
        case RI_OOT_EXT_TRIDENT:
            return "__OTR__textures/icon_item_custom/gItemIconTridentTex";
        case RI_OOT_EXT_CLIMB_BOOTS:
            return "__OTR__textures/icon_item_custom/gItemIconClimbBootsTex";
        case RI_OOT_EXT_ROC_BOOTS:
            return "__OTR__textures/icon_item_custom/gItemIconRocBootsTex";
        // 2026-08-06 page-2 additions — own icons (icon_item_custom PNG pipeline).
        case RI_OOT_NEI_SHEIKAH_SLATE:
            return "__OTR__textures/icon_item_custom/gItemIconSheikahSlateTex";
        case RI_OOT_NEI_PHANTOM_HOURGLASS:
            return "__OTR__textures/icon_item_custom/gItemIconPhantomHourglassTex";
        case RI_OOT_NEI_SHADOW_CRYSTAL:
            return "__OTR__textures/icon_item_custom/gItemIconShadowCrystalTex";
        case RI_OOT_NEI_ROD_OF_SEASONS:
            return "__OTR__textures/icon_item_custom/gItemIconRodOfSeasonsTex";
        // Slate runes: the slate composite with the rune's badge (textbox/tracker icon).
        case RI_OOT_NEI_SLATE_RUNE_BOMB:
            return "__OTR__textures/icon_item_custom/gItemIconSheikahSlateBombTex";
        case RI_OOT_NEI_SLATE_RUNE_MASTER_CYCLE:
            return "__OTR__textures/icon_item_custom/gItemIconSheikahSlateMasterCycleTex";
        case RI_OOT_NEI_SLATE_RUNE_STASIS:
            return "__OTR__textures/icon_item_custom/gItemIconSheikahSlateStasisTex";
        case RI_OOT_NEI_SLATE_RUNE_CRYONIS:
            return "__OTR__textures/icon_item_custom/gItemIconSheikahSlateCryonisTex";
        case RI_OOT_EXT_MAGIC_CAPE:
            return "__OTR__textures/icon_item_custom/gItemIconMagicCapeTex";
        case RI_OOT_EXT_PEGASUS_ANKLET: // = the PEGASUS BOOTS (renamed 2026-07-29)
            return "__OTR__textures/icon_item_custom/gItemIconPegasusBootsTex";
        case RI_OOT_EXT_SHEIKAH_SHIELD: // parity with SoH item_list, which uses the Gerudo Scimitar icon as its stand-in
            return "__OTR__textures/icon_item_custom/gItemIconGerudoScimitarTex";
        case RI_OOT_EXT_SPIRIT_BREASTPLATE: // = the MAGIC TUNIC (renamed 2026-07-29; same behavior)
            return "__OTR__textures/icon_item_custom/gItemIconMagicTunicTex";
        case RI_OOT_EXT_WATER_DRAGON_SCALE:
            return "__OTR__textures/icon_item_custom/gItemIconSagesTunicTex";
        default:
            break;
    }

    s16 itemId = Rando::StaticData::Items[randoItemId].itemId;
    if (itemId >= ITEM_RECOVERY_HEART) {
        itemId = D_801CFF94[Rando::StaticData::Items[randoItemId].getItemId];
    }

    return itemId < ITEM_RECOVERY_HEART ? (const char*)gItemIcons[itemId] : nullptr;
}

bool ShouldShowGetItemCutscene(RandoItemId itemId) {
    if (!CVarGetInteger("gEnhancements.Cutscenes.SkipGetItemCutscenes", 0)) {
        return true;
    }

    switch (Rando::StaticData::Items[itemId].randoItemType) {
        case RITYPE_JUNK:
            return CVarGetInteger("gEnhancements.Cutscenes.SkipGetItemCutscenes", 0) < 1;
        case RITYPE_HEALTH:
        case RITYPE_LESSER:
        case RITYPE_STRAY_FAIRY:
        case RITYPE_SKULLTULA_TOKEN:
        case RITYPE_SMALL_KEY:
            return CVarGetInteger("gEnhancements.Cutscenes.SkipGetItemCutscenes", 0) < 2;
        default:
            return CVarGetInteger("gEnhancements.Cutscenes.SkipGetItemCutscenes", 0) < 3;
    }
}

const std::map<RandoItemId, std::vector<std::string>> fakeItemNames = {
    // Major and Mask items only
    { RI_ABILITY_SWIM, { "Skinny Dipping", "Zora Flippers" } },
    { RI_ARROW_FIRE, { "Fire Rod", "Red Candle" } },
    { RI_ARROW_ICE, { "Ice Rod", "Ancient Arrow", "Ice Trap Arrow" } },
    { RI_ARROW_LIGHT, { "Wind Arrow", "Wand of Gamelon", "Shock Arrow", "Silver Arrow" } },
    { RI_BOMB_BAG_20, { "Bombling Bag", "Water Bomb Bag" } },
    { RI_BOTTLE_CHATEAU_ROMANI, { "Bottle of Castle Milk", "Bottle of Roman Cake" } },
    { RI_BOTTLE_EMPTY,
      { "Empty Canteen", "Vial of Winds", "Tingle Bottle", "Magic Bottle", "Glass Bottle", "Bottle with Water" } },
    { RI_BOTTLE_GOLD_DUST, { "Bottle of Parmesan", "Bottle of Old Crust" } },
    { RI_BOTTLE_MILK, { "Bottle of Mystery Milk", "Bottle of Premium Milk" } },
    { RI_BOTTLE_RED_POTION,
      { "Bottle with Red Chu Jelly", "Bottle with Hibiscus Potion", "Bottle with Medicine of Life",
        "Bottle with Heart Potion" } },
    { RI_BOW, { "Fairy Bow", "Bow of Might", "Bow of Light" } },
    { RI_DEED_LAND, { "Land Mortage", "Land Lease", "Brown Toilet Paper" } },
    { RI_DEED_MOUNTAIN, { "Mountain Mortage", "Mountain Lease", "Red Toilet Paper" } },
    { RI_DEED_OCEAN, { "Ocean Mortage", "Ocean Lease", "Blue Toilet Paper" } },
    { RI_DEED_SWAMP, { "Swamp Mortage", "Swamp Lease", "Green Toilet Paper" } },
    { RI_FROG_BLUE, { "Blue Toad", "Blue Dog", "Ocean Frog" } },
    { RI_FROG_CYAN, { "Cyan Toad", "Cyan Dog", "Sky Frog" } },
    { RI_FROG_PINK, { "Pink Toad", "Pink Dog", "Flower Frog" } },
    { RI_FROG_WHITE, { "White Toad", "White Dog", "Snow Frog" } },
    { RI_HOOKSHOT, { "Clawshot", "Switch Hook", "Grappling Hook", "Longshot" } },
    { RI_LENS, { "Sheikah-leidoscope", "Sheikah Sensor", "Crystal of Vision", "Magnifying Lens" } },
    { RI_LETTER_TO_KAFEI, { "Better to Coffee", "Note to Cafe", "Mail to Coffin" } },
    { RI_LETTER_TO_MAMA, { "Letter to Joe", "Note to Madame Aroma" } },
    { RI_MASK_ALL_NIGHT, { "Caffeine Mask", "Programmer's Mask" } },
    { RI_MASK_BLAST, { "Bomb Mask", "Powder Keg Mask" } },
    { RI_MASK_BREMEN, { "Chicken Mask", "Revali's Mask" } },
    { RI_MASK_BUNNY, { "Rabbit Hood", "Bunny Mask" } },
    { RI_MASK_CAPTAIN, { "Keeta's Mask", "Stalchild Mask" } },
    { RI_MASK_CIRCUS_LEADER, { "Troupe Leader's Mask", "Gorman's Mask" } },
    { RI_MASK_COUPLE, { "Divorce Mask", "Sun and Moon Mask" } },
    { RI_MASK_DEKU, { "Deku Butler's Son's Mask", "Scrub Mask" } },
    { RI_MASK_DON_GERO, { "Dr. Gero Mask", "Frog's Hat" } },
    { RI_MASK_FIERCE_DEITY, { "Adult Link Mask", "Tall Boy Mask" } },
    { RI_MASK_GARO, { "Ninja Mask", "Ikana Hood" } },
    { RI_MASK_GIANT, { "Tiny Mask", "Wrestling Mask" } },
    { RI_MASK_GIBDO, { "ReDead Mask", "Mummy Mask" } },
    { RI_MASK_GORON, { "Darmani's Mask", "Darunia's Mask" } },
    { RI_MASK_GREAT_FAIRY, { "Stray Fairy Mask", "Grate Faerie Mask" } },
    { RI_MASK_KAFEIS_MASK, { "Coffee's Mask", "Anju's Mask" } },
    { RI_MASK_KAMARO, { "Mask of Dance", "Surgery Mask" } },
    { RI_MASK_KEATON, { "Korok Mask", "Lynel Mask", "Cucco Mask", "Remlit Mask" } },
    { RI_MASK_POSTMAN, { "Il Piantissimo's Hat", "Running Man's Hat" } },
    { RI_MASK_ROMANI, { "Cremia's Mask", "Cow Mask", "Milk Bar Mask" } },
    { RI_MASK_SCENTS, { "Pig Mask", "Sniffa Mask", "Truffle Mask" } },
    { RI_MASK_STONE, { "Rock Mask", "Stealth Mask", "Shiro's Mask" } },
    { RI_MASK_TRUTH, { "Majora's Mask", "Hero's Charm", "Dog Mask" } },
    { RI_MASK_ZORA, { "Mikau's Mask", "Mweep Mask" } },
    { RI_MOONS_TEAR, { "Crystallized Fear", "Lunar Cry", "Astral Tear" } },
    { RI_MUSHROOM, { "Odd Mushroom", "Endura Shroom", "Sleepy Toadstool" } },
    { RI_OCARINA_BUTTON_A, { "J Button", "Ayy Button", "A Trigger" } },
    { RI_OCARINA_BUTTON_C_DOWN, { "C South Button", "Z Down Button", "See Down Button", "C Dawn Button" } },
    { RI_OCARINA_BUTTON_C_RIGHT, { "C East Button", "C Wright Button", "Play Button" } },
    { RI_OCARINA_BUTTON_C_LEFT, { "C West Button", "Sea Left Button", "C Lift Button", "Rewind Button" } },
    { RI_OCARINA_BUTTON_C_UP, { "C North Button", "C App Button", "Sup Button" } },
    { RI_OCARINA, { "Fairy Ocarina", "Flute", "Majora's Ocarina" } },
    { RI_PENDANT_OF_MEMORIES, { "Necklace of Memories", "Forget-Me-Not" } },
    { RI_PICTOGRAPH_BOX, { "Camera", "Gallery Device" } },
    { RI_POWDER_KEG, { "Giant Bomb", "Powder Barrel" } },
    { RI_PROGRESSIVE_BOMB_BAG,
      { "Progressive Bomb Capacity", "Progressive Bomb Pack", "Progressive Bomb Box", "Progressive Blast Mask",
        "Progressive Powder Kegs", "Progressive Remote Bombs" } },
    { RI_PROGRESSIVE_BOW,
      { "Progressive Arrow Capacity", "Progressive Fairy Bow", "Progressive Arrow Holder", "Progressive Crossbow",
        "Progressive Sacred Bow", "Progressive Lynel Bow" } },
    { RI_PROGRESSIVE_LULLABY, { "Progressive Zelda's Lullaby", "Progressive Goron Sleep Song" } },
    { RI_PROGRESSIVE_MAGIC, { "Progressive Stamina Meter", "Progressive Energy Gauge", "Progressive Magic Powder" } },
    { RI_PROGRESSIVE_SWORD, { "Progressive Slice and Dice", "Progressive Cutter", "Progressive Stabby" } },
    { RI_PROGRESSIVE_WALLET,
      { "Progressive Rupee Capacity", "Progressive Purse", "Progressive Rupee Bag", "Progressive Rupoor Capacity",
        "Progressive Spoils Bag", "Progressive Ruby Bag" } },
    { RI_REMAINS_GOHT, { "Goat's Remains", "Goht's Retainer" } },
    { RI_REMAINS_GYORG, { "George's Remains", "Fishy Remains" } },
    { RI_REMAINS_ODOLWA, { "Jungle Warrior's Remains", "Odolwa's Reprimands" } },
    { RI_REMAINS_TWINMOLD, { "Moldorm's Remains", "Phantom Ganon's Remains" } },
    { RI_ROOM_KEY, { "Key to the City", "Medium Key" } },
    { RI_SHIELD_HERO, { "Deku Shield", "Hylian Shield", "Goddess Shield" } },
    { RI_SHIELD_MIRROR, { "Magic Mirror", "Magical Shield", "Mirror of Twilight" } },
    { RI_SINGLE_MAGIC, { "Magic Powder", "Green Energy" } },
    { RI_SONG_DOUBLE_TIME, { "Song of Single Time", "Fast Forward Song" } },
    { RI_SONG_ELEGY, { "Eulogy of Emptiness", "A Little Elegy", "Benjamin of Emptiness", "Requiem of Spirit" } },
    { RI_SONG_EPONA, { "Song of Birds", "Song of Soaring", "Song of Horse" } },
    { RI_SONG_HEALING, { "Inverted Saria's Song", "Song of Hurting", "Song of Feel Good" } },
    { RI_SONG_INVERTED_TIME, { "Song of Untime", "Inverted Tune of Ages" } },
    { RI_SONG_LULLABY_INTRO, { "Zelda's Lullaby Intro", "Rock-a-bye Baby Intro", "Bolero of Fire Intro" } },
    { RI_SONG_LULLABY, { "Zelda's Lullaby", "Rock-a-bye Baby", "Bolero of Fire" } },
    { RI_SONG_NOVA, { "Soul Bossa Nova", "New World Order", "Serenade of Water" } },
    { RI_SONG_OATH, { "Oats to Order", "Law and Order", "Nocturne of Shadow" } },
    { RI_SONG_SARIA, { "Inverted Song of Healing", "Tune of Echoes" } },
    { RI_SONG_SOARING, { "Prelude of Light", "Wind's Song", "Owl's Song", "Ballad of Gales" } },
    { RI_SONG_SONATA, { "Minuet of Forest", "Deku's Anti-Lullaby" } },
    { RI_SONG_STORMS, { "Frog's Song of Soul", "Wind's Requiem", "Windmill Song" } },
    { RI_SONG_SUN, { "Song of Passing", "Command Melody", "Moon's Song" } },
    { RI_SONG_TIME, { "Tune of Ages", "Inverted Sun's Song", "Groundhog's Song" } },
    { RI_SOUL_BOSS_GOHT, { "Soul of Goat", "Soul of Goth" } },
    { RI_SOUL_BOSS_GYORG, { "Soul of George", "Soul of Windfish" } },
    { RI_SOUL_BOSS_MAJORA, { "Soul of Minora", "Soul of Vaati" } },
    { RI_SOUL_BOSS_ODOLWA, { "Soul of Jungle Warrior", "Soul of Doll" } },
    { RI_SOUL_BOSS_TWINMOLD, { "Soul of Moldorm", "Soul of Twins" } },
    { RI_SWORD_KOKIRI, { "Master Sword", "Phantom Sword" } },
    { RI_TIME_DAY_1, { "Thyme (Day 4)", "Broken Clock (Day 7)", "Circle Toy (Day 0)" } },
    { RI_TIME_DAY_2, { "Thyme (Day 5)", "Broken Clock (Day 8)", "Circle Toy (Day 22)" } },
    { RI_TIME_DAY_3, { "Thyme (Day 6)", "Broken Clock (Day 9)", "Circle Toy (Day 13)" } },
    { RI_TIME_NIGHT_1, { "Thyme (Night 4)", "Broken Clock (Day 7)", "Circle Toy (Night 0)" } },
    { RI_TIME_NIGHT_2, { "Thyme (Night 5)", "Broken Clock (Day 8)", "Circle Toy (Night 22)" } },
    { RI_TIME_NIGHT_3, { "Thyme (Night 6)", "Broken Clock (Day 9)", "Circle Toy (Night 13)" } },
    { RI_TIME_PROGRESSIVE, { "Progressive Thyme", "Progressive Lime", "Progressive Broken Clock" } },
    { RI_TRIFORCE_PIECE_PREVIOUS, { "Piece of Cheese", "Shiny Rock", "Jiggy" } },
    { RI_TRIFORCE_PIECE, { "Piece of Cheese", "Shiny Rock", "Jiggy" } },
    { RI_WALLET_ADULT, { "Silver Wallet", "Medium Wallet" } },
};

std::string GetItemName(RandoItemId randoItemId, bool includeArticle, RandoCheckId randoCheckId) {
    std::string result;

    if (includeArticle && !Ship_IsCStringEmpty(Rando::StaticData::Items[randoItemId].article)) {
        result += Rando::StaticData::Items[randoItemId].article;
        result += " ";
    }

    result += Rando::StaticData::Items[randoItemId].name;

    if (randoItemId == RI_JUNK && (randoCheckId == RC_UNKNOWN ||
                                   (Rando::StaticData::Checks[randoCheckId].randoCheckType != RCTYPE_SHOP &&
                                    Rando::StaticData::Checks[randoCheckId].randoCheckType != RCTYPE_TINGLE_SHOP))) {
        result += std::string(" (") + Rando::StaticData::Items[Rando::CurrentJunkItem(randoCheckId)].name + ")";
    }

    if (randoItemId == RI_TRAP && randoCheckId != RC_UNKNOWN) {
        // Get the name of the trapped item
        RandoItemId trappedItemId = Rando::CurrentTrapItem(randoCheckId);
        std::string fakeItemName;
        auto fakeNames = fakeItemNames.find(trappedItemId);
        if (fakeNames != fakeItemNames.end()) {
            fakeItemName = fakeNames->second[Ship_Random(0, fakeNames->second.size())];
        } else {
            // Fallback: Double a random letter to fool the player
            auto letterIndex = Ship_Random(0, fakeItemName.length());
            char letterToDouble = fakeItemName[letterIndex];
            // But not spaces
            if (letterToDouble == ' ') {
                letterIndex++;
                letterToDouble = fakeItemName[letterIndex];
            }
            fakeItemName.insert(letterIndex, 1, letterToDouble);
        }

        result.clear();

        if (includeArticle && !Ship_IsCStringEmpty(Rando::StaticData::Items[randoItemId].article)) {
            result += Rando::StaticData::Items[randoItemId].article;
            result += " ";
        }

        result += fakeItemName;
    }

    return result;
}

} // namespace StaticData

} // namespace Rando
