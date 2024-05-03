#pragma once

#include "includes.h"

#include "element.h"

#include "animator.h"

class c_child
{

private:

    friend class c_window;

    std::vector <c_element*> m_elements{};
    std::string m_name{};
    ImVec2 m_size{};
    ImVec2 m_position{};
    std::function<bool(void)> m_visibility_callback{};
    std::function<void(void)> m_start_render_callback{};
    std::function<void(void)> m_end_render_callback{};
    c_animator<float> m_hover_animator{ 0.f, 1.f, framework::animated_border_duration };
    c_animator<float> m_gradient_animator{ 0.f, 1.f, 0.26f };
    c_window* m_parent_window{};
    bool m_has_header{};
    bool m_visible{};
    bool m_child_drawn{};
    bool m_abs_pos{};

    void render_elements(float dpi_scale) noexcept
    {

        if (!m_child_drawn)
            return;

        for (auto& element : m_elements)
        {

            if (!element)
                continue;

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

    }

    void start_child_render(float dpi_scale) noexcept
    {

        auto imgui_alpha = ImGui::GetStyle().Alpha;

        if (m_start_render_callback)
            m_start_render_callback();

        ImGui::SetCursorPos(m_position * dpi_scale);

        m_child_drawn = ImGui::BeginChild(std::string(m_name + "##child").c_str(), m_size * dpi_scale, true, ImGuiWindowFlags_NoScrollbar);

        if (!m_child_drawn)
            return;

        ImGui::RenderDropShadow(ImGui::shadow_texture, 10.f, 100);

        ImGui::GetBackgroundDrawList()->AddRectFilled(ImGui::GetWindowPos(), ImGui::GetWindowPos() + (m_size * dpi_scale), IM_COL32(35, 30, 35, 255 * imgui_alpha), ImGui::GetStyle().ChildRounding);

        if (m_has_header)
        {

            auto foreground = ImGui::GetBackgroundDrawList();
            auto top_bar_sz = 24.f * dpi_scale;
            auto pos = ImGui::GetWindowPos(); auto sz = ImGui::GetWindowSize();

            foreground->AddRectFilledMultiColor(pos, pos + ImVec2(sz.x, top_bar_sz), IM_COL32(35, 31, 36, 255 * imgui_alpha), IM_COL32(35, 31, 36, 255 * imgui_alpha), IM_COL32(45, 44, 59, 255 * imgui_alpha), IM_COL32(45, 44, 59, 255 * imgui_alpha));
            foreground->AddLine(pos + ImVec2(0.f, top_bar_sz), pos + ImVec2(sz.x, top_bar_sz), IM_COL32(130, 125, 150, 200 * imgui_alpha));

            ImGui::PushFont(framework::bold_font);

            auto text_size = ImGui::CalcTextSize(m_name.c_str());

            foreground->AddText(pos + ImVec2(7.f, ((top_bar_sz - text_size.y) + 5.f) / 2.f), IM_COL32(0, 0, 0, 255 * imgui_alpha), m_name.c_str());
            foreground->AddText(pos + ImVec2(9.f, (top_bar_sz - text_size.y) / 2.f), IM_COL32(130, 125, 150, 255 * imgui_alpha), m_name.c_str());

            ImGui::PopFont();

            ImGui::SetCursorPos({ 10.f, top_bar_sz + 11.f });

        }

        m_hover_animator.update_animation();

        const bool hovered = ImGui::IsMouseHoveringRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + (m_size * dpi_scale), false);

        ImGui::GetStyle().Colors[ImGuiCol_AnimatedBorder].w = m_hover_animator.get_value();

        ImGui::GetBackgroundDrawList()->AddRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + (m_size * dpi_scale), ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_AnimatedBorder)), ImGui::GetStyle().ChildRounding);

        ImGui::GetStyle().Colors[ImGuiCol_AnimatedBorder].w = 200 / 255.f;

        if (hovered)
            m_hover_animator.start_animation(true, false, m_hover_animator.get_animation_percent() == 1.f ? true : false);
        else if (m_hover_animator.get_animation_percent() == 1.f)
            m_hover_animator.start_animation(false);

        ImGui::BeginChild(std::string(m_name + "_inner").c_str(), ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 11.f), false);

    }

    void end_child_render(float dpi_scale) const noexcept
    {

        if (m_end_render_callback)
            m_end_render_callback();

        auto innner_child_sz = ImGui::GetWindowSize();
        auto background = ImGui::GetWindowDrawList();

        ImGui::EndChild();

        if (m_child_drawn)
        {

            auto pos = ImGui::GetWindowPos();
            auto size = m_size * dpi_scale;
            auto top_bar_sz = 24.f * dpi_scale;
            auto imgui_alpha = ImGui::GetStyle().Alpha;

            // top gradient
            background->AddRectFilledMultiColor({ pos.x, (pos.y + top_bar_sz) + 1.f }, { (pos + size).x, pos.y + top_bar_sz + (26.f * dpi_scale) }, IM_COL32(130, 125, 150, 100 * imgui_alpha), IM_COL32(130, 125, 150, 100 * imgui_alpha), IM_COL32(35, 30, 35, 25 * imgui_alpha), IM_COL32(35, 30, 35, 25 * imgui_alpha));

            // bottom gradient
            background->AddRectFilledMultiColor({ pos.x, (pos.y + size.y) - ((27.f * dpi_scale) * m_hover_animator.get_animation_percent()) }, { (pos + size).x, (pos.y + size.y) - 1.f }, IM_COL32(35, 30, 35, 25 * imgui_alpha), IM_COL32(35, 30, 35, 25 * imgui_alpha), IM_COL32(130, 125, 150, 100 * imgui_alpha), IM_COL32(130, 125, 150, 100 * imgui_alpha));

            ImGui::EndChild();

        }

    }

