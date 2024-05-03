#pragma once

#include "element.h"
#include "../../scripting.h"

namespace framework
{

    static auto selectable_ex3 = [](const char* label, bool selected, ImGuiSelectableFlags flags, const ImVec2& size_arg, c_animator<float>* animator, float dpi_scale) -> bool {

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

        if (selected) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(130 / 255.f, 125 / 255.f, 150 / 255.f, 245 / 255.f));
        ImGui::RenderTextClipped(text_min, text_max, label, NULL, &label_size, style.SelectableTextAlign, &bb);
        if (selected) ImGui::PopStyleColor();

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

    static auto button_ex2 = [&](const char* label, const ImVec2& pos, const ImVec2& size_arg, ImGuiButtonFlags flags, float dpi_scale, c_animator<float>* animator, c_animator<ImVec4>* text_animator) -> bool {

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);
        const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

        ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

        ImRect button_bb(pos + ImVec2(30.f * dpi_scale, 6.f * dpi_scale), pos + ImVec2(90.f * dpi_scale, 30.f * dpi_scale));

        //ImGui::ItemSize(button_bb, style.FramePadding.y);
        //if (!ImGui::ItemAdd(button_bb, id))
        //    return false;

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(button_bb, id, &hovered, &held, ImGuiButtonFlags_AllowOverlap); pressed = hovered && ImGui::GetIO().MouseDown[0];

        // Render
        const ImU32 col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
        //RenderNavHighlight(bb, id);
        //RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);

        auto imgui_alpha = ImGui::GetStyle().Alpha;

        animator->update_animation();
        text_animator->update_animation();

        window->DrawList->AddRectFilled(button_bb.Min, button_bb.Max, IM_COL32(25, 20, 25, 255 * imgui_alpha));

        auto border_color = ImGui::GetStyleColorVec4(ImGuiCol_Border);

        border_color.w = animator->get_value();

        window->DrawList->AddRect(button_bb.Min, button_bb.Max, ImGui::GetColorU32(border_color));

        if (pressed)
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

__forceinline const char* tm_to_readable_time(tm ctx) {
    char buffer[80];

    strftime(buffer, sizeof(buffer), "%m-%d-%y", &ctx);

    return buffer;
}

__forceinline std::time_t string_to_timet(const char* timestamp) {
    auto cv = strtol(timestamp, NULL, 10); // long

    return (time_t)cv;
}

__forceinline std::tm timet_to_tm(time_t timestamp) {
    std::tm* context;

    context = localtime(&timestamp);

    if (!context)
        return {};

    return *context;
}

class c_chip_panel : public c_element
{

private:

    std::string m_description{};
    std::vector<c_animator<float>> m_selectable_animators{};
    c_animator<float> m_hover_animator{ 0.f, 1.f, framework::animated_border_duration };
    c_animator<float> m_button_animator{ 0.4f, 1.f, 0.16f };
    c_animator<ImVec4> m_text_animator{ ImVec4(120 / 255.f, 120 / 255.f, 120 / 255.f, 1.f), ImVec4(1.f, 1.f, 1.f, 1.f), 0.43f };
    script_t* m_attached_script{};
    int* m_id_selection{};
    int m_id{};

public:

    void set_attached_script(script_t* script) { m_attached_script = script; }

    inline c_chip_panel(const std::string_view& label, const std::string_view& description, int id, int* id_selection, const bool default_visibility = true) noexcept
    {

        this->m_label = label;
        this->m_description = description;
        this->m_visible = default_visibility;
        this->m_id_selection = id_selection;
        this->m_id = id;

        for (int i = 0; i < 2; i++)
            m_selectable_animators.emplace_back<c_animator<float>>({ 0.f, 6.f, 0.3f });

    }

    virtual bool render(float dpi_scale)
    {

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(m_label.c_str());
        const ImVec2 label_size = ImGui::CalcTextSize(m_label.c_str(), NULL, true);
        const ImVec2 pos = window->DC.CursorPos;

        ImRect container_bb(pos, pos + ImVec2(ImGui::GetWindowWidth() - 10.f, 48.f * dpi_scale));

        ImGui::ItemSize(container_bb, style.FramePadding.y);
        if (!ImGui::ItemAdd(container_bb, id))
            return false;

        m_hover_animator.update_animation();

        auto imgui_alpha = ImGui::GetStyle().Alpha;

        window->DrawList->AddRectFilled(container_bb.Min, container_bb.Max, IM_COL32(25, 20, 25, 255 * imgui_alpha));

        window->DrawList->AddRectFilledMultiColor(container_bb.Min, { container_bb.Max.x, container_bb.Min.y + 6.f }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0));
        window->DrawList->AddRectFilledMultiColor(container_bb.Max, { container_bb.Min.x, container_bb.Max.y - 6.f }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0));

