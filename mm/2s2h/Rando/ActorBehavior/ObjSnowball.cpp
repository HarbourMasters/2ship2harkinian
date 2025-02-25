#include "ActorBehavior.h"
#include <libultraship/libultraship.h>
#include "2s2h/CustomItem/CustomItem.h"
#include "assets/2s2h_assets.h"

extern "C" {
#include "variables.h"
#include "src/overlays/actors/ovl_Obj_Snowball/z_obj_snowball.h"
#include "src/overlays/actors/ovl_Obj_Snowball2/z_obj_snowball2.h"

void func_80B02D58(ObjSnowball* snowballActor, PlayState* play);
}

std::map<std::pair<float, float>, RandoCheckId> snowballMap = {
    { { 525.00f, 272.00f }, RC_GORON_VILLAGE_LARGE_SNOWBALL_01 },
    { { -1430.00f, -1458.00f }, RC_GORON_VILLAGE_LARGE_SNOWBALL_02 },
    { { -1644.00f, 120.00f }, RC_GORON_VILLAGE_LARGE_SNOWBALL_03 },
    { { 254.00f, -1450.00f }, RC_GORON_VILLAGE_LARGE_SNOWBALL_04 },
    { { -1999.00f, -716.00f }, RC_GORON_VILLAGE_LARGE_SNOWBALL_05 },
    { { -270.00f, 179.00f }, RC_GORON_VILLAGE_LARGE_SNOWBALL_06 },
    { { -723.00f, -1592.00f }, RC_GORON_VILLAGE_LARGE_SNOWBALL_07 },
    { { -993.00f, 431.00f }, RC_GORON_VILLAGE_LARGE_SNOWBALL_08 },
    { { -447.00f, -1358.00f }, RC_GORON_VILLAGE_LARGE_SNOWBALL_09 },
    { { -338.00f, -724.00f }, RC_GORON_VILLAGE_SMALL_SNOWBALL_01 },
    { { -1719.00f, -608.00f }, RC_GORON_VILLAGE_SMALL_SNOWBALL_02 },
    { { -1717.00f, -460.00f }, RC_GORON_VILLAGE_SMALL_SNOWBALL_03 },
    { { 759.00f, -1067.00f }, RC_GORON_VILLAGE_SMALL_SNOWBALL_04 },
    { { -828.00f, 485.00f }, RC_GORON_VILLAGE_SMALL_SNOWBALL_05 },
    { { -242.00f, -1680.00f }, RC_GORON_VILLAGE_SMALL_SNOWBALL_06 },
    { { -205.00f, 452.00f }, RC_GORON_VILLAGE_SMALL_SNOWBALL_07 },
    { { -1021.00f, -1343.00f }, RC_GORON_VILLAGE_SMALL_SNOWBALL_08 },
    { { 345.00f, 188.00f }, RC_GORON_VILLAGE_SMALL_SNOWBALL_09 },
    { { -1292.00f, -1665.00f }, RC_GORON_VILLAGE_SMALL_SNOWBALL_10 },
    { { 824.00f, -213.00f }, RC_GORON_VILLAGE_SMALL_SNOWBALL_11 },
    { { -274.00f, -309.00f }, RC_GORON_VILLAGE_SMALL_SNOWBALL_12 },
    { { -1025.00f, 137.00f }, RC_MOUNTAIN_VILLAGE_LARGE_SNOWBALL_01 },
    { { -1062.00f, 891.00f }, RC_MOUNTAIN_VILLAGE_LARGE_SNOWBALL_02 },
    { { 1863.00f, 705.00f }, RC_MOUNTAIN_VILLAGE_LARGE_SNOWBALL_03 },
    { { -197.00f, -172.00f }, RC_MOUNTAIN_VILLAGE_LARGE_SNOWBALL_04 },
    { { 1462.00f, 863.00f }, RC_MOUNTAIN_VILLAGE_LARGE_SNOWBALL_05 },
    { { 1574.00f, 559.00f }, RC_MOUNTAIN_VILLAGE_SMALL_SNOWBALL_01 },
    { { 626.00f, 892.00f }, RC_MOUNTAIN_VILLAGE_SMALL_SNOWBALL_02 },
    { { -881.00f, 84.00f }, RC_MOUNTAIN_VILLAGE_SMALL_SNOWBALL_03 },
    { { 973.00f, -211.00f }, RC_MOUNTAIN_VILLAGE_SMALL_SNOWBALL_04 },
    { { 887.00f, -187.00f }, RC_MOUNTAIN_VILLAGE_SMALL_SNOWBALL_05 },
    { { -417.00f, -51.00f }, RC_MOUNTAIN_VILLAGE_SMALL_SNOWBALL_06 },
    { { -751.00f, -197.00f }, RC_MOUNTAIN_VILLAGE_SMALL_SNOWBALL_07 },
    { { -470.00f, -224.00f }, RC_MOUNTAIN_VILLAGE_SMALL_SNOWBALL_08 },
    { { -384.00f, -259.00f }, RC_MOUNTAIN_VILLAGE_SMALL_SNOWBALL_09 },
    { { 244.00f, -1322.00f }, RC_MOUNTAIN_VILLAGE_SMALL_SNOWBALL_10 },
    { { 514.00f, -1213.00f }, RC_MOUNTAIN_VILLAGE_SMALL_SNOWBALL_11 },
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
    { { -912.00f, -1591.00f }, RC_PATH_TO_SNOWHEAD_LARGE_SNOWBALL_01 },
    { { 744.00f, 554.00f }, RC_PATH_TO_SNOWHEAD_LARGE_SNOWBALL_02 },
    { { -1646.00f, -2841.00f }, RC_PATH_TO_SNOWHEAD_LARGE_SNOWBALL_03 },
    { { -1349.00f, -2399.00f }, RC_PATH_TO_SNOWHEAD_LARGE_SNOWBALL_04 },
    { { -1015.00f, -2395.00f }, RC_PATH_TO_SNOWHEAD_SMALL_SNOWBALL_01 },
    { { -912.00f, -2314.00f }, RC_PATH_TO_SNOWHEAD_SMALL_SNOWBALL_02 },
    { { -1023.00f, -2268.00f }, RC_PATH_TO_SNOWHEAD_SMALL_SNOWBALL_03 },
    { { -129.00f, 1195.00f }, RC_SNOWHEAD_LARGE_SNOWBALL_01 },
    { { -1054.00f, 766.00f }, RC_SNOWHEAD_LARGE_SNOWBALL_02 },
    { { 887.00f, 766.00f }, RC_SNOWHEAD_LARGE_SNOWBALL_03 },
    { { 1048.00f, 75.00f }, RC_SNOWHEAD_LARGE_SNOWBALL_04 },
    { { 722.00f, -888.00f }, RC_SNOWHEAD_LARGE_SNOWBALL_05 },
    { { -932.00f, 443.00f }, RC_SNOWHEAD_LARGE_SNOWBALL_06 },
    { { -499.00f, 6081.00f }, RC_SNOWHEAD_SMALL_SNOWBALL_01 },
    { { -370.00f, 6458.00f }, RC_SNOWHEAD_SMALL_SNOWBALL_02 },
    { { -649.00f, 5966.00f }, RC_SNOWHEAD_SMALL_SNOWBALL_03 },
    { { -691.00f, 6209.00f }, RC_SNOWHEAD_SMALL_SNOWBALL_04 },
    { { 690.00f, 1335.00f }, RC_SNOWHEAD_TEMPLE_BRIDGE_ROOM_SMALL_SNOWBALL_01 },
    { { 645.00f, 1350.00f }, RC_SNOWHEAD_TEMPLE_BRIDGE_ROOM_SMALL_SNOWBALL_02 },
    { { 585.00f, 300.00f }, RC_SNOWHEAD_TEMPLE_CENTRAL_ROOM_LEVEL_2_SMALL_SNOWBALL_01 },
    { { 660.00f, 285.00f }, RC_SNOWHEAD_TEMPLE_CENTRAL_ROOM_LEVEL_2_SMALL_SNOWBALL_02 },
    { { 644.00f, 240.00f }, RC_SNOWHEAD_TEMPLE_CENTRAL_ROOM_LEVEL_2_SMALL_SNOWBALL_03 },
    { { -690.00f, -75.00f }, RC_SNOWHEAD_TEMPLE_CENTRAL_ROOM_LEVEL_2_SMALL_SNOWBALL_04 },
    { { -675.00f, 60.00f }, RC_SNOWHEAD_TEMPLE_CENTRAL_ROOM_LEVEL_2_SMALL_SNOWBALL_05 },
    { { -660.00f, -60.00f }, RC_SNOWHEAD_TEMPLE_CENTRAL_ROOM_LEVEL_2_SMALL_SNOWBALL_06 },
    { { 0.00f, 300.00f }, RC_SNOWHEAD_TEMPLE_CENTRAL_ROOM_LEVEL_3_LARGE_SNOWBALL_01 },
    { { 0.00f, 390.00f }, RC_SNOWHEAD_TEMPLE_CENTRAL_ROOM_LEVEL_3_LARGE_SNOWBALL_02 },
    { { 0.00f, 480.00f }, RC_SNOWHEAD_TEMPLE_CENTRAL_ROOM_LEVEL_3_LARGE_SNOWBALL_03 },
    { { 0.00f, 570.00f }, RC_SNOWHEAD_TEMPLE_CENTRAL_ROOM_LEVEL_3_LARGE_SNOWBALL_04 },
    { { -1269.00f, 621.00f }, RC_SNOWHEAD_TEMPLE_ICICLE_ROOM_LARGE_SNOWBALL_01 },
    { { -1290.00f, 555.00f }, RC_SNOWHEAD_TEMPLE_ICICLE_ROOM_SMALL_SNOWBALL_01 },
    { { -1200.00f, 555.00f }, RC_SNOWHEAD_TEMPLE_ICICLE_ROOM_SMALL_SNOWBALL_02 },
    { { -1185.00f, 630.00f }, RC_SNOWHEAD_TEMPLE_ICICLE_ROOM_SMALL_SNOWBALL_03 },
    { { -810.00f, 540.00f }, RC_SNOWHEAD_TEMPLE_ICICLE_ROOM_SMALL_SNOWBALL_04 },
    { { -870.00f, 510.00f }, RC_SNOWHEAD_TEMPLE_ICICLE_ROOM_SMALL_SNOWBALL_05 },
    { { -840.00f, 60.00f }, RC_SNOWHEAD_TEMPLE_SNOW_ROOM_SMALL_SNOWBALL_01 },
    { { -885.00f, 75.00f }, RC_SNOWHEAD_TEMPLE_SNOW_ROOM_SMALL_SNOWBALL_02 },
    { { -810.00f, 90.00f }, RC_SNOWHEAD_TEMPLE_SNOW_ROOM_SMALL_SNOWBALL_03 },
    { { -855.00f, 120.00f }, RC_SNOWHEAD_TEMPLE_SNOW_ROOM_SMALL_SNOWBALL_04 },
    { { -870.00f, 165.00f }, RC_SNOWHEAD_TEMPLE_SNOW_ROOM_SMALL_SNOWBALL_05 },
    { { -1290.00f, -615.00f }, RC_SNOWHEAD_TEMPLE_SNOW_ROOM_SMALL_SNOWBALL_06 },
    { { -1335.00f, -585.00f }, RC_SNOWHEAD_TEMPLE_SNOW_ROOM_SMALL_SNOWBALL_07 },
    { { -1305.00f, -570.00f }, RC_SNOWHEAD_TEMPLE_SNOW_ROOM_SMALL_SNOWBALL_08 },
    { { 427.00f, -613.00f }, RC_TWIN_ISLANDS_LARGE_SNOWBALL_01 },
    { { 65.00f, 345.00f }, RC_TWIN_ISLANDS_LARGE_SNOWBALL_02 },
    { { -179.00f, -876.00f }, RC_TWIN_ISLANDS_LARGE_SNOWBALL_03 },
    { { 851.00f, 643.00f }, RC_TWIN_ISLANDS_LARGE_SNOWBALL_04 },
    { { 371.00f, 778.00f }, RC_TWIN_ISLANDS_LARGE_SNOWBALL_05 },
    { { -563.00f, -523.00f }, RC_TWIN_ISLANDS_LARGE_SNOWBALL_06 },
    { { 1203.00f, 109.00f }, RC_TWIN_ISLANDS_LARGE_SNOWBALL_07 },
    { { -1029.00f, -25.00f }, RC_TWIN_ISLANDS_LARGE_SNOWBALL_08 },
    { { -880.00f, 497.00f }, RC_TWIN_ISLANDS_LARGE_SNOWBALL_09 },
    { { -395.00f, 677.00f }, RC_TWIN_ISLANDS_LARGE_SNOWBALL_10 },
    { { 5.00f, -560.00f }, RC_TWIN_ISLANDS_LARGE_SNOWBALL_11 },
    { { 60.00f, -46.00f }, RC_TWIN_ISLANDS_LARGE_SNOWBALL_12 },
    { { -1525.00f, 1264.00f }, RC_TWIN_ISLANDS_SMALL_SNOWBALL_01 },
    { { -1343.00f, 776.00f }, RC_TWIN_ISLANDS_SMALL_SNOWBALL_02 },
    { { 99.00f, -221.00f }, RC_TWIN_ISLANDS_SMALL_SNOWBALL_03 },
    { { -1349.00f, 114.00f }, RC_TWIN_ISLANDS_SMALL_SNOWBALL_04 },
    { { -1264.00f, 161.00f }, RC_TWIN_ISLANDS_SMALL_SNOWBALL_05 },
    { { -1326.00f, 172.00f }, RC_TWIN_ISLANDS_SMALL_SNOWBALL_06 },

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
            Matrix_Scale(30.0f, 30.0f, 30.0f, MTXMODE_APPLY);
            Rando::DrawItem(Rando::ConvertItem(randoSaveCheck.randoItemId, (RandoCheckId)CUSTOM_ITEM_PARAM), actor);
        });
}

