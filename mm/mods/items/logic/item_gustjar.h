/**
 * Gust Jar Item Header
 * Absorb mode (pull + damage) + Blow mode (elemental cone push)
 */

#ifndef ITEM_GUSTJAR_H
#define ITEM_GUSTJAR_H

#include "z64.h"
#include "../custom_items.h"

// Reach. The same numbers drive both the collider volume and the tornado that gets drawn, so the
// visual can never drift away from the hitbox.
//
// The two modes use DIFFERENT shapes on purpose:
//   SUCK — a CYLINDER along the aim. A cone would taper to nothing at the nozzle, which is
//          exactly where you want the pull to be strongest, so suction keeps full width the
//          whole way out.
//   BLOW — a CONE, which is what the tornado mesh actually is: narrow at the jar, wide at the far
//          end. Its half-angle is derived from the length/radius below, not hardcoded.
// Both default to the same reach so the two modes cover the same ground.
#define GUST_CYL_SUCK_LENGTH 200.0f
#define GUST_CYL_SUCK_RADIUS 100.0f
#define GUST_CONE_BLOW_LENGTH 200.0f
#define GUST_CONE_BLOW_RADIUS 100.0f

#define GUST_RANGE_MAX GUST_CYL_SUCK_LENGTH
#define GUST_RANGE_BLOW GUST_CONE_BLOW_LENGTH
#define LINK_HEIGHT_HITBOX 80.0f

// Knockback. MM speeds are units/frame and enemies rebuild velocity from `speed` every frame, so
// these are MOVEMENT speeds, not the OoT port's per-frame teleport distances (which were 40 and
// tunnelled enemies through walls).
#define GUST_BLOW_PUSH_WIND 20.0f // point-blank knockback speed, bare wind (its ONLY job)
#define GUST_BLOW_PUSH_ELEM 8.0f  // ... with an element primed (the element does the work)
#define GUST_BLOW_LIFT 4.0f       // upward velocity floor while inside the cone
#define GUST_BLOW_NUDGE 6.0f      // wall-tested positional nudge so the shove reads immediately

// AT damage. The damage-table row is a MULTIPLIER (sDamageMultipliers indexed by the row's low
// nibble), so this is the base value: MM's Kokiri sword is 1/2 and the Goron pound is 4.
#define GUST_DAMAGE_SUCK 1 // continuous grind, lands every frame of absorb
#define GUST_DAMAGE_BLOW 4 // one heavy blast, same weight as a Goron pound

// AT collider dimensions. This is the cylinder that actually registers hits; the reach values
// above only govern the pull/push volume tests.
//
// SUCTION is a small nub parked at the jar's mouth: things are dragged in by the reach cylinder
// and only get hurt once they arrive, so a big collider here would damage at a distance and make
// the pull pointless.
#define GUST_COL_SUCK_RADIUS 8
#define GUST_COL_SUCK_HEIGHT 8

// BLOW is NOT a fixed size — it is derived per sweep sample so the collider actually covers the
// cone that gets drawn (see GustJar_Blow). The maths:
//
//   the drawn cone runs GUST_CONE_BLOW_LENGTH along the aim and flares to
//   GUST_CONE_BLOW_RADIUS at its mouth, so its radius at distance d is
//       r(d) = GUST_CONE_BLOW_RADIUS * d / GUST_CONE_BLOW_LENGTH   ( = d/2 at the defaults)
//
// A ColliderCylinder is WORLD-VERTICAL (radius in XZ, height along +Y) and does not rotate with
// the aim, so to swallow the cone's circular cross-section at that distance it needs radius r(d)
// horizontally and 2*r(d) vertically. And because dim.yShift is the cylinder's BOTTOM
// (z_collision_check.c: bottom = pos.y + yShift, centre = + height/2), it must be shifted down by
// one radius or the whole cylinder sits above the aim line — which is exactly why a fixed 45x90
// looked tall and narrow and missed the cone.
//
// At the default 200/100 cone that gives 20/40, 50/100 and 80/160 at the three samples.
#define GUST_COL_BLOW_RADIUS_AT(frac) ((s16)(GUST_CONE_BLOW_RADIUS * (frac)))

// Where along the blow cone the AT collider is sampled, as fractions of its length. At the
// default 200-unit cone these land on 40 / 100 / 160, which is what they were hardcoded to
// before — but now they follow the Blow Length slider instead of staying behind it.
#define GUST_BLOW_SWEEP_NEAR 0.2f
#define GUST_BLOW_SWEEP_MID 0.5f
#define GUST_BLOW_SWEEP_FAR 0.8f

// Supporting smoke. The tornado mesh IS the effect now, so this is deliberately sparse: the
// count that matters is how many are ALIVE, not how many spawn. One particle every 3 frames
// with the existing ~12 frame lifetime keeps 3-6 on screen. (Was 6 per frame = ~70 alive,
// which buried the cone in fog.)
#define GUST_VFX_PARTICLES 1
#define GUST_VFX_SPAWN_EVERY 3

