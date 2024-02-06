//
//  UIWidgets.cpp
//  soh
//
//  Created by David Chavez on 25.08.22.
//

#include "UIWidgets.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <ImGui/imgui_internal.h>
#include <libultraship/libultraship.h>
#include <string>
#include <unordered_map>

#include <libultraship/libultra/types.h>
//#include "soh/Enhancements/cosmetics/CosmeticsEditor.h"

namespace UIWidgets {

    // MARK: - Layout Helper

    // Automatically adds newlines to break up text longer than a specified number of characters
    // Manually included newlines will still be respected and reset the line length
    // If line is midword when it hits the limit, text should break at the last encountered space
    char* WrappedText(const char* text, unsigned int charactersPerLine) {
        std::string newText(text);
        const size_t tipLength = newText.length();
        int lastSpace = -1;
        int currentLineLength = 0;
        for (unsigned int currentCharacter = 0; currentCharacter < tipLength; currentCharacter++) {
            if (newText[currentCharacter] == '\n') {
                currentLineLength = 0;
                lastSpace = -1;
                continue;
            } else if (newText[currentCharacter] == ' ') {
                lastSpace = currentCharacter;
            }

            if ((currentLineLength >= charactersPerLine) && (lastSpace >= 0)) {
                newText[lastSpace] = '\n';
                currentLineLength = currentCharacter - lastSpace - 1;
                lastSpace = -1;
            }
            currentLineLength++;
        }

        return strdup(newText.c_str());
    }

    char* WrappedText(const std::string& text, unsigned int charactersPerLine) {
        return WrappedText(text.c_str(), charactersPerLine);
    }

