#pragma once
// FleetOracle.h — Oráculo lógico del Combo Randomizer (lado MM / 2ship).
//
// El host (Ship/soh) pregunta "con este inventario, ¿qué checks de MM son alcanzables?" durante la
// generación de la seed combinada (assumed fill). Este módulo responde usando la LÓGICA REAL del
// rando de 2ship (FindReachableRegions + GiveItem sobre una copia de gSaveContext), así que los
// pulls de upstream actualizan la lógica del combo gratis.
//
// Protocolo (ver FleetOracle.cpp): request/response por archivos JSON en el dir del Ship host
// (fleet_oracle_req.json / fleet_oracle_resp.json), señalización seq/ack por FscShared
// reservedU[4]/[5] (FleetShipCombo.h). Se procesa por frame vía OnGameStateUpdate, también con el
// juego congelado (mismo mecanismo que FleetSync::ProcessSignals).
//
// Ops soportadas:
//   "manifest"  -> checks (id+nombre), pool por defecto (spoilerNames), starting items y opciones
//                  actuales del rando de MM. El host lo pide UNA vez por generación (interfaz World).
//   "reachable" -> fcItems ([[fcId, count]...], resueltos vía FleetComboItems) + mmItems
//                  ([["RI_NAME", count]...]) -> lista de RandoCheckId alcanzables.
//
// Registro automático via ShipInit; no requiere llamadas externas.

// Rebuild the MM save in `slot` from scratch and write it straight to disk: native Sram_InitNewSave
// plus the combo spoiler (applied by the OnSaveInit hook), same recipe as the oracle's "createSave"
// op. In fleet mode MM's file is DERIVED -- OoT holds the seed -- so a missing or rejected MM save
// is not data loss, it is something we can regenerate. Returns false if the rebuild failed.
bool FleetOracle_RecreateSaveSlot(int slot, const char* name);
