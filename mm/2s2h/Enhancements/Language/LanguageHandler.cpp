#include "public/bridge/consolevariablebridge.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "variables.h"
extern PlayState* gPlayState;
extern SaveContext gSaveContext;
void Message_SetTables(PlayState* play);
}

extern "C" MessageTableEntry* sMessageTableNES;
extern "C" MessageTableEntry* sMessageTableJPN;
extern "C" MessageTableEntry* sMessageTableGER;
extern "C" MessageTableEntry* sMessageTableFRA;
extern "C" MessageTableEntry* sMessageTableESP;

#define CVAR_NAME "gSettings.Language"
#define CVAR CVarGetInteger(CVAR_NAME, LANGUAGE_ENG)

void RegisterLanguageHandler() {

    COND_HOOK(OnGameStateUpdate, true, []() {
        uint8_t oldLanguage = gSaveContext.options.language;
        gSaveContext.options.language = CVAR;

        if (gSaveContext.options.language == LANGUAGE_GER && sMessageTableGER == NULL) {
            gSaveContext.options.language = LANGUAGE_ENG;
            CVarSetInteger(CVAR_NAME, LANGUAGE_ENG);
        } else if (gSaveContext.options.language == LANGUAGE_FRE && sMessageTableFRA == NULL) {
            gSaveContext.options.language = LANGUAGE_ENG;
            CVarSetInteger(CVAR_NAME, LANGUAGE_ENG);
        } else if (gSaveContext.options.language == LANGUAGE_SPA && sMessageTableESP == NULL) {
            gSaveContext.options.language = LANGUAGE_ENG;
            CVarSetInteger(CVAR_NAME, LANGUAGE_ENG);
        }

        if (gSaveContext.options.language == LANGUAGE_ENG && sMessageTableNES == NULL) {
            gSaveContext.options.language = LANGUAGE_JPN;
            CVarSetInteger(CVAR_NAME, LANGUAGE_JPN);
        } else if (gSaveContext.options.language == LANGUAGE_JPN && sMessageTableJPN == NULL) {
            gSaveContext.options.language = LANGUAGE_ENG;
            CVarSetInteger(CVAR_NAME, LANGUAGE_ENG);
        }

        // TODO: Figure out what condition to put this in (GameInteractor_ExecuteOnOpenText?)
        if (gPlayState != nullptr && gSaveContext.options.language != oldLanguage) {
            Message_SetTables(gPlayState);
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterLanguageHandler, { CVAR_NAME });
