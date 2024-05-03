#pragma once

#include "element.h"

namespace framework
{

    static auto begin_combo = [&](const char* label, const char* preview_value, ImGuiComboFlags flags, float dpi_scale, c_animator<float>* animator, c_animator<ImVec4>* text_animator) -> bool {

        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = ImGui::GetCurrentWindow();

        ImGuiNextWindowDataFlags backup_next_window_data_flags = g.NextWindowData.Flags;
        g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume those values
        if (window->SkipItems)
            return false;

        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);
        IM_ASSERT((flags & (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)) != (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)); // Can't use both flags together

        const float arrow_size = (flags & ImGuiComboFlags_NoArrowButton) ? 0.0f : ImGui::GetFrameHeight();
        const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

        ImRect container_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(ImGui::GetWindowWidth() - 10.f, 48.f * dpi_scale));
        ImRect combo_bb(window->DC.CursorPos + ImVec2(34.f * dpi_scale, 19.f * dpi_scale), window->DC.CursorPos + ImVec2((34.f * dpi_scale) + (156.f * dpi_scale), 42.f * dpi_scale));

        ImGui::ItemSize(container_bb, style.FramePadding.y);
        if (!ImGui::ItemAdd(container_bb, id))
            return false;

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(combo_bb, id, &hovered, &held);
        const ImGuiID popup_id = ImHashStr("##ComboPopup", 0, id);
        bool popup_open = ImGui::IsPopupOpen(popup_id, ImGuiPopupFlags_None);
        if (pressed && !popup_open)
        {
            ImGui::OpenPopupEx(popup_id, ImGuiPopupFlags_None);
            popup_open = true;
        }

        const ImU32 frame_col = ImGui::GetColorU32(hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
        const float value_x2 = ImMax(combo_bb.Min.x, combo_bb.Max.x - arrow_size);
        
        if (!(flags & ImGuiComboFlags_NoArrowButton))
        {
            //ImU32 bg_col = ImGui::GetColorU32((popup_open || hovered) ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
            //ImU32 text_col = ImGui::GetColorU32(ImGuiCol_Text);
            //window->DrawList->AddRectFilled(ImVec2(value_x2, combo_bb.Min.y), combo_bb.Max, bg_col, style.FrameRounding, (container_bb.GetWidth() <= arrow_size) ? ImDrawFlags_RoundCornersAll : ImDrawFlags_RoundCornersRight);
            //if (value_x2 + arrow_size - style.FramePadding.x <= combo_bb.Max.x)
                //ImGui::RenderArrow(window->DrawList, ImVec2(value_x2 + style.FramePadding.y, combo_bb.Min.y + style.FramePadding.y), text_col, ImGuiDir_Down, 1.0f);
        }

        auto imgui_alpha = ImGui::GetStyle().Alpha;

        window->DrawList->AddRectFilled(container_bb.Min, container_bb.Max, IM_COL32(25, 20, 25, 255 * imgui_alpha));

        window->DrawList->AddRectFilledMultiColor(container_bb.Min, { container_bb.Max.x, container_bb.Min.y + 6.f }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0));
        window->DrawList->AddRectFilledMultiColor(container_bb.Max, { container_bb.Min.x, container_bb.Max.y - 6.f }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0));

        window->DrawList->AddRectFilledMultiColor(container_bb.Min, { container_bb.Min.x + 6.f, container_bb.Max.y }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0), IM_COL32(95, 92, 103, 200 * imgui_alpha));
        window->DrawList->AddRectFilledMultiColor(container_bb.Max, { container_bb.Max.x - 6.f, container_bb.Min.y }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0), IM_COL32(95, 92, 103, 200 * imgui_alpha));

        window->DrawList->AddRect(container_bb.Min, container_bb.Max, ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_AnimatedBorder)));

        window->DrawList->AddRectFilled(combo_bb.Min, combo_bb.Max, IM_COL32(25, 20, 25, 255 * imgui_alpha));

        animator->update_animation();
        text_animator->update_animation();

        auto border_color = ImGui::GetStyleColorVec4(ImGuiCol_Border);

        border_color.w = animator->get_value();

        //window->DrawList->AddRectFilled(combo_bb.Min, combo_bb.Max, IM_COL32(45, 45, 50, 60 * imgui_alpha));
        window->DrawList->AddRect(combo_bb.Min, combo_bb.Max, ImGui::GetColorU32(border_color));

        if (popup_open)
            animator->start_animation(true, false, animator->get_animation_percent() == 1.f ? true : false);
        else if (animator->get_animation_percent() == 1.f)
            animator->start_animation(false);

        if (ImGui::IsItemHovered())
            text_animator->start_animation(true, false, text_animator->get_animation_percent() == 1.f ? true : false);
        else if (text_animator->get_animation_percent() == 1.f)
            text_animator->start_animation(false);

        if (flags & ImGuiComboFlags_CustomPreview)
        {
            g.ComboPreviewData.PreviewRect = ImRect(combo_bb.Min.x, combo_bb.Min.y, value_x2, combo_bb.Max.y);
            IM_ASSERT(preview_value == NULL || preview_value[0] == 0);
            preview_value = NULL;
        }

        if (preview_value != NULL && !(flags & ImGuiComboFlags_NoPreview))
        {
            if (g.LogEnabled)
                ImGui::LogSetNextTextDecoration("{", "}");
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
            ImGui::RenderTextClipped(combo_bb.Min + (style.FramePadding * dpi_scale), ImVec2(value_x2, combo_bb.Max.y), preview_value, NULL, NULL);
            ImGui::PopStyleColor();
        }

        ImGui::PushStyleColor(ImGuiCol_Text, text_animator->get_value());
        if (label_size.x > 0)
            ImGui::RenderText(combo_bb.Min + ImVec2(0.f, -(2.f + label_size.y)), label);
        ImGui::PopStyleColor();

        if (!popup_open)
            return false;

        g.NextWindowData.Flags = backup_next_window_data_flags;

        return ImGui::BeginComboPopup(popup_id, combo_bb, flags | ImGuiComboFlags_HeightSmall);

    };

    static auto selectable_ex = [](const char* label, bool selected, ImGuiSelectableFlags flags, const ImVec2& size_arg, c_animator<float>* animator, c_animator<ImVec4>* text_animator, float dpi_scale) -> bool {

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;

        // Submit label or explicit size to ItemSize(), whereas ItemAdd() will submit a larger/spanning rectangle.
        ImGuiID id = window->GetID(label);
        ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
        ImVec2 size(size_arg.x != 0.0f ? size_arg.x : label_size.x, size_arg.y != 0.0f ? size_arg.y : label_size.y);
        ImVec2 pos = window->DC.CursorPos;
        pos.y += window->DC.CurrLineTextBaseOffset;
        ImGui::ItemSize(size, 0.0f);

        // Fill horizontal space
        // We don't support (size < 0.0f) in Selectable() because the ItemSpacing extension would make explicitly right-aligned sizes not visibly match other widgets.
        const bool span_all_columns = (flags & ImGuiSelectableFlags_SpanAllColumns) != 0;
        const float min_x = span_all_columns ? window->ParentWorkRect.Min.x : pos.x;
        const float max_x = span_all_columns ? window->ParentWorkRect.Max.x : window->WorkRect.Max.x;
        if (size_arg.x == 0.0f || (flags & ImGuiSelectableFlags_SpanAvailWidth))
            size.x = ImMax(label_size.x, max_x - min_x);

        // Text stays at the submission position, but bounding box may be extended on both sides
        ImVec2 text_min = pos;
        ImVec2 text_max(min_x + size.x, pos.y + size.y);

        // Selectables are meant to be tightly packed together with no click-gap, so we extend their box to cover spacing between selectable.
        ImRect bb(min_x, pos.y, text_max.x, text_max.y);
        if ((flags & ImGuiSelectableFlags_NoPadWithHalfSpacing) == 0)
        {
            const float spacing_x = span_all_columns ? 0.0f : style.ItemSpacing.x;
            const float spacing_y = style.ItemSpacing.y;
            const float spacing_L = IM_FLOOR(spacing_x * 0.50f);
            const float spacing_U = IM_FLOOR(spacing_y * 0.50f);
            bb.Min.x -= spacing_L;
            bb.Min.y -= spacing_U;
            bb.Max.x += (spacing_x - spacing_L);
            bb.Max.y += (spacing_y - spacing_U);
        }
        //if (g.IO.KeyCtrl) { GetForegroundDrawList()->AddRect(bb.Min, bb.Max, IM_COL32(0, 255, 0, 255)); }

        // Modify ClipRect for the ItemAdd(), faster than doing a PushColumnsBackground/PushTableBackground for every Selectable..
        const float backup_clip_rect_min_x = window->ClipRect.Min.x;
        const float backup_clip_rect_max_x = window->ClipRect.Max.x;
        if (span_all_columns)
        {
            window->ClipRect.Min.x = window->ParentWorkRect.Min.x;
            window->ClipRect.Max.x = window->ParentWorkRect.Max.x;
        }

        const bool disabled_item = (flags & ImGuiSelectableFlags_Disabled) != 0;
        const bool item_add = ImGui::ItemAdd(bb, id, NULL, disabled_item ? ImGuiItemFlags_Disabled : ImGuiItemFlags_None);
        if (span_all_columns)
        {
            window->ClipRect.Min.x = backup_clip_rect_min_x;
            window->ClipRect.Max.x = backup_clip_rect_max_x;
        }

        if (!item_add)
            return false;

        const bool disabled_global = (g.CurrentItemFlags & ImGuiItemFlags_Disabled) != 0;
        if (disabled_item && !disabled_global) // Only testing this as an optimization
            ImGui::BeginDisabled();

        // FIXME: We can standardize the behavior of those two, we could also keep the fast path of override ClipRect + full push on render only,
        // which would be advantageous since most selectable are not selected.
        if (span_all_columns && window->DC.CurrentColumns)
            ImGui::PushColumnsBackground();
        else if (span_all_columns && g.CurrentTable)
            ImGui::TablePushBackgroundChannel();

        // We use NoHoldingActiveID on menus so user can click and _hold_ on a menu then drag to browse child entries
        ImGuiButtonFlags button_flags = 0;
        if (flags & ImGuiSelectableFlags_NoHoldingActiveID) { button_flags |= ImGuiButtonFlags_NoHoldingActiveId; }
        if (flags & ImGuiSelectableFlags_NoSetKeyOwner) { button_flags |= ImGuiButtonFlags_NoSetKeyOwner; }
        if (flags & ImGuiSelectableFlags_SelectOnClick) { button_flags |= ImGuiButtonFlags_PressedOnClick; }
        if (flags & ImGuiSelectableFlags_SelectOnRelease) { button_flags |= ImGuiButtonFlags_PressedOnRelease; }
        if (flags & ImGuiSelectableFlags_AllowDoubleClick) { button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick; }
        if ((flags & ImGuiSelectableFlags_AllowOverlap) || (g.LastItemData.InFlags & ImGuiItemflags_AllowOverlap)) { button_flags |= ImGuiButtonFlags_AllowOverlap; }

        const bool was_selected = selected;
        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, button_flags);

        // Auto-select when moved into
        // - This will be more fully fleshed in the range-select branch
        // - This is not exposed as it won't nicely work with some user side handling of shift/control
        // - We cannot do 'if (g.NavJustMovedToId != id) { selected = false; pressed = was_selected; }' for two reasons
        //   - (1) it would require focus scope to be set, need exposing PushFocusScope() or equivalent (e.g. BeginSelection() calling PushFocusScope())
        //   - (2) usage will fail with clipped items
        //   The multi-select API aim to fix those issues, e.g. may be replaced with a BeginSelection() API.
        if ((flags & ImGuiSelectableFlags_SelectOnNav) && g.NavJustMovedToId != 0 && g.NavJustMovedToFocusScopeId == g.CurrentFocusScopeId)
            if (g.NavJustMovedToId == id)
                selected = pressed = true;

        // Update NavId when clicking or when Hovering (this doesn't happen on most widgets), so navigation can be resumed with gamepad/keyboard
        if (pressed || (hovered && (flags & ImGuiSelectableFlags_SetNavIdOnHover)))
        {
            if (!g.NavDisableMouseHover && g.NavWindow == window && g.NavLayer == window->DC.NavLayerCurrent)
            {
                ImGui::SetNavID(id, window->DC.NavLayerCurrent, g.CurrentFocusScopeId, ImGui::WindowRectAbsToRel(window, bb)); // (bb == NavRect)
                g.NavDisableHighlight = true;
            }
        }
        if (pressed)
            ImGui::MarkItemEdited(id);

        // In this branch, Selectable() cannot toggle the selection so this will never trigger.
        if (selected != was_selected) //-V547
            g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_ToggledSelection;

        // Render

        auto bar_position = text_min; bar_position.y += (label_size.y + 3.f); bar_position.x += animator->get_value();

        auto imgui_alpha = ImGui::GetStyle().Alpha;

        if (hovered)
        {

            window->DrawList->AddRectFilledMultiColor(bar_position, bar_position + ImVec2(animator->get_value() * (label_size.x * 0.45f), 1.f * dpi_scale), IM_COL32(130, 125, 150, 255 * imgui_alpha), IM_COL32(27, 27, 27, 10 * imgui_alpha), IM_COL32(27, 27, 27, 10 * imgui_alpha), IM_COL32(130, 125, 150, 255 * imgui_alpha));
            window->DrawList->AddRectFilledMultiColor(bar_position + ImVec2(0.f, 1.f * dpi_scale), bar_position + ImVec2(animator->get_value() * (label_size.x * 0.45f), 2.f * dpi_scale), IM_COL32(130, 125, 150, 255 * imgui_alpha), IM_COL32(27, 27, 27, 10 * imgui_alpha), IM_COL32(27, 27, 27, 10 * imgui_alpha), IM_COL32(130, 125, 150, 255 * imgui_alpha));

        }

        text_min.x += animator->get_value();
        text_max.x += animator->get_value();

        ImGui::PushStyleColor(ImGuiCol_Text, text_animator->get_value());
        ImGui::RenderTextClipped(text_min, text_max, label, NULL, &label_size, style.SelectableTextAlign, &bb);
        ImGui::PopStyleColor();

        if (span_all_columns && window->DC.CurrentColumns)
            ImGui::PopColumnsBackground();
        else if (span_all_columns && g.CurrentTable)
            ImGui::TablePopBackgroundChannel();

        // Automatically close popups
        if (pressed && (window->Flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiSelectableFlags_DontClosePopups) && !(g.LastItemData.InFlags & ImGuiItemFlags_SelectableDontClosePopup))
            ImGui::CloseCurrentPopup();

        if (disabled_item && !disabled_global)
            ImGui::EndDisabled();

        IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
        return pressed; //-V1020

    };

    static auto selectable = [&](const char* label, bool* p_selected, ImGuiSelectableFlags flags, c_animator<float>* animator, c_animator<ImVec4>* text_animator, float dpi_scale, const ImVec2& size_arg = { 0.f, 0.f }) -> bool {

        if (selectable_ex(label, *p_selected, flags, size_arg, animator, text_animator, dpi_scale))
        {

            *p_selected = !*p_selected;
            return true;

        }

        return false;

    };

}

