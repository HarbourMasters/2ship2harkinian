#pragma once

#include <libultraship/libultraship.h>

#ifdef __cplusplus

#include "UIWidgets.hpp"
#include <unordered_map>

class HudEditorWindow : public LUS::GuiWindow {
    public:
        using GuiWindow::GuiWindow;

        void InitElement() override {};
        void DrawElement() override;
        void UpdateElement() override {};
};

extern "C" {
#endif

#include "z64.h"

typedef enum {
    HUD_EDITOR_ELEMENT_NONE = -1,
    HUD_EDITOR_ELEMENT_B,
    HUD_EDITOR_ELEMENT_C_LEFT,
    HUD_EDITOR_ELEMENT_C_DOWN,
    HUD_EDITOR_ELEMENT_C_RIGHT,
    HUD_EDITOR_ELEMENT_A,
    HUD_EDITOR_ELEMENT_C_UP,
    HUD_EDITOR_ELEMENT_START,
    HUD_EDITOR_ELEMENT_MAGIC_METER,
    HUD_EDITOR_ELEMENT_HEARTS,
    HUD_EDITOR_ELEMENT_RUPEE_COUNTER,
    HUD_EDITOR_ELEMENT_KEY_COUNTER,
    HUD_EDITOR_ELEMENT_SKULLTULA_COUNTER,
    HUD_EDITOR_ELEMENT_D_PAD,
    HUD_EDITOR_ELEMENT_MAX,
} HudEditorElementID;

typedef enum {
    HUD_EDITOR_ELEMENT_MODE_VANILLA,
    HUD_EDITOR_ELEMENT_MODE_HIDDEN,
    HUD_EDITOR_ELEMENT_MODE_MOVABLE_43,
    HUD_EDITOR_ELEMENT_MODE_MOVABLE_LEFT,
    HUD_EDITOR_ELEMENT_MODE_MOVABLE_RIGHT,
} HudEditorElementMode;

void HudEditor_SetActiveElement(HudEditorElementID id);
bool HudEditor_ShouldOverrideDraw();
void HudEditor_ModifyDrawValues(s16* rectLeft, s16* rectTop, s16* rectWidth, s16* rectHeight, s16* dsdx, s16* dtdy);

typedef struct {
    HudEditorElementID id;
    const char* name;
    int32_t defaultX;
    int32_t defaultY;
    int32_t defaultR;
    int32_t defaultG;
    int32_t defaultB;
    int32_t defaultA;
    const char* xCvar;
    const char* yCvar;
    const char* scaleCvar;
    const char* colorCvar;
    const char* modeCvar;
} HudEditorElement;

#define HUD_EDITOR_ELEMENT(id, name, cvar, defaultX, defaultY, defaultR, defaultG, defaultB, defaultA) \
    { \
        id, name, defaultX, defaultY, defaultR, defaultG, defaultB, defaultA, \
        "gHudEditor." cvar ".Position.X", "gHudEditor." cvar ".Position.Y", \
        "gHudEditor." cvar ".Scale", \
        "gHudEditor." cvar ".Color.Value", \
        "gHudEditor." cvar ".Mode" \
    }

extern HudEditorElementID hudEditorActiveElement;

extern HudEditorElement hudEditorElements[HUD_EDITOR_ELEMENT_MAX];

#ifdef __cplusplus
}
#endif
