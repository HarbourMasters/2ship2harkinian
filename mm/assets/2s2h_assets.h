#pragma once

#include "align_asset_macro.h"

// This file is manually made
// When new assets are added to the 2ship.otr file
// We need to add the aligned version of the resource names here and use in code
// On Mac, not using aligned resource names was causing crashes in release builds

// textures
#define dgDPad "__OTR__textures/parameter_static/gDPad"
static const ALIGN_ASSET(2) char gDPadTex[] = dgDPad;

#define dgArrowUp "__OTR__textures/parameter_static/gArrowUp"
static const ALIGN_ASSET(2) char gArrowUpTex[] = dgArrowUp;

#define dgArrowDown "__OTR__textures/parameter_static/gArrowDown"
static const ALIGN_ASSET(2) char gArrowDownTex[] = dgArrowDown;

#define dgTriforcePiece "__OTR__textures/parameter_static/gTriforcePiece"
static const ALIGN_ASSET(2) char gTriforcePieceTex[] = dgTriforcePiece;

#define dgFlippers "__OTR__textures/parameter_static/gFlippers"
static const ALIGN_ASSET(2) char gFlippersTex[] = dgFlippers;

// SM64 Mario Mode icons (copied from Shipwright/soh/assets/custom/textures/icon_item_custom/).
// Used by the C-Down slot when gSm64MarioMaskForce is on (Mario Mask) and by
// the D-pad slot icon overlays when gSm64Mario is on (Wing/Metal/Vanish caps).
#define dgItemIconMarioMaskTex "__OTR__textures/icon_item_custom/gItemIconMarioMaskTex"
static const ALIGN_ASSET(2) char gItemIconMarioMaskTex[] = dgItemIconMarioMaskTex;

#define dgItemIconWingCapTex "__OTR__textures/icon_item_custom/gItemIconWingCapTex"
static const ALIGN_ASSET(2) char gItemIconWingCapTex[] = dgItemIconWingCapTex;

#define dgItemIconMetalCapTex "__OTR__textures/icon_item_custom/gItemIconMetalCapTex"
static const ALIGN_ASSET(2) char gItemIconMetalCapTex[] = dgItemIconMetalCapTex;

#define dgItemIconVanishCapTex "__OTR__textures/icon_item_custom/gItemIconVanishCapTex"
static const ALIGN_ASSET(2) char gItemIconVanishCapTex[] = dgItemIconVanishCapTex;

// ─── Skijer's NEI custom item icons (from mm/assets/custom/textures/icon_item_custom/).
// Used as OTR-path char[] by sNeiItems[].icon in extended_player.c / extended_inventory.c.
#define dgItemIconRocsFeatherTex "__OTR__textures/icon_item_custom/gItemIconRocsFeatherTex"
static const ALIGN_ASSET(2) char gItemIconRocsFeatherTex[] = dgItemIconRocsFeatherTex;
#define dgItemIconRocsCapeTex "__OTR__textures/icon_item_custom/gItemIconRocsCapeTex"
static const ALIGN_ASSET(2) char gItemIconRocsCapeTex[] = dgItemIconRocsCapeTex;
#define dgItemIconDesireSensorTex "__OTR__textures/icon_item_custom/gItemIconDesireSensorTex"
static const ALIGN_ASSET(2) char gItemIconDesireSensorTex[] = dgItemIconDesireSensorTex;
#define dgItemIconHyliaGraceTex "__OTR__textures/icon_item_custom/gItemIconHyliaGraceTex"
static const ALIGN_ASSET(2) char gItemIconHyliaGraceTex[] = dgItemIconHyliaGraceTex;
#define dgItemIconZonaiPermafrostTex "__OTR__textures/icon_item_custom/gItemIconZonaiPermafrostTex"
static const ALIGN_ASSET(2) char gItemIconZonaiPermafrostTex[] = dgItemIconZonaiPermafrostTex;
#define dgItemIconDemiseDestructionTex "__OTR__textures/icon_item_custom/gItemIconDemiseDestructionTex"
static const ALIGN_ASSET(2) char gItemIconDemiseDestructionTex[] = dgItemIconDemiseDestructionTex;
#define dgItemIconDekuLeafTex "__OTR__textures/icon_item_custom/gItemIconDekuLeafTex"
static const ALIGN_ASSET(2) char gItemIconDekuLeafTex[] = dgItemIconDekuLeafTex;
#define dgItemIconSwitchHookTex "__OTR__textures/icon_item_custom/gItemIconSwitchHookTex"
static const ALIGN_ASSET(2) char gItemIconSwitchHookTex[] = dgItemIconSwitchHookTex;
#define dgItemIconMogmaMittsTex "__OTR__textures/icon_item_custom/gItemIconMogmaMittsTex"
static const ALIGN_ASSET(2) char gItemIconMogmaMittsTex[] = dgItemIconMogmaMittsTex;
#define dgItemIconGustJarTex "__OTR__textures/icon_item_custom/gItemIconGustJarTex"
static const ALIGN_ASSET(2) char gItemIconGustJarTex[] = dgItemIconGustJarTex;
#define dgItemIconBallAndChainTex "__OTR__textures/icon_item_custom/gItemIconBallAndChainTex"
static const ALIGN_ASSET(2) char gItemIconBallAndChainTex[] = dgItemIconBallAndChainTex;
#define dgItemIconWhipTex "__OTR__textures/icon_item_custom/gItemIconWhipTex"
static const ALIGN_ASSET(2) char gItemIconWhipTex[] = dgItemIconWhipTex;
#define dgItemIconSpinnerTex "__OTR__textures/icon_item_custom/gItemIconSpinnerTex"
static const ALIGN_ASSET(2) char gItemIconSpinnerTex[] = dgItemIconSpinnerTex;
#define dgItemIconCaneOfSomariaTex "__OTR__textures/icon_item_custom/gItemIconCaneOfSomariaTex"
static const ALIGN_ASSET(2) char gItemIconCaneOfSomariaTex[] = dgItemIconCaneOfSomariaTex;
#define dgItemIconDominionRodTex "__OTR__textures/icon_item_custom/gItemIconDominionRodTex"
static const ALIGN_ASSET(2) char gItemIconDominionRodTex[] = dgItemIconDominionRodTex;
#define dgItemIconTimeGateTex "__OTR__textures/icon_item_custom/gItemIconTimeGateTex"
static const ALIGN_ASSET(2) char gItemIconTimeGateTex[] = dgItemIconTimeGateTex;
#define dgItemIconBombArrowsTex "__OTR__textures/icon_item_custom/gItemIconBombArrowsTex"
static const ALIGN_ASSET(2) char gItemIconBombArrowsTex[] = dgItemIconBombArrowsTex;

// Elemental Wand — one icon per rod; the page-2 cell shows whichever mode is active.
#define dgItemIconSandRodTex "__OTR__textures/icon_item_custom/gItemIconSandRodTex"
static const ALIGN_ASSET(2) char gItemIconSandRodTex[] = dgItemIconSandRodTex;

#define dgItemIconTornadoRodTex "__OTR__textures/icon_item_custom/gItemIconTornadoRodTex"
static const ALIGN_ASSET(2) char gItemIconTornadoRodTex[] = dgItemIconTornadoRodTex;

