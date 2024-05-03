#pragma once

#include <cstdint>
#include <string>
#include <sstream>
#include <Windows.h>
#include <filesystem>
#include <iostream>

#include <shlobj_core.h>

#include <angelscript.h>

#include "angelscript/scriptbuilder/scriptbuilder.h"
#include "angelscript/scriptstdstring/scriptstdstring.h"
#include "angelscript/scriptmath/scriptmath.h"
#include "angelscript/scriptarray/scriptarray.h"

#include "interface.h"

#include "../logs/logs.h"

enum SCRIPT_STATE : int
{

	STATE_IDLE,
	STATE_RUNNING

};

struct script_callback_t
{

	asIScriptFunction* function;
	std::string callback_name;

};

struct script_t
{

	std::string name = "none";
	std::string path = "none";
	std::vector<script_callback_t> callbacks{};
	std::vector<scaled_font_t> created_fonts{};
	std::vector<c_element*> created_elements{};
	asIScriptModule* script_module;
	int id;
	SCRIPT_STATE state;
	bool risky_features_enabled = false;

	inline bool operator==(script_t b) { return (this->name == b.name) && (this->path == b.path); }

};

struct execution_context_t
{

	asIScriptContext* context;
	asIScriptFunction* function;

};

void message_callback(const asSMessageInfo* msg, void* param)
{

	std::string type = "error";
	if (msg->type == asMSGTYPE_WARNING)
		type = "warning";
	else if (msg->type == asMSGTYPE_INFORMATION)
		type = "information";

	if (type == "information")
		return;

	std::stringstream ss;

	std::string message = msg->message;

	std::transform(message.begin(), message.end(), message.begin(), [](unsigned char c) { return std::tolower(c); });

	ss << msg->section << " (" << msg->row << ", " << msg->col << ") | " << type << " " << message;

	g_logs->add_log(ss.str(), true);

}

const std::vector<std::string> g_callback_names{ "render", "update", "load", "unload" };

class c_script_engine
{

private:

	asIScriptEngine* m_script_engine{};
	std::vector<script_t> m_scripts{};
	std::vector<execution_context_t> m_execution_contexts{};
	script_t* m_current_script;

	std::string get_documents_path()
	{

		PWSTR   ppszPath;
		HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &ppszPath);

		std::wstring myPath = L"";

		if (SUCCEEDED(hr))
			myPath = ppszPath;
		else if (!SUCCEEDED(hr))
			MessageBoxA(NULL, "failed to get documents path", NULL, NULL);

		CoTaskMemFree(ppszPath);
		return std::string(myPath.begin(), myPath.end()) + "\\";

	}

	void register_callback(int id, std::string callback_name, asIScriptFunction* callback_function)
	{

		for (auto& script : m_scripts)
			if (script.id == id)
			{

				for (const auto& callback : script.callbacks)
					if (callback.callback_name == callback_name)
						return; // already has callback

				script.callbacks.emplace_back<script_callback_t>({ callback_function, callback_name });

			}

	}

