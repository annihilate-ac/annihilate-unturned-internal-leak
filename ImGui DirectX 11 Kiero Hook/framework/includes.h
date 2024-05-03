#pragma once

#include <cstdint>

#include <vector>
#include <string>
#include <algorithm>
#include <functional>
#include <iostream>
#include <string_view>
#include <map>
#include <mutex>

#include "config.h"

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_freetype.h"

#if defined(USE_ICONS)
#include "icons.h" // fix it yoself
#endif

#include "shadow.h"

#include <D3DX11tex.h>
#pragma comment(lib, "D3DX11.lib")

static inline ImVec2  operator*(const ImVec2& lhs, const float rhs) { return ImVec2(lhs.x * rhs, lhs.y * rhs); }
static inline ImVec2  operator/(const ImVec2& lhs, const float rhs) { return ImVec2(lhs.x / rhs, lhs.y / rhs); }
static inline ImVec2  operator+(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
static inline ImVec2  operator-(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }
static inline ImVec2  operator*(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x * rhs.x, lhs.y * rhs.y); }
static inline ImVec2  operator/(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x / rhs.x, lhs.y / rhs.y); }
static inline ImVec2& operator*=(ImVec2& lhs, const float rhs) { lhs.x *= rhs; lhs.y *= rhs; return lhs; }
static inline ImVec2& operator/=(ImVec2& lhs, const float rhs) { lhs.x /= rhs; lhs.y /= rhs; return lhs; }
static inline ImVec2& operator+=(ImVec2& lhs, const ImVec2& rhs) { lhs.x += rhs.x; lhs.y += rhs.y; return lhs; }
static inline ImVec2& operator-=(ImVec2& lhs, const ImVec2& rhs) { lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs; }
static inline ImVec2& operator*=(ImVec2& lhs, const ImVec2& rhs) { lhs.x *= rhs.x; lhs.y *= rhs.y; return lhs; }
static inline ImVec2& operator/=(ImVec2& lhs, const ImVec2& rhs) { lhs.x /= rhs.x; lhs.y /= rhs.y; return lhs; }
static inline ImVec4  operator+(const ImVec4& lhs, const ImVec4& rhs) { return ImVec4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w); }
static inline ImVec4  operator-(const ImVec4& lhs, const ImVec4& rhs) { return ImVec4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w); }
static inline ImVec4  operator*(const ImVec4& lhs, const ImVec4& rhs) { return ImVec4(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w); }
static inline ImVec4  operator*(const ImVec4& lhs, const float rhs) { return ImVec4(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs); }
static inline ImVec4  operator/(const ImVec4& lhs, const float rhs) { return ImVec4(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs, lhs.w / rhs); }
static inline ImVec4  operator/(const ImVec4& lhs, const ImVec4& rhs) { return ImVec4(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w); }
static inline bool  operator<(const ImVec4& lhs, const ImVec4& rhs) { return lhs.x < rhs.x && lhs.y < rhs.y && lhs.z < rhs.z && lhs.w < rhs.w; }

// fix for imgui being trash ^

namespace ImGui
{

    static void TextGradiented(const char* text, ImU32 leftcolor, ImU32 rightcolor, float smooth = 175, float alpha = 266) {


        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return;

        const ImVec2 pos = window->DC.CursorPos;
        ImDrawList* draw_list = window->DrawList;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(text);
        const ImVec2 size = CalcTextSize(text);

        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ItemSize(bb, style.FramePadding.y);
        if (!ItemAdd(bb, id))
            return;

        const ImU32 col_white = IM_COL32(255, 255, 255, alpha == 266 ? static_cast<int>(ImGui::GetStyle().Alpha * 255.f) : alpha);
        float centeredvertex = ImMax((int)smooth, 35);
        float vertex_out = centeredvertex * 0.50f;
        float text_inner = vertex_out - centeredvertex;
        const int vert_start_idx = draw_list->VtxBuffer.Size;
        draw_list->AddText(pos, col_white, text);
        const int vert_end_idx = draw_list->VtxBuffer.Size;
        for (int n = 0; n < 1; n++)
        {
            const ImU32 col_hues[2] = { leftcolor, rightcolor };
            ImVec2 textcenter(pos.x + (size.x / 2), pos.y + (size.y / 2));

            const float a0 = (n) / 6.0f * 2.0f * IM_PI - (0.5f / vertex_out);
            const float a1 = (n + 1.0f) / 6.0f * 2.0f * IM_PI + (0.5f / vertex_out);


            ImVec2 gradient_p0(textcenter.x + ImCos(a0) * text_inner, textcenter.y + ImSin(a0) * text_inner);
            ImVec2 gradient_p1(textcenter.x + ImCos(a1) * text_inner, textcenter.y + ImSin(a1) * text_inner);
            ShadeVertsLinearColorGradientKeepAlpha(draw_list, vert_start_idx, vert_end_idx, gradient_p0, gradient_p1, col_hues[n], col_hues[n + 1]);
        }
    }