        window->DrawList->AddRectFilledMultiColor(container_bb.Min, { container_bb.Min.x + 6.f, container_bb.Max.y }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0), IM_COL32(95, 92, 103, 200 * imgui_alpha));
        window->DrawList->AddRectFilledMultiColor(container_bb.Max, { container_bb.Max.x - 6.f, container_bb.Min.y }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0), IM_COL32(95, 92, 103, 200 * imgui_alpha));

        auto bdr_clr = ImGui::GetStyleColorVec4(ImGuiCol_AnimatedBorder);

        window->DrawList->AddRect(container_bb.Min, container_bb.Max, ImGui::GetColorU32(ImVec4(bdr_clr.x, bdr_clr.y, bdr_clr.z, m_hover_animator.get_value())));

        ImVec2 top_text_pos = ImVec2(container_bb.Min.x + 10.f, container_bb.Min.y + (7.f * dpi_scale));
        ImGui::RenderText(top_text_pos, m_label.c_str());

        auto is_script = m_attached_script != nullptr;

        auto description = m_description;

        if (!is_script)
        {

            description = std::string("last edited: ");

            if (m_description != "-1")
                description += tm_to_readable_time(timet_to_tm(string_to_timet(m_description.c_str())));
            else
                description += "unknown";

        }
        else
            description = m_attached_script->state == STATE_RUNNING ? "running" : "idle";

        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        ImVec2 bot_text_pos = ImVec2(container_bb.Min.x + 10.f, container_bb.Min.y + (25.f * dpi_scale));
        ImGui::RenderText(bot_text_pos, description.c_str());
        ImGui::PopStyleColor();

        auto pressed = framework::button_ex2((std::string("select##") + m_label).c_str(), { container_bb.Min.x + 129.f, container_bb.Min.y + 6.f }, { 60.f, 14.f }, ImGuiButtonFlags_None, dpi_scale, &m_button_animator, &m_text_animator);

        if (pressed)
        {

            if (is_script)
                ImGui::OpenPopup(std::string(m_label + "##popup").c_str(), ImGuiPopupFlags_None);
            else
                *m_id_selection = m_id;

        }

        if (ImGui::BeginPopup(std::string(m_label + "##popup").c_str()) && m_attached_script)
        {

            bool selected = false;

            for (auto& animator : m_selectable_animators)
                animator.update_animation();

            ImGui::BeginGroup();

            if (m_attached_script->state == STATE_RUNNING)
            {

                if (framework::selectable_ex3("unload", &selected, ImGuiSelectableFlags_None, { 0.f, 0.f }, &m_selectable_animators.at(0), dpi_scale))
                    if (g_script_engine->unload_script(m_id))
                        m_description = "idle";

                if (ImGui::IsItemHovered())
                    m_selectable_animators.at(0).start_animation(true, false, m_selectable_animators.at(0).get_animation_percent() == 1.f ? true : false);
                else if (m_selectable_animators.at(0).get_animation_percent() == 1.f)
                    m_selectable_animators.at(0).start_animation(false);

            }
            else
            {

                if (framework::selectable_ex3("load", &selected, ImGuiSelectableFlags_None, { 0.f, 0.f }, &m_selectable_animators.at(1), dpi_scale))
                    if (g_script_engine->run_script(m_id))
                        m_description = "running";

                if (ImGui::IsItemHovered())
                    m_selectable_animators.at(1).start_animation(true, false, m_selectable_animators.at(1).get_animation_percent() == 1.f ? true : false);
                else if (m_selectable_animators.at(1).get_animation_percent() == 1.f)
                    m_selectable_animators.at(1).start_animation(false);

            }

            ImGui::EndGroup();

            ImGui::EndPopup();

        }

        if (ImGui::IsItemHovered())
            m_hover_animator.start_animation(true, false, m_hover_animator.get_animation_percent() == 1.f ? true : false);
        else if (m_hover_animator.get_animation_percent() == 1.f)
            m_hover_animator.start_animation(false);

        return pressed;

    }

};