/**
 * Demise Destruction Item Header
 * Powerful magic attack with explosion and lightning
 */

#ifndef ITEM_DEMISE_DESTRUCTION_H
#define ITEM_DEMISE_DESTRUCTION_H

#include "z64.h"
#include "../custom_items.h"

// States
#define DEMISE_STATE_IDLE 0
#define DEMISE_STATE_WINDUP 1
#define DEMISE_STATE_FINISH 2

// Timings
#define DEMISE_WINDUP_DURATION 70
#define DEMISE_FINISH_DURATION 5
#define DEMISE_ANIM_DURATION 116

// Magic
#define DEMISE_MAGIC_COST 12

// Collision
#define DEMISE_COLLISION_RADIUS 400.0f
#define DEMISE_COLLISION_HEIGHT 200
#define DEMISE_DAMAGE 40

// Damage flags
// TODO(port): self-defined OoT damage bits. In MM bit 0x06 is DMG_UNK_0x06 and
// bit 0x1E is DMG_UNK_0x1E (no hammer in MM), so DMG_HAMMER_SWING/DMG_HAMMER_JUMP
// here are placeholders that no MM enemy damage-table maps to. DMG_BOOMERANG bit
// 0x04 corresponds to MM's DMG_ZORA_BOOMERANG. Left as-is pending review.
#ifndef DMG_HAMMER_SWING
#define DMG_HAMMER_SWING (1 << 0x06)
#endif
#ifndef DMG_HAMMER_JUMP
#define DMG_HAMMER_JUMP (1 << 0x1E)
#endif
#ifndef DMG_BOOMERANG
#define DMG_BOOMERANG (1 << 0x04)
#endif
#define DEMISE_DAMAGE_FLAGS (DMG_HAMMER_SWING | DMG_HAMMER_JUMP | DMG_BOOMERANG)

// State aliases
#define ddActive gCustomItemState.demiseDestructionActive
#define ddState gCustomItemState.timer2
#define ddTimer gCustomItemState.timer1
#define ddCollider gCustomItemState.demiseDestructionCollider

// Collider init
static ColliderCylinderInit sDemiseColInit = { { COL_MATERIAL_NONE, AT_ON | AT_TYPE_PLAYER, AC_NONE, OC1_NONE, OC2_NONE,
                                                 COLSHAPE_CYLINDER },
                                               { ELEM_MATERIAL_UNK2,
                                                 { DEMISE_DAMAGE_FLAGS, 0x00, DEMISE_DAMAGE },
                                                 { 0, 0, 0 },
                                                 ATELEM_ON | ATELEM_SFX_NORMAL,
                                                 ACELEM_NONE,
                                                 OCELEM_NONE },
                                               { 100, 80, 0, { 0, 0, 0 } } };

#endif // ITEM_DEMISE_DESTRUCTION_H