    static void TextGradiented(const char* text, ImDrawList* draw_list, ImVec2 pos, ImU32 leftcolor, ImU32 rightcolor, float smooth = 175, float alpha = 266) {


        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImVec2 size = CalcTextSize(text);

        const ImU32 col_white = IM_COL32(255, 255, 255, alpha == 266 ? static_cast<int>(ImGui::GetStyle().Alpha * 255.f) : alpha);
        float centeredvertex = ImMax((int)smooth, 35);
        float vertex_out = centeredvertex * 0.50f;
        float text_inner = vertex_out - centeredvertex;
        const int vert_start_idx = draw_list->VtxBuffer.Size;
        draw_list->AddText(pos, col_white, text);
        const int vert_end_idx = draw_list->VtxBuffer.Size;
        for (int n = 0; n < 1; n++)
        {
            const ImU32 col_hues[2] = { leftcolor, rightcolor };
            ImVec2 textcenter(pos.x + (size.x / 2), pos.y + (size.y / 2));

            const float a0 = (n) / 6.0f * 2.0f * IM_PI - (0.5f / vertex_out);
            const float a1 = (n + 1.0f) / 6.0f * 2.0f * IM_PI + (0.5f / vertex_out);


            ImVec2 gradient_p0(textcenter.x + ImCos(a0) * text_inner, textcenter.y + ImSin(a0) * text_inner);
            ImVec2 gradient_p1(textcenter.x + ImCos(a1) * text_inner, textcenter.y + ImSin(a1) * text_inner);
            ShadeVertsLinearColorGradientKeepAlpha(draw_list, vert_start_idx, vert_end_idx, gradient_p0, gradient_p1, col_hues[n], col_hues[n + 1]);
        }
    }

    static void RenderDropShadow(ImTextureID tex_id, float size, ImU8 opacity)
    {

        ImVec2 p = ImGui::GetWindowPos();
        ImVec2 s = ImGui::GetWindowSize();
        ImVec2 m = { p.x + s.x, p.y + s.y };
        float uv0 = 0.0f;      // left/top region
        float uv1 = 0.333333f; // leftward/upper region
        float uv2 = 0.666666f; // rightward/lower region
        float uv3 = 1.0f;      // right/bottom region
        ImU32 col = IM_COL32(255, 255, 255, static_cast<int>(ImGui::GetStyle().Alpha * 255.f));
        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        dl->PushClipRectFullScreen();
        dl->AddImage(tex_id, { p.x - size, p.y - size }, { p.x,        p.y }, { uv0, uv0 }, { uv1, uv1 }, col);
        dl->AddImage(tex_id, { p.x,        p.y - size }, { m.x,        p.y }, { uv1, uv0 }, { uv2, uv1 }, col);
        dl->AddImage(tex_id, { m.x,        p.y - size }, { m.x + size, p.y }, { uv2, uv0 }, { uv3, uv1 }, col);
        dl->AddImage(tex_id, { p.x - size, p.y }, { p.x,        m.y }, { uv0, uv1 }, { uv1, uv2 }, col);
        dl->AddImage(tex_id, { m.x,        p.y }, { m.x + size, m.y }, { uv2, uv1 }, { uv3, uv2 }, col);
        dl->AddImage(tex_id, { p.x - size, m.y }, { p.x,        m.y + size }, { uv0, uv2 }, { uv1, uv3 }, col);
        dl->AddImage(tex_id, { p.x,        m.y }, { m.x,        m.y + size }, { uv1, uv2 }, { uv2, uv3 }, col);
        dl->AddImage(tex_id, { m.x,        m.y }, { m.x + size, m.y + size }, { uv2, uv2 }, { uv3, uv3 }, col);
        dl->PopClipRect();

    }

