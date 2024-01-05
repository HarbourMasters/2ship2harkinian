/*
 * File: z_en_boj_02.c
 * Overlay: ovl_En_Boj_02
 * Description: [Empty]
 */

#include "z_en_boj_02.h"

#define FLAGS (ACTOR_FLAG_TARGETABLE | ACTOR_FLAG_FRIENDLY | ACTOR_FLAG_10)

#define THIS ((EnBoj02*)thisx)

void EnBoj02_Init(Actor* thisx, PlayState* play);
void EnBoj02_Destroy(Actor* thisx, PlayState* play);
void EnBoj02_Update(Actor* thisx, PlayState* play);
void EnBoj02_Draw(Actor* thisx, PlayState* play);

ActorInit En_Boj_02_InitVars = {
    /**/ ACTOR_EN_BOJ_02,
    /**/ ACTORCAT_NPC,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(EnBoj02),
    /**/ EnBoj02_Init,
    /**/ EnBoj02_Destroy,
    /**/ EnBoj02_Update,
    /**/ EnBoj02_Draw,
};

void EnBoj02_Init(Actor* thisx, PlayState* play) {
}

void EnBoj02_Destroy(Actor* thisx, PlayState* play) {
}

void EnBoj02_Update(Actor* thisx, PlayState* play) {
}

void EnBoj02_Draw(Actor* thisx, PlayState* play) {
}
