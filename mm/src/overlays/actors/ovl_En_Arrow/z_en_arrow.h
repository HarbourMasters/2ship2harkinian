#ifndef Z_EN_ARROW_H
#define Z_EN_ARROW_H

#include "global.h"
#include "objects/gameplay_keep/gameplay_keep.h"

struct EnArrow;

typedef void (*EnArrowActionFunc)(struct EnArrow*, PlayState*);

typedef enum ArrowType {
    /* 0 */ ARROW_TYPE_NORMAL_LIT, // Normal arrow lit on fire
    /* 1 */ ARROW_TYPE_NORMAL_HORSE, // Normal arrow shot while riding a horse
    /* 2 */ ARROW_TYPE_NORMAL,
    /* 3 */ ARROW_TYPE_FIRE,
    /* 4 */ ARROW_TYPE_ICE,
    /* 5 */ ARROW_TYPE_LIGHT,
    /* 6 */ ARROW_TYPE_SLINGSHOT,
    /* 7 */ ARROW_TYPE_DEKU_BUBBLE,
    /* 8 */ ARROW_TYPE_DEKU_NUT,
    // Skijer's NEI — SW97 elemental system, 1:1 with the SoH fork's EnArrow extension
    // (soh z_en_arrow.h ARROW_SEED_FIRE..0E / ARROW_SW97_FIRE..0E). Elemental SEEDS are
    // slingshot pellets carrying an elemental damage bit + a charge-glow child actor;
    // SW97 arrows are the six medallion bow arrows (incl. dark/soul/wind).
    /*  9 */ ARROW_TYPE_SEED_FIRE,
    /* 10 */ ARROW_TYPE_SEED_ICE,
    /* 11 */ ARROW_TYPE_SEED_LIGHT,
    /* 12 */ ARROW_TYPE_SEED_DARK,
    /* 13 */ ARROW_TYPE_SEED_SOUL,
    /* 14 */ ARROW_TYPE_SEED_WIND,
    /* 15 */ ARROW_TYPE_SW97_FIRE,
    /* 16 */ ARROW_TYPE_SW97_ICE,
    /* 17 */ ARROW_TYPE_SW97_LIGHT,
    /* 18 */ ARROW_TYPE_SW97_DARK,
    /* 19 */ ARROW_TYPE_SW97_SOUL,
    /* 20 */ ARROW_TYPE_SW97_WIND
} ArrowType;

#define ARROW_IS_MAGICAL(arrowType) (((arrowType) >= ARROW_TYPE_FIRE) && ((arrowType) <= ARROW_TYPE_LIGHT))
#define ARROW_GET_MAGIC_FROM_TYPE(arrowType) (s32)((arrowType) - ARROW_TYPE_FIRE)
// Skijer's NEI: SW97 bow arrows behave as real arrows (skeleton, blure, stick-on-hit, 150 speed);
// elemental seeds behave as seed pellets (sparkle billboard, 80 speed, Stone1 + SLING_REFLECT kill).
#define ARROW_IS_SW97_ARROW(arrowType) (((arrowType) >= ARROW_TYPE_SW97_FIRE) && ((arrowType) <= ARROW_TYPE_SW97_WIND))
#define ARROW_IS_SEED(arrowType) \
    (((arrowType) == ARROW_TYPE_SLINGSHOT) || (((arrowType) >= ARROW_TYPE_SEED_FIRE) && ((arrowType) <= ARROW_TYPE_SEED_WIND)))
#define ARROW_IS_ARROW(arrowType) (((arrowType) < ARROW_TYPE_SLINGSHOT) || ARROW_IS_SW97_ARROW(arrowType))
// Element index (0=fire 1=ice 2=light 3=dark 4=soul 5=wind) for glow-child / dmg-flag dispatch.
#define ARROW_GET_ELEMENT_FROM_SEED(arrowType) ((arrowType) - ARROW_TYPE_SEED_FIRE)
#define ARROW_GET_ELEMENT_FROM_SW97(arrowType) ((arrowType) - ARROW_TYPE_SW97_FIRE)

typedef enum ArrowMagic {
    /* -1 */ ARROW_MAGIC_INVALID = -1,
    /*  0 */ ARROW_MAGIC_FIRE = ARROW_GET_MAGIC_FROM_TYPE(ARROW_TYPE_FIRE),
    /*  1 */ ARROW_MAGIC_ICE = ARROW_GET_MAGIC_FROM_TYPE(ARROW_TYPE_ICE),
    /*  2 */ ARROW_MAGIC_LIGHT = ARROW_GET_MAGIC_FROM_TYPE(ARROW_TYPE_LIGHT),
    /*  3 */ ARROW_MAGIC_DEKU_BUBBLE // Only used in Player. Does not map to ARROW_TYPE_SLINGSHOT
} ArrowMagic;


typedef struct {
    /* 0x144 */ SkelAnime skelAnime;
    /* 0x188 */ Vec3s jointTable[ARROW_LIMB_MAX];
} EnArrowArrow; // size = 0x1A8

typedef struct {
    /* 0x144 */ f32 unk_144;
    /* 0x148 */ u8 unk_148;
    /* 0x149 */ s8 unk_149;
    /* 0x14A */ s16 unk_14A;
    /* 0x14C */ s16 unk_14C;
} EnArrowBubble; // size = 0x150

typedef struct EnArrow {
    /* 0x000 */ Actor actor;
    union {
        EnArrowArrow arrow;
        EnArrowBubble bubble;
    };
    /* 0x1A8 */ ColliderQuad collider;
    /* 0x228 */ Vec3f unk_228;
    /* 0x234 */ Vec3f unk_234;
    /* 0x240 */ s32 unk_240;
    /* 0x244 */ WeaponInfo unk_244;
    /* 0x260 */ u8 unk_260; // timer in OoT
    /* 0x261 */ u8 unk_261; // hitFlags in OoT
    /* 0x262 */ u8 unk_262;
    /* 0x263 */ u8 unk_263;
    /* 0x264 */ Actor* unk_264;
    /* 0x268 */ Vec3f unk_268;
    /* 0x274 */ EnArrowActionFunc actionFunc;
} EnArrow; // size = 0x278

#endif // Z_EN_ARROW_H
