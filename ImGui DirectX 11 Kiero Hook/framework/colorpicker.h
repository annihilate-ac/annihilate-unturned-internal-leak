#pragma once

#include "element.h"

#ifdef USE_CUSTOM_COLORS

static std::vector<struct c_color*> g_colors{};

class c_color
{

public:
    float r{ 1.f }, g{ 1.f }, b{ 1.f }, a{ 1.f };
    bool rainbow{ false };

    inline c_color() noexcept
    {

        this->r = this->g = this->b = this->a = 1.f;
        g_colors.push_back(this);

    }

    inline c_color(int r, int g, int b, int a) noexcept
    {

        this->r = r / 255.f;
        this->g = g / 255.f;
        this->b = b / 255.f;
        this->a = a / 255.f;
        g_colors.push_back(this);

    }

    inline c_color(float r, float g, float b, float a) noexcept
    {

        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
        g_colors.push_back(this);

    }

    operator ImColor() const { return ImVec4(r, g, b, a); }

    ImU32 u32()
    {
        return IM_COL32(r * 255.f, g * 255.f, b * 255.f, a * 255.f);
    }

    ImU32 u32(int alpha)
    {

        return IM_COL32(r * 255.f, g * 255.f, b * 255.f, alpha);

    }

    ImU32 u32(float alpha)
    {

        return IM_COL32(r * 255.f, g * 255.f, b * 255.f, alpha * 255.f);

    }

};

__forceinline c_color from_u32(unsigned int col)
{

    c_color ret{};
    ret.r = ((col) & 0xFF) / 255.0;
    ret.g = ((col >> IM_COL32_G_SHIFT) & 0xFF) / 255.0;
    ret.b = ((col >> IM_COL32_B_SHIFT) & 0xFF) / 255.0;
    ret.a = ((col >> IM_COL32_A_SHIFT) & 0xFF) / 255.0;

    return ret;

}

namespace framework
{

    static c_color copied_color = c_color{ 1.f, 1.f, 1.f, 1.f };

    static auto button_ex3 = [&](const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags, float dpi_scale, c_animator<float>* animator) -> bool {

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);
        const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

        ImVec2 pos = window->DC.CursorPos;
        if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
            pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
        ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

        ImRect container_bb(pos, pos + ImVec2(ImGui::GetContentRegionAvail().x, 36.f * dpi_scale));
        ImRect button_bb(pos + ImVec2(21.f * dpi_scale, 6.f * dpi_scale), window->DC.CursorPos + ImVec2(192.f * dpi_scale, 30.f * dpi_scale));

        ImGui::ItemSize(container_bb, style.FramePadding.y);
        if (!ImGui::ItemAdd(container_bb, id))
            return false;

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(button_bb, id, &hovered, &held, flags);

        // Render
        const ImU32 col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
        //RenderNavHighlight(bb, id);
        //RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);

        auto imgui_alpha = ImGui::GetStyle().Alpha;

        window->DrawList->AddRectFilled(container_bb.Min, container_bb.Max, IM_COL32(25, 20, 25, 255 * imgui_alpha));

        window->DrawList->AddRectFilledMultiColor(container_bb.Min, { container_bb.Max.x, container_bb.Min.y + 6.f }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0));
        window->DrawList->AddRectFilledMultiColor(container_bb.Max, { container_bb.Min.x, container_bb.Max.y - 6.f }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0));

        window->DrawList->AddRectFilledMultiColor(container_bb.Min, { container_bb.Min.x + 6.f, container_bb.Max.y }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0), IM_COL32(95, 92, 103, 200 * imgui_alpha));
        window->DrawList->AddRectFilledMultiColor(container_bb.Max, { container_bb.Max.x - 6.f, container_bb.Min.y }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0), IM_COL32(95, 92, 103, 200 * imgui_alpha));

        window->DrawList->AddRect(container_bb.Min, container_bb.Max, ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_AnimatedBorder)));

        animator->update_animation();

        window->DrawList->AddRectFilled(button_bb.Min, button_bb.Max, IM_COL32(25, 20, 25, 255 * imgui_alpha));

        auto border_color = ImGui::GetStyleColorVec4(ImGuiCol_Border);

        border_color.w = animator->get_value();

        window->DrawList->AddRect(button_bb.Min, button_bb.Max, ImGui::GetColorU32(border_color));

        if (pressed || (hovered && ImGui::GetIO().MouseDown[0]))
            animator->start_animation(true, false, animator->get_animation_percent() == 1.f ? true : false);
        else if (animator->get_animation_percent() == 1.f)
            animator->start_animation(false);

        if (g.LogEnabled)
            ImGui::LogSetNextTextDecoration("[", "]");
        ImGui::RenderTextClipped(button_bb.Min + style.FramePadding, button_bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign);

        // Automatically close popups
        //if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
        //    CloseCurrentPopup();

        IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
        return pressed;

    };

}

