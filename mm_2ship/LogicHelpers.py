from __future__ import annotations

from typing import TYPE_CHECKING, Callable, Iterable

from BaseClasses import CollectionState, Location
from worlds.generic.Rules import set_rule

if TYPE_CHECKING:
    from . import MM2ShipWorld


#
# This file intentionally contains NO LOGIC.
# It exists only to provide a compatible framework
# so regions, locations, and rules can be added later.
#


class rule_wrapper:
    """
    Minimal rule wrapper to match Archipelago's expectations.
    Currently all rules always return True (because the wrapped rule should be True).
    """

    def __init__(self, rule: Callable[[CollectionState], bool], world: "MM2ShipWorld"):
        self.rule = rule
        self.world = world

    @staticmethod
    def wrap(rule: Callable[[CollectionState], bool], world: "MM2ShipWorld") -> Callable[[CollectionState], bool]:
        wrapper = rule_wrapper(rule, world)
        return wrapper.evaluate

    def evaluate(self, state: CollectionState) -> bool:
        return self.rule(state)


def add_locations(
    world: "MM2ShipWorld",
    parent_region: str,
    locations: Iterable[str],
) -> None:
    """
    Attach the given location names to the given region.

    This function assumes locations already exist in the multiworld,
    which is the standard AP pattern once you build Regions.py to create them.

    All rules are always True for now.
    """
    region = world.get_region(parent_region)

    for location_name in locations:
        loc_obj: Location = world.get_location(location_name)

        # Attach to region if not already attached
        if loc_obj not in region.locations:
            region.locations.append(loc_obj)

        set_rule(loc_obj, lambda _: True)


def connect_regions(
    world: "MM2ShipWorld",
    parent_region: str,
    child_regions: Iterable[str],
) -> None:
    """
    Connect parent -> child for each child region name.

    No access rules yet (always True).
    """
    parent = world.get_region(parent_region)

    for child_name in child_regions:
        child = world.get_region(child_name)
        parent.connect(child, rule=lambda _: True)
