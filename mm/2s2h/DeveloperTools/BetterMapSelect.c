
#include "BetterMapSelect.h"

#include "overlays/gamestates/ovl_file_choose/z_file_select.h"
#include "overlays/gamestates/ovl_select/z_select.h"
#include <libultraship/bridge.h>
#include "BenPort.h"

extern SceneSelectEntry sScenes[143];

void BetterMapSelect_LoadGame(MapSelectState* mapSelectState, u32 entrance, s32 spawn) {
    CVarSetInteger("gDeveloperTools.BetterMapSelect.CurrentScene", mapSelectState->currentScene);
    CVarSetInteger("gDeveloperTools.BetterMapSelect.TopDisplayedScene", mapSelectState->topDisplayedScene);
    CVarSetInteger("gDeveloperTools.BetterMapSelect.PageDownIndex", mapSelectState->pageDownIndex);
    CVarSave();
    MapSelect_LoadGame(mapSelectState, entrance, spawn);
}

void BetterMapSelect_LoadFileSelect(MapSelectState* mapSelectState) {
    CVarSetInteger("gDeveloperTools.BetterMapSelect.CurrentScene", mapSelectState->currentScene);
    CVarSetInteger("gDeveloperTools.BetterMapSelect.TopDisplayedScene", mapSelectState->topDisplayedScene);
    CVarSetInteger("gDeveloperTools.BetterMapSelect.PageDownIndex", mapSelectState->pageDownIndex);
    CVarSave();
    STOP_GAMESTATE(&mapSelectState->state);
    if (ResourceMgr_GetGameDefaultLanguage(0) == LANGUAGE_JPN) {
        SET_NEXT_GAMESTATE(&mapSelectState->state, FileSelect_JP_Init, sizeof(FileSelectState));
    } else {
        SET_NEXT_GAMESTATE(&mapSelectState->state, FileSelect_Init, sizeof(FileSelectState));
    }
}

// 2S2H Added columns to scene table: entranceSceneId, betterMapSelectIndex, humanName
#define DEFINE_SCENE(_name, _enumValue, _textId, _drawConfig, _restrictionFlags, _persistentCycleFlags, \
                     entranceSceneId, _betterMapSelectIndex, humanName)                                 \
    { humanName, BetterMapSelect_LoadGame, ENTRANCE(entranceSceneId, 0) },
#define DEFINE_SCENE_UNSET(_enumValue)

static SceneSelectEntry sBetterScenes[104] = {
#include "tables/scene_table.h"
    { "File Select", BetterMapSelect_LoadFileSelect, 0 },
    { "Title Screen", MapSelect_LoadConsoleLogo, 0 },
};

#undef DEFINE_SCENE
#undef DEFINE_SCENE_UNSET

// 2S2H Added columns to scene table: entranceSceneId, betterMapSelectIndex, humanName
#define DEFINE_SCENE(_name, _enumValue, _textId, _drawConfig, _restrictionFlags, _persistentCycleFlags, \
                     entranceSceneId, betterMapSelectIndex, humanName)                                  \
    { humanName, ENTRANCE(entranceSceneId, 0), betterMapSelectIndex },
#define DEFINE_SCENE_UNSET(_enumValue)

typedef struct {
    char* name;
    s32 entrance;
    s32 index;
} BetterMapSelectInfoEntry;

static BetterMapSelectInfoEntry sBetterMapSelectInfo[102] = {
#include "tables/scene_table.h"
};

#undef DEFINE_SCENE
#undef DEFINE_SCENE_UNSET

static bool isBetterMapSelect = false;
static bool isInitialized = false;

void BetterMapSelect_Init(MapSelectState* mapSelectState) {
    isBetterMapSelect = CVarGetInteger("gDeveloperTools.BetterMapSelect.Enabled", 0);
    if (CVarGetInteger("gDeveloperTools.BetterMapSelect.Enabled", 0)) {
        if (!isInitialized) {
            s32 i;
            for (i = 0; i < ARRAY_COUNT(sBetterMapSelectInfo); i++) {
                sBetterScenes[sBetterMapSelectInfo[i].index].name = sBetterMapSelectInfo[i].name;
                sBetterScenes[sBetterMapSelectInfo[i].index].entrance = sBetterMapSelectInfo[i].entrance;
            }
            isInitialized = true;
        }

        mapSelectState->scenes = sBetterScenes;
        mapSelectState->count = ARRAY_COUNT(sBetterScenes);
        mapSelectState->currentScene = CVarGetInteger("gDeveloperTools.BetterMapSelect.CurrentScene", 0);
        mapSelectState->topDisplayedScene = CVarGetInteger("gDeveloperTools.BetterMapSelect.TopDisplayedScene", 0);
        mapSelectState->pageDownIndex = CVarGetInteger("gDeveloperTools.BetterMapSelect.PageDownIndex", 0);
    } else {
        mapSelectState->scenes = sScenes;
        mapSelectState->count = ARRAY_COUNT(sScenes);
        mapSelectState->currentScene = 0;
        mapSelectState->topDisplayedScene = 0;
        mapSelectState->pageDownIndex = 0;
    }
}