public:

    inline c_child(const std::string_view& child_name, const ImVec2& position, const ImVec2& size, c_window* parent, const bool absolute_pos = false, const bool has_header = true, const bool default_visibility = true) noexcept
    {

        this->m_name = child_name;
        this->m_position = position;
        this->m_size = size;
        this->m_has_header = has_header;
        this->m_abs_pos = absolute_pos;
        this->m_visible = default_visibility;
        this->m_parent_window = parent;

    }

    void clear_elements(std::string filter = "") 
    { 

        if (filter != "")
        {

            m_elements.clear();

            this->insert_element<c_button>("refresh scripts");

            this->insert_element<c_button>("open scripts folder");

        }
        else
            m_elements.clear();

    }

    void set_visibility_callback(const std::function<bool(void)>& value) noexcept { m_visibility_callback = value; }

    void run_visibility_callback() noexcept { if (m_visibility_callback) m_visible = m_visibility_callback(); }

    void set_start_render_callback(const std::function<void(void)>& value) noexcept { m_start_render_callback = value; }

    void set_end_render_callback(const std::function<void(void)>& value) noexcept { m_end_render_callback = value; }

    template <typename type, typename... args>
    type* insert_element(args... initializers) noexcept
    {

        type* allocated_element = new type(initializers...);

        m_elements.push_back((c_element*)allocated_element);

        return allocated_element;

    }

    std::string get_name() const noexcept { return m_name; }

    template<typename type>
    type* find_element(const std::string_view& element_name) noexcept
    {

        for (const auto& element : m_elements)
            if (element->get_label() == element_name) [[unlikely]]
                return (type*)element; // we have to cast cause they're stored as c_element(s) but
        // if we're doing find element we prolly want the special properties...

        return nullptr; // if we couldn't find the element

    }

    bool pop_element(const std::string_view& element_label) noexcept
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

    bool pop_element(c_element* find_element) noexcept
    {

        if (!find_element)
            return false;

        for (const auto& element : m_elements)
        {

            if (element == find_element)
            {

                const auto position = std::find(m_elements.begin(), m_elements.end(), element);

                if (position != m_elements.end()) [[unlikely]]
                    m_elements.erase(position);

                return true;

            }

        }

        return false;

    }

    void render(float dpi_scale) noexcept
    {

        run_visibility_callback();

        if (!m_visible)
            return;

        start_child_render(dpi_scale);

        render_elements(dpi_scale);

        end_child_render(dpi_scale);

    }

    c_window* get_parent() { return m_parent_window; }

};