class c_combo : public c_element
{

private:

    std::vector<std::string> m_combo_elements{};
    std::vector<c_animator<float>> m_selectable_animators{};
    std::vector<c_animator<ImVec4>> m_selectable_text_animators{};
    c_animator<float> m_hover_animator{ 0.f, 1.f, framework::animated_border_duration };
    c_animator<float> m_border_animator{ 0.4f, 1.f, 0.16f };
    c_animator<ImVec4> m_text_animator{ ImVec4(120 / 255.f, 120 / 255.f, 120 / 255.f, 1.f), ImVec4(1.f, 1.f, 1.f, 1.f), 0.43f };
    int m_value{};

public:

    inline c_combo(const std::string_view& label, const std::vector<std::string> combo_elements, const bool default_visibility = true) noexcept
    {

        this->m_label = label;
        this->m_visible = default_visibility;
        this->m_combo_elements = combo_elements;
        this->m_type = element_type::SINGLE_COMBO;
        this->m_flags = element_flags::WITH_VALUE;

        for (int i = 0; i < m_combo_elements.size(); i++)
        {

            m_selectable_animators.emplace_back<c_animator<float>>({ 0.f, 6.f, 0.21f });
            m_selectable_text_animators.emplace_back<c_animator<ImVec4>>({ ImVec4(230 / 255.f, 230 / 255.f, 230 / 255.f, 130 / 255.f), ImVec4(1.f, 1.f, 1.f, 1.f), 0.43f });

        }

    }