public:

	bool initialize()
	{

		m_script_engine = asCreateScriptEngine();

		if (!m_script_engine)
			return false;

		if (m_script_engine->SetMessageCallback(asFUNCTION(message_callback), 0, asCALL_CDECL) < 0)
			return false;

		RegisterStdString(m_script_engine);

		RegisterScriptArray(m_script_engine, false);

		RegisterScriptMath(m_script_engine);

		if (!script_interface::register_functions(m_script_engine))
			return false;

		return true;

	}

	bool run_script(std::string path)
	{

		FILE* file;

		fopen_s(&file, path.c_str(), "rb");
		size_t filesize;

		if (!file)
			return false;

		script_t target_script;
		int script_idx = 0;

		for (const auto& script : m_scripts)
		{

			if (script.path == path)
			{

				target_script = script;
				script_idx = script.id;
				continue;

			}

		}

		if (target_script.path == "none" || target_script.state == STATE_RUNNING)
			return false;

		fseek(file, 0, SEEK_END);
		filesize = ftell(file);
		fseek(file, 0, SEEK_SET);

		std::string script_content;

		script_content.resize(filesize);
		size_t c = fread(&script_content[0], filesize, 1, file);

		fclose(file);

		if (!c)
			return false;

		asIScriptModule* script_module = m_script_engine->GetModule(path.c_str(), asGM_ALWAYS_CREATE);

		if (!script_module)
			return false;

		int result = script_module->AddScriptSection(target_script.name.c_str(), &script_content[0], filesize);

		if (result < 0)
			return false;

		m_current_script = &m_scripts.at(script_idx);

		result = script_module->Build();

		if (result < 0)
			return false;

		for (const auto& callback : g_callback_names)
		{

			auto callback_function = script_module->GetFunctionByName(callback.c_str());

			if (!callback_function)
				continue;

			register_callback(target_script.id, callback, callback_function);

		}

		for (const auto& callback : m_scripts.at(target_script.id).callbacks)
			if (callback.callback_name == "load")
				run_function(callback.function);

		m_scripts.at(target_script.id).state = STATE_RUNNING;
		m_scripts.at(target_script.id).script_module = script_module;

		g_logs->add_log("loaded " + target_script.name);

		return true;

	}

	bool run_script(int id)
	{

		script_t ret;

		for (const auto& script : m_scripts)
			if (script.id == id)
				ret = script;

		if (ret.path == "none")
			return false;

		return run_script(ret.path);

	}

	bool unload_script(int id)
	{

		if (id > m_scripts.size())
			return false;

		script_t& script = m_scripts.at(id);

		if (script.state != STATE_RUNNING)
			return false;

		for (const auto& callback : m_scripts.at(id).callbacks)
			if (callback.callback_name == "unload")
				run_function(callback.function);

		script.script_module->Discard();

		for (const auto& callback : script.callbacks)
			for (int i = 0; i < m_execution_contexts.size(); i++)
			{

				auto context = m_execution_contexts.at(i);

				if (context.function == callback.function)
				{

					context.context->Release();
					m_execution_contexts.erase(m_execution_contexts.begin() + i);

				}

			}

		framework::g_font_mtx.lock();

		for (auto font : script.created_fonts)
		{

			auto ite = std::find(framework::g_scaled_fonts.begin(), framework::g_scaled_fonts.end(), font);

			if (ite != framework::g_scaled_fonts.end())
				framework::g_scaled_fonts.erase(ite);

		}

		framework::g_font_mtx.unlock();

		for (const auto& element : script.created_elements)
			script_interface::element_child->pop_element(element);

		script.callbacks.clear();
		script.created_fonts.clear();
		script.created_elements.clear();

		script.state = STATE_IDLE;

		g_logs->add_log("unloaded " + script.name);

		return true;

	}

	void reload_scripts()
	{

		for (auto& script : m_scripts)
			unload_script(script.id);

		m_scripts.clear();

		int cur_id = 0;
		std::string config_folder = get_documents_path() + "\\ann_scripts";

		for (auto& entry : std::filesystem::directory_iterator(config_folder))
		{

			if (entry.path().extension() == ".as")
			{

				std::filesystem::path file_path = entry.path();

				std::string file_name = file_path.filename().string();
				std::string file_path_string = file_path.string();

				std::ifstream config_file(file_path_string, std::ifstream::binary);

				if (config_file.good())
				{

					if (std::filesystem::file_size(file_path_string) < 1u)
						continue;

				}
				else
					continue;

				m_scripts.emplace_back<script_t>({ file_name, file_path_string, {}, {}, {}, nullptr, cur_id });

				cur_id++;

			}

		}

	}

	void run_function(asIScriptFunction* func, void* args = nullptr)
	{

		asIScriptContext* execution_context = nullptr;

		for (const auto& context : m_execution_contexts)
			if (context.function == func)
				execution_context = context.context;
		
		if (!execution_context)
		{

			execution_context = m_script_engine->CreateContext();
			m_execution_contexts.emplace_back<execution_context_t>({ execution_context, func });

		}

		int ret = execution_context->Prepare(func);

		if (args != nullptr)
			ret = execution_context->SetArgAddress(0, args);

		script_interface::current_callback = func->GetName();

		execution_context->Execute();

	}

	void refresh_container(c_child* container, int* id_selection);

	std::vector<script_t> get_scripts() const { return m_scripts; }

	script_t* get_current_script() const { return m_current_script; }

	asIScriptEngine* get_script_engine() const { return m_script_engine; }

}; inline c_script_engine* g_script_engine = new c_script_engine();

namespace script_fonts
{

