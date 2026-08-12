#ifndef ITEM_DESIRE_SENSOR_H
#define ITEM_DESIRE_SENSOR_H

// =============================================================================
// RETIRED — intentionally empty.
//
// The Desire Sensor is gone as an equippable C-button item. Its role was taken
// over by the QUARTZ OF MOTION, level 2 of the progressive Stone of Agony,
// which lives entirely outside the player code:
//
//   - ownership + category  -> NeiSaveData.quartz* (mods/nei_save.h)
//   - selection UI          -> A on the Stone of Agony slot of the OoT quest
//                              page (src/overlays/kaleido_scope/.../z_kaleido_collect.c)
//   - sensor brain          -> 2s2h/Rando/DesireCompass.{h,cpp}
//   - on-screen readout     -> 2s2h/Rando/DesireCompassHud.cpp
//
// This file is deliberately kept (empty) rather than deleted: mm/CMakeLists.txt
// globs "mods/*.h" with CONFIGURE_DEPENDS, so removing it changes the glob
// result and forces a full CMake re-configure — which this tree must not do
// (it wipes the custom linker setup, and the pinned vcpkg can no longer
// re-configure anyway). Leave it in place.
// =============================================================================

#endif // ITEM_DESIRE_SENSOR_H