    virtual bool render(float dpi_scale)
    {

        if (m_value > m_combo_elements.size())
            return false;

        m_hover_animator.update_animation();

        ImGui::GetStyle().Colors[ImGuiCol_AnimatedBorder].w = m_hover_animator.get_value();

        if (const auto selected = framework::begin_combo(m_label.c_str(), m_combo_elements.at(m_value).c_str(), NULL, dpi_scale, &m_border_animator, &m_text_animator))
        {

            for (auto& animator : m_selectable_animators)
                animator.update_animation();

            for (auto& animator : m_selectable_text_animators)
                animator.update_animation();

            for (int i = 0; i < m_combo_elements.size(); i++)
            {

                bool was_selected = (m_value == i);
                bool pressed = false;

                if (framework::selectable(m_combo_elements.at(i).c_str(), &was_selected, ImGuiSelectableFlags_DontClosePopups, &m_selectable_animators.at(i), &m_selectable_text_animators.at(i), dpi_scale))
                {

                    if (m_value != i)
                    {

                        m_value = i;
                        pressed = true;

                    }

                }

                bool is_selected = (m_value == i);

                if (ImGui::IsItemHovered())
                    m_selectable_animators.at(i).start_animation(true, false, m_selectable_animators.at(i).get_animation_percent() == 1.f ? true : false);
                else if (m_selectable_animators.at(i).get_animation_percent() == 1.f)
                    m_selectable_animators.at(i).start_animation(false);

                if (pressed && is_selected || (m_selectable_text_animators.at(i).get_animation_percent() == 0.f && is_selected))
                    m_selectable_text_animators.at(i).start_animation(true);
                if (m_selectable_text_animators.at(i).get_animation_percent() == 1.f && !is_selected)
                    m_selectable_text_animators.at(i).start_animation(false);

            }

            ImGui::EndCombo();

            return selected;

        }

        ImGui::GetStyle().Colors[ImGuiCol_AnimatedBorder].w = (200 / 255.f);

        if (ImGui::IsItemHovered())
            m_hover_animator.start_animation(true, false, m_hover_animator.get_animation_percent() == 1.f ? true : false);
        else if (m_hover_animator.get_animation_percent() == 1.f)
            m_hover_animator.start_animation(false);

        if (m_has_tooltip && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
        {

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(230 / 255.f, 230 / 255.f, 230 / 255.f, 130 / 255.f));
            ImGui::SetTooltip(m_tooltip.c_str());
            ImGui::PopStyleColor();

        }

        return false;

    }

