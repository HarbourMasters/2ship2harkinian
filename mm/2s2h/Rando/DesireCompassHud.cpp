// =============================================================================
// DesireCompassHud — ImGui overlay for the Desire Compass.
//
// Two pieces:
//   1. The 8-spoke radial category wheel, shown while the player holds the
//      compass button (Skyward-Sword dowsing style). The spoke the stick is
//      pointing at is highlighted; each spoke also shows how many items of that
//      category are still outstanding in the seed.
//   2. The dowsing arrow: while dowsing, the located target is projected to
//      screen. On-screen targets get a diamond marker + distance readout;
//      off-screen ones get an arrow pinned to the screen edge pointing at them.
//
// Implemented as a Ship::GuiWindow so Draw() runs inside the Gui's ImGui frame
// (foreground-drawlist additions made outside it are discarded) — same
// constraint and structure as expansions/sm64/Sm64CapsHud.cpp. Registration is
// lazy off the OnInterfaceDrawStart hook so no C-side call site is needed.
//
// State lives in C (mods/items/logic/item_desire_sensor.c) and is read through
// the DesireCompass_* accessors.
// =============================================================================

#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <memory>
#include <string>

#include <ship/Context.h>
#include <ship/window/Window.h>
#include <ship/window/gui/Gui.h>
#include <ship/window/gui/GuiWindow.h>
#include <libultraship/libultraship.h>

#include "2s2h/Rando/Rando.h"
#include "2s2h/Rando/DesireCompass.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "z64.h"
#include "macros.h"
extern PlayState* gPlayState;

// World→view projection (z_actor.c). Screen NDC = projectedPos.xy * invW.
void Actor_GetProjectedPos(PlayState* play, Vec3f* worldPos, Vec3f* projectedPos, f32* invW);

// Compass state accessors (mods/items/logic/item_desire_sensor.c).
u8 DesireCompass_IsWheelOpen(void);
u8 DesireCompass_IsDowsing(void);
s32 DesireCompass_GetCategory(void);
u8 DesireCompass_HasTarget(void);
f32 DesireCompass_GetTargetDist(void);
void DesireCompass_GetTargetPos(Vec3f* out);
}