    void SetLastItemHoverText(const std::string& text) {
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("%s", WrappedText(text, 60));
            ImGui::EndTooltip();
        }
    }

    void SetLastItemHoverText(const char* text) {
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("%s", WrappedText(text, 60));
            ImGui::EndTooltip();
        }
    }

    // Adds a "?" next to the previous ImGui item with a custom tooltip
    void InsertHelpHoverText(const std::string& text) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "?");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("%s", WrappedText(text, 60));
            ImGui::EndTooltip();
        }
    }

    void InsertHelpHoverText(const char* text) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "?");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("%s", WrappedText(text, 60));
            ImGui::EndTooltip();
        }
    }


    // MARK: - UI Elements

    void Tooltip(const char* text) {
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", WrappedText(text));
        }
    }
  
    void DrawFlagArray32(const std::string& name, uint32_t& flags) {
        ImGui::PushID(name.c_str());
        for (int32_t flagIndex = 0; flagIndex < 32; flagIndex++) {
            if ((flagIndex % 8) != 0) {
                ImGui::SameLine();
            }
            ImGui::PushID(flagIndex);
            uint32_t bitMask = 1 << flagIndex;
            bool flag = (flags & bitMask) != 0;
            std::string label = std::to_string(flagIndex);
            if (UIWidgets::Checkbox(label.c_str(), &flag, {.tooltip = label.c_str(), 
            .labelPosition = LabelPosition::None })) {
                if (flag) {
                    flags |= bitMask;
                } else {
                    flags &= ~bitMask;
                }
            }
            ImGui::PopID();
        }
        ImGui::PopID();
    }

    void DrawFlagArray16(const std::string& name, uint16_t& flags) {
        ImGui::PushID(name.c_str());
        for (int16_t flagIndex = 0; flagIndex < 16; flagIndex++) {
            if ((flagIndex % 8) != 0) {
                ImGui::SameLine();
            }
            ImGui::PushID(flagIndex);
            uint16_t bitMask = 1 << flagIndex;
            bool flag = (flags & bitMask) != 0;
            std::string label = std::to_string(flagIndex);
            if (UIWidgets::Checkbox(label.c_str(), &flag, {.tooltip = label.c_str(), 
            .labelPosition = LabelPosition::None })) {
                if (flag) {
                    flags |= bitMask;
                } else {
                    flags &= ~bitMask;
                }
            }
            ImGui::PopID();
        }
        ImGui::PopID();
    }

    void DrawFlagArray8(const std::string& name, uint8_t& flags) {
        ImGui::PushID(name.c_str());
        for (int8_t flagIndex = 0; flagIndex < 8; flagIndex++) {
            if ((flagIndex % 8) != 0) {
                ImGui::SameLine();
            }
            ImGui::PushID(flagIndex);
            uint8_t bitMask = 1 << flagIndex;
            bool flag = (flags & bitMask) != 0;
            std::string label = std::to_string(flagIndex);
            if (UIWidgets::Checkbox(label.c_str(), &flag, {.tooltip = label.c_str(), 
            .labelPosition = LabelPosition::None })) {
                if (flag) {
                    flags |= bitMask;
                } else {
                    flags &= ~bitMask;
                }
            }
            ImGui::PopID();
        }
        ImGui::PopID();
    }

    void Spacer(float height) {
        ImGui::Dummy(ImVec2(0.0f, height));
    }

    void PaddedSeparator(bool padTop, bool padBottom, float extraVerticalTopPadding, float extraVerticalBottomPadding) {
        if (padTop) {
            Spacer(extraVerticalTopPadding);
        }
        ImGui::Separator();
        if (padBottom) {
            Spacer(extraVerticalBottomPadding);
        }
    }

    void RenderCross(ImDrawList* draw_list, ImVec2 pos, ImU32 col, float sz) {
        float thickness = ImMax(sz / 5.0f, 1.0f);
        sz -= thickness * 0.5f;
        pos += ImVec2(thickness * 0.25f, thickness * 0.25f);

        draw_list->PathLineTo(ImVec2(pos.x, pos.y));
        draw_list->PathLineTo(ImVec2(pos.x + sz, pos.y + sz));
        draw_list->PathStroke(col, 0, thickness);

        draw_list->PathLineTo(ImVec2(pos.x + sz, pos.y));
        draw_list->PathLineTo(ImVec2(pos.x, pos.y + sz));
        draw_list->PathStroke(col, 0, thickness);
    }

    bool CustomCheckbox(const char* label, bool* v, bool disabled, CheckboxGraphics disabledGraphic) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) {
            return false;
        }

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);
        const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

        const float square_sz = ImGui::GetFrameHeight();
        const ImVec2 pos = window->DC.CursorPos;
        const ImRect total_bb(pos, pos + ImVec2(square_sz + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), label_size.y + style.FramePadding.y * 2.0f));
        ImGui::ItemSize(total_bb, style.FramePadding.y);
        if (!ImGui::ItemAdd(total_bb, id)) {
            IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
            return false;
        }

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
        if (pressed) {
            *v = !(*v);
            ImGui::MarkItemEdited(id);
        }

        const ImRect check_bb(pos, pos + ImVec2(square_sz, square_sz));
        ImGui::RenderNavHighlight(total_bb, id);
        ImGui::RenderFrame(check_bb.Min, check_bb.Max, ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), true, style.FrameRounding);
        ImU32 check_col = ImGui::GetColorU32(ImGuiCol_CheckMark);
        ImU32 cross_col = ImGui::GetColorU32(ImVec4(0.50f, 0.50f, 0.50f, 1.00f));
        bool mixed_value = (g.LastItemData.InFlags & ImGuiItemFlags_MixedValue) != 0;
        if (mixed_value) {
            // Undocumented tristate/mixed/indeterminate checkbox (#2644)
            // This may seem awkwardly designed because the aim is to make ImGuiItemFlags_MixedValue supported by all widgets (not just checkbox)
            ImVec2 pad(ImMax(1.0f, IM_FLOOR(square_sz / 3.6f)), ImMax(1.0f, IM_FLOOR(square_sz / 3.6f)));
            window->DrawList->AddRectFilled(check_bb.Min + pad, check_bb.Max - pad, check_col, style.FrameRounding);
        } else if ((!disabled && *v) || (disabled && disabledGraphic == CheckboxGraphics::Checkmark)) {
            const float pad = ImMax(1.0f, IM_FLOOR(square_sz / 6.0f));
            ImGui::RenderCheckMark(window->DrawList, check_bb.Min + ImVec2(pad, pad), check_col, square_sz - pad * 2.0f);
        } else if (disabled && disabledGraphic == CheckboxGraphics::Cross) {
            const float pad = ImMax(1.0f, IM_FLOOR(square_sz / 6.0f));
            RenderCross(window->DrawList, check_bb.Min + ImVec2(pad, pad), cross_col, square_sz - pad * 2.0f);
        }

        ImVec2 label_pos = ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x, check_bb.Min.y + style.FramePadding.y);
        if (g.LogEnabled) {
            ImGui::LogRenderedText(&label_pos, mixed_value ? "[~]" : *v ? "[x]" : "[ ]");
        }
        if (label_size.x > 0.0f) {
            ImGui::RenderText(label_pos, label);
        }

        IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
        return pressed;
    }

    void ReEnableComponent(const char* disabledTooltipText) {
        // End of disable region of previous component
        ImGui::PopStyleVar(1);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && strcmp(disabledTooltipText, "") != 0) {
            ImGui::SetTooltip("%s", disabledTooltipText);
        }
        ImGui::PopItemFlag();
    }

    void DisableComponent(const float alpha) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
    }

    bool EnhancementCheckbox(const char* text, const char* cvarName, bool disabled, const char* disabledTooltipText, CheckboxGraphics disabledGraphic, bool defaultValue) {
        bool changed = false;
        if (disabled) {
            DisableComponent(ImGui::GetStyle().Alpha * 0.5f);
        }

        bool val = (bool)CVarGetInteger(cvarName, defaultValue);
        if (CustomCheckbox(text, &val, disabled, disabledGraphic)) {
            CVarSetInteger(cvarName, val);
            LUS::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
            changed = true;
        }

        if (disabled) {
            ReEnableComponent(disabledTooltipText);
        }
        return changed;
    }

    bool PaddedEnhancementCheckbox(const char* text, const char* cvarName, bool padTop, bool padBottom, bool disabled, const char* disabledTooltipText, CheckboxGraphics disabledGraphic, bool defaultValue) {
        ImGui::BeginGroup();
        if (padTop) Spacer(0);

        bool changed = EnhancementCheckbox(text, cvarName, disabled, disabledTooltipText, disabledGraphic, defaultValue);

        if (padBottom) Spacer(0);
        ImGui::EndGroup();
        return changed;
    }

    bool EnhancementCombobox(const char* cvarName, std::span<const char*, std::dynamic_extent> comboArray, uint8_t defaultIndex, bool disabled, const char* disabledTooltipText, uint8_t disabledValue) {
        bool changed = false;
        if (defaultIndex <= 0) {
            defaultIndex = 0;
        }

        if (disabled) {
            DisableComponent(ImGui::GetStyle().Alpha * 0.5f);
        }

        uint8_t selected = CVarGetInteger(cvarName, defaultIndex);
        std::string comboName = std::string("##") + std::string(cvarName);
        if (ImGui::BeginCombo(comboName.c_str(), comboArray[selected])) {
            for (uint8_t i = 0; i < comboArray.size(); i++) {
                if (strlen(comboArray[i]) > 0) {
                    if (ImGui::Selectable(comboArray[i], i == selected)) {
                        CVarSetInteger(cvarName, i);
                        selected = i;
                        changed = true;
                        LUS::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
                    }
                }
            }
            ImGui::EndCombo();
        }

        if (disabled) {
            ReEnableComponent(disabledTooltipText);

            if (disabledValue >= 0 && selected != disabledValue) {
                CVarSetInteger(cvarName, disabledValue);
                changed = true;
                LUS::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
            }
        }

        return changed;
    }

    bool LabeledRightAlignedEnhancementCombobox(const char* label, const char* cvarName, std::span<const char*, std::dynamic_extent> comboArray, uint8_t defaultIndex, bool disabled, const char* disabledTooltipText, uint8_t disabledValue) {
        ImGui::Text("%s", label);
        s32 currentValue = CVarGetInteger(cvarName, defaultIndex);

#ifdef __WIIU__
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - (ImGui::CalcTextSize(comboArray[currentValue]).x + 40.0f));
        ImGui::PushItemWidth(ImGui::CalcTextSize(comboArray[currentValue]).x + 60.0f);
