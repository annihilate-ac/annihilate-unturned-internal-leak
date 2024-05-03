#pragma once

#include "element.h"

#include "animator.h"

class c_click_label : public c_element
{

private:

    c_animator<float> m_animator{0.f, 6.f, 0.21f};
    c_animator<ImVec4> m_text_animator{ ImVec4(120 / 255.f, 120 / 255.f, 120 / 255.f, 1.f), ImVec4(1.f, 1.f, 1.f, 1.f), 0.43f };
    int* m_current_tab{};
    int m_this_tab{};

public:

    inline c_click_label(const std::string_view& label, int this_tab, int* current_tab, bool default_visibility = true)
    {

        this->m_label = label;
        this->m_visible = default_visibility;
        this->m_type = element_type::CLICK_LABEL;
        this->m_current_tab = current_tab;
        this->m_this_tab = this_tab;

        if (this_tab == *current_tab)
            m_text_animator.start_animation(true);

    }

    virtual bool render(float dpi_scale)
    {

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        char Buf[64];
        _snprintf(Buf, 62, "%s", m_label.c_str());

        char getid[128];
        _snprintf(getid, 128, "%s%s", m_label.c_str(), std::to_string(m_this_tab).c_str());

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(getid);
        ImGui::PushFont(framework::bold_font);
        const ImVec2 label_size = ImGui::CalcTextSize(m_label.c_str(), NULL, true);
        ImGui::PopFont();

        const ImRect check_bb(window->DC.CursorPos, ImVec2(label_size.y + style.FramePadding.y * 2 + window->DC.CursorPos.x, window->DC.CursorPos.y + label_size.y + style.FramePadding.y * 2));
        ImGui::ItemSize(check_bb, style.FramePadding.y);

        ImRect total_bb = check_bb;

        if (label_size.x > 0)
        {
            ImGui::SameLine(0, style.ItemInnerSpacing.x);
            const ImRect text_bb(ImVec2(window->DC.CursorPos.x, window->DC.CursorPos.y + style.FramePadding.y), ImVec2(window->DC.CursorPos.x + label_size.x, window->DC.CursorPos.y + style.FramePadding.y + label_size.y));

            ImGui::ItemSize(ImVec2(text_bb.GetWidth(), check_bb.GetHeight()), style.FramePadding.y);
            total_bb = ImRect(ImMin(check_bb.Min, text_bb.Min), ImMax(check_bb.Max, text_bb.Max));
        }

        if (!ImGui::ItemAdd(total_bb, id))
            return false;

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
        auto is_current_tab = *m_current_tab == m_this_tab;
        if (is_current_tab)
            pressed = false;
        auto label_position = check_bb.GetTL(); label_position.x += m_animator.get_value(); label_position.y += 1.f;
        auto bar_position = check_bb.Min; bar_position.y += (label_size.y + 3.f); bar_position.x += m_animator.get_value();
        auto imgui_alpha = ImGui::GetStyle().Alpha;

        m_animator.update_animation();

        m_text_animator.update_animation();

        if (m_animator.get_animation_percent() != 1.f && hovered) m_animator.start_animation(true);
        else if (!hovered && m_animator.get_animation_percent() == 1.f) m_animator.start_animation(false);

        if (pressed)
            *m_current_tab = m_this_tab;

        bool cur_tab = *m_current_tab == m_this_tab;

        if (pressed && cur_tab)
            m_text_animator.start_animation(true);
        if (m_text_animator.get_animation_percent() == 1.f && !is_current_tab)
            m_text_animator.start_animation(false);

        ImGui::PushFont(framework::bold_font);

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 255.f / 255.f));

        ImGui::RenderText(label_position + ImVec2(-2.f, 2.f), m_label.c_str());

        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, m_text_animator.get_value());

        ImGui::RenderText(label_position, m_label.c_str());

        ImGui::PopStyleColor();

        ImGui::PopFont();

        ImGui::GetCurrentWindow()->DrawList->AddRectFilledMultiColor(bar_position, bar_position + ImVec2(m_animator.get_value() * (label_size.x * 0.15f), 1.f * dpi_scale), IM_COL32(130, 125, 150, 255 * imgui_alpha), IM_COL32(27, 27, 27, 10 * imgui_alpha), IM_COL32(27, 27, 27, 10 * imgui_alpha), IM_COL32(130, 125, 150, 255 * imgui_alpha));
        ImGui::GetCurrentWindow()->DrawList->AddRectFilledMultiColor(bar_position + ImVec2(0.f, 1.f * dpi_scale), bar_position + ImVec2(m_animator.get_value() * (label_size.x * 0.15f), 2.f * dpi_scale), IM_COL32(130, 125, 150, 255 * imgui_alpha), IM_COL32(27, 27, 27, 10 * imgui_alpha), IM_COL32(27, 27, 27, 10 * imgui_alpha), IM_COL32(130, 125, 150, 255 * imgui_alpha));

        return pressed;

    }

};

