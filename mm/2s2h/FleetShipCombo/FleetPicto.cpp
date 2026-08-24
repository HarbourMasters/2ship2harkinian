/**
 * FleetPicto.cpp — the ONE pictograph of a Fleet Ship Combo run (Skijer's NEI).
 *
 * In combo mode Ocarina of Time and Majora's Mask do not each keep a picture: there is a single
 * pictograph, and whichever game you shoot it in is the one that holds it. That works because both
 * sides already store it in MM's format — this side is vanilla MM (gSaveContext.pictoPhotoI5, the
 * 11200-byte I5 buffer, plus save.saveInfo.pictoFlags0/1, the two u32 PICTO_VALID_* bit-sets), and
 * Ship's NEI pictograph was ported to that exact layout on purpose. So the bridge is a byte copy in
 * both directions, no conversion anywhere:
 *
 *   <ShipDir>/fleet/picture.bin      = pictoPhotoI5            (11200 bytes, byte array, no endianness)
 *   <ShipDir>/fleet/pictoflags.bin   = pictoFlags0, pictoFlags1 (8 bytes, native order — both PC ports
 *                                      hold them native)
 *   <ShipDir>/fleet/picture_rgba.bin = the COLOUR print, 160x112 RGBA16 (35840 bytes)
 *
 * The colour file exists because the I5 buffer is greyscale by construction — it is all Majora's Mask
 * ever stored, so a picture crossing on that alone arrives in sepia no matter what either game had on
 * screen. Colour is the ColorPictograph enhancement's own buffer on this side and sPictoColorTex on
 * Ship's, and both hold 160x112 RGBA16 with the same byte swap, so it crosses as a raw copy too. A
 * print with no colour half (taken while the other game was the one with the enhancement, or an older
 * run) still shows: it just falls back to MM's sepia.
 *
 * When: published the instant the picture is KEPT (z_parameter.c, the 0xF8 "Yes" branch) and deleted
 * the instant it is thrown away, so the other game has it whether or not a warp handoff ever happens.
 * Imported when a combo save is loaded and on arrival from a warp.
 *
 * The flags travel WITH the picture and REPLACE what was there — never merged. That is MM's own rule:
 * Snap_RecordPictographedActors clears both words and rewrites them for the photo being kept, so one
 * picture always means one set of subjects. Merging would leave Lulu "photographed" under a picture
 * of a scarecrow.
 *
 * Ownership of the Box itself is NOT here: it is an inventory item and FleetSync already carries it
 * (inv["pictobox"] <-> SLOT_PICTOGRAPH_BOX). A picture file must never hand anybody a Pictograph Box.
 *
 * Solo MM (no combo) touches none of this — the picture stays in the save, as vanilla.
 */

#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "FleetSync.h"

#include <fstream>
#include <filesystem>
#include <cstring>
#include <string>
#include <vector>
#include <spdlog/spdlog.h>

extern "C" {
#include "variables.h"
#include "z64save.h"
// The colour half of the print lives in the ColorPictograph enhancement's buffer
// (2s2h/Enhancements/Items/ColorPictograph.cpp).
const void* ColorPictograph_GetBuffer(void);
unsigned int ColorPictograph_GetBufferSize(void);
int ColorPictograph_HasImage(void);
void ColorPictograph_SetBuffer(const void* src);
}

#define PICTO_I5_SIZE ((160 * 112) * 5 / 8) // 11200 — MM's PICTO_PHOTO_COMPRESSED_SIZE

// A combo session is the only time the picture is shared. Same master toggle the rest of the combo
// uses; when it is off this file is inert.
static bool FleetPicto_IsCombo() {
    return CVarGetInteger("isFleetShipCombo.Enabled", 0) != 0;
}

static std::string FleetPicto_Path(const char* name) {
    const char* p = FleetSync_SharedFilePath(name);
    return (p != nullptr) ? std::string(p) : std::string();
}

// Publish the picture MM just kept. Called from z_parameter.c right after the keep branch has
// compressed it into gSaveContext.pictoPhotoI5 and recorded the subjects.
extern "C" void FleetPicto_OnPhotoKept(void) {
    if (!FleetPicto_IsCombo()) {
        return;
    }
    std::string picture = FleetPicto_Path("picture.bin");
    std::string flags = FleetPicto_Path("pictoflags.bin");
    if (picture.empty() || flags.empty()) {
        return;
    }
    {
        std::ofstream f(picture, std::ios::binary | std::ios::trunc);
        if (f) {
            f.write(reinterpret_cast<const char*>(gSaveContext.pictoPhotoI5), PICTO_I5_SIZE);
        }
    }
    {
        std::ofstream f(flags, std::ios::binary | std::ios::trunc);
        if (f) {
            uint32_t f0 = gSaveContext.save.saveInfo.pictoFlags0;
            uint32_t f1 = gSaveContext.save.saveInfo.pictoFlags1;
            f.write(reinterpret_cast<const char*>(&f0), sizeof(f0));
            f.write(reinterpret_cast<const char*>(&f1), sizeof(f1));
        }
    }
    // The colour print. ColorPictograph captures it on every shutter regardless of its CVar (only the
    // DISPLAY is gated), so it is there to publish whether or not this player runs in colour — which
    // is what lets the other game show it in colour even if this one didn't.
    if (ColorPictograph_HasImage()) {
        std::string rgba = FleetPicto_Path("picture_rgba.bin");
        if (!rgba.empty()) {
            std::ofstream f(rgba, std::ios::binary | std::ios::trunc);
            if (f) {
                f.write(reinterpret_cast<const char*>(ColorPictograph_GetBuffer()), ColorPictograph_GetBufferSize());
            }
        }
    }

    SPDLOG_INFO("[FleetPicto] published the combo pictograph (flags {:#x}/{:#x}, colour {})",
                gSaveContext.save.saveInfo.pictoFlags0, gSaveContext.save.saveInfo.pictoFlags1,
                ColorPictograph_HasImage() ? "yes" : "no");
}

