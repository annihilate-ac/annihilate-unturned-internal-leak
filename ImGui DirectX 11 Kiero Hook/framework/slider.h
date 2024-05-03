#pragma once

#include "element.h"

#include "animator.h"

namespace framework
{

    static auto slider_scalar = [&](const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags, c_animator<float>* hover_animator, c_animator<ImVec4>* text_animator, float dpi_scale) -> bool {

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);

        const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
        ImRect container_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(ImGui::GetWindowWidth() - 10.f, 38.f * dpi_scale));
        ImRect slider_bb(window->DC.CursorPos + ImVec2(34.f * dpi_scale, 21.f * dpi_scale), window->DC.CursorPos + ImVec2((34.f * dpi_scale) + (156.f * dpi_scale), 29.f * dpi_scale));

        const bool temp_input_allowed = false;
        ImGui::ItemSize(container_bb, style.FramePadding.y);
        if (!ImGui::ItemAdd(container_bb, id))
            return false;

        hover_animator->update_animation();
        text_animator->update_animation();

        if (format == NULL)
            format = ImGui::DataTypeGetInfo(data_type)->PrintFmt;

        const bool hovered = ImGui::ItemHoverable(slider_bb, id, ImGuiItemFlags_None);
        const bool container_hovered = ImGui::ItemHoverable(container_bb, id, ImGuiItemFlags_None);
        bool temp_input_is_active = temp_input_allowed && ImGui::TempInputIsActive(id);
        if (!temp_input_is_active)
        {
            // Tabbing or CTRL-clicking on Slider turns it into an input box
            const bool input_requested_by_tabbing = temp_input_allowed && (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_FocusedByTabbing) != 0;
            const bool clicked = hovered && ImGui::IsMouseClicked(0, id);
            const bool make_active = (input_requested_by_tabbing || clicked || g.NavActivateId == id);
            if (make_active && clicked)
                ImGui::SetKeyOwner(ImGuiKey_MouseLeft, id);
            if (make_active && temp_input_allowed)
                if (input_requested_by_tabbing || (clicked && g.IO.KeyCtrl) || (g.NavActivateId == id && (g.NavActivateFlags & ImGuiActivateFlags_PreferInput)))
                    temp_input_is_active = true;

            if (make_active && !temp_input_is_active)
            {
                ImGui::SetActiveID(id, window);
                ImGui::SetFocusID(id, window);
                ImGui::FocusWindow(window);
                g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
            }
        }

        auto imgui_alpha = ImGui::GetStyle().Alpha;

        window->DrawList->AddRectFilled(container_bb.Min, container_bb.Max, IM_COL32(25, 20, 25, 255 * imgui_alpha));

        window->DrawList->AddRectFilledMultiColor(container_bb.Min, { container_bb.Max.x, container_bb.Min.y + 6.f }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0));
        window->DrawList->AddRectFilledMultiColor(container_bb.Max, { container_bb.Min.x, container_bb.Max.y - 6.f }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0));

        window->DrawList->AddRectFilledMultiColor(container_bb.Min, { container_bb.Min.x + 6.f, container_bb.Max.y }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0), IM_COL32(95, 92, 103, 200 * imgui_alpha));
        window->DrawList->AddRectFilledMultiColor(container_bb.Max, { container_bb.Max.x - 6.f, container_bb.Min.y }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0), IM_COL32(95, 92, 103, 200 * imgui_alpha));

        auto bdr_clr = ImGui::GetStyleColorVec4(ImGuiCol_Border);

        window->DrawList->AddRect(container_bb.Min, container_bb.Max, ImGui::GetColorU32(ImVec4(bdr_clr.x, bdr_clr.y, bdr_clr.z, hover_animator->get_value())));

        ImRect grab_bb;
        const bool value_changed = ImGui::SliderBehavior(slider_bb, id, data_type, p_data, p_min, p_max, format, flags, &grab_bb);

        if (value_changed)
            ImGui::MarkItemEdited(id);

        static std::unordered_map< ImGuiID, float > values;
        auto value = values.find(id);

        if (value == values.end()) {

            values.insert({ id, { 0.f } });
            value = values.find(id);
        }

        float percent = (grab_bb.Max.x - window->DC.CursorPos.x - slider_bb.Min.x + window->Pos.x) / slider_bb.GetWidth();

        static auto ease = [](double t)
        {

           return t < 0.5 ? 4 * t * t * t : 1 + (--t) * (2 * (--t)) * (2 * t);

        };

        auto lerp_time = (framework::vsync ? ease(3.f) : ease(0.1125f)) * (1.f - ImGui::GetIO().DeltaTime);

        value->second = ImLerp(value->second, percent * slider_bb.GetWidth(), framework::vsync ? 1.265f : 0.00825f);

        window->DrawList->AddRectFilled(slider_bb.Min, slider_bb.Max, IM_COL32(45, 45, 50, 100 * imgui_alpha));

        if (grab_bb.Max.x > grab_bb.Min.x)
            window->DrawList->AddRectFilled(slider_bb.Min, { slider_bb.Min.x + value->second, slider_bb.Max.y }, IM_COL32(133, 121, 145, 255 * imgui_alpha));

        ImGui::PushStyleColor(ImGuiCol_Text, text_animator->get_value());
        if (label_size.x > 0.0f)
            ImGui::RenderText({ slider_bb.Min.x, (slider_bb.Min.y - label_size.y) - 2.f }, label);
        ImGui::PopStyleColor();

        char value_buf[64];
        const char* value_buf_end = value_buf + ImGui::DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, format);

        auto value_sz = ImGui::CalcTextSize(value_buf, value_buf_end);

        if (hover_animator->get_value() > 0.f)
        {

            auto text_color = ImGui::GetStyleColorVec4(ImGuiCol_Text);
            text_color.w = hover_animator->get_value();

            ImGui::PushStyleColor(ImGuiCol_Text, text_color);
            ImGui::RenderText({ slider_bb.Max.x - value_sz.x, (slider_bb.Min.y - value_sz.y) - 2.f }, value_buf, value_buf_end);
            ImGui::PopStyleColor();

        }

        if (ImGui::IsItemHovered())
            hover_animator->start_animation(true, false, hover_animator->get_animation_percent() == 1.f ? true : false);
        else if (hover_animator->get_animation_percent() == 1.f)
            hover_animator->start_animation(false);

        if (ImGui::IsItemHovered())
            text_animator->start_animation(true, false, text_animator->get_animation_percent() == 1.f ? true : false);
        else if (text_animator->get_animation_percent() == 1.f)
            text_animator->start_animation(false);

        IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | (temp_input_allowed ? ImGuiItemStatusFlags_Inputable : 0));
        return value_changed;

    };

}

