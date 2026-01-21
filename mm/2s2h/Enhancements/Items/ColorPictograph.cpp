#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"
#include "2s2h/Enhancements/FrameInterpolation/FrameInterpolation.h"
#include <fstream>
#include <filesystem>
#include <cstring>

using json = nlohmann::json;

extern "C" {
#include "variables.h"
}

#define CVAR_NAME "gEnhancements.Items.ColorPictograph"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

static s16 fileNumber = 0;

// currently saving in root
std::string pictoFilePath = "picto.json";

// convert to msgpack or .png or some other format?
void SavePictoToFile(const nlohmann::json& image) {
    std::string filePath = Ship::Context::GetPathRelativeToAppDirectory(pictoFilePath);
    nlohmann::json picto = nlohmann::json::object();

    std::ifstream fileStream(filePath);
    if (fileStream.is_open()) {
        try {
            fileStream >> picto;
        } catch (nlohmann::json::exception& e) {
            throw std::runtime_error("Failed to parse picto file: " + std::string(e.what()));
        }
    }

    std::string fileKey = "file" + std::to_string(fileNumber);
    picto[fileKey]["image"] = image;

    std::ofstream out(filePath);
    if (!out) {
        throw std::runtime_error("Failed to write picto file");
    }
    out << picto.dump(4);
}

void LoadPictoFromFile(s16 fileNum) {
    std::string filePath = Ship::Context::GetPathRelativeToAppDirectory(pictoFilePath);
    std::ifstream fileStream(filePath);
    if (!fileStream.is_open()) {
        throw std::runtime_error("Failed to open picto file");
    }

    nlohmann::json picto;
    try {
        fileStream >> picto;
    } catch (nlohmann::json::exception& e) {
        throw std::runtime_error("Failed to parse picto file: " + std::string(e.what()));
    }

    std::string fileKey = "file" + std::to_string(fileNum);

    if (!picto.contains(fileKey)) {
        std::memset(gSaveContext.shipSaveContext.pictoPhotoRGBA, 0,
                    sizeof(gSaveContext.shipSaveContext.pictoPhotoRGBA));
        return;
    }

    if (!picto[fileKey].contains("image")) {
        std::memset(gSaveContext.shipSaveContext.pictoPhotoRGBA, 0,
                    sizeof(gSaveContext.shipSaveContext.pictoPhotoRGBA));
        return;
    }

    if (!picto[fileKey]["image"].is_array() ||
        picto[fileKey]["image"].size() < std::size(gSaveContext.shipSaveContext.pictoPhotoRGBA)) {
        std::memset(gSaveContext.shipSaveContext.pictoPhotoRGBA, 0,
                    sizeof(gSaveContext.shipSaveContext.pictoPhotoRGBA));
        return;
    }

    for (auto i = 0; i < std::size(gSaveContext.shipSaveContext.pictoPhotoRGBA); i++) {
        gSaveContext.shipSaveContext.pictoPhotoRGBA[i] = picto[fileKey]["image"].at(i).get<uint16_t>();
    }
}

void ConvertImage(u16* destI, u16* srcRgba16, s32 rgba16Width, s32 pixelLeft, s32 pixelTop, s32 pixelRight,
                  s32 pixelBottom) {

    // not sure if best way to do this
    // also not sure if/how endianness needs to be handled for cross-platform
    for (int i = pixelTop; i <= pixelBottom; i++) {
        for (int j = pixelLeft; j <= pixelRight; j++) {
            u16 px = srcRgba16[i * rgba16Width + j];
            px = (px >> 8) | (px << 8);
            destI[(i - pixelTop) * PICTO_PHOTO_WIDTH + (j - pixelLeft)] = px;
        }
    }

    nlohmann::json image = nlohmann::json::array();

    for (auto i = 0; i < std::size(gSaveContext.shipSaveContext.pictoPhotoRGBA); i++) {
        image.push_back(gSaveContext.shipSaveContext.pictoPhotoRGBA[i]);
    }

    SavePictoToFile(image);
}

void DrawPicto(s16 sp2CC) {

    OPEN_DISPS(gPlayState->state.gfxCtx);

    // Get rid of Sepia tone from prior gDPSetPrimColor call
    gDPSetCombineMode(OVERLAY_DISP++, G_CC_DECALRGBA, G_CC_DECALRGBA);

    // Calling invalidate twice because I couldn't cast as a u16*
    gSPInvalidateTexCache(OVERLAY_DISP++, (uintptr_t)(gSaveContext.shipSaveContext.pictoPhotoRGBA) + (0x500 * sp2CC));
    gSPInvalidateTexCache(OVERLAY_DISP++, (uintptr_t)(gSaveContext.shipSaveContext.pictoPhotoRGBA) + (0xA00 * sp2CC));
    gDPLoadTextureBlock(OVERLAY_DISP++, (u16*)(gSaveContext.shipSaveContext.pictoPhotoRGBA) + (0x500 * sp2CC),
                        G_IM_FMT_RGBA, G_IM_SIZ_16b, PICTO_PHOTO_WIDTH, 8, 0, G_TX_NOMIRROR | G_TX_WRAP,
                        G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

    CLOSE_DISPS(gPlayState->state.gfxCtx);
}

void RegisterColorPictograph() {
    if (!std::filesystem::exists(Ship::Context::GetPathRelativeToAppDirectory(pictoFilePath))) {
        json initFile;
        std::ofstream file(Ship::Context::GetPathRelativeToAppDirectory(pictoFilePath));
        file << initFile.dump(4);
        file.close();
    }

    COND_VB_SHOULD(VB_PICTO_TAKE, true /*maybe cvar?*/, {
        PreRender* prerender = va_arg(args, PreRender*);
        ConvertImage(gSaveContext.shipSaveContext.pictoPhotoRGBA, prerender->fbufSave, SCREEN_WIDTH,
                     PICTO_PHOTO_TOPLEFT_X, PICTO_PHOTO_TOPLEFT_Y, (PICTO_PHOTO_TOPLEFT_X + PICTO_PHOTO_WIDTH) - 1,
                     (PICTO_PHOTO_TOPLEFT_Y + PICTO_PHOTO_HEIGHT) - 1);
    });

    COND_VB_SHOULD(VB_PICTO_DISPLAY, CVAR, {
        s16 sp2CC = va_arg(args, s16);

        // might need something better to check for existance
        if (gSaveContext.shipSaveContext.pictoPhotoRGBA[0] != 0) {
            DrawPicto(sp2CC);
            *should = false;
        }
    });

    COND_HOOK(OnSaveLoad, true, [](s16 fileNum) {
        fileNumber = fileNum;
        LoadPictoFromFile(fileNum);
    });
}

static RegisterShipInitFunc initFunc(RegisterColorPictograph, { CVAR_NAME });
