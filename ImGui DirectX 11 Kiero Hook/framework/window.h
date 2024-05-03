#pragma once

#include "includes.h"

#include "element.h"
#include "child.h"
#include "keybind.h"
#include "attachment.h"
#include "group.h"

class c_window
{

private:

    std::vector <c_element*> m_elements{};
    std::vector <c_child*> m_children{};
    std::vector <c_attachment*> m_attachments{};
    std::function<bool(void)> m_visibility_callback{};
    std::function<void(void)> m_start_render_callback{};
    std::function<void(void)> m_end_render_callback{};
    std::string m_name{};
    ImVec2 m_size{};
    ImVec2 m_pos{};
    float m_dpi_scale{};
    int m_flags{};
    bool m_visible{};

    void render_children()
    {

        for (const auto& child : m_children)
            child->render(m_dpi_scale);

    }

public:

#ifdef ENABLE_BINDS

    std::vector<c_keybind*> get_all_binds() const
    {

        std::vector<c_keybind*> tmp_vec{};

        for (const auto& element : m_elements)
            if (element->m_type == element_type::KEYBIND)
                tmp_vec.push_back((c_keybind*)element);

        for (const auto& child : m_children)
            for (const auto& element : child->m_elements)
                if (element->m_type == element_type::KEYBIND)
                    tmp_vec.push_back((c_keybind*)element);
                else if (element->m_type == element_type::GROUP)
                    for (const auto& element : ((c_group*)element)->m_elements)
                        if (element->m_type == element_type::KEYBIND)
                            tmp_vec.push_back((c_keybind*)element);

        return tmp_vec;

    }

#endif

    std::vector<c_element*> get_all_cfg_elements()
    {

        std::vector<c_element*> tmp_vec{};

        for (auto child : m_children)
            for (auto element : child->m_elements)
                if (element->m_flags == element_flags::WITH_VALUE)
                    tmp_vec.push_back(element);
                else if (element->m_type == element_type::GROUP)
                {

                    auto group = reinterpret_cast<c_collapsable_group*>(element);

                    for (auto group_element : group->m_elements)
                        if (group_element->m_flags == element_flags::WITH_VALUE)
                            tmp_vec.push_back(group_element);

                }

        return tmp_vec;

    }

    void render_elements()
    {

        for (auto& element : m_elements)
        {

            element->update_visibility(); // set visibility before render. (todo: make this user changable via the force_visibility param.)

            if (element->get_visible()) [[likely]]
            {

                element->run_render_callback(); // easier to have it here than in every render function...

                if (element->m_wants_same_line)
                    ImGui::SameLine();

                if (element->m_has_custom_position)
                    ImGui::SetCursorPos(element->m_custom_position * m_dpi_scale);

                if (element->m_custom_font)
                    ImGui::PushFont(element->m_custom_font);

                if (element->render(m_dpi_scale))
                    element->run_interaction_callback(); // run dat hoe

                if (element->m_custom_font)
                    ImGui::PopFont();

            }

        }

    }

    void start_window_render() 
    { 
        
        ImGui::SetNextWindowSize(m_size * m_dpi_scale); 
        
        if (m_start_render_callback)
            m_start_render_callback();

        ImGui::Begin(m_name.c_str(), NULL, m_flags); 
        
        ImGui::RenderDropShadow(ImGui::shadow_texture, 120.f * get_dpi_scale(), 255); 

    }

    void end_window_render() 
    { 
        
        if (m_end_render_callback)
            m_end_render_callback();

        ImGui::End(); 
    
    }

    void update_visibility(const bool force_visibility = false)
    { // make this functional lol (idk what I was thinking here)

        if (!force_visibility && m_visibility_callback) // automatically set visibility based on the callback, else force visibility via parameter.
            m_visible = m_visibility_callback();
        else if (!force_visibility && m_visibility_callback)
            m_visible = force_visibility;

    }

    void update_binds()
    {

        auto binds = get_all_binds();

        if (binds.size() > 0)
            for (auto bind : binds)
                bind->get_bind()->update_state(); // can't be const because this function modifies values...

    }

