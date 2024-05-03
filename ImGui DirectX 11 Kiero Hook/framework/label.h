#pragma once

#include "element.h"

class c_label : public c_element
{

private:

    bool m_center_x{};

public:

    c_label(const std::string& label, const bool default_visibility = true)
    {

        this->m_label = label;
        this->m_visible = default_visibility;
        this->m_type = element_type::LABEL;

    }

    std::string get_text() const { return m_label; }
    void set_text(const std::string& value) { m_label = value; }

    virtual bool render(float dpi_scale)
    { 
        
        auto text_sz = ImGui::CalcTextSize(m_label.c_str());
        if (m_center_x)
            ImGui::SetCursorPosX((ImGui::GetWindowWidth() - text_sz.x) / 2.f);

        ImGui::Text(m_label.c_str()); 

        return false; /* return false cause how a label gon return true... */

    }

    void set_custom_font(ImFont* font) { m_custom_font = font; }
    void set_center_x(bool value) { m_center_x = value; }

};

class c_color_label : public c_element
{

private:

    ImVec4 m_col{};

public:

    c_color_label(const std::string& label, ImVec4 color = ImVec4(1.f, 1.f, 1.f, 1.f), const bool default_visibility = true)
    {

        this->m_label = label;
        this->m_visible = default_visibility;
        this->m_type = element_type::LABEL;
        this->m_col = color;

    }

    std::string get_text() const { return m_label; }
    void set_text(const std::string& value) { m_label = value; }

    virtual bool render(float dpi_scale) { ImGui::TextColored(m_col, m_label.c_str()); return false; /* return false cause how a label gon return true... */ }

    void set_custom_font(ImFont* font) { m_custom_font = font; }

};

class c_gradient_label : public c_element
{

private:

    ImU32 m_first_col{};
    ImU32 m_second_col{};
    float m_smooth{};

public:

    c_gradient_label(const std::string& label, ImU32 first_color, ImU32 second_color, float smoothing = 175.f, const bool default_visibility = true)
    {

        this->m_label = label;
        this->m_visible = default_visibility;
        this->m_type = element_type::LABEL;
        this->m_first_col = first_color;
        this->m_second_col = second_color;
        this->m_smooth = smoothing;

    }

    std::string get_text() const { return m_label; }
    void set_text(const std::string& value) { m_label = value; }

    virtual bool render(float dpi_scale) { ImGui::TextGradiented(m_label.c_str(), m_first_col, m_second_col, m_smooth); return false; /* return false cause how a label gon return true... */ }

    void set_custom_font(ImFont* font) { m_custom_font = font; }

};
