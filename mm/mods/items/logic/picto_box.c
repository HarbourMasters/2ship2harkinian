/**
 * picto_box.c - Pictograph Box image pipeline + capture (Skijer's NEI).
 *
 * Ports MM's photo conversion VERBATIM (mm/src/code/z_play.c):
 *   - Play_ConvertRgba16ToIntensityImage  -> Picto_ConvertRgba16ToIntensityImage
 *   - Play_CompressI8ToI5                 -> Picto_CompressI8ToI5
 * and feeds them real pixels via SOH's FB_WriteFramebufferSliceToCPU (the engine's
 * "used by picto box" 320x240 RGBA16 readback). The readback is DEFERRED (gDPReadFB runs when the
 * frame's GBI list is processed), so capture is a small state machine: emit during DRAW frame N,
 * read the CPU buffer in UPDATE frame N+1.
 *
 * Validation (flags) runs the instant the shutter is pressed (actor positions are accurate then),
 * independent of the image. Everything is written in MM's exact save layout (Nei_Save()->pictoFlags
 * + pictoPhotoI5) for a 2Ship bridge; OoT itself gives no reward. #included into custom_items.c.
 */
#include "snap.h"
#include "../../nei_save.h"
#include "../helpers/camera_helper.h" // FirstPerson_* for the aim mode

extern void FB_WriteFramebufferSliceToCPU(Gfx** gfxp, void* buffer, u8 byteSwap);
extern void* MmAssets_LoadResource(const char* path); // MM viewfinder textures from mm.o2r
extern void Picto_SyncWrite(void);                    // mirror the kept photo to the OoT<->MM shared sidecar

// MM viewfinder textures (parameter_static): corner border (IA4 16x16), crosshair (I4 32x16),
// "PICTBOX" label (I4 32x8). Loaded once; NULL falls back to a code-drawn bracket viewfinder.
static void* sVfBorder = NULL;
static void* sVfIcon = NULL;
static void* sVfText = NULL;
static u8 sVfTried = 0;
static void Picto_LoadViewfinderTextures(void) {
    if (sVfTried) {
        return;
    }
    sVfTried = 1;
    sVfBorder = MmAssets_LoadResource("__OTR__parameter_static/gPictoBoxFocusBorderTex");
    sVfIcon = MmAssets_LoadResource("__OTR__parameter_static/gPictoBoxFocusIconTex");
    sVfText = MmAssets_LoadResource("__OTR__parameter_static/gPictoBoxFocusTextTex");
}

// --- MM intensity/color macros (verbatim from MM z_play.c / color.h) ---
#define PLAY_INTENSITY_RED 2
#define PLAY_INTENSITY_GREEN 4
#define PLAY_INTENSITY_BLUE 1
#define PLAY_INTENSITY_NORM (0x1F * PLAY_INTENSITY_RED + 0x1F * PLAY_INTENSITY_GREEN + 0x1F * PLAY_INTENSITY_BLUE)
#define PLAY_INTENSITY_MIX(r, g, b, m) \
    ((((r) * PLAY_INTENSITY_RED + (g) * PLAY_INTENSITY_GREEN + (b) * PLAY_INTENSITY_BLUE) * (m)) / PLAY_INTENSITY_NORM)
#define PICTO_RGBA16_GET_R(pixel) (((pixel) >> 11) & 0x1F)
#define PICTO_RGBA16_GET_G(pixel) (((pixel) >> 6) & 0x1F)
#define PICTO_RGBA16_GET_B(pixel) (((pixel) >> 1) & 0x1F)

#define PLAY_COMPRESS_BITS 5
#define PLAY_DECOMPRESS_BITS 8

