#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/Enhancements/FrameInterpolation/FrameInterpolation.h"
#include <spdlog/spdlog.h>

extern "C" {
#include "macros.h"
#include "variables.h"
#include "functions.h"
}

static HOOK_ID onGameStateUpdateHook = 0;
static HOOK_ID onGameStateDrawFinishHook = 0;
static bool creditsDisplayed = false;
static int scrollOffset = 0; // Current scroll position
static int scrollTimer = 0;  // Frame counter to control scrolling speed

static const u32 SEQUENCE[] = {
    BTN_DUP,
    BTN_DUP,
    BTN_DDOWN,
    BTN_DDOWN,
    BTN_DLEFT,
    BTN_DRIGHT,
    BTN_DLEFT,
    BTN_DRIGHT,
    // BTN_X, // We don't have an X button
    // BTN_Y, // We don't have a Y button
    BTN_B,
    BTN_A,
    BTN_DUP,
    BTN_DLEFT,
    BTN_DDOWN,
    BTN_DRIGHT,
    BTN_A,
    BTN_START,
};

static const u32 ALL_BUTTONS = BTN_DUP | BTN_DDOWN | BTN_DLEFT | BTN_DRIGHT | BTN_B | BTN_A | BTN_START;

// Un-used, original GC version credits, maybe we have a combobox to select between the two?
static const std::string gcCredits[] = {
    "CREDITS",
    "",
    "",
    "ENGINEERING",
    "",
    "Robert Champagne",
    "David Devaty",
    "Rory Johnston",
    "Stephen Lee",
    "YoonJoon Lee",
    "Max Szlagor",
};

// Currently just a list of contributors since the initial 2s2h commit. We could also include contributors from the
// decomp repository, but I'm not sure how many would want their name on the port, so erring on the side of caution for
// now git shortlog -sn 2332f63f5a9ca4b949f029c11a9744d601d7155f..HEAD
static const std::string shipCredits[] = {
    "",
    "Contributors",
    "",
    "Archez",
    "ProxySaw", // Changed manually
    "louist103",
    "inspectredc",
    "briaguya",
    "Patrick12115",
    "PurpleHato",
    "Malkierian",
    "aMannus",
    "Caladius",
    "balloondude2",
    "Eblo",
    "sitton76",
    "Revo",
    "mckinlee",
    "Adam Bird",
    "AltoXorg",
    "Captain Kitty Cat",
    "Hoeloe",
    "Jacob Erly",
    "Kenix3",
    "Louis",
    "Mothstery",
    "Nicholas Estelami",
    "Qlonever",
    "Ralphie Morell",
    "Reinhardt R. Gaming",
    "Rozelette",
    "justawayofthesamurai",
};

static const int totalLinesVisible = 15; // Number of lines to display on the screen
static const int scrollDelay = 20;       // Number of frames to wait before scrolling (adjust for speed)

// Function to print credits with scrolling
void PrintCredits(GfxPrint* printer, int startY) {
    int totalCredits = ARRAY_COUNT(shipCredits);

    for (int i = 0; i < totalLinesVisible; i++) {
        int index = (scrollOffset + i) % totalCredits; // Wrap around when reaching the end of the list
        int x = (41 - shipCredits[index].length()) / 2;

        GfxPrint_SetPos(printer, x, startY + i);
        GfxPrint_Printf(printer, "%s", shipCredits[index].c_str());
    }

    scrollTimer++;

    // Check if the scroll timer has reached the scroll delay threshold
    if (scrollTimer >= scrollDelay) {
        scrollOffset++;

        if (scrollOffset >= totalCredits) {
            scrollOffset = 0;
        }

        scrollTimer = 0;
    }
}

void OnGameStateDrawFinish() {
    GraphicsContext* gfxCtx = gGameState->gfxCtx;

    // this gets us close, but unfortunately the HUD and XLU buffers are still drawn on top of the rect
    func_8012CF0C(gfxCtx, true, true, 0, 0, 0);

    GfxPrint printer;

    OPEN_DISPS(gfxCtx);

    Gfx_SetupDL28_Opa(gfxCtx);

    GfxPrint_Init(&printer);
    GfxPrint_Open(&printer, POLY_OPA_DISP);

    GfxPrint_SetColor(&printer, 255, 255, 255, 255);
    PrintCredits(&printer, 7);

    POLY_OPA_DISP = GfxPrint_Close(&printer);
    GfxPrint_Destroy(&printer);

    CLOSE_DISPS(gfxCtx);
}

void OnGameStateUpdate() {
    // Currently incompatible with Debug Mode, because it uses the same button combination
    if (CVarGetInteger("gDeveloperTools.DebugEnabled", 0)) {
        return;
    }

    if (creditsDisplayed) {
        if (CHECK_BTN_ALL(gGameState->input[0].press.button, BTN_START)) {
            creditsDisplayed = false;
            GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnGameStateDrawFinish>(
                onGameStateDrawFinishHook);
            onGameStateDrawFinishHook = 0;
        }
        return;
    }

    // Check if the player is holding L + R + Z
    if (CHECK_BTN_ALL(gGameState->input[0].cur.button, BTN_L | BTN_R | BTN_Z)) {
        static u32 sequenceIndex = 0;
        // Check if the player pressed the correct button in the sequence
        if (CHECK_BTN_ALL(gGameState->input[0].press.button, SEQUENCE[sequenceIndex])) {
            sequenceIndex++;
            // If the player pressed the wrong button, reset the sequence
        } else if (CHECK_BTN_ALL(gGameState->input[0].press.button, (ALL_BUTTONS & ~SEQUENCE[sequenceIndex]))) {
            sequenceIndex = 0;
        }

        // If the player has pressed the correct button combination, display the credits
        if (sequenceIndex == ARRAY_COUNT(SEQUENCE)) {
            creditsDisplayed = true;
            scrollOffset = 0;
            scrollTimer = 0;
            onGameStateDrawFinishHook =
                GameInteractor::Instance->RegisterGameHook<GameInteractor::OnGameStateDrawFinish>(
                    OnGameStateDrawFinish);
        }
    }
}

// This back-ports a hidden credits screen that displays when the player holds L + R + Z and presses a specific button
// combination
void RegisterHiddenCredits() {
    GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnGameStateUpdate>(onGameStateUpdateHook);
    GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnGameStateDrawFinish>(onGameStateDrawFinishHook);
    onGameStateUpdateHook = 0;
    onGameStateDrawFinishHook = 0;
    creditsDisplayed = false;

    if (CVarGetInteger("gEnhancements.Restorations.HiddenCredits", 0)) {
        onGameStateUpdateHook =
            GameInteractor::Instance->RegisterGameHook<GameInteractor::OnGameStateUpdate>(OnGameStateUpdate);
    }
}
