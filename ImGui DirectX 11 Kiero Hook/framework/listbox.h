#pragma once

#include "element.h"

class c_multi_listbox : public c_element
{

private:

    ImVec2 m_size{};
    std::vector<std::string> m_items{};
    std::vector<BYTE> m_values{};

public:

    inline c_multi_listbox(const std::string_view& label, const std::vector<std::string>& items, const bool default_visibility = true) noexcept
    {

        this->m_label = label;
        this->m_visible = default_visibility;
        this->m_items = items;
        this->m_type = element_type::MULTI_LISTBOX;
        this->m_flags = element_flags::WITH_VALUE;

    }

    virtual bool render(float dpi_scale)
    {

        m_size.x = ImGui::GetContentRegionAvail().x;

        if (m_values.size() != m_items.size())
            m_values.resize(m_items.size());

        ImGui::Text(m_label.c_str());

        if (const bool selected = ImGui::BeginListBox(std::string("##vis_" + m_label).c_str(), m_size))
        {

            ImGui::Spacing();

            for (int i = 0; i < m_items.size(); i++)
                ImGui::Selectable(m_items[i].c_str(), (bool*)&m_values.at(i), ImGuiSelectableFlags_SpanAvailWidth);

            ImGui::EndListBox();

            return selected;

        }

        return false;

    }

    BYTE get_value(int idx) const noexcept { return m_values.at(idx); }
    void set_value(int idx, BYTE value) noexcept { m_values.at(idx) = value; }

};
