# Working with 2 Ship 2 Harkinian - Developer Guide

A practical guide for adding enhancements to the 2Ship Majora's Mask PC port, learned from hands-on development.

## Project Structure

```
2ship2harkinian/
  mm/
    src/                    # Original N64 decomp code (C) - AVOID EDITING
    2s2h/                   # Modern port code (C++) - WHERE YOU WORK
      BenGui/               # UI/menu system (ImGui-based)
      BenPort.cpp/.h        # Main port integration layer
      Enhancements/         # All enhancement/mod code lives here
        Accessibility/      # Input helpers (ESS modifier, half-stick, quick spin)
        Player/             # Player movement enhancements (chain jump, wall kick, etc)
        Camera/             # Camera enhancements
        Cheats/             # Cheat toggles
        ...                 # Other categories
      GameInteractor/       # Hook/event system for intercepting game behavior
      ShipInit.hpp          # Auto-registration system for enhancements
    include/                # Game headers (z64player.h, z64actor.h, etc)
    assets/                 # Game asset definitions
  libultraship/             # Engine submodule - AVOID EDITING (merge conflicts)
  ZAPDTR/                   # Asset extraction tool - DON'T TOUCH
  OTRExporter/              # Asset packaging - DON'T TOUCH
```

### What You Can Edit

| Directory | Safe to Edit? | Notes |
|---|---|---|
| `mm/2s2h/` | YES | This is where all enhancements go |
| `mm/src/` | MINIMAL | Only to add `GameInteractor_Should()` VB hooks |
| `libultraship/` | NO | It's a submodule, changes cause merge hell |
| `ZAPDTR/` | NO | Build tool, not your problem |

## Building

### macOS
```bash
brew install sdl2 libpng glew ninja cmake nlohmann-json libzip libogg libvorbis opus opusfile
git submodule update --init
cmake -S . -B build-cmake -G Ninja
cmake --build build-cmake --target Generate2ShipOtr
cmake --build build-cmake
# Binary: ./build-cmake/mm/2s2h-macos
```

### When to Clean Build
- New files added/moved (CMake needs to re-glob)
- Header files changed
- Submodule changes
- Mysterious "no change" behavior after edits

### When Incremental Build is Fine
- Editing a single `.cpp` file
- Changing `#define` values
- Modifying function bodies (not signatures)

## Creating an Enhancement

### Minimal Enhancement Template

```cpp
#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "variables.h"
}

#define CVAR_NAME "gEnhancements.Category.FeatureName"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterMyFeature() {
    COND_HOOK(OnActorUpdate, CVAR, [](Actor* actor) {
        if (actor->id != ACTOR_PLAYER || gPlayState == NULL) {
            return;
        }
        Player* player = (Player*)actor;
        // Your code here
    });
}

static RegisterShipInitFunc initFunc(RegisterMyFeature, { CVAR_NAME });
```

### Key Points
- File goes in `mm/2s2h/Enhancements/<Category>/`
- CMake auto-discovers new `.cpp` files via glob (clean build required)
- `RegisterShipInitFunc` auto-registers your function on startup and re-runs when the CVar changes
- Use `COND_HOOK` / `COND_VB_SHOULD` for hooks that should only be active when the CVar is enabled

### Adding a Menu Toggle

In `mm/2s2h/BenGui/BenMenu.cpp`, find the appropriate section and add:

```cpp
AddWidget(path, "My Feature", WIDGET_CVAR_CHECKBOX)
    .CVar("gEnhancements.Category.FeatureName")
    .Options(CheckboxOptions().Tooltip("Description of what it does."));
```

Available widget types: `WIDGET_CVAR_CHECKBOX`, `WIDGET_CVAR_COMBOBOX`, `WIDGET_CVAR_SLIDER_INT`, `WIDGET_CVAR_SLIDER_FLOAT`, `WIDGET_CVAR_BTN_SELECTOR`, `WIDGET_CUSTOM`.

## Hook System (GameInteractor)

### Available Hooks

