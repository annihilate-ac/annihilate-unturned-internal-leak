#pragma once

#ifdef ENABLE_BINDS

// this is going to be a mess

#include "includes.h"
#include "element.h"

#include <Windows.h>

namespace framework
{

    static auto selectable_ex2 = [](const char* label, bool selected, ImGuiSelectableFlags flags, const ImVec2& size_arg, c_animator<float>* animator, c_animator<ImVec4>* text_animator, float dpi_scale) -> bool {

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

}

enum keybind_display_type : uint16_t
{

    WITH_LABEL,
    WITHOUT_LABEL

};

enum keybind_interaction_type : uint16_t
{

    TOGGLE,
    HELD,
    ALWAYS,
    NEVER

};

const char* const key_names[] = {
    "Unknown",
    "LBUTTON",
    "RBUTTON",
    "CANCEL",
    "MBUTTON",
    "XBUTTON1",
    "XBUTTON2",
    "Unknown",
    "BACK",
    "TAB",
    "Unknown",
    "Unknown",
    "CLEAR",
    "RETURN",
    "Unknown",
    "Unknown",
    "SHIFT",
    "CONTROL",
    "MENU",
    "PAUSE",
    "CAPITAL",
    "KANA",
    "Unknown",
    "JUNJA",
    "FINAL",
    "KANJI",
    "Unknown",
    "ESCAPE",
    "CONVERT",
    "NONCONVERT",
    "ACCEPT",
    "MODECHANGE",
    "SPACE",
    "PRIOR",
    "NEXT",
    "END",
    "HOME",
    "LEFT",
    "UP",
    "RIGHT",
    "DOWN",
    "SELECT",
    "PRINT",
    "EXECUTE",
    "SNAPSHOT",
    "INSERT",
    "DELETE",
    "HELP",
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z",
    "LWIN",
    "RWIN",
    "APPS",
    "Unknown",
    "SLEEP",
    "NUMPAD0",
    "NUMPAD1",
    "NUMPAD2",
    "NUMPAD3",
    "NUMPAD4",
    "NUMPAD5",
    "NUMPAD6",
    "NUMPAD7",
    "NUMPAD8",
    "NUMPAD9",
    "MULTIPLY",
    "ADD",
    "SEPARATOR",
    "SUBTRACT",
    "DECIMAL",
    "DIVIDE",
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
    "F6",
    "F7",
    "F8",
    "F9",
    "F10",
    "F11",
    "F12",
    "F13",
    "F14",
    "F15",
    "F16",
    "F17",
    "F18",
    "F19",
    "F20",
    "F21",
    "F22",
    "F23",
    "F24",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "NUMLOCK",
    "SCROLL",
    "OEM_NEC_EQUAL",
    "OEM_FJ_MASSHOU",
    "OEM_FJ_TOUROKU",
    "OEM_FJ_LOYA",
    "OEM_FJ_ROYA",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "LSHIFT",
    "RSHIFT",
    "LCONTROL",
    "RCONTROL",
    "LMENU",
    "RMENU"
};

// class designed to represent a bindable key on the keyboard
class c_binding
{

private:

    friend class c_keybind; // I am so sorry.

    std::string m_name{};
    keybind_interaction_type m_type{ keybind_interaction_type::NEVER };
    int m_current_key{ NULL };
    bool m_enabled{ false };
    bool m_waiting_for_input{ false };
    bool m_was_key_down{ false };

    bool bind_new_key()
    {

        // if this errors replace it with "IsKeyPressedMap"
        if (ImGui::IsKeyPressedMap(ImGuiKey_Escape))
        {

            m_current_key = NULL;
            ImGui::ClearActiveID(); // if this errors, replace with "ClearActiveID"
            return true;

        }

        for (auto i = 1; i < 5; i++)
        {

            if (ImGui::GetIO().MouseDown[i])
            {

                switch (i)
                {
                case 1:
                    m_current_key = VK_RBUTTON;
                    break;
                case 2:
                    m_current_key = VK_MBUTTON;
                    break;
                case 3:
                    m_current_key = VK_XBUTTON1;
                    break;
                case 4:
                    m_current_key = VK_XBUTTON2;
                    break;
                }
                return true;

            }

        }

        //if (ImGui::IsKeyPressedMap(ImGuiKey_MouseLeft, false))
        //{

        //    m_current_key = VK_LBUTTON;
        //    return true;

        //}

        for (auto i = VK_BACK; i <= VK_RMENU; i++)
        {

            if (ImGui::GetIO().KeysDown[i])
            {

                m_current_key = i;
                return true;

            }

        }

        return false;

    }

public:

    c_binding(const std::string_view& name, const int default_key = VK_LBUTTON, keybind_interaction_type default_type = keybind_interaction_type::HELD)
    {

        this->m_name = name;
        this->m_current_key = default_key;
        this->m_type = default_type;

    }