// very similar to a button, except it toggles l0l
class c_tab : public c_element
{

private:

    ImVec2 m_size{};
    c_animator<ImVec4> m_hover_animator{ ImGui::GetStyleColorVec4(ImGuiCol_FrameBg), ImGui::GetStyleColorVec4(ImGuiCol_FrameBgHovered), 0.6f };
    std::string m_description{};
    std::string m_icon{};
    bool m_show_name{};
    int* m_current_tab{};
    float* m_dpi_scale{};
    int m_this_tab{};

public:

    inline c_tab(const std::string_view& label, const std::string_view& desc, const std::string_view& icon, const int this_tab, int* current_tab, float* dpi_scale = nullptr, const bool show_name = false, const bool default_visibility = true) noexcept
    {

        this->m_label = label;
        this->m_visible = default_visibility;
        this->m_type = element_type::TAB;
        this->m_current_tab = current_tab;
        this->m_this_tab = this_tab;
        this->m_show_name = show_name;
        this->m_icon = icon;
        this->m_description = desc;
        this->m_dpi_scale = dpi_scale;

    }

    virtual bool render()
    {

        static auto custom_button = [](const char* label, const char* desc, ImVec2 esize, bool tabbed, const char* icon, bool show_name, const ImVec4& frame_color) -> bool
        {
            ImGuiWindow* window = ImGui::GetCurrentWindow();
            int flags = 0;
            ImVec2 size_arg = esize;
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

            const ImRect bb(pos, pos + size);
            ImGui::ItemSize(size, style.FramePadding.y);
            if (!ImGui::ItemAdd(bb, id))
                return false;

            if (g.LastItemData.InFlags & ImGuiItemFlags_ButtonRepeat)
                flags |= ImGuiButtonFlags_Repeat;

            bool hovered, held;
            bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

            const int imgui_alpha = ImGui::GetStyle().Alpha * 255;

            if (tabbed || hovered)
            {

                window->DrawList->AddRectFilled(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(frame_color), 3.f);

                if (tabbed)
                {

                    ImGui::GetForegroundDrawList()->AddRectFilled(bb.Min + ImVec2{ -10.f, 13.f }, bb.Min + ImVec2{ -10.f, 36.f }, IM_COL32(52, 50, 168, imgui_alpha), 6.f);
                    ImGui::GetForegroundDrawList()->AddLine(bb.Min + ImVec2{ -9.f, 14.f }, bb.Min + ImVec2{ -9.f, 35.f }, IM_COL32(52, 50, 168, std::clamp(150, 0, imgui_alpha)));
                    ImGui::GetForegroundDrawList()->AddLine(bb.Min + ImVec2{ -9.f, 16.f }, bb.Min + ImVec2{ -9.f, 33.f }, IM_COL32(52, 50, 168, imgui_alpha));

                }

            }

            const ImRect icon_bb(bb.Min + ImVec2{ 9.f, 9.f }, bb.Min + ImVec2{ 40.f, 40.f });

            window->DrawList->AddRectFilled(icon_bb.Min, icon_bb.Max, tabbed ? IM_COL32(40, 40, 40, imgui_alpha) : IM_COL32(30, 30, 30, imgui_alpha), 3.f);

            window->DrawList->AddText(NULL, 15.f, icon_bb.GetCenter() - (ImGui::CalcTextSize(icon) / 2.f), tabbed ? IM_COL32(190, 190, 190, imgui_alpha) : IM_COL32(160, 160, 160, imgui_alpha), icon);

            window->DrawList->AddText(framework::bold_font, 12.f, icon_bb.Min + ImVec2{ 40.f, 3.f }, IM_COL32(160, 160, 160, imgui_alpha), label);
            window->DrawList->AddText(framework::bold_font, 12.f, icon_bb.Min + ImVec2{ 40.f, 16.f }, IM_COL32(103, 103, 103, imgui_alpha), desc);

            IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
            return pressed;
        };

        m_hover_animator.update_animation();

        auto frame_col = m_hover_animator.get_value();

        frame_col.w = ImGui::GetStyle().Alpha;

        const auto ret = custom_button(m_label.c_str(), m_description.c_str(), { 165.f, 48.f }, (*m_current_tab == m_this_tab), m_icon.c_str(), m_show_name, frame_col);

        if (!ImGui::IsItemHovered())
            m_hover_animator.start_animation(true, true);

        if (ret) *m_current_tab = m_this_tab;

        return ret;

    }

};

class c_sub_tab
{



};
