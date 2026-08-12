/* soh/Enhancements/randomizer/draw.h shim. Get-item draw funcs are ported
 * standalone (decoupled from 2ship's incompatible rando). Symbols surface at build. */
#ifndef NEI_SHIM_RANDO_DRAW_H
#define NEI_SHIM_RANDO_DRAW_H
#include "soh/_nei_compat_core.h"

// The get-item 3D-model draw funcs for the NEI custom items aren't ported yet (they come
// with the real 2s2h/Rando integration + the item DLs, like OoT). Stub each to NULL so the
// sNeiItems[] drawFunc column compiles — NULL just means "no custom rando get-item model".
#define Randomizer_DrawDesireSensor NULL
#define Randomizer_DrawHyliaGrace NULL
#define Randomizer_DrawZonaiPermafrost NULL
#define Randomizer_DrawDemiseDestruction NULL
#define Randomizer_DrawDekuLeaf NULL
#define Randomizer_DrawSwitchHook NULL
#define Randomizer_DrawMogmaMitts NULL
#define Randomizer_DrawGustJar NULL
#define Randomizer_DrawBallAndChain NULL
#define Randomizer_DrawWhip NULL
#define Randomizer_DrawSpinner NULL
#define Randomizer_DrawCaneOfSomaria NULL
#define Randomizer_DrawDominionRod NULL
#define Randomizer_DrawTimeGate NULL
#define Randomizer_DrawBombArrows NULL
#define Randomizer_DrawFireRod NULL
#define Randomizer_DrawIceRod NULL
#define Randomizer_DrawLightRod NULL
#define Randomizer_DrawBeetle NULL
#define Randomizer_DrawShovel NULL
#define Randomizer_DrawMinishCap NULL
#define Randomizer_DrawLantern NULL
#define Randomizer_DrawPokeball NULL
#define Randomizer_DrawBottleWithMagicMushroom NULL
// Elemental Wand (Skijer's NEI) — the real get-item model is picked in 2s2h/Rando/DrawItem.cpp by
// RandoItemId, same as every other NEI item here.
#define Randomizer_DrawElementalWand NULL
#endif