    std::string get_key_name() const
    {

        if (!m_current_key)
            return "none";

        std::string tmp = key_names[m_current_key]; // there's no issues here... right?

        std::transform(tmp.begin(), tmp.end(), tmp.begin(),
            [](unsigned char c) { return std::tolower(c); });

        tmp[0] = std::tolower(tmp[0]);

        return tmp;

    }

    std::string get_bind_name() const { return m_name; }

    keybind_interaction_type get_bind_type() const { return m_type; }

    int get_bound_key() const { return m_current_key; }

    bool get_enabled() const { return m_enabled; }

    bool update_state()
    {

        if (m_type == keybind_interaction_type::ALWAYS)
            m_enabled = true;
        else if (m_type == keybind_interaction_type::HELD)
            m_enabled = GetAsyncKeyState(m_current_key);
        else if (m_type == keybind_interaction_type::TOGGLE)
            if (GetAsyncKeyState(m_current_key) & 1)
                m_enabled = !m_enabled;
        else if (keybind_interaction_type::NEVER)
            m_enabled = false;

       return m_enabled;

    }

    void set_key(int key) { m_current_key = key; }

    void set_mode(keybind_interaction_type type) { m_type = type; }

};

// the element which displays the interface for bindings
class c_keybind : public c_element
{

private:

    c_binding* m_bind{};
    std::vector<c_animator<float>> m_selectable_animators{};
    std::vector<c_animator<ImVec4>> m_selectable_text_animators{};
    c_animator<float> m_hover_animator{ 0.f, 1.f, framework::animated_border_duration };
    c_animator<float> m_wait_animator{ 0.4f, 1.f, 0.26f };
    c_animator<ImVec4> m_text_animator{ ImVec4(230 / 255.f, 230 / 255.f, 230 / 255.f, 130 / 255.f), ImVec4(1.f, 1.f, 1.f, 1.f), 0.43f };
    keybind_display_type m_display_type{};

public:

    c_keybind(const std::string_view& label, c_binding* bind, keybind_display_type display_type = keybind_display_type::WITH_LABEL, const bool default_visibility = true)
    {

        this->m_label = label;
        this->m_bind = bind;
        this->m_display_type = display_type;
        this->m_visible = default_visibility;
        this->m_type = element_type::KEYBIND;
        this->m_flags = element_flags::WITH_VALUE;

        for (int i = 0; i < 5; i++)
        {

            m_selectable_animators.emplace_back<c_animator<float>>({ 0.f, 6.f, 0.3f });
            m_selectable_text_animators.emplace_back<c_animator<ImVec4>>({ ImVec4(230 / 255.f, 230 / 255.f, 230 / 255.f, 130 / 255.f), ImVec4(1.f, 1.f, 1.f, 1.f), 0.43f });

        }

    }

    // unholy function
    virtual bool render(float dpi_scale)
    {

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(m_bind->get_bind_name().c_str());
        const ImVec2 label_size = ImGui::CalcTextSize(m_bind->get_bind_name().c_str());
        const ImVec2 actual_label_size = ImGui::CalcTextSize(m_label.c_str());

        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size = ImGui::CalcItemSize(ImVec2(0, 0), 60.f * dpi_scale, 16.f * dpi_scale);

        ImRect container_bb(pos, pos + ImVec2(ImGui::GetWindowWidth() - 10.f, 36.f * dpi_scale));
        const ImRect bb(ImVec2(container_bb.Max.x - (size.x + 10.f), container_bb.Min.y + 10.f), container_bb.Max - ImVec2(10.f, 10.f));
        ImGui::ItemSize(container_bb, style.FramePadding.y);
        if (!ImGui::ItemAdd(container_bb, id))
            return false;

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, NULL);

        bool value_changed = false;
        int key = m_bind->get_bound_key();

        auto io = ImGui::GetIO();

        std::string name = m_bind->get_key_name();

        // the thing that shows when you're waiting for a key to be pressed
        if (m_bind->m_waiting_for_input)
            name = "waiting";

        m_wait_animator.update_animation();

        if (m_bind->m_waiting_for_input && m_wait_animator.get_animation_percent() == 0.f)
            m_wait_animator.start_animation(true, true);
        else if (!m_bind->m_waiting_for_input && m_wait_animator.get_animation_percent() == 1.f)
            m_wait_animator.start_animation(false, true);

        if (ImGui::GetIO().MouseClicked[0] && hovered)
        {

            if (g.ActiveId == id)
            {

                m_bind->m_waiting_for_input = true;

            }

        }
        else if (ImGui::GetIO().MouseClicked[1] && hovered && !m_bind->m_waiting_for_input) {
            ImGui::OpenPopup(m_bind->get_bind_name().c_str());
        }
        else if (ImGui::GetIO().MouseClicked[0] && !hovered) {
            if (g.ActiveId == id)
                ImGui::ClearActiveID();
        }