// The picture was thrown away (0xF8 "No" -> REMOVE_QUEST_ITEM(QUEST_PICTOGRAPH)). It has to disappear
// on the other side too, or reloading would hand it back — there is only one picture.
extern "C" void FleetPicto_OnPhotoDiscarded(void) {
    if (!FleetPicto_IsCombo()) {
        return;
    }
    std::error_code ec;
    for (const char* name : { "picture.bin", "pictoflags.bin", "picture_rgba.bin" }) {
        std::string p = FleetPicto_Path(name);
        if (!p.empty()) {
            std::filesystem::remove(p, ec);
        }
    }
    SPDLOG_INFO("[FleetPicto] combo pictograph thrown away");
}

// Pull the shared picture into MM's own buffers. QUEST_PICTOGRAPH follows it: with a picture, the
// pictograph button shows it (MM's stored-photo branch); without one, it opens the lens.
extern "C" void FleetPicto_Import(void) {
    if (!FleetPicto_IsCombo()) {
        return;
    }
    std::string picture = FleetPicto_Path("picture.bin");
    std::string flags = FleetPicto_Path("pictoflags.bin");
    if (picture.empty()) {
        return;
    }

    bool gotPhoto = false;
    {
        std::ifstream f(picture, std::ios::binary);
        if (f) {
            f.read(reinterpret_cast<char*>(gSaveContext.pictoPhotoI5), PICTO_I5_SIZE);
            gotPhoto = (bool)f; // a full 11200-byte read succeeded
        }
    }

    // NO shared picture means the run HAS no picture — it does not mean "keep yours". There is one
    // pictograph in a combo, so the other game throwing it away has to throw it away here too;
    // otherwise MM keeps showing a print OoT already deleted, and the next OoT picture looks like it
    // "never arrived" because MM was still holding the old one. Absence travels like presence.
    if (!gotPhoto) {
        memset(gSaveContext.pictoPhotoI5, 0, PICTO_I5_SIZE);
        gSaveContext.save.saveInfo.pictoFlags0 = 0;
        gSaveContext.save.saveInfo.pictoFlags1 = 0;
        REMOVE_QUEST_ITEM(QUEST_PICTOGRAPH);
        SPDLOG_INFO("[FleetPicto] no shared pictograph — cleared this save's copy");
        return;
    }

    // All zeros is not a picture, it is a cleared file. Same conclusion as above.
    bool nonEmpty = false;
    for (int i = 0; i < PICTO_I5_SIZE; i++) {
        if (gSaveContext.pictoPhotoI5[i] != 0) {
            nonEmpty = true;
            break;
        }
    }
    if (!nonEmpty) {
        gSaveContext.save.saveInfo.pictoFlags0 = 0;
        gSaveContext.save.saveInfo.pictoFlags1 = 0;
        REMOVE_QUEST_ITEM(QUEST_PICTOGRAPH);
        return;
    }

    if (!flags.empty()) {
        std::ifstream f(flags, std::ios::binary);
        if (f) {
            uint32_t f0 = 0;
            uint32_t f1 = 0;
            f.read(reinterpret_cast<char*>(&f0), sizeof(f0));
            f.read(reinterpret_cast<char*>(&f1), sizeof(f1));
            if (f) {
                // Replace, never merge: these flags belong to THIS picture.
                gSaveContext.save.saveInfo.pictoFlags0 = f0;
                gSaveContext.save.saveInfo.pictoFlags1 = f1;
            }
        }
    }
    // The colour half. Optional by design: an older run, or a print taken before the colour file
    // existed, simply has none and MM shows its sepia — never a black frame.
    {
        std::string rgba = FleetPicto_Path("picture_rgba.bin");
        unsigned int size = ColorPictograph_GetBufferSize();
        if (!rgba.empty() && size > 0) {
            std::ifstream f(rgba, std::ios::binary);
            if (f) {
                std::vector<char> buf(size);
                f.read(buf.data(), size);
                if (f) {
                    ColorPictograph_SetBuffer(buf.data());
                }
            }
        }
    }

    SET_QUEST_ITEM(QUEST_PICTOGRAPH);
    SPDLOG_INFO("[FleetPicto] imported the combo pictograph (flags {:#x}/{:#x})",
                gSaveContext.save.saveInfo.pictoFlags0, gSaveContext.save.saveInfo.pictoFlags1);
}

void RegisterFleetPicto() {
    // Loading a combo file (including the forced load a cross-game warp does on arrival) adopts the
    // shared picture, so both games always open on the same one.
    COND_HOOK(OnSaveLoad, true, [](s16 fileNum) { FleetPicto_Import(); });
}

static RegisterShipInitFunc initFuncFleetPicto(RegisterFleetPicto, { "isFleetShipCombo.Enabled" });