	scaled_font_t* create_font(std::string file_name, float size)
	{

		auto font_name = std::filesystem::path(file_name).stem().string();

		for (auto font : framework::g_scaled_fonts)
		{

			if (font.size == size)
			{

				auto second_font_name = std::filesystem::path(font.file_name).stem().string();

				if (font_name == second_font_name)
					return &font;

			}

		}

		framework::g_font_mtx.lock();
		scaled_font_t* new_font = &framework::g_scaled_fonts.emplace_back<scaled_font_t>({ file_name, size });
		framework::g_font_mtx.unlock();

		framework::needs_to_build_fonts = true;

		auto current_script = g_script_engine->get_current_script();
		
		if (current_script)
			current_script->created_fonts.push_back(*new_font);

		return new_font;

	}

}

namespace script_elements
{

	c_checkbox* add_checkbox(std::string label, bool value)
	{

		auto returned_element = script_interface::element_child->insert_element<c_checkbox>(label);
		returned_element->set_value(value);
		
		auto current_script = g_script_engine->get_current_script();

		if (current_script)
			current_script->created_elements.push_back(returned_element);

		return returned_element;

	}

	c_slider_float* add_slider_float(std::string label, float min, float max, float value, std::string format_text)
	{

		auto returned_element = script_interface::element_child->insert_element<c_slider_float>(label, min, max, format_text);
		returned_element->set_value(value);

		auto current_script = g_script_engine->get_current_script();

		if (current_script)
			current_script->created_elements.push_back(returned_element);

		return returned_element;

	}

	c_slider_int* add_slider_int(std::string label, int min, int max, int value, std::string format_text)
	{

		auto returned_element = script_interface::element_child->insert_element<c_slider_int>(label, min, max, format_text);
		returned_element->set_value(value);

		auto current_script = g_script_engine->get_current_script();

		if (current_script)
			current_script->created_elements.push_back(returned_element);

		return returned_element;

	}

	c_combo* add_combo(std::string label, CScriptArray* combo_elements, int value)
	{

		std::vector<std::string> elements{};

		for (asUINT i = 0; i < combo_elements->GetSize(); i++)
			elements.push_back(*(std::string*)combo_elements->At(i));

		auto returned_element = script_interface::element_child->insert_element<c_combo>(label, elements);
		returned_element->set_value(value);

		auto current_script = g_script_engine->get_current_script();

		if (current_script)
			current_script->created_elements.push_back(returned_element);

		return returned_element;

	}

	c_multi_combo* add_multi_combo(std::string label, CScriptArray* combo_elements)
	{

		std::vector<std::string> elements{};

		for (asUINT i = 0; i < combo_elements->GetSize(); i++)
			elements.push_back(*(std::string*)combo_elements->At(i));

		auto returned_element = script_interface::element_child->insert_element<c_multi_combo>(label, elements);

		auto current_script = g_script_engine->get_current_script();

		if (current_script)
			current_script->created_elements.push_back(returned_element);

		return returned_element;

	}

	c_label* add_label(std::string label)
	{

		auto returned_element = script_interface::element_child->insert_element<c_label>(label);

		auto current_script = g_script_engine->get_current_script();

		if (current_script)
			current_script->created_elements.push_back(returned_element);

		return returned_element;

	}

	c_divider* add_divider(std::string label)
	{

		auto returned_element = script_interface::element_child->insert_element<c_divider>(label);

		auto current_script = g_script_engine->get_current_script();

		if (current_script)
			current_script->created_elements.push_back(returned_element);

		return returned_element;

	}

	c_label_divider* add_label_divider(std::string label)
	{

		auto returned_element = script_interface::element_child->insert_element<c_label_divider>(label);

		auto current_script = g_script_engine->get_current_script();

		if (current_script)
			current_script->created_elements.push_back(returned_element);

		return returned_element;

	}

	c_button* add_button(std::string label)
	{

		auto returned_element = script_interface::element_child->insert_element<c_button>(label);

		auto current_script = g_script_engine->get_current_script();

		if (current_script)
			current_script->created_elements.push_back(returned_element);

		return returned_element;

	}

	c_colorpicker* add_color_picker(std::string label)
	{

		auto returned_element = script_interface::element_child->insert_element<c_colorpicker>(label);

		auto current_script = g_script_engine->get_current_script();

		if (current_script)
			current_script->created_elements.push_back(returned_element);

		return returned_element;

	}

	void remove_element(std::string label) { script_interface::element_child->pop_element(label); }

}