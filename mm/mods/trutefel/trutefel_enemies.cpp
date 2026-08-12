/**
 * trutefel_enemies.cpp - Trutefel's three custom enemies (Miniblin / Molmauk-Hammergeist /
 * Scissors Beetle), ported from the modern OoT decomp (soh/mods/actors/trutefel) to 2ship's MM.
 *
 * Host TU (same pattern as boss_remains.cpp): the compiled assets and the three actor .c files
 * are unity-#included INSIDE the extern "C" block below so their ActorProfile symbols
 * (EnMiniblin_Profile / EnHammergeist_Profile / EnSbeetle_Profile) get C linkage and match the
 * DEFINE_ACTOR rows appended to mm/include/tables/actor_table.h (0x2C2 / 0x2C3 / 0x2C4).
 *
 * Assets: mm/mods/trutefel/assets/object_*_assets.inc.c hold the compiled FlexSkeletonHeaders
 * (StandardLimb dLists are __OTR__objects/trutefel/... path char arrays — resolved by 2ship's
 * gSPDisplayList/gSPSegment shims at draw time) and the raw AnimationHeader frame data (accepted
 * as-is by 2ship's sig-checking SkelAnime, same as mm/expansions/ssbb's pikachu). The textures
 * and meshes live in trutefel-enemies.o2r (x64/Release/mods/) under objects/trutefel/<object>/.
 * Each actor's Init gates on ResourceMgr_FileExists and self-kills if the archive is missing.
 *
 * None of the .c/.inc.c files are in CMake/vcxproj on their own — they compile as part of THIS
 * TU (only trutefel_enemies.cpp is listed in 2ship.vcxproj).
 */

// OPEN_DISPS / CLOSE_DISPS redeclare these two symbols inline at each call site; in a C++ TU that
// takes C++ linkage unless a C declaration exists at file scope. Force the C symbols (same trick
// as boss_remains.cpp / spiritual_stones.cpp / PropHunt.cpp) so the macro's redeclaration matches
// and links.
extern "C" {
void FrameInterpolation_RecordOpenChild(const void* a, int b);
void FrameInterpolation_RecordCloseChild(void);
}

extern "C" {
#include "z64.h"
#include "functions.h"
#include "macros.h"
#include "variables.h"
#include "align_asset_macro.h" // ALIGN_ASSET used by the assets .inc.c files

// Compiled skeletons + animations FIRST (the actor files reference their symbols).
#include "assets/object_miniblin_assets.inc.c"
#include "assets/object_hammergeist_assets.inc.c"
#include "assets/object_sbeetle_assets.inc.c"

// The three ported actors (see each file's header comment for the OoT->MM mapping notes).
#include "actors/z_en_miniblin.c"
#include "actors/z_en_hammergeist.c"
#include "actors/z_en_sbeetle.c"
}