    int get_value() const noexcept { return m_value; }
    void set_value(int value) noexcept { m_value = value; }

};

class c_multi_combo : public c_element
{

private:

    std::vector<std::string> m_combo_elements{};
    std::vector<BYTE> m_values{};
    std::vector<c_animator<float>> m_selectable_animators{};
    std::vector<c_animator<ImVec4>> m_selectable_text_animators{};
    c_animator<float> m_hover_animator{ 0.f, 1.f, framework::animated_border_duration };
    c_animator<float> m_border_animator{ 0.4f, 1.f, 0.16f };
    c_animator<ImVec4> m_text_animator{ ImVec4(120 / 255.f, 120 / 255.f, 120 / 255.f, 1.f), ImVec4(1.f, 1.f, 1.f, 1.f), 0.43f };
        
public:

    inline c_multi_combo(const std::string_view& label, const std::vector<std::string> combo_elements, const bool default_visibility = true) noexcept
    {

        this->m_label = label;
        this->m_visible = default_visibility;
        this->m_combo_elements = combo_elements;
        this->m_type = element_type::MULTI_COMBO;
        this->m_flags = element_flags::WITH_VALUE;

        for (int i = 0; i < m_combo_elements.size(); i++)
        {

            m_selectable_animators.emplace_back<c_animator<float>>({ 0.f, 6.f, 0.21f });
            m_selectable_text_animators.emplace_back<c_animator<ImVec4>>({ ImVec4(230 / 255.f, 230 / 255.f, 230 / 255.f, 130 / 255.f), ImVec4(1.f, 1.f, 1.f, 1.f), 0.43f });

        }

        if (m_values.size() != m_combo_elements.size())
            m_values.resize(m_combo_elements.size());
        
    }

