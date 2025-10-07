#ifndef RANDO_TRAP_H
#define RANDO_TRAP_H

#include "Rando/Rando.h"

typedef enum { TRAP_BLAST, TRAP_FREEZE, TRAP_SHOCK, TRAP_MAX } TrapTypes;

extern std::string GetTrapMessage();

#endif