    static ID3D11ShaderResourceView* shadow_texture = NULL;

    static void CreateShadowTexture(ID3D11Device* device)
    {

        D3DX11_IMAGE_LOAD_INFO info;
        ID3DX11ThreadPump* pump{ nullptr };
        D3DX11CreateShaderResourceViewFromMemory(device, raw_shadow_data, sizeof(raw_shadow_data), &info, pump, &shadow_texture, 0);

    }

}

struct scaled_font_t
{

    std::string file_name{ "" };
    float size{ 0.f };
    ImGuiFreeTypeBuilderFlags flags{};
    ImFont* font{ NULL };
    bool is_built{ false };

    static void script_conscructor(void* memory)
    { new(memory) scaled_font_t(); }

    static void script_destructor(void* memory)
    { ((scaled_font_t*)memory)->~scaled_font_t(); }

    static scaled_font_t* ref_factory() { return new scaled_font_t(); }

    scaled_font_t& operator=(const scaled_font_t& other)
    {

        file_name = other.file_name;
        size = other.size;
        flags = other.flags;
        font = other.font;
        is_built = other.is_built;

        return *this;

    }

    bool operator==(const scaled_font_t& other)
    {

        return file_name == other.file_name && size == other.size && flags == other.flags;

    }

    bool is_valid()
    { return font && file_name != "" && font->ContainerAtlas && is_built; }

};

namespace framework
{

    static float animated_border_duration = 0.32f;

    static std::vector<scaled_font_t> g_scaled_fonts{};
    static std::mutex g_font_mtx{};
    static ImFont* bold_font = nullptr;
    static int current_scaling{ 0 };
    static bool needs_to_build_fonts{ true };

    static float get_dpi_scale()
    {

        return framework::current_scaling == 0 ? 1.f : framework::current_scaling == 1 ? 1.2f : framework::current_scaling == 2 ? 1.6f : framework::current_scaling == 3 ? 1.8f : framework::current_scaling == 4 ? 2.f : 1.f;

    }

    static bool build_fonts()
    {

        static size_t last_font_size = NULL;

        if (!needs_to_build_fonts)
        {

            if (last_font_size == g_scaled_fonts.size())
                return false;

        }

        auto& io = ImGui::GetIO();

        io.Fonts->Clear();

        int idx = 0;

        framework::g_font_mtx.lock();

        static const ImWchar ranges[] =
        {
            0x0020, 0x00FF, // Basic Latin + Latin Supplement
            0x0400, 0x044F, // Cyrillic
            0,
        };

        for (auto& scaled_font : g_scaled_fonts)
        {

            ImFontConfig config{};
            config.FontBuilderFlags = scaled_font.flags;

            scaled_font.font = io.Fonts->AddFontFromFileTTF(scaled_font.file_name.c_str(), scaled_font.size * (idx < 2 ? framework::get_dpi_scale() : 1.f), &config, ranges);

            idx++;

        }

        framework::g_font_mtx.unlock();

        auto built = io.Fonts->Build();

        if (!built)
            return false;

        framework::g_font_mtx.lock();

        for (auto& scaled_font : g_scaled_fonts)
            scaled_font.is_built = true;

        framework::g_font_mtx.unlock();

        ImGui_ImplDX11_InvalidateDeviceObjects();
        ImGui_ImplDX11_CreateDeviceObjects();

        io.FontDefault = g_scaled_fonts.at(1).font;
        bold_font = g_scaled_fonts.at(0).font;

        needs_to_build_fonts = false;
        last_font_size = g_scaled_fonts.size();

        return true;

    }

    static bool vsync = false;

}