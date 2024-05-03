#pragma once

#include "../includes.h"

#include "jsoncpp/json.h"
#include "jsoncpp/json-forwards.h"

#include <shlobj_core.h>

#include "../logs/logs.h"

struct config_item_t
{

	std::string name;
	std::string path;
	std::string last_edit;
	int id;

};

class c_config_manager
{

private:

	std::vector<config_item_t> m_config_items{};

public:

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

	void refresh_container(c_child* container, int* id_selection)
	{

		container->clear_elements();

		for (auto config_item : m_config_items)
			container->insert_element<c_chip_panel>(config_item.name, config_item.last_edit, config_item.id, id_selection);

	}

	void refresh_configs()
	{

		m_config_items.clear();

		int cur_id = -1;
		std::string config_folder = get_documents_path() + "\\ann_configs";

		for (auto& entry : std::filesystem::directory_iterator(config_folder))
		{

			if (entry.path().extension() == ".cfg")
			{

				std::filesystem::path file_path = entry.path();
				std::string file_name = file_path.filename().string();
				std::string file_path_string = file_path.string();
				std::ifstream config_file(file_path_string, std::ifstream::binary);
				std::string last_edit_time = "-1";

				if (config_file.good())
				{

					if (std::filesystem::file_size(file_path_string) < 30u)
						continue;

					Json::Value config_text;
					config_file >> config_text;

					if (config_text["last_edited"].isNull() || config_text["game_title"].isNull())
					{

						MessageBoxA(NULL, "old config file in config folder, please delete", NULL, NULL);
						continue;

					}

					if (config_text["game_title"].asString() != "unturned")
						continue;

					last_edit_time = config_text["last_edited"].asString();

					config_file.close();

				}

				cur_id += 1;

				m_config_items.emplace_back<config_item_t>({ file_name, file_path_string, last_edit_time, cur_id});

			}

		}

	}

	void create_config(c_child* container, c_window* window, int* id_selection, const char* config_name)
	{

		std::string path = get_documents_path() + "\\ann_configs\\";
		path += config_name;
		path += ".cfg";

		std::string file_name = config_name;
		file_name += ".cfg";

		std::ofstream file(path);

		m_config_items.emplace_back<config_item_t>({ file_name, path, std::to_string(time(NULL)), (int)m_config_items.size()});

		for (auto config_item : m_config_items)
			if (config_item.name == file_name)
				save_config(window, config_item.id);

		g_logs->add_log("created " + std::string(file_name));

	}

	void delete_config(c_child* container, int id, int* id_selection)
	{

		if (id >= m_config_items.size())
			return;

		g_logs->add_log("deleted " + std::string(m_config_items.at(id).name));

		remove(m_config_items.at(id).path.c_str());
		refresh_configs();

		*id_selection = 0;

	}

	void save_config(c_window* window, int id)
	{

		if (id >= m_config_items.size())
			return;

		std::ofstream config_file(m_config_items.at(id).path, std::ifstream::binary);
		Json::Value config_data;
		
		auto last_edit_time = std::to_string(time(NULL));

		config_data["game_title"] = "unturned";
		config_data["last_edited"] = last_edit_time;

		auto config_items = window->get_all_cfg_elements();

		for (auto config_item : config_items)
		{

			auto label = config_item->get_label();

			if (config_item->get_type() == element_type::CHECKBOX)
				config_data[label] = ((c_checkbox*)config_item)->get_value();
			else if (config_item->get_type() == element_type::SLIDER_FLOAT)
				config_data[label] = ((c_slider_float*)config_item)->get_value();
			else if (config_item->get_type() == element_type::SLIDER_INT)
				config_data[label] = ((c_slider_int*)config_item)->get_value();
			else if (config_item->get_type() == element_type::SINGLE_COMBO)
				config_data[label] = ((c_combo*)config_item)->get_value();
			else if (config_item->get_type() == element_type::COLORPICKER)
			{
				auto colorpicker = ((c_colorpicker*)config_item);

				config_data[label] = colorpicker->get_value().u32();
				config_data[label + "_rainbow"] = colorpicker->get_value().rainbow;

			}
			else if (config_item->get_type() == element_type::KEYBIND)
			{

				auto bind = ((c_keybind*)config_item)->get_bind();

				config_data[label + "_key"] = bind->get_bound_key();
				config_data[label + "_type"] = bind->get_bind_type();

			}
			else if (config_item->get_type() == element_type::MULTI_COMBO)
			{

				auto combo = ((c_multi_combo*)config_item);

				for (int i = 0; i < combo->get_values().size(); i++)
					config_data[label + std::to_string(i)] = combo->get_value(i);

			}
			else if (config_item->get_type() == element_type::INPUT_TEXT)
			{

				auto text = ((c_input_text*)config_item);

				config_data[label] = text->get_value();

			}

		}

		for (auto script : g_script_engine->get_scripts())
			if (script.state == STATE_RUNNING)
				config_data[script.name] = script.name;

		config_file.clear();

		config_file << config_data;

		config_file.close();

		m_config_items.at(id).last_edit = last_edit_time;

		g_logs->add_log("saved " + std::string(m_config_items.at(id).name));

	}

	void load_config(c_window* window, int id)
	{

		if (id >= m_config_items.size())
			return;

		std::ifstream config_file(m_config_items.at(id).path, std::ifstream::binary);
		Json::Value config_text;
		config_file >> config_text;

		auto config_members = config_text.getMemberNames();

		g_script_engine->reload_scripts();

		g_script_engine->refresh_container(script_interface::main_child, &script_interface::current_script);

		for (auto script : g_script_engine->get_scripts())
		{

			for (auto config_member : config_members)
				if (config_member == script.name)
					g_script_engine->run_script(script.path);

		}

		auto config_items = window->get_all_cfg_elements();

		for (auto config_item : config_items)
		{

			auto label = config_item->get_label();

			if (config_item->get_type() == element_type::CHECKBOX)
				((c_checkbox*)config_item)->set_value(config_text[label].asBool());
			else if (config_item->get_type() == element_type::SLIDER_FLOAT)
				((c_slider_float*)config_item)->set_value(config_text[label].asFloat());
			else if (config_item->get_type() == element_type::SLIDER_INT)
				((c_slider_int*)config_item)->set_value(config_text[label].asInt());
			else if (config_item->get_type() == element_type::SINGLE_COMBO)
				((c_combo*)config_item)->set_value(config_text[label].asInt());
			else if (config_item->get_type() == element_type::COLORPICKER)
			{

				auto colorpicker = ((c_colorpicker*)config_item);

				colorpicker->set_value(from_u32(config_text[label].asUInt()));
				colorpicker->set_rainbow(config_text[label + "_rainbow"].asBool());

			}
			else if (config_item->get_type() == element_type::KEYBIND)
			{

				auto bind = ((c_keybind*)config_item)->get_bind();

				bind->set_key(config_text[label + "_key"].asInt());
				bind->set_mode((keybind_interaction_type)config_text[label + "_type"].asInt());

			}
			else if (config_item->get_type() == element_type::MULTI_COMBO)
			{

				auto combo = ((c_multi_combo*)config_item);

				for (int i = 0; i < combo->get_values().size(); i++)
					combo->set_value(i, config_text[label + std::to_string(i)].asInt());

			}
			else if (config_item->get_type() == element_type::INPUT_TEXT && config_text.isMember(label))
			{

				auto text = ((c_input_text*)config_item);

				text->set_value(config_text[label].asString());

			}

		}

		g_logs->add_log("loaded " + std::string(m_config_items.at(id).name));

	}

}; inline c_config_manager* g_config_manager = new c_config_manager();