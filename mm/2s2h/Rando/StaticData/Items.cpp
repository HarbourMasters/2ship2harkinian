#include "StaticData.h"
#include "public/bridge/consolevariablebridge.h"
#include "2s2h/ShipUtils.h"
#include "2s2h/Rando/Rando.h"

extern "C" {
extern s16 D_801CFF94[250];
#include "assets/interface/parameter_static/parameter_static.h"
}

namespace Rando {

namespace StaticData {

#define RI(id, articleEng, nameEng, articleFre, nameFre, articleGer, articleGer2, nameGer, articleJpn, nameJpn,       \
           articleSpa, nameSpa, type, itemId, getItemId, drawId)                                                      \
    {                                                                                                                 \
        id, {                                                                                                         \
            id, #id, articleEng, nameEng, articleFre, nameFre, articleGer, articleGer2, nameGer, articleJpn, nameJpn, \
                articleSpa, nameSpa, type, itemId, getItemId, drawId                                                  \
        }                                                                                                             \
    }

// clang-format off
std::map<RandoItemId, RandoStaticItem> Items = {
    RI(RI_UNKNOWN,                    "",     "Unknown",                     "",       "Inconnu",                                        "",      "",       "Unbekannt",                                "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",       RITYPE_JUNK,            ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_ARROW_FIRE,                 "",     "Fire Arrows",                 "la",     "Flèche de Feu",                                  "der",   "den",    "Feuerpfeil",                               "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",        RITYPE_MAJOR,           ITEM_ARROW_FIRE,                 GI_ARROW_FIRE,               GID_ARROW_FIRE),
    RI(RI_ARROW_ICE,                  "",     "Ice Arrows",                  "la",     "Flèche de Glace",                                "der",   "den",    "Eispfeil",                                 "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",        RITYPE_MAJOR,           ITEM_ARROW_ICE,                  GI_ARROW_ICE,                GID_ARROW_ICE),
    RI(RI_ARROW_LIGHT,                "",     "Light Arrows",                "la",     "Flèche de Lumière",                              "der",   "den",    "Lichtpfeil",                               "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",    RITYPE_MAJOR,           ITEM_ARROW_LIGHT,                GI_ARROW_LIGHT,              GID_ARROW_LIGHT),
    RI(RI_ARROWS_10,                  "",     "10 Arrows",                   "",       "10 Flèches",                                     "",      "",       "10 Pfeile",                                "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",    RITYPE_JUNK,            ITEM_ARROWS_10,                  GI_ARROWS_10,                GID_ARROWS_SMALL),
    RI(RI_ARROWS_30,                  "",     "30 Arrows",                   "",       "30 Flèches",                                     "",      "",       "30 Pfeile",                                "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",    RITYPE_JUNK,            ITEM_ARROWS_30,                  GI_ARROWS_30,                GID_ARROWS_MEDIUM),
    RI(RI_ARROWS_50,                  "",     "50 Arrows",                   "",       "50 Flèches",                                     "",      "",       "50 Pfeile",                                "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",    RITYPE_JUNK,            ITEM_ARROWS_50,                  GI_ARROWS_50,                GID_ARROWS_LARGE),
    RI(RI_BLUE_POTION_REFILL,         "a",    "Blue Potion Refill",          "une",    "Recharge de Potion Bleue",                       "ein",   "ein",    "Blaues Elixier",                           "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",       RITYPE_JUNK,            ITEM_POTION_BLUE,                GI_POTION_BLUE,              GID_POTION_BLUE),
    RI(RI_BOMB_BAG_20,                "a",    "Bomb Bag",                    "un ",    "Sac de Bombes",                                  "eine",  "eine",    "Bombentasche",                             "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",         RITYPE_MAJOR,           ITEM_BOMB_BAG_20,                GI_BOMB_BAG_20,              GID_BOMB_BAG_20),
    RI(RI_BOMB_BAG_30,                "a",    "Big Bomb Bag",                "la",     "Grand Sac de Bombes",                            "eine",  "eine",   "Große Bombentasche",                       "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",            RITYPE_LESSER,          ITEM_BOMB_BAG_30,                GI_BOMB_BAG_30,              GID_BOMB_BAG_30),
    RI(RI_BOMB_BAG_40,                "the",  "Biggest Bomb Bag",            "la",     "Sac de Bombes Géant",                            "eine",  "eine",   "Riesen-Bombentasche",                      "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",     RITYPE_LESSER,          ITEM_BOMB_BAG_40,                GI_BOMB_BAG_40,              GID_BOMB_BAG_40),
    RI(RI_BOMBCHU_10,                 "",     "10 Bombchus",                 "",       "10 Missiles Teigneux",                           "",      "",       "10 Krabbelminen",                          "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",   RITYPE_JUNK,            ITEM_BOMBCHUS_10,                GI_BOMBCHUS_10,              GID_BOMBCHU),
    RI(RI_BOMBCHU_5,                  "",     "5 Bombchus",                  "",       "5 Missiles Teigneux",                            "",      "",       "5 Krabbelminen",                           "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",   RITYPE_JUNK,            ITEM_BOMBCHUS_5,                 GI_BOMBCHUS_5,               GID_BOMBCHU),
    RI(RI_BOMBCHU,                    "a",    "Bombchu",                     "un",     "Missile Teigneux",                               "eine",  "eine",   "Krabbelmine",                              "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",    RITYPE_JUNK,            ITEM_BOMBCHUS_1,                 GI_BOMBCHUS_1,               GID_BOMBCHU), // not sure about this
    RI(RI_BOMBERS_NOTEBOOK,           "the",  "Bomber's Notebook",           "le",     "Journal des Bombers",                            "das",   "das",    "Notizbuch der Bomber",                     "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",        RITYPE_LESSER,          ITEM_BOMBERS_NOTEBOOK,           GI_BOMBERS_NOTEBOOK,         GID_BOMBERS_NOTEBOOK),
    RI(RI_BOMBS_10,                   "",     "10 Bombs",                    "",       "10 Bombes",                                      "",      "",       "10 Bomben",                                "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",    RITYPE_JUNK,            ITEM_BOMBS_10,                   GI_BOMBS_10,                 GID_BOMB),
    RI(RI_BOMBS_5,                    "",     "5 Bombs",                     "",       "5 Bombes",                                       "",      "",       "5 Bomben",                                 "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",    RITYPE_JUNK,            ITEM_BOMBS_5,                    GI_BOMBS_5,                  GID_BOMB),
    RI(RI_BOTTLE_CHATEAU_ROMANI,      "a",    "Bottle of Chateau Romani",    "une",    "Bouteille de Cuvée Romani",                      "eine",  "eine",   "Flasche (Chateau Romani)",                 "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",    RITYPE_MAJOR,           ITEM_CHATEAU,                    GI_CHATEAU,                  GID_CHATEAU),
    RI(RI_BOTTLE_EMPTY,               "an",   "Empty Bottle",                "une",    "Bouteille Vide",                                 "eine",  "eine",   "Leere Flasche",                            "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",    RITYPE_MAJOR,           ITEM_BOTTLE,                     GI_BOTTLE,                   GID_BOTTLE),
    RI(RI_BOTTLE_GOLD_DUST,           "a",    "Bottle With Gold Dust",       "une",    "Bouteille avec de la Poudre d'Or",               "eine",  "eine",   "Flasche (Goldstaub)",                      "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",     RITYPE_MAJOR,           ITEM_GOLD_DUST,                  GI_GOLD_DUST,                GID_SEAHORSE), // bottle of gold dust
    RI(RI_BOTTLE_MILK,                "a",    "Bottle of Milk",              "une",    "Bouteille de Lait",                              "eine",  "eine",   "Flasche (Milch)",                          "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",     RITYPE_MAJOR,           ITEM_MILK_BOTTLE,                GI_MILK_BOTTLE,              GID_MILK),
    RI(RI_BOTTLE_RED_POTION,          "a",    "Bottle with Red Potion",      "une",    "Bouteille avec de la Potion Rouge",              "eine",  "eine",   "Flasche (Rotes Elixier)",                  "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",   RITYPE_MAJOR,           ITEM_POTION_RED,                 GI_POTION_RED_BOTTLE,        GID_57), // bottle of red potion
    RI(RI_BOW,                        "a",    "Hero's Bow",                  "l'",     "Arc du Brâve",                                   "der",   "den",    "Heroenbogen",                              "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",              RITYPE_MAJOR,           ITEM_BOW,                        GI_QUIVER_30,                GID_BOW),
    RI(RI_CHATEAU_ROMANI_REFILL,      "a",    "Chateau Romani Refill",       "une",    "Recharge de Cuvée Romani",                       "",      "",       "Chateau Romani",                           "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",       RITYPE_JUNK,            ITEM_CHATEAU_2,                  GI_CHATEAU,                  GID_CHATEAU),
    RI(RI_CLOCK_TOWN_STRAY_FAIRY,     "a",    "Clock Town Stray Fairy",      "une",    "Fée Perdue du Bourg Clocher",                    "eine",  "eine",   "Verirrte Fee (Unruh-Stadt)",               "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",            RITYPE_STRAY_FAIRY,     ITEM_STRAY_FAIRIES,              GI_STRAY_FAIRY,              GID_NONE),
    RI(RI_DEED_LAND,                  "the",  "Land Title Deed",             "le",     "Titre de Terre pour la Villa",                   "eine",  "eine",   "Land-Urkunde",                             "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",            RITYPE_MAJOR,           ITEM_DEED_LAND,                  GI_DEED_LAND,                GID_DEED_LAND),
    RI(RI_DEED_MOUNTAIN,              "the",  "Mountain Title Deed",         "le",     "Titre de Terre",                                 "eine",  "eine",   "Berg-Urkunde",                             "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",        RITYPE_MAJOR,           ITEM_DEED_MOUNTAIN,              GI_DEED_MOUNTAIN,            GID_DEED_MOUNTAIN),
    RI(RI_DEED_OCEAN,                 "the",  "Ocean Title Deed",            "le",     "Titre d'Océan",                                  "eine",  "eine",   "Meeres-Urkunde",                           "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",           RITYPE_MAJOR,           ITEM_DEED_OCEAN,                 GI_DEED_OCEAN,               GID_DEED_OCEAN),
    RI(RI_DEED_SWAMP,                 "the",  "Swamp Title Deed",            "le",     "Titre du Marais",                                "eine",  "eine",   "Sumpf-Urkunde",                            "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",           RITYPE_MAJOR,           ITEM_DEED_SWAMP,                 GI_DEED_SWAMP,               GID_DEED_SWAMP),
    RI(RI_DEKU_NUT,                   "a",    "Deku Nut",                    "une",    "Noix Mojo",                                      "eine",  "eine",   "Deku-Nuss",                                "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",                   RITYPE_JUNK,            ITEM_DEKU_NUT,                   GI_DEKU_NUTS_1,              GID_DEKU_NUTS),
    RI(RI_DEKU_NUTS_10,               "",     "10 Deku Nuts",                "",       "10 Noix Mojo",                                   "",      "",       "10 Deku-Nüsse",                            "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",               RITYPE_JUNK,            ITEM_DEKU_NUTS_10,               GI_DEKU_NUTS_10,             GID_DEKU_NUTS),
    RI(RI_DEKU_NUTS_5,                "",     "5 Deku Nuts",                 "",       "5 Noix Mojo",                                    "",      "",       "5 Deku-Nüsse",                             "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",                RITYPE_JUNK,            ITEM_DEKU_NUTS_5,                GI_DEKU_NUTS_5,              GID_DEKU_NUTS),
    RI(RI_DEKU_STICK,                 "a",    "Deku Stick",                  "un",     "Bâton Mojo",                                     "ein",   "einen",  "Deku-Stab",                                "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",                 RITYPE_JUNK,            ITEM_DEKU_STICK,                 GI_DEKU_STICKS_1,            GID_DEKU_STICK),
    RI(RI_DEKU_STICKS_5,              "",     "5 Deku Sticks",               "",       "5 Bâtons Mojo",                                  "",      "",       "5 Deku-Stäbe",                             "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",              RITYPE_JUNK,            ITEM_DEKU_STICKS_5,              GI_NONE,                     GID_DEKU_STICK),
    RI(RI_DOUBLE_DEFENSE,             "",     "Double Defense",              "la",     "Double Défense",                                 "die",   "die",    "Doppelte Verteidigung",                    "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",             RITYPE_HEALTH,          ITEM_NONE,                       GI_NONE,                     GID_HEART_CONTAINER),
    RI(RI_DOUBLE_MAGIC,               "a",    "Magic Upgrade",               "une",    "Amélioration de Magie",                          "die",   "die",    "Verb. Magische Kraft",                     "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",              RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_MAGIC_JAR_BIG),
    RI(RI_FAIRY_REFILL,               "a",    "Fairy",                       "une",    "Fée",                                            "eine",  "eine",   "Fee",                                      "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",                      RITYPE_JUNK,            ITEM_FAIRY,                      GI_FAIRY,                    GID_FAIRY_2),
    RI(RI_FROG_BLUE,                  "a",    "Blue Frog",                   "une",    "Grenouille Bleue",                               "ein",   "einen",   "Blauer Frosch",                           "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",             RITYPE_MAJOR,           ITEM_NONE,                       GI_MASK_DON_GERO,            GID_NONE),
    RI(RI_FROG_CYAN,                  "a",    "Cyan Frog",                   "une",    "Grenouille Bleu Ciel",                           "ein",   "einen",   "Hellblauer Frosch",                       "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",            RITYPE_MAJOR,           ITEM_NONE,                       GI_MASK_DON_GERO,            GID_NONE),
    RI(RI_FROG_PINK,                  "a",    "Pink Frog",                   "une",    "Grenouille Rose",                                "einen", "einen",   "Pinken Frosch",                           "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",           RITYPE_MAJOR,           ITEM_NONE,                       GI_MASK_DON_GERO,            GID_NONE),
    RI(RI_FROG_WHITE,                 "a",    "White Frog",                  "une",    "Grenouille Blanche",                             "ein",   "einen",   "Weißer Frosch",                           "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",           RITYPE_MAJOR,           ITEM_NONE,                       GI_MASK_DON_GERO,            GID_NONE),
    RI(RI_GOLD_DUST_REFILL,           "a",    "Gold Dust Refill",            "une",    "Recharge de Poudre d'Or",                        "",      "",       "Goldstaub",                                "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",           RITYPE_LESSER,          ITEM_GOLD_DUST_2,                GI_GOLD_DUST_2,              GID_GOLD_DUST),
    RI(RI_GREAT_BAY_BOSS_KEY,         "the",  "Great Bay Boss Key",          "la",     "Clé d'Or du Temple de la Grande Baie",           "der",   "den",    "Master-Schlüssel (Schädelbucht Tempel)",   "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",         RITYPE_BOSS_KEY,        ITEM_KEY_BOSS,                   GI_KEY_BOSS,                 GID_KEY_BOSS),
    RI(RI_GREAT_BAY_COMPASS,          "the",  "Great Bay Compass",           "la",     "Boussole du Temple de la Grande Baie",           "der",   "den",    "Kompass (Schädelbucht Tempel)",            "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",          RITYPE_LESSER,          ITEM_COMPASS,                    GI_COMPASS,                  GID_COMPASS),
    RI(RI_GREAT_BAY_MAP,              "the",  "Great Bay Map",               "la",     "Carte du Temple de la Grande Baie",              "die",   "die",    "Karte (Schädelbucht Tempel)",              "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",              RITYPE_LESSER,          ITEM_DUNGEON_MAP,                GI_MAP,                      GID_DUNGEON_MAP),
    RI(RI_GREAT_BAY_SMALL_KEY,        "a",    "Great Bay Small Key",         "une",    "Petite Clé du Temple de la Grande Baie",         "ein",   "einen",  "Kleiner Schlüssel (Schädelbucht Tempel)",  "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",        RITYPE_SMALL_KEY,       ITEM_KEY_SMALL,                  GI_KEY_SMALL,                GID_KEY_SMALL),
    RI(RI_GREAT_BAY_STRAY_FAIRY,      "a",    "Great Bay Stray Fairy",       "une",    "Fée Perdue du Temple de la Grande Baie",         "eine",  "eine",   "Verirrte Fee (Schädelbucht Tempel)",       "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",      RITYPE_STRAY_FAIRY,     ITEM_STRAY_FAIRIES,              GI_STRAY_FAIRY,              GID_NONE),
    RI(RI_GREAT_FAIRY_SWORD,          "the",  "Great Fairy's Sword",         "la",     "Grande Epée des Fées",                           "das",   "das",    "Feenschwert",                              "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",        RITYPE_LESSER,          ITEM_SWORD_GREAT_FAIRY,          GI_SWORD_GREAT_FAIRY,        GID_SWORD_GREAT_FAIRY),
    RI(RI_GREAT_SPIN_ATTACK,          "the",  "Great Spin Attack",           "l'",     "Attaque Cyclone Maîtrisée",                      "die",   "die",    "Wirbelattacke",                            "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",          RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_SWORD_KOKIRI),
    RI(RI_GREEN_POTION_REFILL,        "a",    "Green Potion Refill",         "une",    "Recharge de Potion Verte",                       "ein",   "ein",    "Grünes Elixier",                           "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",        RITYPE_JUNK,            ITEM_POTION_GREEN,               GI_POTION_GREEN,             GID_POTION_GREEN),
    RI(RI_GS_TOKEN_OCEAN,             "an",   "Ocean Gold Skulltula Token",  "un",     "Symbole de Skulltula d'Or de la Grande Baie",    "ein",   "ein",    "Skulltula-Symbol (Meer)",                  "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME", RITYPE_SKULLTULA_TOKEN, ITEM_SKULL_TOKEN,                GI_SKULL_TOKEN,              GID_SKULL_TOKEN_2),
    RI(RI_GS_TOKEN_SWAMP,             "a",    "Swamp Gold Skulltula Token",  "un",     "Symbole de Skulltula d'Or du Marais",            "ein",   "ein",    "Skulltula-Symbol (Sümpfe)",                "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME", RITYPE_SKULLTULA_TOKEN, ITEM_SKULL_TOKEN,                GI_SKULL_TOKEN,              GID_SKULL_TOKEN_2),
    RI(RI_HEART_CONTAINER,            "a",    "Heart Container",             "un",     "Réceptacle de Coeur",                            "ein",   "einen",  "Herzcontainer",                            "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",            RITYPE_HEALTH,          ITEM_HEART_CONTAINER,            GI_HEART_CONTAINER,          GID_HEART_CONTAINER),
    RI(RI_HEART_PIECE,                "a",    "Heart Piece",                 "un",     "Quart de Coeur",                                 "ein",   "ein",    "Herzteil",                                 "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",                RITYPE_HEALTH,          ITEM_HEART_PIECE,                GI_HEART_PIECE,              GID_HEART_PIECE),
    RI(RI_HOOKSHOT,                   "the",  "Hookshot",                    "le",     "Grappin",                                        "der",   "den",    "Fanghaken",                                "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",                   RITYPE_MAJOR,           ITEM_HOOKSHOT,                   GI_HOOKSHOT,                 GID_HOOKSHOT),
    RI(RI_JUNK,                       "",     "Junk",                        "de la",  "Camelote",                                       "",      "",       "Plunder",                                  "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",                       RITYPE_JUNK,            ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_LENS,                       "the",  "Lens of Truth",               "le",     "Monocle de Vérité",                              "das",   "das",    "Auge der Wahrheit",                        "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",              RITYPE_MAJOR,           ITEM_LENS_OF_TRUTH,              GI_LENS_OF_TRUTH,            GID_LENS),
    RI(RI_LETTER_TO_KAFEI,            "the",  "Letter to Kafei",             "la",     "Lettre pour Kafei",                              "der",   "den",    "Brief an Kafei",                           "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",            RITYPE_MAJOR,           ITEM_LETTER_TO_KAFEI,            GI_LETTER_TO_KAFEI,          GID_LETTER_TO_KAFEI),
    RI(RI_LETTER_TO_MAMA,             "the",  "Letter to Mama",              "la",     "Lettre Express pour Maman",                      "die",   "die",    "Eilpost an Mama",                          "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",             RITYPE_MAJOR,           ITEM_LETTER_MAMA,                GI_LETTER_TO_MAMA,           GID_LETTER_MAMA),
    RI(RI_MAGIC_BEAN,                 "a",    "Magic Bean",                  "un",     "Haricot Magique",                                "eine",  "eine",   "Wundererbse",                              "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",                 RITYPE_LESSER,          ITEM_MAGIC_BEANS,                GI_MAGIC_BEANS,              GID_MAGIC_BEANS),
    RI(RI_MAGIC_JAR_BIG,              "a",    "Large Magic Refill",          "une",    "Grande Recharge de Magie",                       "eine",  "eine",   "Große Magieflasche",                       "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",         RITYPE_JUNK,            ITEM_MAGIC_JAR_BIG,              GI_MAGIC_JAR_BIG,            GID_MAGIC_JAR_BIG),
    RI(RI_MAGIC_JAR_SMALL,            "a",    "Small Magic Refill",          "une",    "Petite Recharge de Magie",                       "eine",  "eine",   "Kleine Magieflasche",                      "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",         RITYPE_JUNK,            ITEM_MAGIC_JAR_SMALL,            GI_MAGIC_JAR_SMALL,          GID_MAGIC_JAR_SMALL),
    RI(RI_MASK_ALL_NIGHT,             "the",  "All-Night Mask",              "le",     "Masque de la Nuit Blanche",                      "die",   "die",    "Maske der Nacht",                          "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",             RITYPE_MASK,            ITEM_MASK_ALL_NIGHT,             GI_MASK_ALL_NIGHT,           GID_MASK_ALL_NIGHT),
    RI(RI_MASK_BLAST,                 "the",  "Blast Mask",                  "le",     "Masque d'Explosion",                             "die",   "die",    "Ka-Bumm-Maske",                            "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",                 RITYPE_MASK,            ITEM_MASK_BLAST,                 GI_MASK_BLAST,               GID_MASK_BLAST),
    RI(RI_MASK_BREMEN,                "the",  "Bremen Mask",                 "le",     "Masque de Brême",                                "die",   "die",    "Bremer Maske",                             "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",                RITYPE_MASK,            ITEM_MASK_BREMEN,                GI_MASK_BREMEN,              GID_MASK_BREMEN),
    RI(RI_MASK_BUNNY,                 "the",  "Bunny Hood",                  "le",     "Masque du Lapin",                                "die",   "die",    "Hasenohren",                               "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",                 RITYPE_MASK,            ITEM_MASK_BUNNY,                 GI_MASK_BUNNY,               GID_MASK_BUNNY),
    RI(RI_MASK_CAPTAIN,               "the",  "Captain's Hat",               "le",     "Heaume du Capitaine",                            "der",   "den",    "Helm des Hauptmanns",                      "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",              RITYPE_MASK,            ITEM_MASK_CAPTAIN,               GI_MASK_CAPTAIN,             GID_MASK_CAPTAIN),
    RI(RI_MASK_CIRCUS_LEADER,         "the",  "Circus Leader's Mask",        "le",     "Masque du Directeur de Cirque",                  "die",   "die",    "Maske des Zirkusdirektors",                "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",       RITYPE_MASK,            ITEM_MASK_CIRCUS_LEADER,         GI_MASK_CIRCUS_LEADER,       GID_MASK_CIRCUS_LEADER),
    RI(RI_MASK_COUPLE,                "the",  "Couples Mask",               "le",     "Masque des Amoureux",                            "die",   "die",    "Maske der Liebenden",                      "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",               RITYPE_MASK,            ITEM_MASK_COUPLE,                GI_MASK_COUPLE,              GID_MASK_COUPLE),
    RI(RI_MASK_DEKU,                  "the",  "Deku Mask",                   "le",     "Masque Mojo",                                    "die",   "die",    "Deku-Schale",                              "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",                  RITYPE_MASK,            ITEM_MASK_DEKU,                  GI_MASK_DEKU,                GID_MASK_DEKU),
    RI(RI_MASK_DON_GERO,              "the",  "Don Gero Mask",               "le",     "Masque de Don Gero",                             "",      "",       "Don Geros Maske",                          "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",              RITYPE_MASK,            ITEM_MASK_DON_GERO,              GI_MASK_DON_GERO,            GID_MASK_DON_GERO),
    RI(RI_MASK_FIERCE_DEITY,          "the",  "Fierce Deity Mask",           "le",     "Masque de Puissance des Fées",                   "das",   "das",    "Gesicht der grimmigen Gottheit",           "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",          RITYPE_MASK,            ITEM_MASK_FIERCE_DEITY,          GI_MASK_FIERCE_DEITY,        GID_MASK_FIERCE_DEITY),
    RI(RI_MASK_GARO,                  "",     "Garo's Mask",                 "le",     "Masque de Don Gero",                             "",      "",       "Garos Maske",                              "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",                RITYPE_MASK,            ITEM_MASK_GARO,                  GI_MASK_GARO,                GID_MASK_GARO),
    RI(RI_MASK_GIANT,                 "the",  "Giant's Mask",                "le",     "Masque du Géant",                                "die",   "die",    "Maske des Giganten",                       "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",               RITYPE_MASK,            ITEM_MASK_GIANT,                 GI_MASK_GIANT,               GID_MASK_GIANT),
    RI(RI_MASK_GIBDO,                 "the",  "Gibdo Mask",                  "le",     "Masque de la Momie",                             "",      "",       "Gibdos Maske",                             "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",                 RITYPE_MASK,            ITEM_MASK_GIBDO,                 GI_MASK_GIBDO,               GID_MASK_GIBDO),
    RI(RI_MASK_GORON,                 "the",  "Goron Mask",                  "le",     "Masque Goron",                                   "die",   "die",    "Goronen-Haut",                             "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",                 RITYPE_MASK,            ITEM_MASK_GORON,                 GI_MASK_GORON,               GID_MASK_GORON),
    RI(RI_MASK_GREAT_FAIRY,           "the",  "Great Fairy Mask",            "le",     "Masque des Grandes Fées",                        "die",   "die",    "Feen-Maske",                               "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",           RITYPE_MASK,            ITEM_MASK_GREAT_FAIRY,           GI_MASK_GREAT_FAIRY,         GID_MASK_GREAT_FAIRY),
    RI(RI_MASK_KAFEIS_MASK,           "",     "Kafei's Mask",                "le",     "Masque de Kafei",                                "",      "",       "Kafeis Maske",                             "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",               RITYPE_MASK,            ITEM_MASK_KAFEIS_MASK,           GI_MASK_KAFEIS_MASK,         GID_MASK_KAFEIS_MASK),
    RI(RI_MASK_KAMARO,                "",     "Kamaro's Mask",               "le",     "Masque de Kamaro",                               "",      "",       "Kamaros Maske",                            "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",              RITYPE_MASK,            ITEM_MASK_KAMARO,                GI_MASK_KAMARO,              GID_MASK_KAMARO),
    RI(RI_MASK_KEATON,                "the",  "Keaton Mask",                 "le",     "Masque du Renard",                               "die",   "die",    "Fuchs-Maske",                              "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",                RITYPE_MASK,            ITEM_MASK_KEATON,                GI_MASK_KEATON,              GID_MASK_KEATON),
    RI(RI_MASK_POSTMAN,               "the",  "Postman's Hat",               "la",     "Casquette du Facteur",                           "die",   "die",    "Mütze des Postboten",                      "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",              RITYPE_MASK,            ITEM_MASK_POSTMAN,               GI_MASK_POSTMAN,             GID_MASK_POSTMAN),
    RI(RI_MASK_ROMANI,                "",     "Romani's Mask",               "le",     "Masque de Romani",                               "",      "",       "Romanis Maske",                            "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",              RITYPE_MASK,            ITEM_MASK_ROMANI,                GI_MASK_ROMANI,              GID_MASK_ROMANI),
    RI(RI_MASK_SCENTS,                "the",  "Mask of Scents",              "le",     "Masque des Parfums",                             "die",   "die",    "Maske der Düfte",                          "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",             RITYPE_MASK,            ITEM_MASK_SCENTS,                GI_MASK_SCENTS,              GID_MASK_SCENTS),
    RI(RI_MASK_STONE,                 "the",  "Stone Mask",                  "le",     "Masque de la Pierre",                            "die",   "die",    "Fels-Maske",                               "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",                 RITYPE_MASK,            ITEM_MASK_STONE,                 GI_MASK_STONE,               GID_MASK_STONE),
    RI(RI_MASK_TRUTH,                 "the",  "Mask of Truth",               "le",     "Masque de Vérité",                               "die",   "die",    "Maske der Wahrheit",                       "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",              RITYPE_MASK,            ITEM_MASK_TRUTH,                 GI_MASK_TRUTH,               GID_MASK_TRUTH),
    RI(RI_MASK_ZORA,                  "the",  "Zora Mask",                   "le",     "Masque Zora",                                    "die",   "die",    "Zora-Schuppen",                            "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",                  RITYPE_MASK,            ITEM_MASK_ZORA,                  GI_MASK_ZORA,                GID_MASK_ZORA),
    RI(RI_MILK_REFILL,                "a",    "Milk Refill",                 "une",    "Recharge de Lait",                               "",      "",       "Milch",                                    "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",                RITYPE_JUNK,            ITEM_MILK,                       GI_MILK,                     GID_MILK),
    RI(RI_MOONS_TEAR,                 "the",  "Moon's Tear",                 "une",    "Larme de Lune",                                  "die",   "die",    "Mondträne",                                "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",                RITYPE_MAJOR,           ITEM_MOONS_TEAR,                 GI_MOONS_TEAR,               GID_MOONS_TEAR),
    RI(RI_MUSHROOM,                   "a",    "Magic Mushroom",              "un",     "Champignon Magique",                             "ein",   "einen",  "Magischer Pilz",                           "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",             RITYPE_MAJOR,           ITEM_MUSHROOM,                   GI_MUSHROOM,                 GID_MUSHROOM),
    RI(RI_NONE,                       "",     "literally nothing",           "",       "littéralement rien",                             "",      "",       "NICHTS",                                   "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",           RITYPE_JUNK,            ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OCARINA,                    "the",  "Ocarina of Time",             "l'",     "Ocarina du Temps",                               "die",   "die",    "Okarina der Zeit",                         "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",           RITYPE_MAJOR,            ITEM_OCARINA_OF_TIME,                       GI_OCARINA_OF_TIME,                     GID_OCARINA),
    RI(RI_OWL_CLOCK_TOWN_SOUTH,       "the",  "Clock Town Owl Statue",       "la",     "Statue de Hibou de Bourg Clocher",               "die",   "die",    "Eulenstatue (Unruh-Stadt)",                "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",      RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OWL_GREAT_BAY_COAST,        "the",  "Great Bay Coast Owl Statue",  "la",     "Statue de Hibou de la Plage de la Grande Baie",  "die",   "die",    "Eulenstatue (Schädelküste)",               "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME", RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OWL_IKANA_CANYON,           "the",  "Ikana Canyon Owl Statue",     "la",     "Statue de Hibou de la Vallée Ikana",             "die",   "die",    "Eulenstatue (Ikana Canyon)",               "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",    RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OWL_MILK_ROAD,              "the",  "Milk Road Owl Statue",        "la",     "Statue de Hibou de la Route du Lait",            "die",   "die",    "Eulenstatue (Milchstraße)",                "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",       RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OWL_MOUNTAIN_VILLAGE,       "the",  "Mountain Village Owl Statue", "la",     "Statue de Hibou du 	Village dans la Montagne",   "die",   "die",    "Eulenstatue (Bergsiedlung)",               "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OWL_SNOWHEAD,               "the",  "Snowhead Owl Statue",         "la",     "Statue de Hibou du Pic des Neiges",              "die",   "die",    "Eulenstatue (Pic Hibernia)",               "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",        RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OWL_SOUTHERN_SWAMP,         "the",  "Southern Swamp Owl Statue",   "la",     "Statue de Hibou Marais du Sud",                  "die",   "die",    "Eulenstatue (Sümpfe des Vergessens)",      "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",  RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OWL_STONE_TOWER,            "the",  "Stone Tower Owl Statue",      "la",     "Statue de Hibou de la Forteresse de Pierre",     "die",   "die",    "Eulenstatue (Felsenturm)",                 "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",     RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OWL_WOODFALL,               "the",  "Woodfall Owl Statue",         "la",     "Statue de Hibou du Bois-Cascade",                "die",   "die",    "Eulenstatue (Dämmerwald)",                 "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",        RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_OWL_ZORA_CAPE,              "the",  "Zora Cape Owl Statue",        "la",     "Statue de Hibou du Théâtre des Zoras",           "die",   "die",    "Eulenstatue (Kap Zora)",                   "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",       RITYPE_LESSER,          ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_PENDANT_OF_MEMORIES,        "the",  "Pendant of Memories",         "le",     "Pendentif des Amoureux",                         "der",   "den",    "Glücksbringer",                            "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",        RITYPE_MAJOR,           ITEM_PENDANT_OF_MEMORIES,        GI_PENDANT_OF_MEMORIES,      GID_PENDANT_OF_MEMORIES),
    RI(RI_PICTOGRAPH_BOX,             "a",    "Pictograph Box",              "la",     "Boîte à Images",                                 "die",   "die",    "Foto-Box",                                 "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",             RITYPE_MAJOR,           ITEM_PICTOGRAPH_BOX,             GI_PICTOGRAPH_BOX,           GID_PICTOGRAPH_BOX),
    RI(RI_POWDER_KEG,                 "a",    "Powder Keg",                  "un",     "Baril de Poudre",                                "ein",   "ein",    "Pulverfass",                               "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",                 RITYPE_MAJOR,           ITEM_POWDER_KEG,                 GI_POWDER_KEG,               GID_POWDER_KEG),
    RI(RI_PROGRESSIVE_BOMB_BAG,       "a",    "Progressive Bomb Bag",        "un",     "Sac de Bombes Progressif",                       "eine",  "eine",   "Bombentasche (prog.)",                     "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",       RITYPE_MAJOR,           ITEM_BOMB_BAG_20,                GI_BOMB_BAG_20,              GID_BOMB_BAG_20),
    RI(RI_PROGRESSIVE_BOW,            "a",    "Progressive Bow",             "un",     "Arc Progressif",                                 "ein",   "einen",  "Bogen (prog.)",                            "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",            RITYPE_MAJOR,           ITEM_BOW,                        GI_QUIVER_30,                GID_BOW),
    RI(RI_PROGRESSIVE_LULLABY,        "",     "Progressive Goron Lullaby",   "la",     "Berceuse Goron Progressive",                     "ein",   "ein",    "Goronisches Schlummerlied (prog.)",        "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",  RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_PROGRESSIVE_MAGIC,          "",     "Progressive Magic",           "de la",  "Magie Progressive",                              "",      "",       "Magische Kraft (prog.)",                   "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",          RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_MAGIC_JAR_SMALL),
    RI(RI_PROGRESSIVE_SWORD,          "a",    "Progressive Sword",           "une",    "Épée Progressive",                               "ein",   "ein",    "Schwert (prog.)",                          "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",          RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_SWORD_KOKIRI),
    RI(RI_PROGRESSIVE_WALLET,         "a",    "Progressive Wallet",          "une",    "Bourse Progressive",                             "eine",  "eine",   "Geldbörse (prog.)",                        "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",         RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_WALLET_ADULT),
    RI(RI_QUIVER_40,                  "the",  "Large Quiver",                "le",     "Grand Carquois",                                 "ein",   "einen",  "Großer Köcher",                            "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",               RITYPE_LESSER,          ITEM_QUIVER_40,                  GI_QUIVER_40,                GID_QUIVER_40),
    RI(RI_QUIVER_50,                  "the",  "Largest Quiver",              "le",     "Carquois Géant",                                 "der",   "den",    "Riesenköcher",                             "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",             RITYPE_LESSER,          ITEM_QUIVER_50,                  GI_QUIVER_50,                GID_QUIVER_50),
    RI(RI_RECOVERY_HEART,             "a",    "Recovery Heart",              "un",     "Coeur",                                          "ein",   "ein",    "Herz",                                     "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",             RITYPE_JUNK,            ITEM_RECOVERY_HEART,             GI_RECOVERY_HEART,           GID_RECOVERY_HEART),
    RI(RI_RED_POTION_REFILL,          "a",    "Red Potion Refill",           "une",    "Recharge de Potion Rouge",                       "",      "",       "Rotes Elixier",                            "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",          RITYPE_JUNK,            ITEM_POTION_RED,                 GI_POTION_RED,               GID_POTION_RED),
    RI(RI_REMAINS_GOHT,               "",     "Goht's Remains",              "les",    "Restes de Rhork",                                "",      "",       "Gohts Vermächtnis",                        "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",             RITYPE_MAJOR,           ITEM_REMAINS_GOHT,               GI_REMAINS_GOHT,             GID_REMAINS_GOHT),
    RI(RI_REMAINS_GYORG,              "",     "Gyorg's Remains",             "les",    "Restes de Gyorg",                                "",      "",       "Gyorgs Vermächtnis",                       "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",            RITYPE_MAJOR,           ITEM_REMAINS_GYORG,              GI_REMAINS_GYORG,            GID_REMAINS_GYORG),
    RI(RI_REMAINS_ODOLWA,             "",     "Odolwa's Remains",            "les",    "Restes d'Odolwa",                                "",      "",       "Odolwas Vermächtnis",                      "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",           RITYPE_MAJOR,           ITEM_REMAINS_ODOLWA,             GI_REMAINS_ODOLWA,           GID_REMAINS_ODOLWA),
    RI(RI_REMAINS_TWINMOLD,           "",     "Twinmold's Remains",          "les",    "Restes de Skorn",                                "",      "",       "Twinmolds Vermächtnis",                    "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",         RITYPE_MAJOR,           ITEM_REMAINS_TWINMOLD,           GI_REMAINS_TWINMOLD,         GID_REMAINS_TWINMOLD),
    RI(RI_ROOM_KEY,                   "the",  "Room Key",                    "la",     "Clé de la Chambre",                              "ein",   "einen",  "Zimmerschlüssel",                          "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",                   RITYPE_MAJOR,           ITEM_ROOM_KEY,                   GI_ROOM_KEY,                 GID_ROOM_KEY),
    RI(RI_RUPEE_BLUE,                 "a",    "Blue Rupee",                  "un",     "Rubis Bleu",                                     "ein",   "einen",  "Blauer Rubin",                             "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",                 RITYPE_JUNK,            ITEM_RUPEE_BLUE,                 GI_RUPEE_BLUE,               GID_RUPEE_BLUE),
    RI(RI_RUPEE_GREEN,                "a",    "Green Rupee",                 "un",     "Rubis Vert",                                     "ein",   "einen",  "Grüner Rubin",                             "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",                RITYPE_JUNK,            ITEM_RUPEE_GREEN,                GI_RUPEE_GREEN,              GID_RUPEE_GREEN),
    RI(RI_RUPEE_HUGE,                 "a",    "Huge Rupee",                  "un",     "Rubis Jaune",                                    "ein",   "einen",  "Goldener Rubin",                           "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",                 RITYPE_JUNK,            ITEM_RUPEE_HUGE,                 GI_RUPEE_HUGE,               GID_RUPEE_HUGE),
    RI(RI_RUPEE_PURPLE,               "a",    "Purple Rupee",                "un",     "Rubis Violet",                                   "ein",   "einen",  "Purpurner Rubin",                          "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",               RITYPE_JUNK,            ITEM_RUPEE_PURPLE,               GI_RUPEE_PURPLE,             GID_RUPEE_PURPLE),
    RI(RI_RUPEE_RED,                  "a",    "Red Rupee",                   "un",     "Rubis Rouge",                                    "ein",   "einen",  "Roter Rubin",                              "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",                  RITYPE_JUNK,            ITEM_RUPEE_RED,                  GI_RUPEE_RED,                GID_RUPEE_RED),
    RI(RI_RUPEE_SILVER,               "a",    "Silver Rupee",                "un",     "Rubis d'Argent",                                 "ein",   "einen",  "Silberner Rubin",                          "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",               RITYPE_JUNK,            ITEM_RUPEE_SILVER,               GI_RUPEE_SILVER,             GID_RUPEE_SILVER),
    RI(RI_SHIELD_HERO,                "the",  "Hero's Shield",               "le",     "Bouclier du Brave",                              "der",   "den",    "Heroenschild",                             "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",              RITYPE_MAJOR,           ITEM_SHIELD_HERO,                GI_SHIELD_HERO,              GID_SHIELD_HERO),
    RI(RI_SHIELD_MIRROR,              "the",  "Mirror Shield",               "le",     "Bouclier Miroir",                                "der",   "den",    "Spiegelschild",                            "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",              RITYPE_MAJOR,           ITEM_SHIELD_MIRROR,              GI_SHIELD_MIRROR,            GID_SHIELD_MIRROR),
    RI(RI_SINGLE_MAGIC,               "the",  "Power of Magic",              "la",     "Magie",                                          "die",   "die",    "Magische Kraft",                           "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",             RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_MAGIC_JAR_SMALL),
    RI(RI_SNOWHEAD_BOSS_KEY,          "the",  "Snowhead Boss Key",           "la",     "Clé d'Or Temple du Pic des Neiges",              "der",   "den",    "Master-Schlüssel (Pic Hibernia Tempel)",   "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",          RITYPE_BOSS_KEY,        ITEM_KEY_BOSS,                   GI_KEY_BOSS,                 GID_KEY_BOSS),
    RI(RI_SNOWHEAD_COMPASS,           "the",  "Snowhead Compass",            "la",     "Boussole du Temple du Pic des Neiges",           "der",   "den",    "Kompass (Pic Hibernia Tempel)",            "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",           RITYPE_LESSER,          ITEM_COMPASS,                    GI_COMPASS,                  GID_COMPASS),
    RI(RI_SNOWHEAD_MAP,               "the",  "Snowhead Map",                "la",     "Carte du Temple du Pic des Neiges",              "die",   "die",    "Karte (Pic Hibernia Tempel)",              "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",               RITYPE_LESSER,          ITEM_DUNGEON_MAP,                GI_MAP,                      GID_DUNGEON_MAP),
    RI(RI_SNOWHEAD_SMALL_KEY,         "a",    "Snowhead Small Key",          "une",    "Petite Clé du Temple du Pic des Neiges",         "ein",   "einen",  "Kleiner Schlüssel (Pic Hibernia Tempel)",  "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",         RITYPE_SMALL_KEY,       ITEM_KEY_SMALL,                  GI_KEY_SMALL,                GID_KEY_SMALL),
    RI(RI_SNOWHEAD_STRAY_FAIRY,       "a",    "Snowhead Stray Fairy",        "une",    "Fée Perdue du Temple du Pic des Neiges",         "eine",  "eine",   "Verirrte Fee (Pic Hibernia Tempel)",       "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",       RITYPE_STRAY_FAIRY,     ITEM_STRAY_FAIRIES,              GI_STRAY_FAIRY,              GID_NONE),
    RI(RI_SONG_ELEGY,                 "the",  "Elegy of Emptiness",          "l'",     "Hymne du Vide",                                  "die",   "die",    "Elegie des leeren Herzens",                "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",         RITYPE_MAJOR,           ITEM_SONG_ELEGY,                 GI_NONE,                     GID_NONE),
    RI(RI_SONG_EPONA,                 "",     "Epona's Song",                "le",     "Chant d'Epona",                                  "",      "",       "Eponas Lied",                              "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",               RITYPE_MAJOR,           ITEM_SONG_EPONA,                 GI_NONE,                     GID_NONE),
    RI(RI_SONG_HEALING,               "the",  "Song of Healing",             "le",     "Chant de l'Apaisement",                          "das",   "das",    "Lied der Befreiung",                       "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",            RITYPE_MAJOR,           ITEM_SONG_HEALING,               GI_NONE,                     GID_NONE),
    RI(RI_SONG_LULLABY_INTRO,         "the",  "Goron Lullaby Intro",         "l'",     "Intro de la Berceuse Goron",                     "das",   "das",    "Schlummerlied (Intro)",                    "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",        RITYPE_MAJOR,           ITEM_SONG_LULLABY_INTRO,         GI_NONE,                     GID_NONE),
    RI(RI_SONG_LULLABY,               "the",  "Goron Lullaby",               "la",     "Berceuse Goron",                                 "das",   "das",    "Goronische Schlummerlied",                 "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",              RITYPE_MAJOR,           ITEM_SONG_LULLABY,               GI_NONE,                     GID_NONE),
    RI(RI_SONG_NOVA,                  "the",  "New Wave Bossa Nova",         "la",     "Bossa Nova des Flots",                           "der",   "den",    "Bossa Nova der Kaskaden",                  "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",        RITYPE_MAJOR,           ITEM_SONG_NOVA,                  GI_NONE,                     GID_NONE),
    RI(RI_SONG_OATH,                  "the",  "Oath to Order",               "l'",     "Ode de l'Appel",                                 "der",   "den",    "Gesang des Himmels",                       "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",              RITYPE_MAJOR,           ITEM_SONG_OATH,                  GI_NONE,                     GID_NONE),
    RI(RI_SONG_SOARING,               "the",  "Song of Soaring",             "le",     "Chant de l'Envol",                               "das",   "das",    "Lied der Schwingen",                       "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",            RITYPE_MAJOR,           ITEM_SONG_SOARING,               GI_NONE,                     GID_NONE),
    RI(RI_SONG_SONATA,                "the",  "Sonata of Awakening",         "le",     "Sonate de l'Éveil",                              "die",   "die",    "Sonate des Erwachens",                     "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",        RITYPE_MAJOR,           ITEM_SONG_SONATA,                GI_NONE,                     GID_NONE),
    RI(RI_SONG_STORMS,                "the",  "Song of Storms",              "le",     "Chant des Tempêtes",                             "die",   "die",    "Hymne des Sturms",                         "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",             RITYPE_MAJOR,           ITEM_SONG_STORMS,                GI_NONE,                     GID_NONE),
    RI(RI_SONG_SUN,                   "the",  "Sun's Song",                  "le",     "Chant du Soleil",                                "die",   "die",    "Hymne der Sonne",                          "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",                 RITYPE_MAJOR,           ITEM_SONG_SUN,                   GI_NONE,                     GID_NONE),
    RI(RI_SONG_TIME,                  "the",  "Song of Time",                "le",     "Chant du Temps",                                 "die",   "die",    "Hymne der Zeit",                           "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",               RITYPE_MAJOR,           ITEM_SONG_TIME,                  GI_NONE,                     GID_NONE),
    RI(RI_SOUL_GOHT,                  "the",  "Soul of Goht",                "l'",     "Âme de Rhork",                                   "",      "",       "Gohts Seele",                              "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",               RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_GYORG,                 "the",  "Soul of Gyorg",               "l'",     "Âme de Gyorg",                                   "",      "",       "Gyorgs Seele",                             "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",              RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_MAJORA,                "the",  "Soul of Majora",              "l'",     "Âme de Majora",                                  "",      "",       "Majoras Seele",                            "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",             RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_ODOLWA,                "the",  "Soul of Odolwa",              "l'",     "Âme d'Odolwa",                                   "",      "",       "Odolwas Seele",                            "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",             RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_SOUL_TWINMOLD,              "the",  "Soul of Twinmold",            "l'",     "Âme de Skorn",                                   "",      "",       "Twinmolds Seele",                          "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",           RITYPE_MAJOR,           ITEM_NONE,                       GI_NONE,                     GID_NONE),
    RI(RI_STONE_TOWER_BOSS_KEY,       "the",  "Stone Tower Boss Key",        "la",     "Clé d'Or de la Forteresse de Pierre",            "der",   "den",    "Master-Schlüssel (Felsenturm Tempel)",     "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",       RITYPE_BOSS_KEY,        ITEM_KEY_BOSS,                   GI_KEY_BOSS,                 GID_KEY_BOSS),
    RI(RI_STONE_TOWER_COMPASS,        "the",  "Stone Tower Compass",         "la",     "Boussole de la Forteresse de Pierre",            "der",   "den",    "Kompass (Felsenturm Tempel)",              "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",        RITYPE_LESSER,          ITEM_COMPASS,                    GI_COMPASS,                  GID_COMPASS),
    RI(RI_STONE_TOWER_MAP,            "the",  "Stone Tower Map",             "la",     "Carte de la Forteresse de Pierre",               "die",   "die",    "Karte (Felsenturm Tempel)",                "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",            RITYPE_LESSER,          ITEM_DUNGEON_MAP,                GI_MAP,                      GID_DUNGEON_MAP),
    RI(RI_STONE_TOWER_SMALL_KEY,      "a",    "Stone Tower Small Key",       "une",    "Petite Clé de la Forteresse de Pierre",          "ein",   "einen",  "Kleiner Schlüssel (Felsenturm Tempel)",    "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",      RITYPE_SMALL_KEY,       ITEM_KEY_SMALL,                  GI_KEY_SMALL,                GID_KEY_SMALL),
    RI(RI_STONE_TOWER_STRAY_FAIRY,    "a",    "Stone Tower Stray Fairy",     "une",    "Fée Perdue de la Forteresse de Pierre ",         "eine",  "eine",   "Verirrte Fee (Felsenturm Tempel)",         "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",    RITYPE_STRAY_FAIRY,     ITEM_STRAY_FAIRIES,              GI_STRAY_FAIRY,              GID_NONE),
    RI(RI_SWORD_GILDED,               "the",  "Gilded Sword",                "la",     "Lame Dorée",                                     "die",   "die",    "Schmirgelklinge",                          "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",               RITYPE_LESSER,          ITEM_SWORD_GILDED,               GI_SWORD_GILDED,             GID_SWORD_GILDED),
    RI(RI_SWORD_KOKIRI,               "the",  "Kokiri Sword",                "l'",     "Épée Kokiri",                                    "das",   "das",    "Kokiri-Schwert",                           "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",               RITYPE_MAJOR,           ITEM_SWORD_KOKIRI,               GI_SWORD_KOKIRI,             GID_SWORD_KOKIRI),
    RI(RI_SWORD_RAZOR,                "the",  "Razor Sword",                 "la",     "Lame Rasoir",                                    "das",   "das",    "Elfenschwert",                             "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",                RITYPE_LESSER,          ITEM_SWORD_RAZOR,                GI_SWORD_RAZOR,              GID_SWORD_RAZOR),
    RI(RI_TINGLE_MAP_CLOCK_TOWN,      "",     "Tingle's Clock Town Map",     "la",     "Carte de Tingle du Bourg-Clocher",               "",      "",       "Tingles Karte (Unruh-Stadt)",              "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",    RITYPE_LESSER,          ITEM_TINGLE_MAP,                 GI_TINGLE_MAP_CLOCK_TOWN,    GID_TINGLE_MAP),
    RI(RI_TINGLE_MAP_GREAT_BAY,       "",     "Tingle's Great Bay Map",      "la",     "Carte de Tingle de la Grande Baie",              "",      "",       "Tingles Karte (Schädelbucht)",             "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",     RITYPE_LESSER,          ITEM_TINGLE_MAP,                 GI_TINGLE_MAP_GREAT_BAY,     GID_TINGLE_MAP),
    RI(RI_TINGLE_MAP_ROMANI_RANCH,    "",     "Tingle's Romani Ranch Map",   "la",     "Carte de Tingle du Ranch Romani",                "",      "",       "Tingles Karte (Romani Ranch)",             "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",  RITYPE_LESSER,          ITEM_TINGLE_MAP,                 GI_TINGLE_MAP_ROMANI_RANCH,  GID_TINGLE_MAP),
    RI(RI_TINGLE_MAP_SNOWHEAD,        "",     "Tingle's Snowhead Map",       "la",     "Carte de Tingle du Pic des Neiges",              "",      "",       "Tingles Karte (Pic Hibernia)",             "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",      RITYPE_LESSER,          ITEM_TINGLE_MAP,                 GI_TINGLE_MAP_SNOWHEAD,      GID_TINGLE_MAP),
    RI(RI_TINGLE_MAP_STONE_TOWER,     "",     "Tingle's Stone Tower Map",    "la",     "Carte de Tingle de la Forteresse de Pierre",     "",      "",       "Tingles Karte (Felsenturm)",               "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",   RITYPE_LESSER,          ITEM_TINGLE_MAP,                 GI_TINGLE_MAP_STONE_TOWER,   GID_TINGLE_MAP),
    RI(RI_TINGLE_MAP_WOODFALL,        "",     "Tingle's Woodfall Map",       "la",     "Carte de Tingle du Bois-Cascade",                "",      "",       "Tingles Karte (Dämmerwald)",               "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",      RITYPE_LESSER,          ITEM_TINGLE_MAP,                 GI_TINGLE_MAP_WOODFALL,      GID_TINGLE_MAP),
    RI(RI_WALLET_ADULT,               "the",  "Adult's Wallet",              "la",     "Bourse d'Adulte",                                "eine",  "eine",   "Große Geldbörse",                          "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",             RITYPE_MAJOR,           ITEM_WALLET_ADULT,               GI_WALLET_ADULT,             GID_WALLET_ADULT),
    RI(RI_WALLET_GIANT,               "the",  "Giant's Wallet",              "la",     "Bourse Géante",                                  "eine",  "eine",   "Riesenbörse",                              "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",             RITYPE_LESSER,          ITEM_WALLET_GIANT,               GI_WALLET_GIANT,             GID_WALLET_GIANT),
    RI(RI_WOODFALL_BOSS_KEY,          "the",  "Woodfall Boss Key",           "la",     "Clé d'Or du Temple de Bois-Cascade",             "der",   "den",    "Master-Schlüssel (Dämmerwald Tempel)",     "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",          RITYPE_BOSS_KEY,        ITEM_KEY_BOSS,                   GI_KEY_BOSS,                 GID_KEY_BOSS),
    RI(RI_WOODFALL_COMPASS,           "the",  "Woodfall Compass",            "la",     "Boussole du Temple de Bois-Cascade",             "der",   "den",    "Kompass (Dämmerwald Tempel)",              "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",           RITYPE_LESSER,          ITEM_COMPASS,                    GI_COMPASS,                  GID_COMPASS),
    RI(RI_WOODFALL_MAP,               "the",  "Woodfall Map",                "la",     "Carte du Temple de Bois-Cascade",                "die",   "die",    "Karte (Dämmerwald Tempel)",                "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",               RITYPE_LESSER,          ITEM_DUNGEON_MAP,                GI_MAP,                      GID_DUNGEON_MAP),
    RI(RI_WOODFALL_SMALL_KEY,         "a",    "Woodfall Small Key",          "une",    "Petite Clé du Temple de Bois-Cascade",           "ein",   "einen",  "Kleiner Schlüssel (Dämmerwald Tempel)",    "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",         RITYPE_SMALL_KEY,       ITEM_KEY_SMALL,                  GI_KEY_SMALL,                GID_KEY_SMALL),
    RI(RI_WOODFALL_STRAY_FAIRY,       "a",    "Woodfall Stray Fairy",        "une",    "Fée Perdue du Temple de Bois-Cascade",           "eine",  "eine",   "Verirrte Fee (Dämmerwald Tempel)",         "TODO_JAPANESE_ARTICLE",  "TODO_JAPANESE_NAME",  "TODO_SPANISH_ARTICLE", "TODO_SPANISH_NAME",       RITYPE_STRAY_FAIRY,     ITEM_STRAY_FAIRIES,              GI_STRAY_FAIRY,              GID_NONE),
};
// clang-format on

std::vector<RandoItemId> StartingItemsMap = {
    RI_ARROW_FIRE,
    RI_ARROW_ICE,
    RI_ARROW_LIGHT,
    RI_BOMBERS_NOTEBOOK,
    RI_BOTTLE_EMPTY,
    RI_DEED_LAND,
    RI_DEED_MOUNTAIN,
    RI_DEED_OCEAN,
    RI_DEED_SWAMP,
    RI_DOUBLE_DEFENSE,
    RI_GREAT_FAIRY_SWORD,
    RI_GREAT_SPIN_ATTACK,
    RI_HOOKSHOT,
    RI_LENS,
    RI_LETTER_TO_KAFEI,
    RI_LETTER_TO_MAMA,
    RI_MASK_ALL_NIGHT,
    RI_MASK_BLAST,
    RI_MASK_BREMEN,
    RI_MASK_BUNNY,
    RI_MASK_CAPTAIN,
    RI_MASK_CIRCUS_LEADER,
    RI_MASK_COUPLE,
    RI_MASK_DEKU,
    RI_MASK_DON_GERO,
    RI_MASK_FIERCE_DEITY,
    RI_MASK_GARO,
    RI_MASK_GIANT,
    RI_MASK_GIBDO,
    RI_MASK_GORON,
    RI_MASK_GREAT_FAIRY,
    RI_MASK_KAFEIS_MASK,
    RI_MASK_KAMARO,
    RI_MASK_KEATON,
    RI_MASK_POSTMAN,
    RI_MASK_ROMANI,
    RI_MASK_SCENTS,
    RI_MASK_STONE,
    RI_MASK_TRUTH,
    RI_MASK_ZORA,
    RI_MOONS_TEAR,
    RI_OCARINA,
    RI_PENDANT_OF_MEMORIES,
    RI_PICTOGRAPH_BOX,
    RI_POWDER_KEG,
    RI_PROGRESSIVE_BOMB_BAG,
    RI_PROGRESSIVE_BOMB_BAG,
    RI_PROGRESSIVE_BOMB_BAG,
    RI_PROGRESSIVE_BOW,
    RI_PROGRESSIVE_BOW,
    RI_PROGRESSIVE_BOW,
    RI_PROGRESSIVE_MAGIC,
    RI_PROGRESSIVE_MAGIC,
    RI_PROGRESSIVE_SWORD,
    RI_PROGRESSIVE_SWORD,
    RI_PROGRESSIVE_SWORD,
    RI_PROGRESSIVE_WALLET,
    RI_PROGRESSIVE_WALLET,
    RI_REMAINS_GOHT,
    RI_REMAINS_GYORG,
    RI_REMAINS_ODOLWA,
    RI_REMAINS_TWINMOLD,
    RI_ROOM_KEY,
    RI_SHIELD_HERO,
    RI_SHIELD_MIRROR,
    RI_SONG_ELEGY,
    RI_SONG_EPONA,
    RI_SONG_HEALING,
    RI_PROGRESSIVE_LULLABY,
    RI_PROGRESSIVE_LULLABY,
    RI_SONG_NOVA,
    RI_SONG_OATH,
    RI_SONG_SOARING,
    RI_SONG_SONATA,
    RI_SONG_STORMS,
    RI_SONG_TIME,
};

RandoItemId GetItemIdFromName(const char* name) {
    for (auto& [randoItemId, randoStaticItem] : Items) {
        if (strcmp(name, randoStaticItem.spoilerName) == 0) {
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
        default:
            break;
    }

    if (Rando::StaticData::Items[randoItemId].getItemId != GI_NONE) {
        return (u8)Rando::StaticData::Items[randoItemId].getItemId;
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
        case RI_PROGRESSIVE_LULLABY:
            return (const char*)gItemIcons[ITEM_SONG_LULLABY];
        case RI_PROGRESSIVE_MAGIC:
            return (const char*)gItemIcons[ITEM_MAGIC_JAR_SMALL];
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

std::string GetItemName(RandoItemId randoItemId, bool includeArticle) {
    std::string result;
    auto& item = Rando::StaticData::Items[randoItemId];

    const char* article = nullptr;
    const char* name = nullptr;

    switch (gSaveContext.options.language) {
        case LANGUAGE_FRE:
            article = item.articleFre;
            name = item.nameFre;
            break;
        case LANGUAGE_GER:
            article = item.articleGer2;
            name = item.nameGer;
            break;
        case LANGUAGE_JPN:
            article = item.articleJpn;
            name = item.nameJpn;
            break;
        case LANGUAGE_SPA:
            article = item.articleSpa;
            name = item.nameSpa;
            break;
        case LANGUAGE_ENG:
        default:
            article = item.articleEng;
            name = item.nameEng;
            break;
    }

    if (includeArticle && !Ship_IsCStringEmpty(article)) {
        result += article;
        if (article != "l'") { // Special case handling with l' french article
            result += " ";
        }
    }
    result += name;

    if (randoItemId == RI_JUNK) {
        auto& junkItem = Rando::StaticData::Items[Rando::CurrentJunkItem()];
        const char* junkName = nullptr;

        switch (gSaveContext.options.language) {
            case LANGUAGE_FRE:
                junkName = junkItem.nameFre;
                break;
            case LANGUAGE_GER:
                junkName = junkItem.nameGer;
                break;
            case LANGUAGE_JPN:
                junkName = junkItem.nameJpn;
                break;
            case LANGUAGE_SPA:
                junkName = junkItem.nameSpa;
                break;
            case LANGUAGE_ENG:
            default:
                junkName = junkItem.nameEng;
                break;
        }

        result += std::string(" (") + junkName + ")";
    }

    return result;
}

} // namespace StaticData

} // namespace Rando