void BetterMapSelect_Update(MapSelectState* mapSelectState) {
    if (isBetterMapSelect != CVarGetInteger("gDeveloperTools.BetterMapSelect.Enabled", 0)) {
        BetterMapSelect_Init(mapSelectState);
    }
}

static const char* betterFormLabels[] = {
    "Deity", "Goron", "Zora", "Deku", "Child",
};

void BetterMapSelect_PrintMenu(MapSelectState* mapSelectState, GfxPrint* printer) {
    s32 i;
    s32 sceneIndex;
    char* sceneName;
    char* stageName;
    char* dayName;

    // Header
    GfxPrint_SetColor(printer, 255, 255, 255, 255);
    GfxPrint_SetPos(printer, 12, 2);
    GfxPrint_Printf(printer, "Scene Selection");
    GfxPrint_SetColor(printer, 255, 255, 255, 255);

    // Scenes
    for (i = 0; i < 20; i++) {
        GfxPrint_SetPos(printer, 3, i + 4);

        sceneIndex = (mapSelectState->topDisplayedScene + i + mapSelectState->count) % mapSelectState->count;
        if (sceneIndex == mapSelectState->currentScene) {
            GfxPrint_SetColor(printer, 255, 100, 100, 255);
        } else {
            GfxPrint_SetColor(printer, 175, 175, 175, 255);
        }

        sceneName = sBetterScenes[sceneIndex].name;
        GfxPrint_Printf(printer, "%3d %s", sceneIndex, sceneName);
    };

    // Entrance
    GfxPrint_SetColor(printer, 100, 100, 100, 255);
    GfxPrint_SetPos(printer, 28, 26);
    GfxPrint_Printf(printer, "Entrance:");
    GfxPrint_SetColor(printer, 200, 200, 50, 255);
    GfxPrint_Printf(printer, "%2d", mapSelectState->opt);

    // Form
    GfxPrint_SetPos(printer, 26, 25);
    GfxPrint_SetColor(printer, 100, 100, 100, 255);
    GfxPrint_Printf(printer, "(B)Form:");
    GfxPrint_SetColor(printer, 55, 200, 50, 255);
    GfxPrint_Printf(printer, "%s", betterFormLabels[GET_PLAYER_FORM]);

    // Day
    GfxPrint_SetPos(printer, 1, 25);
    GfxPrint_SetColor(printer, 100, 100, 100, 255);
    GfxPrint_Printf(printer, "Day:");
    GfxPrint_SetColor(printer, 100, 100, 200, 255);
    switch (gSaveContext.save.day) {
        case 1:
            dayName = "First Day";
            break;
        case 2:
            dayName = "Second Day";
            break;
        case 3:
            dayName = "Final Day";
            break;
        case 4:
            dayName = "Clear Day";
            break;
        default:
            gSaveContext.save.day = 1;
            dayName = "First Day";
            break;
    }
    GfxPrint_Printf(printer, "%s", dayName);

    // Stage
    GfxPrint_SetPos(printer, 1, 26);
    GfxPrint_SetColor(printer, 100, 100, 100, 255);
    GfxPrint_Printf(printer, "(Z/R)Stage:");
    GfxPrint_SetColor(printer, 200, 100, 200, 255);
    switch (gSaveContext.save.cutsceneIndex) {
        case 0:
            gSaveContext.save.time = CLOCK_TIME(12, 0);
            stageName = "Afternoon";
            break;
        case 0x8000:
            gSaveContext.save.time = CLOCK_TIME(6, 0) + 1;
            stageName = "Morning";
            break;
        case 0x8800:
            gSaveContext.save.time = CLOCK_TIME(18, 1);
            stageName = "Night";
            break;
        case 0xFFF0:
            gSaveContext.save.time = CLOCK_TIME(12, 0);
            stageName = "0";
            break;
        case 0xFFF1:
            stageName = "1";
            break;
        case 0xFFF2:
            stageName = "2";
            break;
        case 0xFFF3:
            stageName = "3";
            break;
        case 0xFFF4:
            stageName = "4";
            break;
        case 0xFFF5:
            stageName = "5";
            break;
        case 0xFFF6:
            stageName = "6";
            break;
        case 0xFFF7:
            stageName = "7";
            break;
        case 0xFFF8:
            stageName = "8";
            break;
        case 0xFFF9:
            stageName = "9";
            break;
        case 0xFFFA:
            stageName = "A";
            break;
        default:
            stageName = "???";
            break;
    }
    GfxPrint_Printf(printer, "%s", stageName);
};
