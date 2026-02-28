# ShopItems.py
from __future__ import annotations

from typing import TYPE_CHECKING

# Keep these imports in place so the file shape stays compatible with SoH-style usage.
# They may be unused during scaffolding.
from worlds.generic.Rules import add_rule  # noqa: F401
from Fill import fill_restrictive  # noqa: F401
from BaseClasses import CollectionState  # noqa: F401

from .LogicHelpers import rule_wrapper, can_afford  # noqa: F401
from .Enums import Regions, Items  # noqa: F401

if TYPE_CHECKING:
    from . import MM2ShipWorld


# -----------------------------
# Scaffolding tables (empty)
# -----------------------------

# In SoH this maps shop-buy items to their vanilla rupee prices.
# MM2Ship shops differ, so keep this empty until mapping is defined.
vanilla_shop_prices: dict[Items, int] = {}

# In SoH these map AP location names -> vanilla shop items.
# Keep empty until you define MM2Ship shop slot locations.
all_shop_locations: list[tuple[Regions, dict[str, Items]]] = []

# Used by SoH to prefill "vanilla" items into non-shuffled slots.
# Keep empty until you actually support shuffling shops.
vanilla_items_to_add: list[list[Items]] = []


# -----------------------------
# Framework functions
# -----------------------------

def fill_shop_items(world: "MM2ShipWorld") -> None:
    """
    Framework-first scaffolding:
    - If/when shop shuffle is implemented, this will place vanilla items and randomize others.
    - For now, safe no-op that initializes expected fields.
    """
    _ensure_shop_state(world)
    return


def no_shop_shuffle(world: "MM2ShipWorld") -> None:
    """
    Framework-first scaffolding:
    - SoH uses this to place everything as plain vanilla.
    - MM2Ship: not implemented yet.
    """
    _ensure_shop_state(world)
    return


def generate_scrub_prices(world: "MM2ShipWorld") -> None:
    """
    Framework-first scaffolding:
    - SoH generates prices for scrub locations.
    - MM2Ship: not implemented yet.
    """
    _ensure_shop_state(world)
    return


def create_random_price(min_price: int, max_price: int, world: "MM2ShipWorld") -> int:
    """
    Keep the helper (structure is useful later), even if it's unused right now.
    """
    if min_price == max_price:
        price = min_price
    else:
        # Prefer world.random if present; fall back to multiworld RNG if needed.
        rng = getattr(world, "random", None) or world.multiworld.random
        price = rng.randrange(min_price, max_price)

    price = price - (price % 5)
    return price


def set_price_rules(world: "MM2ShipWorld") -> None:
    """
    Framework-first scaffolding:
    - SoH adds rupee affordability rules to shop/scrub locations.
    - MM2Ship: no shop locations exist yet, so no-op.
    """
    _ensure_shop_state(world)
    return


# -----------------------------
# Internal helpers
# -----------------------------

def _ensure_shop_state(world: "MM2ShipWorld") -> None:
    """
    Ensure fields exist so other code can read them safely,
    even before shops are implemented.
    """
    if not hasattr(world, "shop_prices") or world.shop_prices is None:
        world.shop_prices = {}

    if not hasattr(world, "shop_vanilla_items") or world.shop_vanilla_items is None:
        world.shop_vanilla_items = {}

    if not hasattr(world, "scrub_prices") or world.scrub_prices is None:
        world.scrub_prices = {}

    # Some worlds use this to decide whether to pull through passthrough data (UT).
    if not hasattr(world, "using_ut"):
        world.using_ut = False

    if not hasattr(world, "passthrough") or world.passthrough is None:
        world.passthrough = {}