    void render_attachments()
    {

        for (const auto& attachment : m_attachments)
            attachment->render(ImGui::GetWindowPos(), ImGui::GetWindowSize(), ImGui::GetWindowDrawList());

    }

public:

    c_window(const std::string_view& window_name, const ImVec2 window_size = ImVec2(0, 0), const int window_flags = NULL, const bool default_visible = true)
    {

        this->m_name = window_name;
        this->m_flags = window_flags;
        this->m_visible = default_visible;
        this->m_size = window_size;
        this->m_dpi_scale = 1.f;

    }

    float get_dpi_scale() const { return m_dpi_scale; }
    void set_dpi_scale(float scale) { m_dpi_scale = scale; }

    bool get_visibility() const { return m_visible; }
    void set_visibility(const bool value) { m_visible = value; }

    void set_visibility_callback(const std::function<bool(void)>& value) { m_visibility_callback = value; }

    void set_start_render_callback(const std::function<void(void)>& value) noexcept { m_start_render_callback = value; }

    void set_end_render_callback(const std::function<void(void)>& value) noexcept { m_end_render_callback = value; }

    ImVec2 get_size() const { return m_size; }
    ImVec2 get_scaled_size() const { return m_size * m_dpi_scale; }

    ImVec2 get_pos() const { return m_pos; }

    void stop_animations() { }

    void render()
    {

        // mfw OOP wrapper around immediate mode ui lib...

        // update this window's visibility
        update_visibility();

        if (m_visible) [[likely]] // yessir...
        {

            start_window_render();

            m_pos = ImGui::GetWindowPos();

            render_attachments();

            render_elements();

            render_children();

            end_window_render();

        }

    }

    template <typename type, typename... args>
    type* insert_element(args... initializers)
    {

        type* allocated_element = new type(initializers...);

        m_elements.push_back((c_element*)allocated_element);

        return allocated_element;

    }

    bool insert_attachment(const std::function<void(const ImVec2&, const ImVec2&, ImDrawList*)>& fn)
    {

        c_attachment* allocated_attachment = new c_attachment(fn);

        m_attachments.push_back(allocated_attachment);

        return true;

    }

    c_child* insert_child(const std::string_view& child_name, const ImVec2& position, const ImVec2& size, bool has_header = true, bool abs_pos = false)
    {

        c_child* allocated_child = new c_child(child_name, position, size, this, abs_pos, has_header);

        m_children.push_back(allocated_child);

        return allocated_child;

    }

    c_child* find_child(const std::string_view& child_name)
    {

        for (const auto& child : m_children)
            if (child->get_name() == child_name) [[unlikely]]
                return child;

        return nullptr; // if we couldn't find the child

    }

    std::vector<c_child*> get_children() { return m_children; }

    template<typename type>
    type* find_element(const std::string_view& element_name)
    {

        for (const auto& element : m_elements)
            if (element->get_name() == element_name) [[unlikely]]
                return (type*)element; // we have to cast cause they're stored as c_element(s) but
                                       // if we're doing find element we prolly want the special properties...

        return nullptr; // if we couldn't find the element

    }

    template <typename type>
    std::vector<type*> find_elements_by_type(element_type _type)
    {

        std::vector<type*> ret{};

        for (auto element : m_elements)
            if (element->get_type() == _type)
                ret.push_back((type*)element);

        for (auto child : m_children)
            for (auto element : child->m_elements)
                if (element->get_type() == _type)
                    ret.push_back((type*)element);


        return ret;

    }

    bool pop_element(const std::string_view& element_label)
    {

        for (const auto& element : m_elements)
            if (element->get_label() == element_label) [[likely]]
            {

                const auto position = std::find(m_elements.begin(), m_elements.end(), element);

                // memory leak I think
                if (position != m_elements.end()) [[unlikely]]
                    m_elements.erase(position);

                return true;

            }

        return false;

    }

    size_t get_element_count() const { return m_elements.size(); }

};
