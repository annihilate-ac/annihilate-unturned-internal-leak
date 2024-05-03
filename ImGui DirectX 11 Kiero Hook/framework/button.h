#pragma once

#include "includes.h"

#include "element.h"

namespace framework
{

    static auto button_ex = [&](const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags, float dpi_scale, c_animator<float>* animator, c_animator<ImVec4>* text_animator) -> bool {

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

        ImRect container_bb(pos, pos + ImVec2(ImGui::GetWindowWidth() - 10.f, 36.f * dpi_scale));
        ImRect button_bb(pos + ImVec2(34.f * dpi_scale, 6.f * dpi_scale), window->DC.CursorPos + ImVec2(190.f * dpi_scale, 30.f * dpi_scale));

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
        text_animator->update_animation();

        window->DrawList->AddRectFilled(button_bb.Min, button_bb.Max, IM_COL32(25, 20, 25, 255 * imgui_alpha));

        auto border_color = ImGui::GetStyleColorVec4(ImGuiCol_Border);

        border_color.w = animator->get_value();

        window->DrawList->AddRect(button_bb.Min, button_bb.Max, ImGui::GetColorU32(border_color));

        if (pressed || (hovered && ImGui::GetIO().MouseDown[0]))
            animator->start_animation(true, false, animator->get_animation_percent() == 1.f ? true : false);
        else if (animator->get_animation_percent() == 1.f)
            animator->start_animation(false);

        if (ImGui::IsItemHovered())
            text_animator->start_animation(true, false, text_animator->get_animation_percent() == 1.f ? true : false);
        else if (text_animator->get_animation_percent() == 1.f)
            text_animator->start_animation(false);

        if (g.LogEnabled)
            ImGui::LogSetNextTextDecoration("[", "]");

        ImGui::PushStyleColor(ImGuiCol_Text, text_animator->get_value());
        ImGui::RenderTextClipped(button_bb.Min + style.FramePadding, button_bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign);
        ImGui::PopStyleColor();

        // Automatically close popups
        //if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
        //    CloseCurrentPopup();

        IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
        return pressed;

    };

}

class c_button : public c_element
{

private:

    c_animator<float> m_hover_animator{ 0.f, 1.f, framework::animated_border_duration };
    c_animator<float> m_button_animator{ 0.4f, 1.f, 0.16f };
    c_animator<ImVec4> m_text_animator{ ImVec4(230 / 255.f, 230 / 255.f, 230 / 255.f, 130 / 255.f), ImVec4(1.f, 1.f, 1.f, 1.f), 0.43f };
    ImVec2 m_size{};

public:

    c_button(const std::string_view& label, const ImVec2& size = ImVec2(0, 0), const bool default_visibility = true)
    {

        this->m_label = label;
        this->m_size = size;
        this->m_visible = default_visibility;
        this->m_type = element_type::BUTTON;

    }

    virtual bool render(float dpi_scale)
    {

        m_hover_animator.update_animation();

        ImGui::GetStyle().Colors[ImGuiCol_AnimatedBorder].w = m_hover_animator.get_value();

        auto ret = framework::button_ex(m_label.c_str(), m_size, NULL, dpi_scale, &m_button_animator, &m_text_animator);

        ImGui::GetStyle().Colors[ImGuiCol_AnimatedBorder].w = 200 * 255.f;

        if (ImGui::IsItemHovered())
            m_hover_animator.start_animation(true, false, m_hover_animator.get_animation_percent() == 1.f ? true : false);
        else if (m_hover_animator.get_animation_percent() == 1.f)
            m_hover_animator.start_animation(false);

        return ret;

    }

};