// Verbatim MM Play_ConvertRgba16ToIntensityImage (only the bit depths the picto box uses).
static void Picto_ConvertRgba16ToIntensityImage(void* destI, u16* srcRgba16, s32 rgba16Width, s32 pixelLeft,
                                                s32 pixelTop, s32 pixelRight, s32 pixelBottom, s32 bitDepth) {
    s32 i;
    s32 j;
    u32 pixel;
    u32 r;
    u32 g;
    u32 b;

    if (bitDepth == 8) {
        u8* destI8 = destI;
        for (i = pixelTop; i <= pixelBottom; i++) {
            for (j = pixelLeft; j <= pixelRight; j++) {
                pixel = srcRgba16[i * rgba16Width + j];
                r = PICTO_RGBA16_GET_R(pixel);
                g = PICTO_RGBA16_GET_G(pixel);
                b = PICTO_RGBA16_GET_B(pixel);
                *(destI8++) = PLAY_INTENSITY_MIX(r, g, b, 255);
            }
        }
    } else if (bitDepth == 16) {
        // ColorPictograph (BenUI): keep the raw RGBA16 instead of intensity.
        u16* destI16 = destI;
        for (i = pixelTop; i <= pixelBottom; i++) {
            for (j = pixelLeft; j <= pixelRight; j++) {
                *(destI16++) = srcRgba16[i * rgba16Width + j];
            }
        }
    }
}

// Verbatim MM Play_CompressI8ToI5: packs five 5-bit pixels into eight bits.
static void Picto_CompressI8ToI5(void* srcI8, void* destI5, size_t size) {
    u32 i;
    u8* src = srcI8;
    s8* dest = destI5;
    s32 bitsLeft = PLAY_DECOMPRESS_BITS;
    u32 destPixel = 0;
    s32 shift;
    u32 srcPixel;

    for (i = 0; i < size; i++) {
        srcPixel = *src++;
        srcPixel = (srcPixel * 0x1F + 0x80) / 0xFF;
        shift = bitsLeft - PLAY_COMPRESS_BITS;
        if (shift > 0) {
            destPixel |= srcPixel << shift;
        } else {
            destPixel |= srcPixel >> -shift;
            *dest++ = destPixel;
            shift += PLAY_DECOMPRESS_BITS;
            destPixel = srcPixel << shift;
        }
        bitsLeft = shift;
    }

    if (bitsLeft < PLAY_DECOMPRESS_BITS) {
        *dest = destPixel;
    }
}

// --- Pictograph state (MM PICTO_BOX_STATE: LENS aiming -> shutter -> SETUP capture -> PHOTO + keep) ---
// Deferred framebuffer readback (gDPReadFB runs when the GBI list is processed): emit during DRAW
// frame N, read the CPU buffer in UPDATE frame N+1.
typedef enum { PICTO_CAP_IDLE, PICTO_CAP_EMIT, PICTO_CAP_PROCESS } PictoCaptureState;
static PictoCaptureState sPictoCapState = PICTO_CAP_IDLE;

static u8 sPictoAimActive = 0;   // first-person viewfinder (MM PICTO_BOX_STATE_LENS)
static u8 sPictoPhotoActive = 0; // captured photo + keep/discard prompt shown (MM PICTO_BOX_STATE_PHOTO)
static u8 sPictoOnLens = 0;      // "pictobox mode" on the Lens slot (kaleido wheel)
static u8 sPictoJustEntered = 0; // the press that opened the viewfinder must not also fire the shutter

// 320x240 RGBA16 readback; the 160x112 I8 scratch (grayscale save) and the contiguous RGBA16 color
// copy (the on-screen ColorPictograph display).
static u16 sPictoFrameRgba16[SCREEN_WIDTH * SCREEN_HEIGHT];
static u8 sPictoI8[PICTO_PHOTO_SIZE];
static u16 sPictoColorTex[PICTO_PHOTO_WIDTH * PICTO_PHOTO_HEIGHT];

// pictoFlags before the shutter — restored if the photo is discarded. (MM freezes actors via
// haltAllActors and records at keep-time; OoT has no halt, so we record at the shutter, when actor
// positions still match the framed view, and revert on discard.)
static u32 sPictoPrevFlags0 = 0;
static u32 sPictoPrevFlags1 = 0;