// Timers
#define GUST_HEAT_MAX 300      // 10 seconds to overheat (absorb → blow)
#define GUST_BLOW_DURATION 300 // 10 seconds of blow
#define GUST_COOLDOWN 120      // 2 seconds after blow ends

// Modes
#define GUST_MODE_OFF 0
#define GUST_MODE_IDLE 1
#define GUST_MODE_ABSORB 2
#define GUST_MODE_BLOW 3
#define GUST_MODE_ELEMENT_SELECT 4
// LOADED — gust jar is primed with an element but doesn't auto-discharge.
// Entered from ABSORB by pressing L+R together (cycles element + stores blow);
// exits to BLOW when the player releases the C-button (manual discharge).
#define GUST_MODE_LOADED 5

// Element types (selected from medallions)
typedef enum {
    GUST_ELEMENT_WIND = 0,   // Default (no medallion needed, Forest)
    GUST_ELEMENT_FIRE = 1,   // Fire Medallion
    GUST_ELEMENT_ICE = 2,    // Water Medallion
    GUST_ELEMENT_SHADOW = 3, // Shadow Medallion
    GUST_ELEMENT_SPIRIT = 4, // Spirit Medallion
    GUST_ELEMENT_LIGHT = 5,  // Light Medallion
    GUST_ELEMENT_COUNT = 6,
} GustJarElement;

// State aliases
#define gjEquipped gCustomItemState.gustJarEquipped
#define gjMode gCustomItemState.gustJarMode
#define gjElement gCustomItemState.gustJarElement
#define gjBlowActive gCustomItemState.gustJarBlowActive
#define gjHeatTimer gCustomItemState.gustJarHeatTimer
#define gjBlowTimer gCustomItemState.gustJarBlowTimer
#define gjCooldownTimer gCustomItemState.gustJarCooldownTimer
#define gjTimer gCustomItemState.gustJarTimer
#define gjCollider gCustomItemState.gustJarCollider
#define gjFirstPerson gCustomItemState.gustJarFirstPersonActive
#define gjAimMode gCustomItemState.gustJarAimMode
#define gjButtonMask gCustomItemState.gustJarButtonMask
#define gjBlowDir gCustomItemState.gustJarBlowDir

// Blow direction values (gjBlowDir):
#define GUST_DIR_SUCK 0
#define GUST_DIR_BLOW 1

// Collider init — MM damage bit.
//
// MM does NOT share OoT's damage-flag layout: OoT bit 6 is DMG_HAMMER_SWING but MM bit 6 is
// DMG_UNK_0x06, whose damage-table row is DMG_ENTRY(0, ...) on essentially every actor — the
// straight OoT port (0x48) therefore did literally zero damage in MM.
//
// It also matters that this stays a SINGLE bit: CollisionCheck_GetDamageAndEffectOnElementAC
// (z_collision_check.c:78) resolves the damage-table row by shifting dmgFlags until it equals 1,
// i.e. only the HIGHEST set bit is ever read. OR-ing extra flags silently discards the low ones.
//
// DMG_GORON_POUND is MM's heavy-physical equivalent of the Megaton Hammer swing, and it's the one
// bit accepted by every breakable this item targets: Obj_Tsubo (mask 0x058BFFBC), En_Ishi (0x508),
// Obj_Kibako2 (1 << 8 | 1 << 10), plus Obj_Kibako / En_Kusa which take any AC hit.
static ColliderCylinderInit sGustJarColliderInit = { { COL_MATERIAL_NONE, AT_ON | AT_TYPE_PLAYER, AC_NONE, OC1_NONE,
                                                       OC2_TYPE_PLAYER, COLSHAPE_CYLINDER },
                                                     { ELEM_MATERIAL_UNK0,
                                                       { DMG_GORON_POUND, 0x00, GUST_DAMAGE_SUCK },
                                                       { 0x00000000, 0x00, 0x00 },
                                                       ATELEM_ON | ATELEM_SFX_NONE,
                                                       ACELEM_NONE,
                                                       OCELEM_NONE },
                                                     { GUST_COL_SUCK_RADIUS, GUST_COL_SUCK_HEIGHT, 0, { 0, 0, 0 } } };

// Suckable prop actor IDs (for attraction loop). MM ids — the OoT port carried OoT's numbers,
// which point at unrelated MM actors.
static const s16 sGustSuckableProps[] = {
    ACTOR_OBJ_TSUBO,   // Pots
    ACTOR_OBJ_KIBAKO,  // Small grabbable crate
    ACTOR_OBJ_KIBAKO2, // Large wooden crate
    ACTOR_EN_KUSA,     // Grass/bushes
    ACTOR_EN_KUSA2,    // Keaton grass
    ACTOR_EN_ISHI,     // Liftable rocks
};
#define GUST_SUCKABLE_PROP_COUNT (sizeof(sGustSuckableProps) / sizeof(sGustSuckableProps[0]))

