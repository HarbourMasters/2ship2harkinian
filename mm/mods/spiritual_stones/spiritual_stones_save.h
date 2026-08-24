// Spiritual Stones — per-save custom state (2ship ShipSaveInfo embed).
//
// Mirrors the nei_save.h pattern: a small POD persisted INSIDE 2ship's per-save
// struct (gSaveContext.save.shipSaveInfo.spiritualStones) and serialized by
// 2s2h/BenJsonConversions.hpp. 2ship has no SoH SaveManager, so this replaces the
// SaveManager AddInit/Save/Load + SaveData/LoadData section the SoH original used.
//
// Only fixed-width types here (like nei_save.h) so this header can be included by
// include/z64save.h without pulling in z64.h (which would create an include cycle
// with spiritual_stones.h).
#ifndef SPIRITUAL_STONES_SAVE_H
#define SPIRITUAL_STONES_SAVE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SPIRITUAL_STONE_SAVE_COUNT 3

// One warp point per stone. entranceId == -1 means "no warp set".
typedef struct SpiritualStoneWarpSave {
    int32_t entranceId; // -1 = unset
    int16_t sceneId;
    int8_t roomNum;
    int16_t rotY;
    float posX;
    float posY;
    float posZ;
} SpiritualStoneWarpSave;

typedef struct SpiritualStonesSaveData {
    uint8_t passive[SPIRITUAL_STONE_SAVE_COUNT]; // 0/1 buff toggle per stone
    SpiritualStoneWarpSave warp[SPIRITUAL_STONE_SAVE_COUNT];
} SpiritualStonesSaveData;

#ifdef __cplusplus
}
#endif

#endif // SPIRITUAL_STONES_SAVE_H