| Hook | Signature | When it fires |
|---|---|---|
| `OnActorUpdate` | `(Actor* actor)` | After each actor's update function |
| `OnPassPlayerInputs` | `(Input* input)` | Before player processes input (can modify!) |
| `OnGameStateUpdate` | `()` | Every game frame |
| `OnSceneInit` | `(s16 sceneId, s8 spawnNum)` | Scene load |
| `ShouldActorInit` | `(Actor* actor, bool* should)` | Before actor init |

### Hook Types

```cpp
// Always active when CVar is on
COND_HOOK(OnActorUpdate, CVAR, [](Actor* actor) { ... });

// Override vanilla behavior (return true/false via *should)
COND_VB_SHOULD(VB_PLAYER_CAN_SPIN_ATTACK, CVAR, {
    Player* player = va_arg(args, Player*);
    *should = true; // Force spin attack
});

// ID-filtered (only fires for specific actor type)
COND_ID_HOOK(OnActorInit, ACTOR_PLAYER, CVAR, [](Actor* actor) { ... });
```

### OnPassPlayerInputs vs OnActorUpdate

- `OnPassPlayerInputs` runs BEFORE the player action function. Use it to modify input (`input->cur.button`, `input->press.button`, `stick_x/y`) before the game sees it.
- `OnActorUpdate` runs AFTER the player action function. Use it to react to state changes, modify velocity/position, or override animations.

**Important**: `sPlayerUseHeldItem` (which controls B button attacks) is set BEFORE `OnPassPlayerInputs`, so eating `BTN_B` in the input hook does NOT prevent aerial jump slashes. The game uses `sPlayerUseHeldItem` for the player, not `input->press.button`.

## CVar Naming

### CRITICAL: No Parent-Child Conflicts

CVars use dots as JSON path separators. You CANNOT have:
- `gEnhancements.Player.Feature` (a value)
- `gEnhancements.Player.Feature.SubSetting` (a child)

The config serializer will crash on `unflatten()`. Use flat sibling names:
- `gEnhancements.Player.FeatureEnable`
- `gEnhancements.Player.FeatureSubSetting`

### Convention
```
g[Category].[Subcategory].[FeatureName]
```
Examples: `gEnhancements.Player.ChainJump`, `gCheats.MoonJumpOnL`, `gSettings.Menu.Theme`

## Player State Reference

### Useful State Flags (stateFlags1)

| Flag | Meaning |
|---|---|
| `PLAYER_STATE1_1` | Action locked |
| `PLAYER_STATE1_4` | Climbing ledge |
| `PLAYER_STATE1_8` | Various interaction |
| `PLAYER_STATE1_4000` | Ledge climb |
| `PLAYER_STATE1_40000` | Ledge related |
| `PLAYER_STATE1_100000` | Z-targeting active |
| `PLAYER_STATE1_CARRYING_ACTOR` | Holding object |
| `PLAYER_STATE1_CHARGING_SPIN_ATTACK` | Charging B |
| `PLAYER_STATE1_FRIENDLY_ACTOR_FOCUS` | Talking/interacting |
| `PLAYER_STATE1_8000000` | Swimming |
| `PLAYER_STATE1_20000000` | Cutscene/locked |

### Ground Check Flags

| Flag | Meaning |
|---|---|
| `BGCHECKFLAG_GROUND` | Currently on ground |
| `BGCHECKFLAG_GROUND_TOUCH` | Just landed this frame |
| `BGCHECKFLAG_GROUND_LEAVE` | Just left ground this frame |
| `BGCHECKFLAG_WALL` | Touching wall (generic actors) |
| `BGCHECKFLAG_PLAYER_WALL_INTERACT` | Touching wall (PLAYER ONLY - use this!) |

**Important**: The player does NOT use `BGCHECKFLAG_WALL`. It uses `BGCHECKFLAG_PLAYER_WALL_INTERACT` for wall detection.

### Player Actions (Key Functions)

| Function | What it does |
|---|---|
| `func_808373F8(play, player, sfxId)` | Auto-jump (speed-based, like ledge jump) |
| `func_80834D50(play, player, anim, speed, sfxId)` | Direct jump with specific animation/speed |
| `func_80834DB8(player, anim, speed, play)` | Wrapper for func_80834D50 |
| `func_808395F0(play, player, meleeAnim, linearVel, yVel)` | Jump slash/kick start |
| `Player_SetAction(play, player, actionFunc, flag)` | Change player action state |
| `func_8082DC38(player)` | Clear melee weapon state/colliders |

