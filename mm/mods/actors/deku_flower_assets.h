/**
 * deku_flower_assets.h - MM Gold Deku Flower DL paths from mm.o2r
 *
 * The "gold" Deku flower is the launching flower Deku Link dives into to be
 * shot upward — in MM these are placed in scenes; we spawn one dynamically
 * each time Deku uses the Deku Leaf on the ground (custom enhancement, not
 * present in MM original which relied on pre-placed scene flowers).
 *
 * DLs live in MM's gameplay_keep object (packed into mm.o2r).
 *
 * NOTE: gGoldDekuFlowerIdleDL is ALREADY defined by the canonical asset header
 * objects/gameplay_keep/gameplay_keep.h (as `static const ALIGN_ASSET(2) char
 * gGoldDekuFlowerIdleDL[]`, the same OTR path). Defining it a second time here
 * produced a "redefinition / multiple initialization" error in any TU that
 * includes both this header and gameplay_keep.h (e.g. mm_player_form.cpp, which
 * includes gameplay_keep.h AND this file). So this header now just pulls in the
 * canonical definition instead of duplicating it — a single source of truth.
 */

#ifndef DEKU_FLOWER_ASSETS_H
#define DEKU_FLOWER_ASSETS_H

// Canonical MM definition of gGoldDekuFlowerIdleDL (and the full gameplay_keep
// symbol set). This is the same header the vanilla Obj_Etcetera flower actor
// (z_obj_etcetera.c) uses for the DL.
#include "objects/gameplay_keep/gameplay_keep.h"

#endif // DEKU_FLOWER_ASSETS_H
