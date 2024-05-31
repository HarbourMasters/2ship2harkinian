#include "3DItemDrops.h"
#include "libultraship/libultraship.h"
#include "2s2h/Enhancements/GameInteractor/GameInteractor.h"

extern "C" {
#include "z64.h"
#include "functions.h"
extern PlayState* gPlayState;
}

void Register3DItemDrops() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::On3DSpinItemdrops>(
        [](EnItem00* actor, PlayState* play) {
                if (CVarGetInteger("gEnhancements.Graphics.Item3D", 0) && ((actor->actor.params >= ITEM00_ARROWS_30 && actor->actor.params < ITEM00_MASK) || actor->actor.params == ITEM00_BOMBS_A || actor->actor.params == ITEM00_ARROWS_10 || actor->actor.params == ITEM00_SMALL_KEY || actor->actor.params == ITEM00_DEKU_NUTS_10 || actor->actor.params == ITEM00_BOMBS_0))
                    actor->actor.shape.rot.y += 960;
                else actor->actor.shape.rot.y = 0;
        }
    );
    GameInteractor::Instance->RegisterGameHook<GameInteractor::On3DItemDrops>(
        [](EnItem00* actor, PlayState* play) {
            if (CVarGetInteger("gEnhancements.Graphics.Item3D", 0)) {
                f32 mtxScale;
                switch (actor->actor.params) {
                    case ITEM00_RECOVERY_HEART:
                        mtxScale = 16.0f;
                        Matrix_Scale(mtxScale, mtxScale, mtxScale, MTXMODE_APPLY);
                        GetItem_Draw(play, GID_RECOVERY_HEART);
                        break;
                    case ITEM00_BOMBS_0:
                    case ITEM00_BOMBS_A:
                    case ITEM00_BOMBS_B:
                        mtxScale = 8.0f;
                        Matrix_Scale(mtxScale, mtxScale, mtxScale, MTXMODE_APPLY);
                        GetItem_Draw(play, GID_BOMB);
                        break;
                    case ITEM00_ARROWS_10:
                    case ITEM00_ARROWS_30:
                        mtxScale = 7.0f;
                        Matrix_Scale(mtxScale, mtxScale, mtxScale, MTXMODE_APPLY);
                        GetItem_Draw(play, GID_ARROWS_SMALL);
                        break;
                    case ITEM00_ARROWS_40:
                        mtxScale = 7.0f;
                        Matrix_Scale(mtxScale, mtxScale, mtxScale, MTXMODE_APPLY);
                        GetItem_Draw(play, GID_ARROWS_MEDIUM);
                        break;
                    case ITEM00_ARROWS_50:
                        mtxScale = 7.0f;
                        Matrix_Scale(mtxScale, mtxScale, mtxScale, MTXMODE_APPLY);
                        GetItem_Draw(play, GID_ARROWS_LARGE);
                        break;
                    case ITEM00_DEKU_NUTS_1:
                    case ITEM00_DEKU_NUTS_10:
                        mtxScale = 9.0f;
                        Matrix_Scale(mtxScale, mtxScale, mtxScale, MTXMODE_APPLY);
                        GetItem_Draw(play, GID_DEKU_NUTS);
                        break;
                    case ITEM00_DEKU_STICK:
                        mtxScale = 7.5f;
                        Matrix_Scale(mtxScale, mtxScale, mtxScale, MTXMODE_APPLY);
                        GetItem_Draw(play, GID_DEKU_STICK);
                        break;
                    case ITEM00_MAGIC_JAR_BIG:
                        mtxScale = 8.0f;
                        Matrix_Scale(mtxScale, mtxScale, mtxScale, MTXMODE_APPLY);
                        GetItem_Draw(play, GID_MAGIC_JAR_BIG);
                        break;
                    case ITEM00_MAGIC_JAR_SMALL:
                        mtxScale = 8.0f;
                        Matrix_Scale(mtxScale, mtxScale, mtxScale, MTXMODE_APPLY);
                        GetItem_Draw(play, GID_MAGIC_JAR_SMALL);
                        break;
                    case ITEM00_SMALL_KEY:
                        mtxScale = 8.0f;
                        Matrix_Scale(mtxScale, mtxScale, mtxScale, MTXMODE_APPLY);
                        GetItem_Draw(play, GID_KEY_SMALL);
                        break;
                }
            }
        }
    );
}