### Player Animation

```cpp
// Play once with root motion adjustment
Player_Anim_PlayOnceAdjusted(play, player, anim);

// Play once without root motion (use for mid-air to avoid position jumps)
PlayerAnimation_PlayOnce(play, &player->skelAnime, anim);

// Set to specific frame (useful for holding a pose)
PlayerAnimation_Change(play, &player->skelAnime, anim, speed, startFrame, endFrame, ANIMMODE_ONCE, 0.0f);

// Get animation length
f32 lastFrame = Animation_GetLastFrame(anim);
```

**Warning**: `Player_Anim_PlayOnceAdjusted` applies root motion from the animation. Using it mid-air can cause position pulses. Use `PlayerAnimation_PlayOnce` instead when airborne.

### Common Jump Animations

| Name | Description |
|---|---|
| `gPlayerAnim_link_normal_jump` | Standard ground/falling jump |
| `gPlayerAnim_link_normal_run_jump` | Running jump |
| `gPlayerAnim_link_fighter_front_jump` | Forward leap/frontflip |
| `gPlayerAnim_link_normal_250jump_start` | Backflip |
| `gPlayerAnim_link_normal_newside_jump_20f` | Side flip |
| `gPlayerAnim_link_normal_newroll_jump_20f` | Roll jump |
| `gPlayerAnim_link_fighter_rolling_kiru` | 1H spin attack (13 frames) |
| `gPlayerAnim_link_fighter_Lrolling_kiru` | 2H spin attack (16 frames) |
| `gPlayerAnim_link_normal_landing_wait` | Neutral landing/standing |

## Common Pitfalls

### 1. ImGui Crashes on Startup
`TextDraw` and similar overlay functions crash if called before ImGui is initialized. `OnActorUpdate` fires during actor init before the render pipeline is ready. Use `TextDrawNotification` instead, or guard with null checks.

### 2. Stale Object Files
If behavior doesn't change after editing, the old `.o` file might be cached. Do a clean build: `cmake --build build-cmake --clean-first`

### 3. Static Variables in z_player.c
Variables like `sPlayerControlInput`, `sPlayerUseHeldItem`, `sPlayerFloorType` are file-scope in z_player.c. You can't easily access them from enhancement code. Use `CONTROLLER1(&gPlayState->state)` for input, and player struct fields for state.

### 4. OnActorUpdate Timing
`OnActorUpdate` runs AFTER the actor's own update. If you set `velocity.y` here, the position was already updated with the old velocity. Your change affects the NEXT frame's position, not this frame's.

### 5. Animation Override Race
`Player_Action_25` (jump action) calls `PlayerAnimation_Update` every frame, which can override your custom animation. To hold a custom animation, re-set it every frame in `OnActorUpdate`.

### 6. Modifier Buttons
`BTN_CUSTOM_MODIFIER1` (0x0040) and `BTN_CUSTOM_MODIFIER2` (0x0080) are defined in `BenPort.h`. They need to be mapped to physical keys in the Input Editor to work. All 16 bits of `CONTROLLERBUTTONS_T` are used - you can't add more without expanding the type.

### 7. extern "C" for Game Functions
Game functions are compiled as C. When declaring them in C++ enhancement files:
```cpp
extern "C" {
#include "variables.h"
void func_808373F8(PlayState* play, Player* player, u16 sfxId);
}
```
Check `z64player.h` first - many functions are already declared there and don't need a manual extern.

## Formatting

The project uses clang-format. Run before committing:
```bash
/Library/Developer/CommandLineTools/usr/bin/clang-format -i -style=file <your_files>
```

## Adding VB Hooks to Decomp Code

When you need to override vanilla behavior that has no existing hook:

1. Add the enum to `mm/2s2h/GameInteractor/GameInteractor_VanillaBehavior.h`
2. Wrap the vanilla call in z_player.c with `GameInteractor_Should(VB_YOUR_HOOK, vanillaCondition, args...)`
3. Keep the decomp change minimal - one line wrapper per call site

This is the only acceptable reason to edit decomp code.