class c_colorpicker : public c_element
{

private:

    c_color m_col{};
    c_animator<float> m_copy_animator{ 0.f, 6.f, 0.21f };
    c_animator<float> m_paste_animator{ 0.f, 6.f, 0.21f };
    c_animator<ImVec4> m_copy_text_animator{ ImVec4(230 / 255.f, 230 / 255.f, 230 / 255.f, 130 / 255.f), ImVec4(1.f, 1.f, 1.f, 1.f), 0.43f };
    c_animator<ImVec4> m_paste_text_animator{ ImVec4(230 / 255.f, 230 / 255.f, 230 / 255.f, 130 / 255.f), ImVec4(1.f, 1.f, 1.f, 1.f), 0.43f };
    c_animator<float> m_hover_animator{ 0.f, 1.f, framework::animated_border_duration };
    c_animator<float> m_button_animator{ 0.4f, 1.f, 0.16f };
    c_animator<ImVec4> m_text_animator{ ImVec4(120 / 255.f, 120 / 255.f, 120 / 255.f, 1.f), ImVec4(1.f, 1.f, 1.f, 1.f), 0.43f };

public:

    inline c_colorpicker(const std::string_view& label, const bool default_visibility = true) noexcept
    {

        this->m_label = label;
        this->m_visible = default_visibility;
        this->m_type = element_type::COLORPICKER;
        this->m_flags = element_flags::WITH_VALUE;

        m_col.r = 1.f;
        m_col.g = 1.f;
        m_col.b = 1.f;
        m_col.a = 1.f;

    }