#define dgItemIconWaterRodTex "__OTR__textures/icon_item_custom/gItemIconWaterRodTex"
static const ALIGN_ASSET(2) char gItemIconWaterRodTex[] = dgItemIconWaterRodTex;

#define dgItemIconMeteorRodTex "__OTR__textures/icon_item_custom/gItemIconMeteorRodTex"
static const ALIGN_ASSET(2) char gItemIconMeteorRodTex[] = dgItemIconMeteorRodTex;

#define dgItemIconStormRodTex "__OTR__textures/icon_item_custom/gItemIconStormRodTex"
static const ALIGN_ASSET(2) char gItemIconStormRodTex[] = dgItemIconStormRodTex;

#define dgItemIconShadowScepterTex "__OTR__textures/icon_item_custom/gItemIconShadowScepterTex"
static const ALIGN_ASSET(2) char gItemIconShadowScepterTex[] = dgItemIconShadowScepterTex;
#define dgItemIconFireRodTex "__OTR__textures/icon_item_custom/gItemIconFireRodTex"
static const ALIGN_ASSET(2) char gItemIconFireRodTex[] = dgItemIconFireRodTex;
#define dgItemIconIceRodTex "__OTR__textures/icon_item_custom/gItemIconIceRodTex"
static const ALIGN_ASSET(2) char gItemIconIceRodTex[] = dgItemIconIceRodTex;
#define dgItemIconLightRodTex "__OTR__textures/icon_item_custom/gItemIconLightRodTex"
static const ALIGN_ASSET(2) char gItemIconLightRodTex[] = dgItemIconLightRodTex;
#define dgItemIconBeetleTex "__OTR__textures/icon_item_custom/gItemIconBeetleTex"
static const ALIGN_ASSET(2) char gItemIconBeetleTex[] = dgItemIconBeetleTex;
#define dgItemIconShovelTex "__OTR__textures/icon_item_custom/gItemIconShovelTex"
static const ALIGN_ASSET(2) char gItemIconShovelTex[] = dgItemIconShovelTex;
#define dgItemIconMinishCapTex "__OTR__textures/icon_item_custom/gItemIconMinishCapTex"
static const ALIGN_ASSET(2) char gItemIconMinishCapTex[] = dgItemIconMinishCapTex;
#define dgItemIconPokeballTex "__OTR__textures/icon_item_custom/gItemIconPokeballTex"
static const ALIGN_ASSET(2) char gItemIconPokeballTex[] = dgItemIconPokeballTex;
#define dgItemIconNetTex "__OTR__textures/icon_item_custom/gItemIconNetTex"
static const ALIGN_ASSET(2) char gItemIconNetTex[] = dgItemIconNetTex;
#define dgItemIconBottomlessBottleTex "__OTR__textures/icon_item_custom/gItemIconBottomlessBottleTex"
static const ALIGN_ASSET(2) char gItemIconBottomlessBottleTex[] = dgItemIconBottomlessBottleTex;

#define dgThreeDayClockHour13Tex "__OTR__textures/parameter_static/gThreeDayClockHour13Tex"
static const ALIGN_ASSET(2) char gThreeDayClockHour13Tex[] = dgThreeDayClockHour13Tex;

#define dgThreeDayClockHour14Tex "__OTR__textures/parameter_static/gThreeDayClockHour14Tex"
static const ALIGN_ASSET(2) char gThreeDayClockHour14Tex[] = dgThreeDayClockHour14Tex;

#define dgThreeDayClockHour15Tex "__OTR__textures/parameter_static/gThreeDayClockHour15Tex"
static const ALIGN_ASSET(2) char gThreeDayClockHour15Tex[] = dgThreeDayClockHour15Tex;

#define dgThreeDayClockHour16Tex "__OTR__textures/parameter_static/gThreeDayClockHour16Tex"
static const ALIGN_ASSET(2) char gThreeDayClockHour16Tex[] = dgThreeDayClockHour16Tex;

#define dgThreeDayClockHour17Tex "__OTR__textures/parameter_static/gThreeDayClockHour17Tex"
static const ALIGN_ASSET(2) char gThreeDayClockHour17Tex[] = dgThreeDayClockHour17Tex;

#define dgThreeDayClockHour18Tex "__OTR__textures/parameter_static/gThreeDayClockHour18Tex"
static const ALIGN_ASSET(2) char gThreeDayClockHour18Tex[] = dgThreeDayClockHour18Tex;

#define dgThreeDayClockHour19Tex "__OTR__textures/parameter_static/gThreeDayClockHour19Tex"
static const ALIGN_ASSET(2) char gThreeDayClockHour19Tex[] = dgThreeDayClockHour19Tex;

#define dgThreeDayClockHour20Tex "__OTR__textures/parameter_static/gThreeDayClockHour20Tex"
static const ALIGN_ASSET(2) char gThreeDayClockHour20Tex[] = dgThreeDayClockHour20Tex;

#define dgThreeDayClockHour21Tex "__OTR__textures/parameter_static/gThreeDayClockHour21Tex"
static const ALIGN_ASSET(2) char gThreeDayClockHour21Tex[] = dgThreeDayClockHour21Tex;

#define dgThreeDayClockHour22Tex "__OTR__textures/parameter_static/gThreeDayClockHour22Tex"
static const ALIGN_ASSET(2) char gThreeDayClockHour22Tex[] = dgThreeDayClockHour22Tex;

#define dgThreeDayClockHour23Tex "__OTR__textures/parameter_static/gThreeDayClockHour23Tex"
static const ALIGN_ASSET(2) char gThreeDayClockHour23Tex[] = dgThreeDayClockHour23Tex;

#define dgThreeDayClockHour24Tex "__OTR__textures/parameter_static/gThreeDayClockHour24Tex"
static const ALIGN_ASSET(2) char gThreeDayClockHour24Tex[] = dgThreeDayClockHour24Tex;

#define dgEmptyTexture "__OTR__textures/virtual/gEmptyTexture"
static const ALIGN_ASSET(2) char gEmptyTexture[] = dgEmptyTexture;

#define dgThreeDayClock3DSEdgeTex "__OTR__textures/parameter_static/gThreeDayClock3DSEdgeTex"
static const ALIGN_ASSET(2) char gThreeDayClock3DSEdgeTex[] = dgThreeDayClock3DSEdgeTex;

#define dgThreeDayClock3DSMiddleTex "__OTR__textures/parameter_static/gThreeDayClock3DSMiddleTex"
static const ALIGN_ASSET(2) char gThreeDayClock3DSMiddleTex[] = dgThreeDayClock3DSMiddleTex;

#define dgThreeDayClock3DSFillTex "__OTR__textures/parameter_static/gThreeDayClock3DSFillTex"
static const ALIGN_ASSET(2) char gThreeDayClock3DSFillTex[] = dgThreeDayClock3DSFillTex;

#define dgThreeDayClock3DSArrowTex "__OTR__textures/parameter_static/gThreeDayClock3DSArrowTex"
static const ALIGN_ASSET(2) char gThreeDayClock3DSArrowTex[] = dgThreeDayClock3DSArrowTex;

#define dgThreeDayClock3DSTimeBackdropTex "__OTR__textures/parameter_static/gThreeDayClock3DSTimeBackdropTex"
static const ALIGN_ASSET(2) char gThreeDayClock3DSTimeBackdropTex[] = dgThreeDayClock3DSTimeBackdropTex;