#else
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - (ImGui::CalcTextSize(comboArray[currentValue]).x + 20.0f));
        ImGui::PushItemWidth(ImGui::CalcTextSize(comboArray[currentValue]).x + 30.0f);
#endif

        bool changed = EnhancementCombobox(cvarName, comboArray, defaultIndex, disabled, disabledTooltipText, disabledValue);

        ImGui::PopItemWidth();
        return changed;
    }

    void PaddedText(const char* text, bool padTop, bool padBottom) {
        if (padTop) Spacer(0);

        ImGui::Text("%s", text);

        if (padBottom) Spacer(0);
    }

    bool EnhancementSliderInt(const char* text, const char* id, const char* cvarName, int min, int max, const char* format, int defaultValue, bool PlusMinusButton, bool disabled, const char* disabledTooltipText) {
        bool changed = false;
        int val = CVarGetInteger(cvarName, defaultValue);

        if (disabled) {
            DisableComponent(ImGui::GetStyle().Alpha * 0.5f);
        }

        ImGui::Text(text, val);
        Spacer(0);

        ImGui::BeginGroup();
        if (PlusMinusButton) {
            std::string MinusBTNName = " - ##" + std::string(cvarName);
            if (ImGui::Button(MinusBTNName.c_str())) {
                val--;
                changed = true;
            }
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 7.0f);
        }

        ImGui::PushItemWidth(std::min((ImGui::GetContentRegionAvail().x - (PlusMinusButton ? sliderButtonWidth : 0.0f)), maxSliderWidth));
        if (ImGui::SliderInt(id, &val, min, max, format, ImGuiSliderFlags_AlwaysClamp))
        {
            changed = true;
        }
        ImGui::PopItemWidth();
        
        if (PlusMinusButton) {
            std::string PlusBTNName = " + ##" + std::string(cvarName);
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 7.0f);
            if (ImGui::Button(PlusBTNName.c_str())) {
                val++;
                changed = true;
            }
        }
        ImGui::EndGroup();

        if (disabled) {
            ReEnableComponent(disabledTooltipText);
        }

        if (val < min) {
            val = min;
            changed = true;
        }

        if (val > max) {
            val = max;
            changed = true;
        }

        if (changed) {
            CVarSetInteger(cvarName, val);
            LUS::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
        }

        return changed;
    }

    bool EnhancementSliderFloat(const char* text, const char* id, const char* cvarName, float min, float max, const char* format, float defaultValue, bool isPercentage, bool PlusMinusButton, bool disabled, const char* disabledTooltipText) {
        bool changed = false;
        float val = CVarGetFloat(cvarName, defaultValue);

        if (disabled) {
            DisableComponent(ImGui::GetStyle().Alpha * 0.5f);
        }

        if (!isPercentage) {
            ImGui::Text(text, val);
        } else {
            ImGui::Text(text, static_cast<int>(100 * val));
        }
        Spacer(0);

        ImGui::BeginGroup();
        if (PlusMinusButton) {
            std::string MinusBTNName = " - ##" + std::string(cvarName);
            if (ImGui::Button(MinusBTNName.c_str())) {
                if (isPercentage) {
                    val -= 0.01f;
                } else {
                    val -= 0.1f;
                }
                changed = true;
            }
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 7.0f);
        }

        ImGui::PushItemWidth(std::min((ImGui::GetContentRegionAvail().x - (PlusMinusButton ? sliderButtonWidth : 0.0f)), maxSliderWidth));
        if (ImGui::SliderFloat(id, &val, min, max, format, ImGuiSliderFlags_AlwaysClamp)) {
            if (isPercentage) {
                val = roundf(val * 100) / 100;
            }
            changed = true;
        }
        ImGui::PopItemWidth();
        
        if (PlusMinusButton) {
            std::string PlusBTNName = " + ##" + std::string(cvarName);
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 7.0f);
            if (ImGui::Button(PlusBTNName.c_str())) {
                if (isPercentage) {
                    val += 0.01f;
                } else {
                    val += 0.1f;
                }
                changed = true;
            }
        }
        ImGui::EndGroup();

        if (disabled) {
            ReEnableComponent(disabledTooltipText);
        }

        if (val < min) {
            val = min;
            changed = true;
        }

        if (val > max) {
            val = max;
            changed = true;
        }

        if (changed) {
            CVarSetFloat(cvarName, val);
            LUS::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
        }

        return changed;
    }

    bool PaddedEnhancementSliderInt(const char* text, const char* id, const char* cvarName, int min, int max, const char* format, int defaultValue, bool PlusMinusButton, bool padTop, bool padBottom, bool disabled, const char* disabledTooltipText) {
        bool changed = false;
        ImGui::BeginGroup();
        if (padTop) Spacer(0);

        changed = EnhancementSliderInt(text, id, cvarName, min, max, format, defaultValue, PlusMinusButton, disabled, disabledTooltipText);

        if (padBottom) Spacer(0);
        ImGui::EndGroup();
        return changed;
    }

    bool PaddedEnhancementSliderFloat(const char* text, const char* id, const char* cvarName, float min, float max, const char* format, float defaultValue, bool isPercentage, bool PlusMinusButton, bool padTop, bool padBottom, bool disabled, const char* disabledTooltipText) {
        bool changed = false;
        ImGui::BeginGroup();
        if (padTop) Spacer(0);

        changed = EnhancementSliderFloat(text, id, cvarName, min, max, format, defaultValue, isPercentage, PlusMinusButton, disabled, disabledTooltipText);

        if (padBottom) Spacer(0);
        ImGui::EndGroup();
        return changed;
    }

    bool EnhancementRadioButton(const char* text, const char* cvarName, int id) {
        /*Usage :
        EnhancementRadioButton("My Visible Name","gMyCVarName", MyID);
        First arg is the visible name of the Radio button
        Second is the cvar name where MyID will be saved.
        Note: the CVar name should be the same to each Buddies.
        Example :
            EnhancementRadioButton("English", "gLanguages", LANGUAGE_ENG);
            EnhancementRadioButton("German", "gLanguages", LANGUAGE_GER);
            EnhancementRadioButton("French", "gLanguages", LANGUAGE_FRA);
        */
        std::string make_invisible = "##" + std::string(text) + std::string(cvarName);

        bool ret = false;
        int val = CVarGetInteger(cvarName, 0);
        if (ImGui::RadioButton(make_invisible.c_str(), id == val)) {
            CVarSetInteger(cvarName, id);
            LUS::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
            ret = true;
        }
        ImGui::SameLine();
        ImGui::Text("%s", text);
        
        return ret;
    }

    bool DrawResetColorButton(const char* cvarName, ImVec4* colors, ImVec4 defaultcolors, bool has_alpha) {
        bool changed = false;
        std::string Cvar_RBM = std::string(cvarName) + "RBM";
        std::string MakeInvisible = "Reset##" + std::string(cvarName) + "Reset";
        if (ImGui::Button(MakeInvisible.c_str())) {
            colors->x = defaultcolors.x;
            colors->y = defaultcolors.y;
            colors->z = defaultcolors.z;
            colors->w = has_alpha ? defaultcolors.w : 255.0f;

            Color_RGBA8 colorsRGBA;
            colorsRGBA.r = defaultcolors.x;
            colorsRGBA.g = defaultcolors.y;
            colorsRGBA.b = defaultcolors.z;
            colorsRGBA.a = has_alpha ? defaultcolors.w : 255.0f;

            CVarSetColor(cvarName, colorsRGBA);
            CVarSetInteger(Cvar_RBM.c_str(), 0); //On click disable rainbow mode.
            LUS::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
            changed = true;
        }
        Tooltip("Revert colors to the game's original colors (GameCube version)\nOverwrites previously chosen color");
        return changed;
    }

    bool DrawRandomizeColorButton(const char* cvarName, ImVec4* colors) {
        return false;
    }

    void DrawLockColorCheckbox(const char* cvarName) {
        std::string Cvar_Lock = std::string(cvarName) + "Lock";
        s32 lock = CVarGetInteger(Cvar_Lock.c_str(), 0);
        std::string FullName = "Lock##" + Cvar_Lock;
        EnhancementCheckbox(FullName.c_str(), Cvar_Lock.c_str());
        Tooltip("Prevents this color from being changed upon selecting \"Randomize all\"");
    }

    void RainbowColor(const char* cvarName, ImVec4* colors) {
        std::string Cvar_RBM = std::string(cvarName) + "RBM";
        std::string MakeInvisible = "Rainbow##" + std::string(cvarName) + "Rainbow";

        EnhancementCheckbox(MakeInvisible.c_str(), Cvar_RBM.c_str());
        Tooltip("Cycles through colors on a timer\nOverwrites previously chosen color");
    }

    void LoadPickersColors(ImVec4& ColorArray, const char* cvarname, const ImVec4& default_colors, bool has_alpha) {
        Color_RGBA8 defaultColors;
        defaultColors.r = default_colors.x;
        defaultColors.g = default_colors.y;
        defaultColors.b = default_colors.z;
        defaultColors.a = default_colors.w;

        Color_RGBA8 cvarColor = CVarGetColor(cvarname, defaultColors);

        ColorArray.x = cvarColor.r / 255.0;
        ColorArray.y = cvarColor.g / 255.0;
        ColorArray.z = cvarColor.b / 255.0;
        ColorArray.w = cvarColor.a / 255.0;
    }

    bool EnhancementColor(const char* text, const char* cvarName, ImVec4 ColorRGBA, ImVec4 default_colors, bool allow_rainbow, bool has_alpha, bool TitleSameLine) {
        bool changed = false;
        LoadPickersColors(ColorRGBA, cvarName, default_colors, has_alpha);

        ImGuiColorEditFlags flags = ImGuiColorEditFlags_None;

        if (!TitleSameLine) {
            ImGui::Text("%s", text);
            flags = ImGuiColorEditFlags_NoLabel;
        }

        ImGui::PushID(cvarName);

        if (!has_alpha) {
            if (ImGui::ColorEdit3(text, (float*)&ColorRGBA, flags))
            {
                Color_RGBA8 colors;
                colors.r = ColorRGBA.x * 255.0;
                colors.g = ColorRGBA.y * 255.0;
                colors.b = ColorRGBA.z * 255.0;
                colors.a = 255.0;

                CVarSetColor(cvarName, colors);
                LUS::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
                changed = true;
            }
        }
        else
        {
            flags |= ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview;
            if (ImGui::ColorEdit4(text, (float*)&ColorRGBA, flags))
            {
                Color_RGBA8 colors;
                colors.r = ColorRGBA.x * 255.0;
                colors.g = ColorRGBA.y * 255.0;
                colors.b = ColorRGBA.z * 255.0;
                colors.a = ColorRGBA.w * 255.0;

                CVarSetColor(cvarName, colors);
                LUS::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
                changed = true;
            }
        }

        ImGui::PopID();

        //ImGui::SameLine(); // Removing that one to gain some width spacing on the HUD editor
        ImGui::PushItemWidth(-FLT_MIN);
        if (DrawResetColorButton(cvarName, &ColorRGBA, default_colors, has_alpha)) {
            changed = true;
        }
        ImGui::SameLine();
        if (DrawRandomizeColorButton(cvarName, &ColorRGBA)) {
            changed = true;
        }
        if (allow_rainbow) {
            if (ImGui::GetContentRegionAvail().x > 185) {
                ImGui::SameLine();
            }
            RainbowColor(cvarName, &ColorRGBA);
        }
        DrawLockColorCheckbox(cvarName);
        ImGui::NewLine();
        ImGui::PopItemWidth();

        return changed;
    }

    void DrawFlagArray32(const std::string& name, uint32_t& flags) {
        ImGui::PushID(name.c_str());
        for (int32_t flagIndex = 0; flagIndex < 32; flagIndex++) {
            if ((flagIndex % 8) != 0) {
                ImGui::SameLine();
            }
            ImGui::PushID(flagIndex);
            uint32_t bitMask = 1 << flagIndex;
            bool flag = (flags & bitMask) != 0;
            std::string label = std::to_string(flagIndex);
            if (UIWidgets::Checkbox(label.c_str(), &flag, {.tooltip = label.c_str(), 
            .labelPosition = LabelPosition::None })) {
                if (flag) {
                    flags |= bitMask;
                } else {
                    flags &= ~bitMask;
                }
            }
            ImGui::PopID();
        }
        ImGui::PopID();
    }

    void DrawFlagArray16(const std::string& name, uint16_t& flags) {
        ImGui::PushID(name.c_str());
        for (int16_t flagIndex = 0; flagIndex < 16; flagIndex++) {
            if ((flagIndex % 8) != 0) {
                ImGui::SameLine();
            }
            ImGui::PushID(flagIndex);
            uint16_t bitMask = 1 << flagIndex;
            bool flag = (flags & bitMask) != 0;
            std::string label = std::to_string(flagIndex);
            if (UIWidgets::Checkbox(label.c_str(), &flag, {.tooltip = label.c_str(), 
            .labelPosition = LabelPosition::None })) {
                if (flag) {
                    flags |= bitMask;
                } else {
                    flags &= ~bitMask;
                }
            }
            ImGui::PopID();
        }
        ImGui::PopID();
    }

    void DrawFlagArray8(const std::string& name, uint8_t& flags) {
        ImGui::PushID(name.c_str());
        for (int8_t flagIndex = 0; flagIndex < 8; flagIndex++) {
            if ((flagIndex % 8) != 0) {
                ImGui::SameLine();
            }
            ImGui::PushID(flagIndex);
            uint8_t bitMask = 1 << flagIndex;
            bool flag = (flags & bitMask) != 0;
            std::string label = std::to_string(flagIndex);
            if (UIWidgets::Checkbox(label.c_str(), &flag, {.tooltip = label.c_str(), 
            .labelPosition = LabelPosition::None })) {
                if (flag) {
                    flags |= bitMask;
                } else {
                    flags &= ~bitMask;
                }
            }
            ImGui::PopID();
        }
        ImGui::PopID();
    }

    bool StateButtonEx(const char* str_id, const char* label, ImVec2 size, ImGuiButtonFlags flags) {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        const ImGuiStyle& style = g.Style;
        const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

        const ImGuiID id = window->GetID(str_id);
        const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
        const float default_size = ImGui::GetFrameHeight();
        ImGui::ItemSize(size, (size.y >= default_size) ? g.Style.FramePadding.y : -1.0f);
        if (!ImGui::ItemAdd(bb, id))
            return false;

        if (g.LastItemData.InFlags & ImGuiItemFlags_ButtonRepeat)
            flags |= ImGuiButtonFlags_Repeat;

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

        // Render
        const ImU32 bg_col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_ButtonActive
                                         : hovered         ? ImGuiCol_ButtonHovered
                                                           : ImGuiCol_Button);
        //const ImU32 text_col = ImGui::GetColorU32(ImGuiCol_Text);
        ImGui::RenderNavHighlight(bb, id);
        ImGui::RenderFrame(bb.Min, bb.Max, bg_col, true, g.Style.FrameRounding);
        ImGui::RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, {0.55f, 0.45f}, &bb);
        /*ImGui::RenderArrow(window->DrawList,
                    bb.Min +
                        ImVec2(ImMax(0.0f, (size.x - g.FontSize) * 0.5f), ImMax(0.0f, (size.y - g.FontSize) * 0.5f)),
                    text_col, dir);*/

        IMGUI_TEST_ENGINE_ITEM_INFO(id, str_id, g.LastItemData.StatusFlags);
        return pressed;
    }

    bool StateButton(const char* str_id, const char* label) {
        float sz = ImGui::GetFrameHeight();
        return StateButtonEx(str_id, label, ImVec2(sz, sz), ImGuiButtonFlags_None);
    }

    // UIWidgets V2

    void PushStyleMenu(const ImVec4& color) {
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(color.x, color.y, color.z, 0.5f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(color.x, color.y, color.z, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_PopupBg, UIWidgets::Colors::DarkGray);
        ImGui::PushStyleColor(ImGuiCol_Border, UIWidgets::Colors::DarkGray);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 15.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 3.0f);
    }

    void PopStyleMenu() {
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(4);
    }

    bool BeginMenu(const char* label, const ImVec4& color) {
        bool dirty = false;
        PushStyleMenu(color);
        ImGui::SetNextWindowSizeConstraints(ImVec2(200.0f, 0.0f), ImVec2(FLT_MAX, FLT_MAX));
        if (ImGui::BeginMenu(label)) {
            dirty = true;
        }
        PopStyleMenu();
        return dirty;
    }

    void PushStyleMenuItem(const ImVec4& color) {
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(color.x, color.y, color.z, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(20.0f, 15.0f));
    }

    void PopStyleMenuItem() {
        ImGui::PopStyleVar(1);
        ImGui::PopStyleColor(1);
    }

    bool MenuItem(const char* label, const char* shortcut, const ImVec4& color) {
        bool dirty = false;
        PushStyleMenuItem(color);
        if (ImGui::MenuItem(label, shortcut)) {
            dirty = true;
        }
        PopStyleMenuItem();
        return dirty;
    }

    void PushStyleButton(const ImVec4& color) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(color.x, color.y, color.z, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(color.x, color.y, color.z, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(color.x, color.y, color.z, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.3f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 8.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 5.0f);
    }

    void PopStyleButton() {
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(4);
    }

    bool Button(const char* label, const ButtonOptions& options) {
        ImGui::BeginDisabled(options.disabled);
        PushStyleButton(options.color);
        bool dirty = ImGui::Button(label, options.size);
        PopStyleButton();
        ImGui::EndDisabled();
        if (options.disabled && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && strcmp(options.disabledTooltip, "") != 0) {
            ImGui::SetTooltip("%s", WrappedText(options.disabledTooltip));
        } else if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && strcmp(options.tooltip, "") != 0) {
            ImGui::SetTooltip("%s", WrappedText(options.tooltip));
        }
        return dirty;
    }

    bool WindowButton(const char* label, const char* cvarName, std::shared_ptr<LUS::GuiWindow> windowPtr, const ButtonOptions& options) {
        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0, 0));
        std::string buttonText = label;
        bool dirty = false;
        if (CVarGetInteger(cvarName, 0)) {
            buttonText = ICON_FA_WINDOW_CLOSE " " + buttonText;
        } else {
            buttonText = ICON_FA_EXTERNAL_LINK_SQUARE " " + buttonText;
        }
        if (Button(buttonText.c_str(), options)) {
            windowPtr->ToggleVisibility();
            dirty = true;
        }
        ImGui::PopStyleVar();
        return dirty;
    }

    void PushStyleCheckbox(const ImVec4& color) {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(color.x, color.y, color.z, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(color.x, color.y, color.z, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(color.x, color.y, color.z, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.3f));
        ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.0f, 1.0f, 1.0f, 0.7f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 6.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 5.0f);
    }

    void PopStyleCheckbox() {
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(5);
    }

    bool Checkbox(const char* label, bool* value, const CheckboxOptions& options) {
        ImGui::PushID(label);
        bool dirty = false;
        float startX = ImGui::GetCursorPosX();
        std::string invisibleLabelStr = "##" + std::string(label);
        const char* invisibleLabel = invisibleLabelStr.c_str();
        ImGui::BeginDisabled(options.disabled);
        PushStyleCheckbox(options.color);
        if (options.alignment == ComponentAlignment::Right) {
            if (options.labelPosition == LabelPosition::Near || options.labelPosition == LabelPosition::Far || options.labelPosition == LabelPosition::None) {
                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::GetStyle().FramePadding.x * 2 - ImGui::GetStyle().ItemSpacing.x);
            } else if (options.labelPosition == LabelPosition::Above) {
                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(label).x);
                ImGui::Text(label);
                ImGui::NewLine();
                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::GetStyle().FramePadding.x * 2 - ImGui::GetStyle().ItemSpacing.x);
            }
        } else if (options.alignment == ComponentAlignment::Left) {
            if (options.labelPosition == LabelPosition::Above) {
                ImGui::Text(label);
            }
        }
        dirty = ImGui::Checkbox(invisibleLabel, value);
        if (options.alignment == ComponentAlignment::Right) {
            if (options.labelPosition == LabelPosition::Near) {
                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(label).x - ImGui::GetStyle().FramePadding.x * 2 - ImGui::GetStyle().ItemSpacing.x * 2);
                ImGui::Text(label);
            } else if (options.labelPosition == LabelPosition::Far) {
                ImGui::SameLine();
                ImGui::SetCursorPosX(startX);
                ImGui::Text(label);
            }
        } else if (options.alignment == ComponentAlignment::Left) {
            if (options.labelPosition == LabelPosition::Near) {
                ImGui::SameLine();
                ImGui::Text(label);
            } else if (options.labelPosition == LabelPosition::Far) {
                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(label).x);
                ImGui::Text(label);
            }
        }
        PopStyleCheckbox();
        ImGui::EndDisabled();
        if (options.disabled && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && strcmp(options.disabledTooltip, "") != 0) {
            ImGui::SetTooltip("%s", WrappedText(options.disabledTooltip));
        } else if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && strcmp(options.tooltip, "") != 0) {
            ImGui::SetTooltip("%s", WrappedText(options.tooltip));
        }
        ImGui::PopID();
        return dirty;
    }

    bool CVarCheckbox(const char* label, const char* cvarName, const CheckboxOptions& options) {
        bool dirty = false;
        bool value = (bool)CVarGetInteger(cvarName, options.defaultValue);
        if (Checkbox(label, &value, options)) {
            CVarSetInteger(cvarName, value);
            LUS::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
            dirty = true;
        }
        return dirty;
    }

    void PushStyleCombobox(const ImVec4& color) {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(color.x, color.y, color.z, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(color.x, color.y, color.z, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(color.x, color.y, color.z, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(color.x, color.y, color.z, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(color.x, color.y, color.z, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(color.x, color.y, color.z, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(color.x, color.y, color.z, 0.5f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(color.x, color.y, color.z, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(color.x, color.y, color.z, 0.6f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 3.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 6.0f));
    }

    void PopStyleCombobox() {
        ImGui::PopStyleVar(4);
        ImGui::PopStyleColor(9);
    }

    void PushStyleSlider(const ImVec4& color) {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(color.x, color.y, color.z, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(color.x, color.y, color.z, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(color.x, color.y, color.z, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(color.x, color.y, color.z, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.0, 1.0, 1.0, 0.4f));
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(1.0, 1.0, 1.0, 0.5f));
        ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 3.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 8.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    }

    void PopStyleSlider() {
        ImGui::PopStyleVar(4);
        ImGui::PopStyleColor(6);
    }

    bool SliderInt(const char* label, int32_t* value, int32_t min, int32_t max, const IntSliderOptions& options) {
        bool dirty = false;
        std::string invisibleLabelStr = "##" + std::string(label);
        const char* invisibleLabel = invisibleLabelStr.c_str();
        ImGui::PushID(label);
        ImGui::BeginGroup();
        ImGui::BeginDisabled(options.disabled);
        PushStyleSlider(options.color);
        if (options.alignment == ComponentAlignment::Left) {
            if (options.labelPosition == LabelPosition::Above) {
                ImGui::Text(label, *value);
            }
        } else if (options.alignment == ComponentAlignment::Right) {
            if (options.labelPosition == LabelPosition::Above) {
                ImGui::NewLine();
                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(label).x);
                ImGui::Text(label, *value);
            }
        }
        if (options.showButtons) {
            if (Button("-", { .color = options.color, .size = Sizes::Inline }) && *value > min) {
                *value -= options.step;
                if (*value < min) *value = min;
                LUS::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
                dirty = true;
            }
            ImGui::SameLine(0, 3.0f);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - (ImGui::CalcTextSize("+").x + 20.0f + 3.0f));
        } else {
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        }
        if (ImGui::SliderScalar(invisibleLabel, ImGuiDataType_S32, value, &min, &max, options.format, options.flags)) {
            LUS::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
            dirty = true;
        }
        if (options.showButtons) {
            ImGui::SameLine(0, 3.0f);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (Button("+", { .color = options.color, .size = Sizes::Inline }) && *value < max) {
                *value += options.step;
                if (*value > max) *value = max;
                LUS::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
                dirty = true;
            }
        }
        PopStyleSlider();
        ImGui::EndDisabled();
        ImGui::EndGroup();
        if (options.disabled && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && strcmp(options.disabledTooltip, "") != 0) {
            ImGui::SetTooltip("%s", WrappedText(options.disabledTooltip));
        } else if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && strcmp(options.tooltip, "") != 0) {
            ImGui::SetTooltip("%s", WrappedText(options.tooltip));
        }
        ImGui::PopID();
        return dirty;
    }

    bool CVarSliderInt(const char* label, const char* cvarName, int32_t min, int32_t max, const int32_t defaultValue, const IntSliderOptions& options) {
        bool dirty = false;
        int32_t value = CVarGetInteger(cvarName, defaultValue);
        if (SliderInt(label, &value, min, max, options)) {
            CVarSetInteger(cvarName, value);
            LUS::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
            dirty = true;
        }
        return dirty;
    }

    bool SliderFloat(const char* label, float* value, float min, float max, const FloatSliderOptions& options) {
        bool dirty = false;
        std::string invisibleLabelStr = "##" + std::string(label);
        const char* invisibleLabel = invisibleLabelStr.c_str();
        float valueToDisplay = options.isPercentage ? *value * 100.0f : *value;
        float maxToDisplay = options.isPercentage ? max * 100.0f : max;
        float minToDisplay = options.isPercentage ? min * 100.0f : min;
        ImGui::PushID(label);
        ImGui::BeginGroup();
        ImGui::BeginDisabled(options.disabled);
        PushStyleSlider(options.color);
        if (options.alignment == ComponentAlignment::Left) {
            if (options.labelPosition == LabelPosition::Above) {
                ImGui::Text(label, valueToDisplay);
            }
        } else if (options.alignment == ComponentAlignment::Right) {
            if (options.labelPosition == LabelPosition::Above) {
                ImGui::NewLine();
                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(label).x);
                ImGui::Text(label, valueToDisplay);
            }
        }
        if (options.showButtons) {
            if (Button("-", { .color = options.color, .size = Sizes::Inline }) && *value > min) {
                *value -= options.step;
                if (*value < min) *value = min;
                LUS::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
                dirty = true;
            }
            ImGui::SameLine(0, 3.0f);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - (ImGui::CalcTextSize("+").x + 20.0f + 3.0f));
        } else {
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        }
        if (ImGui::SliderScalar(invisibleLabel, ImGuiDataType_Float, &valueToDisplay, &minToDisplay, &maxToDisplay, options.format, options.flags)) {
            *value = options.isPercentage ? valueToDisplay / 100.0f : valueToDisplay;
            LUS::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
            dirty = true;
        }
        if (options.showButtons) {
            ImGui::SameLine(0, 3.0f);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (Button("+", { .color = options.color, .size = Sizes::Inline }) && *value < max) {
                *value += options.step;
                if (*value > max) *value = max;
                LUS::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
                dirty = true;
            }
        }
        PopStyleSlider();
        ImGui::EndDisabled();
        ImGui::EndGroup();
        if (options.disabled && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && strcmp(options.disabledTooltip, "") != 0) {
            ImGui::SetTooltip("%s", WrappedText(options.disabledTooltip));
        } else if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && strcmp(options.tooltip, "") != 0) {
            ImGui::SetTooltip("%s", WrappedText(options.tooltip));
        }
        ImGui::PopID();
        return dirty;
    }

    bool CVarSliderFloat(const char* label, const char* cvarName, float min, float max, const float defaultValue, const FloatSliderOptions& options) {
        bool dirty = false;
        float value = CVarGetFloat(cvarName, defaultValue);
        if (SliderFloat(label, &value, min, max, options)) {
            CVarSetFloat(cvarName, value);
            LUS::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
            dirty = true;
        }
        return dirty;
    }
}
