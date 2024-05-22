#include "Infinite.h"
#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"
#include "variables.h"

void RegisterInfiniteCheats() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnGameStateUpdate>([]() {
        if (gPlayState == nullptr)
            return;

        if (CVarGetInteger("gCheats.InfiniteHealth", 0)) {
            gSaveContext.save.saveInfo.playerData.health = gSaveContext.save.saveInfo.playerData.healthCapacity;
        }

        if (CVarGetInteger("gCheats.InfiniteMagic", 0)) {
            uint8_t magicLevel = gSaveContext.save.saveInfo.playerData.magicLevel;
            if (magicLevel == 1) {
                gSaveContext.save.saveInfo.playerData.magic = MAGIC_NORMAL_METER;
            } else if (magicLevel == 2) {
                gSaveContext.save.saveInfo.playerData.magic = MAGIC_DOUBLE_METER;
            }
        }

        if (CVarGetInteger("gCheats.InfiniteRupees", 0)) {
            gSaveContext.save.saveInfo.playerData.rupees = CUR_CAPACITY(UPG_WALLET);
        }

        if (CVarGetInteger("gCheats.InfiniteConsumables", 0)) {
            AMMO(ITEM_BOW) = CUR_CAPACITY(UPG_QUIVER);
            AMMO(ITEM_BOMB) = CUR_CAPACITY(UPG_BOMB_BAG);
            AMMO(ITEM_BOMBCHU) = CUR_CAPACITY(UPG_BOMB_BAG);
            AMMO(ITEM_DEKU_STICK) = CUR_CAPACITY(UPG_DEKU_STICKS);
            AMMO(ITEM_DEKU_NUT) = CUR_CAPACITY(UPG_DEKU_NUTS);
            AMMO(ITEM_MAGIC_BEANS) = 20;
            AMMO(ITEM_POWDER_KEG) = 1;
        }
    });
}
