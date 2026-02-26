from __future__ import annotations

import logging
import pkgutil
from typing import Any, ClassVar

import orjson

from BaseClasses import Tutorial
from worlds.AutoWorld import WebWorld, World
from settings import Group, Bool

from .Enums import Items  # IMPORTANT: SoH-style string Enum
from .Items import MM2ShipItem, item_data_table, item_table, item_name_groups
from .Locations import location_table, location_name_groups
from .Options import MM2ShipOptions, mm2ship_option_groups
from .Regions import create_regions_and_locations

logger = logging.getLogger("MM2SHIP")


class MM2ShipWebWorld(WebWorld):
    theme = "ice"  # placeholder
    option_groups = mm2ship_option_groups

    setup_en = Tutorial(
        tutorial_name="Start Guide",
        description="A guide to playing 2 Ship 2 Harkinian (MM) in Archipelago.",
        language="English",
        file_name="guide_en.md",
        link="guide/en",
        authors=["ItsHeckinPat"],
    )

    tutorials = [setup_en]
    game_info_languages = ["en"]


class MM2ShipSettings(Group):
    class AllowTrueNoLogic(Bool):
        """
        Host safeguard for enabling a future true_no_logic option.
        """

    allow_true_no_logic: AllowTrueNoLogic | bool = False


