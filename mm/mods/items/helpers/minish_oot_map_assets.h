#ifndef MINISH_OOT_MAP_ASSETS_H
#define MINISH_OOT_MAP_ASSETS_H 1
// Ported OoT map-page assets (Hyrule warp map) for minish_kaleido. OTR-path char[]
// EXTRACTED (deduped) from Shipwright soh/assets — only PauseMap/WorldMapCloud/
// PositionName/PauseCurrentPosition, to avoid the gItemIcon* clashes of the full OoT
// headers. Textures load from the OoT o2r at runtime.
#include "align_asset_macro.h"

#define dgPauseMap00Tex "__OTR__textures/icon_item_static/gPauseMap00Tex"
static const ALIGN_ASSET(2) char gPauseMap00Tex[] = dgPauseMap00Tex;
#define dgPauseMap20Tex "__OTR__textures/icon_item_static/gPauseMap20Tex"
static const ALIGN_ASSET(2) char gPauseMap20Tex[] = dgPauseMap20Tex;
#define dgPauseMap01Tex "__OTR__textures/icon_item_static/gPauseMap01Tex"
static const ALIGN_ASSET(2) char gPauseMap01Tex[] = dgPauseMap01Tex;
#define dgPauseMap11Tex "__OTR__textures/icon_item_static/gPauseMap11Tex"
static const ALIGN_ASSET(2) char gPauseMap11Tex[] = dgPauseMap11Tex;
#define dgPauseMap21Tex "__OTR__textures/icon_item_static/gPauseMap21Tex"
static const ALIGN_ASSET(2) char gPauseMap21Tex[] = dgPauseMap21Tex;
#define dgPauseMap02Tex "__OTR__textures/icon_item_static/gPauseMap02Tex"
static const ALIGN_ASSET(2) char gPauseMap02Tex[] = dgPauseMap02Tex;
#define dgPauseMap12Tex "__OTR__textures/icon_item_static/gPauseMap12Tex"
static const ALIGN_ASSET(2) char gPauseMap12Tex[] = dgPauseMap12Tex;
#define dgPauseMap22Tex "__OTR__textures/icon_item_static/gPauseMap22Tex"
static const ALIGN_ASSET(2) char gPauseMap22Tex[] = dgPauseMap22Tex;
#define dgPauseMap03Tex "__OTR__textures/icon_item_static/gPauseMap03Tex"
static const ALIGN_ASSET(2) char gPauseMap03Tex[] = dgPauseMap03Tex;
#define dgPauseMap13Tex "__OTR__textures/icon_item_static/gPauseMap13Tex"
static const ALIGN_ASSET(2) char gPauseMap13Tex[] = dgPauseMap13Tex;
#define dgPauseMap23Tex "__OTR__textures/icon_item_static/gPauseMap23Tex"
static const ALIGN_ASSET(2) char gPauseMap23Tex[] = dgPauseMap23Tex;
#define dgPauseMap04Tex "__OTR__textures/icon_item_static/gPauseMap04Tex"
static const ALIGN_ASSET(2) char gPauseMap04Tex[] = dgPauseMap04Tex;
#define dgPauseMap14Tex "__OTR__textures/icon_item_static/gPauseMap14Tex"
static const ALIGN_ASSET(2) char gPauseMap14Tex[] = dgPauseMap14Tex;
#define dgPauseMap24Tex "__OTR__textures/icon_item_static/gPauseMap24Tex"
static const ALIGN_ASSET(2) char gPauseMap24Tex[] = dgPauseMap24Tex;
#define dgWorldMapCloud1Tex "__OTR__textures/icon_item_field_static/gWorldMapCloud1Tex"
static const ALIGN_ASSET(2) char gWorldMapCloud1Tex[] = dgWorldMapCloud1Tex;
#define dgWorldMapCloud2Tex "__OTR__textures/icon_item_field_static/gWorldMapCloud2Tex"
static const ALIGN_ASSET(2) char gWorldMapCloud2Tex[] = dgWorldMapCloud2Tex;
#define dgWorldMapCloud3Tex "__OTR__textures/icon_item_field_static/gWorldMapCloud3Tex"
static const ALIGN_ASSET(2) char gWorldMapCloud3Tex[] = dgWorldMapCloud3Tex;
#define dgWorldMapCloud4Tex "__OTR__textures/icon_item_field_static/gWorldMapCloud4Tex"
static const ALIGN_ASSET(2) char gWorldMapCloud4Tex[] = dgWorldMapCloud4Tex;
#define dgWorldMapCloud5Tex "__OTR__textures/icon_item_field_static/gWorldMapCloud5Tex"
static const ALIGN_ASSET(2) char gWorldMapCloud5Tex[] = dgWorldMapCloud5Tex;
#define dgWorldMapCloud6Tex "__OTR__textures/icon_item_field_static/gWorldMapCloud6Tex"
static const ALIGN_ASSET(2) char gWorldMapCloud6Tex[] = dgWorldMapCloud6Tex;
#define dgWorldMapCloud7Tex "__OTR__textures/icon_item_field_static/gWorldMapCloud7Tex"
static const ALIGN_ASSET(2) char gWorldMapCloud7Tex[] = dgWorldMapCloud7Tex;
#define dgWorldMapCloud8Tex "__OTR__textures/icon_item_field_static/gWorldMapCloud8Tex"
static const ALIGN_ASSET(2) char gWorldMapCloud8Tex[] = dgWorldMapCloud8Tex;
#define dgWorldMapCloud9Tex "__OTR__textures/icon_item_field_static/gWorldMapCloud9Tex"
static const ALIGN_ASSET(2) char gWorldMapCloud9Tex[] = dgWorldMapCloud9Tex;
#define dgWorldMapCloud10Tex "__OTR__textures/icon_item_field_static/gWorldMapCloud10Tex"
static const ALIGN_ASSET(2) char gWorldMapCloud10Tex[] = dgWorldMapCloud10Tex;
#define dgWorldMapCloud11Tex "__OTR__textures/icon_item_field_static/gWorldMapCloud11Tex"
static const ALIGN_ASSET(2) char gWorldMapCloud11Tex[] = dgWorldMapCloud11Tex;
#define dgWorldMapCloud12Tex "__OTR__textures/icon_item_field_static/gWorldMapCloud12Tex"
static const ALIGN_ASSET(2) char gWorldMapCloud12Tex[] = dgWorldMapCloud12Tex;
#define dgWorldMapCloud13Tex "__OTR__textures/icon_item_field_static/gWorldMapCloud13Tex"
static const ALIGN_ASSET(2) char gWorldMapCloud13Tex[] = dgWorldMapCloud13Tex;
#define dgWorldMapCloud14Tex "__OTR__textures/icon_item_field_static/gWorldMapCloud14Tex"
static const ALIGN_ASSET(2) char gWorldMapCloud14Tex[] = dgWorldMapCloud14Tex;
#define dgWorldMapCloud15Tex "__OTR__textures/icon_item_field_static/gWorldMapCloud15Tex"
static const ALIGN_ASSET(2) char gWorldMapCloud15Tex[] = dgWorldMapCloud15Tex;
#define dgWorldMapCloud16Tex "__OTR__textures/icon_item_field_static/gWorldMapCloud16Tex"
static const ALIGN_ASSET(2) char gWorldMapCloud16Tex[] = dgWorldMapCloud16Tex;
#define dgPauseCurrentPositionENGTex "__OTR__textures/icon_item_nes_static/gPauseCurrentPositionENGTex"
static const ALIGN_ASSET(2) char gPauseCurrentPositionENGTex[] = dgPauseCurrentPositionENGTex;
#define dgPauseMap10ENGTex "__OTR__textures/icon_item_nes_static/gPauseMap10ENGTex"
static const ALIGN_ASSET(2) char gPauseMap10ENGTex[] = dgPauseMap10ENGTex;
#define dgPauseCurrentPositionGERTex "__OTR__textures/icon_item_ger_static/gPauseCurrentPositionGERTex"
static const ALIGN_ASSET(2) char gPauseCurrentPositionGERTex[] = dgPauseCurrentPositionGERTex;
#define dgPauseMap10GERTex "__OTR__textures/icon_item_ger_static/gPauseMap10GERTex"
static const ALIGN_ASSET(2) char gPauseMap10GERTex[] = dgPauseMap10GERTex;
#define dgPauseCurrentPositionFRATex "__OTR__textures/icon_item_fra_static/gPauseCurrentPositionFRATex"
static const ALIGN_ASSET(2) char gPauseCurrentPositionFRATex[] = dgPauseCurrentPositionFRATex;
#define dgPauseMap10FRATex "__OTR__textures/icon_item_fra_static/gPauseMap10FRATex"
static const ALIGN_ASSET(2) char gPauseMap10FRATex[] = dgPauseMap10FRATex;
#define dgPauseCurrentPositionJPNTex "__OTR__textures/icon_item_jpn_static/gPauseCurrentPositionJPNTex"
static const ALIGN_ASSET(2) char gPauseCurrentPositionJPNTex[] = dgPauseCurrentPositionJPNTex;
#define dgPauseMap10JPNTex "__OTR__textures/icon_item_jpn_static/gPauseMap10JPNTex"
static const ALIGN_ASSET(2) char gPauseMap10JPNTex[] = dgPauseMap10JPNTex;
#define dgHyruleFieldPositionNameENGTex "__OTR__textures/map_name_static/gHyruleFieldPositionNameENGTex"
static const ALIGN_ASSET(2) char gHyruleFieldPositionNameENGTex[] = dgHyruleFieldPositionNameENGTex;
#define dgKakarikoVillagePositionNameENGTex "__OTR__textures/map_name_static/gKakarikoVillagePositionNameENGTex"
static const ALIGN_ASSET(2) char gKakarikoVillagePositionNameENGTex[] = dgKakarikoVillagePositionNameENGTex;
#define dgGraveyardPositionNameENGTex "__OTR__textures/map_name_static/gGraveyardPositionNameENGTex"
static const ALIGN_ASSET(2) char gGraveyardPositionNameENGTex[] = dgGraveyardPositionNameENGTex;
#define dgZorasRiverPositionNameENGTex "__OTR__textures/map_name_static/gZorasRiverPositionNameENGTex"
static const ALIGN_ASSET(2) char gZorasRiverPositionNameENGTex[] = dgZorasRiverPositionNameENGTex;
#define dgKokiriForestPositionNameENGTex "__OTR__textures/map_name_static/gKokiriForestPositionNameENGTex"
static const ALIGN_ASSET(2) char gKokiriForestPositionNameENGTex[] = dgKokiriForestPositionNameENGTex;
#define dgSacredForestMeadowPositionNameENGTex "__OTR__textures/map_name_static/gSacredForestMeadowPositionNameENGTex"
static const ALIGN_ASSET(2) char gSacredForestMeadowPositionNameENGTex[] = dgSacredForestMeadowPositionNameENGTex;
#define dgLakeHyliaPositionNameENGTex "__OTR__textures/map_name_static/gLakeHyliaPositionNameENGTex"
static const ALIGN_ASSET(2) char gLakeHyliaPositionNameENGTex[] = dgLakeHyliaPositionNameENGTex;
#define dgZorasDomainPositionNameENGTex "__OTR__textures/map_name_static/gZorasDomainPositionNameENGTex"
static const ALIGN_ASSET(2) char gZorasDomainPositionNameENGTex[] = dgZorasDomainPositionNameENGTex;
#define dgZorasFountainPositionNameENGTex "__OTR__textures/map_name_static/gZorasFountainPositionNameENGTex"
static const ALIGN_ASSET(2) char gZorasFountainPositionNameENGTex[] = dgZorasFountainPositionNameENGTex;
#define dgGerudoValleyPositionNameENGTex "__OTR__textures/map_name_static/gGerudoValleyPositionNameENGTex"
static const ALIGN_ASSET(2) char gGerudoValleyPositionNameENGTex[] = dgGerudoValleyPositionNameENGTex;
#define dgLostWoodsPositionNameENGTex "__OTR__textures/map_name_static/gLostWoodsPositionNameENGTex"
static const ALIGN_ASSET(2) char gLostWoodsPositionNameENGTex[] = dgLostWoodsPositionNameENGTex;
#define dgDesertColossusPositionNameENGTex "__OTR__textures/map_name_static/gDesertColossusPositionNameENGTex"
static const ALIGN_ASSET(2) char gDesertColossusPositionNameENGTex[] = dgDesertColossusPositionNameENGTex;
#define dgGerudosFortressPositionNameENGTex "__OTR__textures/map_name_static/gGerudosFortressPositionNameENGTex"
static const ALIGN_ASSET(2) char gGerudosFortressPositionNameENGTex[] = dgGerudosFortressPositionNameENGTex;
#define dgHauntedWastelandPositionNameENGTex "__OTR__textures/map_name_static/gHauntedWastelandPositionNameENGTex"
static const ALIGN_ASSET(2) char gHauntedWastelandPositionNameENGTex[] = dgHauntedWastelandPositionNameENGTex;
#define dgMarketPositionNameENGTex "__OTR__textures/map_name_static/gMarketPositionNameENGTex"
static const ALIGN_ASSET(2) char gMarketPositionNameENGTex[] = dgMarketPositionNameENGTex;
#define dgHyruleCastlePositionNameENGTex "__OTR__textures/map_name_static/gHyruleCastlePositionNameENGTex"
static const ALIGN_ASSET(2) char gHyruleCastlePositionNameENGTex[] = dgHyruleCastlePositionNameENGTex;
#define dgDeathMountainTrailPositionNameENGTex "__OTR__textures/map_name_static/gDeathMountainTrailPositionNameENGTex"
static const ALIGN_ASSET(2) char gDeathMountainTrailPositionNameENGTex[] = dgDeathMountainTrailPositionNameENGTex;
#define dgDeathMountainCraterPositionNameENGTex "__OTR__textures/map_name_static/gDeathMountainCraterPositionNameENGTex"
static const ALIGN_ASSET(2) char gDeathMountainCraterPositionNameENGTex[] = dgDeathMountainCraterPositionNameENGTex;
#define dgGoronCityPositionNameENGTex "__OTR__textures/map_name_static/gGoronCityPositionNameENGTex"
static const ALIGN_ASSET(2) char gGoronCityPositionNameENGTex[] = dgGoronCityPositionNameENGTex;
#define dgLonLonRanchPositionNameENGTex "__OTR__textures/map_name_static/gLonLonRanchPositionNameENGTex"
static const ALIGN_ASSET(2) char gLonLonRanchPositionNameENGTex[] = dgLonLonRanchPositionNameENGTex;
#define dgQuestionMarkPositionNameENGTex "__OTR__textures/map_name_static/gQuestionMarkPositionNameENGTex"
static const ALIGN_ASSET(2) char gQuestionMarkPositionNameENGTex[] = dgQuestionMarkPositionNameENGTex;
#define dgGanonsCastlePositionNameENGTex "__OTR__textures/map_name_static/gGanonsCastlePositionNameENGTex"
static const ALIGN_ASSET(2) char gGanonsCastlePositionNameENGTex[] = dgGanonsCastlePositionNameENGTex;
#define dgHyruleFieldPositionNameGERTex "__OTR__textures/map_name_static/gHyruleFieldPositionNameGERTex"
static const ALIGN_ASSET(2) char gHyruleFieldPositionNameGERTex[] = dgHyruleFieldPositionNameGERTex;
#define dgKakarikoVillagePositionNameGERTex "__OTR__textures/map_name_static/gKakarikoVillagePositionNameGERTex"
static const ALIGN_ASSET(2) char gKakarikoVillagePositionNameGERTex[] = dgKakarikoVillagePositionNameGERTex;
#define dgGraveyardPositionNameGERTex "__OTR__textures/map_name_static/gGraveyardPositionNameGERTex"
static const ALIGN_ASSET(2) char gGraveyardPositionNameGERTex[] = dgGraveyardPositionNameGERTex;
#define dgZorasRiverPositionNameGERTex "__OTR__textures/map_name_static/gZorasRiverPositionNameGERTex"
static const ALIGN_ASSET(2) char gZorasRiverPositionNameGERTex[] = dgZorasRiverPositionNameGERTex;
#define dgKokiriForestPositionNameGERTex "__OTR__textures/map_name_static/gKokiriForestPositionNameGERTex"
static const ALIGN_ASSET(2) char gKokiriForestPositionNameGERTex[] = dgKokiriForestPositionNameGERTex;
#define dgSacredForestMeadowPositionNameGERTex "__OTR__textures/map_name_static/gSacredForestMeadowPositionNameGERTex"
static const ALIGN_ASSET(2) char gSacredForestMeadowPositionNameGERTex[] = dgSacredForestMeadowPositionNameGERTex;
#define dgLakeHyliaPositionNameGERTex "__OTR__textures/map_name_static/gLakeHyliaPositionNameGERTex"
static const ALIGN_ASSET(2) char gLakeHyliaPositionNameGERTex[] = dgLakeHyliaPositionNameGERTex;
#define dgZorasDomainPositionNameGERTex "__OTR__textures/map_name_static/gZorasDomainPositionNameGERTex"
static const ALIGN_ASSET(2) char gZorasDomainPositionNameGERTex[] = dgZorasDomainPositionNameGERTex;
#define dgZorasFountainPositionNameGERTex "__OTR__textures/map_name_static/gZorasFountainPositionNameGERTex"
static const ALIGN_ASSET(2) char gZorasFountainPositionNameGERTex[] = dgZorasFountainPositionNameGERTex;
#define dgGerudoValleyPositionNameGERTex "__OTR__textures/map_name_static/gGerudoValleyPositionNameGERTex"
static const ALIGN_ASSET(2) char gGerudoValleyPositionNameGERTex[] = dgGerudoValleyPositionNameGERTex;
#define dgLostWoodsPositionNameGERTex "__OTR__textures/map_name_static/gLostWoodsPositionNameGERTex"
static const ALIGN_ASSET(2) char gLostWoodsPositionNameGERTex[] = dgLostWoodsPositionNameGERTex;
#define dgDesertColossusPositionNameGERTex "__OTR__textures/map_name_static/gDesertColossusPositionNameGERTex"
static const ALIGN_ASSET(2) char gDesertColossusPositionNameGERTex[] = dgDesertColossusPositionNameGERTex;
#define dgGerudosFortressPositionNameGERTex "__OTR__textures/map_name_static/gGerudosFortressPositionNameGERTex"
static const ALIGN_ASSET(2) char gGerudosFortressPositionNameGERTex[] = dgGerudosFortressPositionNameGERTex;
#define dgHauntedWastelandPositionNameGERTex "__OTR__textures/map_name_static/gHauntedWastelandPositionNameGERTex"
static const ALIGN_ASSET(2) char gHauntedWastelandPositionNameGERTex[] = dgHauntedWastelandPositionNameGERTex;
#define dgMarketPositionNameGERTex "__OTR__textures/map_name_static/gMarketPositionNameGERTex"
static const ALIGN_ASSET(2) char gMarketPositionNameGERTex[] = dgMarketPositionNameGERTex;
#define dgHyruleCastlePositionNameGERTex "__OTR__textures/map_name_static/gHyruleCastlePositionNameGERTex"
static const ALIGN_ASSET(2) char gHyruleCastlePositionNameGERTex[] = dgHyruleCastlePositionNameGERTex;
#define dgDeathMountainTrailPositionNameGERTex "__OTR__textures/map_name_static/gDeathMountainTrailPositionNameGERTex"
static const ALIGN_ASSET(2) char gDeathMountainTrailPositionNameGERTex[] = dgDeathMountainTrailPositionNameGERTex;
#define dgDeathMountainCraterPositionNameGERTex "__OTR__textures/map_name_static/gDeathMountainCraterPositionNameGERTex"
static const ALIGN_ASSET(2) char gDeathMountainCraterPositionNameGERTex[] = dgDeathMountainCraterPositionNameGERTex;
#define dgGoronCityPositionNameGERTex "__OTR__textures/map_name_static/gGoronCityPositionNameGERTex"
static const ALIGN_ASSET(2) char gGoronCityPositionNameGERTex[] = dgGoronCityPositionNameGERTex;
#define dgLonLonRanchPositionNameGERTex "__OTR__textures/map_name_static/gLonLonRanchPositionNameGERTex"
static const ALIGN_ASSET(2) char gLonLonRanchPositionNameGERTex[] = dgLonLonRanchPositionNameGERTex;
#define dgQuestionMarkPositionNameGERTex "__OTR__textures/map_name_static/gQuestionMarkPositionNameGERTex"
static const ALIGN_ASSET(2) char gQuestionMarkPositionNameGERTex[] = dgQuestionMarkPositionNameGERTex;
#define dgGanonsCastlePositionNameGERTex "__OTR__textures/map_name_static/gGanonsCastlePositionNameGERTex"
static const ALIGN_ASSET(2) char gGanonsCastlePositionNameGERTex[] = dgGanonsCastlePositionNameGERTex;
#define dgHyruleFieldPositionNameFRATex "__OTR__textures/map_name_static/gHyruleFieldPositionNameFRATex"
static const ALIGN_ASSET(2) char gHyruleFieldPositionNameFRATex[] = dgHyruleFieldPositionNameFRATex;
#define dgKakarikoVillagePositionNameFRATex "__OTR__textures/map_name_static/gKakarikoVillagePositionNameFRATex"
static const ALIGN_ASSET(2) char gKakarikoVillagePositionNameFRATex[] = dgKakarikoVillagePositionNameFRATex;
#define dgGraveyardPositionNameFRATex "__OTR__textures/map_name_static/gGraveyardPositionNameFRATex"
static const ALIGN_ASSET(2) char gGraveyardPositionNameFRATex[] = dgGraveyardPositionNameFRATex;
#define dgZorasRiverPositionNameFRATex "__OTR__textures/map_name_static/gZorasRiverPositionNameFRATex"
static const ALIGN_ASSET(2) char gZorasRiverPositionNameFRATex[] = dgZorasRiverPositionNameFRATex;
#define dgKokiriForestPositionNameFRATex "__OTR__textures/map_name_static/gKokiriForestPositionNameFRATex"
static const ALIGN_ASSET(2) char gKokiriForestPositionNameFRATex[] = dgKokiriForestPositionNameFRATex;
#define dgSacredForestMeadowPositionNameFRATex "__OTR__textures/map_name_static/gSacredForestMeadowPositionNameFRATex"
static const ALIGN_ASSET(2) char gSacredForestMeadowPositionNameFRATex[] = dgSacredForestMeadowPositionNameFRATex;
#define dgLakeHyliaPositionNameFRATex "__OTR__textures/map_name_static/gLakeHyliaPositionNameFRATex"
static const ALIGN_ASSET(2) char gLakeHyliaPositionNameFRATex[] = dgLakeHyliaPositionNameFRATex;
#define dgZorasDomainPositionNameFRATex "__OTR__textures/map_name_static/gZorasDomainPositionNameFRATex"
static const ALIGN_ASSET(2) char gZorasDomainPositionNameFRATex[] = dgZorasDomainPositionNameFRATex;
#define dgZorasFountainPositionNameFRATex "__OTR__textures/map_name_static/gZorasFountainPositionNameFRATex"
static const ALIGN_ASSET(2) char gZorasFountainPositionNameFRATex[] = dgZorasFountainPositionNameFRATex;
#define dgGerudoValleyPositionNameFRATex "__OTR__textures/map_name_static/gGerudoValleyPositionNameFRATex"
static const ALIGN_ASSET(2) char gGerudoValleyPositionNameFRATex[] = dgGerudoValleyPositionNameFRATex;
#define dgLostWoodsPositionNameFRATex "__OTR__textures/map_name_static/gLostWoodsPositionNameFRATex"
static const ALIGN_ASSET(2) char gLostWoodsPositionNameFRATex[] = dgLostWoodsPositionNameFRATex;
#define dgDesertColossusPositionNameFRATex "__OTR__textures/map_name_static/gDesertColossusPositionNameFRATex"
static const ALIGN_ASSET(2) char gDesertColossusPositionNameFRATex[] = dgDesertColossusPositionNameFRATex;
#define dgGerudosFortressPositionNameFRATex "__OTR__textures/map_name_static/gGerudosFortressPositionNameFRATex"
static const ALIGN_ASSET(2) char gGerudosFortressPositionNameFRATex[] = dgGerudosFortressPositionNameFRATex;
#define dgHauntedWastelandPositionNameFRATex "__OTR__textures/map_name_static/gHauntedWastelandPositionNameFRATex"
static const ALIGN_ASSET(2) char gHauntedWastelandPositionNameFRATex[] = dgHauntedWastelandPositionNameFRATex;
#define dgMarketPositionNameFRATex "__OTR__textures/map_name_static/gMarketPositionNameFRATex"
static const ALIGN_ASSET(2) char gMarketPositionNameFRATex[] = dgMarketPositionNameFRATex;
#define dgHyruleCastlePositionNameFRATex "__OTR__textures/map_name_static/gHyruleCastlePositionNameFRATex"
static const ALIGN_ASSET(2) char gHyruleCastlePositionNameFRATex[] = dgHyruleCastlePositionNameFRATex;
#define dgDeathMountainTrailPositionNameFRATex "__OTR__textures/map_name_static/gDeathMountainTrailPositionNameFRATex"
static const ALIGN_ASSET(2) char gDeathMountainTrailPositionNameFRATex[] = dgDeathMountainTrailPositionNameFRATex;
#define dgDeathMountainCraterPositionNameFRATex "__OTR__textures/map_name_static/gDeathMountainCraterPositionNameFRATex"
static const ALIGN_ASSET(2) char gDeathMountainCraterPositionNameFRATex[] = dgDeathMountainCraterPositionNameFRATex;
#define dgGoronCityPositionNameFRATex "__OTR__textures/map_name_static/gGoronCityPositionNameFRATex"
static const ALIGN_ASSET(2) char gGoronCityPositionNameFRATex[] = dgGoronCityPositionNameFRATex;
#define dgLonLonRanchPositionNameFRATex "__OTR__textures/map_name_static/gLonLonRanchPositionNameFRATex"
static const ALIGN_ASSET(2) char gLonLonRanchPositionNameFRATex[] = dgLonLonRanchPositionNameFRATex;
#define dgQuestionMarkPositionNameFRATex "__OTR__textures/map_name_static/gQuestionMarkPositionNameFRATex"
static const ALIGN_ASSET(2) char gQuestionMarkPositionNameFRATex[] = dgQuestionMarkPositionNameFRATex;
#define dgGanonsCastlePositionNameFRATex "__OTR__textures/map_name_static/gGanonsCastlePositionNameFRATex"
static const ALIGN_ASSET(2) char gGanonsCastlePositionNameFRATex[] = dgGanonsCastlePositionNameFRATex;
#define dgHyruleFieldPositionNameJPNTex "__OTR__textures/map_name_static/gHyruleFieldPositionNameJPNTex"
static const ALIGN_ASSET(2) char gHyruleFieldPositionNameJPNTex[] = dgHyruleFieldPositionNameJPNTex;
#define dgKakarikoVillagePositionNameJPNTex "__OTR__textures/map_name_static/gKakarikoVillagePositionNameJPNTex"
static const ALIGN_ASSET(2) char gKakarikoVillagePositionNameJPNTex[] = dgKakarikoVillagePositionNameJPNTex;
#define dgGraveyardPositionNameJPNTex "__OTR__textures/map_name_static/gGraveyardPositionNameJPNTex"
static const ALIGN_ASSET(2) char gGraveyardPositionNameJPNTex[] = dgGraveyardPositionNameJPNTex;
#define dgZorasRiverPositionNameJPNTex "__OTR__textures/map_name_static/gZorasRiverPositionNameJPNTex"
static const ALIGN_ASSET(2) char gZorasRiverPositionNameJPNTex[] = dgZorasRiverPositionNameJPNTex;
#define dgKokiriForestPositionNameJPNTex "__OTR__textures/map_name_static/gKokiriForestPositionNameJPNTex"
static const ALIGN_ASSET(2) char gKokiriForestPositionNameJPNTex[] = dgKokiriForestPositionNameJPNTex;
#define dgSacredForestMeadowPositionNameJPNTex "__OTR__textures/map_name_static/gSacredForestMeadowPositionNameJPNTex"
static const ALIGN_ASSET(2) char gSacredForestMeadowPositionNameJPNTex[] = dgSacredForestMeadowPositionNameJPNTex;
#define dgLakeHyliaPositionNameJPNTex "__OTR__textures/map_name_static/gLakeHyliaPositionNameJPNTex"
static const ALIGN_ASSET(2) char gLakeHyliaPositionNameJPNTex[] = dgLakeHyliaPositionNameJPNTex;
#define dgZorasDomainPositionNameJPNTex "__OTR__textures/map_name_static/gZorasDomainPositionNameJPNTex"
static const ALIGN_ASSET(2) char gZorasDomainPositionNameJPNTex[] = dgZorasDomainPositionNameJPNTex;
#define dgZorasFountainPositionNameJPNTex "__OTR__textures/map_name_static/gZorasFountainPositionNameJPNTex"
static const ALIGN_ASSET(2) char gZorasFountainPositionNameJPNTex[] = dgZorasFountainPositionNameJPNTex;
#define dgGerudoValleyPositionNameJPNTex "__OTR__textures/map_name_static/gGerudoValleyPositionNameJPNTex"
static const ALIGN_ASSET(2) char gGerudoValleyPositionNameJPNTex[] = dgGerudoValleyPositionNameJPNTex;
#define dgLostWoodsPositionNameJPNTex "__OTR__textures/map_name_static/gLostWoodsPositionNameJPNTex"
static const ALIGN_ASSET(2) char gLostWoodsPositionNameJPNTex[] = dgLostWoodsPositionNameJPNTex;
#define dgDesertColossusPositionNameJPNTex "__OTR__textures/map_name_static/gDesertColossusPositionNameJPNTex"
static const ALIGN_ASSET(2) char gDesertColossusPositionNameJPNTex[] = dgDesertColossusPositionNameJPNTex;
#define dgGerudosFortressPositionNameJPNTex "__OTR__textures/map_name_static/gGerudosFortressPositionNameJPNTex"
static const ALIGN_ASSET(2) char gGerudosFortressPositionNameJPNTex[] = dgGerudosFortressPositionNameJPNTex;
#define dgHauntedWastelandPositionNameJPNTex "__OTR__textures/map_name_static/gHauntedWastelandPositionNameJPNTex"
static const ALIGN_ASSET(2) char gHauntedWastelandPositionNameJPNTex[] = dgHauntedWastelandPositionNameJPNTex;
#define dgMarketPositionNameJPNTex "__OTR__textures/map_name_static/gMarketPositionNameJPNTex"
static const ALIGN_ASSET(2) char gMarketPositionNameJPNTex[] = dgMarketPositionNameJPNTex;
#define dgHyruleCastlePositionNameJPNTex "__OTR__textures/map_name_static/gHyruleCastlePositionNameJPNTex"
static const ALIGN_ASSET(2) char gHyruleCastlePositionNameJPNTex[] = dgHyruleCastlePositionNameJPNTex;
#define dgDeathMountainTrailPositionNameJPNTex "__OTR__textures/map_name_static/gDeathMountainTrailPositionNameJPNTex"
static const ALIGN_ASSET(2) char gDeathMountainTrailPositionNameJPNTex[] = dgDeathMountainTrailPositionNameJPNTex;
#define dgDeathMountainCraterPositionNameJPNTex "__OTR__textures/map_name_static/gDeathMountainCraterPositionNameJPNTex"
static const ALIGN_ASSET(2) char gDeathMountainCraterPositionNameJPNTex[] = dgDeathMountainCraterPositionNameJPNTex;
#define dgGoronCityPositionNameJPNTex "__OTR__textures/map_name_static/gGoronCityPositionNameJPNTex"
static const ALIGN_ASSET(2) char gGoronCityPositionNameJPNTex[] = dgGoronCityPositionNameJPNTex;
#define dgLonLonRanchPositionNameJPNTex "__OTR__textures/map_name_static/gLonLonRanchPositionNameJPNTex"
static const ALIGN_ASSET(2) char gLonLonRanchPositionNameJPNTex[] = dgLonLonRanchPositionNameJPNTex;
#define dgQuestionMarkPositionNameJPNTex "__OTR__textures/map_name_static/gQuestionMarkPositionNameJPNTex"
static const ALIGN_ASSET(2) char gQuestionMarkPositionNameJPNTex[] = dgQuestionMarkPositionNameJPNTex;
#define dgGanonsCastlePositionNameJPNTex "__OTR__textures/map_name_static/gGanonsCastlePositionNameJPNTex"
static const ALIGN_ASSET(2) char gGanonsCastlePositionNameJPNTex[] = dgGanonsCastlePositionNameJPNTex;

#define dgQuestIconGoldSkulltulaTex "__OTR__textures/icon_item_24_static/gQuestIconGoldSkulltulaTex"
static const ALIGN_ASSET(2) char gQuestIconGoldSkulltulaTex[] = dgQuestIconGoldSkulltulaTex;
#define dgWorldMapImageTex "__OTR__textures/icon_item_field_static/gWorldMapImageTex"
static const ALIGN_ASSET(2) char gWorldMapImageTex[] = dgWorldMapImageTex;
#define dgWorldMapImageTLUT "__OTR__textures/icon_item_field_static/gWorldMapImageTLUT"
static const ALIGN_ASSET(2) char gWorldMapImageTLUT[] = dgWorldMapImageTLUT;

// Minish Cap item icon (Skijer-custom, not in OoT/MM) — OTR-path stub; add the real texture later.
#define dgItemIconPecoriTex "__OTR__icon_item_custom/gItemIconPecoriTex"
static const ALIGN_ASSET(2) char gItemIconPecoriTex[] = dgItemIconPecoriTex;
#endif // MINISH_OOT_MAP_ASSETS_H
