#include "3DItemDrops.h"
#include "libultraship/libultraship.h"
#include "2s2h/Enhancements/GameInteractor/GameInteractor.h"

extern "C" {
#include "z64.h"
#include "functions.h"
extern PlayState* gPlayState;
void EnItem00_Draw(Actor* thisx, PlayState* play);
}

bool ItemShouldSpinWhen3D(Actor* actor) {
    EnItem00* enItem00 = (EnItem00*)actor;

    // Exclude actors that already have spin normally, or shouldn't spin ever
    if ((actor->params <= ITEM00_RUPEE_RED) || ((actor->params == ITEM00_RECOVERY_HEART) && (enItem00->unk152 < 0)) ||
        (actor->params == ITEM00_HEART_PIECE) || (actor->params == ITEM00_HEART_CONTAINER) ||
        (actor->params == ITEM00_RUPEE_HUGE) || (actor->params == ITEM00_RUPEE_PURPLE) ||
        (actor->params == ITEM00_SHIELD_HERO) || (actor->params == ITEM00_MAP) || (actor->params == ITEM00_COMPASS)) {
        return false;
    }

    return true;
}

void EnItem00_3DItemsDraw(Actor* actor, PlayState* play) {
    EnItem00* enItem00 = (EnItem00*)actor;

    if (!(enItem00->unk14E & enItem00->unk150)) {
        switch (enItem00->actor.params) {
            case ITEM00_RUPEE_GREEN:
                Matrix_Scale(25.0f, 25.0f, 25.0f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_RUPEE_GREEN);
                break;
            case ITEM00_RUPEE_BLUE:
                Matrix_Scale(25.0f, 25.0f, 25.0f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_RUPEE_BLUE);
                break;
            case ITEM00_RUPEE_RED:
                Matrix_Scale(25.0f, 25.0f, 25.0f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_RUPEE_RED);
                break;
            case ITEM00_RUPEE_HUGE:
                Matrix_Scale(17.5f, 17.5f, 17.5f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_RUPEE_HUGE);
                break;
            case ITEM00_RUPEE_PURPLE:
                Matrix_Scale(17.5f, 17.5f, 17.5f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_RUPEE_PURPLE);
                break;

            case ITEM00_HEART_PIECE:
                Matrix_Scale(16.0f, 16.0f, 16.0f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_HEART_PIECE);
                break;

            case ITEM00_HEART_CONTAINER:
                Matrix_Scale(16.0f, 16.0f, 16.0f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_HEART_CONTAINER);
                break;

            case ITEM00_RECOVERY_HEART:
                Matrix_Scale(16.0f, 16.0f, 16.0f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_RECOVERY_HEART);
                break;
            case ITEM00_BOMBS_A:
            case ITEM00_BOMBS_B:
            case ITEM00_BOMBS_0:
                Matrix_Scale(8.0f, 8.0f, 8.0f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_BOMB);
                break;
            case ITEM00_ARROWS_10:
            case ITEM00_ARROWS_30:
            case ITEM00_ARROWS_40:
            case ITEM00_ARROWS_50:
                Matrix_Scale(7.0f, 7.0f, 7.0f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_ARROWS_MEDIUM);
                break;
            case ITEM00_MAGIC_JAR_SMALL:
                Matrix_Scale(8.0f, 8.0f, 8.0f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_MAGIC_JAR_SMALL);
                break;
            case ITEM00_MAGIC_JAR_BIG:
                Matrix_Scale(8.0f, 8.0f, 8.0f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_MAGIC_JAR_BIG);
                break;
            case ITEM00_DEKU_STICK:
                Matrix_Scale(7.5f, 7.5f, 7.5f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_DEKU_STICK);
                break;
            case ITEM00_SMALL_KEY:
                Matrix_Scale(8.0f, 8.0f, 8.0f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_KEY_SMALL);
                break;
            case ITEM00_DEKU_NUTS_1:
            case ITEM00_DEKU_NUTS_10:
                Matrix_Scale(9.0f, 9.0f, 9.0f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_DEKU_NUTS);
                break;

            case ITEM00_SHIELD_HERO:
                GetItem_Draw(play, GID_SHIELD_HERO);
                break;

            case ITEM00_MAP:
                GetItem_Draw(play, GID_DUNGEON_MAP);
                break;

            case ITEM00_COMPASS:
                GetItem_Draw(play, GID_COMPASS);
                break;
        }
    }
}

void Register3DItemDrops() {
    static HOOK_ID actorInitHookID = 0;
    static HOOK_ID actorUpdateHookID = 0;
    GameInteractor::Instance->UnregisterGameHookForID<GameInteractor::OnActorInit>(actorInitHookID);
    GameInteractor::Instance->UnregisterGameHookForID<GameInteractor::OnActorUpdate>(actorUpdateHookID);
    actorInitHookID = 0;
    actorUpdateHookID = 0;

    if (gPlayState != NULL) {
        Actor* actor = gPlayState->actorCtx.actorLists[ACTORCAT_MISC].first;

        while (actor != NULL) {
            if (actor->id == ACTOR_EN_ITEM00) {
                if (CVarGetInteger("gEnhancements.Graphics.3DItemDrops", 0)) {
                    actor->draw = EnItem00_3DItemsDraw;
                } else {
                    actor->draw = EnItem00_Draw;

                    // Reset rotation for bill-boarded sprites
                    if (ItemShouldSpinWhen3D(actor)) {
                        actor->shape.rot.y = 0;
                    }
                }
            }

            actor = actor->next;
        }
    }

    if (!CVarGetInteger("gEnhancements.Graphics.3DItemDrops", 0)) {
        return;
    }

    actorInitHookID = GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnActorInit>(
        ACTOR_EN_ITEM00, [](Actor* actor) { actor->draw = EnItem00_3DItemsDraw; });
    actorUpdateHookID = GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnActorUpdate>(
        ACTOR_EN_ITEM00, [](Actor* actor) {
            // Add spin to normally bill-boarded items
            if (ItemShouldSpinWhen3D(actor)) {
                actor->shape.rot.y += 0x3C0;
            }
        });
}