// Shutter (MM: NA_SE_SY_CAMERA_SHUTTER + halt + SETUP_PHOTO). Validate subjects now (accurate actor
// positions), remember the previous flags for a possible discard, and queue the framebuffer readback.
static void Picto_Shutter(PlayState* play) {
    extern s32 MmSfx_IsAvailable(void);
    extern s32 MmSfx_PlayAtPos(u16 sfxId, Vec3f * pos);
    sPictoPrevFlags0 = Nei_Save()->pictoFlags0;
    sPictoPrevFlags1 = Nei_Save()->pictoFlags1;
    Snap_RecordPictographedActors(play); // writes pictoFlags0/1
    Nei_Save()->pictoboxOwned = 1;
    sPictoCapState = PICTO_CAP_EMIT;
    // MM camera-shutter sfx (NA_SE_SY_CAMERA_SHUTTER = 0x4850 in mm_sources/audio/sfx/systembank_table.h)
    // — an MM-only sfx, played through the MM audio engine (not in the OOT sfx banks). No-ops if mm.o2r
    // isn't loaded.
    if (MmSfx_IsAvailable()) {
        MmSfx_PlayAtPos(0x4850, &GET_PLAYER(play)->actor.world.pos);
    }
}

// Fire the shutter from outside the viewfinder (menu/debug). The capture state machine raises the
// photo display once the readback completes.
void Picto_TakePhoto(PlayState* play) {
    Picto_Shutter(play);
}

// DRAW hook: emit the framebuffer readback into the GBI list when a capture is queued (native-endian
// for the I8 convert, matching 2Ship's picto capture).
void Picto_EmitCapture(PlayState* play, Gfx** gfxp) {
    if (sPictoCapState == PICTO_CAP_EMIT) {
        FB_WriteFramebufferSliceToCPU(gfxp, sPictoFrameRgba16, 0);
        sPictoCapState = PICTO_CAP_PROCESS;
    }
}

// Build the contiguous RGBA16 color copy of the captured region (the on-screen photo). The readback is
// native-endian; F3D wants RGBA16 textures big-endian, so byte-swap each pixel.
static void Picto_BuildColor(void) {
    s32 y;
    s32 x;
    for (y = 0; y < PICTO_PHOTO_HEIGHT; y++) {
        for (x = 0; x < PICTO_PHOTO_WIDTH; x++) {
            u16 px = sPictoFrameRgba16[(PICTO_PHOTO_TOPLEFT_Y + y) * SCREEN_WIDTH + (PICTO_PHOTO_TOPLEFT_X + x)];
            sPictoColorTex[y * PICTO_PHOTO_WIDTH + x] = (u16)((px >> 8) | (px << 8));
        }
    }
}

static void Picto_UpdateState(PlayState* play); // defined below

// UPDATE hook: drive the input state machine; one frame after the shutter emit, convert the readback
// to I8 (grayscale, for the save) + color (display) and raise the photo + prompt. The compressed I5 is
// committed only when the player KEEPS the photo (Picto_UpdateState), like MM.
void Picto_Update(PlayState* play) {
    Picto_UpdateState(play);

    if (sPictoCapState == PICTO_CAP_PROCESS) {
        Picto_ConvertRgba16ToIntensityImage(sPictoI8, sPictoFrameRgba16, SCREEN_WIDTH, PICTO_PHOTO_TOPLEFT_X,
                                            PICTO_PHOTO_TOPLEFT_Y, (PICTO_PHOTO_TOPLEFT_X + PICTO_PHOTO_WIDTH) - 1,
                                            (PICTO_PHOTO_TOPLEFT_Y + PICTO_PHOTO_HEIGHT) - 1, 8);
        Picto_BuildColor();
        sPictoCapState = PICTO_CAP_IDLE;
        // Capture done: leave first-person so the photo + textbox show as an overlay over the normal
        // view (MM-style — MM never holds the lens view during PICTO_BOX_STATE_PHOTO), and the player
        // returns to a clean state so the next photo can be taken.
        Player* player = GET_PLAYER(play);
        FirstPerson_Exit(player, play);
        sPictoAimActive = 0;
        sPictoPhotoActive = 1;
        // Freeze Link AND halt the enemies while the keep/discard textbox is up (MM halts all actors).
        // PLAYER_STATE1_IN_CUTSCENE both freezes the player and halts ENEMY/MISC via z_actor.c's
        // D_80116068 mask — so the game stops behind the textbox instead of running.
        player->stateFlags1 |= PLAYER_STATE1_IN_CUTSCENE;
        Message_StartTextbox(play, PICTO_KEEP_TEXTID, NULL); // MM 0xF8 "Keep this picture?" (keep/discard)
    }
}

