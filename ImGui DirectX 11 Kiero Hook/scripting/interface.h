#pragma once

#pragma once

#include <angelscript.h>

#include "angelscript/vector_binding.h"

#include "../imgui/imgui.h"

#include "../sdk/game.h"

#include "../logs/logs.h"

namespace script_fonts
{

	scaled_font_t* create_font(std::string file_name, float size);

}

namespace script_elements
{

	c_checkbox* add_checkbox(std::string label, bool value = false);

	c_slider_float* add_slider_float(std::string label, float min, float max, float value = 0.f, std::string format_text = "%.1f");

	c_slider_int* add_slider_int(std::string label, int min, int max, int value = 0.f, std::string format_text = "%d");

	c_combo* add_combo(std::string label, CScriptArray* combo_elements, int value);

	c_multi_combo* add_multi_combo(std::string label, CScriptArray* combo_elements);

	c_label* add_label(std::string label);

	c_divider* add_divider(std::string label);

	c_label_divider* add_label_divider(std::string label);

	c_button* add_button(std::string label);

	c_colorpicker* add_color_picker(std::string label);

	void remove_element(std::string label);

}

namespace script_interface
{

	inline c_child* element_child = nullptr;
	inline c_child* main_child = nullptr;
	inline std::string current_callback = "";
	inline int current_script = 0;

	void construct_vector_3(float x, float y, float z, vec3_t* self) { new(self) vec3_t(x, y, z); }

	void construct_list_vector_3(float* list, vec3_t* self) { new(self) vec3_t(list[0], list[1], list[2]); }

	void construct_vector_2(float x, float y, ImVec2* self) { new(self) ImVec2(x, y); }

	void construct_list_vector_2(float* list, ImVec2* self) { new(self) ImVec2(list[0], list[1]); }

	void construct_color(int r, int g, int b, int a, c_color* self) { new(self) c_color(r / 255.f, g / 255.f, b / 255.f, a / 255.f); }

	void construct_list_color(int* list, c_color* self) { new(self) c_color(list[0] / 255.f, list[1] / 255.f, list[2] / 255.f, list[3] / 255.f); }

	__forceinline bool callback_ret(std::string callback_name, std::string fn_name) 
	{ 
		
		if (current_callback != callback_name)
		{

			std::stringstream ss{};

			ss << "calling \"" << fn_name << "\" outside of \"" << callback_name << "\"!";

			g_logs->add_log(ss.str());

			return false;

		}
	
		return true;

	}

	namespace cheat
	{

		void add_log(std::string text, bool extend_length = false)
		{

			g_logs->add_log(text, extend_length);

		}

		CScriptArray* get_players()
		{

			if (!callback_ret("update", "get_players"))
				return nullptr;

			asIScriptContext* ctx = asGetActiveContext();

			if (ctx)
			{

				auto engine = ctx->GetEngine();

				static asITypeInfo* type = engine->GetTypeInfoById(engine->GetTypeIdByDecl("array<player_t>"));
				CScriptArray* script_array = CScriptArray::Create(type, ::cheat::players.size());

				for (unsigned int i = 0; i < ::cheat::players.size(); ++i)
					script_array->SetValue(i, &::cheat::players.at(i));

				return script_array;

			}

			return nullptr;

		}

		CScriptArray* get_items()
		{

			if (!callback_ret("update", "get_items"))
				return nullptr;

			asIScriptContext* ctx = asGetActiveContext();

			if (ctx)
			{

				auto engine = ctx->GetEngine();
				
				static asITypeInfo* type = engine->GetTypeInfoById(engine->GetTypeIdByDecl("array<item_t>"));
				CScriptArray* script_array = CScriptArray::Create(type, ::cheat::items.size());

				for (size_t i = 0; i < ::cheat::items.size(); ++i)
					script_array->SetValue(i, &::cheat::items.at(i));

				return script_array;

			}

			return nullptr;

		}

		CScriptArray* get_keys()
		{

			asIScriptContext* ctx = asGetActiveContext();

			if (ctx)
			{

				auto engine = ctx->GetEngine();
				auto keybinds = element_child->get_parent()->get_all_binds();

				static asITypeInfo* type = engine->GetTypeInfoById(engine->GetTypeIdByDecl("array<bind_t>"));
				CScriptArray* script_array = CScriptArray::Create(type, keybinds.size());

				for (size_t i = 0; i < keybinds.size(); ++i)
					script_array->SetValue(i, keybinds.at(i)->get_bind());

				return script_array;

			}

			return nullptr;

		}

	}

	namespace render
	{

		void add_rect(ImVec2 pos, ImVec2 size, c_color color, float rounding = 0.f) { ::render.rect(pos, pos + size, color.u32(), rounding); }
		void add_outlined_rect(ImVec2 pos, ImVec2 size, c_color color, float rounding = 0.f) { ::render.outlined_rect(pos, size, color.u32(), rounding); }
		void add_rect_filled(ImVec2 pos, ImVec2 size, c_color color, float rounding = 0.f) { ::render.rect_filled(pos, pos + size, color.u32(), rounding); }

