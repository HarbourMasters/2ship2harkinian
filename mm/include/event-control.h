#ifndef EVENT_CONTROL_H
#define EVENT_CONTROL_H

#include "luslog.h"

struct Input;

#define EVENT_CONTROL_ROLL 1
#define EVENT_CONTROL_INTERACT 2

typedef struct EventControl {
    // Traditional `A` button controls
    /**
     * Control that defines that interaction was requested by the player.
     * Example: talking to an NPC, which is the `A` button in vanilla.
    */
    uint16_t (*interactionControl)(Input *sPlayerControlInput);
    /**
     * Control that defines that player is requesting rolling action or jumping.
     * `A` in vanilla.
    */
    uint16_t (*roll)(Input *sPlayerControlInput);
} EventControl;

// typedef struct {
//     bool eventRollEnabled;
//     bool eventInteractEnabled;
// } DefaultStorage;
// extern DefaultStorage eventControlStorage;

// extern EventControl curEventControl;

// void initEventControl(EventControl* control){
//     control->roll = &defaultRoll;
// }

#endif