    BYTE get_value(int idx) const noexcept { return m_values.at(idx); }
    void set_value(int idx, BYTE value) noexcept { m_values.at(idx) = value; }

    std::vector<BYTE> get_values() const noexcept { return m_values; }

    int get_active_values() const
    {

        int active_values = 0;

        for (const auto value : m_values)
            if (value) ++active_values; // true = active, false = inactive (duh)

        return active_values;

    }

    virtual bool render(float dpi_scale)
    {

        m_hover_animator.update_animation();

       ImGui::GetStyle().Colors[ImGuiCol_AnimatedBorder].w = m_hover_animator.get_value();

        if (const auto selected = framework::begin_combo(m_label.c_str(), (std::to_string(get_active_values()) + "/" + std::to_string(m_values.size()) + " selected").c_str(), NULL, dpi_scale, &m_border_animator, &m_text_animator))
        {

            for (auto& animator : m_selectable_animators)
                animator.update_animation();

            for (auto& animator : m_selectable_text_animators)
                animator.update_animation();

            for (int i = 0; i < m_combo_elements.size(); i++)
            {

                bool was_selected = (bool)m_values.at(i);

                auto pressed = framework::selectable(m_combo_elements.at(i).c_str(), (bool*)&m_values.at(i), ImGuiSelectableFlags_DontClosePopups, &m_selectable_animators.at(i), &m_selectable_text_animators.at(i), dpi_scale);

                bool is_selected = (bool)m_values.at(i);

                if (ImGui::IsItemHovered())
                    m_selectable_animators.at(i).start_animation(true, false, m_selectable_animators.at(i).get_animation_percent() == 1.f ? true : false);
                else if (m_selectable_animators.at(i).get_animation_percent() == 1.f)
                    m_selectable_animators.at(i).start_animation(false);

                if (pressed && is_selected || (m_selectable_text_animators.at(i).get_animation_percent() == 0.f && is_selected))
                    m_selectable_text_animators.at(i).start_animation(true);
                if (m_selectable_text_animators.at(i).get_animation_percent() == 1.f && !is_selected)
                    m_selectable_text_animators.at(i).start_animation(false);

            }

            ImGui::EndCombo();

            return selected;

        }

        ImGui::GetStyle().Colors[ImGuiCol_AnimatedBorder].w = 200 * 255.f;

        if (ImGui::IsItemHovered())
            m_hover_animator.start_animation(true, false, m_hover_animator.get_animation_percent() == 1.f ? true : false);
        else if (m_hover_animator.get_animation_percent() == 1.f)
            m_hover_animator.start_animation(false);

        if (m_has_tooltip && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
        {

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(230 / 255.f, 230 / 255.f, 230 / 255.f, 130 / 255.f));
            ImGui::SetTooltip(m_tooltip.c_str());
            ImGui::PopStyleColor();

        }

        return false;

    }

};