class c_slider_int : public c_element
{

private:

    std::string m_format_text{ "%d" };
    c_animator<float> m_hover_animator{ 0.f, 1.f, framework::animated_border_duration };
    c_animator<ImVec4> m_text_animator{ ImVec4(230 / 255.f, 230 / 255.f, 230 / 255.f, 130 / 255.f), ImVec4(1.f, 1.f, 1.f, 1.f), 0.43f };
    int m_value = 0.f;
    int m_min{}, m_max{};
    float last_x = 0.f;

public:

    inline c_slider_int(const std::string_view& label, int min, int max, const std::string_view& format_str = "%d", const bool default_visibility = true) noexcept
    {

        this->m_label = label;
        this->m_visible = default_visibility;
        this->m_min = min;
        this->m_max = max;
        this->m_type = element_type::SLIDER_INT;
        this->m_format_text = format_str;
        this->m_flags = element_flags::WITH_VALUE;

        if (this->m_value < this->m_min)
            this->m_value = this->m_min;

    }

    int get_min() const { return m_min; }
    void set_min(const int value) { m_min = value; }

    int get_max() const { return m_max; }
    void set_max(const int value) { m_max = value; }

    int get_value() const { return m_value; }
    void set_value(const int value) { m_value = value; }

    void set_format_text(const std::string_view& style) { m_format_text = style; }

    virtual bool render(float dpi_scale)
    {

        const auto ret = framework::slider_scalar(m_label.c_str(), ImGuiDataType_S32, &m_value, &m_min, &m_max, m_format_text.c_str(), ImGuiSliderFlags_None, &m_hover_animator, &m_text_animator, dpi_scale);

        return ret;

    }

};

class c_slider_float : public c_element
{

private:

    std::string m_format_text{"%1.f"};
    c_animator<float> m_hover_animator{ 0.f, 1.f, framework::animated_border_duration };
    c_animator<ImVec4> m_text_animator{ ImVec4(230 / 255.f, 230 / 255.f, 230 / 255.f, 130 / 255.f), ImVec4(1.f, 1.f, 1.f, 1.f), 0.43f };
    float m_value = 0.f;
    float m_min{}, m_max{};
    float last_x = 0.f;
    
public:

    inline c_slider_float(const std::string_view& label, float min, float max, const std::string_view& format_str = "%1.f", const bool default_visibility = true) noexcept
    {

        this->m_label = label;
        this->m_visible = default_visibility;
        this->m_min = min;
        this->m_max = max;
        this->m_type = element_type::SLIDER_FLOAT;
        this->m_format_text = format_str;
        this->m_flags = element_flags::WITH_VALUE;

        if (this->m_value < this->m_min)
            this->m_value = this->m_min;

    }

    float get_min() const { return m_min; }
    void set_min(const float value) { m_min = value; }

    float get_max() const { return m_max; }
    void set_max(const float value) { m_max = value; }

    float get_value() const { return m_value; }
    void set_value(const float value) { m_value = value; }

    void set_format_text(std::string val) { m_format_text = val; }
    std::string get_format_text() const { return m_format_text; }

    virtual bool render(float dpi_scale)
    {

        const auto ret = framework::slider_scalar(m_label.c_str(), ImGuiDataType_Float, &m_value, &m_min, &m_max, m_format_text.c_str(), ImGuiSliderFlags_None, &m_hover_animator, &m_text_animator, dpi_scale);

        if (m_has_tooltip && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
        {

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(230 / 255.f, 230 / 255.f, 230 / 255.f, 130 / 255.f));
            ImGui::SetTooltip(m_tooltip.c_str());
            ImGui::PopStyleColor();

        }

        return ret;

    }

};
