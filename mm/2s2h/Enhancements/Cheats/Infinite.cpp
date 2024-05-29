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
            uint32_t ammoItems[7] = { ITEM_BOW,      ITEM_BOMB,        ITEM_BOMBCHU,   ITEM_DEKU_STICK,
                                      ITEM_DEKU_NUT, ITEM_MAGIC_BEANS, ITEM_POWDER_KEG };
            uint32_t upgCapacity[7] = { UPG_QUIVER,    UPG_BOMB_BAG,     UPG_BOMB_BAG,   UPG_DEKU_STICKS,
                                        UPG_DEKU_NUTS, ITEM_MAGIC_BEANS, ITEM_POWDER_KEG };
            for (int i = 0; i <= sizeof(ammoItems) / sizeof(ammoItems[0]); i++) {
                if (INV_CONTENT(ammoItems[i]) == ammoItems[i]) {
                    if (ammoItems[i] == ITEM_MAGIC_BEANS) {
                        AMMO(ammoItems[i]) = 20;
                    } else if (ammoItems[i] == ITEM_POWDER_KEG) {
                        AMMO(ammoItems[i]) = 1;
                    } else {
                        AMMO(ammoItems[i]) = CUR_CAPACITY(upgCapacity[i]);
                    }
                } else if (ammoItems[i] == ITEM_POWDER_KEG) {
                    AMMO(ammoItems[i]) = 0;
                }
            };
        }
    });
}