		void add_circle(ImVec2 pos, c_color color, float radius) { ::render.circle(pos, color.u32(), radius); }
		void add_circle_filled(ImVec2 pos, c_color color, float radius) { ::render.circle_filled(pos, color.u32(), radius); }

		void add_text(ImVec2 pos, std::string text, c_color color, int font) { ::render.text(pos, text, color.u32(), &framework::g_scaled_fonts.at(font)); }
		void add_text(ImVec2 pos, std::string text, c_color color, scaled_font_t* font) { if ( !font || !font->is_valid() ) return; ::render.text(pos, text, color.u32(), font); }
		void add_text_shadowed(ImVec2 pos, std::string text, c_color color, int font) { ::render.text_shadowed(pos, text, color.u32(), &framework::g_scaled_fonts.at(font)); }
		void add_text_shadowed(ImVec2 pos, std::string text, c_color color, scaled_font_t* font) { if ( !font || !font->is_valid() ) return; ::render.text_shadowed(pos, text, color.u32(), font); }

		ImVec2 to_screen(vec3_t pos)
		{

			ImVec2 ret{};

			if (::game::to_screen(pos, ret))
				return ret;
			else
				ret = ImVec2(-1.f, -1.f);

			return ret;

		}

		ImVec2 get_cursor_pos() { return ImGui::GetIO().MousePos; }
		ImVec2 get_display_size() { return ImGui::GetIO().DisplaySize; }
		float get_delta_time() { return ImGui::GetIO().DeltaTime; }
		double get_time() { return ImGui::GetTime(); }

		bool is_in_bounds(ImVec2 pos, ImVec2 size, ImVec2 test_pos)
		{

			const ImRect bb(pos, pos + size);

			return bb.Contains(test_pos);

		}

		bool is_key_down(int key) { return GetAsyncKeyState(key); }

	}

	namespace game
	{

		mono_class_t* get_class(std::string class_name, std::string assembly_name)
		{

			auto klass = ::mono::find_class(assembly_name.c_str(), class_name.c_str());

			if (!klass)
				return nullptr;
			
			return klass;

		}

		int8_t read_int8(uintptr_t address) { return driver.read<int8_t>(address); }
		int16_t read_int16(uintptr_t address) { return driver.read<int16_t>(address); }
		int32_t read_int32(uintptr_t address) { return driver.read<int32_t>(address); }
		int64_t read_int64(uintptr_t address) { return driver.read<int64_t>(address); }

		uint8_t read_uint8(uintptr_t address) { return driver.read<uint8_t>(address); }
		uint16_t read_uint16(uintptr_t address) { return driver.read<uint16_t>(address); }
		uint32_t read_uint32(uintptr_t address) { return driver.read<uint32_t>(address); }
		uint64_t read_uint64(uintptr_t address) { return driver.read<uint64_t>(address); }

		float read_float(uintptr_t address) { return driver.read<float>(address); }
		double read_double(uintptr_t address) { return driver.read<double>(address); }

		bool read_bool(uintptr_t address) { return driver.read<bool>(address); }

		vec3_t read_vec3(uintptr_t address) { return driver.read<vec3_t>(address); }
		void write_vec3(uintptr_t address, vec3_t val) { driver.write<vec3_t>(address, val); }

		void write_int8(uintptr_t address, int8_t value) { driver.write<int8_t>(address, value); }
		void write_int16(uintptr_t address, int16_t value) { driver.write<int16_t>(address, value); }
		void write_int32(uintptr_t address, int32_t value) { driver.write<int32_t>(address, value); }
		void write_int64(uintptr_t address, int64_t value) { driver.write<int64_t>(address, value); }

		void write_uint8(uintptr_t address, uint8_t value) { driver.write<uint8_t>(address, value); }
		void write_uint16(uintptr_t address, uint16_t value) { driver.write<uint16_t>(address, value); }
		void write_uint32(uintptr_t address, uint32_t value) { driver.write<uint32_t>(address, value); }
		void write_uint64(uintptr_t address, uint64_t value) { driver.write<uint64_t>(address, value); }

		void write_float(uintptr_t address, float value) { driver.write<float>(address, value); }
		void write_double(uintptr_t address, double value) { driver.write<double>(address, value); }

		void write_bool(uintptr_t address, bool value) { driver.write<bool>(address, value); }

		std::string read_string(uintptr_t address)
		{

			auto managed_string = driver.read<uintptr_t>(address);

			if (!managed_string)
				return "NULL";

			auto wide_str = driver.read_wstr(managed_string + 0x14);

			return get_string_from_wcs(wide_str.c_str());
			
		}

