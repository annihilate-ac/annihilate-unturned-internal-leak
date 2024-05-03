#pragma once

#include "element.h"
#include "animator.h"

// easy way to group da elemuntaz
class c_group : public c_element
{

private:

    friend class c_window;
    friend class c_collapsable_group;

    std::vector <c_element*> m_elements{};

public:

    c_group() = default;

    inline c_group(const std::string& label, const bool default_visibility = true) noexcept
    {

        this->m_label = label;
        this->m_visible = default_visibility;
        this->m_type = element_type::GROUP;

    }

    template <typename type, typename... args>
    type* insert_element(args... initializers)
    {

        type* allocated_element = new type(initializers...);

        m_elements.push_back((c_element*)allocated_element);

        return allocated_element;

    }

    bool pop_element(const std::string_view& element_label)
    {

        for (const auto& element : m_elements)
            if (element->get_label() == element_label) [[likely]]
            {

                const auto position = std::find(m_elements.begin(), m_elements.end(), element);

                if (position != m_elements.end()) [[unlikely]]
                    m_elements.erase(position);

                return true;

            }

        return false;

    }

    virtual bool render(float dpi_scale)
    {

        int multiplier = 1;

        for (const auto& element : m_elements)
            if (element->m_type != element_type::KEYBIND)
                ++multiplier;

        // someone fix dis 4 me...
        if (const bool ret = ImGui::BeginChild(m_label.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, (30 * multiplier)), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize))
        {

            const auto text_size = ImGui::CalcTextSize(m_label.c_str());
            ImGui::SetCursorPos(ImVec2(ImGui::GetStyle().WindowPadding.x, (ImGui::GetCurrentWindow()->Size.y - text_size.y) / 2.f)); ImGui::TextGradiented(m_label.c_str(), IM_COL32(137, 254, 236, 255), IM_COL32(3, 150, 80, 255), 130.f); ImGui::Spacing();

            ImGui::BeginGroup();

            for (const auto& element : m_elements)
            {

                // properly handle rendering...

                element->run_render_callback();

                if (element->m_wants_same_line)
                    ImGui::SameLine();

                if (element->m_has_custom_position)
                    ImGui::SetCursorPos(element->m_custom_position * dpi_scale);

                if (element->m_custom_font)
                    ImGui::PushFont(element->m_custom_font);

                if (element->render(dpi_scale))
                    element->run_interaction_callback(); // run dat hoe

                if (element->m_custom_font)
                    ImGui::PopFont();

            }

            ImGui::EndGroup();

            ImGui::EndChild();

            return ret;

        }

        return false;

    }

};

class c_collapsable_group : public c_group
{

private:

    bool m_is_collapsed{};

    c_animator<float> m_animator{ 32.f, 60.f, 0.63f };
    c_animator<float> m_element_animator{ 0.f, 1.f, 0.63f };
    c_animator<ImVec4> m_label_animator{ ImVec4(230 / 255.f, 230 / 255.f, 230 / 255.f, 130 / 255.f), ImVec4(1.f, 1.f, 1.f, 1.f), 0.43f };

public:

    inline c_collapsable_group(const std::string& label, const bool collapsed = true, const bool default_visibility = true) noexcept
    {

        this->m_label = label;
        this->m_visible = default_visibility;
        this->m_type = element_type::GROUP;
        this->m_is_collapsed = collapsed;
        m_animator.set_starting_animation(!m_is_collapsed);

    }

