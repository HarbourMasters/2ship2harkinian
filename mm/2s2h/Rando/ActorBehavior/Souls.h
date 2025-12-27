#ifndef SOULS_H
#define SOULS_H

#include "Rando/Rando.h"

#define ENEMY_SOUL_RI_TO_RANDO_INF(randoItemId) ((randoItemId - RI_SOUL_ALIEN) + RANDO_INF_OBTAINED_SOUL_OF_ALIENS)

extern bool HaveEnemySoul(ActorId enemyId);

#endif // SOULS_H