// Suckable enemy actor IDs (small/medium enemies that get pulled in by absorb).
// MM ids — the previous list was OoT's raw hex (0x0013 Keese, 0x001B Tektite, ...), which in MM
// resolves to completely different actors, so absorb never latched onto anything.
static const s16 sGustSuckableEnemies[] = {
    ACTOR_EN_FIREFLY,     // Keese (all types)
    ACTOR_EN_TITE,        // Tektite
    ACTOR_EN_ST,          // Skulltula
    ACTOR_EN_SW,          // Skullwalltula
    ACTOR_EN_BB,          // Bad Bat / Blue Bubble
    ACTOR_EN_BBFALL,      // Red Bubble
    ACTOR_EN_DEKUBABA,    // Deku Baba
    ACTOR_EN_KAREBABA,    // Withered / mini Deku Baba
    ACTOR_EN_DEKUNUTS,    // Mad Scrub
    ACTOR_EN_SB,          // Shellblade
    ACTOR_EN_CROW,        // Guay
    ACTOR_EN_SKB,         // Stalchild
    ACTOR_EN_SLIME,       // Chuchu
    ACTOR_EN_GRASSHOPPER, // Dragonfly
    ACTOR_EN_NEO_REEBA,   // Leever
    ACTOR_EN_SNOWMAN,     // Eeno
    ACTOR_EN_BAGUO,       // Nejiron
    ACTOR_EN_TUBO_TRAP,   // Flying pot trap
    ACTOR_EN_RAT,         // Real Bombchu
    ACTOR_EN_INSECT,      // Bug
    ACTOR_EN_OKUTA,       // Octorok
};
#define GUST_SUCKABLE_ENEMY_COUNT (sizeof(sGustSuckableEnemies) / sizeof(sGustSuckableEnemies[0]))

// Element colors for blow cone VFX
typedef struct {
    Color_RGBA8 prim;
    Color_RGBA8 env;
} GustElementColor;

static const GustElementColor sGustElementColors[GUST_ELEMENT_COUNT] = {
    // WIND is the bare gust jar's own element and reads GREEN, so a plain blow is visibly
    // "just wind" and not mistaken for an uncoloured/elementless one.
    [GUST_ELEMENT_WIND] = { { 60, 220, 90, 255 }, { 20, 140, 50, 200 } },
    [GUST_ELEMENT_FIRE] = { { 255, 80, 0, 255 }, { 255, 200, 0, 200 } },
    [GUST_ELEMENT_ICE] = { { 80, 180, 255, 255 }, { 150, 220, 255, 200 } },
    [GUST_ELEMENT_SHADOW] = { { 130, 50, 180, 255 }, { 80, 0, 130, 200 } },
    [GUST_ELEMENT_SPIRIT] = { { 255, 150, 50, 255 }, { 255, 200, 100, 200 } },
    [GUST_ELEMENT_LIGHT] = { { 255, 255, 100, 255 }, { 255, 255, 200, 200 } },
};

// NOTE: there is deliberately no QUEST_MEDALLION_* table here. MM has no medallions, so
// nei_oot_compat.h stubs every QUEST_MEDALLION_* to 0 — CHECK_QUEST_ITEM(QUEST_MEDALLION_FIRE)
// and friends all collapsed onto quest bit 0, which is why element selection ignored the
// medallions entirely. Ownership now goes through Nei_Save()->ootQuestItems (the parallel OoT
// quest store, OOT_QUEST_MEDALLION_* bits), via GustJar_ElementOwned() in item_gustjar.c.

// Element selection, shared with the kaleido wheel (z_kaleido_item.c). Elements are the owned
// subset of GustJarElement, always starting with WIND.
u8 GustJar_ElementCount(void);
u8 GustJar_ElementAt(u8 index);
u8 GustJar_GetElement(void);
void GustJar_SetElement(u8 element);
u8 GustJar_ElementNeighbor(u8 element, s32 dir);
// Medallion ITEM_* used as this element's icon (WIND -> ITEM_MEDALLION_FOREST).
u16 GustJar_ElementIcon(u8 element);

// Particle spawn helpers — exposed so the Harpoon dummy update can replay the
// suck/blow cones for remote players (the local Update path that normally
// spawns these never runs for a dummy). This header is C-only (designated
// initializers above), so C++ consumers must forward-declare these directly
// rather than #include this file.
void GustJar_SpawnSuckVFX(PlayState* play, Vec3f* nozzle, s16 aimYaw);
void GustJar_SpawnBlowVFX(PlayState* play, Vec3f* nozzle, s16 aimYaw, u8 element);

// Returns the medallion ITEM_* matching the gust jar's currently-selected element,
// or -1 when the element is WIND (no overlay needed). Used by the C-button HUD
// draw (z_parameter.c) to show which element is currently primed on the gust jar.
s32 GustJar_GetActiveMedallionItem(void);

// Strip L/R from a player Input copy when the gust jar is the active aimable
// item. Called from inside Player_Update (z_player.c) BEFORE sp44 is handed to
// Player_UpdateCommon — this is the only point early enough to prevent the
// vanilla shield action from latching onto BTN_R. Modelled after
// TransformMasks_FilterB.
void GustJar_FilterPlayerInput(Input* input);

#endif // ITEM_GUSTJAR_H