#define dgThreeDayClock3DSSlowTimeTex "__OTR__textures/parameter_static/gThreeDayClock3DSSlowTimeTex"
static const ALIGN_ASSET(2) char gThreeDayClock3DSSlowTimeTex[] = dgThreeDayClock3DSSlowTimeTex;

#define dgThreeDayClock3DSFinalHoursMoonTex "__OTR__textures/parameter_static/gThreeDayClock3DSFinalHoursMoonTex"
static const ALIGN_ASSET(2) char gThreeDayClock3DSFinalHoursMoonTex[] = dgThreeDayClock3DSFinalHoursMoonTex;

#define dgThreeDayClock3DSFinalHoursDigit0Tex "__OTR__textures/parameter_static/gThreeDayClock3DSFinalHoursDigit0Tex"
static const ALIGN_ASSET(2) char gThreeDayClock3DSFinalHoursDigit0Tex[] = dgThreeDayClock3DSFinalHoursDigit0Tex;

#define dgThreeDayClock3DSFinalHoursDigit1Tex "__OTR__textures/parameter_static/gThreeDayClock3DSFinalHoursDigit1Tex"
static const ALIGN_ASSET(2) char gThreeDayClock3DSFinalHoursDigit1Tex[] = dgThreeDayClock3DSFinalHoursDigit1Tex;

#define dgThreeDayClock3DSFinalHoursDigit2Tex "__OTR__textures/parameter_static/gThreeDayClock3DSFinalHoursDigit2Tex"
static const ALIGN_ASSET(2) char gThreeDayClock3DSFinalHoursDigit2Tex[] = dgThreeDayClock3DSFinalHoursDigit2Tex;

#define dgThreeDayClock3DSFinalHoursDigit3Tex "__OTR__textures/parameter_static/gThreeDayClock3DSFinalHoursDigit3Tex"
static const ALIGN_ASSET(2) char gThreeDayClock3DSFinalHoursDigit3Tex[] = dgThreeDayClock3DSFinalHoursDigit3Tex;

#define dgThreeDayClock3DSFinalHoursDigit4Tex "__OTR__textures/parameter_static/gThreeDayClock3DSFinalHoursDigit4Tex"
static const ALIGN_ASSET(2) char gThreeDayClock3DSFinalHoursDigit4Tex[] = dgThreeDayClock3DSFinalHoursDigit4Tex;

#define dgThreeDayClock3DSFinalHoursDigit5Tex "__OTR__textures/parameter_static/gThreeDayClock3DSFinalHoursDigit5Tex"
static const ALIGN_ASSET(2) char gThreeDayClock3DSFinalHoursDigit5Tex[] = dgThreeDayClock3DSFinalHoursDigit5Tex;

#define dgThreeDayClock3DSFinalHoursDigit6Tex "__OTR__textures/parameter_static/gThreeDayClock3DSFinalHoursDigit6Tex"
static const ALIGN_ASSET(2) char gThreeDayClock3DSFinalHoursDigit6Tex[] = dgThreeDayClock3DSFinalHoursDigit6Tex;

#define dgThreeDayClock3DSFinalHoursDigit7Tex "__OTR__textures/parameter_static/gThreeDayClock3DSFinalHoursDigit7Tex"
static const ALIGN_ASSET(2) char gThreeDayClock3DSFinalHoursDigit7Tex[] = dgThreeDayClock3DSFinalHoursDigit7Tex;

#define dgThreeDayClock3DSFinalHoursDigit8Tex "__OTR__textures/parameter_static/gThreeDayClock3DSFinalHoursDigit8Tex"
static const ALIGN_ASSET(2) char gThreeDayClock3DSFinalHoursDigit8Tex[] = dgThreeDayClock3DSFinalHoursDigit8Tex;

#define dgThreeDayClock3DSFinalHoursDigit9Tex "__OTR__textures/parameter_static/gThreeDayClock3DSFinalHoursDigit9Tex"
static const ALIGN_ASSET(2) char gThreeDayClock3DSFinalHoursDigit9Tex[] = dgThreeDayClock3DSFinalHoursDigit9Tex;

#define dgThreeDayClock3DSFinalHoursColonTex "__OTR__textures/parameter_static/gThreeDayClock3DSFinalHoursColonTex"
static const ALIGN_ASSET(2) char gThreeDayClock3DSFinalHoursColonTex[] = dgThreeDayClock3DSFinalHoursColonTex;

#define dgPauseSavePromptENGTex "__OTR__textures/icon_item_static/gPauseSavePromptENGTex"
static const ALIGN_ASSET(2) char gPauseSavePromptENGTex[] = dgPauseSavePromptENGTex;

#define dgPauseYesENGTex "__OTR__textures/icon_item_static/gPauseYesENGTex"
static const ALIGN_ASSET(2) char gPauseYesENGTex[] = dgPauseYesENGTex;

#define dgPauseNoENGTex "__OTR__textures/icon_item_static/gPauseNoENGTex"
static const ALIGN_ASSET(2) char gPauseNoENGTex[] = dgPauseNoENGTex;

#define dgPauseSaveConfirmationENGTex "__OTR__textures/icon_item_static/gPauseSaveConfirmationENGTex"
static const ALIGN_ASSET(2) char gPauseSaveConfirmationENGTex[] = dgPauseSaveConfirmationENGTex;

#define dgContinuePlayingENGTex "__OTR__textures/icon_item_static/gContinuePlayingENGTex"
static const ALIGN_ASSET(2) char gContinuePlayingENGTex[] = dgContinuePlayingENGTex;

#define dgFileSelFourthDayTex "__OTR__misc/title_static/gFileSelFourthDayTex"
static const ALIGN_ASSET(2) char gFileSelFourthDayTex[] = dgFileSelFourthDayTex;

#define dgFileSelCheatingDayTex "__OTR__misc/title_static/gFileSelCheatingDayTex"
static const ALIGN_ASSET(2) char gFileSelCheatingDayTex[] = dgFileSelCheatingDayTex;

#define dgFileSelRandIconTex "__OTR__misc/title_static/gFileSelRandIconTex"
static const ALIGN_ASSET(2) char gFileSelRandIconTex[] = dgFileSelRandIconTex;

// File select new file setup labels, generated by tools/label_gen/genlabel.py
#define dgFileSelNewFileTex "__OTR__misc/title_static/gFileSelNewFileTex"
static const ALIGN_ASSET(2) char gFileSelNewFileTex[] = dgFileSelNewFileTex;

#define dgFileSelModeHeaderTex "__OTR__misc/title_static/gFileSelModeHeaderTex"
static const ALIGN_ASSET(2) char gFileSelModeHeaderTex[] = dgFileSelModeHeaderTex;

#define dgFileSelSeedHeaderTex "__OTR__misc/title_static/gFileSelSeedHeaderTex"
static const ALIGN_ASSET(2) char gFileSelSeedHeaderTex[] = dgFileSelSeedHeaderTex;

#define dgFileSelPresetHeaderTex "__OTR__misc/title_static/gFileSelPresetHeaderTex"
static const ALIGN_ASSET(2) char gFileSelPresetHeaderTex[] = dgFileSelPresetHeaderTex;

#define dgFileSelGenerateNewTex "__OTR__misc/title_static/gFileSelGenerateNewTex"
static const ALIGN_ASSET(2) char gFileSelGenerateNewTex[] = dgFileSelGenerateNewTex;