		CScriptArray* read_array(uintptr_t address)
		{

			auto list = reinterpret_cast<unity_list_t*>(address);

			if (!list)
				return NULL;

			auto size = list->size();

			if (!size)
				return NULL;

			auto list_ptr = list->list();

			if (!list_ptr)
				return NULL;

			auto vec = driver.read_vector<uintptr_t>(list_ptr, size);

			asIScriptContext* ctx = asGetActiveContext();

			auto engine = ctx->GetEngine();

			static asITypeInfo* type = engine->GetTypeInfoById(engine->GetTypeIdByDecl("array<uint64>"));
			CScriptArray* script_array = CScriptArray::Create(type, vec.size());

			for (size_t i = 0; i < vec.size(); ++i)
				script_array->SetValue(i, &vec.at(i));

			return script_array;


		}

		CScriptArray* read_multi_dimensional_array(uintptr_t address)
		{

			auto list = reinterpret_cast<unity_list_2d<uintptr_t>*>(address);

			if (!list)
				return NULL;

			auto vec = list->data();

			if (!vec.size())
				return NULL;

			asIScriptContext* ctx = asGetActiveContext();

			auto engine = ctx->GetEngine();

			static asITypeInfo* type = engine->GetTypeInfoById(engine->GetTypeIdByDecl("array<uint64>"));
			CScriptArray* script_array = CScriptArray::Create(type, vec.size());

			for (size_t i = 0; i < vec.size(); ++i)
				script_array->SetValue(i, &vec.at(i));

			return script_array;

		}

		vec3_t get_transform_position(uintptr_t transform)
		{

			if (!transform)
				return {};

			auto transform_ptr = driver.read<uintptr_t>(transform + 0x10);

			if (!transform_ptr)
				return {};

			TransformInternal internal_transform = TransformInternal(transform_ptr);

			return internal_transform.position();


		}

		void move_mouse(ImVec2 pos, float smoothing) 
		{

			static int horizontal = 0;
			static int vertical = 0;

			if (!horizontal || !vertical)
			{

				RECT desktop;

				const HWND hDesktop = GetDesktopWindow();

				GetWindowRect(hDesktop, &desktop);

				horizontal = desktop.right;
				vertical = desktop.bottom;

			}

			float ScreenCenterX = (horizontal / 2);
			float ScreenCenterY = (vertical / 2);

			float TargetX = 0;
			float TargetY = 0;

			auto x = pos.x;
			auto y = pos.y;

			bool has_smooth = smoothing > 0.f;

			if (x != 0)
			{
				if (x > ScreenCenterX)
				{
					TargetX = -(ScreenCenterX - x);
					if (has_smooth)
						TargetX /= smoothing;
					if (TargetX + ScreenCenterX > ScreenCenterX * 2) TargetX = 0;
				}

				if (x < ScreenCenterX)
				{
					TargetX = x - ScreenCenterX;
					if (has_smooth)
						TargetX /= smoothing;
					if (TargetX + ScreenCenterX < 0) TargetX = 0;
				}
			}

			if (y != 0)
			{
				if (y > ScreenCenterY)
				{
					TargetY = -(ScreenCenterY - y);
					if (has_smooth)
						TargetY /= smoothing;
					if (TargetY + ScreenCenterY > ScreenCenterY * 2) TargetY = 0;
				}

				if (y < ScreenCenterY)
				{
					TargetY = y - ScreenCenterY;
					if (has_smooth)
						TargetY /= smoothing;
					if (TargetY + ScreenCenterY < 0) TargetY = 0;
				}
			}

			mouse_event(MOUSEEVENTF_MOVE, static_cast<DWORD>(TargetX), static_cast<DWORD>(TargetY), NULL, NULL);


		}

		vec3_t get_camera_position()
		{

			if (!::game::camera.object)
				return {};

			return ::game::camera.position;

		}

	}

