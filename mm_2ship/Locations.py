from __future__ import annotations

from enum import StrEnum
from typing import TYPE_CHECKING

from BaseClasses import CollectionState, Location

from .Enums import Locations  # StrEnum: Locations.SOMETHING.value == "Human Readable Name"

if TYPE_CHECKING:
    from . import MM2ShipWorld


class MM2ShipLocation(Location):
    game = "2 Ship 2 Harkinian (MM)"

    # No age swapping logic for MM.
    def can_reach(self, state: CollectionState) -> bool:
        return super().can_reach(state)


# --------------------------------------------------------------------------
# Stable AP Location IDs
#
# Since Types.h is NOT shipped in the apworld, we assign AP-only IDs here.
# These MUST remain stable: do not reorder Locations in Enums.py; only append new ones.
# --------------------------------------------------------------------------

base_location_table: dict[Locations, int] = {
    loc: i for i, loc in enumerate(Locations, start=1)
}

location_data_table: dict[Locations, int | None] = {
    **base_location_table,
    # Event locations (no network address)
    Locations.VICTORY: None,
}

# Archipelago expects {name: address} where name is a string.
# Filter out event locations (address=None) from the network table
location_table: dict[str, int] = {loc.value: addr for loc, addr in location_data_table.items() if addr is not None}


# pickling will fail unless the items in the group are actual strings
def stringify_set(items: set[StrEnum]) -> set[str]:
    return {str(item) for item in items}


location_name_groups: dict[str, set[str]] = {
    # Add groups later if you want.
    # Example:
    # "Bosses": stringify_set({Locations.ODOLWA, Locations.GOHT, Locations.GYORG, Locations.TWINMOLD}),
}
