#pragma once

#include "stdint.h"
#include "window/gui/GuiWindow.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <imgui.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <set>
#include "controller/controldevice/controller/Controller.h"

class BenInputEditorWindow : public Ship::GuiWindow {
  public:
    using Ship::GuiWindow::GuiWindow;
    ~BenInputEditorWindow();

    void DrawButton(const char* label, int32_t n64Btn, int32_t currentPort, int32_t* btnReading);

    void DrawInputChip(const char* buttonName, ImVec4 color);
    void DrawAnalogPreview(const char* label, ImVec2 stick, float deadzone = 0, bool gyro = false);
    void DrawControllerSchema();
    bool TestingRumble();
    void DrawFullContents();
    void DrawPortTabContents(uint8_t portIndex);

  protected:
    void InitElement() override;
    void DrawElement() override;
    void UpdateElement() override;

  private:
    void DrawStickDirectionLine(const char* axisDirectionName, uint8_t port, uint8_t stick, Ship::Direction direction,
                                ImVec4 color);
    void DrawButtonLine(const char* buttonName, uint8_t port, CONTROLLERBUTTONS_T bitmask, ImVec4 color);
    void DrawButtonLineEditMappingButton(uint8_t port, CONTROLLERBUTTONS_T bitmask, std::string id);
    void DrawButtonLineAddMappingButton(uint8_t port, CONTROLLERBUTTONS_T bitmask);

    void DrawStickDirectionLineEditMappingButton(uint8_t port, uint8_t stick, Ship::Direction direction,
                                                 std::string id);
    void DrawStickDirectionLineAddMappingButton(uint8_t port, uint8_t stick, Ship::Direction direction);
    void DrawStickSection(uint8_t port, uint8_t stick, int32_t id, ImVec4 color);

    void DrawRumbleSection(uint8_t port);
    void DrawRemoveRumbleMappingButton(uint8_t port, std::string id);
    void DrawAddRumbleMappingButton(uint8_t port);

    void DrawLEDSection(uint8_t port);
    void DrawRemoveLEDMappingButton(uint8_t port, std::string id);
    void DrawAddLEDMappingButton(uint8_t port);

    void DrawGyroSection(uint8_t port);
    void DrawRemoveGyroMappingButton(uint8_t port, std::string id);
    void DrawAddGyroMappingButton(uint8_t port);

    int32_t mGameInputBlockTimer;
    int32_t mMappingInputBlockTimer;
    int32_t mRumbleTimer;
    std::shared_ptr<Ship::ControllerRumbleMapping> mRumbleMappingToTest;

    // mBitmaskToMappingIds[port][bitmask] = { id0, id1, ... }
    std::unordered_map<uint8_t, std::unordered_map<CONTROLLERBUTTONS_T, std::vector<std::string>>> mBitmaskToMappingIds;

    // mStickDirectionToMappingIds[port][stick][direction] = { id0, id1, ... }
    std::unordered_map<uint8_t,
                       std::unordered_map<uint8_t, std::unordered_map<Ship::Direction, std::vector<std::string>>>>
        mStickDirectionToMappingIds;

    void UpdateBitmaskToMappingIds(uint8_t port);
    void UpdateStickDirectionToMappingIds(uint8_t port);

    void GetButtonColorsForShipDeviceIndex(Ship::ShipDeviceIndex lusIndex, ImVec4& buttonColor,
                                           ImVec4& buttonHoveredColor);
    void DrawPortTab(uint8_t portIndex);
    std::set<CONTROLLERBUTTONS_T> mButtonsBitmasks;
    std::set<CONTROLLERBUTTONS_T> mDpadBitmasks;
    void DrawButtonDeviceIcons(uint8_t portIndex, std::set<CONTROLLERBUTTONS_T> bitmasks);
    void DrawAnalogStickDeviceIcons(uint8_t portIndex, Ship::Stick stick);
    void DrawRumbleDeviceIcons(uint8_t portIndex);
    void DrawGyroDeviceIcons(uint8_t portIndex);
    void DrawLEDDeviceIcons(uint8_t portIndex);
    bool mInputEditorPopupOpen;
    void DrawSetDefaultsButton(uint8_t portIndex);
    void DrawClearAllButton(uint8_t portIndex);

    std::map<Ship::ShipDeviceIndex, bool> mDeviceIndexVisiblity;
    void DrawDeviceVisibilityButtons();
};
