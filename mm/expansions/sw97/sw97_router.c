/**
 * sw97_router.c - Master include file for SW97 spell/arrow actors
 *
 * Original actors: z64proto/sw97 team (Spaceworld '97 Experience)
 * Adapted for Ship of Harkinian (Shipwright)
 *
 * This file is #included from z_player.c to bring in all SW97 actor code
 * as part of the same translation unit.
 */

// Foundation
#include "expansions/sw97/sw97_compat.h"
#include "expansions/sw97/sw97_config.h"

// Harpoon C bridge — exposes Harpoon_NotifyVfxSpawn() for spawn hooks below.
#include "soh/Network/Harpoon/HarpoonBridge.h"

// Player behavior hooks (spell/arrow dispatch)
#include "expansions/sw97/player/sw97_player_behavior.inc.c"

// ============================================================
// Magic spell actors (prefixed to avoid COMDAT conflicts with OOT overlays)
// ============================================================

// --- MagicFire ---
#define MagicFire_Init Sw97_MagicFire_Init
#define MagicFire_Destroy Sw97_MagicFire_Destroy
#define MagicFire_Update Sw97_MagicFire_Update
#define MagicFire_Draw Sw97_MagicFire_Draw
#define MagicFire_UpdateBeforeCast Sw97_MagicFire_UpdateBeforeCast
#include "expansions/sw97/actors/spells/z_magic_fire.inc.c"
#undef MagicFire_Init
#undef MagicFire_Destroy
#undef MagicFire_Update
#undef MagicFire_Draw
#undef MagicFire_UpdateBeforeCast

// --- MagicIce ---
#define MagicIce_Init Sw97_MagicIce_Init
#define MagicIce_Destroy Sw97_MagicIce_Destroy
#define MagicIce_Update Sw97_MagicIce_Update
#define MagicIce_Draw Sw97_MagicIce_Draw
#include "expansions/sw97/actors/spells/z_magic_ice.inc.c"
#undef MagicIce_Init
#undef MagicIce_Destroy
#undef MagicIce_Update
#undef MagicIce_Draw

// --- MagicLight ---
#define MagicLight_Init Sw97_MagicLight_Init
#define MagicLight_Destroy Sw97_MagicLight_Destroy
#define MagicLight_Update Sw97_MagicLight_Update
#define MagicLight_Draw Sw97_MagicLight_Draw
#define MagicLight_SetupAction Sw97_MagicLight_SetupAction
#define MagicLight_GrowCylinder Sw97_MagicLight_GrowCylinder
#define MagicLight_End Sw97_MagicLight_End
#define MagicLight_Wait Sw97_MagicLight_Wait
#include "expansions/sw97/actors/spells/z_magic_light.inc.c"
#undef MagicLight_Init
#undef MagicLight_Destroy
#undef MagicLight_Update
#undef MagicLight_Draw
#undef MagicLight_SetupAction
#undef MagicLight_GrowCylinder
#undef MagicLight_End
#undef MagicLight_Wait

// --- MagicDark ---
#define MagicDark_Init Sw97_MagicDark_Init
#define MagicDark_Destroy Sw97_MagicDark_Destroy
#define MagicDark_Update Sw97_MagicDark_Update
#define MagicDark_Draw Sw97_MagicDark_Draw
#define MagicDark_OrbUpdate Sw97_MagicDark_OrbUpdate
#define MagicDark_OrbDraw Sw97_MagicDark_OrbDraw
#define MagicDark_DiamondUpdate Sw97_MagicDark_DiamondUpdate
#define MagicDark_DiamondDraw Sw97_MagicDark_DiamondDraw
#define MagicDark_DimLighting Sw97_MagicDark_DimLighting
#include "expansions/sw97/actors/spells/z_magic_dark.inc.c"
#undef MagicDark_Init
#undef MagicDark_Destroy
#undef MagicDark_Update
#undef MagicDark_Draw
#undef MagicDark_OrbUpdate
#undef MagicDark_OrbDraw
#undef MagicDark_DiamondUpdate
#undef MagicDark_DiamondDraw
#undef MagicDark_DimLighting