    virtual bool render(float dpi_scale)
    {

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        //ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - (24 * 2));
        //ImGui::SameLine(ImGui::GetContentRegionAvail().x - 24.f);
        //ImGui::Text(m_label.c_str()); ImGui::SameLine(ImGui::GetContentRegionAvail().x - 24.f);
        //ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::CalcTextSize(m_label.c_str()).y);

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const float square_sz = ImGui::GetFrameHeight();
        const float w_full = ImGui::CalcItemWidth();
        const float w_button = (NULL & ImGuiColorEditFlags_NoSmallPreview) ? 0.0f : (square_sz + style.ItemInnerSpacing.x);
        const float w_inputs = w_full - w_button;
        const char* label_display_end = ImGui::FindRenderedTextEnd(m_label.c_str());
        g.NextItemData.ClearFlags();
        auto pos = window->DC.CursorPos;
        ImRect container_bb(pos, pos + ImVec2(ImGui::GetWindowWidth() - 10.f, 36.f * dpi_scale));

        ImGui::ItemSize(container_bb, style.FramePadding.y);
        if (!ImGui::ItemAdd(container_bb, window->GetID(m_label.c_str())))
            return false;

        c_color* real_color = &m_col;

        auto col = &real_color->r;

        auto flags = NULL;
        const ImGuiColorEditFlags flags_untouched = flags;

        m_hover_animator.update_animation();
        m_text_animator.update_animation();

        auto imgui_alpha = ImGui::GetStyle().Alpha;

        window->DrawList->AddRectFilled(container_bb.Min, container_bb.Max, IM_COL32(25, 20, 25, 255 * imgui_alpha));

        window->DrawList->AddRectFilledMultiColor(container_bb.Min, { container_bb.Max.x, container_bb.Min.y + 6.f }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0));
        window->DrawList->AddRectFilledMultiColor(container_bb.Max, { container_bb.Min.x, container_bb.Max.y - 6.f }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0));

        window->DrawList->AddRectFilledMultiColor(container_bb.Min, { container_bb.Min.x + 6.f, container_bb.Max.y }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0), IM_COL32(95, 92, 103, 200 * imgui_alpha));
        window->DrawList->AddRectFilledMultiColor(container_bb.Max, { container_bb.Max.x - 6.f, container_bb.Min.y }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0), IM_COL32(95, 92, 103, 200 * imgui_alpha));

        ImGui::GetStyle().Colors[ImGuiCol_AnimatedBorder].w = m_hover_animator.get_value();

        window->DrawList->AddRect(container_bb.Min, container_bb.Max, ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_AnimatedBorder)));

        ImGui::GetStyle().Colors[ImGuiCol_AnimatedBorder].w = 200 / 255.f;

        if (ImGui::IsMouseHoveringRect(container_bb.Min, container_bb.Max))
            m_hover_animator.start_animation(true, false, m_hover_animator.get_animation_percent() == 1.f ? true : false);
        else if (m_hover_animator.get_animation_percent() == 1.f)
            m_hover_animator.start_animation(false);

        if (ImGui::IsMouseHoveringRect(container_bb.Min, container_bb.Max))
            m_text_animator.start_animation(true, false, m_text_animator.get_animation_percent() == 1.f ? true : false);
        else if (m_text_animator.get_animation_percent() == 1.f)
            m_text_animator.start_animation(false);

        auto label_sz = ImGui::CalcTextSize(m_label.c_str());
        
        auto text_color = m_text_animator.get_value();
        window->DrawList->AddText(container_bb.Min + ImVec2(10.f, (label_sz.y / 2.f) + 4.f), IM_COL32(text_color.x * 255.f, text_color.y * 255.f, text_color.z * 255.f, imgui_alpha * 255.f), m_label.c_str());

        flags |= ImGuiColorEditFlags_NoLabel;
        flags |= ImGuiColorEditFlags_NoOptions;
        flags |= ImGuiColorEditFlags_NoDragDrop;
        flags |= ImGuiColorEditFlags_NoTooltip;

        const bool alpha = (flags & ImGuiColorEditFlags_NoAlpha) == 0;
        const bool hdr = (flags & ImGuiColorEditFlags_HDR) != 0;
        const int components = alpha ? 4 : 3;

        // Convert to the formats we need
        float f[4] = { col[0], col[1], col[2], alpha ? col[3] : 1.0f };
        if ((flags & ImGuiColorEditFlags_InputHSV) && (flags & ImGuiColorEditFlags_DisplayRGB))
            ImGui::ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
        else if ((flags & ImGuiColorEditFlags_InputRGB) && (flags & ImGuiColorEditFlags_DisplayHSV))
        {
            // Hue is lost when converting from greyscale rgb (saturation=0). Restore it.
            ImGui::ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);
            if (memcmp((void*)g.ColorEditSavedColor, col, sizeof(float) * 3) == 0)
            {
                if (f[1] == 0)
                    f[0] = g.ColorEditSavedHue;
                if (f[2] == 0)
                    f[1] = g.ColorEditSavedHue;
            }
        }
        int i[4] = { IM_F32_TO_INT8_UNBOUND(f[0]), IM_F32_TO_INT8_UNBOUND(f[1]), IM_F32_TO_INT8_UNBOUND(f[2]), IM_F32_TO_INT8_UNBOUND(f[3]) };

        bool value_changed = false;
        bool value_changed_as_float = false;
        const float inputs_offset_x = (style.ColorButtonPosition == ImGuiDir_Left) ? w_button : 0.0f;
        window->DC.CursorPos.x = pos.x + inputs_offset_x;

        flags |= ImGuiColorEditFlags_NoInputs;

        //ImGui::PushFontShadow(IM_COL32_BLACK);

        if ((flags & (ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_DisplayHSV)) != 0 && (flags & ImGuiColorEditFlags_NoInputs) == 0)
        {
        }
        else if ((flags & ImGuiColorEditFlags_DisplayHex) != 0)
        {

        }

        ImGuiWindow* picker_active_window = NULL;
        if (!(flags & ImGuiColorEditFlags_NoSmallPreview))
        {
            const float button_offset_x = ((flags & ImGuiColorEditFlags_NoInputs) || (style.ColorButtonPosition == ImGuiDir_Left)) ? 0.0f : w_inputs + style.ItemInnerSpacing.x;
            window->DC.CursorPos = ImVec2(container_bb.Max.x - (34.f * dpi_scale), container_bb.Min.y + (10.f * dpi_scale));

            const ImVec4 col_v4(col[0], col[1], col[2], alpha ? col[3] : 1.0f);
            if (ImGui::ColorButton(m_label.c_str(), col_v4, flags, {24.f * dpi_scale, 16.f * dpi_scale}))
            {
                if (!(flags & ImGuiColorEditFlags_NoPicker))
                {
                    // Store current color and open a picker
                    g.ColorPickerRef = col_v4;
                    ImGui::OpenPopup(std::string(m_label + "picker").c_str());
                    ImGui::SetNextWindowPos(container_bb.Min);
                }
            }
            if (ImGui::IsItemHovered() && ImGui::GetIO().MouseClicked[1])
                ImGui::OpenPopup(std::string(m_label + "copymenu").c_str());

            if (!(flags & ImGuiColorEditFlags_NoOptions))
                ImGui::OpenPopupOnItemClick("context");

            m_copy_animator.update_animation(); m_paste_animator.update_animation();
            m_copy_text_animator.update_animation(); m_paste_text_animator.update_animation();

            if (ImGui::BeginPopup(std::string(m_label + "copymenu").c_str()))
            {

                bool copy_selected = false; bool copy_pressed = false;
                bool paste_selected = false; bool paste_pressed = false;

                ImGui::BeginGroup();

                if (framework::selectable("copy", &copy_selected, ImGuiSelectableFlags_None, &m_copy_animator, &m_copy_text_animator, dpi_scale))
                    framework::copied_color = m_col;

                if (ImGui::IsItemHovered())
                    m_copy_animator.start_animation(true, false, m_copy_animator.get_animation_percent() == 1.f ? true : false);
                else if (m_copy_animator.get_animation_percent() == 1.f)
                    m_copy_animator.start_animation(false);

                if (framework::selectable("paste", &paste_selected, ImGuiSelectableFlags_None, &m_paste_animator, &m_paste_text_animator, dpi_scale)) 
                    m_col = framework::copied_color;

                if (ImGui::IsItemHovered())
                    m_paste_animator.start_animation(true, false, m_paste_animator.get_animation_percent() == 1.f ? true : false);
                else if (m_paste_animator.get_animation_percent() == 1.f)
                    m_paste_animator.start_animation(false);

                ImGui::EndGroup();

                ImGui::EndPopup();

            }

            if (ImGui::IsPopupOpen(std::string(m_label + "picker").c_str(), ImGuiPopupFlags_None))
                ImGui::SetNextWindowSize({ 233.f, 223.f });
            
            if (ImGui::BeginPopup(std::string(m_label + "picker").c_str()))
            {

                ImGui::RenderDropShadow(ImGui::shadow_texture, 20.f, 255);
                picker_active_window = g.CurrentWindow;
                //if (m_label.c_str() != label_display_end)
                //{
                //    ImGui::TextEx(m_label.c_str(), label_display_end);
                //    ImGui::Spacing();
                //}
                ImGuiColorEditFlags picker_flags_to_forward = ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_AlphaBar;
                ImGuiColorEditFlags picker_flags = (flags_untouched & picker_flags_to_forward) | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_AlphaBar;
                //ImGui::SetNextItemWidth(square_sz * 12.0f); // Use 256 + bar sizes?
                value_changed |= ImGui::ColorPicker4(m_label.c_str(), col, picker_flags, &g.ColorPickerRef.x);
                if (framework::button_ex3(m_col.rainbow ? "disable rainbow" : "enable rainbow", ImVec2{}, ImGuiButtonFlags_None, dpi_scale, &m_button_animator))
                    m_col.rainbow ^= 1;
                ImGui::EndPopup();
            }

        }

        // Convert back
        if (value_changed && picker_active_window == NULL)
        {
            if (!value_changed_as_float)
                for (int n = 0; n < 4; n++)
                    f[n] = i[n] / 255.0f;
            if ((flags & ImGuiColorEditFlags_DisplayHSV) && (flags & ImGuiColorEditFlags_InputRGB))
            {
                g.ColorEditSavedHue = f[0];
                g.ColorEditSavedSat = f[1];
                ImGui::ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
                memcpy((void*)g.ColorEditSavedColor, f, sizeof(float) * 3);
            }
            if ((flags & ImGuiColorEditFlags_DisplayRGB) && (flags & ImGuiColorEditFlags_InputHSV))
                ImGui::ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);

            col[0] = f[0];
            col[1] = f[1];
            col[2] = f[2];
            if (alpha)
                col[3] = f[3];
        }

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
        {

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(230 / 255.f, 230 / 255.f, 230 / 255.f, 130 / 255.f));
            ImGui::SetTooltip("RMB for more options");
            ImGui::PopStyleColor();

        }

        // When picker is being actively used, use its active id so IsItemActive() will function on ColorEdit4().
        if (picker_active_window && g.ActiveId != 0 && g.ActiveIdWindow == picker_active_window)
            window->NavLastIds[0] = g.ActiveId;

        if (value_changed)
            ImGui::MarkItemEdited(window->NavLastIds[0]);

        //ImGui::PopFontShadow();

        window->DC.CursorPos.y += 9.f;

        return value_changed;

    }

    c_color get_value() noexcept { return m_col; }
    void set_value(c_color value) noexcept { m_col = value; }
    void set_rainbow(bool value) noexcept { m_col.rainbow = value; }

};

#endif