namespace {

// Wheel order must match DC_CategoryFromStick in item_desire_sensor.c:
// sector 0 = straight up, advancing clockwise.
const char* kCategoryNames[DCOMPASS_CAT_MAX] = {
    "Boss Souls", "Keys", "Other Souls", "Major Items", "Skills", "Junk", "Triforce", "Other",
};

constexpr float kPi = 3.14159265f;

ImU32 CategoryColor(int cat, bool selected) {
    // Muted when idle, bright gold when the stick is on that spoke.
    if (selected) {
        return IM_COL32(255, 215, 90, 255);
    }
    return IM_COL32(190, 190, 205, 170);
}

void DrawWheel(ImDrawList* dl, const ImVec2& disp) {
    const float cx = disp.x * 0.5f;
    const float cy = disp.y * 0.5f;
    const float radius = std::min(disp.x, disp.y) * 0.22f;
    const int selected = DesireCompass_GetCategory();

    // Dim backing disc so the spokes read over any scene.
    dl->AddCircleFilled(ImVec2(cx, cy), radius * 1.35f, IM_COL32(0, 0, 0, 110), 64);
    dl->AddCircle(ImVec2(cx, cy), radius * 1.35f, IM_COL32(255, 215, 90, 90), 64, 2.0f);

    for (int i = 0; i < DCOMPASS_CAT_MAX; i++) {
        // Sector 0 = up, clockwise. Screen Y grows downward, hence the -cos.
        const float angle = (float)i * (2.0f * kPi / (float)DCOMPASS_CAT_MAX);
        const float px = cx + std::sin(angle) * radius;
        const float py = cy - std::cos(angle) * radius;
        const bool isSel = (i == selected);
        const ImU32 col = CategoryColor(i, isSel);

        dl->AddCircleFilled(ImVec2(px, py), isSel ? 9.0f : 5.0f, col, 20);

        std::string label = kCategoryNames[i];
        const int remaining = Rando_DesireCompass_CountRemaining((DesireCompassCategory)i, DCOMPASS_SUBCAT_ANY);
        label += " (" + std::to_string(remaining) + ")";

        const ImVec2 ts = ImGui::CalcTextSize(label.c_str());
        // Push the label outward from the hub so it never overlaps the spoke.
        float lx = cx + std::sin(angle) * (radius + 26.0f) - ts.x * 0.5f;
        float ly = cy - std::cos(angle) * (radius + 26.0f) - ts.y * 0.5f;
        dl->AddText(ImVec2(lx + 1.0f, ly + 1.0f), IM_COL32(0, 0, 0, 200), label.c_str());
        dl->AddText(ImVec2(lx, ly), isSel ? IM_COL32(255, 240, 170, 255) : IM_COL32(220, 220, 230, 210),
                    label.c_str());
    }

    const char* hint = "Tilt to choose  -  release to dowse (costs 1 heart container)";
    const ImVec2 hs = ImGui::CalcTextSize(hint);
    dl->AddText(ImVec2(cx - hs.x * 0.5f + 1.0f, cy + radius * 1.35f + 15.0f), IM_COL32(0, 0, 0, 200), hint);
    dl->AddText(ImVec2(cx - hs.x * 0.5f, cy + radius * 1.35f + 14.0f), IM_COL32(235, 235, 245, 230), hint);
}

void DrawTargetMarker(ImDrawList* dl, const ImVec2& disp) {
    const int cat = DesireCompass_GetCategory();
    const char* catName = (cat >= 0 && cat < DCOMPASS_CAT_MAX) ? kCategoryNames[cat] : "?";

    if (!DesireCompass_HasTarget()) {
        // Dowsing but nothing of this category is loaded in the scene.
        std::string msg = std::string("Dowsing: ") + catName + " - nothing senses nearby";
        const ImVec2 ts = ImGui::CalcTextSize(msg.c_str());
        float x = disp.x * 0.5f - ts.x * 0.5f;
        float y = disp.y * 0.12f;
        dl->AddText(ImVec2(x + 1.0f, y + 1.0f), IM_COL32(0, 0, 0, 200), msg.c_str());
        dl->AddText(ImVec2(x, y), IM_COL32(220, 180, 180, 220), msg.c_str());
        return;
    }

    Vec3f world;
    DesireCompass_GetTargetPos(&world);
    world.y += 30.0f; // float the marker above the carrier actor

    Vec3f proj;
    f32 invW = 0.0f;
    Actor_GetProjectedPos(gPlayState, &world, &proj, &invW);

    // Same NDC→screen mapping as the SM64 caps HUD.
    const bool behind = (invW <= 0.0f);
    float sx = (proj.x * invW * 0.5f + 0.5f) * disp.x;
    float sy = (proj.y * invW * -0.5f + 0.5f) * disp.y;

    const float cx = disp.x * 0.5f;
    const float cy = disp.y * 0.5f;
    const ImU32 gold = IM_COL32(255, 215, 90, 235);

    const bool onScreen = !behind && sx >= 0.0f && sx <= disp.x && sy >= 0.0f && sy <= disp.y;

    if (onScreen) {
        // Diamond marker + distance.
        const float s = 11.0f;
        dl->AddQuadFilled(ImVec2(sx, sy - s), ImVec2(sx + s, sy), ImVec2(sx, sy + s), ImVec2(sx - s, sy), gold);
        dl->AddQuad(ImVec2(sx, sy - s), ImVec2(sx + s, sy), ImVec2(sx, sy + s), ImVec2(sx - s, sy),
                    IM_COL32(60, 40, 0, 220), 2.0f);

        char buf[64];
        snprintf(buf, sizeof(buf), "%s  %dm", catName, (int)(DesireCompass_GetTargetDist() / 10.0f));
        const ImVec2 ts = ImGui::CalcTextSize(buf);
        dl->AddText(ImVec2(sx - ts.x * 0.5f + 1.0f, sy + s + 5.0f), IM_COL32(0, 0, 0, 200), buf);
        dl->AddText(ImVec2(sx - ts.x * 0.5f, sy + s + 4.0f), IM_COL32(255, 240, 170, 240), buf);
        return;
    }

    // Off-screen (or behind): pin an arrow to the screen edge pointing at it.
    float dx = sx - cx;
    float dy = sy - cy;
    if (behind) { // projection mirrors when behind the camera
        dx = -dx;
        dy = -dy;
    }
    if (std::fabs(dx) < 0.001f && std::fabs(dy) < 0.001f) {
        dy = -1.0f;
    }
    const float halfW = disp.x * 0.42f;
    const float halfH = disp.y * 0.42f;
    const float scaleX = (std::fabs(dx) > 0.001f) ? halfW / std::fabs(dx) : 1e9f;
    const float scaleY = (std::fabs(dy) > 0.001f) ? halfH / std::fabs(dy) : 1e9f;
    const float scl = std::min(scaleX, scaleY);
    const float ex = cx + dx * scl;
    const float ey = cy + dy * scl;

    const float a = std::atan2(dy, dx);
    const float sz = 14.0f;
    const float tipX = ex + std::cos(a) * sz;
    const float tipY = ey + std::sin(a) * sz;
    const float perpX = std::sin(a) * sz * 0.65f;
    const float perpY = -std::cos(a) * sz * 0.65f;
    const float backX = -std::cos(a) * sz;
    const float backY = -std::sin(a) * sz;
    dl->AddTriangleFilled(ImVec2(tipX, tipY), ImVec2(tipX + backX + perpX, tipY + backY + perpY),
                          ImVec2(tipX + backX - perpX, tipY + backY - perpY), gold);

    char buf[64];
    snprintf(buf, sizeof(buf), "%s  %dm", catName, (int)(DesireCompass_GetTargetDist() / 10.0f));
    const ImVec2 ts = ImGui::CalcTextSize(buf);
    dl->AddText(ImVec2(ex - ts.x * 0.5f + 1.0f, ey + 19.0f), IM_COL32(0, 0, 0, 200), buf);
    dl->AddText(ImVec2(ex - ts.x * 0.5f, ey + 18.0f), IM_COL32(255, 240, 170, 235), buf);
}

class DesireCompassHudWindow final : public Ship::GuiWindow {
  public:
    using GuiWindow::GuiWindow;
    void InitElement() override {}
    void DrawElement() override {}
    void UpdateElement() override {}
    void Draw() override;
};

void DesireCompassHudWindow::Draw() {
    const bool wheel = DesireCompass_IsWheelOpen() != 0;
    const bool dowsing = DesireCompass_IsDowsing() != 0;
    if (!wheel && !dowsing) {
        return;
    }
    if (gPlayState == nullptr || gPlayState->pauseCtx.state != 0) {
        return;
    }
    auto gui = Ship::Context::GetInstance()->GetWindow()->GetGui();
    if (gui == nullptr || gui->GetMenuOrMenubarVisible()) {
        return;
    }
    if (ImGui::GetCurrentContext() == nullptr) {
        return;
    }
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    if (viewport == nullptr) {
        return;
    }
    ImDrawList* dl = ImGui::GetForegroundDrawList(viewport);
    if (dl == nullptr) {
        return;
    }
    const ImVec2 disp = ImGui::GetIO().DisplaySize;
    if (disp.x < 1.0f || disp.y < 1.0f) {
        return;
    }

    if (wheel) {
        DrawWheel(dl, disp);
    } else {
        DrawTargetMarker(dl, disp);
    }
}

std::shared_ptr<DesireCompassHudWindow> sHudWindow = nullptr;

// Lazily register the GuiWindow the first time the interface draws. Doing it
// here (rather than at ShipInit time) guarantees the Gui exists.
void EnsureRegistered() {
    if (sHudWindow != nullptr) {
        return;
    }
    auto ctx = Ship::Context::GetInstance();
    if (ctx == nullptr) {
        return;
    }
    auto window = ctx->GetWindow();
    if (window == nullptr) {
        return;
    }
    auto gui = window->GetGui();
    if (gui == nullptr) {
        return;
    }
    sHudWindow = std::make_shared<DesireCompassHudWindow>("gDesireCompassHud", "Desire Compass HUD");
    gui->AddGuiWindow(sHudWindow);
}

void RegisterDesireCompassHud() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnInterfaceDrawStart>([]() { EnsureRegistered(); });
}

static RegisterShipInitFunc initFunc(RegisterDesireCompassHud, {});

} // namespace
