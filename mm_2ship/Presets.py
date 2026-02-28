# Presets.py
from typing import Any
from Options import Accessibility, ProgressionBalancing


# Keep the import style, but don't pull in unused SoH symbols.
# Importing * is fine for scaffolding (matches SoH pattern).
from .Options import *  # noqa: F403


default_options: dict[str, Any] = {
    "progression_balancing": ProgressionBalancing.default,
    "accessibility": Accessibility.option_full,

    # MM2Ship options (scaffolding)
    "goal": Goal.option_beat_game,  # noqa: F405
    "triforce_hunt": TriforceHunt.option_false,  # noqa: F405
    "triforce_hunt_pieces_total": TriforceHuntPiecesTotal.default,  # noqa: F405
    "triforce_hunt_pieces_required_percentage": TriforceHuntPiecesRequiredPercentage.default,  # noqa: F405

    # Common AP plumbing
    "start_inventory_from_pool": {},

    # SoH-style escape hatch
    "true_no_logic": TrueNoLogic.option_false,  # noqa: F405
}


beginner_options: dict[str, Any] = {
    "progression_balancing": ProgressionBalancing.default,
    "accessibility": Accessibility.option_full,

    "goal": Goal.option_beat_game,  # noqa: F405
    "triforce_hunt": TriforceHunt.option_false,  # noqa: F405
    "triforce_hunt_pieces_total": TriforceHuntPiecesTotal.default,  # noqa: F405
    "triforce_hunt_pieces_required_percentage": TriforceHuntPiecesRequiredPercentage.default,  # noqa: F405

    "start_inventory_from_pool": {},
    "true_no_logic": TrueNoLogic.option_false,  # noqa: F405
}


standard_options: dict[str, Any] = {
    "progression_balancing": ProgressionBalancing.default,
    "accessibility": Accessibility.option_full,

    "goal": Goal.option_beat_game,  # noqa: F405
    "triforce_hunt": TriforceHunt.option_false,  # noqa: F405
    "triforce_hunt_pieces_total": TriforceHuntPiecesTotal.default,  # noqa: F405
    "triforce_hunt_pieces_required_percentage": TriforceHuntPiecesRequiredPercentage.default,  # noqa: F405

    "start_inventory_from_pool": {},
    "true_no_logic": TrueNoLogic.option_false,  # noqa: F405
}


advanced_options: dict[str, Any] = {
    "progression_balancing": ProgressionBalancing.default,
    "accessibility": Accessibility.option_full,

    # Still scaffolding; keep values sane but a little "spicier"
    "goal": Goal.option_triforce_hunt,  # noqa: F405
    "triforce_hunt": TriforceHunt.option_true,  # noqa: F405
    "triforce_hunt_pieces_total": TriforceHuntPiecesTotal.default,  # noqa: F405
    "triforce_hunt_pieces_required_percentage": TriforceHuntPiecesRequiredPercentage.default,  # noqa: F405

    "start_inventory_from_pool": {},
    "true_no_logic": TrueNoLogic.option_false,  # noqa: F405
}


hell_mode_options: dict[str, Any] = {
    "progression_balancing": ProgressionBalancing.default,
    "accessibility": Accessibility.option_full,

    "goal": Goal.option_triforce_hunt,  # noqa: F405
    "triforce_hunt": TriforceHunt.option_true,  # noqa: F405
    "triforce_hunt_pieces_total": TriforceHuntPiecesTotal.default,  # noqa: F405
    "triforce_hunt_pieces_required_percentage": TriforceHuntPiecesRequiredPercentage.default,  # noqa: F405

    "start_inventory_from_pool": {},

    # If you want a "no logic" preset at all, keep it explicit here.
    # Leaving it false by default keeps generation predictable.
    "true_no_logic": TrueNoLogic.option_false,  # noqa: F405
}


mm2ship_options_presets: dict[str, dict[str, Any]] = {
    "MM2Ship Default": default_options,
    "MM2Ship Beginner": beginner_options,
    "MM2Ship Standard": standard_options,
    "MM2Ship Advanced": advanced_options,
    "MM2Ship Hell Mode": hell_mode_options,
}
