#pragma once

#include "includes.h"

#include <string>
#include <array>

#include "imgui/imgui.h"

struct render_t
{

#define vec_ref const ImVec2&
#define decl static auto

	decl rect(vec_ref from, vec_ref to, ImU32 col, float rounding = 0.f) {
		ImGui::GetBackgroundDrawList()->AddRect(from, to, col, rounding);
	}

	decl rect_filled(vec_ref from, vec_ref to, ImU32 col, float rounding = 0.f) {
		ImGui::GetBackgroundDrawList()->AddRectFilled(from, to, col, rounding);
	}

	decl outlined_rect(vec_ref pos, vec_ref size, ImU32 col, float rounding = 0.f) {

		auto real_pos = pos;
		auto real_size = size;

		auto draw = ImGui::GetBackgroundDrawList();
		const ImRect rect_bb(real_pos, real_pos + real_size);

		draw->AddRect(rect_bb.Min, rect_bb.Max, IM_COL32(0, 0, 0, col >> 24));
		draw->AddRect(rect_bb.Min - ImVec2{ 2.f, 2.f }, rect_bb.Max + ImVec2{ 2.f, 2.f }, IM_COL32(0, 0, 0, col >> 24));
		draw->AddRect(rect_bb.Min - ImVec2{ 1.f, 1.f }, rect_bb.Max + ImVec2{ 1.f, 1.f }, col);

	}

	decl cornered_rect(vec_ref pos, vec_ref size, ImU32 col)
	{

		auto draw = ImGui::GetBackgroundDrawList();

		float X = pos.x; float Y = pos.y;
		float W = size.x; float H = size.y;

		float lineW = (size.x / 4);
		float lineH = (size.y / 4);
		float lineT = 1;

		auto outline = IM_COL32(0, 0, 0, col >> 24);

		//outline
		draw->AddLine({ X - lineT + 1.f, Y - lineT }, { X + lineW, Y - lineT }, outline); //top left
		draw->AddLine({ X - lineT, Y - lineT }, { X - lineT, Y + lineH }, outline);

		draw->AddLine({ X + W - lineW, Y - lineT }, { X + W + lineT, Y - lineT }, outline); // top right
		draw->AddLine({ X + W + lineT, Y - lineT }, { X + W + lineT, Y + lineH }, outline);

		draw->AddLine({ X + W + lineT, Y + H - lineH }, { X + W + lineT, Y + H + lineT }, outline); // bot right
		draw->AddLine({ X + W - lineW, Y + H + lineT }, { X + W + lineT, Y + H + lineT }, outline);

		draw->AddLine({ X - lineT, Y + H - lineH }, { X - lineT, Y + H + lineT }, outline); //bot left
		draw->AddLine({ X - lineT, Y + H + lineT }, { X + lineW, Y + H + lineT }, outline);

		{

			draw->AddLine({ X - (lineT - 3), Y - (lineT - 2) }, { X + lineW, Y - (lineT - 2) }, outline); //top left
			draw->AddLine({ X - (lineT - 2), Y - (lineT - 2) }, { X - (lineT - 2), Y + lineH }, outline);

			draw->AddLine({ X - (lineT - 2), Y + H - lineH }, { X - (lineT - 2), Y + H + (lineT - 2) }, outline); //bot left
			draw->AddLine({ X - (lineT - 2), Y + H + (lineT - 2) }, { X + lineW, Y + H + (lineT - 2) }, outline);

			draw->AddLine({ X + W - lineW, Y - (lineT - 2) }, { X + W + (lineT - 2), Y - (lineT - 2) }, outline); // top right
			draw->AddLine({ X + W + (lineT - 2), Y - (lineT - 2) }, { X + W + (lineT - 2), Y + lineH }, outline);

			draw->AddLine({ X + W + (lineT - 2), Y + H - lineH }, { X + W + (lineT - 2), Y + H + (lineT - 2) }, outline); // bot right
			draw->AddLine({ X + W - lineW, Y + H + (lineT - 2) }, { X + W + (lineT - 2), Y + H + (lineT - 2) }, outline);

		}

		//inline
		draw->AddLine({ X, Y }, { X, Y + lineH }, col);//top left
		draw->AddLine({ X + 1.f, Y }, { X + lineW, Y }, col);

		draw->AddLine({ X + W - lineW, Y }, { X + W, Y }, col); //top right
		draw->AddLine({ X + W , Y }, { X + W, Y + lineH }, col);

		draw->AddLine({ X, Y + H - lineH }, { X, Y + H }, col); //bot left
		draw->AddLine({ X, Y + H }, { X + lineW, Y + H }, col);

		draw->AddLine({ X + W - lineW, Y + H }, { X + W, Y + H }, col);//bot right
		draw->AddLine({ X + W, Y + H - lineH }, { X + W, Y + H }, col);

	}