// --- MagicSoul ---
#define MagicSoul_Init Sw97_MagicSoul_Init
#define MagicSoul_Destroy Sw97_MagicSoul_Destroy
#define MagicSoul_Update Sw97_MagicSoul_Update
#define MagicSoul_Draw Sw97_MagicSoul_Draw
#define MagicSoul_OrbUpdate Sw97_MagicSoul_OrbUpdate
#define MagicSoul_OrbDraw Sw97_MagicSoul_OrbDraw
#define MagicSoul_DiamondUpdate Sw97_MagicSoul_DiamondUpdate
#define MagicSoul_DiamondDraw Sw97_MagicSoul_DiamondDraw
#define MagicSoul_DimLighting Sw97_MagicSoul_DimLighting
#define MagicSoul_UpdateFlash Sw97_MagicSoul_UpdateFlash
#include "expansions/sw97/actors/spells/z_magic_soul.inc.c"
#undef MagicSoul_Init
#undef MagicSoul_Destroy
#undef MagicSoul_Update
#undef MagicSoul_Draw
#undef MagicSoul_OrbUpdate
#undef MagicSoul_OrbDraw
#undef MagicSoul_DiamondUpdate
#undef MagicSoul_DiamondDraw
#undef MagicSoul_DimLighting
#undef MagicSoul_UpdateFlash

// --- MagicWind ---
#define MagicWind_Init Sw97_MagicWind_Init
#define MagicWind_Destroy Sw97_MagicWind_Destroy
#define MagicWind_Update Sw97_MagicWind_Update
#define MagicWind_Draw Sw97_MagicWind_Draw
#define MagicWind_SetupAction Sw97_MagicWind_SetupAction
#define MagicWind_WaitForTimer Sw97_MagicWind_WaitForTimer
#define MagicWind_Grow Sw97_MagicWind_Grow
#define MagicWind_WaitAtFullSize Sw97_MagicWind_WaitAtFullSize
#define MagicWind_FadeOut Sw97_MagicWind_FadeOut
#define MagicWind_Shrink Sw97_MagicWind_Shrink
#define MagicWind_UpdateAlpha Sw97_MagicWind_UpdateAlpha
#define MagicWind_OverrideLimbDraw Sw97_MagicWind_OverrideLimbDraw
#include "expansions/sw97/actors/spells/z_magic_wind.inc.c"
#undef MagicWind_Init
#undef MagicWind_Destroy
#undef MagicWind_Update
#undef MagicWind_Draw
#undef MagicWind_SetupAction
#undef MagicWind_WaitForTimer
#undef MagicWind_Grow
#undef MagicWind_WaitAtFullSize
#undef MagicWind_FadeOut
#undef MagicWind_Shrink
#undef MagicWind_UpdateAlpha
#undef MagicWind_OverrideLimbDraw

// ============================================================
// Arrow variant actors (prefixed to avoid COMDAT conflicts with OOT overlays)
// ============================================================

// --- ArrowFire ---
#define ArrowFire_Init Sw97_ArrowFire_Init
#define ArrowFire_Destroy Sw97_ArrowFire_Destroy
#define ArrowFire_Update Sw97_ArrowFire_Update
#define ArrowFire_Draw Sw97_ArrowFire_Draw
#define ArrowFire_SetupAction Sw97_ArrowFire_SetupAction
#define ArrowFire_Charge Sw97_ArrowFire_Charge
#define ArrowFire_Fly Sw97_ArrowFire_Fly
#define ArrowFire_Hit Sw97_ArrowFire_Hit
#include "expansions/sw97/actors/arrows/z_arrow_fire.inc.c"
#undef ArrowFire_Init
#undef ArrowFire_Destroy
#undef ArrowFire_Update
#undef ArrowFire_Draw
#undef ArrowFire_SetupAction
#undef ArrowFire_Charge
#undef ArrowFire_Fly
#undef ArrowFire_Hit

// --- ArrowIce ---
#define ArrowIce_Init Sw97_ArrowIce_Init
#define ArrowIce_Destroy Sw97_ArrowIce_Destroy
#define ArrowIce_Update Sw97_ArrowIce_Update
#define ArrowIce_Draw Sw97_ArrowIce_Draw
#define ArrowIce_SetupAction Sw97_ArrowIce_SetupAction
#define ArrowIce_Charge Sw97_ArrowIce_Charge
#define ArrowIce_Fly Sw97_ArrowIce_Fly
#define ArrowIce_Hit Sw97_ArrowIce_Hit
#include "expansions/sw97/actors/arrows/z_arrow_ice.inc.c"
#undef ArrowIce_Init
#undef ArrowIce_Destroy
#undef ArrowIce_Update
#undef ArrowIce_Draw
#undef ArrowIce_SetupAction
#undef ArrowIce_Charge
#undef ArrowIce_Fly
#undef ArrowIce_Hit

