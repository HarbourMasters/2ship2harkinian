#include "ActorBehavior.h"
#include <libultraship/libultraship.h>
#include "2s2h/CustomItem/CustomItem.h"
#include "assets/2s2h_assets.h"

extern "C" {
#include "variables.h"
#include "src/overlays/actors/ovl_Obj_Snowball/z_obj_snowball.h"
#include "src/overlays/actors/ovl_Obj_Snowball2/z_obj_snowball2.h"
}

std::map<std::pair<float, float>, RandoCheckId> snowballMap = {
    { { 1674.00f, 6811.00f }, RC_PATH_TO_MOUNTAIN_VILLAGE_LARGE_SNOWBALL_01 },
    { { 2061.00f, 6748.00f }, RC_PATH_TO_MOUNTAIN_VILLAGE_LARGE_SNOWBALL_02 },
    { { 203.00f, 6068.00f }, RC_PATH_TO_MOUNTAIN_VILLAGE_LARGE_SNOWBALL_03 },
    { { 623.00f, 6237.00f }, RC_PATH_TO_MOUNTAIN_VILLAGE_LARGE_SNOWBALL_04 },
    { { 323.00f, 5523.00f }, RC_PATH_TO_MOUNTAIN_VILLAGE_LARGE_SNOWBALL_05 },
    { { 230.00f, 5526.00f }, RC_PATH_TO_MOUNTAIN_VILLAGE_LARGE_SNOWBALL_06 },
    { { 467.00f, 5462.00f }, RC_PATH_TO_MOUNTAIN_VILLAGE_LARGE_SNOWBALL_07 },
    { { 393.00f, 5465.00f }, RC_PATH_TO_MOUNTAIN_VILLAGE_LARGE_SNOWBALL_08 },
    { { 987.00f, 4607.00f }, RC_PATH_TO_MOUNTAIN_VILLAGE_LARGE_SNOWBALL_09 },
    { { 976.00f, 4691.00f }, RC_PATH_TO_MOUNTAIN_VILLAGE_LARGE_SNOWBALL_10 },
    { { 979.00f, 4517.00f }, RC_PATH_TO_MOUNTAIN_VILLAGE_LARGE_SNOWBALL_11 },
    { { 1834.00f, 6538.00f }, RC_PATH_TO_MOUNTAIN_VILLAGE_SMALL_SNOWBALL_01 },
    { { 610.00f, 6038.00f }, RC_PATH_TO_MOUNTAIN_VILLAGE_SMALL_SNOWBALL_02 },
    { { 395.00f, 6236.00f }, RC_PATH_TO_MOUNTAIN_VILLAGE_SMALL_SNOWBALL_03 },
    { { 682.00f, 4648.00f }, RC_PATH_TO_MOUNTAIN_VILLAGE_SMALL_SNOWBALL_04 },
};

RandoCheckId IdentifySnowball(Vec3f pos) {
    RandoCheckId randoCheckId = RC_UNKNOWN;

    auto it = snowballMap.find({ pos.x, pos.z });
    if (it == snowballMap.end()) {
        return randoCheckId;
    }

    return it->second;

}

void SpawnSnowballDrop(Vec3f pos, RandoCheckId randoCheckId) {
    CustomItem::Spawn(
        pos.x, pos.y, pos.z, 0, CustomItem::KILL_ON_TOUCH | CustomItem::TOSS_ON_SPAWN | CustomItem::ABLE_TO_ZORA_RANG,
        randoCheckId,
        [](Actor* actor, PlayState* play) {
            RandoSaveCheck& randoSaveCheck = RANDO_SAVE_CHECKS[CUSTOM_ITEM_PARAM];
            randoSaveCheck.eligible = true;
        },
        [](Actor* actor, PlayState* play) {
            auto& randoSaveCheck = RANDO_SAVE_CHECKS[CUSTOM_ITEM_PARAM];
            RandoItemId randoItemId = Rando::ConvertItem(randoSaveCheck.randoItemId);
            Matrix_Scale(30.0f, 30.0f, 30.0f, MTXMODE_APPLY);
            Rando::DrawItem(Rando::ConvertItem(randoSaveCheck.randoItemId, (RandoCheckId)CUSTOM_ITEM_PARAM), actor);
        });
}