	bool register_functions(asIScriptEngine* engine)
	{

#define CHECK(f) if (f < 0) return false;

		CHECK(engine->SetDefaultNamespace(""));

		// vec3_t
		{

			CHECK(engine->RegisterObjectType("vec3_t", sizeof(vec3_t), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<vec3_t>()));
			CHECK(engine->RegisterObjectBehaviour("vec3_t", asBEHAVE_CONSTRUCT, "void f(float x, float y, float z)", asFUNCTION(construct_vector_3), asCALL_CDECL_OBJLAST));
			CHECK(engine->RegisterObjectBehaviour("vec3_t", asBEHAVE_LIST_CONSTRUCT, "void f(int &in) {float, float, float}", asFUNCTION(construct_list_vector_3), asCALL_CDECL_OBJLAST));

			CHECK(engine->RegisterObjectProperty("vec3_t", "float x", asOFFSET(vec3_t, vec3_t::x)));
			CHECK(engine->RegisterObjectProperty("vec3_t", "float y", asOFFSET(vec3_t, vec3_t::y)));
			CHECK(engine->RegisterObjectProperty("vec3_t", "float z", asOFFSET(vec3_t, vec3_t::z)));

		}

		// vec2_t
		{

			CHECK(engine->RegisterObjectType("vec2_t", sizeof(ImVec2), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<ImVec2>()));
			CHECK(engine->RegisterObjectBehaviour("vec2_t", asBEHAVE_CONSTRUCT, "void f(float x, float y)", asFUNCTION(construct_vector_2), asCALL_CDECL_OBJLAST));
			CHECK(engine->RegisterObjectBehaviour("vec2_t", asBEHAVE_LIST_CONSTRUCT, "void f(int &in) {float, float}", asFUNCTION(construct_list_vector_2), asCALL_CDECL_OBJLAST));

			CHECK(engine->RegisterObjectProperty("vec2_t", "float x", asOFFSET(ImVec2, ImVec2::x)));
			CHECK(engine->RegisterObjectProperty("vec2_t", "float y", asOFFSET(ImVec2, ImVec2::y)));

		}

		// color_t
		{

			CHECK(engine->RegisterObjectType("color_t", sizeof(c_color), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<c_color>()));
			CHECK(engine->RegisterObjectBehaviour("color_t", asBEHAVE_CONSTRUCT, "void f(int r, int g, int b, int a)", asFUNCTION(construct_color), asCALL_CDECL_OBJLAST));
			CHECK(engine->RegisterObjectBehaviour("color_t", asBEHAVE_LIST_CONSTRUCT, "void f(int &in) {int, int, int, int}", asFUNCTION(construct_list_color), asCALL_CDECL_OBJLAST));

			CHECK(engine->RegisterObjectProperty("color_t", "float r", asOFFSET(c_color, c_color::r)));
			CHECK(engine->RegisterObjectProperty("color_t", "float g", asOFFSET(c_color, c_color::g)));
			CHECK(engine->RegisterObjectProperty("color_t", "float b", asOFFSET(c_color, c_color::b)));
			CHECK(engine->RegisterObjectProperty("color_t", "float a", asOFFSET(c_color, c_color::a)));
			CHECK(engine->RegisterObjectProperty("color_t", "bool rainbow", asOFFSET(c_color, c_color::rainbow)));

		}

		// checkbox_t
		{

			CHECK(engine->RegisterObjectType("checkbox_t", sizeof(c_checkbox), asOBJ_REF | asOBJ_NOCOUNT));
			CHECK(engine->RegisterObjectMethod("checkbox_t", "bool get_value()", asMETHOD(c_checkbox, c_checkbox::get_value), asCALL_THISCALL));
			CHECK(engine->RegisterObjectMethod("checkbox_t", "void set_value(bool value)", asMETHOD(c_checkbox, c_checkbox::set_value), asCALL_THISCALL));
			CHECK(engine->RegisterObjectMethod("checkbox_t", "string get_label()", asMETHOD(c_checkbox, c_checkbox::get_label), asCALL_THISCALL));
			CHECK(engine->RegisterObjectMethod("checkbox_t", "void set_label(string value)", asMETHOD(c_checkbox, c_checkbox::set_label), asCALL_THISCALL));

		}

		// slider_float_t
		{

			CHECK(engine->RegisterObjectType("slider_float_t", sizeof(c_slider_float), asOBJ_REF | asOBJ_NOCOUNT));
			CHECK(engine->RegisterObjectMethod("slider_float_t", "float get_value()", asMETHOD(c_slider_float, c_slider_float::get_value), asCALL_THISCALL));
			CHECK(engine->RegisterObjectMethod("slider_float_t", "void set_value(float value)", asMETHOD(c_slider_float, c_slider_float::set_value), asCALL_THISCALL));
			CHECK(engine->RegisterObjectMethod("slider_float_t", "string get_label()", asMETHOD(c_slider_float, c_slider_float::get_label), asCALL_THISCALL));
			CHECK(engine->RegisterObjectMethod("slider_float_t", "void set_label(string value)", asMETHOD(c_slider_float, c_slider_float::set_label), asCALL_THISCALL));

		}

		// slider_int_t
		{

			CHECK(engine->RegisterObjectType("slider_int_t", sizeof(c_slider_int), asOBJ_REF | asOBJ_NOCOUNT));
			CHECK(engine->RegisterObjectMethod("slider_int_t", "int get_value()", asMETHOD(c_slider_int, c_slider_int::get_value), asCALL_THISCALL));
			CHECK(engine->RegisterObjectMethod("slider_int_t", "void set_value(int value)", asMETHOD(c_slider_int, c_slider_int::set_value), asCALL_THISCALL));
			CHECK(engine->RegisterObjectMethod("slider_int_t", "string get_label()", asMETHOD(c_slider_int, c_slider_int::get_label), asCALL_THISCALL));
			CHECK(engine->RegisterObjectMethod("slider_int_t", "void set_label(string value)", asMETHOD(c_slider_int, c_slider_int::set_label), asCALL_THISCALL));

		}

		// combo_t
		{

			CHECK(engine->RegisterObjectType("combo_t", sizeof(c_combo), asOBJ_REF | asOBJ_NOCOUNT));
			CHECK(engine->RegisterObjectMethod("combo_t", "int get_value()", asMETHOD(c_combo, c_combo::get_value), asCALL_THISCALL));
			CHECK(engine->RegisterObjectMethod("combo_t", "void set_value(int value)", asMETHOD(c_combo, c_combo::set_value), asCALL_THISCALL));
			CHECK(engine->RegisterObjectMethod("combo_t", "string get_label()", asMETHOD(c_combo, c_combo::get_label), asCALL_THISCALL));
			CHECK(engine->RegisterObjectMethod("combo_t", "void set_label(string value)", asMETHOD(c_combo, c_combo::set_label), asCALL_THISCALL));

		}

		// multi_combo_t
		{

			CHECK(engine->RegisterObjectType("multi_combo_t", sizeof(c_multi_combo), asOBJ_REF | asOBJ_NOCOUNT));
			CHECK(engine->RegisterObjectMethod("multi_combo_t", "uint8 get_value()", asMETHOD(c_multi_combo, c_multi_combo::get_value), asCALL_THISCALL));
			CHECK(engine->RegisterObjectMethod("multi_combo_t", "void set_value(int idx, uint8 value)", asMETHOD(c_multi_combo, c_multi_combo::set_value), asCALL_THISCALL));
			CHECK(engine->RegisterObjectMethod("multi_combo_t", "string get_label()", asMETHOD(c_multi_combo, c_multi_combo::get_label), asCALL_THISCALL));
			CHECK(engine->RegisterObjectMethod("multi_combo_t", "void set_label(string value)", asMETHOD(c_multi_combo, c_multi_combo::set_label), asCALL_THISCALL));

		}

		// label_t
		{

			CHECK(engine->RegisterObjectType("label_t", sizeof(c_label), asOBJ_REF | asOBJ_NOCOUNT));
			CHECK(engine->RegisterObjectMethod("label_t", "string get_label()", asMETHOD(c_label, c_label::get_label), asCALL_THISCALL));
			CHECK(engine->RegisterObjectMethod("label_t", "void set_label(string value)", asMETHOD(c_label, c_label::set_label), asCALL_THISCALL));

		}

		// divider_t
		{

			CHECK(engine->RegisterObjectType("divider_t", sizeof(c_divider), asOBJ_REF | asOBJ_NOCOUNT));
			CHECK(engine->RegisterObjectMethod("divider_t", "string get_label()", asMETHOD(c_divider, c_divider::get_label), asCALL_THISCALL));
			CHECK(engine->RegisterObjectMethod("divider_t", "void set_label(string value)", asMETHOD(c_divider, c_divider::set_label), asCALL_THISCALL));

		}

		// label_divider_t
		{

			CHECK(engine->RegisterObjectType("label_divider_t", sizeof(c_label_divider), asOBJ_REF | asOBJ_NOCOUNT));
			CHECK(engine->RegisterObjectMethod("label_divider_t", "string get_label()", asMETHOD(c_label_divider, c_label_divider::get_label), asCALL_THISCALL));
			CHECK(engine->RegisterObjectMethod("label_divider_t", "void set_label(string value)", asMETHOD(c_label_divider, c_label_divider::set_label), asCALL_THISCALL));

		}

		// color_picker_t
		{

			CHECK(engine->RegisterObjectType("color_picker_t", sizeof(c_colorpicker), asOBJ_REF | asOBJ_NOCOUNT));
			CHECK(engine->RegisterObjectMethod("color_picker_t", "color_t get_value()", asMETHOD(c_colorpicker, c_colorpicker::get_value), asCALL_THISCALL));
			CHECK(engine->RegisterObjectMethod("color_picker_t", "void set_value(color_t value)", asMETHOD(c_colorpicker, c_colorpicker::set_value), asCALL_THISCALL));
			CHECK(engine->RegisterObjectMethod("color_picker_t", "string get_label()", asMETHOD(c_colorpicker, c_colorpicker::get_label), asCALL_THISCALL));
			CHECK(engine->RegisterObjectMethod("color_picker_t", "void set_label(string value)", asMETHOD(c_colorpicker, c_colorpicker::set_label), asCALL_THISCALL));

		}

		// button_t
		{

			CHECK(engine->RegisterObjectType("button_t", sizeof(c_button), asOBJ_REF | asOBJ_NOCOUNT));
			CHECK(engine->RegisterObjectMethod("button_t", "string get_label()", asMETHOD(c_button, c_button::get_label), asCALL_THISCALL));
			CHECK(engine->RegisterObjectMethod("button_t", "void set_label(string value)", asMETHOD(c_button, c_button::set_label), asCALL_THISCALL));

		}

		// bind_t
		{

			CHECK(engine->RegisterObjectType("bind_t", sizeof(c_binding), asOBJ_VALUE | asOBJ_POD));

			CHECK(engine->RegisterObjectMethod("bind_t", "string get_key()", asMETHOD(c_binding, c_binding::get_key_name), asCALL_THISCALL));
			CHECK(engine->RegisterObjectMethod("bind_t", "string get_name()", asMETHOD(c_binding, c_binding::get_bind_name), asCALL_THISCALL));
			CHECK(engine->RegisterObjectMethod("bind_t", "int get_type()", asMETHOD(c_binding, c_binding::get_bind_type), asCALL_THISCALL));
			CHECK(engine->RegisterObjectMethod("bind_t", "bool get_enabled()", asMETHOD(c_binding, c_binding::get_enabled), asCALL_THISCALL));

		}

		// font_t
		{

			CHECK(engine->RegisterObjectType("font_t", sizeof(scaled_font_t), asOBJ_REF | asOBJ_NOCOUNT));

			CHECK(engine->RegisterObjectBehaviour("font_t", asBEHAVE_FACTORY, "font_t@ f()", asFUNCTION(scaled_font_t::ref_factory), asCALL_CDECL));
			//CHECK(engine->RegisterObjectMethod("font_t", "font_t& opAssign(const font_t& in)", asMETHODPR(scaled_font_t, operator=, (const scaled_font_t&), scaled_font_t&), asCALL_THISCALL))

		}

		// bounding_box_t
		{

			CHECK(engine->RegisterObjectType("bounding_box_t", sizeof(bounding_box_t), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<bounding_box_t>()));

			CHECK(engine->RegisterObjectProperty("bounding_box_t", "float x", asOFFSET(bounding_box_t, bounding_box_t::x)));
			CHECK(engine->RegisterObjectProperty("bounding_box_t", "float y", asOFFSET(bounding_box_t, bounding_box_t::y)));
			CHECK(engine->RegisterObjectProperty("bounding_box_t", "float w", asOFFSET(bounding_box_t, bounding_box_t::w)));
			CHECK(engine->RegisterObjectProperty("bounding_box_t", "float h", asOFFSET(bounding_box_t, bounding_box_t::h)));
			CHECK(engine->RegisterObjectProperty("bounding_box_t", "bool on_screen", asOFFSET(bounding_box_t, bounding_box_t::on_screen)));

		}

		// internal_transform_t
		{

			CHECK(engine->RegisterObjectType("internal_transform_t", sizeof(internal_transform_t), asOBJ_VALUE | asOBJ_POD));
			CHECK(engine->RegisterObjectMethod("internal_transform_t", "vec3_t position()", asMETHOD(internal_transform_t, internal_transform_t::position), asCALL_THISCALL));

		}

		// item_t
		{

			CHECK(engine->RegisterObjectType("item_t", sizeof(cached_item_t), asOBJ_REF));

			CHECK(engine->RegisterObjectBehaviour("item_t", asBEHAVE_FACTORY, "item_t@ f()", asFUNCTION(cached_item_t::ref_factory), asCALL_CDECL));
			CHECK(engine->RegisterObjectBehaviour("item_t", asBEHAVE_ADDREF, "void f()", asMETHOD(cached_item_t, cached_item_t::add_ref), asCALL_THISCALL));
			CHECK(engine->RegisterObjectBehaviour("item_t", asBEHAVE_RELEASE, "void f()", asMETHOD(cached_item_t, cached_item_t::release), asCALL_THISCALL));

			CHECK(engine->RegisterObjectMethod("item_t", "item_t& opAssign(const item_t& in)", asMETHODPR(cached_item_t, operator=, (const cached_item_t&), cached_item_t&), asCALL_THISCALL))

			CHECK(engine->RegisterObjectProperty("item_t", "vec3_t position", asOFFSET(cached_item_t, cached_item_t::position)));

		}

		// player_t
		{

			CHECK(engine->RegisterObjectType("player_t", sizeof(cached_player_t), asOBJ_REF));

			CHECK(engine->RegisterObjectBehaviour("player_t", asBEHAVE_FACTORY, "player_t@ f()", asFUNCTION(cached_player_t::ref_factory), asCALL_CDECL));
			CHECK(engine->RegisterObjectBehaviour("player_t", asBEHAVE_ADDREF, "void f()", asMETHOD(cached_player_t, cached_player_t::add_ref), asCALL_THISCALL));
			CHECK(engine->RegisterObjectBehaviour("player_t", asBEHAVE_RELEASE, "void f()", asMETHOD(cached_player_t, cached_player_t::release), asCALL_THISCALL));

			CHECK(engine->RegisterObjectMethod("player_t", "player_t& opAssign(const player_t& in)", asMETHODPR(cached_player_t, operator=, (const cached_player_t&), cached_player_t&), asCALL_THISCALL));

			CHECK(engine->RegisterObjectMethod("player_t", "uint64 get_player()", asMETHOD(cached_player_t, cached_player_t::get_player), asCALL_THISCALL));
			CHECK(engine->RegisterObjectMethod("player_t", "uint64 get_steam_player()", asMETHOD(cached_player_t, cached_player_t::get_steam_player), asCALL_THISCALL));
			CHECK(engine->RegisterObjectMethod("player_t", "uint64 get_player_look()", asMETHOD(cached_player_t, cached_player_t::get_player_look), asCALL_THISCALL));
			CHECK(engine->RegisterObjectMethod("player_t", "uint64 get_player_movement()", asMETHOD(cached_player_t, cached_player_t::get_player_movement), asCALL_THISCALL));

			CHECK(engine->RegisterObjectProperty("player_t", "string name", asOFFSET(cached_player_t, cached_player_t::name)));
			CHECK(engine->RegisterObjectProperty("player_t", "string weapon_name", asOFFSET(cached_player_t, cached_player_t::weapon_name)));
			CHECK(engine->RegisterObjectProperty("player_t", "vec3_t position", asOFFSET(cached_player_t, cached_player_t::origin)));
			CHECK(engine->RegisterObjectProperty("player_t", "bounding_box_t bounds", asOFFSET(cached_player_t, cached_player_t::bounds)));


		}

		// mono_class_t
		{

			CHECK(engine->RegisterObjectType("mono_class_t", sizeof(mono_class_t), asOBJ_REF | asOBJ_NOCOUNT));
			CHECK(engine->RegisterObjectMethod("mono_class_t", "int get_field_offset(string field_name)", asMETHOD(mono_class_t, mono_class_t::find_field_offset), asCALL_THISCALL));
			CHECK(engine->RegisterObjectMethod("mono_class_t", "uint64 get_static_instance()", asMETHOD(mono_class_t, mono_class_t::get_static_instance), asCALL_THISCALL));

		}

		CHECK(engine->SetDefaultNamespace("menu"));

		{

			CHECK(engine->RegisterGlobalFunction("void remove_element(string label)", asFUNCTION(script_elements::remove_element), asCALL_CDECL));

			CHECK(engine->RegisterGlobalFunction("checkbox_t@ add_checkbox(string label, bool value = false)", asFUNCTION(script_elements::add_checkbox), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("slider_float_t@ add_slider_float(string label, float min, float max, float value = 0.f, string format_text = \"%.1f\")", asFUNCTION(script_elements::add_slider_float), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("slider_int_t@ add_slider_int(string label, int min, int max, int value = 0.f, string format_text = \"%d\")", asFUNCTION(script_elements::add_slider_int), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("combo_t@ add_combo(string label, array<string>@ combo_elements, int value = 0)", asFUNCTION(script_elements::add_combo), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("multi_combo_t@ add_multi_combo(string label, array<string>@ combo_elements)", asFUNCTION(script_elements::add_multi_combo), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("label_t@ add_label(string label)", asFUNCTION(script_elements::add_label), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("divider_t@ add_divider(string label)", asFUNCTION(script_elements::add_divider), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("label_divider_t@ add_label_divider(string label)", asFUNCTION(script_elements::add_label_divider), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("button_t@ add_button(string label)", asFUNCTION(script_elements::add_button), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("color_picker_t@ add_color_picker(string label)", asFUNCTION(script_elements::add_color_picker), asCALL_CDECL));
			
		}

		CHECK(engine->SetDefaultNamespace("cheat"));

		{

			CHECK(engine->RegisterGlobalFunction("void add_log(string text, bool extend_length = false)", asFUNCTION(cheat::add_log), asCALL_CDECL));

			CHECK(engine->RegisterGlobalFunction("array<player_t>@ get_players()", asFUNCTION(cheat::get_players), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("array<item_t>@ get_items()", asFUNCTION(cheat::get_items), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("array<bind_t>@ get_binds()", asFUNCTION(cheat::get_keys), asCALL_CDECL));

		}

		CHECK(engine->SetDefaultNamespace("render"));

		{

			CHECK(engine->RegisterGlobalFunction("void add_rect(vec2_t pos, vec2_t size, color_t color, float rounding = 0.f)", asFUNCTION(render::add_rect), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("void add_rect_outlined(vec2_t pos, vec2_t size, color_t color, float rounding = 0.f)", asFUNCTION(render::add_outlined_rect), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("void add_rect_filled(vec2_t pos, vec2_t size, color_t color, float rounding = 0.f)", asFUNCTION(render::add_rect_filled), asCALL_CDECL));

			CHECK(engine->RegisterGlobalFunction("void add_circle(vec2_t pos, color_t color, float radius)", asFUNCTION(render::add_circle), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("void add_circle_filled(vec2_t pos, color_t color, float radius)", asFUNCTION(render::add_circle_filled), asCALL_CDECL));

			CHECK(engine->RegisterGlobalFunction("void add_text(vec2_t pos, string text, color_t color, int font)", asFUNCTIONPR(render::add_text, (ImVec2, std::string, c_color, int), void), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("void add_text(vec2_t pos, string text, color_t color, font_t@ font)", asFUNCTIONPR(render::add_text, (ImVec2, std::string, c_color, scaled_font_t*), void), asCALL_CDECL));

			CHECK(engine->RegisterGlobalFunction("void add_text_shadowed(vec2_t pos, string text, color_t color, int font)", asFUNCTIONPR(render::add_text_shadowed, (ImVec2, std::string, c_color, int), void), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("void add_text_shadowed(vec2_t pos, string text, color_t color, font_t@ font)", asFUNCTIONPR(render::add_text_shadowed, (ImVec2, std::string, c_color, scaled_font_t*), void), asCALL_CDECL));

			CHECK(engine->RegisterGlobalFunction("font_t@ create_font(string file_name, float size)", asFUNCTION(script_fonts::create_font), asCALL_CDECL));

			CHECK(engine->RegisterGlobalFunction("vec2_t to_screen(vec3_t pos)", asFUNCTION(render::to_screen), asCALL_CDECL));

			CHECK(engine->RegisterGlobalFunction("vec2_t get_cursor_pos()", asFUNCTION(render::get_cursor_pos), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("vec2_t get_display_size()", asFUNCTION(render::get_display_size), asCALL_CDECL));

			CHECK(engine->RegisterGlobalFunction("float get_delta_time()", asFUNCTION(render::get_delta_time), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("double get_time()", asFUNCTION(render::get_time), asCALL_CDECL));

			CHECK(engine->RegisterGlobalFunction("bool area_contains(vec2_t pos, vec2_t size, vec2_t test_pos)", asFUNCTION(render::is_in_bounds), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("bool get_key_state(int key)", asFUNCTION(render::is_key_down), asCALL_CDECL));

		}

		CHECK(engine->SetDefaultNamespace("game"));

		{

			CHECK(engine->RegisterGlobalFunction("mono_class_t@ get_class(string class_name, string assembly_name = \"Assembly-CSharp\")", asFUNCTION(game::get_class), asCALL_CDECL));

			CHECK(engine->RegisterGlobalFunction("int8 read_int8(uint64 address)", asFUNCTION(game::read_int8), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("int16 read_int16(uint64 address)", asFUNCTION(game::read_int16), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("int32 read_int32(uint64 address)", asFUNCTION(game::read_int32), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("int64 read_int64(uint64 address)", asFUNCTION(game::read_int64), asCALL_CDECL));

			CHECK(engine->RegisterGlobalFunction("uint8 read_uint8(uint64 address)", asFUNCTION(game::read_uint8), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("uint16 read_uint16(uint64 address)", asFUNCTION(game::read_uint16), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("uint32 read_uint32(uint64 address)", asFUNCTION(game::read_uint32), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("uint64 read_uint64(uint64 address)", asFUNCTION(game::read_uint64), asCALL_CDECL));

			CHECK(engine->RegisterGlobalFunction("float read_float(uint64 address)", asFUNCTION(game::read_float), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("double read_double(uint64 address)", asFUNCTION(game::read_double), asCALL_CDECL));

			CHECK(engine->RegisterGlobalFunction("bool read_bool(uint64 address)", asFUNCTION(game::read_bool), asCALL_CDECL));

			CHECK(engine->RegisterGlobalFunction("vec3_t read_vec3(uint64 address)", asFUNCTION(game::read_vec3), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("void write_vec3(uint64 address, vec3_t value)", asFUNCTION(game::write_vec3), asCALL_CDECL));

			CHECK(engine->RegisterGlobalFunction("void write_int8(uint64 address, int8 value)", asFUNCTION(game::write_int8), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("void write_int16(uint64 address, int16 value)", asFUNCTION(game::write_int16), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("void write_int32(uint64 address, int32 value)", asFUNCTION(game::write_int32), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("void write_int64(uint64 address, int64 value)", asFUNCTION(game::write_int64), asCALL_CDECL));

			CHECK(engine->RegisterGlobalFunction("void write_uint8(uint64 address, uint8 value)", asFUNCTION(game::write_uint8), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("void write_uint16(uint64 address, uint16 value)", asFUNCTION(game::write_uint16), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("void write_uint32(uint64 address, uint32 value)", asFUNCTION(game::write_uint32), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("void write_uint64(uint64 address, uint64 value)", asFUNCTION(game::write_uint64), asCALL_CDECL));

			CHECK(engine->RegisterGlobalFunction("void write_float(uint64 address, float value)", asFUNCTION(game::write_float), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("void write_double(uint64 address, double value)", asFUNCTION(game::write_double), asCALL_CDECL));

			CHECK(engine->RegisterGlobalFunction("void write_bool(uint64 address, bool value)", asFUNCTION(game::write_bool), asCALL_CDECL));

			CHECK(engine->RegisterGlobalFunction("string read_string(uint64 address)", asFUNCTION(game::read_string), asCALL_CDECL));

			CHECK(engine->RegisterGlobalFunction("array<uint64>@ read_array(uint64 address)", asFUNCTION(game::read_array), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("array<uint64>@ read_multi_dimensional_array(uint64 address)", asFUNCTION(game::read_multi_dimensional_array), asCALL_CDECL));

			CHECK(engine->RegisterGlobalFunction("vec3_t get_transform_position(uint64 transform)", asFUNCTION(game::get_transform_position), asCALL_CDECL));

			CHECK(engine->RegisterGlobalFunction("void move_mouse(vec2_t pos, float smoothing = 0.f)", asFUNCTION(game::move_mouse), asCALL_CDECL));
			CHECK(engine->RegisterGlobalFunction("vec3_t get_camera_position()", asFUNCTION(game::get_camera_position), asCALL_CDECL));

		}

#undef CHECK

		return true;

	}

}