        if (m_bind->m_waiting_for_input)
            if (m_bind->bind_new_key())
            {

                ImGui::ClearActiveID();
                m_bind->m_waiting_for_input = false;

            }

        // render dat hoe

        m_hover_animator.update_animation();
        m_text_animator.update_animation();

        auto imgui_alpha = ImGui::GetStyle().Alpha;

        window->DrawList->AddRectFilled(container_bb.Min, container_bb.Max, IM_COL32(25, 20, 25, 255 * imgui_alpha));

        window->DrawList->AddRectFilledMultiColor(container_bb.Min, { container_bb.Max.x, container_bb.Min.y + 6.f }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0));
        window->DrawList->AddRectFilledMultiColor(container_bb.Max, { container_bb.Min.x, container_bb.Max.y - 6.f }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0));

        window->DrawList->AddRectFilledMultiColor(container_bb.Min, { container_bb.Min.x + 6.f, container_bb.Max.y }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0), IM_COL32(95, 92, 103, 200 * imgui_alpha));
        window->DrawList->AddRectFilledMultiColor(container_bb.Max, { container_bb.Max.x - 6.f, container_bb.Min.y }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0), IM_COL32(95, 92, 103, 200 * imgui_alpha));

        ImGui::GetStyle().Colors[ImGuiCol_AnimatedBorder].w = m_hover_animator.get_value();

        window->DrawList->AddRect(container_bb.Min, container_bb.Max, ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_AnimatedBorder)));

        ImGui::GetStyle().Colors[ImGuiCol_AnimatedBorder].w = 200 / 255.f;

        ImGui::PushStyleColor(ImGuiCol_Text, m_text_animator.get_value());
        window->DrawList->AddText(container_bb.Min + ImVec2(10.f, (label_size.y / 2.f) + 4.f), ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_Text)), m_label.c_str());
        ImGui::PopStyleColor();

        window->DrawList->AddRectFilled(bb.Min, bb.Max, ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_FrameBg)), ImGui::GetStyle().FrameRounding);

        auto bdr_color = ImGui::GetStyleColorVec4(ImGuiCol_Border);

        bdr_color.w = m_wait_animator.get_value();

        if (ImGui::GetStyle().FrameBorderSize)
            window->DrawList->AddRect(bb.Min, bb.Max, ImGui::GetColorU32(bdr_color), ImGui::GetStyle().FrameRounding);

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
        {

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(230 / 255.f, 230 / 255.f, 230 / 255.f, 130 / 255.f));
            ImGui::SetTooltip("RMB for more options");
            ImGui::PopStyleColor();

        }

        if (ImGui::IsMouseHoveringRect(container_bb.Min, container_bb.Max))
            m_hover_animator.start_animation(true, false, m_hover_animator.get_animation_percent() == 1.f ? true : false);
        else if (m_hover_animator.get_animation_percent() == 1.f)
            m_hover_animator.start_animation(false);

        if (ImGui::IsItemHovered())
             m_text_animator.start_animation(true, false, m_text_animator.get_animation_percent() == 1.f ? true : false);
        else if (m_text_animator.get_animation_percent() == 1.f)
            m_text_animator.start_animation(false);

        const auto text_sz = ImGui::CalcTextSize(name.c_str());
        const auto centered_x = (size.x - text_sz.x) * 0.5f;
        const auto centered_y = (size.y - text_sz.y) * 0.5f;

        window->DrawList->AddText(ImVec2(bb.Min.x + centered_x, bb.Min.y + centered_y), ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_Text)), name.c_str());

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

        ImGui::SetNextWindowPos(bb.Min);
        ImGui::SetNextWindowSize({ bb.GetWidth(), 0.f });

        for (auto& animator : m_selectable_animators)
            animator.update_animation();

        for (auto& animator : m_selectable_text_animators)
            animator.update_animation();

        if (ImGui::BeginPopup(m_bind->get_bind_name().c_str(), window_flags))
        {

            ImGui::BeginGroup();

            {

                auto pressed = false;

                if (framework::selectable_ex2("always", m_bind->m_type == keybind_interaction_type::ALWAYS, ImGuiSelectableFlags_DontClosePopups, { 0.f, 0.f }, &m_selectable_animators.at(0), &m_selectable_text_animators.at(0), dpi_scale)) 
                {

                    if (m_bind->m_type != keybind_interaction_type::ALWAYS)
                    {

                        m_bind->m_type = keybind_interaction_type::ALWAYS;
                        pressed = true;

                    }

                }

                auto selected = m_bind->m_type == keybind_interaction_type::ALWAYS;

                if (ImGui::IsItemHovered())
                    m_selectable_animators.at(0).start_animation(true, false, m_selectable_animators.at(0).get_animation_percent() == 1.f ? true : false);
                else if (m_selectable_animators.at(0).get_animation_percent() == 1.f)
                    m_selectable_animators.at(0).start_animation(false);

                if (pressed && selected || (m_selectable_text_animators.at(0).get_animation_percent() == 0.f && selected))
                    m_selectable_text_animators.at(0).start_animation(true);
                else if (m_selectable_text_animators.at(0).get_animation_percent() == 1.f && !selected)
                    m_selectable_text_animators.at(0).start_animation(false);

            }

            {

                auto pressed = false;

                if (framework::selectable_ex2("on key", m_bind->m_type == keybind_interaction_type::HELD, ImGuiSelectableFlags_DontClosePopups, { 0.f, 0.f }, &m_selectable_animators.at(1), &m_selectable_text_animators.at(1), dpi_scale))
                {

                    if (m_bind->m_type != keybind_interaction_type::HELD)
                    {

                        m_bind->m_type = keybind_interaction_type::HELD;
                        pressed = true;

                    }

                }

                auto selected = m_bind->m_type == keybind_interaction_type::HELD;

                if (ImGui::IsItemHovered())
                    m_selectable_animators.at(1).start_animation(true, false, m_selectable_animators.at(1).get_animation_percent() == 1.f ? true : false);
                else if (m_selectable_animators.at(1).get_animation_percent() == 1.f)
                    m_selectable_animators.at(1).start_animation(false);

                if (pressed && selected || (m_selectable_text_animators.at(1).get_animation_percent() == 0.f && selected))
                    m_selectable_text_animators.at(1).start_animation(true);
                else if (m_selectable_text_animators.at(1).get_animation_percent() == 1.f && !selected)
                    m_selectable_text_animators.at(1).start_animation(false);

            }

            {

                auto pressed = false;

                if (framework::selectable_ex2("toggle", m_bind->m_type == keybind_interaction_type::TOGGLE, ImGuiSelectableFlags_DontClosePopups, { 0.f, 0.f }, &m_selectable_animators.at(2), &m_selectable_text_animators.at(2), dpi_scale))
                {

                    if (m_bind->m_type != keybind_interaction_type::TOGGLE)
                    {

                        m_bind->m_type = keybind_interaction_type::TOGGLE;
                        pressed = true;

                    }

                }

                auto selected = m_bind->m_type == keybind_interaction_type::TOGGLE;

                if (ImGui::IsItemHovered())
                    m_selectable_animators.at(2).start_animation(true, false, m_selectable_animators.at(2).get_animation_percent() == 1.f ? true : false);
                else if (m_selectable_animators.at(2).get_animation_percent() == 1.f)
                    m_selectable_animators.at(2).start_animation(false);

                if (pressed && selected || (m_selectable_text_animators.at(2).get_animation_percent() == 0.f && selected))
                    m_selectable_text_animators.at(2).start_animation(true);
                else if (m_selectable_text_animators.at(2).get_animation_percent() == 1.f && !selected)
                    m_selectable_text_animators.at(2).start_animation(false);

            }

            {

                auto pressed = false;

                if (framework::selectable_ex2("never", m_bind->m_type == keybind_interaction_type::NEVER, ImGuiSelectableFlags_DontClosePopups, { 0.f, 0.f }, &m_selectable_animators.at(3), &m_selectable_text_animators.at(3), dpi_scale))
                {

                    if (m_bind->m_type != keybind_interaction_type::NEVER)
                    {

                        m_bind->m_type = keybind_interaction_type::NEVER;
                        pressed = true;

                    }

                }

                auto selected = m_bind->m_type == keybind_interaction_type::NEVER;

                if (ImGui::IsItemHovered())
                    m_selectable_animators.at(3).start_animation(true, false, m_selectable_animators.at(3).get_animation_percent() == 1.f ? true : false);
                else if (m_selectable_animators.at(3).get_animation_percent() == 1.f)
                    m_selectable_animators.at(3).start_animation(false);

                if (pressed && selected || (m_selectable_text_animators.at(3).get_animation_percent() == 0.f && selected))
                    m_selectable_text_animators.at(3).start_animation(true);
                else if (m_selectable_text_animators.at(3).get_animation_percent() == 1.f && !selected)
                    m_selectable_text_animators.at(3).start_animation(false);

            }

            ImGui::EndGroup();

            ImGui::EndPopup();

        }

        return pressed;

    }

    c_binding* get_bind() const { return m_bind; }

    keybind_display_type get_display_type() const { return m_display_type; }

};

#endif