class MM2ShipWorld(World):
    """2 Ship 2 Harkinian (Majora's Mask)"""

    game = "2 Ship 2 Harkinian (MM)"
    web = MM2ShipWebWorld()

    options: MM2ShipOptions
    options_dataclass = MM2ShipOptions
    settings: ClassVar[MM2ShipSettings]

    # name -> id mappings (dict[str, int])
    location_name_to_id = location_table
    item_name_to_id = item_table

    # optional groups
    item_name_groups = item_name_groups
    location_name_groups = location_name_groups

    def __init__(self, multiworld, player):
        super().__init__(multiworld, player)

        apworld_manifest = orjson.loads(
            pkgutil.get_data(__name__, "archipelago.json").decode("utf-8")
        )
        self.apworld_version: str = apworld_manifest.get("world_version", "0.0.0")

        # local tracking (used by ItemPool.py)
        self.item_pool: list[MM2ShipItem] = []

        # Shop prices (location_name -> price in rupees)
        self.shop_prices: dict[str, int] = {}

    def generate_early(self) -> None:
        # Build filtered location table — inactive locations are removed from the world
        # entirely so the spoiler log is clean and AP never places items there.
        # The C++ resync loop applies the same option-based filter before sending
        # LocationChecks, so the server will never receive an ID it doesn't know about.
        from .Locations import location_data_table
        from .Regions import location_should_be_included

        filtered_location_table = {
            loc.value: addr
            for loc, addr in location_data_table.items()
            if addr is not None and location_should_be_included(self, loc)
        }
        self.__class__.location_name_to_id = filtered_location_table

        # Generate random prices for shop and tingle shop locations
        from .ShopLocations import all_shop_locations

        for shop_location in all_shop_locations:
            # Random price between 0-200 rupees (matching 2Ship's range)
            price = self.random.randint(0, 200)
            # Use the RC_ enum name (e.g., "RC_BOMB_SHOP_ITEM_01") as the key
            # to match what the C++ code expects in RandoStaticCheck.name
            location_key = f"RC_{shop_location.name}"
            self.shop_prices[location_key] = price

    def create_regions(self) -> None:
        create_regions_and_locations(self)

        # SoH-style safety normalization
        for location in self.get_locations():
            location.name = str(location.name)
        for region in self.get_regions():
            region.name = str(region.name)

    def create_item(self, name: str, create_as_event: bool = False) -> MM2ShipItem:
        from .Enums import Items as ItemsEnum  # local import prevents name collisions
        from .Items import item_data_table

        item_enum = ItemsEnum(name)
        entry = item_data_table[item_enum]

        return MM2ShipItem(
            str(item_enum.value),
            entry.classification,
            None if create_as_event else entry.item_id,
            self.player,
        )

    def create_items(self) -> None:
        """
        Build the item pool (no logic for now).
        This is where options start to actually have effects.
        """
        from .ItemPool import create_item_pool, create_plentiful_and_trap_items

        create_item_pool(self)
        create_plentiful_and_trap_items(self)

    def get_filler_item_name(self) -> str:
        """
        Called by Archipelago to automatically fill remaining locations.
        """
        from .ItemPool import get_filler_item
        return get_filler_item(self)


    def set_rules(self) -> None:
        # Victory condition: collect the Victory event (triggered by C++ code when beating Majora or collecting triforce pieces)
        self.multiworld.completion_condition[self.player] = lambda state: state.has("Victory", self.player)
        if self.options.true_no_logic.value:
            return


    def fill_slot_data(self) -> dict[str, Any]:
        slot_data = {
            "apworld_version": self.apworld_version,
            # Access options
            "access_dungeons": self.options.access_dungeons.value,
            "access_majora_masks_count": self.options.access_majora_masks_count.value,
            "access_majora_remains_count": self.options.access_majora_remains_count.value,
            "access_moon_masks_count": self.options.access_moon_masks_count.value,
            "access_moon_remains_count": self.options.access_moon_remains_count.value,
            "access_trials": self.options.access_trials.value,
            # Clock options
            "clock_shuffle": self.options.clock_shuffle.value,
            "clock_shuffle_progressive": self.options.clock_shuffle_progressive.value,
            "clock_terminal_time": self.options.clock_terminal_time.value,
            # Exclusion options
            "exclude_termina_field_grass": self.options.exclude_termina_field_grass.value,
            "exclude_cow_grotto_grass": self.options.exclude_cow_grotto_grass.value,
            # Hints
            "hints_boss_remains": self.options.hints_boss_remains.value,
            "hints_gossip_stones": self.options.hints_gossip_stones.value,
            "hints_hookshot": self.options.hints_hookshot.value,
            "hints_oath_to_order": self.options.hints_oath_to_order.value,
            "hints_purchaseable": self.options.hints_purchaseable.value,
            "hints_song_of_soaring": self.options.hints_song_of_soaring.value,
            "hints_spider_houses": self.options.hints_spider_houses.value,
            # Logic
            "logic": self.options.logic.value,
            "plentiful_items": self.options.plentiful_items.value,
            # Shuffle options
            "shuffle_barrel_drops": self.options.shuffle_barrel_drops.value,
            "shuffle_boss_remains": self.options.shuffle_boss_remains.value,
            "shuffle_boss_souls": self.options.shuffle_boss_souls.value,
            "shuffle_cows": self.options.shuffle_cows.value,
            "shuffle_crate_drops": self.options.shuffle_crate_drops.value,
            "shuffle_enemy_drops": self.options.shuffle_enemy_drops.value,
            "shuffle_enemy_souls": self.options.shuffle_enemy_souls.value,
            "shuffle_freestanding_items": self.options.shuffle_freestanding_items.value,
            "shuffle_frogs": self.options.shuffle_frogs.value,
            "shuffle_gold_skulltulas": self.options.shuffle_gold_skulltulas.value,
            "shuffle_grass_drops": self.options.shuffle_grass_drops.value,
            "shuffle_ocarina_buttons": self.options.shuffle_ocarina_buttons.value,
            "shuffle_ocarina": self.options.shuffle_ocarina.value,
            "shuffle_owl_statues": self.options.shuffle_owl_statues.value,
            "shuffle_pot_drops": self.options.shuffle_pot_drops.value,
            "shuffle_shield": self.options.shuffle_shield.value,
            "shuffle_shops": self.options.shuffle_shops.value,
            "shuffle_snowball_drops": self.options.shuffle_snowball_drops.value,
            "shuffle_song_double_time": self.options.shuffle_song_double_time.value,
            "shuffle_song_inverted_time": self.options.shuffle_song_inverted_time.value,
            "shuffle_song_saria": self.options.shuffle_song_saria.value,
            "shuffle_song_sun": self.options.shuffle_song_sun.value,
            "shuffle_song_time": self.options.shuffle_song_time.value,
            "shuffle_sword": self.options.shuffle_sword.value,
            "shuffle_swim": self.options.shuffle_swim.value,
            "shuffle_tingle_shops": self.options.shuffle_tingle_shops.value,
            "shuffle_traps": self.options.shuffle_traps.value,
            "shuffle_tree_drops": self.options.shuffle_tree_drops.value,
            "shuffle_triforce_pieces": self.options.shuffle_triforce_pieces.value,
            # Token/piece counts
            "skulltula_tokens_max": self.options.skulltula_tokens_max.value,
            "skulltula_tokens_required": self.options.skulltula_tokens_required.value,
            "stray_fairies_max": self.options.stray_fairies_max.value,
            "stray_fairies_required": self.options.stray_fairies_required.value,
            "triforce_pieces_max": self.options.triforce_pieces_max.value,
            "triforce_pieces_required": self.options.triforce_pieces_required.value,
            # Starting items
            "starting_bunny_hood": self.options.starting_bunny_hood.value,
            "starting_consumables": self.options.starting_consumables.value,
            "starting_health": self.options.starting_health.value,
            "starting_maps_and_compasses": self.options.starting_maps_and_compasses.value,
            "starting_rupees": self.options.starting_rupees.value,
            # Other
            "trap_amount": self.options.trap_amount.value,
            # Shop prices (dict[location_name, price])
            "shop_prices": self.shop_prices,
        }
        return slot_data


__all__ = [
    "MM2ShipWorld",
    "MM2ShipWebWorld",
    "MM2ShipSettings",
]
