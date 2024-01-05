/*
 * File: z_en_fire_rock.c
 * Overlay: ovl_En_Fire_Rock
 * Description: [Empty]
 */

#include "z_en_fire_rock.h"

#define FLAGS (ACTOR_FLAG_10 | ACTOR_FLAG_20)

#define THIS ((EnFireRock*)thisx)

void EnFireRock_Init(Actor* thisx, PlayState* play);
void EnFireRock_Destroy(Actor* thisx, PlayState* play);
void EnFireRock_Update(Actor* thisx, PlayState* play);
void EnFireRock_Draw(Actor* thisx, PlayState* play);

ActorInit En_Fire_Rock_InitVars = {
    /**/ ACTOR_EN_FIRE_ROCK,
    /**/ ACTORCAT_ENEMY,
    /**/ FLAGS,
    /**/ OBJECT_EFC_STAR_FIELD,
    /**/ sizeof(EnFireRock),
    /**/ EnFireRock_Init,
    /**/ EnFireRock_Destroy,
    /**/ EnFireRock_Update,
    /**/ EnFireRock_Draw,
};

void EnFireRock_Init(Actor* thisx, PlayState* play) {
}

void EnFireRock_Destroy(Actor* thisx, PlayState* play) {
}

void EnFireRock_Update(Actor* thisx, PlayState* play) {
}

void EnFireRock_Draw(Actor* thisx, PlayState* play) {
}
