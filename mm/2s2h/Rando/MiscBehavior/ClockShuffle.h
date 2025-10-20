#ifndef RANDO_CLOCK_SHUFFLE_H
#define RANDO_CLOCK_SHUFFLE_H

#include "Rando/Types.h"
#include <string>
#include <vector>

extern "C" {
#include "variables.h"
#include "functions.h"
}

namespace Rando {
namespace ClockItems {

RandoItemId GetClockItemFromHalfDayIndex(int halfDayIndex);
int GetHalfDayIndexFromClockItem(RandoItemId clockItemId);
int FindEarliestOwnedHalfDay(bool searchFromEnd = false);
u8 GetAllOwnedHalfDaysMask();

} // namespace ClockItems

namespace ClockShuffle {

void InitializeFileClocks(std::vector<RandoItemId>& itemPool);
void OnFileLoad();
void SetTimeToHalfDayStart(int halfDayIndex);

bool IsTimeOwnedForClockShuffle(s32 day, u16 time);
int GetHalfDayIndexFromTime(s32 day, u16 time);
std::string GetTimeDescriptionForMessage(s32 day, u16 time);

} // namespace ClockShuffle
} // namespace Rando

#endif // RANDO_CLOCK_SHUFFLE_H
