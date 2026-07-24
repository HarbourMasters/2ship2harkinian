/**
 * sw97_init.cpp - SW97 spell/arrow actor registration
 *
 * Original actors: z64proto/sw97 team (Spaceworld '97 Experience)
 * Adapted for Ship of Harkinian (Shipwright)
 *
 * 2ship port note:
 *   SoH registered the 12 SW97 custom actors (6 spells + 6 arrows) at runtime via
 *   SoH's `ActorDB` (ActorDB::Instance->AddEntry). 2ship has NO ActorDB — instead
 *   (Skijer's NEI) the six SPELL actors got STATIC rows appended to the vanilla
 *   actor table (tables/actor_table.h, ACTOR_SW97_MAGIC_*; ActorProfiles at the end
 *   of sw97_router.c), so gSw97ActorId_Magic* now hold real constant IDs and
 *   Sw97_TrySpawnMagicSpell spawns them like any native actor. The six ARROW
 *   variants remain sentinel -1: they fire through the vanilla EN_ARROW actor with
 *   ARROW_TYPE_SW97_* params, so they never needed IDs of their own.
 */

// Include headers outside extern "C" — they transitively pull in C++ headers
#include "global.h"

extern "C" {

// Forward declarations for actor lifecycle functions (defined in ported .c files)
// These are compiled in z_player.c's TU via sw97_router.c
// All names are Sw97_ prefixed via #define in sw97_router.c

// Magic spell actors
extern void Sw97_MagicFire_Init(Actor* thisx, PlayState* play);
extern void Sw97_MagicFire_Destroy(Actor* thisx, PlayState* play);
extern void Sw97_MagicFire_Update(Actor* thisx, PlayState* play);
extern void Sw97_MagicFire_Draw(Actor* thisx, PlayState* play);

extern void Sw97_MagicIce_Init(Actor* thisx, PlayState* play);
extern void Sw97_MagicIce_Destroy(Actor* thisx, PlayState* play);
extern void Sw97_MagicIce_Update(Actor* thisx, PlayState* play);
extern void Sw97_MagicIce_Draw(Actor* thisx, PlayState* play);

extern void Sw97_MagicLight_Init(Actor* thisx, PlayState* play);
extern void Sw97_MagicLight_Destroy(Actor* thisx, PlayState* play);
extern void Sw97_MagicLight_Update(Actor* thisx, PlayState* play);
extern void Sw97_MagicLight_Draw(Actor* thisx, PlayState* play);

extern void Sw97_MagicDark_Init(Actor* thisx, PlayState* play);
extern void Sw97_MagicDark_Destroy(Actor* thisx, PlayState* play);
extern void Sw97_MagicDark_Update(Actor* thisx, PlayState* play);
extern void Sw97_MagicDark_Draw(Actor* thisx, PlayState* play);

extern void Sw97_MagicSoul_Init(Actor* thisx, PlayState* play);
extern void Sw97_MagicSoul_Destroy(Actor* thisx, PlayState* play);
extern void Sw97_MagicSoul_Update(Actor* thisx, PlayState* play);
extern void Sw97_MagicSoul_Draw(Actor* thisx, PlayState* play);

extern void Sw97_MagicWind_Init(Actor* thisx, PlayState* play);
extern void Sw97_MagicWind_Destroy(Actor* thisx, PlayState* play);
extern void Sw97_MagicWind_Update(Actor* thisx, PlayState* play);
extern void Sw97_MagicWind_Draw(Actor* thisx, PlayState* play);

// Arrow variant actors
extern void Sw97_ArrowFire_Init(Actor* thisx, PlayState* play);
extern void Sw97_ArrowFire_Destroy(Actor* thisx, PlayState* play);
extern void Sw97_ArrowFire_Update(Actor* thisx, PlayState* play);
extern void Sw97_ArrowFire_Draw(Actor* thisx, PlayState* play);

extern void Sw97_ArrowIce_Init(Actor* thisx, PlayState* play);
extern void Sw97_ArrowIce_Destroy(Actor* thisx, PlayState* play);
extern void Sw97_ArrowIce_Update(Actor* thisx, PlayState* play);
extern void Sw97_ArrowIce_Draw(Actor* thisx, PlayState* play);

extern void Sw97_ArrowLight_Init(Actor* thisx, PlayState* play);
extern void Sw97_ArrowLight_Destroy(Actor* thisx, PlayState* play);
extern void Sw97_ArrowLight_Update(Actor* thisx, PlayState* play);
extern void Sw97_ArrowLight_Draw(Actor* thisx, PlayState* play);

extern void Sw97_ArrowDark_Init(Actor* thisx, PlayState* play);
extern void Sw97_ArrowDark_Destroy(Actor* thisx, PlayState* play);
extern void Sw97_ArrowDark_Update(Actor* thisx, PlayState* play);
extern void Sw97_ArrowDark_Draw(Actor* thisx, PlayState* play);

extern void Sw97_ArrowSoul_Init(Actor* thisx, PlayState* play);
extern void Sw97_ArrowSoul_Destroy(Actor* thisx, PlayState* play);
extern void Sw97_ArrowSoul_Update(Actor* thisx, PlayState* play);
extern void Sw97_ArrowSoul_Draw(Actor* thisx, PlayState* play);

extern void Sw97_ArrowWind_Init(Actor* thisx, PlayState* play);
extern void Sw97_ArrowWind_Destroy(Actor* thisx, PlayState* play);
extern void Sw97_ArrowWind_Update(Actor* thisx, PlayState* play);
extern void Sw97_ArrowWind_Draw(Actor* thisx, PlayState* play);

// Runtime actor IDs (globals accessed from C code).
// Skijer's NEI: the six SPELL actors now have STATIC slots in the actor table
// (tables/actor_table.h ACTOR_SW97_MAGIC_*, profiles at the end of sw97_router.c),
// so their IDs are real constants — every Sw97_TrySpawnMagicSpell short-circuit on
// a negative ID goes live. The arrow variants remain sentinel -1 (they are spawned
// through the vanilla EN_ARROW path with ARROW_TYPE_SW97_* params instead).
s16 gSw97ActorId_MagicFire = ACTOR_SW97_MAGIC_FIRE;
s16 gSw97ActorId_MagicIce = ACTOR_SW97_MAGIC_ICE;
s16 gSw97ActorId_MagicLight = ACTOR_SW97_MAGIC_LIGHT;
s16 gSw97ActorId_MagicDark = ACTOR_SW97_MAGIC_DARK;
s16 gSw97ActorId_MagicSoul = ACTOR_SW97_MAGIC_SOUL;
s16 gSw97ActorId_MagicWind = ACTOR_SW97_MAGIC_WIND;
s16 gSw97ActorId_ArrowFire = -1;
s16 gSw97ActorId_ArrowIce = -1;
s16 gSw97ActorId_ArrowLight = -1;
s16 gSw97ActorId_ArrowDark = -1;
s16 gSw97ActorId_ArrowSoul = -1;
s16 gSw97ActorId_ArrowWind = -1;

} // extern "C"

