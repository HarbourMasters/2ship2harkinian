#include <libultraship/libultraship.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

#include "2s2h/Rando/Logic/Logic.h"

using namespace Rando::Logic;

// clang-format off
static RegisterShipInitFunc initFunc([]() {
    Regions[RR_ASTRAL_OBSERVATORY_OUTSIDE] = RandoRegion{ .name = "Outside Astral Observatory", .sceneId = SCENE_00KEIKOKU,
        .checks = {
            CHECK(RC_ASTRAL_OBSERVATORY_MOON_TEAR, true),
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(ASTRAL_OBSERVATORY, 1),           ENTRANCE(TERMINA_FIELD, 9), true),
        },
        .connections = {
            CONNECTION(RR_TERMINA_FIELD, CAN_GROW_BEAN_PLANT)
        },
    };
    Regions[RR_ASTRAL_OBSERVATORY_PASSAGE] = RandoRegion{ .name = "Passage", .sceneId = SCENE_TENMON_DAI,
        .checks = {
            CHECK(RC_ASTRAL_OBSERVATORY_PASSAGE_CHEST, CAN_USE_EXPLOSIVE),
            CHECK(RC_ASTRAL_OBSERVATORY_PASSAGE_POT_01, true),
            CHECK(RC_ASTRAL_OBSERVATORY_PASSAGE_POT_02, true),
            CHECK(RC_ASTRAL_OBSERVATORY_PASSAGE_POT_03, true),
            CHECK(RC_ASTRAL_OBSERVATORY_PASSAGE_POT_04, true),
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(EAST_CLOCK_TOWN, 2),              ENTRANCE(ASTRAL_OBSERVATORY, 0), true),
        },
        .connections = {
            CONNECTION(RR_ASTRAL_OBSERVATORY, true),
        },
    };
    Regions[RR_ASTRAL_OBSERVATORY] = RandoRegion{ .name = "Inside Astral Observatory", .sceneId = SCENE_TENMON_DAI,
        .checks = {
            CHECK(RC_ASTRAL_OBSERVATORY_POT_01, true),
            CHECK(RC_ASTRAL_OBSERVATORY_POT_02, true),
            CHECK(RC_ASTRAL_OBSERVATORY_POT_03, true),
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(TERMINA_FIELD, 9),                ENTRANCE(ASTRAL_OBSERVATORY, 1), true),
        },
        .connections = {
            CONNECTION(RR_ASTRAL_OBSERVATORY_PASSAGE, true),
        },
    };
    Regions[RR_BOMB_SHOP] = RandoRegion{ .sceneId = SCENE_BOMYA,
        .checks = {
            CHECK(RC_BOMB_SHOP_ITEM_01, CAN_AFFORD(RC_BOMB_SHOP_ITEM_01)),
            CHECK(RC_BOMB_SHOP_ITEM_02, CAN_AFFORD(RC_BOMB_SHOP_ITEM_02)),
            // Upon saving the Bomb Shop lady, one item in the shop gets replaced with the other for the remainder of the cycle.
            CHECK(RC_BOMB_SHOP_ITEM_03, CAN_AFFORD(RC_BOMB_SHOP_ITEM_03)),
            CHECK(RC_BOMB_SHOP_ITEM_04_OR_CURIOSITY_SHOP_ITEM, CAN_AFFORD(RC_BOMB_SHOP_ITEM_04_OR_CURIOSITY_SHOP_ITEM) && RANDO_EVENTS[RE_SAVE_BOMB_SHOP_LADY]),
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(WEST_CLOCK_TOWN, 6),              ENTRANCE(BOMB_SHOP, 0), true),
        },
    };
    Regions[RR_CLOCK_TOWER_INTERIOR] = RandoRegion{ .sceneId = SCENE_INSIDETOWER,
        .checks = {
            // There are no checks here, the 2 that the mask salesman would give you are in RR_MAX
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(SOUTH_CLOCK_TOWN, 0),             ENTRANCE(CLOCK_TOWER_INTERIOR, 1), true),
        },
    };
    Regions[RR_CLOCK_TOWER_ROOF] = RandoRegion{ .sceneId = SCENE_OKUJOU,
        .checks = {
            CHECK(RC_CLOCK_TOWER_ROOF_SONG_OF_TIME, (HAS_MAGIC && CAN_BE_DEKU) || HAS_ITEM(ITEM_BOW) || HAS_ITEM(ITEM_HOOKSHOT)),
            CHECK(RC_CLOCK_TOWER_ROOF_OCARINA, (HAS_MAGIC && CAN_BE_DEKU) || HAS_ITEM(ITEM_BOW) || HAS_ITEM(ITEM_HOOKSHOT)),
            CHECK(RC_CLOCK_TOWER_ROOF_POT_01, true),
            CHECK(RC_CLOCK_TOWER_ROOF_POT_02, true),
            CHECK(RC_CLOCK_TOWER_ROOF_POT_03, true),
            CHECK(RC_CLOCK_TOWER_ROOF_POT_04, true),
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(THE_MOON, 0),                              ONE_WAY_EXIT, CAN_PLAY_SONG(OATH) && CHECK_QUEST_ITEM(QUEST_REMAINS_ODOLWA) && CHECK_QUEST_ITEM(QUEST_REMAINS_GOHT) && CHECK_QUEST_ITEM(QUEST_REMAINS_GYORG) && CHECK_QUEST_ITEM(QUEST_REMAINS_TWINMOLD)),
        },
        .oneWayEntrances = {
            ENTRANCE(CLOCK_TOWER_ROOFTOP, 0), // From clock tower platform
        },
    };
    Regions[RR_CLOCK_TOWN_EAST] = RandoRegion{ .sceneId = SCENE_TOWN,
        .checks = {
            CHECK(RC_CLOCK_TOWN_EAST_SMALL_CRATE_01, true),
            CHECK(RC_CLOCK_TOWN_EAST_SMALL_CRATE_02, true),
            CHECK(RC_CLOCK_TOWN_EAST_POSTMAN_HAT,    Flags_GetRandoInf(RANDO_INF_OBTAINED_LETTER_TO_MAMA)),
            CHECK(RC_CLOCK_TOWN_STRAY_FAIRY,         CAN_BE_DEKU),
            CHECK(RC_CLOCK_TOWN_EAST_UPPER_CHEST,    true),
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(TERMINA_FIELD, 7),                ENTRANCE(EAST_CLOCK_TOWN, 0), true),
            EXIT(ENTRANCE(SOUTH_CLOCK_TOWN, 7),             ENTRANCE(EAST_CLOCK_TOWN, 1), true), // To lower
            EXIT(ENTRANCE(ASTRAL_OBSERVATORY, 0),           ENTRANCE(EAST_CLOCK_TOWN, 2), CAN_USE_PROJECTILE),
            EXIT(ENTRANCE(SOUTH_CLOCK_TOWN, 2),             ENTRANCE(EAST_CLOCK_TOWN, 3), true), // To upper
            EXIT(ENTRANCE(TREASURE_CHEST_SHOP, 0),          ENTRANCE(EAST_CLOCK_TOWN, 4), true),
            EXIT(ENTRANCE(NORTH_CLOCK_TOWN, 1),             ENTRANCE(EAST_CLOCK_TOWN, 5), true),
            EXIT(ENTRANCE(HONEY_AND_DARLINGS_SHOP, 0),      ENTRANCE(EAST_CLOCK_TOWN, 6), true),
            EXIT(ENTRANCE(MAYORS_RESIDENCE, 0),             ENTRANCE(EAST_CLOCK_TOWN, 7), true),
            EXIT(ENTRANCE(TOWN_SHOOTING_GALLERY, 0),        ENTRANCE(EAST_CLOCK_TOWN, 8), true),
            EXIT(ENTRANCE(STOCK_POT_INN, 0),                ENTRANCE(EAST_CLOCK_TOWN, 9), true), // To lobby
            EXIT(ENTRANCE(STOCK_POT_INN, 1),                ENTRANCE(EAST_CLOCK_TOWN, 10), CAN_BE_DEKU), // To upstairs
            EXIT(ENTRANCE(MILK_BAR, 0),                     ENTRANCE(EAST_CLOCK_TOWN, 11), HAS_ITEM(ITEM_MASK_ROMANI)),
        },
    };
    Regions[RR_CLOCK_TOWN_GREAT_FAIRY_FOUNTAIN] = RandoRegion{ .name = "Clock Town", .sceneId = SCENE_YOUSEI_IZUMI,
        .checks = {
            CHECK(RC_CLOCK_TOWN_GREAT_FAIRY, CHECK_WEEKEVENTREG(WEEKEVENTREG_08_80)),
            CHECK(RC_CLOCK_TOWN_GREAT_FAIRY_ALT, CHECK_WEEKEVENTREG(WEEKEVENTREG_08_80) && (CAN_BE_DEKU || CAN_BE_GORON || CAN_BE_ZORA)),
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(NORTH_CLOCK_TOWN, 3),             ENTRANCE(FAIRY_FOUNTAIN, 0), true),
        },
    };
    Regions[RR_CLOCK_TOWN_LAUNDRY] = RandoRegion{ .sceneId = SCENE_ALLEY,
        .checks = {
            CHECK(RC_CLOCK_TOWN_STRAY_FAIRY,                    true),
            CHECK(RC_CLOCK_TOWN_LAUNDRY_FREESTANDING_RUPEE_01,  true),
            CHECK(RC_CLOCK_TOWN_LAUNDRY_FREESTANDING_RUPEE_02,  true),
            CHECK(RC_CLOCK_TOWN_LAUNDRY_FREESTANDING_RUPEE_03,  true),
            CHECK(RC_CLOCK_TOWN_LAUNDRY_GURU_GURU,              true),
            CHECK(RC_CLOCK_TOWN_LAUNDRY_SMALL_CRATE,            true),
            CHECK(RC_CLOCK_TOWN_LAUNDRY_POOL_GRASS_01, true),
            CHECK(RC_CLOCK_TOWN_LAUNDRY_POOL_GRASS_02, true),
            CHECK(RC_CLOCK_TOWN_LAUNDRY_POOL_GRASS_03, true),
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(SOUTH_CLOCK_TOWN, 6),             ENTRANCE(LAUNDRY_POOL, 0), true),
            EXIT(ENTRANCE(CURIOSITY_SHOP, 1),               ENTRANCE(LAUNDRY_POOL, 1), Flags_GetRandoInf(RANDO_INF_OBTAINED_LETTER_TO_KAFEI))
        },
    };
    Regions[RR_CLOCK_TOWN_NORTH] = RandoRegion{ .sceneId = SCENE_BACKTOWN,
        .checks = {
            CHECK(RC_CLOCK_TOWN_NORTH_TINGLE_MAP_01, CAN_USE_PROJECTILE && CAN_AFFORD(RC_CLOCK_TOWN_NORTH_TINGLE_MAP_01)),
            CHECK(RC_CLOCK_TOWN_NORTH_TINGLE_MAP_02, CAN_USE_PROJECTILE && CAN_AFFORD(RC_CLOCK_TOWN_NORTH_TINGLE_MAP_02)),
            CHECK(RC_CLOCK_TOWN_NORTH_TREE_PIECE_OF_HEART, true),
            CHECK(RC_CLOCK_TOWN_NORTH_BOMB_LADY, CAN_USE_SWORD || CAN_BE_ZORA || CAN_BE_GORON),
            CHECK(RC_CLOCK_TOWN_BOMBERS_NOTEBOOK, CAN_USE_PROJECTILE), // TODO: This will have to check for access with entrance rando
            CHECK(RC_CLOCK_TOWN_POSTBOX, HAS_ITEM(ITEM_MASK_POSTMAN)),
            CHECK(RC_KEATON_QUIZ, HAS_ITEM(ITEM_MASK_KEATON)),
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(TERMINA_FIELD, 8),                ENTRANCE(NORTH_CLOCK_TOWN, 0), true),
            EXIT(ENTRANCE(EAST_CLOCK_TOWN, 5),              ENTRANCE(NORTH_CLOCK_TOWN, 1), true),
            EXIT(ENTRANCE(SOUTH_CLOCK_TOWN, 4),             ENTRANCE(NORTH_CLOCK_TOWN, 2), true),
            EXIT(ENTRANCE(FAIRY_FOUNTAIN, 0),               ENTRANCE(NORTH_CLOCK_TOWN, 3), true),
            EXIT(ENTRANCE(DEKU_SCRUB_PLAYGROUND, 0),        ENTRANCE(NORTH_CLOCK_TOWN, 4), CAN_BE_DEKU),
        },
        .events = {
            EVENT(RE_SAVE_BOMB_SHOP_LADY, CAN_USE_SWORD || CAN_BE_ZORA || CAN_BE_GORON),
        },
    };
    Regions[RR_CLOCK_TOWN_SOUTH] = RandoRegion{ .sceneId = SCENE_CLOCKTOWER,
        .checks = {
            CHECK(RC_CLOCK_TOWN_POSTBOX, HAS_ITEM(ITEM_MASK_POSTMAN)),
            CHECK(RC_CLOCK_TOWN_SOUTH_PLATFORM_PIECE_OF_HEART, true),
            CHECK(RC_CLOCK_TOWN_SCRUB_DEED, Flags_GetRandoInf(RANDO_INF_OBTAINED_MOONS_TEAR)),
            CHECK(RC_CLOCK_TOWN_SOUTH_CHEST_UPPER, (CAN_BE_DEKU && Flags_GetRandoInf(RANDO_INF_OBTAINED_MOONS_TEAR)) || HAS_ITEM(ITEM_HOOKSHOT)),
            CHECK(RC_CLOCK_TOWN_SOUTH_CHEST_LOWER, (CAN_BE_DEKU && Flags_GetRandoInf(RANDO_INF_OBTAINED_MOONS_TEAR)) || HAS_ITEM(ITEM_HOOKSHOT)),
            CHECK(RC_CLOCK_TOWN_SOUTH_OWL_STATUE, CAN_USE_SWORD),
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(CLOCK_TOWER_INTERIOR, 1),         ENTRANCE(SOUTH_CLOCK_TOWN, 0), true),
            EXIT(ENTRANCE(TERMINA_FIELD, 6),                ENTRANCE(SOUTH_CLOCK_TOWN, 1), true),
            EXIT(ENTRANCE(EAST_CLOCK_TOWN, 3),              ENTRANCE(SOUTH_CLOCK_TOWN, 2), true), // To upper
            EXIT(ENTRANCE(WEST_CLOCK_TOWN, 2),              ENTRANCE(SOUTH_CLOCK_TOWN, 3), true), // To upper
            EXIT(ENTRANCE(NORTH_CLOCK_TOWN, 2),             ENTRANCE(SOUTH_CLOCK_TOWN, 4), true),
            EXIT(ENTRANCE(WEST_CLOCK_TOWN, 1),              ENTRANCE(SOUTH_CLOCK_TOWN, 5), true), // To lower
            EXIT(ENTRANCE(LAUNDRY_POOL, 0),                 ENTRANCE(SOUTH_CLOCK_TOWN, 6), true),
            EXIT(ENTRANCE(EAST_CLOCK_TOWN, 1),              ENTRANCE(SOUTH_CLOCK_TOWN, 7), true), // To lower
            EXIT(ENTRANCE(CLOCK_TOWER_ROOFTOP, 0),                   ONE_WAY_EXIT, true),
        },
        .connections = {
            CONNECTION(RR_MAX, true),
        },
        .oneWayEntrances = {
            ENTRANCE(SOUTH_CLOCK_TOWN, 9), // From Song of Soaring
        }
    };
    Regions[RR_CLOCK_TOWN_WEST] = RandoRegion{ .sceneId = SCENE_ICHIBA,
        .checks = {
            CHECK(RC_CLOCK_TOWN_POSTBOX, HAS_ITEM(ITEM_MASK_POSTMAN)),
            CHECK(RC_CLOCK_TOWN_WEST_BANK_ADULTS_WALLET, true),
            CHECK(RC_CLOCK_TOWN_WEST_BANK_PIECE_OF_HEART, CUR_UPG_VALUE(UPG_WALLET) >= 1),
            CHECK(RC_CLOCK_TOWN_WEST_BANK_INTEREST, CUR_UPG_VALUE(UPG_WALLET) >= 1),
            CHECK(RC_CLOCK_TOWN_WEST_SISTERS_PIECE_OF_HEART, HAS_ITEM(ITEM_MASK_KAMARO)),
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(TERMINA_FIELD, 0),                ENTRANCE(WEST_CLOCK_TOWN, 0), true),
            EXIT(ENTRANCE(SOUTH_CLOCK_TOWN, 5),             ENTRANCE(WEST_CLOCK_TOWN, 1), true), // To lower
            EXIT(ENTRANCE(SOUTH_CLOCK_TOWN, 3),             ENTRANCE(WEST_CLOCK_TOWN, 2), true), // To upper
            EXIT(ENTRANCE(SWORDMANS_SCHOOL, 0),             ENTRANCE(WEST_CLOCK_TOWN, 3), true),
            EXIT(ENTRANCE(CURIOSITY_SHOP, 0),               ENTRANCE(WEST_CLOCK_TOWN, 4), true),
            EXIT(ENTRANCE(TRADING_POST, 0),                 ENTRANCE(WEST_CLOCK_TOWN, 5), true),
            EXIT(ENTRANCE(BOMB_SHOP, 0),                    ENTRANCE(WEST_CLOCK_TOWN, 6), true),
            EXIT(ENTRANCE(POST_OFFICE, 0),                  ENTRANCE(WEST_CLOCK_TOWN, 7), true),
            EXIT(ENTRANCE(LOTTERY_SHOP, 0),                 ENTRANCE(WEST_CLOCK_TOWN, 8), true),
        },
    };
    Regions[RR_CURIOSITY_SHOP_BACK] = RandoRegion{ .name = "Back", .sceneId = SCENE_AYASHIISHOP,
        .checks = {
            CHECK(RC_KAFEIS_HIDEOUT_KEATON_MASK, true),
            CHECK(RC_KAFEIS_HIDEOUT_LETTER_TO_MAMA, true),
            CHECK(RC_KAFEIS_HIDEOUT_PENDANT_OF_MEMORIES, Flags_GetRandoInf(RANDO_INF_OBTAINED_LETTER_TO_KAFEI)),
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(LAUNDRY_POOL, 1),                 ENTRANCE(CURIOSITY_SHOP, 1), true)
        },
    };
    Regions[RR_CURIOSITY_SHOP_FRONT] = RandoRegion{ .name = "Front", .sceneId = SCENE_AYASHIISHOP,
        .checks = {
            CHECK(RC_BOMB_SHOP_ITEM_04_OR_CURIOSITY_SHOP_ITEM, CAN_AFFORD(RC_BOMB_SHOP_ITEM_04_OR_CURIOSITY_SHOP_ITEM)),
            CHECK(RC_CURIOSITY_SHOP_SPECIAL_ITEM, CAN_AFFORD(RC_CURIOSITY_SHOP_SPECIAL_ITEM) && (RANDO_EVENTS[RE_SAVE_BOMB_SHOP_LADY] || CHECK_WEEKEVENTREG(WEEKEVENTREG_SAKON_DEAD))),
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(WEST_CLOCK_TOWN, 4),              ENTRANCE(CURIOSITY_SHOP, 0), true)
        },
    };
    Regions[RR_HONEY_AND_DARLING] = RandoRegion{ .sceneId = SCENE_BOWLING,
        .checks = {
            CHECK(RC_CLOCK_TOWN_EAST_HONEY_DARLING_ALL_DAYS, HAS_ITEM(ITEM_BOW) && HAS_ITEM(ITEM_BOMBCHU) && HAS_ITEM(ITEM_BOMB)),
            CHECK(RC_CLOCK_TOWN_EAST_HONEY_DARLING_ANY_DAY, HAS_ITEM(ITEM_BOW) || HAS_ITEM(ITEM_BOMBCHU) || HAS_ITEM(ITEM_BOMB)),
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(EAST_CLOCK_TOWN, 6),              ENTRANCE(HONEY_AND_DARLINGS_SHOP, 0), true),
        },
    };
    Regions[RR_INN] = RandoRegion{ .sceneId = SCENE_YADOYA,
        .checks = {
            CHECK(RC_STOCK_POT_INN_COUPLES_MASK, HAS_ITEM(ITEM_MASK_KAFEIS_MASK) && Flags_GetRandoInf(RANDO_INF_OBTAINED_PENDANT_OF_MEMORIES) && RANDO_EVENTS[RE_RETRIEVE_SUN_MASK]),
            CHECK(RC_STOCK_POT_INN_GRANDMA_LONG_STORY, HAS_ITEM(ITEM_MASK_ALL_NIGHT)),
            CHECK(RC_STOCK_POT_INN_GRANDMA_SHORT_STORY, HAS_ITEM(ITEM_MASK_ALL_NIGHT)),
            CHECK(RC_STOCK_POT_INN_GUEST_ROOM_CHEST,Flags_GetRandoInf(RANDO_INF_OBTAINED_ROOM_KEY)),
            CHECK(RC_STOCK_POT_INN_LETTER_TO_KAFEI, HAS_ITEM(ITEM_MASK_KAFEIS_MASK)),
            CHECK(RC_STOCK_POT_INN_ROOM_KEY, true),
            CHECK(RC_STOCK_POT_INN_STAFF_ROOM_CHEST, true),
            CHECK(RC_STOCK_POT_INN_TOILET_HAND, 
                Flags_GetRandoInf(RANDO_INF_OBTAINED_DEED_LAND) || Flags_GetRandoInf(RANDO_INF_OBTAINED_DEED_SWAMP) ||
                Flags_GetRandoInf(RANDO_INF_OBTAINED_DEED_MOUNTAIN) || Flags_GetRandoInf(RANDO_INF_OBTAINED_DEED_OCEAN) ||
                Flags_GetRandoInf(RANDO_INF_OBTAINED_LETTER_TO_MAMA) || Flags_GetRandoInf(RANDO_INF_OBTAINED_LETTER_TO_KAFEI)
            ),
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(EAST_CLOCK_TOWN, 9),              ENTRANCE(STOCK_POT_INN, 0), true), // From ground floor
            EXIT(ENTRANCE(EAST_CLOCK_TOWN, 10),             ENTRANCE(STOCK_POT_INN, 1), true), // From upstairs
        },
        .events = {
            EVENT(RE_ACCESS_FISH, true),
            EVENT(RE_ACCESS_BUGS, true),
        },
    };
    Regions[RR_LOTTERY_SHOP] = RandoRegion{ .sceneId = SCENE_TAKARAKUJI,
        .checks = {
            CHECK(RC_CLOCK_TOWN_WEST_LOTTERY, true),
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(WEST_CLOCK_TOWN, 8),              ENTRANCE(LOTTERY_SHOP, 0), true),
        },
    };
    Regions[RR_MAYOR_RESIDENCE] = RandoRegion{ .sceneId = SCENE_SONCHONOIE,
        .checks = {
            CHECK(RC_MAYORS_OFFICE_PIECE_OF_HEART, HAS_ITEM(ITEM_MASK_COUPLE)),
            CHECK(RC_MAYORS_OFFICE_KAFEIS_MASK, true)
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(EAST_CLOCK_TOWN, 7),              ENTRANCE(MAYORS_RESIDENCE, 0), true),
        },
    };
    Regions[RR_MILK_BAR] = RandoRegion{ .sceneId = SCENE_MILK_BAR,
        .checks = {
            // TODO : Add shop checks.
            CHECK(RC_MILK_BAR_CIRCUS_LEADER_MASK, CAN_BE_DEKU && CAN_BE_GORON && CAN_BE_ZORA && HAS_ITEM(ITEM_OCARINA_OF_TIME)),
            CHECK(RC_MILK_BAR_MADAME_AROMA, HAS_ITEM(ITEM_MASK_KAFEIS_MASK) && Flags_GetRandoInf(RANDO_INF_OBTAINED_LETTER_TO_MAMA)),
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(EAST_CLOCK_TOWN, 11),             ENTRANCE(MILK_BAR, 0), true),
        },
    };
    Regions[RR_POST_OFFICE] = RandoRegion{ .sceneId = SCENE_POSTHOUSE,
        .checks = {
            // TODO: Trick for doing without the Bunny Hood
            CHECK(RC_CLOCK_TOWN_WEST_POSTMAN_MINIGAME, HAS_ITEM(ITEM_MASK_BUNNY)),
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(WEST_CLOCK_TOWN, 7),              ENTRANCE(POST_OFFICE, 0), true),
        },
    };
    Regions[RR_SWORDSMAN_SCHOOL] = RandoRegion{ .sceneId = SCENE_DOUJOU,
        .checks = {
            CHECK(RC_SWORDSMAN_SCHOOL_PIECE_OF_HEART, CAN_USE_HUMAN_SWORD),
            CHECK(RC_SWORDSMAN_SCHOOL_POT_01, CAN_USE_HUMAN_SWORD),
            CHECK(RC_SWORDSMAN_SCHOOL_POT_02, CAN_USE_HUMAN_SWORD),
            CHECK(RC_SWORDSMAN_SCHOOL_POT_03, CAN_USE_HUMAN_SWORD),
            CHECK(RC_SWORDSMAN_SCHOOL_POT_04, CAN_USE_HUMAN_SWORD),
            CHECK(RC_SWORDSMAN_SCHOOL_POT_05, CAN_USE_HUMAN_SWORD),
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(WEST_CLOCK_TOWN, 3),              ENTRANCE(SWORDMANS_SCHOOL, 0), true),
        },
    };
    Regions[RR_TERMINA_FIELD_BEFORE_GREAT_BAY_COAST] = RandoRegion{ .name = "Before Great Bay Coast", .sceneId = SCENE_00KEIKOKU,
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(GREAT_BAY_COAST, 0),              ENTRANCE(TERMINA_FIELD, 2), true),
        },
        .connections = {
            CONNECTION(RR_TERMINA_FIELD, CAN_PLAY_SONG(EPONA)),
        },
    };
    Regions[RR_TERMINA_FIELD_BEFORE_PATH_TO_MOUNTAIN_VILLAGE] = RandoRegion{ .sceneId = SCENE_00KEIKOKU,
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(PATH_TO_MOUNTAIN_VILLAGE, 0),     ENTRANCE(TERMINA_FIELD, 3), true),
        },
        .connections = {
            CONNECTION(RR_TERMINA_FIELD, HAS_ITEM(ITEM_BOW)),
        },
    };
    Regions[RR_TERMINA_FIELD_BIO_BABA_GROTTO] = RandoRegion{ .name = "Termina Field Bio Baba", .sceneId = SCENE_KAKUSIANA,
        .checks = {
            CHECK(RC_TERMINA_FIELD_BIO_BABA_GROTTO, CAN_BE_ZORA),
            CHECK(RC_TERMINA_FIELD_BIO_BABA_GROTTO_GRASS_01, true),
            CHECK(RC_TERMINA_FIELD_BIO_BABA_GROTTO_GRASS_02, true),
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(TERMINA_FIELD, 0),                ENTRANCE(GROTTOS, 11), true), // TODO: Grotto mapping
        },
    };
    Regions[RR_TERMINA_FIELD_COW_GROTTO] = RandoRegion{ .name = "Termina Field Cow", .sceneId = SCENE_KAKUSIANA,
        .checks = {
            CHECK(RC_TERMINA_FIELD_COW_BACK, CAN_PLAY_SONG(EPONA)),
            CHECK(RC_TERMINA_FIELD_COW_FRONT, CAN_PLAY_SONG(EPONA)),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_01, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_02, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_03, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_04, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_05, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_06, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_07, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_08, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_09, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_10, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_11, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_12, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_13, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_14, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_15, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_16, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_17, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_18, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_19, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_20, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_21, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_22, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_23, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_24, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_25, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_26, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_27, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_28, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_29, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_30, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_31, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_32, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_33, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_34, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_35, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_36, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_37, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_38, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_39, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_40, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_41, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_42, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_43, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_44, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_45, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_46, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_47, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_48, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_49, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_50, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_51, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_52, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_53, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_54, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_55, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_56, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_57, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_58, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_59, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_60, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_61, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_62, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_63, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_64, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_65, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_66, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_67, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_68, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_69, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_70, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_71, true),
            CHECK(RC_TERMINA_FIELD_COW_GROTTO_GRASS_72, true),
        },
        .connections = {
            CONNECTION(RR_TERMINA_FIELD, true), // TODO: Grotto mapping
        },
    };
    Regions[RR_TERMINA_FIELD_DODONGO_GROTTO] = RandoRegion{ .name = "Termina Field Dodongo", .sceneId = SCENE_KAKUSIANA,
        .checks = {
            CHECK(RC_TERMINA_FIELD_DODONGO_GROTTO_CHEST, CAN_USE_SWORD || CAN_BE_ZORA || CAN_BE_GORON),
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(TERMINA_FIELD, 0),                ENTRANCE(GROTTOS, 7), true), // TODO: Grotto mapping
        },
    };
    Regions[RR_TERMINA_FIELD_GOSSIP_STONE_GROTTO_1] = RandoRegion{ .name = "Termina Field Gossip Stone #1", .sceneId = SCENE_KAKUSIANA,
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(TERMINA_FIELD, 0),                ENTRANCE(GROTTOS, 1), true), // TODO: Grotto mapping
        },
        .events = {
            // TODO: gSaveContext.save.saveInfo.unk_EA0 for gossip stone check
        },
    };
    Regions[RR_TERMINA_FIELD_GOSSIP_STONE_GROTTO_2] = RandoRegion{ .name = "Termina Field Gossip Stone #2", .sceneId = SCENE_KAKUSIANA,
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(TERMINA_FIELD, 0),                ENTRANCE(GROTTOS, 3), true), // TODO: Grotto mapping
        },
        .events = {
            // TODO: gSaveContext.save.saveInfo.unk_EA0 for gossip stone check
        },
    };
    Regions[RR_TERMINA_FIELD_GOSSIP_STONE_GROTTO_3] = RandoRegion{ .name = "Termina Field Gossip Stone #3", .sceneId = SCENE_KAKUSIANA,
        .checks = {
            CHECK(RC_TERMINA_FIELD_GOSSIP_STONE_GROTTO_3_GRASS_01, true),
            CHECK(RC_TERMINA_FIELD_GOSSIP_STONE_GROTTO_3_GRASS_02, true),
            CHECK(RC_TERMINA_FIELD_GOSSIP_STONE_GROTTO_3_GRASS_03, true),
            CHECK(RC_TERMINA_FIELD_GOSSIP_STONE_GROTTO_3_GRASS_04, true),
            CHECK(RC_TERMINA_FIELD_GOSSIP_STONE_GROTTO_3_GRASS_05, true),
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(TERMINA_FIELD, 0),                ENTRANCE(GROTTOS, 0), true), // TODO: Grotto mapping
        },
        .events = {
            // TODO: gSaveContext.save.saveInfo.unk_EA0 for gossip stone check
        },
    };
    Regions[RR_TERMINA_FIELD_GOSSIP_STONE_GROTTO_4] = RandoRegion{ .name = "Termina Field Gossip Stone #4", .sceneId = SCENE_KAKUSIANA,
        .checks = {
            CHECK(RC_TERMINA_FIELD_GOSSIP_STONE_GROTTO_4_GRASS_01, true),
            CHECK(RC_TERMINA_FIELD_GOSSIP_STONE_GROTTO_4_GRASS_02, true),
            CHECK(RC_TERMINA_FIELD_GOSSIP_STONE_GROTTO_4_GRASS_03, true),
            CHECK(RC_TERMINA_FIELD_GOSSIP_STONE_GROTTO_4_GRASS_04, true),
            CHECK(RC_TERMINA_FIELD_GOSSIP_STONE_GROTTO_4_GRASS_05, true),
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(TERMINA_FIELD, 0),                ENTRANCE(GROTTOS, 2), true), // TODO: Grotto mapping
        },
        .events = {
            // TODO: gSaveContext.save.saveInfo.unk_EA0 for gossip stone check
        },
    };
    Regions[RR_TERMINA_FIELD_PEAHAT_GROTTO] = RandoRegion{ .name = "Termina Field Peahat", .sceneId = SCENE_KAKUSIANA,
        .checks = {
            CHECK(RC_TERMINA_FIELD_PEAHAT_GROTTO_CHEST, CAN_USE_SWORD || CAN_BE_ZORA || CAN_BE_GORON),
            CHECK(RC_TERMINA_FIELD_PEAHAT_GROTTO_GRASS_01, true),
            CHECK(RC_TERMINA_FIELD_PEAHAT_GROTTO_GRASS_02, true),
            CHECK(RC_TERMINA_FIELD_PEAHAT_GROTTO_GRASS_03, true),
            CHECK(RC_TERMINA_FIELD_PEAHAT_GROTTO_GRASS_04, true),
            CHECK(RC_TERMINA_FIELD_PEAHAT_GROTTO_GRASS_05, true),
            CHECK(RC_TERMINA_FIELD_PEAHAT_GROTTO_GRASS_06, true),
            CHECK(RC_TERMINA_FIELD_PEAHAT_GROTTO_GRASS_07, true),
            CHECK(RC_TERMINA_FIELD_PEAHAT_GROTTO_GRASS_08, true),
            CHECK(RC_TERMINA_FIELD_PEAHAT_GROTTO_GRASS_09, true),
            CHECK(RC_TERMINA_FIELD_PEAHAT_GROTTO_GRASS_10, true),
            CHECK(RC_TERMINA_FIELD_PEAHAT_GROTTO_GRASS_11, true),
            CHECK(RC_TERMINA_FIELD_PEAHAT_GROTTO_GRASS_12, true),
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(TERMINA_FIELD, 0),                ENTRANCE(GROTTOS, 13), true), // TODO: Grotto mapping
        },
    };
    Regions[RR_TERMINA_FIELD_PILLAR_GROTTO] = RandoRegion{ .name = "Termina Field Pillar", .sceneId = SCENE_KAKUSIANA,
        .checks = {
            CHECK(RC_TERMINA_FIELD_PILLAR_GROTTO_CHEST, true),
            CHECK(RC_TERMINA_FIELD_PILLAR_GROTTO_GRASS_01, true),
            CHECK(RC_TERMINA_FIELD_PILLAR_GROTTO_GRASS_02, true),
            CHECK(RC_TERMINA_FIELD_PILLAR_GROTTO_GRASS_03, true),
            CHECK(RC_TERMINA_FIELD_PILLAR_GROTTO_GRASS_04, true),
            CHECK(RC_TERMINA_FIELD_PILLAR_GROTTO_GRASS_05, true),
            CHECK(RC_TERMINA_FIELD_PILLAR_GROTTO_GRASS_06, true),
            CHECK(RC_TERMINA_FIELD_PILLAR_GROTTO_GRASS_07, true),
            CHECK(RC_TERMINA_FIELD_PILLAR_GROTTO_GRASS_08, true),
            CHECK(RC_TERMINA_FIELD_PILLAR_GROTTO_GRASS_09, true),
            CHECK(RC_TERMINA_FIELD_PILLAR_GROTTO_GRASS_10, true),
            CHECK(RC_TERMINA_FIELD_PILLAR_GROTTO_GRASS_11, true),
            CHECK(RC_TERMINA_FIELD_PILLAR_GROTTO_GRASS_12, true),
            CHECK(RC_TERMINA_FIELD_PILLAR_GROTTO_GRASS_13, true),
            CHECK(RC_TERMINA_FIELD_PILLAR_GROTTO_GRASS_14, true),
        },
        .connections = {
            CONNECTION(RR_TERMINA_FIELD, true), // TODO: Grotto mapping
        },
    };
    Regions[RR_TERMINA_FIELD_SCRUB_GROTTO] = RandoRegion{ .name = "Termina Field Scrub", .sceneId = SCENE_KAKUSIANA,
        .checks = {
            CHECK(RC_TERMINA_FIELD_SCRUB_LARGE_CRATE, true),
            CHECK(RC_TERMINA_FIELD_SCRUB_POT,         true),
            // TODO: Add scrub HP
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(TERMINA_FIELD, 0),                ENTRANCE(GROTTOS, 9), true), // TODO: Grotto mapping
        },
    };
    Regions[RR_TERMINA_FIELD_TALL_GRASS_GROTTO] = RandoRegion{ .name = "Termina Field Tall Grass", .sceneId = SCENE_KAKUSIANA,
        .checks = {
            CHECK(RC_TERMINA_FIELD_TALL_GRASS_GROTTO_CHEST, true),
        },
        .connections = {
            CONNECTION(RR_TERMINA_FIELD, true), // TODO: Grotto mapping
        },
    };
    Regions[RR_TERMINA_FIELD] = RandoRegion{ .sceneId = SCENE_00KEIKOKU,
        .checks = {
            CHECK(RC_TERMINA_FIELD_KAMARO_MASK, CAN_PLAY_SONG(HEALING)),
            CHECK(RC_TERMINA_FIELD_POT, CAN_GROW_BEAN_PLANT),
            CHECK(RC_TERMINA_FIELD_TALL_GRASS_CHEST, true),
            CHECK(RC_TERMINA_FIELD_TREE_STUMP_CHEST, CAN_GROW_BEAN_PLANT || HAS_ITEM(ITEM_HOOKSHOT)),
            CHECK(RC_TERMINA_FIELD_WATER_CHEST, CAN_BE_ZORA),
            CHECK(RC_TERMINA_FIELD_FREESTANDING_RUPEE_01, true), // TODO: Fix duplicate item get...
            CHECK(RC_TERMINA_FIELD_GUAY_RUPEE_DROP_01, CAN_PLAY_SONG(SONATA) || CAN_PLAY_SONG(LULLABY) || CAN_PLAY_SONG(BOSSA_NOVA)),
            CHECK(RC_TERMINA_FIELD_GUAY_RUPEE_DROP_02, CAN_PLAY_SONG(SONATA) || CAN_PLAY_SONG(LULLABY) || CAN_PLAY_SONG(BOSSA_NOVA)),
            CHECK(RC_TERMINA_FIELD_GUAY_RUPEE_DROP_03, CAN_PLAY_SONG(SONATA) || CAN_PLAY_SONG(LULLABY) || CAN_PLAY_SONG(BOSSA_NOVA)),
            CHECK(RC_TERMINA_FIELD_GUAY_RUPEE_DROP_04, CAN_PLAY_SONG(SONATA) || CAN_PLAY_SONG(LULLABY) || CAN_PLAY_SONG(BOSSA_NOVA)),
            CHECK(RC_TERMINA_FIELD_GUAY_RUPEE_DROP_05, CAN_PLAY_SONG(SONATA) || CAN_PLAY_SONG(LULLABY) || CAN_PLAY_SONG(BOSSA_NOVA)),
            CHECK(RC_TERMINA_FIELD_GUAY_RUPEE_DROP_06, CAN_PLAY_SONG(SONATA) || CAN_PLAY_SONG(LULLABY) || CAN_PLAY_SONG(BOSSA_NOVA)),
            CHECK(RC_TERMINA_FIELD_GUAY_RUPEE_DROP_07, CAN_PLAY_SONG(SONATA) || CAN_PLAY_SONG(LULLABY) || CAN_PLAY_SONG(BOSSA_NOVA)),
            CHECK(RC_TERMINA_FIELD_GUAY_RUPEE_DROP_08, CAN_PLAY_SONG(SONATA) || CAN_PLAY_SONG(LULLABY) || CAN_PLAY_SONG(BOSSA_NOVA)),
            CHECK(RC_TERMINA_FIELD_GUAY_RUPEE_DROP_09, CAN_PLAY_SONG(SONATA) || CAN_PLAY_SONG(LULLABY) || CAN_PLAY_SONG(BOSSA_NOVA)),
            CHECK(RC_TERMINA_FIELD_GUAY_RUPEE_DROP_10, CAN_PLAY_SONG(SONATA) || CAN_PLAY_SONG(LULLABY) || CAN_PLAY_SONG(BOSSA_NOVA)),
            CHECK(RC_TERMINA_FIELD_GUAY_RUPEE_DROP_11, CAN_PLAY_SONG(SONATA) || CAN_PLAY_SONG(LULLABY) || CAN_PLAY_SONG(BOSSA_NOVA)),
            CHECK(RC_TERMINA_FIELD_GUAY_RUPEE_DROP_12, CAN_PLAY_SONG(SONATA) || CAN_PLAY_SONG(LULLABY) || CAN_PLAY_SONG(BOSSA_NOVA)),
            CHECK(RC_TERMINA_FIELD_GUAY_RUPEE_DROP_13, CAN_PLAY_SONG(SONATA) || CAN_PLAY_SONG(LULLABY) || CAN_PLAY_SONG(BOSSA_NOVA)),
            CHECK(RC_TERMINA_FIELD_GUAY_RUPEE_DROP_14, CAN_PLAY_SONG(SONATA) || CAN_PLAY_SONG(LULLABY) || CAN_PLAY_SONG(BOSSA_NOVA)),
            CHECK(RC_TERMINA_FIELD_GUAY_RUPEE_DROP_15, CAN_PLAY_SONG(SONATA) || CAN_PLAY_SONG(LULLABY) || CAN_PLAY_SONG(BOSSA_NOVA)),
            CHECK(RC_TERMINA_FIELD_GUAY_RUPEE_DROP_16, CAN_PLAY_SONG(SONATA) || CAN_PLAY_SONG(LULLABY) || CAN_PLAY_SONG(BOSSA_NOVA)),
            CHECK(RC_TERMINA_FIELD_GUAY_RUPEE_DROP_17, CAN_PLAY_SONG(SONATA) || CAN_PLAY_SONG(LULLABY) || CAN_PLAY_SONG(BOSSA_NOVA)),
            CHECK(RC_TERMINA_FIELD_GUAY_RUPEE_DROP_18, CAN_PLAY_SONG(SONATA) || CAN_PLAY_SONG(LULLABY) || CAN_PLAY_SONG(BOSSA_NOVA)),
            CHECK(RC_TERMINA_FIELD_GUAY_RUPEE_DROP_19, CAN_PLAY_SONG(SONATA) || CAN_PLAY_SONG(LULLABY) || CAN_PLAY_SONG(BOSSA_NOVA)),
            CHECK(RC_TERMINA_FIELD_GUAY_RUPEE_DROP_20, CAN_PLAY_SONG(SONATA) || CAN_PLAY_SONG(LULLABY) || CAN_PLAY_SONG(BOSSA_NOVA)),
            CHECK(RC_TERMINA_FIELD_GRASS_01, true),
            CHECK(RC_TERMINA_FIELD_GRASS_02, true),
            CHECK(RC_TERMINA_FIELD_GRASS_03, true),
            CHECK(RC_TERMINA_FIELD_GRASS_04, true),
            CHECK(RC_TERMINA_FIELD_GRASS_05, true),
            CHECK(RC_TERMINA_FIELD_GRASS_06, true),
            CHECK(RC_TERMINA_FIELD_GRASS_07, true),
            CHECK(RC_TERMINA_FIELD_GRASS_08, true),
            CHECK(RC_TERMINA_FIELD_GRASS_09, true),
            CHECK(RC_TERMINA_FIELD_GRASS_10, true),
            CHECK(RC_TERMINA_FIELD_GRASS_11, true),
            CHECK(RC_TERMINA_FIELD_GRASS_12, true),
            CHECK(RC_TERMINA_FIELD_GRASS_13, true),
            CHECK(RC_TERMINA_FIELD_GRASS_14, true),
            CHECK(RC_TERMINA_FIELD_GRASS_15, true),
            CHECK(RC_TERMINA_FIELD_GRASS_16, true),
            CHECK(RC_TERMINA_FIELD_GRASS_17, true),
            CHECK(RC_TERMINA_FIELD_GRASS_18, true),
            CHECK(RC_TERMINA_FIELD_GRASS_19, true),
            CHECK(RC_TERMINA_FIELD_GRASS_20, true),
            CHECK(RC_TERMINA_FIELD_GRASS_21, true),
            CHECK(RC_TERMINA_FIELD_GRASS_22, true),
            CHECK(RC_TERMINA_FIELD_GRASS_23, true),
            CHECK(RC_TERMINA_FIELD_GRASS_24, true),
            CHECK(RC_TERMINA_FIELD_GRASS_25, true),
            CHECK(RC_TERMINA_FIELD_GRASS_26, true),
            CHECK(RC_TERMINA_FIELD_GRASS_27, true),
            CHECK(RC_TERMINA_FIELD_GRASS_28, true),
            CHECK(RC_TERMINA_FIELD_GRASS_29, true),
            CHECK(RC_TERMINA_FIELD_GRASS_30, true),
            CHECK(RC_TERMINA_FIELD_GRASS_31, true),
            CHECK(RC_TERMINA_FIELD_GRASS_32, true),
            CHECK(RC_TERMINA_FIELD_GRASS_33, true),
            CHECK(RC_TERMINA_FIELD_GRASS_34, true),
            CHECK(RC_TERMINA_FIELD_GRASS_35, true),
            CHECK(RC_TERMINA_FIELD_GRASS_36, true),
            CHECK(RC_TERMINA_FIELD_GRASS_37, true),
            CHECK(RC_TERMINA_FIELD_GRASS_38, true),
            CHECK(RC_TERMINA_FIELD_GRASS_39, true),
            CHECK(RC_TERMINA_FIELD_GRASS_40, true),
            CHECK(RC_TERMINA_FIELD_GRASS_41, true),
            CHECK(RC_TERMINA_FIELD_GRASS_42, true),
            CHECK(RC_TERMINA_FIELD_GRASS_43, true),
            CHECK(RC_TERMINA_FIELD_GRASS_44, true),
            CHECK(RC_TERMINA_FIELD_GRASS_45, true),
            CHECK(RC_TERMINA_FIELD_GRASS_46, true),
            CHECK(RC_TERMINA_FIELD_GRASS_47, true),
            CHECK(RC_TERMINA_FIELD_GRASS_48, true),
            CHECK(RC_TERMINA_FIELD_GRASS_49, true),
            CHECK(RC_TERMINA_FIELD_GRASS_50, true),
            CHECK(RC_TERMINA_FIELD_GRASS_51, true),
            CHECK(RC_TERMINA_FIELD_GRASS_52, true),
            CHECK(RC_TERMINA_FIELD_GRASS_53, true),
            CHECK(RC_TERMINA_FIELD_GRASS_54, true),
            CHECK(RC_TERMINA_FIELD_GRASS_55, true),
            CHECK(RC_TERMINA_FIELD_GRASS_56, true),
            CHECK(RC_TERMINA_FIELD_GRASS_57, true),
            CHECK(RC_TERMINA_FIELD_GRASS_58, true),
            CHECK(RC_TERMINA_FIELD_GRASS_59, true),
            CHECK(RC_TERMINA_FIELD_GRASS_60, true),
            CHECK(RC_TERMINA_FIELD_GRASS_61, true),
            CHECK(RC_TERMINA_FIELD_GRASS_62, true),
            CHECK(RC_TERMINA_FIELD_GRASS_63, true),
            CHECK(RC_TERMINA_FIELD_GRASS_64, true),
            CHECK(RC_TERMINA_FIELD_GRASS_65, true),
            CHECK(RC_TERMINA_FIELD_GRASS_66, true),
            CHECK(RC_TERMINA_FIELD_GRASS_67, true),
            CHECK(RC_TERMINA_FIELD_GRASS_68, true),
            CHECK(RC_TERMINA_FIELD_GRASS_69, true),
            CHECK(RC_TERMINA_FIELD_GRASS_70, true),
            CHECK(RC_TERMINA_FIELD_GRASS_71, true),
            CHECK(RC_TERMINA_FIELD_GRASS_72, true),
            CHECK(RC_TERMINA_FIELD_GRASS_73, true),
            CHECK(RC_TERMINA_FIELD_GRASS_74, true),
            CHECK(RC_TERMINA_FIELD_GRASS_75, true),
            CHECK(RC_TERMINA_FIELD_GRASS_76, true),
            CHECK(RC_TERMINA_FIELD_GRASS_77, true),
            CHECK(RC_TERMINA_FIELD_GRASS_78, true),
            CHECK(RC_TERMINA_FIELD_GRASS_79, true),
            CHECK(RC_TERMINA_FIELD_GRASS_80, true),
            CHECK(RC_TERMINA_FIELD_GRASS_81, true),
            CHECK(RC_TERMINA_FIELD_GRASS_82, true),
            CHECK(RC_TERMINA_FIELD_GRASS_83, true),
            CHECK(RC_TERMINA_FIELD_GRASS_84, true),
            CHECK(RC_TERMINA_FIELD_GRASS_85, true),
            CHECK(RC_TERMINA_FIELD_GRASS_86, true),
            CHECK(RC_TERMINA_FIELD_GRASS_87, true),
            CHECK(RC_TERMINA_FIELD_GRASS_88, true),
            CHECK(RC_TERMINA_FIELD_GRASS_89, true),
            CHECK(RC_TERMINA_FIELD_GRASS_90, true),
            CHECK(RC_TERMINA_FIELD_GRASS_91, true),
            CHECK(RC_TERMINA_FIELD_GRASS_92, true),
            CHECK(RC_TERMINA_FIELD_GRASS_93, true),
            CHECK(RC_TERMINA_FIELD_GRASS_94, true),
            CHECK(RC_TERMINA_FIELD_GRASS_95, true),
            CHECK(RC_TERMINA_FIELD_GRASS_96, true),
            CHECK(RC_TERMINA_FIELD_GRASS_97, true),
            CHECK(RC_TERMINA_FIELD_GRASS_98, true),
            CHECK(RC_TERMINA_FIELD_GRASS_99, true),
            CHECK(RC_TERMINA_FIELD_GRASS_100, true),
            CHECK(RC_TERMINA_FIELD_GRASS_101, true),
            CHECK(RC_TERMINA_FIELD_GRASS_102, true),
            CHECK(RC_TERMINA_FIELD_GRASS_103, true),
            CHECK(RC_TERMINA_FIELD_GRASS_104, true),
            CHECK(RC_TERMINA_FIELD_GRASS_105, true),
            CHECK(RC_TERMINA_FIELD_GRASS_106, true),
            CHECK(RC_TERMINA_FIELD_GRASS_107, true),
            CHECK(RC_TERMINA_FIELD_GRASS_108, true),
            CHECK(RC_TERMINA_FIELD_GRASS_109, true),
            CHECK(RC_TERMINA_FIELD_GRASS_110, true),
            CHECK(RC_TERMINA_FIELD_GRASS_111, true),
            CHECK(RC_TERMINA_FIELD_GRASS_112, true),
            CHECK(RC_TERMINA_FIELD_GRASS_113, true),
            CHECK(RC_TERMINA_FIELD_GRASS_114, true),
            CHECK(RC_TERMINA_FIELD_GRASS_115, true),
            CHECK(RC_TERMINA_FIELD_GRASS_116, true),
            CHECK(RC_TERMINA_FIELD_GRASS_117, true),
            CHECK(RC_TERMINA_FIELD_GRASS_118, true),
            CHECK(RC_TERMINA_FIELD_GRASS_119, true),
            CHECK(RC_TERMINA_FIELD_GRASS_120, true),
            CHECK(RC_TERMINA_FIELD_GRASS_121, true),
            CHECK(RC_TERMINA_FIELD_GRASS_122, true),
            CHECK(RC_TERMINA_FIELD_GRASS_123, true),
            CHECK(RC_TERMINA_FIELD_GRASS_124, true),
            CHECK(RC_TERMINA_FIELD_GRASS_125, true),
            CHECK(RC_TERMINA_FIELD_GRASS_126, true),
            CHECK(RC_TERMINA_FIELD_GRASS_127, true),
            CHECK(RC_TERMINA_FIELD_GRASS_128, true),
            CHECK(RC_TERMINA_FIELD_GRASS_129, true),
            CHECK(RC_TERMINA_FIELD_GRASS_130, true),
            CHECK(RC_TERMINA_FIELD_GRASS_131, true),
            CHECK(RC_TERMINA_FIELD_GRASS_132, true),
            CHECK(RC_TERMINA_FIELD_GRASS_133, true),
            CHECK(RC_TERMINA_FIELD_GRASS_134, true),
            CHECK(RC_TERMINA_FIELD_GRASS_135, true),
            CHECK(RC_TERMINA_FIELD_GRASS_136, true),
            CHECK(RC_TERMINA_FIELD_GRASS_137, true),
            CHECK(RC_TERMINA_FIELD_GRASS_138, true),
            CHECK(RC_TERMINA_FIELD_GRASS_139, true),
            CHECK(RC_TERMINA_FIELD_GRASS_140, true),
            CHECK(RC_TERMINA_FIELD_GRASS_141, true),
            CHECK(RC_TERMINA_FIELD_GRASS_142, true),
            CHECK(RC_TERMINA_FIELD_GRASS_143, true),
            CHECK(RC_TERMINA_FIELD_GRASS_144, true),
            CHECK(RC_TERMINA_FIELD_GRASS_145, true),
            CHECK(RC_TERMINA_FIELD_GRASS_146, true),
            CHECK(RC_TERMINA_FIELD_GRASS_147, true),
            CHECK(RC_TERMINA_FIELD_GRASS_148, true),
            CHECK(RC_TERMINA_FIELD_GRASS_149, true),
            CHECK(RC_TERMINA_FIELD_GRASS_150, true),
            CHECK(RC_TERMINA_FIELD_GRASS_151, true),
            CHECK(RC_TERMINA_FIELD_GRASS_152, true),
            CHECK(RC_TERMINA_FIELD_GRASS_153, true),
            CHECK(RC_TERMINA_FIELD_GRASS_154, true),
            CHECK(RC_TERMINA_FIELD_GRASS_155, true),
            CHECK(RC_TERMINA_FIELD_GRASS_156, true),
            CHECK(RC_TERMINA_FIELD_GRASS_157, true),
            CHECK(RC_TERMINA_FIELD_GRASS_158, true),
            CHECK(RC_TERMINA_FIELD_GRASS_159, true),
            CHECK(RC_TERMINA_FIELD_GRASS_160, true),
            CHECK(RC_TERMINA_FIELD_GRASS_161, true),
            CHECK(RC_TERMINA_FIELD_GRASS_162, true),
            CHECK(RC_TERMINA_FIELD_GRASS_163, true),
            CHECK(RC_TERMINA_FIELD_GRASS_164, true),
            CHECK(RC_TERMINA_FIELD_GRASS_165, true),
            CHECK(RC_TERMINA_FIELD_GRASS_166, true),
            CHECK(RC_TERMINA_FIELD_GRASS_167, true),
            CHECK(RC_TERMINA_FIELD_GRASS_168, true),
            CHECK(RC_TERMINA_FIELD_GRASS_169, true),
            CHECK(RC_TERMINA_FIELD_GRASS_170, true),
            CHECK(RC_TERMINA_FIELD_GRASS_171, true),
            CHECK(RC_TERMINA_FIELD_GRASS_172, true),
            CHECK(RC_TERMINA_FIELD_GRASS_173, true),
            CHECK(RC_TERMINA_FIELD_GRASS_174, true),
            CHECK(RC_TERMINA_FIELD_GRASS_175, true),
            CHECK(RC_TERMINA_FIELD_GRASS_176, true),
            CHECK(RC_TERMINA_FIELD_GRASS_177, true),
            CHECK(RC_TERMINA_FIELD_GRASS_178, true),
            CHECK(RC_TERMINA_FIELD_GRASS_179, true),
            CHECK(RC_TERMINA_FIELD_GRASS_180, true),
            CHECK(RC_TERMINA_FIELD_GRASS_181, true),
            CHECK(RC_TERMINA_FIELD_GRASS_182, true),
            CHECK(RC_TERMINA_FIELD_GRASS_183, true),
            CHECK(RC_TERMINA_FIELD_GRASS_184, true),
            CHECK(RC_TERMINA_FIELD_GRASS_185, true),
            CHECK(RC_TERMINA_FIELD_GRASS_186, true),
            CHECK(RC_TERMINA_FIELD_GRASS_187, true),
            CHECK(RC_TERMINA_FIELD_GRASS_188, true),
            CHECK(RC_TERMINA_FIELD_GRASS_189, true),
            CHECK(RC_TERMINA_FIELD_GRASS_190, true),
            CHECK(RC_TERMINA_FIELD_GRASS_191, true),
            CHECK(RC_TERMINA_FIELD_GRASS_192, true),
            CHECK(RC_TERMINA_FIELD_GRASS_193, true),
            CHECK(RC_TERMINA_FIELD_GRASS_194, true),
            CHECK(RC_TERMINA_FIELD_GRASS_195, true),
            CHECK(RC_TERMINA_FIELD_GRASS_196, true),
            CHECK(RC_TERMINA_FIELD_GRASS_197, true),
            CHECK(RC_TERMINA_FIELD_GRASS_198, true),
            CHECK(RC_TERMINA_FIELD_GRASS_199, true),
            CHECK(RC_TERMINA_FIELD_GRASS_200, true),
            CHECK(RC_TERMINA_FIELD_GRASS_201, true),
            CHECK(RC_TERMINA_FIELD_GRASS_202, true),
            CHECK(RC_TERMINA_FIELD_GRASS_203, true),
            CHECK(RC_TERMINA_FIELD_GRASS_204, true),
            CHECK(RC_TERMINA_FIELD_GRASS_205, true),
            CHECK(RC_TERMINA_FIELD_GRASS_206, true),
            CHECK(RC_TERMINA_FIELD_GRASS_207, true),
            CHECK(RC_TERMINA_FIELD_GRASS_208, true),
            CHECK(RC_TERMINA_FIELD_GRASS_209, true),
            CHECK(RC_TERMINA_FIELD_GRASS_210, true),
            CHECK(RC_TERMINA_FIELD_GRASS_211, true),
            CHECK(RC_TERMINA_FIELD_GRASS_212, true),
            CHECK(RC_TERMINA_FIELD_GRASS_213, true),
            CHECK(RC_TERMINA_FIELD_GRASS_214, true),
            CHECK(RC_TERMINA_FIELD_GRASS_215, true),
            CHECK(RC_TERMINA_FIELD_GRASS_216, true),

        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(GROTTOS, 0),                      ENTRANCE(TERMINA_FIELD, 0), CAN_USE_EXPLOSIVE || CAN_BE_GORON), // TODO: Grotto mapping Gossip Stone #3
            EXIT(ENTRANCE(GROTTOS, 1),                      ENTRANCE(TERMINA_FIELD, 0), true), // TODO: Grotto mapping Gossip Stone #1
            EXIT(ENTRANCE(GROTTOS, 2),                      ENTRANCE(TERMINA_FIELD, 0), true), // TODO: Grotto mapping Gossip Stone #4
            EXIT(ENTRANCE(GROTTOS, 3),                      ENTRANCE(TERMINA_FIELD, 0), true), // TODO: Grotto mapping Gossip Stone #2
            EXIT(ENTRANCE(GROTTOS, 7),                      ENTRANCE(TERMINA_FIELD, 0), true), // TODO: Grotto mapping Dodongo
            EXIT(ENTRANCE(GROTTOS, 9),                      ENTRANCE(TERMINA_FIELD, 0), true), // TODO: Grotto mapping Scrub
            EXIT(ENTRANCE(GROTTOS, 11),                     ENTRANCE(TERMINA_FIELD, 0), CAN_USE_EXPLOSIVE || CAN_BE_GORON), // TODO: Grotto mapping bio baba grotto
            EXIT(ENTRANCE(GROTTOS, 13),                     ENTRANCE(TERMINA_FIELD, 0), true), // TODO: Grotto mapping peahat grotto
            EXIT(ENTRANCE(WEST_CLOCK_TOWN, 0),              ENTRANCE(TERMINA_FIELD, 0), true),
            EXIT(ENTRANCE(ROAD_TO_SOUTHERN_SWAMP, 0),       ENTRANCE(TERMINA_FIELD, 1), true),
            EXIT(ENTRANCE(ROAD_TO_IKANA, 0),                ENTRANCE(TERMINA_FIELD, 4), true),
            EXIT(ENTRANCE(MILK_ROAD, 0),                    ENTRANCE(TERMINA_FIELD, 5), true),
            EXIT(ENTRANCE(SOUTH_CLOCK_TOWN, 1),             ENTRANCE(TERMINA_FIELD, 6), true),
            EXIT(ENTRANCE(EAST_CLOCK_TOWN, 0),              ENTRANCE(TERMINA_FIELD, 7), true),
            EXIT(ENTRANCE(NORTH_CLOCK_TOWN, 0),             ENTRANCE(TERMINA_FIELD, 8), true),
        },
        .connections = {
            CONNECTION(RR_TERMINA_FIELD_BEFORE_PATH_TO_MOUNTAIN_VILLAGE, HAS_ITEM(ITEM_BOW)),
            CONNECTION(RR_TERMINA_FIELD_BEFORE_GREAT_BAY_COAST, CAN_PLAY_SONG(EPONA)),
            CONNECTION(RR_ASTRAL_OBSERVATORY_OUTSIDE, CAN_BE_DEKU),
            CONNECTION(RR_TERMINA_FIELD_COW_GROTTO, CAN_USE_EXPLOSIVE), // TODO: Grotto mapping
            CONNECTION(RR_TERMINA_FIELD_PILLAR_GROTTO, true), // TODO: Grotto mapping
            CONNECTION(RR_TERMINA_FIELD_TALL_GRASS_GROTTO, true), // TODO: Grotto mapping
        },
    };
    Regions[RR_TOWN_DEKU_PLAYGROUND] = RandoRegion{ .sceneId = SCENE_DEKUTES,
        .checks = {
            CHECK(RC_DEKU_PLAYGROUND_ALL_DAYS, CAN_BE_DEKU),
            CHECK(RC_DEKU_PLAYGROUND_ANY_DAY, CAN_BE_DEKU),
            CHECK(RC_DEKU_PLAYGROUND_DAY_1_RUPEE_01, CAN_BE_DEKU),
            CHECK(RC_DEKU_PLAYGROUND_DAY_1_RUPEE_02, CAN_BE_DEKU),
            CHECK(RC_DEKU_PLAYGROUND_DAY_1_RUPEE_03, CAN_BE_DEKU),
            CHECK(RC_DEKU_PLAYGROUND_DAY_1_RUPEE_04, CAN_BE_DEKU),
            CHECK(RC_DEKU_PLAYGROUND_DAY_1_RUPEE_05, CAN_BE_DEKU),
            CHECK(RC_DEKU_PLAYGROUND_DAY_1_RUPEE_06, CAN_BE_DEKU),
            CHECK(RC_DEKU_PLAYGROUND_DAY_2_RUPEE_01, CAN_BE_DEKU),
            CHECK(RC_DEKU_PLAYGROUND_DAY_2_RUPEE_02, CAN_BE_DEKU),
            CHECK(RC_DEKU_PLAYGROUND_DAY_2_RUPEE_03, CAN_BE_DEKU),
            CHECK(RC_DEKU_PLAYGROUND_DAY_2_RUPEE_04, CAN_BE_DEKU),
            CHECK(RC_DEKU_PLAYGROUND_DAY_2_RUPEE_05, CAN_BE_DEKU),
            CHECK(RC_DEKU_PLAYGROUND_DAY_2_RUPEE_06, CAN_BE_DEKU),
            CHECK(RC_DEKU_PLAYGROUND_DAY_3_RUPEE_01, CAN_BE_DEKU),
            CHECK(RC_DEKU_PLAYGROUND_DAY_3_RUPEE_02, CAN_BE_DEKU),
            CHECK(RC_DEKU_PLAYGROUND_DAY_3_RUPEE_03, CAN_BE_DEKU),
            CHECK(RC_DEKU_PLAYGROUND_DAY_3_RUPEE_04, CAN_BE_DEKU),
            CHECK(RC_DEKU_PLAYGROUND_DAY_3_RUPEE_05, CAN_BE_DEKU),
            CHECK(RC_DEKU_PLAYGROUND_DAY_3_RUPEE_06, CAN_BE_DEKU),
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(NORTH_CLOCK_TOWN, 4),             ENTRANCE(DEKU_SCRUB_PLAYGROUND, 0), true),
        },
    };
    Regions[RR_TOWN_SHOOTING_GALLERY] = RandoRegion{ .sceneId = SCENE_SYATEKI_MIZU,
        .checks = {
            CHECK(RC_CLOCK_TOWN_EAST_SHOOTING_GALLERY_HIGH_SCORE, HAS_ITEM(ITEM_BOW)),
            CHECK(RC_CLOCK_TOWN_EAST_SHOOTING_GALLERY_PERFECT_SCORE, HAS_ITEM(ITEM_BOW)),
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(EAST_CLOCK_TOWN, 8),              ENTRANCE(TOWN_SHOOTING_GALLERY, 0), true),
        },
    };
    Regions[RR_TRADING_POST] = RandoRegion{ .sceneId = SCENE_8ITEMSHOP,
        .checks = {
            CHECK(RC_CLOCK_TOWN_WEST_TRADING_POST_POT, true), // Note : Goron has to sidehop to get up.
            CHECK(RC_TRADING_POST_SHOP_ITEM_01, CAN_AFFORD(RC_TRADING_POST_SHOP_ITEM_01)),
            CHECK(RC_TRADING_POST_SHOP_ITEM_02, CAN_AFFORD(RC_TRADING_POST_SHOP_ITEM_02)),
            CHECK(RC_TRADING_POST_SHOP_ITEM_03, CAN_AFFORD(RC_TRADING_POST_SHOP_ITEM_03)),
            CHECK(RC_TRADING_POST_SHOP_ITEM_04, CAN_AFFORD(RC_TRADING_POST_SHOP_ITEM_04)),
            CHECK(RC_TRADING_POST_SHOP_ITEM_05, CAN_AFFORD(RC_TRADING_POST_SHOP_ITEM_05)),
            CHECK(RC_TRADING_POST_SHOP_ITEM_06, CAN_AFFORD(RC_TRADING_POST_SHOP_ITEM_06)),
            CHECK(RC_TRADING_POST_SHOP_ITEM_07, CAN_AFFORD(RC_TRADING_POST_SHOP_ITEM_07)),
            CHECK(RC_TRADING_POST_SHOP_ITEM_08, CAN_AFFORD(RC_TRADING_POST_SHOP_ITEM_08)),
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(WEST_CLOCK_TOWN, 5),              ENTRANCE(TRADING_POST, 0), true),
        
        },
    };
    Regions[RR_TREASURE_SHOP] = RandoRegion{ .sceneId = SCENE_TAKARAYA,
        .checks = {
            // TODO : Add check for each form(minus FD)
        },
        .exits = { //     TO                                         FROM
            EXIT(ENTRANCE(EAST_CLOCK_TOWN, 4),              ENTRANCE(TREASURE_CHEST_SHOP, 0), true),
        },
    };
}, {});
// clang-format on