#define dgFileSelVanillaTex "__OTR__misc/title_static/gFileSelVanillaTex"
static const ALIGN_ASSET(2) char gFileSelVanillaTex[] = dgFileSelVanillaTex;

#define dgFileSelRandomizerTex "__OTR__misc/title_static/gFileSelRandomizerTex"
static const ALIGN_ASSET(2) char gFileSelRandomizerTex[] = dgFileSelRandomizerTex;

#define dgFileSelArrowLeftTex "__OTR__misc/title_static/gFileSelArrowLeftTex"
static const ALIGN_ASSET(2) char gFileSelArrowLeftTex[] = dgFileSelArrowLeftTex;

#define dgFileSelArrowRightTex "__OTR__misc/title_static/gFileSelArrowRightTex"
static const ALIGN_ASSET(2) char gFileSelArrowRightTex[] = dgFileSelArrowRightTex;

#define dgBoxChestCornerHealthTex "__OTR__objects/object_box/gBoxChestCornerHealthTex"
static const ALIGN_ASSET(2) char gBoxChestCornerHealthTex[] = dgBoxChestCornerHealthTex;

#define dgBoxChestCornerLesserTex "__OTR__objects/object_box/gBoxChestCornerLesserTex"
static const ALIGN_ASSET(2) char gBoxChestCornerLesserTex[] = dgBoxChestCornerLesserTex;

#define dgBoxChestCornerMajorTex "__OTR__objects/object_box/gBoxChestCornerMajorTex"
static const ALIGN_ASSET(2) char gBoxChestCornerMajorTex[] = dgBoxChestCornerMajorTex;

#define dgBoxChestCornerMaskTex "__OTR__objects/object_box/gBoxChestCornerMaskTex"
static const ALIGN_ASSET(2) char gBoxChestCornerMaskTex[] = dgBoxChestCornerMaskTex;

#define dgBoxChestCornerSkullTokenTex "__OTR__objects/object_box/gBoxChestCornerSkullTokenTex"
static const ALIGN_ASSET(2) char gBoxChestCornerSkullTokenTex[] = dgBoxChestCornerSkullTokenTex;

#define dgBoxChestCornerSmallKeyTex "__OTR__objects/object_box/gBoxChestCornerSmallKeyTex"
static const ALIGN_ASSET(2) char gBoxChestCornerSmallKeyTex[] = dgBoxChestCornerSmallKeyTex;

#define dgBoxChestCornerStrayFairyTex "__OTR__objects/object_box/gBoxChestCornerStrayFairyTex"
static const ALIGN_ASSET(2) char gBoxChestCornerStrayFairyTex[] = dgBoxChestCornerStrayFairyTex;

#define dgBoxChestLockHealthTex "__OTR__objects/object_box/gBoxChestLockHealthTex"
static const ALIGN_ASSET(2) char gBoxChestLockHealthTex[] = dgBoxChestLockHealthTex;

#define dgBoxChestLockLesserTex "__OTR__objects/object_box/gBoxChestLockLesserTex"
static const ALIGN_ASSET(2) char gBoxChestLockLesserTex[] = dgBoxChestLockLesserTex;

#define dgBoxChestLockMajorTex "__OTR__objects/object_box/gBoxChestLockMajorTex"
static const ALIGN_ASSET(2) char gBoxChestLockMajorTex[] = dgBoxChestLockMajorTex;

#define dgBoxChestLockMaskTex "__OTR__objects/object_box/gBoxChestLockMaskTex"
static const ALIGN_ASSET(2) char gBoxChestLockMaskTex[] = dgBoxChestLockMaskTex;

#define dgBoxChestLockSkullTokenTex "__OTR__objects/object_box/gBoxChestLockSkullTokenTex"
static const ALIGN_ASSET(2) char gBoxChestLockSkullTokenTex[] = dgBoxChestLockSkullTokenTex;

#define dgBoxChestLockSmallKeyTex "__OTR__objects/object_box/gBoxChestLockSmallKeyTex"
static const ALIGN_ASSET(2) char gBoxChestLockSmallKeyTex[] = dgBoxChestLockSmallKeyTex;

#define dgBoxChestLockStrayFairyTex "__OTR__objects/object_box/gBoxChestLockStrayFairyTex"
static const ALIGN_ASSET(2) char gBoxChestLockStrayFairyTex[] = dgBoxChestLockStrayFairyTex;

#define dgPotBossKeyDL "__OTR__objects/object_tsubo/gPotBossKeyDL"
static const ALIGN_ASSET(2) char gPotBossKeyDL[] = dgPotBossKeyDL;

#define dgPotFairyDL "__OTR__objects/object_tsubo/gPotFairyDL"
static const ALIGN_ASSET(2) char gPotFairyDL[] = dgPotFairyDL;

#define dgPotHeartDL "__OTR__objects/object_tsubo/gPotHeartDL"
static const ALIGN_ASSET(2) char gPotHeartDL[] = dgPotHeartDL;

#define dgPotMajorDL "__OTR__objects/object_tsubo/gPotMajorDL"
static const ALIGN_ASSET(2) char gPotMajorDL[] = dgPotMajorDL;

#define dgPotMaskDL "__OTR__objects/object_tsubo/gPotMaskDL"
static const ALIGN_ASSET(2) char gPotMaskDL[] = dgPotMaskDL;

#define dgPotMinorDL "__OTR__objects/object_tsubo/gPotMinorDL"
static const ALIGN_ASSET(2) char gPotMinorDL[] = dgPotMinorDL;

#define dgPotRandomDL "__OTR__objects/object_tsubo/gPotRandomDL"
static const ALIGN_ASSET(2) char gPotRandomDL[] = dgPotRandomDL;

#define dgPotSmallKeyDL "__OTR__objects/object_tsubo/gPotSmallKeyDL"
static const ALIGN_ASSET(2) char gPotSmallKeyDL[] = dgPotSmallKeyDL;

#define dgPotStandardDL "__OTR__objects/object_tsubo/gPotStandardDL"
static const ALIGN_ASSET(2) char gPotStandardDL[] = dgPotStandardDL;

#define dgPotTokenDL "__OTR__objects/object_tsubo/gPotTokenDL"
static const ALIGN_ASSET(2) char gPotTokenDL[] = dgPotTokenDL;

#define dgLargeMajorCrateDL "__OTR__objects/object_kibako2/gLargeMajorCrateDL"
static const ALIGN_ASSET(2) char gLargeMajorCrateDL[] = dgLargeMajorCrateDL;

#define dgLargeMaskCrateDL "__OTR__objects/object_kibako2/gLargeMaskCrateDL"
static const ALIGN_ASSET(2) char gLargeMaskCrateDL[] = dgLargeMaskCrateDL;

#define dgLargeMinorCrateDL "__OTR__objects/object_kibako2/gLargeMinorCrateDL"
static const ALIGN_ASSET(2) char gLargeMinorCrateDL[] = dgLargeMinorCrateDL;

#define dgLargeRandoCrateDL "__OTR__objects/object_kibako2/gLargeRandoCrateDL"
static const ALIGN_ASSET(2) char gLargeRandoCrateDL[] = dgLargeRandoCrateDL;

#define dgLargeSmallKeyCrateDL "__OTR__objects/object_kibako2/gLargeSmallKeyCrateDL"
static const ALIGN_ASSET(2) char gLargeSmallKeyCrateDL[] = dgLargeSmallKeyCrateDL;