void Rando::ActorBehavior::InitObjSnowballBehavior() {
    COND_VB_SHOULD(VB_SNOWBALL_DROP_COLLECTIBLE, IS_RANDO, {
        Actor* actor = va_arg(args, Actor*);

        if (actor->id != ACTOR_OBJ_SNOWBALL && actor->id != ACTOR_OBJ_SNOWBALL2) {
            return;
        }

        RandoCheckId randoCheckId = IdentifySnowball(actor->home.pos);

        if (randoCheckId == RC_UNKNOWN) {
            return;
        } else {
            SpawnSnowballDrop(actor->home.pos, randoCheckId);
        }

        *should = false;
    });

    COND_VB_SHOULD(VB_SNOWBALL_SET_FLAG, IS_RANDO, {
        Actor* actor = va_arg(args, Actor*);
        ActorId actorToSpawn = (ActorId)va_arg(args, s32);
        ObjSnowballActionFunc actorFunction = va_arg(args, ObjSnowballActionFunc);

        if (actorToSpawn == ACTOR_EN_JG) {
            *should = false;
            return;
        }

        if (actorFunction == func_80B02D58) {
            *should = false;
            return;
        }

        RandoCheckId randoCheckId = IdentifySnowball(actor->home.pos);

        if (randoCheckId == RC_UNKNOWN) {
            return;
        }

        SpawnSnowballDrop(actor->home.pos, randoCheckId);
        *should = false;
    });
}
