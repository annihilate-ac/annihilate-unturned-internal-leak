#pragma once

#include "element.h"

class c_checkbox : public c_element
{

private:

    bool m_value{ false };
    c_animator<float> m_animator{ 0.f, 1.f, 0.26f };
    c_animator<float> m_hover_animator{ 0.f, 1.f, framework::animated_border_duration };
    c_animator<ImVec4> m_text_animator{ ImVec4(230 / 255.f, 230 / 255.f, 230 / 255.f, 130 / 255.f), ImVec4(1.f, 1.f, 1.f, 1.f), 0.43f };

public:

    inline c_checkbox(const std::string_view& label, bool* value = nullptr, const bool default_visibility = true) noexcept
    {

        this->m_label = label;
        this->m_visible = default_visibility;
        if (value) this->m_value = *value;
        this->m_type = element_type::CHECKBOX;
        this->m_flags = element_flags::WITH_VALUE;

    }

    bool get_value() noexcept { return m_value; }
    void set_value(bool value) noexcept { m_value = value; }

    bool* get_value_ptr() { return &m_value; }

    virtual bool render(float dpi_scale)
    {

        static auto custom_checkbox = [](const char* label, bool* v, c_animator<float>* animator, c_animator<float>* hover_animator, c_animator<ImVec4>* text_animator, float dpi_scale)
        {

            ImGuiWindow* window = ImGui::GetCurrentWindow();
            if (window->SkipItems)
                return false;

            ImGuiContext& g = *GImGui;
            const ImGuiStyle& style = g.Style;
            const ImGuiID id = window->GetID(label);
            const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
            const ImVec2 pos = window->DC.CursorPos;

            ImRect container_bb(pos, pos + ImVec2(ImGui::GetWindowWidth() - 10.f, 24.f * dpi_scale));
            ImRect check_bb(pos + ImVec2(8.f * dpi_scale, 6.f * dpi_scale), pos + ImVec2(20.f * dpi_scale, 18.f * dpi_scale));

            ImGui::ItemSize(container_bb, style.FramePadding.y);
            if (!ImGui::ItemAdd(container_bb, id))
                return false;

            bool hovered, held;
            bool pressed = ImGui::ButtonBehavior(container_bb, id, &hovered, &held);

            hover_animator->update_animation();
            animator->update_animation();
            text_animator->update_animation();

            if (pressed && animator->start_animation(!(*v)))
            {
                *v = !(*v);
                ImGui::MarkItemEdited(id);
            }

            auto imgui_alpha = ImGui::GetStyle().Alpha;

            window->DrawList->AddRectFilled(container_bb.Min, container_bb.Max, IM_COL32(25, 20, 25, 255 * imgui_alpha));

            window->DrawList->AddRectFilledMultiColor(container_bb.Min, { container_bb.Max.x, container_bb.Min.y + 6.f }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0));
            window->DrawList->AddRectFilledMultiColor(container_bb.Max, { container_bb.Min.x, container_bb.Max.y - 6.f }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0));

            window->DrawList->AddRectFilledMultiColor(container_bb.Min, { container_bb.Min.x + 6.f, container_bb.Max.y }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0), IM_COL32(95, 92, 103, 200 * imgui_alpha));
            window->DrawList->AddRectFilledMultiColor(container_bb.Max, { container_bb.Max.x - 6.f, container_bb.Min.y }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0), IM_COL32(95, 92, 103, 200 * imgui_alpha));

            auto bdr_clr = ImGui::GetStyleColorVec4(ImGuiCol_AnimatedBorder);

            window->DrawList->AddRect(container_bb.Min, container_bb.Max, ImGui::GetColorU32(ImVec4(bdr_clr.x, bdr_clr.y, bdr_clr.z, hover_animator->get_value())));

            window->DrawList->AddRectFilled(check_bb.Min, check_bb.Max, IM_COL32(25, 20, 25, 255 * imgui_alpha));
            window->DrawList->AddRect(check_bb.Min, check_bb.Max, ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_Border)));

            auto adj_min = check_bb.Min + ImVec2(1.f, 1.f);
            auto adj_max = check_bb.Max - ImVec2(1.f, 1.f);

            window->DrawList->AddRectFilled(adj_min, adj_max, IM_COL32(133, 121, 145, static_cast<int>((animator->get_value() * 255.f) * imgui_alpha)));

            ImVec2 label_pos = ImVec2(check_bb.Max.x + 6.f, (pos.y - 1.f) + (container_bb.GetHeight() - label_size.y) / 2.f);

            ImGui::PushStyleColor(ImGuiCol_Text, text_animator->get_value());

            if (label_size.x > 0.0f)
                ImGui::RenderText(label_pos, label);

            ImGui::PopStyleColor();

            if (ImGui::IsItemHovered())
                hover_animator->start_animation(true, false, hover_animator->get_animation_percent() == 1.f ? true : false);
            else if (hover_animator->get_animation_percent() == 1.f)
                hover_animator->start_animation(false);

            if (*v && text_animator->get_animation_percent() == 0.f)
                text_animator->start_animation(true, false, text_animator->get_animation_percent() == 1.f ? true : false);
            else if (!*v && text_animator->get_animation_percent() == 1.f)
                text_animator->start_animation(false);

            if (*v && animator->get_animation_percent() == 0.f)
                animator->start_animation(true);
            else if (!*v && animator->get_animation_percent() == 1.f)
                animator->start_animation(false);

            return pressed;

        };

        auto ret = custom_checkbox(m_label.c_str(), &m_value, &m_animator, &m_hover_animator, &m_text_animator, dpi_scale);

        if (m_has_tooltip && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
        {

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(230 / 255.f, 230 / 255.f, 230 / 255.f, 130 / 255.f));
            ImGui::SetTooltip(m_tooltip.c_str());
            ImGui::PopStyleColor();

        }

        return ret;

    }

};
