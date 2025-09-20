#ifndef RANDO_CLOCK_SHUFFLE_H
#define RANDO_CLOCK_SHUFFLE_H

#include "Rando/Types.h"

extern "C" {
#include "variables.h"
#include "functions.h"
}

#define RANDO_SAVE_CHECKS gSaveContext.save.shipSaveInfo.rando.randoSaveChecks

namespace Rando {
namespace ClockItems {

RandoItemId GetClockItemFromHalfDayIndex(int halfDayIndex);

int GetHalfDayIndexFromClockItem(RandoItemId clockItemId);
int FindEarliestOwnedHalfDay(bool searchFromEnd = false);

bool DoesPlayerOwnHalfDay(int halfDayIndex);

void GivePlayerHalfDay(int halfDayIndex);
void TakeAwayHalfDay(int halfDayIndex);

} // namespace ClockItems

namespace ClockShuffle {

void OnFileLoad();

} // namespace ClockShuffle
} // namespace Rando

#endif // RANDO_CLOCK_SHUFFLE_H