#define dgLargeTokenCrateDL "__OTR__objects/object_kibako2/gLargeTokenCrateDL"
static const ALIGN_ASSET(2) char gLargeTokenCrateDL[] = dgLargeTokenCrateDL;

#define dgLargeBossKeyCrateDL "__OTR__objects/object_kibako2/gLargeBossKeyCrateDL"
static const ALIGN_ASSET(2) char gLargeBossKeyCrateDL[] = dgLargeBossKeyCrateDL;

#define dgLargeFairyCrateDL "__OTR__objects/object_kibako2/gLargeFairyCrateDL"
static const ALIGN_ASSET(2) char gLargeFairyCrateDL[] = dgLargeFairyCrateDL;

#define dgLargeHeartCrateDL "__OTR__objects/object_kibako2/gLargeHeartCrateDL"
static const ALIGN_ASSET(2) char gLargeHeartCrateDL[] = dgLargeHeartCrateDL;

#define dgLargeJunkCrateDL "__OTR__objects/object_kibako2/gLargeJunkCrateDL"
static const ALIGN_ASSET(2) char gLargeJunkCrateDL[] = dgLargeJunkCrateDL;

#define dgSmallMajorCrateDL "__OTR__objects/object_kibako/gSmallMajorCrateDL"
static const ALIGN_ASSET(2) char gSmallMajorCrateDL[] = dgSmallMajorCrateDL;

#define dgSmallMaskCrateDL "__OTR__objects/object_kibako/gSmallMaskCrateDL"
static const ALIGN_ASSET(2) char gSmallMaskCrateDL[] = dgSmallMaskCrateDL;

#define dgSmallMinorCrateDL "__OTR__objects/object_kibako/gSmallMinorCrateDL"
static const ALIGN_ASSET(2) char gSmallMinorCrateDL[] = dgSmallMinorCrateDL;

#define dgSmallRandoCrateDL "__OTR__objects/object_kibako/gSmallRandoCrateDL"
static const ALIGN_ASSET(2) char gSmallRandoCrateDL[] = dgSmallRandoCrateDL;

#define dgSmallSmallKeyCrateDL "__OTR__objects/object_kibako/gSmallSmallKeyCrateDL"
static const ALIGN_ASSET(2) char gSmallSmallKeyCrateDL[] = dgSmallSmallKeyCrateDL;

#define dgSmallTokenCrateDL "__OTR__objects/object_kibako/gSmallTokenCrateDL"
static const ALIGN_ASSET(2) char gSmallTokenCrateDL[] = dgSmallTokenCrateDL;

#define dgSmallBossKeyCrateDL "__OTR__objects/object_kibako/gSmallBossKeyCrateDL"
static const ALIGN_ASSET(2) char gSmallBossKeyCrateDL[] = dgSmallBossKeyCrateDL;

#define dgSmallFairyCrateDL "__OTR__objects/object_kibako/gSmallFairyCrateDL"
static const ALIGN_ASSET(2) char gSmallFairyCrateDL[] = dgSmallFairyCrateDL;

#define dgSmallHeartCrateDL "__OTR__objects/object_kibako/gSmallHeartCrateDL"
static const ALIGN_ASSET(2) char gSmallHeartCrateDL[] = dgSmallHeartCrateDL;

#define dgSmallJunkCrateDL "__OTR__objects/object_kibako/gSmallJunkCrateDL"
static const ALIGN_ASSET(2) char gSmallJunkCrateDL[] = dgSmallJunkCrateDL;

#define dgBarrelMajorDL "__OTR__objects/object_taru/gBarrelMajorDL"
static const ALIGN_ASSET(2) char gBarrelMajorDL[] = dgBarrelMajorDL;

#define dgBarrelMaskDL "__OTR__objects/object_taru/gBarrelMaskDL"
static const ALIGN_ASSET(2) char gBarrelMaskDL[] = dgBarrelMaskDL;

#define dgBarrelMinorDL "__OTR__objects/object_taru/gBarrelMinorDL"
static const ALIGN_ASSET(2) char gBarrelMinorDL[] = dgBarrelMinorDL;

#define dgBarrelRandoDL "__OTR__objects/object_taru/gBarrelRandoDL"
static const ALIGN_ASSET(2) char gBarrelRandoDL[] = dgBarrelRandoDL;

#define dgBarrelSmallKeyDL "__OTR__objects/object_taru/gBarrelSmallKeyDL"
static const ALIGN_ASSET(2) char gBarrelSmallKeyDL[] = dgBarrelSmallKeyDL;

#define dgBarrelTokenDL "__OTR__objects/object_taru/gBarrelTokenDL"
static const ALIGN_ASSET(2) char gBarrelTokenDL[] = dgBarrelTokenDL;

#define dgBarrelBossKeyDL "__OTR__objects/object_taru/gBarrelBossKeyDL"
static const ALIGN_ASSET(2) char gBarrelBossKeyDL[] = dgBarrelBossKeyDL;

#define dgBarrelFairyDL "__OTR__objects/object_taru/gBarrelFairyDL"
static const ALIGN_ASSET(2) char gBarrelFairyDL[] = dgBarrelFairyDL;

#define dgBarrelHeartDL "__OTR__objects/object_taru/gBarrelHeartDL"
static const ALIGN_ASSET(2) char gBarrelHeartDL[] = dgBarrelHeartDL;

#define dgBarrelJunkDL "__OTR__objects/object_taru/gBarrelJunkDL"
static const ALIGN_ASSET(2) char gBarrelJunkDL[] = dgBarrelJunkDL;

#define dgRandoBushDL "__OTR__objects/gameplay_field_keep/gFieldBushRandomDL"
static const ALIGN_ASSET(2) char gRandoBushDL[] = dgRandoBushDL;

#define dgRandoBushXluDL "__OTR__objects/gameplay_field_keep/gFieldBushRandomXluDL"
static const ALIGN_ASSET(2) char gRandoBushXluDL[] = dgRandoBushXluDL;

#define dgRandoBushMinorDL "__OTR__objects/gameplay_field_keep/gFieldBushMinorDL"
static const ALIGN_ASSET(2) char gRandoBushMinorDL[] = dgRandoBushMinorDL;

#define dgRandoBushMinorXluDL "__OTR__objects/gameplay_field_keep/gFieldBushMinorXluDL"
static const ALIGN_ASSET(2) char gRandoBushMinorXluDL[] = dgRandoBushMinorXluDL;

#define dgRandoBushMajorDL "__OTR__objects/gameplay_field_keep/gFieldBushMajorDL"
static const ALIGN_ASSET(2) char gRandoBushMajorDL[] = dgRandoBushMajorDL;

#define dgRandoBushMajorXluDL "__OTR__objects/gameplay_field_keep/gFieldBushMajorXluDL"
static const ALIGN_ASSET(2) char gRandoBushMajorXluDL[] = dgRandoBushMajorXluDL;

#define dgRandoBushSmallKeyDL "__OTR__objects/gameplay_field_keep/gFieldBushSmallKeyDL"
static const ALIGN_ASSET(2) char gRandoBushSmallKeyDL[] = dgRandoBushSmallKeyDL;

#define dgRandoBushSmallKeyXluDL "__OTR__objects/gameplay_field_keep/gFieldBushSmallKeyXluDL"
static const ALIGN_ASSET(2) char gRandoBushSmallKeyXluDL[] = dgRandoBushSmallKeyXluDL;

