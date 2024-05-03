#pragma once

#include "element.h"
#include "animator.h"

class c_toggle : public c_element
{

private:

    bool m_value{};

    c_animator<float> m_animator{ 0.f, 0.f, 0.6f };
    c_animator<ImVec4> m_track_animator{ ImVec4(43 / 255.f, 43 / 255.f, 46 / 255.f, 1.f), ImVec4(52 / 255.f, 50 / 255.f, 168 / 255.f, 1.f) };
    c_animator<ImVec4> m_circle_animator{ ImVec4(85 / 255.f, 85 / 255.f, 85 / 255.f, 1.f), ImVec4(255 / 255.f, 255 / 255.f, 255 / 255.f, 1.f) };

public:

    inline c_toggle(const std::string_view& label, const bool default_visibility = true) noexcept
    {

        this->m_label = label;
        this->m_visible = default_visibility;
        this->m_type = element_type::SWITCH;
        this->m_flags = element_flags::WITH_VALUE;

    }

    virtual bool render(float dpi_scale)
    {

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(200.f / 255.f, 200.f / 255.f, 200 / 255.f, 1.f));
        ImGui::Text(m_label.c_str());
        ImGui::PopStyleColor();

        ImGui::SameLine(ImGui::GetContentRegionAvail().x - (16.f * 2.f));

        ImVec2 p = ImGui::GetCursorScreenPos();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        const float height = 16.f;
        const float width = height * 2.f;
        const float radius = height * 0.5f;

        const bool value_modified = ImGui::InvisibleButton(std::string("##" + m_label).c_str(), ImVec2(width, height), ImGuiButtonFlags_AlignTextBaseLine);

        const float end_x = p.x + width - radius;
        const float base_x = p.x + radius + 0.5f;
        constexpr float duration = 0.46f;

        m_animator.set_end_value(end_x);
        m_animator.set_start_value(base_x);

        m_animator.update_animation();
        m_track_animator.update_animation();
        m_circle_animator.update_animation();

        if (value_modified && !m_animator.is_animating() && !m_track_animator.is_animating() && !m_circle_animator.is_animating())
            if (m_animator.start_animation(!m_value) && m_track_animator.start_animation(!m_value) && m_circle_animator.start_animation(!m_value))
                m_value ^= 1;

        if (m_value && m_animator.get_value() == base_x)
        {

            m_animator.start_animation(m_value);
            m_track_animator.start_animation(m_value);
            m_circle_animator.start_animation(m_value);

        }

        ImU32 col_bg;

        const bool hovered = ImGui::IsItemHovered();

        auto track_color = m_track_animator.get_value();
        auto circle_color = m_circle_animator.get_value();

        draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), ImGui::ColorConvertFloat4ToU32({ track_color.x, track_color.y, track_color.z, ImGui::GetStyle().Alpha}), ImGui::GetStyle().FrameRounding);
        draw_list->AddCircleFilled(ImVec2(m_animator.get_value(), p.y + radius), radius, ImGui::ColorConvertFloat4ToU32({ circle_color.x, circle_color.y, circle_color.z, (ImGui::GetStyle().Alpha * 170.f) / 255.f }));

        return value_modified;

    }

    bool get_value() const noexcept { return m_value; }
    void set_value(bool value) noexcept { m_value = value; }

};
