#pragma once

#include "includes.h"

// allows you to draw before the window's contents are rendered...
class c_attachment
{

private:

    std::function<void(const ImVec2&, const ImVec2&, ImDrawList*)> m_render_function{};

public:

    inline c_attachment(const std::function<void(const ImVec2&, const ImVec2&, ImDrawList*)>& function) noexcept
    {

        this->m_render_function = function;

    }

    void render(const ImVec2& pos, const ImVec2& sz, ImDrawList* draw) { if (m_render_function) m_render_function(pos, sz, draw); }

};