#define dgRandoBushBossKeyDL "__OTR__objects/gameplay_field_keep/gFieldBushBossKeyDL"
static const ALIGN_ASSET(2) char gRandoBushBossKeyDL[] = dgRandoBushBossKeyDL;

#define dgRandoBushBossKeyXluDL "__OTR__objects/gameplay_field_keep/gFieldBushBossKeyXluDL"
static const ALIGN_ASSET(2) char gRandoBushBossKeyXluDL[] = dgRandoBushBossKeyXluDL;

#define dgRandoBushTokenDL "__OTR__objects/gameplay_field_keep/gFieldBushTokenDL"
static const ALIGN_ASSET(2) char gRandoBushTokenDL[] = dgRandoBushTokenDL;

#define dgRandoBushTokenXluDL "__OTR__objects/gameplay_field_keep/gFieldBushTokenXluDL"
static const ALIGN_ASSET(2) char gRandoBushTokenXluDL[] = dgRandoBushTokenXluDL;

#define dgRandoBushMaskDL "__OTR__objects/gameplay_field_keep/gFieldBushMaskDL"
static const ALIGN_ASSET(2) char gRandoBushMaskDL[] = dgRandoBushMaskDL;

#define dgRandoBushMaskXluDL "__OTR__objects/gameplay_field_keep/gFieldBushMaskXluDL"
static const ALIGN_ASSET(2) char gRandoBushMaskXluDL[] = dgRandoBushMaskXluDL;

#define dgRandoBushFairyDL "__OTR__objects/gameplay_field_keep/gFieldBushFairyDL"
static const ALIGN_ASSET(2) char gRandoBushFairyDL[] = dgRandoBushFairyDL;

#define dgRandoBushFairyXluDL "__OTR__objects/gameplay_field_keep/gFieldBushFairyXluDL"
static const ALIGN_ASSET(2) char gRandoBushFairyXluDL[] = dgRandoBushFairyXluDL;

#define dgRandoBushHeartDL "__OTR__objects/gameplay_field_keep/gFieldBushHeartDL"
static const ALIGN_ASSET(2) char gRandoBushHeartDL[] = dgRandoBushHeartDL;

#define dgRandoBushHeartXluDL "__OTR__objects/gameplay_field_keep/gFieldBushHeartXluDL"
static const ALIGN_ASSET(2) char gRandoBushHeartXluDL[] = dgRandoBushHeartXluDL;

#define dgRandoBushJunkDL "__OTR__objects/gameplay_field_keep/gFieldBushJunkDL"
static const ALIGN_ASSET(2) char gRandoBushJunkDL[] = dgRandoBushJunkDL;

#define dgRandoBushJunkXluDL "__OTR__objects/gameplay_field_keep/gFieldBushJunkXluDL"
static const ALIGN_ASSET(2) char gRandoBushJunkXluDL[] = dgRandoBushJunkXluDL;

#define dgRandoCuttableGrassRandomDL "__OTR__objects/gameplay_keep/gRandoCuttableGrassRandomDL"
static const ALIGN_ASSET(2) char gRandoCuttableGrassRandomDL[] = dgRandoCuttableGrassRandomDL;

#define dgRandoCuttableGrassMinorDL "__OTR__objects/gameplay_keep/gRandoCuttableGrassMinorDL"
static const ALIGN_ASSET(2) char gRandoCuttableGrassMinorDL[] = dgRandoCuttableGrassMinorDL;

#define dgRandoCuttableGrassMajorDL "__OTR__objects/gameplay_keep/gRandoCuttableGrassMajorDL"
static const ALIGN_ASSET(2) char gRandoCuttableGrassMajorDL[] = dgRandoCuttableGrassMajorDL;

#define dgRandoCuttableGrassSmallKeyDL "__OTR__objects/gameplay_keep/gRandoCuttableGrassSmallKeyDL"
static const ALIGN_ASSET(2) char gRandoCuttableGrassSmallKeyDL[] = dgRandoCuttableGrassSmallKeyDL;

#define dgRandoCuttableGrassBossKeyDL "__OTR__objects/gameplay_keep/gRandoCuttableGrassBossKeyDL"
static const ALIGN_ASSET(2) char gRandoCuttableGrassBossKeyDL[] = dgRandoCuttableGrassBossKeyDL;

#define dgRandoCuttableGrassTokenDL "__OTR__objects/gameplay_keep/gRandoCuttableGrassTokenDL"
static const ALIGN_ASSET(2) char gRandoCuttableGrassTokenDL[] = dgRandoCuttableGrassTokenDL;

#define dgRandoCuttableGrassMaskDL "__OTR__objects/gameplay_keep/gRandoCuttableGrassMaskDL"
static const ALIGN_ASSET(2) char gRandoCuttableGrassMaskDL[] = dgRandoCuttableGrassMaskDL;

#define dgRandoCuttableGrassFairyDL "__OTR__objects/gameplay_keep/gRandoCuttableGrassFairyDL"
static const ALIGN_ASSET(2) char gRandoCuttableGrassFairyDL[] = dgRandoCuttableGrassFairyDL;

#define dgRandoCuttableGrassHeartDL "__OTR__objects/gameplay_keep/gRandoCuttableGrassHeartDL"
static const ALIGN_ASSET(2) char gRandoCuttableGrassHeartDL[] = dgRandoCuttableGrassHeartDL;

#define dgRandoCuttableGrassJunkDL "__OTR__objects/gameplay_keep/gRandoCuttableGrassJunkDL"
static const ALIGN_ASSET(2) char gRandoCuttableGrassJunkDL[] = dgRandoCuttableGrassJunkDL;

#define dgChestTrackerIcon "__OTR__textures/icons/gChestTrackerIcon"
static const ALIGN_ASSET(2) char gChestTrackerIcon[] = dgChestTrackerIcon;

#define dgPotTrackerIcon "__OTR__textures/icons/gPotTrackerIcon"
static const ALIGN_ASSET(2) char gPotTrackerIcon[] = dgPotTrackerIcon;

#define dgCrateTrackerIcon "__OTR__textures/icons/gCrateTrackerIcon"
static const ALIGN_ASSET(2) char gCrateTrackerIcon[] = dgCrateTrackerIcon;

#define dgBarrelTrackerIcon "__OTR__textures/icons/gBarrelTrackerIcon"
static const ALIGN_ASSET(2) char gBarrelTrackerIcon[] = dgBarrelTrackerIcon;

#define dgGiFlippersDL "__OTR__objects/object_ability_swim/gGiFlippersDL"
static const ALIGN_ASSET(2) char gGiFlippersDL[] = dgGiFlippersDL;

#define dgTriforcePiece0DL "__OTR__objects/object_triforce_piece_0/gTriforcePiece0DL"
static const ALIGN_ASSET(2) char gTriforcePiece0DL[] = dgTriforcePiece0DL;

#define dgTriforcePiece1DL "__OTR__objects/object_triforce_piece_1/gTriforcePiece1DL"
static const ALIGN_ASSET(2) char gTriforcePiece1DL[] = dgTriforcePiece1DL;

#define dgTriforcePiece2DL "__OTR__objects/object_triforce_piece_2/gTriforcePiece2DL"
static const ALIGN_ASSET(2) char gTriforcePiece2DL[] = dgTriforcePiece2DL;