// SW97 actor registration.
//
// 2ship has no ActorDB / dynamic custom-actor table (SoH-only). Without a runtime
// registry there is no valid actor ID to hand back, so we leave every
// `gSw97ActorId_*` at its sentinel -1. All SW97 spawn sites already short-circuit
// on a negative ID (e.g. Sw97_TrySpawnMagicSpell returns NULL), so this is an inert
// but well-defined no-op rather than a crash. The per-actor lifecycle functions
// (Sw97_*_Init/Update/Draw/Destroy) are still compiled in via sw97_router.c and
// referenced below so they are not dropped as unused — they become live the moment
// a 2ship-native registration path assigns real IDs to these globals.
void Sw97_RegisterActors() {
    // Reference the lifecycle functions so the linker keeps them and the intent
    // (which function backs each ID) stays documented. No-op pointers; not invoked.
    static void (*const kSw97ActorFns[])(Actor*, PlayState*) = {
        Sw97_MagicFire_Init,  Sw97_MagicFire_Destroy,  Sw97_MagicFire_Update,  Sw97_MagicFire_Draw,
        Sw97_MagicIce_Init,   Sw97_MagicIce_Destroy,   Sw97_MagicIce_Update,   Sw97_MagicIce_Draw,
        Sw97_MagicLight_Init, Sw97_MagicLight_Destroy, Sw97_MagicLight_Update, Sw97_MagicLight_Draw,
        Sw97_MagicDark_Init,  Sw97_MagicDark_Destroy,  Sw97_MagicDark_Update,  Sw97_MagicDark_Draw,
        Sw97_MagicSoul_Init,  Sw97_MagicSoul_Destroy,  Sw97_MagicSoul_Update,  Sw97_MagicSoul_Draw,
        Sw97_MagicWind_Init,  Sw97_MagicWind_Destroy,  Sw97_MagicWind_Update,  Sw97_MagicWind_Draw,
        Sw97_ArrowFire_Init,  Sw97_ArrowFire_Destroy,  Sw97_ArrowFire_Update,  Sw97_ArrowFire_Draw,
        Sw97_ArrowIce_Init,   Sw97_ArrowIce_Destroy,   Sw97_ArrowIce_Update,   Sw97_ArrowIce_Draw,
        Sw97_ArrowLight_Init, Sw97_ArrowLight_Destroy, Sw97_ArrowLight_Update, Sw97_ArrowLight_Draw,
        Sw97_ArrowDark_Init,  Sw97_ArrowDark_Destroy,  Sw97_ArrowDark_Update,  Sw97_ArrowDark_Draw,
        Sw97_ArrowSoul_Init,  Sw97_ArrowSoul_Destroy,  Sw97_ArrowSoul_Update,  Sw97_ArrowSoul_Draw,
        Sw97_ArrowWind_Init,  Sw97_ArrowWind_Destroy,  Sw97_ArrowWind_Update,  Sw97_ArrowWind_Draw,
    };
    (void)kSw97ActorFns;
    // IDs intentionally remain -1 (no ActorDB on 2ship). See note above.
}

void Sw97_RegisterHooks() {
    // No hooks needed — medallion equipping is handled by KaleidoScope and z_player.c
}
