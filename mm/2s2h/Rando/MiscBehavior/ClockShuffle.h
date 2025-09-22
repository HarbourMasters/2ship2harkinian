#ifndef RANDO_CLOCK_SHUFFLE_H
#define RANDO_CLOCK_SHUFFLE_H

#include "Rando/Types.h"

extern "C" {
#include "variables.h"
#include "functions.h"
}

namespace Rando {
namespace ClockItems {

RandoItemId GetClockItemFromHalfDayIndex(int halfDayIndex);

int GetHalfDayIndexFromClockItem(RandoItemId clockItemId);
int FindEarliestOwnedHalfDay(bool searchFromEnd = false);

} // namespace ClockItems

namespace ClockShuffle {

void OnFileLoad();
void SetTimeToHalfDayStart(int halfDayIndex);

} // namespace ClockShuffle
} // namespace Rando

#endif // RANDO_CLOCK_SHUFFLE_H