#define dgTriforcePieceCompletedDL "__OTR__objects/object_triforce_completed/gTriforcePieceCompletedDL"
static const ALIGN_ASSET(2) char gTriforcePieceCompletedDL[] = dgTriforcePieceCompletedDL;

#define dgTrapDL "__OTR__objects/object_trap/gTrapDL"
static const ALIGN_ASSET(2) char gTrapDL[] = dgTrapDL;

#define dgSkeletonKeyDL "__OTR__objects/object_key/gSkeletonKeyDL"
static const ALIGN_ASSET(2) char gSkeletonKeyDL[] = dgSkeletonKeyDL;

#define dgOcarinaAButtonDL "__OTR__objects/object_ocarina_a_button/gOcarinaAButtonDL"
static const ALIGN_ASSET(2) char gOcarinaAButtonDL[] = dgOcarinaAButtonDL;

#define dgOcarinaCDownButtonDL "__OTR__objects/object_ocarina_c_down_button/gOcarinaCDownButtonDL"
static const ALIGN_ASSET(2) char gOcarinaCDownButtonDL[] = dgOcarinaCDownButtonDL;

#define dgOcarinaCLeftButtonDL "__OTR__objects/object_ocarina_c_left_button/gOcarinaCLeftButtonDL"
static const ALIGN_ASSET(2) char gOcarinaCLeftButtonDL[] = dgOcarinaCLeftButtonDL;

#define dgOcarinaCRightButtonDL "__OTR__objects/object_ocarina_c_right_button/gOcarinaCRightButtonDL"
static const ALIGN_ASSET(2) char gOcarinaCRightButtonDL[] = dgOcarinaCRightButtonDL;

#define dgOcarinaCUpButtonDL "__OTR__objects/object_ocarina_c_up_button/gOcarinaCUpButtonDL"
static const ALIGN_ASSET(2) char gOcarinaCUpButtonDL[] = dgOcarinaCUpButtonDL;

// ─── NEI item icons/names ported from Shipwright soh_assets.h (Skijer's NEI) ───
#define dgBallAndChainNameTex "__OTR__textures/item_name_custom/gBallAndChainNameTex"
static const ALIGN_ASSET(2) char gBallAndChainNameTex[] = dgBallAndChainNameTex;
// Skijer's NEI: custom ocarina song name-box textures (OoT quest page).
#define dgFugueOfHomeNameTex "__OTR__textures/item_name_custom/gFugueOfHomeNameTex"
static const ALIGN_ASSET(2) char gFugueOfHomeNameTex[] = dgFugueOfHomeNameTex;
#define dgCommandMelodyNameTex "__OTR__textures/item_name_custom/gCommandMelodyNameTex"
static const ALIGN_ASSET(2) char gCommandMelodyNameTex[] = dgCommandMelodyNameTex;
#define dgBalladOfHeroNameTex "__OTR__textures/item_name_custom/gBalladOfHeroNameTex"
static const ALIGN_ASSET(2) char gBalladOfHeroNameTex[] = dgBalladOfHeroNameTex;
#define dgBeetleNameTex "__OTR__textures/item_name_custom/gBeetleNameTex"
static const ALIGN_ASSET(2) char gBeetleNameTex[] = dgBeetleNameTex;
#define dgBombArrowsNameTex "__OTR__textures/item_name_custom/gBombArrowsNameTex"
static const ALIGN_ASSET(2) char gBombArrowsNameTex[] = dgBombArrowsNameTex;

// Elemental Wand — one name banner per rod.
#define dgSandRodNameTex "__OTR__textures/item_name_custom/gSandRodNameTex"
static const ALIGN_ASSET(2) char gSandRodNameTex[] = dgSandRodNameTex;

#define dgTornadoRodNameTex "__OTR__textures/item_name_custom/gTornadoRodNameTex"
static const ALIGN_ASSET(2) char gTornadoRodNameTex[] = dgTornadoRodNameTex;

#define dgWaterRodNameTex "__OTR__textures/item_name_custom/gWaterRodNameTex"
static const ALIGN_ASSET(2) char gWaterRodNameTex[] = dgWaterRodNameTex;

#define dgMeteorRodNameTex "__OTR__textures/item_name_custom/gMeteorRodNameTex"
static const ALIGN_ASSET(2) char gMeteorRodNameTex[] = dgMeteorRodNameTex;

#define dgStormRodNameTex "__OTR__textures/item_name_custom/gStormRodNameTex"
static const ALIGN_ASSET(2) char gStormRodNameTex[] = dgStormRodNameTex;

