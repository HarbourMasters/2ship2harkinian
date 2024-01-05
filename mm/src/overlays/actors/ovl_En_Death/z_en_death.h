#ifndef Z_EN_DEATH_H
#define Z_EN_DEATH_H

#include "global.h"
#include "overlays/actors/ovl_En_Minideath/z_en_minideath.h"
#include "objects/object_death/object_death.h"

struct EnDeath;

typedef void (*EnDeathActionFunc)(struct EnDeath*, PlayState*);

typedef struct EnDeath {
    /* 0x000 */ Actor actor;
    /* 0x144 */ SkelAnime skelAnime;
    /* 0x188 */ EnDeathActionFunc actionFunc;
    /* 0x18C */ u8 unk_18C;
    /* 0x18D */ u8 unk_18D;
    /* 0x18E */ u8 unk_18E;
    /* 0x18F */ u8 dmgEffect;
    /* 0x190 */ s8 numScytheAfterImages;
    /* 0x191 */ u8 unk_191; // animated material color step?
    /* 0x192 */ u8 unk_192;
    /* 0x193 */ u8 unk_193[GOMESS_LIMB_MAX];
    /* 0x1A9 */ u8 unk_1A9[56];
    /* 0x1E2 */ Vec3s jointTable[GOMESS_LIMB_MAX];
    /* 0x266 */ Vec3s morphTable[GOMESS_LIMB_MAX];
    /* 0x2EA */ s16 floatTimer;
    /* 0x2EC */ s16 unk_2EC;
    /* 0x2EE */ s16 actionTimer;
    /* 0x2F0 */ char unk_2F0[0x2];
    /* 0x2F2 */ s16 unk_2F2;
    /* 0x2F4 */ s16 unk_2F4;
    /* 0x2F6 */ s16 coreRotation;
    /* 0x2F8 */ s16 unk_2F8;
    /* 0x2FA */ s16 camId;
    /* 0x2FC */ s16 unk_2FC;
    /* 0x300 */ s32 effectIndex;
    /* 0x304 */ f32 scytheScale;
    /* 0x308 */ f32 coreVelocity;
    /* 0x30C */ f32 camEyeSpeed;
    /* 0x310 */ f32 camAtSpeed;
    /* 0x314 */ f32 dmgEffectScale;
    /* 0x318 */ f32 dmgEffectSteamScale;
    /* 0x31C */ f32 dmgEffectAlpha;
    /* 0x320 */ Vec3f corePos;
    /* 0x32C */ Vec3f scytheWorldPos;
    /* 0x338 */ Vec3f scytheScreenPos;
    /* 0x344 */ Vec3f camEyeTarget;
    /* 0x350 */ Vec3f camAtTarget;
    /* 0x35C */ Vec3f dmgEffectPositions[14];
    /* 0x404 */ Vec3f unk_404[56];
    /* 0x6A4 */ MtxF unk_6A4;
    /* 0x6E4 */ EnMinideath* miniDeaths[20];
    /* 0x734 */ AnimatedMaterial* bodyMatAnim;
    /* 0x738 */ AnimatedMaterial* coreMatAnim;
    /* 0x73C */ ColliderCylinder bodyCollider;
    /* 0x788 */ ColliderQuad unk_788; // weapon?
    /* 0x808 */ ColliderSphere coreCollider;
    /* 0x860 */ ColliderTris unk_860;
    /* 0x880 */ ColliderTrisElement unk_880[2];
} EnDeath; // size = 0x938

#endif // Z_EN_DEATH_H
