#include "PlayerFDFlipbooks.h"
#include "z64player.h"
#include "2s2h/BenPort.h"

extern "C" {
extern TexturePtr sPlayerEyesTextures[PLAYER_FORM_MAX][PLAYER_EYES_MAX];
extern TexturePtr sPlayerMouthTextures[PLAYER_FORM_MAX][PLAYER_MOUTH_MAX];
uint8_t ResourceMgr_FileExists(const char* resName);
}

static const char* sFDEyesTextures[PLAYER_EYES_MAX] = {
    "__OTR__objects/object_link_boy/gLinkFierceDeityEyesOpenTex",
    "__OTR__objects/object_link_boy/gLinkFierceDeityEyesHalfTex",
    "__OTR__objects/object_link_boy/gLinkFierceDeityEyesClosedTex",
    "__OTR__objects/object_link_boy/gLinkFierceDeityEyesRightTex",
    "__OTR__objects/object_link_boy/gLinkFierceDeityEyesLeftTex",
    "__OTR__objects/object_link_boy/gLinkFierceDeityEyesUpTex",
    "__OTR__objects/object_link_boy/gLinkFierceDeityEyesDownTex",
    "__OTR__objects/object_link_boy/gLinkFierceDeityEyesWincingTex",
};

static const char* sFDMouthTextures[PLAYER_MOUTH_MAX] = {
    "__OTR__objects/object_link_boy/gLinkFierceDeityMouthClosedTex",
    "__OTR__objects/object_link_boy/gLinkFierceDeityMouthHalfTex",
    "__OTR__objects/object_link_boy/gLinkFierceDeityMouthOpenTex",
    "__OTR__objects/object_link_boy/gLinkFierceDeityMouthSmileTex",
};

static s32 sFDFacePatchState = 0;

static void PlayerFDFlipbooks_PatchOnce(void) {
    if (sFDFacePatchState != 0) {
        return;
    }

    for (s32 i = 0; i < PLAYER_EYES_MAX; i++) {
        if (!ResourceMgr_FileExists(sFDEyesTextures[i])) {
            sFDFacePatchState = 1;
            return;
        }
    }

    for (s32 i = 0; i < PLAYER_MOUTH_MAX; i++) {
        if (!ResourceMgr_FileExists(sFDMouthTextures[i])) {
            sFDFacePatchState = 1;
            return;
        }
    }

    for (s32 i = 0; i < PLAYER_EYES_MAX; i++) {
        sPlayerEyesTextures[PLAYER_FORM_FIERCE_DEITY][i] = (TexturePtr)sFDEyesTextures[i];
    }

    for (s32 i = 0; i < PLAYER_MOUTH_MAX; i++) {
        sPlayerMouthTextures[PLAYER_FORM_FIERCE_DEITY][i] = (TexturePtr)sFDMouthTextures[i];
    }

    sFDFacePatchState = 1;
}

void PlayerFDFlipbooks_Patch(void) {
    PlayerFDFlipbooks_PatchOnce();
}
