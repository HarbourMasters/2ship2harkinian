// FleetComboItemsGlue.cpp (MM side) — resuelve la tabla FC compartida a RandoItemId (RI_*).
//
// Mirror del lado OoT (soh/soh/FleetShipCombo/FleetComboItemsGlue.cpp, que resuelve a RG_*).
// La columna rgToken de la X-macro NUNCA se expande aquí: los tokens RG_* no existen en 2ship
// y no hace falta que existan (los argumentos de macro no emitidos son solo tokens).

#include "FleetComboItemsGlue.h"
#include "FleetComboItems.h"
#include "FleetComboIds.h" // FC_COMBO_OBTAINED_FC_SIZE
#include "2s2h/Rando/StaticData/StaticData.h"
#include "2s2h/ShipInit.hpp"

#include <cstring>
#include <unordered_map>
#include <spdlog/spdlog.h>

namespace {

// Columna RI de la tabla compartida (índices alineados 1:1 con gFcComboItems / FcComboItemId).
const int sFcNative[] = {
#define X(id, chainLen, flags, rg, ri, comboName, ootName, mmName) (int)(ri),
    FC_COMBO_ITEM_LIST(X)
#undef X
};
static_assert(sizeof(sFcNative) / sizeof(sFcNative[0]) == FCI_MAX, "FleetComboItems.h desalineado con FCI_MAX");
static_assert(FCI_MAX <= FC_COMBO_OBTAINED_FC_SIZE, "comboObtainedFc[] demasiado pequeño para la tabla FC");

// Tokens stringificados: el spoilerName de 2ship ES el nombre del enum ("RI_HOOKSHOT"), así que
// la stringificación del token es la referencia exacta para validar y para el spoiler.
const char* sFcNativeName[] = {
#define X(id, chainLen, flags, rg, ri, comboName, ootName, mmName) #ri,
    FC_COMBO_ITEM_LIST(X)
#undef X
};
const char* sFcPeerName[] = {
#define X(id, chainLen, flags, rg, ri, comboName, ootName, mmName) #rg,
    FC_COMBO_ITEM_LIST(X)
#undef X
};

std::unordered_map<int, int>& ReverseMap() {
    static std::unordered_map<int, int> map = [] {
        std::unordered_map<int, int> m;
        for (int i = 0; i < FCI_MAX; i++) {
            if (sFcNative[i] != FCI_NO_ITEM) {
                m.emplace(sFcNative[i], i);
            }
        }
        return m;
    }();
    return map;
}

} // namespace

extern "C" int FcCombo_NativeForItem(int fcId) {
    if (fcId < 0 || fcId >= FCI_MAX) {
        return FCI_NO_ITEM;
    }
    return sFcNative[fcId];
}

extern "C" int FcCombo_ItemForNative(int nativeId) {
    auto& m = ReverseMap();
    auto it = m.find(nativeId);
    return it == m.end() ? FCI_NO_ITEM : it->second;
}

extern "C" const char* FcCombo_NativeNameForItem(int fcId) {
    if (fcId < 0 || fcId >= FCI_MAX) {
        return "FCI_NO_ITEM";
    }
    return sFcNativeName[fcId];
}

extern "C" const char* FcCombo_PeerNameForItem(int fcId) {
    if (fcId < 0 || fcId >= FCI_MAX) {
        return "FCI_NO_ITEM";
    }
    return sFcPeerName[fcId];
}

extern "C" void FcCombo_ValidateTable(void) {
    if (Rando::StaticData::Items.empty()) {
        SPDLOG_WARN("[FcCombo] tabla de items de 2ship aún no inicializada — validación diferida");
        return;
    }
    int issues = 0;
    for (int i = 0; i < FC_COMBO_ITEM_COUNT; i++) {
        const FcComboItemInfo& row = gFcComboItems[i];
        int ri = sFcNative[i];
        if (ri == FCI_NO_ITEM) {
            continue;
        }
        auto it = Rando::StaticData::Items.find((RandoItemId)ri);
        if (it == Rando::StaticData::Items.end()) {
            SPDLOG_WARN("[FcCombo] drift MM: {} — el RI {} ya no está en StaticData::Items", row.comboName, ri);
            issues++;
            continue;
        }
        // El spoilerName de 2ship es el nombre del enum: comparamos contra el token stringificado
        // (mmName de la tabla es solo el nombre legible, no participa en la validación).
        if (strcmp(it->second.spoilerName, sFcNativeName[i]) != 0) {
            SPDLOG_WARN("[FcCombo] drift MM: {} — token dice \"{}\", spoilerName del juego dice \"{}\"", row.comboName,
                        sFcNativeName[i], it->second.spoilerName);
            issues++;
        }
    }
    SPDLOG_INFO("[FcCombo] validación MM: {} items compartidos, {} discrepancias", FC_COMBO_ITEM_COUNT, issues);
}

static void RegisterFcComboItems() {
    FcCombo_ValidateTable();
}

static RegisterShipInitFunc initFcComboItems(RegisterFcComboItems, {});
