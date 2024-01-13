#include "BenGui.hpp"

#include <spdlog/spdlog.h>
#include <ImGui/imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <ImGui/imgui_internal.h>
#include <libultraship/libultraship.h>
#include <Fast3D/gfx_pc.h>
#include "UIWidgets.hpp"

#ifdef __APPLE__
#include "graphic/Fast3D/gfx_metal.h"
#endif

#ifdef __SWITCH__
#include <port/switch/SwitchImpl.h>
#endif

#include "include/global.h"
#include "include/z64audio.h"
#include "libultraship/libultraship.h"

bool ShouldClearTextureCacheAtEndOfFrame = false;

namespace BenGui {
    // MARK: - Delegates

    std::shared_ptr<BenMenuBar> mBenMenuBar;

    std::shared_ptr<LUS::GuiWindow> mConsoleWindow;
    std::shared_ptr<LUS::GuiWindow> mStatsWindow;
    std::shared_ptr<LUS::GuiWindow> mInputEditorWindow;
    std::shared_ptr<LUS::GuiWindow> mGfxDebuggerWindow;

    std::shared_ptr<SaveEditorWindow> mSaveEditorWindow;

    void SetupGuiElements() {
        auto gui = LUS::Context::GetInstance()->GetWindow()->GetGui();

        auto& style = ImGui::GetStyle();
        style.FramePadding = ImVec2(4.0f, 6.0f);
        style.ItemSpacing = ImVec2(8.0f, 6.0f);
        style.Colors[ImGuiCol_MenuBarBg] = UIWidgets::Colors::DarkGray;

        mBenMenuBar = std::make_shared<BenMenuBar>("gOpenMenuBar", CVarGetInteger("gOpenMenuBar", 0));
        gui->SetMenuBar(std::reinterpret_pointer_cast<LUS::GuiMenuBar>(mBenMenuBar));

        if (gui->GetMenuBar() && !gui->GetMenuBar()->IsVisible()) {
#if defined(__SWITCH__) || defined(__WIIU__)
            gui->GetGameOverlay()->TextDrawNotification(30.0f, true, "Press - to access enhancements menu");
#else
            gui->GetGameOverlay()->TextDrawNotification(30.0f, true, "Press F1 to access enhancements menu");
#endif
        }

        mStatsWindow = gui->GetGuiWindow("Stats");
        if (mStatsWindow == nullptr) {
            SPDLOG_ERROR("Could not find stats window");
        }

        mConsoleWindow = gui->GetGuiWindow("Console");
        if (mConsoleWindow == nullptr) {
            SPDLOG_ERROR("Could not find console window");
        }

        mInputEditorWindow = gui->GetGuiWindow("Input Editor");
        if (mInputEditorWindow == nullptr) {
            SPDLOG_ERROR("Could not find input editor window");
        }

        mGfxDebuggerWindow = gui->GetGuiWindow("GfxDebuggerWindow");
        if (mGfxDebuggerWindow == nullptr) {
            SPDLOG_ERROR("Could not find input GfxDebuggerWindow");
        }

        mSaveEditorWindow = std::make_shared<SaveEditorWindow>("gSaveEditorEnabled", "Save Editor");
        gui->AddGuiWindow(mSaveEditorWindow);
    }

    void Destroy() {
        mSaveEditorWindow = nullptr;
        mStatsWindow = nullptr;
        mConsoleWindow = nullptr;
        mBenMenuBar = nullptr;
    }
}