//void ObjSnowball_RandoDraw(Actor* actor, PlayState* play) {
//    if (!CVarGetInteger("gRando.CSMC", 0)) {
//        Gfx_DrawDListOpa(play, (Gfx*)gBarrelJunkDL);
//        return;
//    }
//
//    RandoItemId randoItemId = Rando::ConvertItem(RANDO_SAVE_CHECKS[OBJTARU_RC].randoItemId, (RandoCheckId)OBJTARU_RC);
//    RandoItemType randoItemType = Rando::StaticData::Items[randoItemId].randoItemType;
//
//    switch (randoItemType) {
//        case RITYPE_BOSS_KEY:
//            Gfx_DrawDListOpa(play, (Gfx*)gBarrelBossKeyDL);
//            break;
//        case RITYPE_HEALTH:
//            Gfx_DrawDListOpa(play, (Gfx*)gBarrelHeartDL);
//            break;
//        case RITYPE_LESSER:
//            Gfx_DrawDListOpa(play, (Gfx*)gBarrelMinorDL);
//            break;
//        case RITYPE_MAJOR:
//            Gfx_DrawDListOpa(play, (Gfx*)gBarrelMajorDL);
//            break;
//        case RITYPE_MASK:
//            Gfx_DrawDListOpa(play, (Gfx*)gBarrelMaskDL);
//            break;
//        case RITYPE_SKULLTULA_TOKEN:
//            Gfx_DrawDListOpa(play, (Gfx*)gBarrelTokenDL);
//            break;
//        case RITYPE_SMALL_KEY:
//            Gfx_DrawDListOpa(play, (Gfx*)gBarrelSmallKeyDL);
//            break;
//        case RITYPE_STRAY_FAIRY:
//            Gfx_DrawDListOpa(play, (Gfx*)gBarrelFairyDL);
//            break;
//        default:
//            Gfx_DrawDListOpa(play, (Gfx*)gBarrelJunkDL);
//            break;
//    }
//}

void Rando::ActorBehavior::InitObjSnowballBehavior() {
    //COND_ID_HOOK(OnActorInit, ACTOR_OBJ_SNOWBALL, IS_RANDO, [](Actor* actor) {
    //    RandoCheckId randoCheckId = RC_UNKNOWN;

    //    auto it = snowballMap.find({ actor->home.pos.x, actor->home.pos.z });
    //    if (it == snowballMap.end()) {
    //        return;
    //    }

    //    randoCheckId = it->second;

    //    if (!RANDO_SAVE_CHECKS[randoCheckId].shuffled || RANDO_SAVE_CHECKS[randoCheckId].cycleObtained) {
    //        return;
    //    }

    //    OBJSNOWBALL_RC = randoCheckId;
    //    //actor->draw = ObjSnowball_RandoDraw;
    //});

    //COND_ID_HOOK(OnActorInit, ACTOR_OBJ_SNOWBALL2, IS_RANDO, [](Actor* actor) {
    //    RandoCheckId randoCheckId = RC_UNKNOWN;

    //    auto it = snowballMap.find({ actor->home.pos.x, actor->home.pos.z });
    //    if (it == snowballMap.end()) {
    //        return;
    //    }

    //    randoCheckId = it->second;

    //    if (!RANDO_SAVE_CHECKS[randoCheckId].shuffled || RANDO_SAVE_CHECKS[randoCheckId].cycleObtained) {
    //        return;
    //    }

    //    OBJSNOWBALL_RC = randoCheckId;
    //    //actor->draw = ObjSnowball_RandoDraw;
    //});

    COND_HOOK(OnActorKill, IS_RANDO, [](Actor* actor) {
         if (actor->id != ACTOR_OBJ_SNOWBALL && actor->id != ACTOR_OBJ_SNOWBALL2) {
            return;
        }

        if (IdentifySnowball(actor->home.pos) == RC_UNKNOWN) {
            return;
        } else {
            SpawnSnowballDrop(actor->home.pos, IdentifySnowball(actor->home.pos));
        }
    });

    COND_VB_SHOULD(VB_SNOWBALL_DROP_COLLECTIBLE, IS_RANDO, {
        Actor* actor = va_arg(args, Actor*);

        if (actor->id != ACTOR_OBJ_SNOWBALL && actor->id != ACTOR_OBJ_SNOWBALL2) {
            return;
        }

        if (IdentifySnowball(actor->home.pos) == RC_UNKNOWN) {
            return;
        }

        *should = false;
    });
}