// DRAW hook (OVERLAY): show the captured photo for a few seconds, scaled 2x and centered, with a
// 1px black frame. No-op when no preview is armed.
void Picto_DrawPhoto(PlayState* play, Gfx** gfxp) {
    Gfx* g = *gfxp;

    // Aim mode: MM viewfinder. Authentic textures from mm.o2r (parameter_static) when available —
    // 4 mirrored corner borders + center crosshair + "PICTBOX" label — exactly like MM's
    // Interface_Draw PICTO_BOX_STATE_LENS block. Falls back to code-drawn brackets if absent.
    if (sPictoAimActive && !sPictoPhotoActive) {
        s32 lx = PICTO_PHOTO_TOPLEFT_X;
        s32 ty = PICTO_PHOTO_TOPLEFT_Y;
        s32 rx = lx + PICTO_PHOTO_WIDTH;
        s32 by = ty + PICTO_PHOTO_HEIGHT;
        s32 cx = SCREEN_WIDTH / 2;
        s32 cy = SCREEN_HEIGHT / 2;

        Picto_LoadViewfinderTextures();

        if (sVfBorder != NULL && sVfIcon != NULL && sVfText != NULL) {
            gDPPipeSync(g++);
            gDPSetCycleType(g++, G_CYC_1CYCLE);
            gDPSetAlphaCompare(g++, G_AC_THRESHOLD);
            gDPSetRenderMode(g++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
            gDPSetCombineMode(g++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
            gDPSetTextureFilter(g++, G_TF_BILERP);
            gDPSetPrimColor(g++, 0, 0, 255, 255, 155, 255);

            // 4 corner borders (IA4 16x16, mirrored: s/t = 512 = one 16px mirror period)
            gDPLoadTextureBlock_4b(g++, sVfBorder, G_IM_FMT_IA, 16, 16, 0, G_TX_MIRROR | G_TX_WRAP,
                                   G_TX_MIRROR | G_TX_WRAP, 4, 4, G_TX_NOLOD, G_TX_NOLOD);
            gSPTextureRectangle(g++, lx << 2, ty << 2, (lx + 16) << 2, (ty + 16) << 2, G_TX_RENDERTILE, 0, 0, 1 << 10,
                                1 << 10);
            gSPTextureRectangle(g++, (rx - 16) << 2, ty << 2, rx << 2, (ty + 16) << 2, G_TX_RENDERTILE, 512, 0,
                                1 << 10, 1 << 10);
            gSPTextureRectangle(g++, lx << 2, (by - 16) << 2, (lx + 16) << 2, by << 2, G_TX_RENDERTILE, 0, 512,
                                1 << 10, 1 << 10);
            gSPTextureRectangle(g++, (rx - 16) << 2, (by - 16) << 2, rx << 2, by << 2, G_TX_RENDERTILE, 512, 512,
                                1 << 10, 1 << 10);

            // Center crosshair (I4 32x16)
            gDPLoadTextureBlock_4b(g++, sVfIcon, G_IM_FMT_I, 32, 16, 0, G_TX_NOMIRROR | G_TX_WRAP,
                                   G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            gSPTextureRectangle(g++, (cx - 16) << 2, (cy - 8) << 2, (cx + 16) << 2, (cy + 8) << 2, G_TX_RENDERTILE, 0,
                                0, 1 << 10, 1 << 10);

            // "PICTBOX" label (I4 32x8), just below the frame
            gDPLoadTextureBlock_4b(g++, sVfText, G_IM_FMT_I, 32, 8, 0, G_TX_NOMIRROR | G_TX_WRAP,
                                   G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            gSPTextureRectangle(g++, (cx - 16) << 2, (by + 4) << 2, (cx + 16) << 2, (by + 12) << 2, G_TX_RENDERTILE, 0,
                                0, 1 << 10, 1 << 10);
            gDPPipeSync(g++);
        } else {
            // Fallback: code-drawn corner brackets + crosshair.
            s32 b = 16;
            gDPPipeSync(g++);
            gDPSetCycleType(g++, G_CYC_FILL);
            gDPSetRenderMode(g++, G_RM_NOOP, G_RM_NOOP2);
            gDPSetFillColor(g++, 0xFFFFFFFF);
            gDPFillRectangle(g++, lx, ty, lx + b, ty + 2);
            gDPFillRectangle(g++, lx, ty, lx + 2, ty + b);
            gDPFillRectangle(g++, rx - b, ty, rx, ty + 2);
            gDPFillRectangle(g++, rx - 2, ty, rx, ty + b);
            gDPFillRectangle(g++, lx, by - 2, lx + b, by);
            gDPFillRectangle(g++, lx, by - b, lx + 2, by);
            gDPFillRectangle(g++, rx - b, by - 2, rx, by);
            gDPFillRectangle(g++, rx - 2, by - b, rx, by);
            gDPFillRectangle(g++, cx - 5, cy, cx + 6, cy + 1);
            gDPFillRectangle(g++, cx, cy - 5, cx + 1, cy + 6);
            gDPPipeSync(g++);
        }
    }

    // Captured photo display — MM PICTO_BOX_STATE_PHOTO (z_parameter.c:6718-6743), ported 1:1: a gray
    // border panel, then the photo at its native 160x112 size offset UP 33px to leave room for the
    // prompt box along the bottom. NOT a full-screen modal. Photo drawn in 8-row strips (color RGBA16
    // via ColorPictograph; MM uses sepia I8). A = keep, B = discard (handled in Picto_UpdateState).
    if (sPictoPhotoActive) {
        s32 top;
        s32 left;
        s32 sy;

        // Gray border/background panel (MM: prim 200,200,200,250 XLU, gDPFillRectangle(70,22,251,151)).
        gDPPipeSync(g++);
        gDPSetCycleType(g++, G_CYC_1CYCLE);
        gDPSetRenderMode(g++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        gDPSetCombineMode(g++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
        gDPSetPrimColor(g++, 0, 0, 200, 200, 200, 250);
        gDPFillRectangle(g++, 70, 22, 251, 151);

        // The photo at native size, offset up 33px (MM "to give room for the message box at the bottom").
        gDPPipeSync(g++);
        gDPSetRenderMode(g++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
        gDPSetCombineMode(g++, G_CC_DECALRGBA, G_CC_DECALRGBA);
        gDPSetTextureFilter(g++, G_TF_POINT);
        top = PICTO_PHOTO_TOPLEFT_Y - 33; // 31
        for (sy = 0; sy < PICTO_PHOTO_HEIGHT; sy += 8, top += 8) {
            left = PICTO_PHOTO_TOPLEFT_X; // 80
            // sPictoColorTex lives at a fixed address, so Fast3D's texture cache would keep showing the
            // PREVIOUS photo even after a new capture overwrites the buffer. Invalidate each strip so the
            // new image is re-uploaded (same fix 2Ship's ColorPictograph uses). Skijer's NEI
            gSPInvalidateTexCache(g++, &sPictoColorTex[sy * PICTO_PHOTO_WIDTH]);
            gDPLoadTextureBlock(g++, &sPictoColorTex[sy * PICTO_PHOTO_WIDTH], G_IM_FMT_RGBA, G_IM_SIZ_16b,
                                PICTO_PHOTO_WIDTH, 8, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK,
                                G_TX_NOLOD, G_TX_NOLOD);
            gSPTextureRectangle(g++, left << 2, top << 2, (left + PICTO_PHOTO_WIDTH) << 2, (top + 8) << 2,
                                G_TX_RENDERTILE, 0, 0, 1 << 10, 1 << 10);
        }

        // The "Keep this picture?" textbox itself is drawn by the message system (Message_StartTextbox
        // PICTO_KEEP_TEXTID) at the bottom — the photo is offset up 33px to leave room for it, MM-style.
        gDPPipeSync(g++);
    }

    *gfxp = g;
}

// --- First-person viewfinder + photo state machine (MM PICTO_BOX_STATE) ---
u8 Picto_IsAiming(void) {
    return sPictoAimActive;
}

// Enter the viewfinder (MM PICTO_BOX_STATE_LENS). Shared by the D-pad standalone fallback and the
// Lens-slot equipped C-button (Player_UseItem in z_player.c, when pictobox mode is selected). Guards
// against unsafe player states. Skijer's NEI
void Picto_EnterAimMode(PlayState* play) {
    Player* player = GET_PLAYER(play);
    if (sPictoAimActive || !Nei_Save()->pictoboxOwned) {
        return;
    }
    if (player->meleeWeaponState != 0 ||
        (player->stateFlags1 & (PLAYER_STATE1_IN_WATER | PLAYER_STATE1_ON_HORSE | PLAYER_STATE1_IN_CUTSCENE |
                                PLAYER_STATE1_DEAD | PLAYER_STATE1_TALKING))) {
        return;
    }
    FirstPerson_Init(player, play);
    sPictoAimActive = 1;
    sPictoJustEntered = 1; // first press = aim only; the shutter waits for the next press
}

static void Picto_UpdateState(PlayState* play) {
    Player* player = GET_PLAYER(play);
    Input* input = &play->state.input[0];

    if (!Nei_Save()->pictoboxOwned) {
        if (sPictoAimActive) {
            FirstPerson_Exit(player, play);
        }
        sPictoAimActive = 0;
        sPictoPhotoActive = 0;
        return;
    }

    // PHOTO: the captured photo + the "Keep this picture?" 2-choice textbox (MM PICTO_BOX_STATE_PHOTO,
    // z_parameter.c:2382-2408). Freeze the player; let the message system drive the choice and read
    // choiceIndex like MM (0 = keep, !=0 = discard).
    if (sPictoPhotoActive) {
        player->linearVelocity = 0.0f;
        player->stateFlags1 |= PLAYER_STATE1_IN_CUTSCENE; // keep Link + the enemies frozen while the textbox is up
        // B discards directly (the "Throw it away" shortcut); otherwise the 2-choice menu drives it
        // (A confirms the selected option, read via choiceIndex like MM).
        u8 bDiscard = CHECK_BTN_ALL(input->press.button, BTN_B);
        if (bDiscard || (Message_GetState(&play->msgCtx) == TEXT_STATE_CHOICE && Message_ShouldAdvance(play))) {
            Message_CloseTextbox(play);
            player->stateFlags1 &= ~PLAYER_STATE1_IN_CUTSCENE; // release the freeze (keep -> OFF / discard -> re-aim)
            if (bDiscard || play->msgCtx.choiceIndex != 0) {
                // "Throw it away": revert the flags this shutter set and re-enter the viewfinder (MM
                // returns to PICTO_BOX_STATE_LENS).
                Nei_Save()->pictoFlags0 = sPictoPrevFlags0;
                Nei_Save()->pictoFlags1 = sPictoPrevFlags1;
                sPictoPhotoActive = 0;
                FirstPerson_Init(player, play);
                sPictoAimActive = 1;
                sPictoJustEntered = 1; // don't let the choice-confirm A press also fire the shutter
                Audio_PlaySoundGeneral(NA_SE_SY_CANCEL, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                                       &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
            } else {
                // "Keep it": commit the compressed I5 to the save (MM compresses at keep-time); the
                // flags recorded at the shutter stay. First-person was already exited when the photo
                // came up, so just close out (MM -> PICTO_BOX_STATE_OFF).
                Picto_CompressI8ToI5(sPictoI8, Nei_Save()->pictoPhotoI5, PICTO_PHOTO_SIZE);
                Nei_Save()->pictoHasPhoto = 1; // gates the "Replace?" warn on the next capture
                Picto_SyncWrite();             // mirror to the OoT<->MM shared sidecar for this save slot
                sPictoPhotoActive = 0;
                Audio_PlaySoundGeneral(NA_SE_SY_DECIDE, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                                       &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
            }
        }
        return;
    }

    // OFF: the pictobox is entered ONLY via its equipped C-button (the Lens slot in pictobox mode,
    // handled in Player_UseItem). No D-pad fallback — it spuriously opened the viewfinder when a D-pad
    // button was used for another item (e.g. the Iron Knuckle's Axe / hammer on D-Up).
    if (!sPictoAimActive) {
        return;
    }

    // LENS: aiming. Keep the player put, run the first-person camera.
    player->linearVelocity = 0.0f;
    FirstPerson_Update(player, play);

    if (sPictoJustEntered) {
        // Consume the press that opened the viewfinder (it ran via Player_UseItem earlier this frame,
        // so its button is still in press.button here). First press = aim, second press = capture, like
        // the bow/rods — and this also lets first-person fully engage, so the photo is the aimed view
        // instead of the 3rd-person frame.
        sPictoJustEntered = 0;
        return;
    }

    if (sPictoCapState != PICTO_CAP_IDLE) {
        return; // mid-capture; ignore input until the readback completes
    }

    // Shutter = ONLY the pictobox's equipped C-button (the one it's assigned to) — you snap with the
    // same button you equipped, like the bow/rods. B cancels the viewfinder. (Not A.)
    // Capture IMMEDIATELY, in first-person (the aimed view). If a photo already exists, the keep/discard
    // prompt warns about replacing it (Picto_BuildKeepMessage) — we must NOT pop a pre-capture textbox,
    // because that drops the player out of first-person and the shot would come out in 3rd person.
    {
        u16 shutterBtn = ItemInput_GetEquippedButton(ITEM_LENS, play);
        if (shutterBtn != 0 && CHECK_BTN_ALL(input->press.button, shutterBtn)) {
            Picto_Shutter(play);
        } else if (CHECK_BTN_ALL(input->press.button, BTN_B) ||
                   (player->stateFlags1 & (PLAYER_STATE1_DEAD | PLAYER_STATE1_IN_CUTSCENE | PLAYER_STATE1_DAMAGED))) {
            FirstPerson_Exit(player, play);
            sPictoAimActive = 0;
        }
    }
}

// --- "Pictobox mode" on the Lens of Truth slot (toggled by the kaleido wheel) ---
// When active, the Lens slot represents the Pictograph Box; the in-game trigger lets the player aim.
// (sPictoOnLens is declared up top so the aim trigger sees it.)
u8 Picto_IsOnLensActive(void) {
    // Without the real Lens of Truth, the shared slot is ALWAYS the pictobox — you can't be in "Lens
    // mode" for an item you don't own (so the equipped C-button routes to the camera, not the lens).
    // With the Lens owned, the kaleido wheel's sPictoOnLens decides.
    if (Nei_Save()->pictoboxOwned && gSaveContext.save.saveInfo.inventory.items[SLOT_LENS] != ITEM_LENS) {
        return 1;
    }
    return sPictoOnLens;
}

void Picto_SetOnLensActive(u8 on) {
    sPictoOnLens = on ? 1 : 0;
}

// --- Item ownership + debug shutter (menu/save-editor accessors) ---
u8 Picto_IsOwned(void) {
    return Nei_Save()->pictoboxOwned;
}

void Picto_SetOwned(u8 on) {
    Nei_Save()->pictoboxOwned = on ? 1 : 0;
}

// Fire the shutter from the menu without the kaleido wheel (for testing capture + validation).
void Picto_TakePhotoNow(void) {
    if (gPlayState != NULL) {
        Picto_TakePhoto(gPlayState);
    }
}