// --- ArrowLight ---
#define ArrowLight_Init Sw97_ArrowLight_Init
#define ArrowLight_Destroy Sw97_ArrowLight_Destroy
#define ArrowLight_Update Sw97_ArrowLight_Update
#define ArrowLight_Draw Sw97_ArrowLight_Draw
#define ArrowLight_SetupAction Sw97_ArrowLight_SetupAction
#define ArrowLight_Charge Sw97_ArrowLight_Charge
#define ArrowLight_Fly Sw97_ArrowLight_Fly
#define ArrowLight_Hit Sw97_ArrowLight_Hit
#include "expansions/sw97/actors/arrows/z_arrow_light.inc.c"
#undef ArrowLight_Init
#undef ArrowLight_Destroy
#undef ArrowLight_Update
#undef ArrowLight_Draw
#undef ArrowLight_SetupAction
#undef ArrowLight_Charge
#undef ArrowLight_Fly
#undef ArrowLight_Hit

// --- ArrowDark ---
#define ArrowDark_Init Sw97_ArrowDark_Init
#define ArrowDark_Destroy Sw97_ArrowDark_Destroy
#define ArrowDark_Update Sw97_ArrowDark_Update
#define ArrowDark_Draw Sw97_ArrowDark_Draw
#define ArrowDark_SetupAction Sw97_ArrowDark_SetupAction
#define ArrowDark_Charge Sw97_ArrowDark_Charge
#define ArrowDark_Fly Sw97_ArrowDark_Fly
#define ArrowDark_Hit Sw97_ArrowDark_Hit
#include "expansions/sw97/actors/arrows/z_arrow_dark.inc.c"
#undef ArrowDark_Init
#undef ArrowDark_Destroy
#undef ArrowDark_Update
#undef ArrowDark_Draw
#undef ArrowDark_SetupAction
#undef ArrowDark_Charge
#undef ArrowDark_Fly
#undef ArrowDark_Hit

// --- ArrowSoul ---
#define ArrowSoul_Init Sw97_ArrowSoul_Init
#define ArrowSoul_Destroy Sw97_ArrowSoul_Destroy
#define ArrowSoul_Update Sw97_ArrowSoul_Update
#define ArrowSoul_Draw Sw97_ArrowSoul_Draw
#include "expansions/sw97/actors/arrows/z_arrow_soul.inc.c"
#undef ArrowSoul_Init
#undef ArrowSoul_Destroy
#undef ArrowSoul_Update
#undef ArrowSoul_Draw

// --- ArrowWind ---
#define ArrowWind_Init Sw97_ArrowWind_Init
#define ArrowWind_Destroy Sw97_ArrowWind_Destroy
#define ArrowWind_Update Sw97_ArrowWind_Update
#define ArrowWind_Draw Sw97_ArrowWind_Draw
#include "expansions/sw97/actors/arrows/z_arrow_wind.inc.c"
#undef ArrowWind_Init
#undef ArrowWind_Destroy
#undef ArrowWind_Update
#undef ArrowWind_Draw

// ============================================================
// Skijer's NEI: real ActorProfiles for the six spell actors.
// SoH registered these through ActorDB at runtime; 2ship instead gives them
// static slots in the actor table (tables/actor_table.h ACTOR_SW97_MAGIC_*,
// rows point at these profiles). Placed here because the Magic* struct types
// and the prefixed Sw97_Magic*_* functions are only visible at the end of
// this router include chain. Flags mirror the actors' own FLAGS defines
// (0x02000010/30 = OoT update/draw-culling-disabled bits) using MM's names.
// ============================================================
#define SW97_SPELL_ACTOR_FLAGS (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED)

ActorProfile Sw97_MagicWind_Profile = {
    /**/ ACTOR_SW97_MAGIC_WIND,
    /**/ ACTORCAT_ITEMACTION,
    /**/ SW97_SPELL_ACTOR_FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(MagicWind),
    /**/ Sw97_MagicWind_Init,
    /**/ Sw97_MagicWind_Destroy,
    /**/ Sw97_MagicWind_Update,
    /**/ Sw97_MagicWind_Draw,
};