    virtual bool render(float dpi_scale)
    {

        float item_height = 48.f;

        for (const auto& element : m_elements)
            if (element->get_visible())
            {

                switch (element->get_type())
                {

                case element_type::CHECKBOX:
                    item_height += 34.f;
                    break;
                case element_type::SINGLE_COMBO:
                    item_height += 58.f;
                    break;
                case element_type::MULTI_COMBO:
                    item_height += 58.f;
                    break;
                case element_type::KEYBIND:
                    item_height += 46.f;
                    break;
                case element_type::SLIDER_FLOAT:
                    item_height += 48.f;
                    break;
                case element_type::SLIDER_INT:
                    item_height += 48.f;
                    break;
                case element_type::BUTTON:
                    item_height += 46.f;
                    break;
                case element_type::COLORPICKER:
                    item_height += 46.f;
                    break;


                }

            }

        auto window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollWithMouse;

        m_animator.set_end_value(item_height);

        m_animator.update_animation();
        m_label_animator.update_animation();
        m_element_animator.update_animation();

        const bool child_drawn = ImGui::BeginChild(m_label.c_str(), ImVec2(ImGui::GetContentRegionAvail().x - 10.f, m_animator.get_value()), true, window_flags);
        bool collapsed_changed = false;

        if (child_drawn)
        {

            auto imgui_alpha = ImGui::GetStyle().Alpha;
            auto child_window = ImGui::GetCurrentWindow();

            auto child_pos = ImGui::GetWindowPos();
            auto child_sz = ImGui::GetWindowSize();

            auto child_bb = ImRect(child_pos, child_pos + child_sz);

            child_window->DrawList->AddRectFilled(child_bb.Min, child_bb.Max, IM_COL32(25, 20, 25, 255 * imgui_alpha), ImGui::GetStyle().ChildRounding);

            child_window->DrawList->AddRectFilledMultiColor(child_bb.Min, { child_bb.Max.x, child_bb.Min.y + 6.f }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0));
            child_window->DrawList->AddRectFilledMultiColor(child_bb.Max, { child_bb.Min.x, child_bb.Max.y - 6.f }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0));

            child_window->DrawList->AddRectFilledMultiColor(child_bb.Min, { child_bb.Min.x + 6.f, child_bb.Max.y }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0), IM_COL32(95, 92, 103, 200 * imgui_alpha));
            child_window->DrawList->AddRectFilledMultiColor(child_bb.Max, { child_bb.Max.x - 6.f, child_bb.Min.y }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0), IM_COL32(95, 92, 103, 200 * imgui_alpha));

            const auto label_bb = ImRect(child_pos + ImVec2(8.f, 8.f), (child_pos + ImVec2(8.f, 8.f)) + framework::g_scaled_fonts.at(1).font->CalcTextSizeA(framework::g_scaled_fonts.at(1).size * dpi_scale, FLT_MAX, 0.f, m_label.c_str()));

            ImGui::SetCursorPos(ImVec2(8, 8)); 
            
            ImGui::PushStyleColor(ImGuiCol_Text, m_label_animator.get_value());

            ImGui::Text(m_label.c_str(), IM_COL32(255, 123, 114, static_cast<int>(ImGui::GetStyle().Alpha * 255.f)), IM_COL32(210, 168, 255, static_cast<int>(ImGui::GetStyle().Alpha * 255.f)), 130.f);

            ImGui::PopStyleColor();

            auto label_hovered = ImGui::IsMouseHoveringRect(label_bb.Min, label_bb.Max);
            auto label_pressed = label_hovered && ImGui::GetIO().MouseClicked[0];

            if (label_pressed && m_animator.start_animation(m_is_collapsed))
            {

                m_is_collapsed = !m_is_collapsed;
                collapsed_changed = true;

            }

            if (label_hovered || (!m_is_collapsed))
                m_label_animator.start_animation(true, false, m_label_animator.get_animation_percent() == 1.f ? true : false);
            else if (m_label_animator.get_animation_percent() == 1.f)
                m_label_animator.start_animation(false);

            if (!m_is_collapsed && collapsed_changed)
                m_element_animator.start_animation(true);
            else if (m_is_collapsed && collapsed_changed)
                m_element_animator.start_animation(false);

            if (m_element_animator.get_value() > 0.f)
            {

                ImGui::Spacing();

                ImGui::SetCursorPosX(10.f);
                ImGui::SetCursorPosY(32.f);

                ImGui::BeginChild(std::string("##inner" + m_label).c_str(), { child_sz.x - 10.f, child_sz.y - 47.f }, false);

                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, m_element_animator.get_value() * ImGui::GetStyle().Alpha);

                for (auto& element : m_elements)
                {

                    element->update_visibility(); // set visibility before render. (todo: make this user changable via the force_visibility param.)

                    if (element->get_visible()) [[likely]]
                        {

                            element->run_render_callback(); // easier to have it here than in every render function...

                            if (element->m_wants_same_line)
                                ImGui::SameLine();

                            if (element->m_has_custom_position)
                                ImGui::SetCursorPos(element->m_custom_position * dpi_scale);

                            if (element->m_custom_font)
                                ImGui::PushFont(element->m_custom_font);

                            if (element->render(dpi_scale))
                                element->run_interaction_callback(); // run dat hoe

                            if (element->m_custom_font)
                                ImGui::PopFont();

                        }

                }

                ImGui::PopStyleVar();

                ImGui::EndChild();

            }

        }

        ImGui::EndChild();

        return child_drawn;

    }

};