#define dgShadowScepterNameTex "__OTR__textures/item_name_custom/gShadowScepterNameTex"
static const ALIGN_ASSET(2) char gShadowScepterNameTex[] = dgShadowScepterNameTex;
#define dgBottomlessBottleNameTex "__OTR__textures/item_name_custom/gBottomlessBottleNameTex"
static const ALIGN_ASSET(2) char gBottomlessBottleNameTex[] = dgBottomlessBottleNameTex;
#define dgCaneOfSomariaNameTex "__OTR__textures/item_name_custom/gCaneOfSomariaNameTex"
static const ALIGN_ASSET(2) char gCaneOfSomariaNameTex[] = dgCaneOfSomariaNameTex;
#define dgDekuLeafNameTex "__OTR__textures/item_name_custom/gDekuLeafNameTex"
static const ALIGN_ASSET(2) char gDekuLeafNameTex[] = dgDekuLeafNameTex;
#define dgDemiseDestructionNameTex "__OTR__textures/item_name_custom/gDemiseDestructionNameTex"
static const ALIGN_ASSET(2) char gDemiseDestructionNameTex[] = dgDemiseDestructionNameTex;
#define dgDesireSensorNameTex "__OTR__textures/item_name_custom/gDesireSensorNameTex"
static const ALIGN_ASSET(2) char gDesireSensorNameTex[] = dgDesireSensorNameTex;
#define dgDominionRodNameTex "__OTR__textures/item_name_custom/gDominionRodNameTex"
static const ALIGN_ASSET(2) char gDominionRodNameTex[] = dgDominionRodNameTex;
#define dgFireRodNameTex "__OTR__textures/item_name_custom/gFireRodNameTex"
static const ALIGN_ASSET(2) char gFireRodNameTex[] = dgFireRodNameTex;
#define dgGustJarNameTex "__OTR__textures/item_name_custom/gGustJarNameTex"
static const ALIGN_ASSET(2) char gGustJarNameTex[] = dgGustJarNameTex;
#define dgHyliaGraceNameTex "__OTR__textures/item_name_custom/gHyliaGraceNameTex"
static const ALIGN_ASSET(2) char gHyliaGraceNameTex[] = dgHyliaGraceNameTex;
#define dgIceRodNameTex "__OTR__textures/item_name_custom/gIceRodNameTex"
static const ALIGN_ASSET(2) char gIceRodNameTex[] = dgIceRodNameTex;
#define dgItemIconClawshotTex "__OTR__textures/icon_item_custom/gItemIconClawshotTex"
static const ALIGN_ASSET(2) char gItemIconClawshotTex[] = dgItemIconClawshotTex;
#define dgItemIconDrillshaftTex "__OTR__textures/icon_item_custom/gItemIconDrillshaftTex"
static const ALIGN_ASSET(2) char gItemIconDrillshaftTex[] = dgItemIconDrillshaftTex;
#define dgItemIconFireFlowerTex "__OTR__textures/icon_item_custom/gItemIconFireFlowerTex"
static const ALIGN_ASSET(2) char gItemIconFireFlowerTex[] = dgItemIconFireFlowerTex;
#define dgItemIconGaleBoomerangTex "__OTR__textures/icon_item_custom/gItemIconGaleBoomerangTex"
static const ALIGN_ASSET(2) char gItemIconGaleBoomerangTex[] = dgItemIconGaleBoomerangTex;
#define dgItemIconLanternBlueTex "__OTR__textures/icon_item_custom/gItemIconLanternBlueTex"
static const ALIGN_ASSET(2) char gItemIconLanternBlueTex[] = dgItemIconLanternBlueTex;
#define dgItemIconLanternFireTex "__OTR__textures/icon_item_custom/gItemIconLanternFireTex"
static const ALIGN_ASSET(2) char gItemIconLanternFireTex[] = dgItemIconLanternFireTex;
#define dgItemIconLanternGreenTex "__OTR__textures/icon_item_custom/gItemIconLanternGreenTex"
static const ALIGN_ASSET(2) char gItemIconLanternGreenTex[] = dgItemIconLanternGreenTex;
#define dgItemIconLanternPoeTex "__OTR__textures/icon_item_custom/gItemIconLanternPoeTex"
static const ALIGN_ASSET(2) char gItemIconLanternPoeTex[] = dgItemIconLanternPoeTex;
#define dgItemIconLanternTex "__OTR__textures/icon_item_custom/gItemIconLanternTex"
static const ALIGN_ASSET(2) char gItemIconLanternTex[] = dgItemIconLanternTex;
#define dgItemIconPropHuntChangeTex "__OTR__textures/icon_item_custom/gItemIconPropHuntChangeTex"
static const ALIGN_ASSET(2) char gItemIconPropHuntChangeTex[] = dgItemIconPropHuntChangeTex;
#define dgItemIconPropHuntEnemyTex  "__OTR__textures/icon_item_custom/gItemIconPropHuntEnemyTex"
#define dgItemIconPropHuntNextTex   "__OTR__textures/icon_item_custom/gItemIconPropHuntNextTex"
#define dgItemIconPropHuntNpcTex    "__OTR__textures/icon_item_custom/gItemIconPropHuntNpcTex"
#define dgItemIconPropHuntPotTex    "__OTR__textures/icon_item_custom/gItemIconPropHuntPotTex"
#define dgItemIconPropHuntPrevTex   "__OTR__textures/icon_item_custom/gItemIconPropHuntPrevTex"
#define dgLanternNameTex "__OTR__textures/item_name_custom/gLanternNameTex"
static const ALIGN_ASSET(2) char gLanternNameTex[] = dgLanternNameTex;
#define dgLightRodNameTex "__OTR__textures/item_name_custom/gLightRodNameTex"
static const ALIGN_ASSET(2) char gLightRodNameTex[] = dgLightRodNameTex;
#define dgMinishCapNameTex "__OTR__textures/item_name_custom/gMinishCapNameTex"
static const ALIGN_ASSET(2) char gMinishCapNameTex[] = dgMinishCapNameTex;
#define dgMogmaMittsNameTex "__OTR__textures/item_name_custom/gMogmaMittsNameTex"
static const ALIGN_ASSET(2) char gMogmaMittsNameTex[] = dgMogmaMittsNameTex;
#define dgNetNameTex "__OTR__textures/item_name_custom/gNetNameTex"
static const ALIGN_ASSET(2) char gNetNameTex[] = dgNetNameTex;
#define dgPokeballNameTex "__OTR__textures/item_name_custom/gPokeballNameTex"
static const ALIGN_ASSET(2) char gPokeballNameTex[] = dgPokeballNameTex;
#define dgRocsCapeNameTex "__OTR__textures/item_name_custom/gRocsCapeNameTex"
static const ALIGN_ASSET(2) char gRocsCapeNameTex[] = dgRocsCapeNameTex;
#define dgRocsFeatherNameTex "__OTR__textures/item_name_custom/gRocsFeatherNameTex"
static const ALIGN_ASSET(2) char gRocsFeatherNameTex[] = dgRocsFeatherNameTex;
#define dgShovelNameTex "__OTR__textures/item_name_custom/gShovelNameTex"
static const ALIGN_ASSET(2) char gShovelNameTex[] = dgShovelNameTex;
#define dgSpinnerNameTex "__OTR__textures/item_name_custom/gSpinnerNameTex"
static const ALIGN_ASSET(2) char gSpinnerNameTex[] = dgSpinnerNameTex;
#define dgSwitchHookNameTex "__OTR__textures/item_name_custom/gSwitchHookNameTex"
static const ALIGN_ASSET(2) char gSwitchHookNameTex[] = dgSwitchHookNameTex;
// Skijer's NEI hookshot overhaul: Clawshot (MM-native hookshot renamed) + Ultrashot (Longshot L3).
#define dgClawshotNameTex "__OTR__textures/item_name_custom/gClawshotNameTex"
static const ALIGN_ASSET(2) char gClawshotNameTex[] = dgClawshotNameTex;
#define dgUltrashotNameTex "__OTR__textures/item_name_custom/gUltrashotNameTex"
static const ALIGN_ASSET(2) char gUltrashotNameTex[] = dgUltrashotNameTex;
#define dgTimeGateNameTex "__OTR__textures/item_name_custom/gTimeGateNameTex"
static const ALIGN_ASSET(2) char gTimeGateNameTex[] = dgTimeGateNameTex;
#define dgWhipNameTex "__OTR__textures/item_name_custom/gWhipNameTex"
static const ALIGN_ASSET(2) char gWhipNameTex[] = dgWhipNameTex;
#define dgZonaiPermafrostNameTex "__OTR__textures/item_name_custom/gZonaiPermafrostNameTex"
static const ALIGN_ASSET(2) char gZonaiPermafrostNameTex[] = dgZonaiPermafrostNameTex;
#define dgItemIconPropHuntEnemyTex  "__OTR__textures/icon_item_custom/gItemIconPropHuntEnemyTex"
static const ALIGN_ASSET(2) char gItemIconPropHuntEnemyTex[]  = dgItemIconPropHuntEnemyTex;
#define dgItemIconPropHuntNextTex   "__OTR__textures/icon_item_custom/gItemIconPropHuntNextTex"
static const ALIGN_ASSET(2) char gItemIconPropHuntNextTex[]   = dgItemIconPropHuntNextTex;
#define dgItemIconPropHuntNpcTex    "__OTR__textures/icon_item_custom/gItemIconPropHuntNpcTex"
static const ALIGN_ASSET(2) char gItemIconPropHuntNpcTex[]    = dgItemIconPropHuntNpcTex;
#define dgItemIconPropHuntPotTex    "__OTR__textures/icon_item_custom/gItemIconPropHuntPotTex"
static const ALIGN_ASSET(2) char gItemIconPropHuntPotTex[]    = dgItemIconPropHuntPotTex;
#define dgItemIconPropHuntPrevTex   "__OTR__textures/icon_item_custom/gItemIconPropHuntPrevTex"
static const ALIGN_ASSET(2) char gItemIconPropHuntPrevTex[]   = dgItemIconPropHuntPrevTex;