ActorProfile Sw97_MagicSoul_Profile = {
    /**/ ACTOR_SW97_MAGIC_SOUL,
    /**/ ACTORCAT_ITEMACTION,
    /**/ SW97_SPELL_ACTOR_FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(MagicSoul),
    /**/ Sw97_MagicSoul_Init,
    /**/ Sw97_MagicSoul_Destroy,
    /**/ Sw97_MagicSoul_Update,
    /**/ Sw97_MagicSoul_Draw,
};

ActorProfile Sw97_MagicDark_Profile = {
    /**/ ACTOR_SW97_MAGIC_DARK,
    /**/ ACTORCAT_ITEMACTION,
    /**/ SW97_SPELL_ACTOR_FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(MagicDark),
    /**/ Sw97_MagicDark_Init,
    /**/ Sw97_MagicDark_Destroy,
    /**/ Sw97_MagicDark_Update,
    /**/ Sw97_MagicDark_Draw,
};

ActorProfile Sw97_MagicIce_Profile = {
    /**/ ACTOR_SW97_MAGIC_ICE,
    /**/ ACTORCAT_ITEMACTION,
    /**/ SW97_SPELL_ACTOR_FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(MagicIce),
    /**/ Sw97_MagicIce_Init,
    /**/ Sw97_MagicIce_Destroy,
    /**/ Sw97_MagicIce_Update,
    /**/ Sw97_MagicIce_Draw,
};

ActorProfile Sw97_MagicLight_Profile = {
    /**/ ACTOR_SW97_MAGIC_LIGHT,
    /**/ ACTORCAT_ITEMACTION,
    /**/ SW97_SPELL_ACTOR_FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(MagicLight),
    /**/ Sw97_MagicLight_Init,
    /**/ Sw97_MagicLight_Destroy,
    /**/ Sw97_MagicLight_Update,
    /**/ Sw97_MagicLight_Draw,
};

ActorProfile Sw97_MagicFire_Profile = {
    /**/ ACTOR_SW97_MAGIC_FIRE,
    /**/ ACTORCAT_ITEMACTION,
    /**/ SW97_SPELL_ACTOR_FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(MagicFire),
    /**/ Sw97_MagicFire_Init,
    /**/ Sw97_MagicFire_Destroy,
    /**/ Sw97_MagicFire_Update,
    /**/ Sw97_MagicFire_Draw,
};

// Skijer's NEI: the THREE vanilla OoT spells as their own actors. They reuse the medallion actors'
// functions but register under their OWN ids — actor->id (== profile->id, z_actor.c:4009) is what
// the shared functions read to pick the vanilla behavior (Din's == Fire dome identical to medallion;
// Nayru's == blue diamond + full invincibility vs Shadow's black diamond + stealth; Farore's ==
// cosmetic swirl, NO tornado, vs the Forest Medallion tornado).
ActorProfile Oot_DinsFire_Profile = {
    /**/ ACTOR_OOT_DINS_FIRE,
    /**/ ACTORCAT_ITEMACTION,
    /**/ SW97_SPELL_ACTOR_FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(MagicFire),
    /**/ Sw97_MagicFire_Init,
    /**/ Sw97_MagicFire_Destroy,
    /**/ Sw97_MagicFire_Update,
    /**/ Sw97_MagicFire_Draw,
};

ActorProfile Oot_NayrusLove_Profile = {
    /**/ ACTOR_OOT_NAYRUS_LOVE,
    /**/ ACTORCAT_ITEMACTION,
    /**/ SW97_SPELL_ACTOR_FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(MagicDark),
    /**/ Sw97_MagicDark_Init,
    /**/ Sw97_MagicDark_Destroy,
    /**/ Sw97_MagicDark_Update,
    /**/ Sw97_MagicDark_Draw,
};

ActorProfile Oot_FaroresWind_Profile = {
    /**/ ACTOR_OOT_FARORES_WIND,
    /**/ ACTORCAT_ITEMACTION,
    /**/ SW97_SPELL_ACTOR_FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(MagicWind),
    /**/ Sw97_MagicWind_Init,
    /**/ Sw97_MagicWind_Destroy,
    /**/ Sw97_MagicWind_Update,
    /**/ Sw97_MagicWind_Draw,
};