	decl text(vec_ref pos, const std::string& text, ImU32 col, scaled_font_t* font) {
		ImGui::GetBackgroundDrawList()->AddText(font->font, font->size, pos, col, text.c_str());
	}

	decl text_shadowed(vec_ref pos, const std::string& text, ImU32 col, scaled_font_t* font) 
	{

		auto alpha = col >> 24;

		if (alpha > 0)
		{

			ImGui::GetBackgroundDrawList()->AddText(font->font, font->size, pos + ImVec2(-1.f, 1.f), IM_COL32(0, 0, 0, col >> 24), text.c_str());
			ImGui::GetBackgroundDrawList()->AddText(font->font, font->size, pos + ImVec2(1.f, -1.f), IM_COL32(0, 0, 0, col >> 24), text.c_str());

			ImGui::GetBackgroundDrawList()->AddText(font->font, font->size, pos + ImVec2(1.f, 1.f), IM_COL32(0, 0, 0, col >> 24), text.c_str());
			ImGui::GetBackgroundDrawList()->AddText(font->font, font->size, pos + ImVec2(-1.f, -1.f), IM_COL32(0, 0, 0, col >> 24), text.c_str());

			ImGui::GetBackgroundDrawList()->AddText(font->font, font->size, pos + ImVec2(0.f, 1.f), IM_COL32(0, 0, 0, col >> 24), text.c_str());
			ImGui::GetBackgroundDrawList()->AddText(font->font, font->size, pos + ImVec2(0.f, -1.f), IM_COL32(0, 0, 0, col >> 24), text.c_str());
			
			ImGui::GetBackgroundDrawList()->AddText(font->font, font->size, pos + ImVec2(1.f, 0.f), IM_COL32(0, 0, 0, col >> 24), text.c_str());
			ImGui::GetBackgroundDrawList()->AddText(font->font, font->size, pos + ImVec2(-1.f, 0.f), IM_COL32(0, 0, 0, col >> 24), text.c_str());

		}

		ImGui::GetBackgroundDrawList()->AddText(font->font, font->size, pos, col, text.c_str());

	}

	decl line(vec_ref from, vec_ref to, ImU32 col, float thickness = 1.f) {
		ImGui::GetBackgroundDrawList()->AddLine(from, to, col, thickness);
	}

	decl line_segment(vec_ref from, vec_ref to, ImU32 col, float thickness, float segments = 1.f) {

		if (segments > 1) 
		{

			auto draw_list = ImGui::GetBackgroundDrawList();

			float segment_length = 1.0f / segments;
			ImVec2 delta = to - from;

			for (int i = 0; i < segments; ++i) {

				float alpha = segment_length * i;

				// Calculate the position for each segment
				ImVec2 segment_pos = ImVec2(from.x + delta.x * alpha, from.y + delta.y * alpha);

				// Draw a circle at the calculated position to simulate a dot
				draw_list->AddCircleFilled(segment_pos, thickness, col);
			}

		}

	}

	decl circle(vec_ref pos, ImU32 col, float radius) {
		ImGui::GetBackgroundDrawList()->AddCircle(pos, radius, col, 128);
	}

	decl circle_filled(vec_ref pos, ImU32 col, float radius) {
		ImGui::GetBackgroundDrawList()->AddCircleFilled(pos, radius, col, 128);
	}

	decl triangle_filled(std::array <ImVec2, 3> points, ImU32 col) {
		ImGui::GetBackgroundDrawList()->AddTriangleFilled(points[0], points[1], points[2], col);
	}

#undef vec_ref
#undef decl

}; inline render_t render{};