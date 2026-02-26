"""
Shop and Tingle shop locations that need randomized prices
"""

from .Enums import Locations

# All shop locations (RCTYPE_SHOP)
shop_locations: set[Locations] = {
    Locations.BOMB_SHOP_ITEM_01,
    Locations.BOMB_SHOP_ITEM_02,
    Locations.BOMB_SHOP_ITEM_03,
    Locations.BOMB_SHOP_ITEM_04_OR_CURIOSITY_SHOP_ITEM,
    Locations.CURIOSITY_SHOP_SPECIAL_ITEM,
    Locations.GORMAN_MILK_PURCHASE,
    Locations.GORON_SHOP_ITEM_01,
    Locations.GORON_SHOP_ITEM_02,
    Locations.GORON_SHOP_ITEM_03,
    Locations.HAGS_POTION_SHOP_ITEM_01,
    Locations.HAGS_POTION_SHOP_ITEM_02,
    Locations.HAGS_POTION_SHOP_ITEM_03,
    Locations.MILK_BAR_PURCHASE_CHATEAU,
    Locations.MILK_BAR_PURCHASE_MILK,
    Locations.TRADING_POST_SHOP_ITEM_01,
    Locations.TRADING_POST_SHOP_ITEM_02,
    Locations.TRADING_POST_SHOP_ITEM_03,
    Locations.TRADING_POST_SHOP_ITEM_04,
    Locations.TRADING_POST_SHOP_ITEM_05,
    Locations.TRADING_POST_SHOP_ITEM_06,
    Locations.TRADING_POST_SHOP_ITEM_07,
    Locations.TRADING_POST_SHOP_ITEM_08,
    Locations.ZORA_SHOP_ITEM_01,
    Locations.ZORA_SHOP_ITEM_02,
    Locations.ZORA_SHOP_ITEM_03,
}

# All tingle shop locations (RCTYPE_TINGLE_SHOP)
tingle_shop_locations: set[Locations] = {
    Locations.CLOCK_TOWN_NORTH_TINGLE_MAP_01,
    Locations.CLOCK_TOWN_NORTH_TINGLE_MAP_02,
    Locations.GREAT_BAY_COAST_TINGLE_MAP_01,
    Locations.GREAT_BAY_COAST_TINGLE_MAP_02,
    Locations.IKANA_CANYON_TINGLE_MAP_01,
    Locations.IKANA_CANYON_TINGLE_MAP_02,
    Locations.MILK_ROAD_TINGLE_MAP_01,
    Locations.MILK_ROAD_TINGLE_MAP_02,
    Locations.ROAD_TO_SOUTHERN_SWAMP_TINGLE_MAP_01,
    Locations.ROAD_TO_SOUTHERN_SWAMP_TINGLE_MAP_02,
    Locations.TWIN_ISLANDS_TINGLE_MAP_01,
    Locations.TWIN_ISLANDS_TINGLE_MAP_02,
}

# Combined set of all shop/tingle locations that need prices
all_shop_locations: set[Locations] = shop_locations | tingle_shop_locations
