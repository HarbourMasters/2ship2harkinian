/*
 * mm_bgm_loader.cpp — 2ship port: NO-OP STUB (user decision: redundant in 2ship).
 *
 * The SoH-side "inject MM sequences/fonts into the audio engine" path depended on
 * SoH audio-engine extensions that DON'T exist in 2ship (AudioLoad_RegisterMmFont/
 * Sequence, AudioLoad_FindNextFree*, Audio_PrimeMmSideChannel, the gFontMap/
 * gSequenceMap globals). In 2ship the game IS Majora's Mask, so its music plays
 * natively and this cross-game BGM injection is redundant.
 *
 * Per the design note in mm_bgm_loader.h ("If mm.o2r is not loaded, every call
 * here is a silent no-op"), all entry points are no-ops here — callers
 * (mm_asset_loader, mm_mask_wear) keep linking and simply get silence. If
 * cross-game MM BGM is ever needed (e.g. FleetShipCombo on the OoT side), re-port
 * the engine extensions into 2ship's audio/lib/load.c.
 */
#include "mm_bgm_loader.h"

extern "C" void MmBgm_RegisterSequences(void) {}

extern "C" s32 MmBgm_IsAvailable(void) {
    return 0;
}

extern "C" u16 MmBgm_GetSeqId(const char* mmBgmName) {
    (void)mmBgmName;
    return 0xFFFF; // NA_BGM_DISABLED
}

extern "C" void MmBgm_PlayFanfare(const char* mmBgmName) {
    (void)mmBgmName;
}

extern "C" void MmBgm_PlayMain(const char* mmBgmName) {
    (void)mmBgmName;
}

extern "C" void MmBgm_PlayLoop(const char* mmBgmName) {
    (void)mmBgmName;
}

extern "C" void MmBgm_RestorePreviousBgm(void) {}

extern "C" void MmBgm_StopFanfare(void) {}
