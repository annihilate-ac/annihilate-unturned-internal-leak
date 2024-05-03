#define _CRT_SECURE_NO_WARNINGS

#include "includes.h"

#include "networking/mylar.h"

#include "sdk/game.h"

#include "imgui/imgui_freetype.h"
#include "framework/framework.h"

#include "render.h"

#include "features/aimbot.h"
#include "features/players.h"
#include "features/weapons.h"
#include "features/visuals.h"

#include "scripting/scripting.h"

#include "config/config.h"

#include "sounds/custom_sounds.hpp"

#include "kiero/minhook/include/MinHook.h"

c_window cheat_window("cheat_window", { 560.f, 419.f }, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
c_window preview_window("esp_preview", { 300.f, 419.f }, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

c_window weapon_panel_window("weapon_panel", { 0.f, 0.f }, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysAutoResize);

auto top_bar_sz = 28.f;

bool menu_open = true;
bool init_hook = false;

c_animator<float> loading_alpha_animator{ 0.f, 1.f, 0.86f };
c_animator<float> alpha_animator{ 0.f, 1.f, 0.26f };
c_animator<float> child_alpha_animator{ 0.f, 1.f, 0.17f };
c_animator<float> counter_animator{ 0.f, 0.f, 0.25f };
c_animator<float> preview_animator{ 0.f, 1.f, 0.46f };

c_child* preset_child = nullptr;

int current_config = 0;
int current_tab = 0;

mylar::local_client_t client{};

ULONGLONG start_time = NULL;
bool finished_start = false;

int items_through = 0;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Present oPresent;
HWND window = NULL;
WNDPROC oWndProc;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;
static ID3D11ShaderResourceView* g_peter_png = nullptr;

std::string user_name = "";

void c_script_engine::refresh_container(c_child* container, int* id_selection)
{

    container->clear_elements("refresh scripts");

    auto refresh_button = container->find_element<c_button>("refresh scripts");

    refresh_button->set_interaction_callback([]() -> void {
        g_script_engine->reload_scripts();
        g_script_engine->refresh_container(script_interface::main_child, &script_interface::current_script);
        });

    auto script_button = container->find_element<c_button>("open scripts folder");

    script_button->set_interaction_callback([]() -> void {
        ShellExecuteA(NULL, "open", std::string(g_config_manager->get_documents_path() + "\\ann_scripts").c_str(), NULL, NULL, SW_SHOWNORMAL);
    });

    for (int i = 0; i < m_scripts.size(); i++)
    {

        auto& script = m_scripts.at(i);

        auto added_container = container->insert_element<c_chip_panel>(script.name, "script", script.id, id_selection);
        added_container->set_attached_script(&script);

    }

}

namespace widgets
{

    c_combo* dpi_combo = nullptr;
    c_keybind* menu_bind = nullptr; c_binding menu_key{ "menu key", VK_DELETE, keybind_interaction_type::TOGGLE };
    c_checkbox* fps_counter_checkbox = nullptr;

    c_checkbox* fov_changer_checkbox = nullptr;
    c_slider_float* fov_factor_slider = nullptr;
    c_keybind* fov_factor_bind = nullptr; c_binding fov_factor_key{ "fov factor", VK_XBUTTON1, keybind_interaction_type::HELD };
    c_slider_float* fov_factor_key_slider = nullptr;

    c_checkbox* instant_equip_checkbox = nullptr;
    c_checkbox* weapon_info_panel_checkbox = nullptr;

    c_checkbox* no_exit_timer_checkbox = nullptr;
    c_checkbox* allow_compass_checkbox = nullptr;
    c_checkbox* allow_gps_checkbox = nullptr;
    c_checkbox* admin_flag_checkbox = nullptr;
    c_combo* hitsound_combo = nullptr;
    c_slider_int* hitsound_volume_slider = nullptr;
    c_input_text* custom_hitsound_name_input = nullptr;
    c_combo* time_changer_combo = nullptr;
    c_checkbox* allow_thirdperson_checkbox = nullptr;

    c_combo* force_hitbox_combo = nullptr;

    c_combo* chams_combo = nullptr;
    c_checkbox* chams_through_walls_checkbox = nullptr;
    c_colorpicker* chams_colorpicker = nullptr;

    c_checkbox* local_player_chams_checkbox = nullptr;
    c_colorpicker* local_player_chams_colorpicker = nullptr;

    c_checkbox* hand_chams_checkbox = nullptr;
    c_colorpicker* hand_chams_colorpicker = nullptr;

    c_checkbox* force_3d_hitmarkers_checkbox = nullptr;

    c_input_text* config_name_input = nullptr;

    c_checkbox* freecam_checkbox = nullptr;
    c_keybind* freecam_bind = nullptr; c_binding freecam_key{ "freecam", VK_XBUTTON2, keybind_interaction_type::TOGGLE };
    c_slider_float* freecam_speed_slider = nullptr;

    c_checkbox* sky_color_checkbox = nullptr;
    c_colorpicker* sky_color_picker = nullptr;

    c_checkbox* viewmodel_changer_checkbox = nullptr;

    c_slider_float* viewmodel_aim_fov_slider = nullptr;
    c_slider_float* viewmodel_hip_fov_slider = nullptr;

    c_slider_float* viewmodel_x_pos_slider = nullptr;
    c_slider_float* viewmodel_y_pos_slider = nullptr;
    c_slider_float* viewmodel_z_pos_slider = nullptr;

    c_checkbox* disable_pain_checkbox = nullptr;
    c_checkbox* disable_stun_checkbox = nullptr;
    c_checkbox* disable_hallucinations_checkbox = nullptr;

}

__forceinline void setup_combat_tab()
{

    static auto show_if_tab = []() -> bool {
        return current_tab == 0 && child_alpha_animator.get_starting_animation();
    };

    auto main_child = cheat_window.insert_child("aimbot", { 21.f, top_bar_sz + 22.f }, { 249.f, 348.f });
    {

        auto general_group = main_child->insert_element<c_collapsable_group>("general");
        {

            widgets::aimbot_checkbox = general_group->insert_element<c_checkbox>("enable aimbot");

            widgets::aimbot_keybind = general_group->insert_element<c_keybind>("aimbot hotkey", &widgets::aimbot_key);

            widgets::aimbot_type_combo = general_group->insert_element<c_combo>("aimbot type", std::vector<std::string>{ "viewangles", "silent" });

            widgets::aimbot_smoothing_combo = general_group->insert_element<c_combo>("angle smoothing", std::vector<std::string>({ "off", "fast", "slow" }));

            widgets::aimbot_smoothing_slider = general_group->insert_element<c_slider_float>("smoothing amount", 1.f, 100.f, "%1.f%%");
            widgets::aimbot_smoothing_slider->set_visibility_callback([]() -> bool { return widgets::aimbot_smoothing_combo->get_value() > 0; });
            widgets::aimbot_smoothing_slider->set_value(35.f);

        }

        auto targeting_group = main_child->insert_element<c_collapsable_group>("targeting");
        {

            widgets::aimbot_hitbox_combo = targeting_group->insert_element<c_combo>("target hitbox", std::vector<std::string>{ "head", "chest", "random" });

            widgets::aimbot_hitbox_override_combo = targeting_group->insert_element<c_combo>("target hitbox override", std::vector<std::string>{ "off", "head", "chest", "random" });

            widgets::aimbot_hitbox_override_keybind = targeting_group->insert_element<c_keybind>("hitbox override hotkey", &widgets::aimbot_hitbox_override_key);

            widgets::aimbot_hitbox_override_keybind->set_visibility_callback([]() -> bool {
                return widgets::aimbot_hitbox_override_combo->get_value() > 0;
                });

            widgets::aimbot_avoid_friendly_checkbox = targeting_group->insert_element<c_checkbox>("avoid friendlies");

            widgets::aimbot_only_visible = targeting_group->insert_element<c_checkbox>("only target visible");

            widgets::aimbot_fov_slider = targeting_group->insert_element<c_slider_float>("max target fov", 10.f, 1001.f, "%1.f deg");
            widgets::aimbot_fov_slider->set_value(100.f);

            widgets::aimbot_fov_slider->set_render_callback([]() -> void {
                if (widgets::aimbot_fov_slider->get_value() == 1001.f)
                    widgets::aimbot_fov_slider->set_format_text("ignore fov");
                else if (widgets::aimbot_fov_slider->get_format_text() == "ignore fov" && widgets::aimbot_fov_slider->get_value() != 1001.f)
                    widgets::aimbot_fov_slider->set_format_text("%1.f deg");
                });

            widgets::aimbot_fov_slider->set_has_tooltip(true);

            widgets::aimbot_fov_slider->set_tooltip("set to the maximum for aimbot to ignore fov");

            widgets::aimbot_max_distance_slider = targeting_group->insert_element<c_slider_float>("max target distance", 0.f, 1000.f, "%1.fft");

            widgets::aimbot_max_distance_slider->set_render_callback([]() -> void {
                if (widgets::aimbot_max_distance_slider->get_value() == 0.f)
                    widgets::aimbot_max_distance_slider->set_format_text("auto");
                else if (widgets::aimbot_max_distance_slider->get_format_text() == "auto" && widgets::aimbot_max_distance_slider->get_value() != 0.f)
                {

                    auto format_text = std::string("%1.f");
                    format_text += get_unit_text();

                    widgets::aimbot_max_distance_slider->set_format_text(format_text.c_str());

                }
            });

            widgets::aimbot_max_distance_slider->set_value(0.f);

            widgets::aimbot_max_distance_slider->set_has_tooltip(true);
            widgets::aimbot_max_distance_slider->set_tooltip("set to 0 for automatic distance");

        }

        auto extras_group = main_child->insert_element<c_collapsable_group>("extras");
        {

            widgets::aimbot_sorting_combo = extras_group->insert_element<c_combo>("sorting type", std::vector<std::string>{ "fov", "smart" });

            widgets::aimbot_ignore_occlusion_checkbox = extras_group->insert_element<c_checkbox>("ignore occlusion");

            widgets::aimbot_ignore_occlusion_checkbox->set_has_tooltip(true);

            widgets::aimbot_ignore_occlusion_checkbox->set_tooltip("enable to allow silent aimbot to penetrate walls");

            widgets::aimbot_auto_shoot_checkbox = extras_group->insert_element<c_checkbox>("auto shoot");

        }

        auto visuals_group = main_child->insert_element<c_collapsable_group>("visual options");
        {

            widgets::aimbot_draw_fov_checkbox = visuals_group->insert_element<c_checkbox>("draw aimbot fov");

            widgets::aimbot_fov_color = visuals_group->insert_element<c_colorpicker>("aimbot fov color");
            widgets::aimbot_fov_color->set_visibility_callback([]() -> bool { return widgets::aimbot_draw_fov_checkbox->get_value(); });

            widgets::aimbot_target_color_override_checkbox = visuals_group->insert_element<c_checkbox>("override target color");

            widgets::aimbot_target_color_override_color = visuals_group->insert_element<c_colorpicker>("target color");

            widgets::aimbot_target_color_override_color->set_visibility_callback([]() -> bool { return widgets::aimbot_target_color_override_checkbox->get_value(); });

            widgets::aimbot_show_target_combo = visuals_group->insert_element<c_multi_combo>("target visualization", std::vector<std::string>{ "under crosshair", "aim line", "hit cross" });

            widgets::aimbot_show_target_size = visuals_group->insert_element<c_slider_float>("cross size", 3.f, 16.f, "%.fpx");
            widgets::aimbot_show_target_size->set_visibility_callback([]() -> bool { return widgets::aimbot_show_target_combo->get_value(2); });
            widgets::aimbot_show_target_size->set_value(6.f);

            widgets::aimbot_target_color = visuals_group->insert_element<c_colorpicker>("visual color");
            widgets::aimbot_target_color->set_visibility_callback([]() -> bool { return widgets::aimbot_show_target_combo->get_active_values(); });

        }

    } main_child->set_visibility_callback(show_if_tab);

    auto other_child = cheat_window.insert_child("weapon", { 249.f + 41.f, top_bar_sz + 22.f }, { 249.f, 348.f });
    {

        widgets::no_sway_checkbox = other_child->insert_element<c_checkbox>("no sway");

        widgets::no_recoil_checkbox = other_child->insert_element<c_checkbox>("no recoil");

        widgets::no_recoil_percentage_slider = other_child->insert_element<c_slider_float>("recoil percentage", 0.f, 100.f, "%.f%%");

        widgets::no_recoil_percentage_slider->set_value(50.f);

        widgets::no_recoil_percentage_slider->set_visibility_callback([]() -> bool {
            return widgets::no_recoil_checkbox->get_value();
            });

        widgets::no_spread_checkbox = other_child->insert_element<c_checkbox>("no spread");

        widgets::force_hitbox_combo = other_child->insert_element<c_combo>("force hitbox", std::vector<std::string>{ "off", "head", "chest" });

        widgets::force_hitbox_combo->set_has_tooltip(true);

        widgets::force_hitbox_combo->set_tooltip("overrides all shots to have hit this hitbox");

        widgets::extend_range_combo = other_child->insert_element<c_multi_combo>("range extender", std::vector<std::string>{ "fists", "melee" });

        widgets::extend_range_combo->set_has_tooltip(true);

        widgets::extend_range_combo->set_tooltip("extends the useable range of your fists/melee to 26ft");

        widgets::no_shake_checkbox = other_child->insert_element<c_checkbox>("no shake");

        widgets::no_drop_checkbox = other_child->insert_element<c_checkbox>("no bullet drop");

        widgets::fast_bullet_checkbox = other_child->insert_element<c_checkbox>("fast bullet");

        widgets::instant_aim_checkbox = other_child->insert_element<c_checkbox>("instant aim");

        widgets::no_animations_checkbox = other_child->insert_element<c_checkbox>("disable animations");

        widgets::instant_equip_checkbox = other_child->insert_element<c_checkbox>("instant equip");

    } other_child->set_visibility_callback(show_if_tab);

}

__forceinline void setup_visuals_tab()
{

    static auto show_if_tab = []() -> bool {
        return current_tab == 1 && child_alpha_animator.get_starting_animation();
        };

    auto main_child = cheat_window.insert_child("players", { 21.f, top_bar_sz + 22.f }, { 249.f, 348.f });
    {

        widgets::player_esp_keybind = main_child->insert_element<c_keybind>("player esp hotkey", &widgets::player_esp_key);

        widgets::esp_max_distance_slider = main_child->insert_element<c_slider_float>("max render distance", 1.f, 8000.f, "%1.fft");
        widgets::esp_max_distance_slider->set_value(1600.f);

        widgets::esp_max_distance_slider->set_render_callback([]() -> void {
            auto format_text = std::string("%1.f");
            format_text += get_unit_text();
            widgets::esp_max_distance_slider->set_format_text(format_text.c_str());
        });

        widgets::only_visible_checkbox = main_child->insert_element<c_checkbox>("only show visible");

        widgets::visible_color_override_checkbox = main_child->insert_element<c_checkbox>("override visible color");

        widgets::box_combo = main_child->insert_element<c_combo>("bounding box", std::vector<std::string>{ "off", "standard", "corner", "3d" });

        widgets::name_checkbox = main_child->insert_element<c_checkbox>("name tag");

        widgets::weapon_checkbox = main_child->insert_element<c_checkbox>("current item");

        widgets::distance_checkbox = main_child->insert_element<c_checkbox>("current distance");

        widgets::chams_combo = main_child->insert_element<c_combo>("chams type", std::vector<std::string>{ "off", "flat" });

        widgets::chams_through_walls_checkbox = main_child->insert_element<c_checkbox>("show through walls");

        widgets::chams_through_walls_checkbox->set_visibility_callback([]() -> bool {
            return widgets::chams_combo->get_value() > 0;
        });

        widgets::trail_checkbox = main_child->insert_element<c_checkbox>("player trail");

        widgets::trail_esp_checkbox = main_child->insert_element<c_checkbox>("esp trail");
        widgets::trail_esp_checkbox->set_visibility_callback([]() -> bool { return widgets::trail_checkbox->get_value(); });

        widgets::trail_length_slider = main_child->insert_element<c_slider_int>("trail length", 3, 127, "%d units");

        widgets::trail_length_slider->set_visibility_callback([]() -> bool {
            return widgets::trail_checkbox->get_value();
            });

        //widgets::out_of_fov_arrows_checkbox = main_child->insert_element<c_checkbox>("out of fov arrows");

        //widgets::fov_arrow_colorpicker = main_child->insert_element<c_colorpicker>("fov arrow color");

        //widgets::fov_arrow_distance_slider = main_child->insert_element<c_slider_int>("arrow distance", 1, 30, "%dpx");

        //widgets::fov_arrow_distance_slider->set_value(15);

        //widgets::fov_arrow_size_slider = main_child->insert_element<c_slider_int>("arrow size", 1, 30, "%dpx");

        //widgets::fov_arrow_size_slider->set_value(20);

        //widgets::fov_arrow_colorpicker->set_visibility_callback([]() -> bool {
        //    return widgets::out_of_fov_arrows_checkbox->get_value();
        //});

        //widgets::fov_arrow_distance_slider->set_visibility_callback([]() -> bool {
        //    return widgets::out_of_fov_arrows_checkbox->get_value();
        //});

        //widgets::fov_arrow_size_slider->set_visibility_callback([]() -> bool {
        //    return widgets::out_of_fov_arrows_checkbox->get_value();
        //});

        widgets::snaplines_checkbox = main_child->insert_element<c_checkbox>("snaplines");

        widgets::snaplines_start_combo = main_child->insert_element<c_combo>("start position", std::vector<std::string>{ "top", "middle", "bottom" });

        widgets::snaplines_end_combo = main_child->insert_element<c_combo>("end position", std::vector<std::string>{ "top", "head", "bottom" });

        widgets::snaplines_start_combo->set_visibility_callback([]() -> bool { return widgets::snaplines_checkbox->get_value(); });
        widgets::snaplines_end_combo->set_visibility_callback([]() -> bool { return widgets::snaplines_checkbox->get_value(); });

        auto colors_group = main_child->insert_element<c_collapsable_group>("player colors");
        {

            widgets::visible_override_colorpicker = colors_group->insert_element<c_colorpicker>("visible override color");
            widgets::visible_override_colorpicker->set_visibility_callback([]() -> bool {
                return widgets::visible_color_override_checkbox->get_value();
                });

            widgets::box_color = colors_group->insert_element<c_colorpicker>("bounding box color");
            widgets::box_color->set_visibility_callback([]() -> bool { return widgets::box_combo->get_value() > 0; });

            widgets::name_color = colors_group->insert_element<c_colorpicker>("name tag color");
            widgets::name_color->set_visibility_callback([]() -> bool { return widgets::name_checkbox->get_value(); });

            widgets::weapon_color = colors_group->insert_element<c_colorpicker>("current item color");
            widgets::weapon_color->set_visibility_callback([]() -> bool { return widgets::weapon_checkbox->get_value(); });

            widgets::distance_colorpicker = colors_group->insert_element<c_colorpicker>("distance color");
            widgets::distance_colorpicker->set_visibility_callback([]() -> bool {
                return widgets::distance_checkbox->get_value();
                });

            widgets::chams_colorpicker = colors_group->insert_element<c_colorpicker>("chams color");
            widgets::chams_colorpicker->set_visibility_callback([]() -> bool {
                return widgets::chams_combo->get_value() > 0;
                });

            widgets::trail_colorpicker = colors_group->insert_element<c_colorpicker>("trail color");
            widgets::trail_colorpicker->set_visibility_callback([]() -> bool { return widgets::trail_checkbox->get_value(); });

            widgets::snaplines_color = colors_group->insert_element<c_colorpicker>("snaplines color");
            widgets::snaplines_color->set_visibility_callback([]() -> bool { return widgets::snaplines_checkbox->get_value(); });

        }

    } main_child->set_visibility_callback(show_if_tab);

    auto other_child = cheat_window.insert_child("visuals", { 249.f + 41.f, top_bar_sz + 22.f }, { 249.f, 348.f });
    {

        auto world_overrides_group = other_child->insert_element<c_collapsable_group>("world overrides");
        {

            widgets::time_changer_combo = world_overrides_group->insert_element<c_combo>("time changer", std::vector<std::string>{ "off", "day", "sunset", "night" });

            widgets::sky_color_checkbox = world_overrides_group->insert_element<c_checkbox>("change sky color");

            widgets::sky_color_picker = world_overrides_group->insert_element<c_colorpicker>("sky color");

            widgets::sky_color_picker->set_visibility_callback([]() -> bool {
                return widgets::sky_color_checkbox->get_value();
            });

        }

        auto local_chams_group = other_child->insert_element<c_collapsable_group>("local chams");
        {

            widgets::local_player_chams_checkbox = local_chams_group->insert_element<c_checkbox>("local chams");

            widgets::local_player_chams_colorpicker = local_chams_group->insert_element<c_colorpicker>("local chams color");

            widgets::local_player_chams_colorpicker->set_visibility_callback([]() -> bool {
                return widgets::local_player_chams_checkbox->get_value();
            });

            widgets::hand_chams_checkbox = local_chams_group->insert_element<c_checkbox>("hand chams");

            widgets::hand_chams_colorpicker = local_chams_group->insert_element<c_colorpicker>("hand chams color");

            widgets::hand_chams_colorpicker->set_visibility_callback([]() -> bool {
                return widgets::hand_chams_checkbox->get_value();
            });

        }

        auto fov_changer_group = other_child->insert_element<c_collapsable_group>("fov changer");
        {

            widgets::fov_changer_checkbox = fov_changer_group->insert_element<c_checkbox>("fov changer");

            widgets::fov_factor_slider = fov_changer_group->insert_element<c_slider_float>("fov factor", 0.6f, 3.f, "%.1fx");

            widgets::fov_factor_slider->set_value(1.f);

            widgets::fov_factor_bind = fov_changer_group->insert_element<c_keybind>("fov factor", &widgets::fov_factor_key);

            widgets::fov_factor_key_slider = fov_changer_group->insert_element<c_slider_float>("fov factor on key", 0.6f, 12.f, "%.1fx");

            widgets::fov_factor_key_slider->set_value(2.f);

            widgets::fov_factor_slider->set_visibility_callback([]() -> bool { return widgets::fov_changer_checkbox->get_value(); });
            widgets::fov_factor_key_slider->set_visibility_callback([]() -> bool { return widgets::fov_changer_checkbox->get_value(); });
            widgets::fov_factor_bind->set_visibility_callback([]() -> bool { return widgets::fov_changer_checkbox->get_value(); });

        }

        auto bullet_tracer_group = other_child->insert_element<c_collapsable_group>("bullet tracers");
        {

            widgets::bullet_tracer_checkbox = bullet_tracer_group->insert_element<c_checkbox>("bullet tracers");

            widgets::bullet_tracer_colorpicker = bullet_tracer_group->insert_element<c_colorpicker>("bullet tracer color");
            widgets::bullet_tracer_colorpicker->set_visibility_callback([]() -> bool { return widgets::bullet_tracer_checkbox->get_value(); });

            widgets::tracer_lifetime_slider = bullet_tracer_group->insert_element<c_slider_float>("tracer lifetime", 1.f, 5.f, "%.1fs"); widgets::tracer_lifetime_slider->set_value(3.5f);
            widgets::tracer_lifetime_slider->set_visibility_callback([]() -> bool { return widgets::bullet_tracer_checkbox->get_value(); });
            widgets::tracer_lifetime_slider->set_value(4.5f);

            widgets::tracer_line_style_combo = bullet_tracer_group->insert_element<c_combo>("tracer style", std::vector<std::string>{ "line", "beam", "dots" });
            widgets::tracer_line_style_combo->set_visibility_callback([]() -> bool { return widgets::bullet_tracer_checkbox->get_value(); });

            widgets::tracer_line_thickness = bullet_tracer_group->insert_element<c_slider_float>("tracer thickness", 1.f, 6.f, "%1.fpx");
            widgets::tracer_line_thickness->set_value(3.f);
            widgets::tracer_line_thickness->set_visibility_callback([]() -> bool { return widgets::bullet_tracer_checkbox->get_value(); });

        }

        auto item_esp_group = other_child->insert_element<c_collapsable_group>("item esp");
        {

            widgets::item_esp_checkbox = item_esp_group->insert_element<c_checkbox>("item esp");

            widgets::item_esp_color = item_esp_group->insert_element<c_colorpicker>("item esp color");

            widgets::item_distance_checkbox = item_esp_group->insert_element<c_checkbox>("item distance");

            widgets::item_max_distance_slider = item_esp_group->insert_element<c_slider_float>("max item distance", 1.f, 1200.f, "%1.fft");

            widgets::item_max_distance_slider->set_render_callback([]() -> void {
                auto format_text = std::string("%1.f");
                format_text += get_unit_text();
                widgets::esp_max_distance_slider->set_format_text(format_text.c_str());
            });

            widgets::item_max_distance_slider->set_value(30.f);

            widgets::item_esp_color->set_visibility_callback([]() -> bool { return widgets::item_esp_checkbox->get_value(); });
            widgets::item_distance_checkbox->set_visibility_callback([]() -> bool { return widgets::item_esp_checkbox->get_value(); });
            widgets::item_max_distance_slider->set_visibility_callback([]() -> bool { return widgets::item_esp_checkbox->get_value(); });

        }

        widgets::weapon_info_panel_checkbox = other_child->insert_element<c_checkbox>("weapon info panel");

    } other_child->set_visibility_callback(show_if_tab);

}

__forceinline void setup_misc_tab()
{

    static auto show_if_tab = []() -> bool {
        return current_tab == 2 && child_alpha_animator.get_starting_animation();
        };

    auto main_child = cheat_window.insert_child("main", { 21.f, top_bar_sz + 22.f }, { 249.f, 348.f });
    {

        widgets::dpi_combo = main_child->insert_element<c_combo>("menu scaling", std::vector<std::string>{ "1.0x", "1.2x", "1.6x", "1.8x", "2.0x" });

        widgets::dpi_combo->set_interaction_callback([]() -> void {

            framework::current_scaling = widgets::dpi_combo->get_value();

            float dpi_scale = framework::get_dpi_scale();

            if (dpi_scale == cheat_window.get_dpi_scale())
                return;

            cheat_window.set_dpi_scale(dpi_scale);

            top_bar_sz = 28.f * dpi_scale;

            auto& io = ImGui::GetIO();

            ImGui::GetStyle().ItemSpacing.y = 10 * dpi_scale;
            ImGui::GetStyle().ItemSpacing.x = 10 * dpi_scale;

            framework::needs_to_build_fonts = true;

        });

        widgets::menu_bind = main_child->insert_element<c_keybind>("menu hotkey", &widgets::menu_key);

        widgets::fps_counter_checkbox = main_child->insert_element<c_checkbox>("watermark");

        widgets::distance_units_combo = main_child->insert_element<c_combo>("distance units", std::vector<std::string>{ "feet", "meters", "yards" });

        widgets::fps_counter_checkbox->set_value(true);

        widgets::force_3d_hitmarkers_checkbox = main_child->insert_element<c_checkbox>("force 3d hitmarkers");

        widgets::force_3d_hitmarkers_checkbox->set_has_tooltip(true);

        widgets::force_3d_hitmarkers_checkbox->set_tooltip("forces game hitmarkers to be 3d");

        main_child->insert_element<c_button>("rage quit")->set_interaction_callback([]() -> void {
            exit(0);
        });

    } main_child->set_visibility_callback(show_if_tab);

    auto other_child = cheat_window.insert_child("other", { 249.f + 41.f, top_bar_sz + 22.f }, { 249.f, 348.f });
    {

        auto game_override_group = other_child->insert_element<c_collapsable_group>("game overrides");
        {

            widgets::no_exit_timer_checkbox = game_override_group->insert_element<c_checkbox>("disable exit timer");

            widgets::disable_stun_checkbox = game_override_group->insert_element<c_checkbox>("no flashbang effect");

            widgets::disable_pain_checkbox = game_override_group->insert_element<c_checkbox>("no pain effect");

            widgets::disable_hallucinations_checkbox = game_override_group->insert_element<c_checkbox>("no hallcinations");

            widgets::allow_compass_checkbox = game_override_group->insert_element<c_checkbox>("allow compass");

            widgets::allow_gps_checkbox = game_override_group->insert_element<c_checkbox>("allow gps");

            widgets::admin_flag_checkbox = game_override_group->insert_element<c_checkbox>("admin flag");

            widgets::admin_flag_checkbox->set_has_tooltip(true);

            widgets::admin_flag_checkbox->set_tooltip("shift + f1 to enable freecam, f2 to disable");

            widgets::allow_thirdperson_checkbox = game_override_group->insert_element<c_checkbox>("allow thirdperson");

        }

        auto freecam_group = other_child->insert_element<c_collapsable_group>("freecam settings");
        {

            widgets::freecam_checkbox = freecam_group->insert_element<c_checkbox>("freecam");

            widgets::freecam_bind = freecam_group->insert_element<c_keybind>("freecam key", &widgets::freecam_key);

            widgets::freecam_speed_slider = freecam_group->insert_element<c_slider_float>("freecam speed", 1.f, 16.f, "%1.f u/s");

            widgets::freecam_speed_slider->set_value(4.f);

            widgets::freecam_bind->set_visibility_callback([]() -> bool { return widgets::freecam_checkbox->get_value(); });
            widgets::freecam_speed_slider->set_visibility_callback([]() -> bool { return widgets::freecam_checkbox->get_value(); });

        }

        auto hitsounds_group = other_child->insert_element<c_collapsable_group>("hitsounds");
        {

            widgets::custom_hitsound_name_input = hitsounds_group->insert_element<c_input_text>("custom sound name", "example. hitsound.wav");

            widgets::custom_hitsound_name_input->set_has_tooltip(true);

            widgets::custom_hitsound_name_input->set_tooltip("path to sounds is in your documents folder");

            widgets::custom_hitsound_name_input->set_visibility_callback([]() -> bool {
                return widgets::hitsound_combo->get_value() == 4;
            });

            widgets::hitsound_combo = hitsounds_group->insert_element<c_combo>("hitsound", std::vector<std::string>{ "off", "bell", "cod", "stapler", "custom" });

            widgets::hitsound_volume_slider = hitsounds_group->insert_element<c_slider_int>("hitsounds volume", 1, 100, "%d%%");

            widgets::hitsound_volume_slider->set_value(50);

            widgets::hitsound_volume_slider->set_visibility_callback([]() -> bool {
                return widgets::hitsound_combo->get_value() > 0;
            });

        }

        auto friend_system_group = other_child->insert_element<c_collapsable_group>("friend system");
        {

            widgets::friend_override_checkbox = friend_system_group->insert_element<c_checkbox>("friend override");

            widgets::friend_override_add_keybind = friend_system_group->insert_element<c_keybind>("add friend", &widgets::friend_override_add_key);

            widgets::friend_override_remove_keybind = friend_system_group->insert_element<c_keybind>("remove friend", &widgets::friend_override_remove_key);

            widgets::friend_override_color = friend_system_group->insert_element<c_colorpicker>("friend color override");

            widgets::friend_override_add_keybind->set_visibility_callback([]() -> bool {
                return widgets::friend_override_checkbox->get_value();
                });

            widgets::friend_override_remove_keybind->set_visibility_callback([]() -> bool {
                return widgets::friend_override_checkbox->get_value();
                });

            widgets::friend_override_color->set_visibility_callback([]() -> bool {
                return widgets::friend_override_checkbox->get_value();
                });

        }

        auto viewmodel_group = other_child->insert_element<c_collapsable_group>("viewmodel overrides");
        {

            widgets::viewmodel_changer_checkbox = viewmodel_group->insert_element<c_checkbox>("viewmodel changer");

            widgets::viewmodel_aim_fov_slider = viewmodel_group->insert_element<c_slider_float>("aim fov", 1.f, 179.f, "%1.f degrees");

            widgets::viewmodel_hip_fov_slider = viewmodel_group->insert_element<c_slider_float>("hip fov", 1.f, 179.f, "%1.f degrees");

            widgets::viewmodel_x_pos_slider = viewmodel_group->insert_element<c_slider_float>("x offset", -1.f, 1.f, "%.1f units");

            widgets::viewmodel_y_pos_slider = viewmodel_group->insert_element<c_slider_float>("y offset", -1.f, 1.f, "%.1f units");

            widgets::viewmodel_z_pos_slider = viewmodel_group->insert_element<c_slider_float>("z offset", -0.5f, 0.5f, "%.1f units");

            widgets::viewmodel_aim_fov_slider->set_value(60.f);
            widgets::viewmodel_hip_fov_slider->set_value(60.f);

            widgets::viewmodel_x_pos_slider->set_value(0.f);
            widgets::viewmodel_y_pos_slider->set_value(0.f);
            widgets::viewmodel_z_pos_slider->set_value(0.f);

            widgets::viewmodel_aim_fov_slider->set_visibility_callback([]() -> bool {
                return widgets::viewmodel_changer_checkbox->get_value();
            });

            widgets::viewmodel_hip_fov_slider->set_visibility_callback([]() -> bool {
                return widgets::viewmodel_changer_checkbox->get_value();
            });

            widgets::viewmodel_x_pos_slider->set_visibility_callback([]() -> bool {
                return widgets::viewmodel_changer_checkbox->get_value();
            });

            widgets::viewmodel_y_pos_slider->set_visibility_callback([]() -> bool {
                return widgets::viewmodel_changer_checkbox->get_value();
            });

            widgets::viewmodel_z_pos_slider->set_visibility_callback([]() -> bool {
                return widgets::viewmodel_changer_checkbox->get_value();
            });

        }

    } other_child->set_visibility_callback(show_if_tab);

}

__forceinline void setup_config_tab()
{

    static auto show_if_tab = []() -> bool {
        return current_tab == 3 && child_alpha_animator.get_starting_animation();
        };

    preset_child = cheat_window.insert_child("presets", { 249.f + 41.f, top_bar_sz + 22.f }, { 249.f, 348.f });
    {

    } preset_child->set_visibility_callback(show_if_tab);

    auto main_child = cheat_window.insert_child("controls", { 21.f, top_bar_sz + 22.f }, { 249.f, 348.f });
    {

        widgets::config_name_input = main_child->insert_element<c_input_text>("preset name", "enter a present name");

        widgets::config_name_input->set_no_save();

        main_child->insert_element<c_button>("create preset")->set_interaction_callback([]() -> void {
            g_config_manager->create_config(preset_child, &cheat_window, &current_config, widgets::config_name_input->get_value().c_str());
            g_config_manager->refresh_container(preset_child, &current_config);
            });

        main_child->insert_element<c_button>("refresh presets")->set_interaction_callback([]() -> void {
            g_config_manager->refresh_configs();
            g_config_manager->refresh_container(preset_child, &current_config);
            });

        main_child->insert_element<c_button>("delete preset")->set_interaction_callback([]() -> void {
            g_config_manager->delete_config(preset_child, current_config, &current_config);
            g_config_manager->refresh_container(preset_child, &current_config);
            });

        main_child->insert_element<c_button>("save preset")->set_interaction_callback([]() -> void {
            g_config_manager->save_config(&cheat_window, current_config);
            });

        main_child->insert_element<c_button>("load preset")->set_interaction_callback([]() -> void {
            g_config_manager->load_config(&cheat_window, current_config);
            });

        main_child->insert_element<c_button>("open preset folder")->set_interaction_callback([]() -> void {
            ShellExecuteA(NULL, "open", std::string(g_config_manager->get_documents_path() + "\\ann_configs").c_str(), NULL, NULL, SW_SHOWNORMAL);
            });

    } main_child->set_visibility_callback(show_if_tab);

}

__forceinline void setup_scripts_tab()
{

    static auto show_if_tab = []() -> bool {
        return current_tab == 4 && child_alpha_animator.get_starting_animation();
        };

    script_interface::element_child = cheat_window.insert_child("elements", { 249.f + 41.f, top_bar_sz + 22.f }, { 249.f, 348.f });
    {

    } script_interface::element_child->set_visibility_callback(show_if_tab);

    script_interface::main_child = cheat_window.insert_child("available scripts", { 21.f, top_bar_sz + 22.f }, { 249.f, 348.f });
    {

        script_interface::main_child->insert_element<c_button>("refresh scripts")->set_interaction_callback([]() -> void {
            g_script_engine->reload_scripts();
            g_script_engine->refresh_container(script_interface::main_child, &script_interface::current_script);
        });

    } script_interface::main_child->set_visibility_callback(show_if_tab);

}

bool switched_tabs = true;
bool switched_to_visuals = false;

void setup_menu()
{

    setup_combat_tab();

    setup_visuals_tab();

    setup_misc_tab();

    setup_config_tab();

    setup_scripts_tab();

    for (auto& child : cheat_window.get_children())
    {

        child->set_start_render_callback([]() -> void {
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, child_alpha_animator.get_value() * alpha_animator.get_value());
        });

        child->set_end_render_callback([]() -> void {
            ImGui::PopStyleVar();
        });

    }

    cheat_window.insert_attachment([](const ImVec2& pos, const ImVec2& sz, ImDrawList* draw) -> void {

        auto foreground = ImGui::GetForegroundDrawList(); auto background = ImGui::GetBackgroundDrawList();
        auto adj_pos = pos - ImVec2(-1.f, -1.f);
        auto adj_sz = sz - ImVec2(2.f, 2.f);
        auto imgui_alpha = ImGui::GetStyle().Alpha;

        ImGui::RenderColorRectWithAlphaCheckerboard(ImGui::GetBackgroundDrawList(), adj_pos, adj_pos + adj_sz, ImColor(25, 25, 25, 200), 4.f * cheat_window.get_dpi_scale(), ImVec2(3, 3) * cheat_window.get_dpi_scale());

        ImGui::GetBackgroundDrawList()->AddRectFilledMultiColor(adj_pos + adj_sz, adj_pos, IM_COL32(27, 27, 27, 160 * imgui_alpha), IM_COL32(27, 27, 27, 160 * imgui_alpha), IM_COL32(17, 17, 17, 60 * imgui_alpha), IM_COL32(17, 17, 17, 60 * imgui_alpha));

        background->AddRectFilledMultiColor(adj_pos, adj_pos + ImVec2(adj_sz.x, top_bar_sz), IM_COL32(35, 31, 36, 255 * imgui_alpha), IM_COL32(35, 31, 36, 255 * imgui_alpha), IM_COL32(45, 44, 59, 255 * imgui_alpha), IM_COL32(45, 44, 59, 255 * imgui_alpha));
        background->AddLine(adj_pos + ImVec2(0.f, top_bar_sz), adj_pos + ImVec2(adj_sz.x, top_bar_sz), IM_COL32(130, 125, 150, 200 * imgui_alpha));

        ImGui::PushFont(framework::bold_font);

        auto text_size = ImGui::CalcTextSize("annihilate.ac");

        background->AddText(adj_pos + ImVec2(7.f, ((top_bar_sz - text_size.y) + 5.f) / 2.f), IM_COL32(0, 0, 0, 255 * imgui_alpha), "annihilate.ac");
        background->AddText(adj_pos + ImVec2(9.f, (top_bar_sz - text_size.y) / 2.f), IM_COL32(230, 230, 230, 225 * imgui_alpha), "annihilate.ac");

        ImGui::PopFont();

    });

    preview_window.insert_attachment([](const ImVec2& pos, const ImVec2& sz, ImDrawList* draw) -> void {

        auto foreground = ImGui::GetForegroundDrawList(); auto background = ImGui::GetBackgroundDrawList();
        auto adj_pos = pos - ImVec2(-1.f, -1.f);
        auto adj_sz = sz - ImVec2(2.f, 2.f);
        auto imgui_alpha = ImGui::GetStyle().Alpha;

        ImGui::RenderColorRectWithAlphaCheckerboard(ImGui::GetBackgroundDrawList(), adj_pos, adj_pos + adj_sz, ImColor(25, 25, 25, 200), 4.f * cheat_window.get_dpi_scale(), ImVec2(3, 3) * cheat_window.get_dpi_scale());

        ImGui::GetBackgroundDrawList()->AddRectFilledMultiColor(adj_pos + adj_sz, adj_pos, IM_COL32(27, 27, 27, 255 * imgui_alpha), IM_COL32(27, 27, 27, 255 * imgui_alpha), IM_COL32(17, 17, 17, 80 * imgui_alpha), IM_COL32(17, 17, 17, 10 * imgui_alpha));

        ImVec2 box_padding{ 50.f * preview_window.get_dpi_scale(), 70.f * preview_window.get_dpi_scale() };

        ImRect box_bounds{ adj_pos + box_padding, adj_sz - ImVec2{ box_padding.x * 2.f, box_padding.y * 2.f } };
        ImVec2 box_min = adj_pos + box_padding;
        ImVec2 box_max = (adj_pos + adj_sz) - ImVec2{ box_padding.x * 2.f, box_padding.y * 2.f };

        float box_width = adj_sz.x - (box_padding.x * 2.f);
        float box_height = adj_sz.y - (box_padding.y * 2.f);

        auto box_color = widgets::box_color->get_value(); box_color.a *= imgui_alpha;

        if (widgets::box_combo->get_value() == 1)
            render.outlined_rect(box_bounds.Min, box_bounds.Max, box_color.u32());
        else if (widgets::box_combo->get_value() == 2)
            render.cornered_rect(box_bounds.Min, box_bounds.Max, box_color.u32());

        auto tahoma_pixel = framework::g_scaled_fonts.at(2);
        auto tahoma_bold_pixel = framework::g_scaled_fonts.at(3);

        ImGui::PushFont(tahoma_bold_pixel.font);

        auto top_text = std::string("Peter");
        auto top_text_sz = ImGui::CalcTextSize(top_text.c_str());
        auto text_padding = 15.f * preview_window.get_dpi_scale();

        auto name_color = widgets::name_color->get_value(); name_color.a *= imgui_alpha;

        if (widgets::name_checkbox->get_value())
            render.text_shadowed({ box_min.x + (box_width - top_text_sz.x) / 2.f, box_min.y - text_padding }, top_text, name_color.u32(), &tahoma_bold_pixel);

        ImGui::PopFont();

        ImGui::PushFont(tahoma_pixel.font);

        auto bottom_text = std::string("Hands");
        auto distance_text = std::string("");

        auto distance = static_cast<int>(100.f * get_unit_multiplier());
        distance_text = std::to_string(distance);
        distance_text += get_unit_text();

        auto bottom_text_sz = ImGui::CalcTextSize(bottom_text.c_str());
        auto distance_text_sz = ImGui::CalcTextSize(distance_text.c_str());

        auto weapon_color = widgets::weapon_color->get_value(); weapon_color.a *= imgui_alpha;
        auto distance_color = widgets::distance_colorpicker->get_value(); distance_color.a *= imgui_alpha;

        if (widgets::weapon_checkbox->get_value())
            render.text_shadowed({ box_min.x + (box_width - bottom_text_sz.x) / 2.f, box_min.y + (box_height + 2.f) }, bottom_text, weapon_color.u32(), &tahoma_pixel);

        if (widgets::distance_checkbox->get_value())
            render.text_shadowed({ box_min.x + (box_width - distance_text_sz.x) / 2.f, box_min.y + (box_height + (widgets::weapon_checkbox->get_value() ? 14.f : 2.f)) }, distance_text, distance_color.u32(), &tahoma_pixel);
        
        if (widgets::chams_combo->get_value())
        {

            auto cham_color = widgets::chams_colorpicker->get_value(); cham_color.a *= imgui_alpha;

            ImGui::SetCursorPos({ box_padding.x - 15.f, box_padding.y - 15.f });
            ImGui::Image(g_peter_png, { box_width + 30.f, box_height + 30.f }, { 0.f, 0.f }, { 1.f, 1.f }, { cham_color.r, cham_color.g, cham_color.b, cham_color.a * 1.2f });

        }

        ImGui::PopFont();

    });

    preview_window.set_start_render_callback([]() -> void {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, preview_animator.get_value() * alpha_animator.get_value());
    });

    preview_window.set_end_render_callback([]() -> void {
        ImGui::PopStyleVar();
    });

    auto combat_tab = cheat_window.insert_element<c_click_label>("combat", 0, &current_tab); combat_tab->set_custom_position({ 164.f, (14.f / 2.f) });

    auto visuals_tab = cheat_window.insert_element<c_click_label>("visuals", 1, &current_tab); visuals_tab->set_same_line(true);

    auto misc_tab = cheat_window.insert_element<c_click_label>("misc", 2, &current_tab); misc_tab->set_same_line(true);

    auto config_tab = cheat_window.insert_element<c_click_label>("configs", 3, &current_tab); config_tab->set_same_line(true);

    auto scripts_tab = cheat_window.insert_element<c_click_label>("scripts", 4, &current_tab); scripts_tab->set_same_line(true);

    for (auto tab : cheat_window.find_elements_by_type<c_click_label>(element_type::CLICK_LABEL))
    {

        tab->set_interaction_callback([]() -> void {
            switched_tabs = true;
        });

    }

    cheat_window.set_visibility_callback([]() -> bool {
        return alpha_animator.get_value() > 0.f && finished_start;
    });

    preview_window.set_visibility_callback([]() -> bool {
        return preview_animator.get_value() > 0.f;
    });

    alpha_animator.set_starting_animation(menu_open);

    loading_alpha_animator.set_starting_animation(true);

    g_config_manager->refresh_configs();

    g_config_manager->refresh_container(preset_child, &current_config);

    g_script_engine->reload_scripts();

    g_script_engine->refresh_container(script_interface::main_child, &script_interface::current_script);

}

c_label* weapon_name_label = nullptr;
c_label* weapon_range_label = nullptr;
c_progress_bar* weapon_ammo_progress_bar = nullptr;

#include "Peter_NPC.h"

void InitImGui()
{
#ifndef NO_SECURITY
    VIRTUALIZER_START;
#endif

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

    ImGui::CreateShadowTexture(pDevice);

    framework::g_scaled_fonts.emplace_back<scaled_font_t>({ "C:\\Windows\\Fonts\\Verdanab.ttf", 14.f, ImGuiFreeTypeBuilderFlags_ForceAutoHint });
    framework::g_scaled_fonts.emplace_back<scaled_font_t>({ "C:\\Windows\\Fonts\\Tahoma.ttf", 13.f, ImGuiFreeTypeBuilderFlags_ForceAutoHint });

    framework::g_scaled_fonts.emplace_back<scaled_font_t>({ "C:\\Windows\\Fonts\\Tahoma.ttf", 12.f, (ImGuiFreeTypeBuilderFlags)(ImGuiFreeTypeBuilderFlags_Monochrome | ImGuiFreeTypeBuilderFlags_MonoHinting) });
    framework::g_scaled_fonts.emplace_back<scaled_font_t>({ "C:\\Windows\\Fonts\\Tahomabd.ttf", 12.f, (ImGuiFreeTypeBuilderFlags)(ImGuiFreeTypeBuilderFlags_Monochrome | ImGuiFreeTypeBuilderFlags_MonoHinting) });

    framework::g_scaled_fonts.emplace_back<scaled_font_t>({ "C:\\Windows\\Fonts\\verdanaz.ttf", 48.f, ImGuiFreeTypeBuilderFlags_ForceAutoHint });
    framework::g_scaled_fonts.emplace_back<scaled_font_t>({ "C:\\Windows\\Fonts\\Verdana.ttf", 12.f, (ImGuiFreeTypeBuilderFlags)(ImGuiFreeTypeBuilderFlags_Monochrome | ImGuiFreeTypeBuilderFlags_MonoHinting) });

    srand((unsigned)time(NULL));

	//io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(pDevice, pContext);

    ImGui::StyleColorsDark();

    setup_menu();

    weapon_name_label = weapon_panel_window.insert_element<c_label>("weapon name");
    weapon_name_label->set_center_x(true);

    weapon_range_label = weapon_panel_window.insert_element<c_label>("weapon range");
    weapon_range_label->set_center_x(true);

    weapon_ammo_progress_bar = weapon_panel_window.insert_element<c_progress_bar>("weapon ammo", ImVec2(200.f, 22.f), 100.f, 0.f);

    weapon_panel_window.insert_attachment([](const ImVec2& pos, const ImVec2& sz, ImDrawList* draw) -> void
    {

        auto adj_pos = pos - ImVec2(-1.f, -1.f);
        auto adj_sz = sz - ImVec2(1.f, 1.f);
        auto add_sz = pos + adj_sz;
        auto imgui_alpha = ImGui::GetStyle().Alpha;
        auto draw_list = ImGui::GetBackgroundDrawList();

        ImGui::RenderColorRectWithAlphaCheckerboard(ImGui::GetBackgroundDrawList(), adj_pos, adj_pos + adj_sz, ImColor(25, 25, 25, 200), 4.f * cheat_window.get_dpi_scale(), ImVec2(3, 3) * cheat_window.get_dpi_scale());

        ImGui::GetBackgroundDrawList()->AddRectFilledMultiColor(adj_pos + adj_sz, adj_pos, IM_COL32(27, 27, 27, 160 * imgui_alpha), IM_COL32(27, 27, 27, 160 * imgui_alpha), IM_COL32(17, 17, 17, 60 * imgui_alpha), IM_COL32(17, 17, 17, 60 * imgui_alpha));

        draw_list->AddRectFilledMultiColor(adj_pos, { add_sz.x, adj_pos.y + 6.f }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0));
        draw_list->AddRectFilledMultiColor(add_sz, { adj_pos.x, add_sz.y - 6.f }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0));

        draw_list->AddRectFilledMultiColor(adj_pos, { adj_pos.x + 6.f, add_sz.y }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0), IM_COL32(95, 92, 103, 200 * imgui_alpha));
        draw_list->AddRectFilledMultiColor(add_sz, { add_sz.x - 6.f, adj_pos.y }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(27, 22, 27, 0), IM_COL32(27, 22, 27, 0), IM_COL32(95, 92, 103, 200 * imgui_alpha));

    });

    start_time = GetTickCount64();

#ifndef NO_SECURITY
    VIRTUALIZER_END;
#endif

    return;
}

bool is_inactive = false;

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{

    if (menu_open)
        ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

    if (menu_open && !is_inactive)
    {
        CallWindowProc(reinterpret_cast<WNDPROC>(oWndProc), hWnd, WM_ACTIVATE, WA_INACTIVE, 0);
        is_inactive = true;
    }
    else if (!menu_open && is_inactive)
    {
        CallWindowProc(reinterpret_cast<WNDPROC>(oWndProc), hWnd, WM_ACTIVATE, WA_ACTIVE, 0);
        is_inactive = false;
    }

    return menu_open ? menu_open : CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);

}

// goto statements are autism
__forceinline void clear_cache()
{

    if (cheat::players.size() > 0 || cheat::items.size() > 0 || cheat::local_gun || cheat::local_player.player || g_snapshots.size() > 0 || g_vis_data.size() > 0)
    {

        //cheat::sync.lock();

        cheat::players.clear();
        cheat::items.clear();

        cheat::local_player = cached_player_t{};
        cheat::local_gun = nullptr;

        g_snapshots.clear();

        g_vis_mtx.lock();

        g_vis_data.clear();

        g_vis_mtx.unlock();

        //cheat::sync.unlock();

    }

}

bool init = false;

#include <numbers>

__forceinline void do_rainbow(c_color* col)
{

    if (!col->rainbow)
        return;

    constexpr float pi = std::numbers::pi_v<float>;

    auto r_rain = std::sin(3.f * ImGui::GetTime()) * 0.5f + 0.5f;
    auto g_rain = std::sin(3.f * ImGui::GetTime() + 2 * pi / 3) * 0.5f + 0.5f;
    auto b_rain = std::sin(3.f * ImGui::GetTime() + 4 * pi / 3) * 0.5f + 0.5f;

    col->r = r_rain;
    col->g = g_rain;
    col->b = b_rain;

}

ULONGLONG last_update_time = NULL;

HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{

	if (!init)
	{

		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)& pDevice)))
		{

            pDevice->GetImmediateContext(&pContext);
            DXGI_SWAP_CHAIN_DESC sd;
            pSwapChain->GetDesc(&sd);
            window = sd.OutputWindow;
            ID3D11Texture2D* pBackBuffer;
            pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
            pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
            pBackBuffer->Release();
            oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
            InitImGui();
            D3DX11_IMAGE_LOAD_INFO info2;
            ID3DX11ThreadPump* pump2{ nullptr };
            D3DX11CreateShaderResourceViewFromMemory(pDevice, peter_png, sizeof(peter_png), &info2, pump2, &g_peter_png, 0);
            init = true;

		}
		else
			return oPresent(pSwapChain, SyncInterval, Flags);

	}

    framework::build_fonts();

    for (auto color : g_colors)
        do_rainbow(color);

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

    cheat_window.update_binds();

    ImVec2 counter_text_sz = { 0.f, 12.f };

    if (widgets::fps_counter_checkbox->get_value())
        counter_text_sz.x += 1.f;

    g_logs->render_logs(counter_text_sz);

    if (widgets::fps_counter_checkbox->get_value())
    {

#ifdef _DEBUG

        auto counter_text = std::string("debug");

        counter_text += " | annihilate @ " + std::to_string(static_cast<int>(floor(ImGui::GetIO().Framerate))) + " fps";

        ImGui::PushFont(framework::g_scaled_fonts.at(5).font);
        counter_text_sz = ImGui::CalcTextSize(counter_text.c_str());
        ImGui::PopFont();

        auto counter_bb = ImRect{ 10.f, 10.f, counter_text_sz.x + 30.f, (counter_text_sz.y + 10.f) + 18.f };
        auto draw_list = ImGui::GetBackgroundDrawList();
        auto imgui_alpha = 1.f;

        draw_list->AddRectFilled(counter_bb.Min, counter_bb.Max, IM_COL32(25, 20, 25, 255 * imgui_alpha));

        draw_list->AddRectFilledMultiColor(counter_bb.Min, { counter_bb.Max.x, counter_bb.Min.y + 6.f }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(0, 0, 0, 0 * imgui_alpha), IM_COL32(0, 0, 0, 0 * imgui_alpha));
        draw_list->AddRectFilledMultiColor(counter_bb.Max, { counter_bb.Min.x, counter_bb.Max.y - 6.f }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(0, 0, 0, 0 * imgui_alpha), IM_COL32(0, 0, 0, 0 * imgui_alpha));

        draw_list->AddRectFilledMultiColor(counter_bb.Min, { counter_bb.Min.x + 6.f, counter_bb.Max.y }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(0, 0, 0, 0 * imgui_alpha), IM_COL32(0, 0, 0, 0 * imgui_alpha), IM_COL32(95, 92, 103, 200 * imgui_alpha));
        draw_list->AddRectFilledMultiColor(counter_bb.Max, { counter_bb.Max.x - 6.f, counter_bb.Min.y }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(0, 0, 0, 0 * imgui_alpha), IM_COL32(0, 0, 0, 0 * imgui_alpha), IM_COL32(95, 92, 103, 200 * imgui_alpha));

        draw_list->AddRectFilledMultiColor({ counter_bb.Min.x, counter_bb.Max.y }, { counter_bb.Max.x, counter_bb.Max.y + 15.f }, IM_COL32(130, 125, 150, 200 * imgui_alpha), IM_COL32(130, 125, 150, 200 * imgui_alpha), IM_COL32(27, 27, 27, 0 * imgui_alpha), IM_COL32(27, 27, 27, 0 * imgui_alpha));

        auto bdr_clr = ImGui::GetStyleColorVec4(ImGuiCol_AnimatedBorder);
        bdr_clr.w = imgui_alpha;

        draw_list->AddRect(counter_bb.Min, counter_bb.Max, IM_COL32(bdr_clr.x * 255.f, bdr_clr.y * 255.f, bdr_clr.z * 255.f, 255.f));

        render.text_shadowed(counter_bb.Min + ImVec2{ 10.f, 9.f }, counter_text, IM_COL32(255, 255, 255, 255 * imgui_alpha), &framework::g_scaled_fonts.at(5));

#elif !_DEBUG

        auto counter_text = user_name;

        counter_text += " | annihilate @ " + std::to_string(static_cast<int>(floor(ImGui::GetIO().Framerate))) + " fps";

        ImGui::PushFont(framework::g_scaled_fonts.at(5).font);
        auto counter_text_sz = ImGui::CalcTextSize(counter_text.c_str());
        ImGui::PopFont();

        auto counter_bb = ImRect{ 10.f, 10.f, counter_text_sz.x + 30.f, (counter_text_sz.y + 10.f) + 18.f };
        auto draw_list = ImGui::GetBackgroundDrawList();
        auto imgui_alpha = 1.f;

        draw_list->AddRectFilled(counter_bb.Min, counter_bb.Max, IM_COL32(25, 20, 25, 255 * imgui_alpha));

        draw_list->AddRectFilledMultiColor(counter_bb.Min, { counter_bb.Max.x, counter_bb.Min.y + 6.f }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(0, 0, 0, 0 * imgui_alpha), IM_COL32(0, 0, 0, 0 * imgui_alpha));
        draw_list->AddRectFilledMultiColor(counter_bb.Max, { counter_bb.Min.x, counter_bb.Max.y - 6.f }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(0, 0, 0, 0 * imgui_alpha), IM_COL32(0, 0, 0, 0 * imgui_alpha));

        draw_list->AddRectFilledMultiColor(counter_bb.Min, { counter_bb.Min.x + 6.f, counter_bb.Max.y }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(0, 0, 0, 0 * imgui_alpha), IM_COL32(0, 0, 0, 0 * imgui_alpha), IM_COL32(95, 92, 103, 200 * imgui_alpha));
        draw_list->AddRectFilledMultiColor(counter_bb.Max, { counter_bb.Max.x - 6.f, counter_bb.Min.y }, IM_COL32(95, 92, 103, 200 * imgui_alpha), IM_COL32(0, 0, 0, 0 * imgui_alpha), IM_COL32(0, 0, 0, 0 * imgui_alpha), IM_COL32(95, 92, 103, 200 * imgui_alpha));

        draw_list->AddRectFilledMultiColor({ counter_bb.Min.x, counter_bb.Max.y }, { counter_bb.Max.x, counter_bb.Max.y + 15.f }, IM_COL32(130, 125, 150, 200 * imgui_alpha), IM_COL32(130, 125, 150, 200 * imgui_alpha), IM_COL32(27, 27, 27, 0 * imgui_alpha), IM_COL32(27, 27, 27, 0 * imgui_alpha));

        auto bdr_clr = ImGui::GetStyleColorVec4(ImGuiCol_AnimatedBorder);
        bdr_clr.w = imgui_alpha;

        draw_list->AddRect(counter_bb.Min, counter_bb.Max, IM_COL32(bdr_clr.x * 255.f, bdr_clr.y * 255.f, bdr_clr.z * 255.f, 255.f));

        render.text_shadowed(counter_bb.Min + ImVec2{ 10.f, 9.f }, counter_text, IM_COL32(255, 255, 255, 255 * imgui_alpha), &framework::g_scaled_fonts.at(5));

#endif

    }

    if (init_hook)
    {

        auto provider = provider_t::get_instance();
        auto camera_instance = main_camera_t::get_instance();
        
        if (camera_instance && provider)
        {

            auto camera = camera_instance->instance();

            if (camera)
            {

                auto camera_obj = driver.read<uintptr_t>(camera + 0x10);

                if (camera_obj && provider->is_connected())
                {

                    game::camera.object = camera_obj;

                    if (game::camera.object && (last_update_time + 750) < GetTickCount64())
                    {

                        auto local_player = player_t::get_local();

                        if (local_player)
                        {

                            auto clients = provider->clients();

                            if (clients)
                            {

                                cached_player_t cached_local{};

                                cached_local.player = local_player;

                                cached_local.playerlook = local_player->look();
                                cached_local.equipment = local_player->equipment();
                                cached_local.animator = local_player->animator();
                                cached_local.movement = local_player->movement();

                                auto local_game_object = local_player->game_object();

                                if (local_game_object)
                                    cached_local.transform = local_game_object->transform();

                                if (cached_local.playerlook && cached_local.equipment && cached_local.animator && local_game_object && cached_local.transform)
                                {

                                    auto client_list_ptr = clients->list();
                                    auto client_list_sz = clients->size();

                                    if (client_list_sz && client_list_ptr)
                                    {

                                        auto client_list = driver.read_vector<steam_player_t*>(client_list_ptr, client_list_sz);

                                        if (client_list.size() > 1)
                                        {

                                            std::vector<cached_player_t> tmp_cache{};

                                            for (auto client : client_list)
                                            {

                                                if (!client)
                                                    continue;

                                                auto player = client->player();

                                                if (!player)
                                                    continue;

                                                auto player_life = player->life();

                                                if (!player_life || player_life->dead())
                                                    continue;

                                                if (player == local_player)
                                                {

                                                    cached_local.steamplayer = client;
                                                    continue;

                                                }

                                                auto plr_game_object = player->game_object();

                                                if (!plr_game_object)
                                                    continue;

                                                auto plr_id = client->player_id();

                                                if (!plr_id)
                                                    continue;

                                                auto plr_equipment = player->equipment();

                                                if (!plr_equipment)
                                                    continue;

                                                std::string item_name = "Hands";

                                                auto current_asset = plr_equipment->asset();

                                                if (current_asset)
                                                    item_name = current_asset->item_name();

                                                tmp_cache.emplace_back<cached_player_t>({ client, player, player_life, player->look(), nullptr, player->movement(), plr_game_object->transform(), nullptr, plr_id->character_name(), current_asset->item_name(), bounding_box_t{}, nullptr, nullptr});

                                            }

                                            cheat::players = tmp_cache;
                                            cheat::local_player = cached_local;
                                            tmp_cache.clear();

                                        }

                                        std::vector<cached_item_t> tmp_item_cache{};

                                        static auto item_manager = item_manager_t::get_instance();

                                        auto item_regions_ptr = item_manager->item_regions();

                                        if (item_regions_ptr)
                                        {
                                            auto item_regions = item_regions_ptr->data();

                                            if (item_regions.size() > 0)
                                            {
                                                for (auto region : item_regions)
                                                {
                                                    if (!region)
                                                        continue;
                                                    auto region_drops = region->drops();

                                                    if (!region_drops)
                                                        continue;

                                                    auto drop_list = region_drops->data();
                                                    for (auto item_drop : drop_list)
                                                    {
                                                        if (!item_drop)
                                                            continue;

                                                        auto interactable = item_drop->interactable();
                                                        if (!interactable)
                                                            continue;

                                                        auto asset = interactable->asset();
                                                        auto item = interactable->item();

                                                        if (!asset || !item)
                                                            continue;

                                                        auto model = item_drop->model();
                                                        if (!model)
                                                            continue;

                                                        auto model_ptr = reinterpret_cast<uintptr_t>(
                                                            model->m_CachedPtr() 
                                                        );
                                                        if (!model_ptr)
                                                            continue;

                                                        TransformInternal model_transform(model_ptr);

                                                        cached_item_t cached_item = cached_item_t();
                                                        cached_item.position = model_transform.position();
                                                        cached_item.name = asset->item_name();
                                                        cached_item.count = asset->amount();
                                                        cached_item.rarity = asset->rarity();
                                                        cached_item.type = asset->type();

                                                        tmp_item_cache.push_back(cached_item);
                                                    }
                                                }
                                            }

                                            cheat::items = tmp_item_cache;

                                            tmp_item_cache.clear();
                                        }
                                    }
                                }
                                else 
                                    clear_cache();

                            }

                        }
                        else
                            clear_cache();

                        last_update_time = GetTickCount64();

                    }

                    if (cheat::players.size() > 0 && cheat::local_player.equipment)
                    {

                        game::camera.position = driver.read<vec3_t>(camera_obj + 0x42C);
                        game::camera.matrix = driver.read<game::vmatrix_t>(camera_obj + 0x2E4);

                        cheat::local_gun = (item_gun_asset_t*)cheat::local_player.equipment->asset();

                    }

                    if (cheat::players.size() > 0 && cheat::local_player.player && cheat::local_player.animator && cheat::local_player.playerlook && cheat::local_player.equipment)
                    {
#ifndef NO_SECURITY
                        VIRTUALIZER_MUTATE_ONLY_START;
#endif

                        auto fov_factor = widgets::fov_factor_slider->get_value();

                        if (widgets::fov_factor_key.get_enabled())
                            fov_factor = widgets::fov_factor_key_slider->get_value();

                        if (widgets::fov_changer_checkbox->get_value())
                        {

                            cheat::local_player.playerlook->main_camera_zoom_factor(fov_factor);
                            cheat::local_player.playerlook->scope_camera_zoom_factor(fov_factor);

                        }

                        if (widgets::freecam_checkbox->get_value() && widgets::freecam_key.get_enabled())
                        {

                            auto look = cheat::local_player.playerlook;

                            auto is_orbiting = look->is_orbiting();
                            auto is_tracking = look->is_tracking();
                            auto is_locking = look->is_locking();
                            auto is_focusing = look->is_focusing();

                            look->is_orbiting(!is_orbiting);

                            if (!is_tracking)
                            {

                                is_orbiting = look->is_orbiting();

                                if (is_orbiting && !is_locking && !is_focusing)
                                {

                                    look->is_tracking(true);
                                    look->orbit_speed(widgets::freecam_speed_slider->get_value());

                                    look->orbit_position({ 0.f, 2.f, 0.f });

                                }

                            }
                            else
                                look->is_tracking(false);

                        }

                        auto channel = cheat::local_player.player->channel();

                        if (channel)
                        {

                            auto owner = channel->owner();

                            if (owner)
                            {

                                auto is_admin = owner->is_admin();

                                if (!is_admin && widgets::admin_flag_checkbox->get_value())
                                {

                                    owner->is_admin(true);

                                    if (owner->is_admin())
                                        g_logs->add_log("successfully set admin flag");

                                }
                                else if (is_admin && !widgets::admin_flag_checkbox->get_value())
                                {

                                    owner->is_admin(false);
                                    g_logs->add_log("successfully unset admin flag");

                                }

                            }

                        }

                        auto data = provider->config_data();

                        if (data)
                        {

                            auto config_data = data->config_data();

                            if (config_data)
                            {

                                if (widgets::no_exit_timer_checkbox->get_value())
                                    config_data->exit_timer(0u);

                                if (widgets::allow_compass_checkbox->get_value())
                                    config_data->compass(true);

                                if (widgets::allow_gps_checkbox->get_value())
                                {

                                    config_data->satellite(true);
                                    config_data->chart(true);

                                }

                                if (widgets::allow_thirdperson_checkbox->get_value())
                                {

                                    provider->camera_mode(2);
                                    config_data->allow_shoulder_camera(true);

                                }

                            }

                        }

                        if (widgets::time_changer_combo->get_value() == 1)
                            provider->time(2400u);
                        else if (widgets::time_changer_combo->get_value() == 2)
                            provider->time(1700u);
                        else if (widgets::time_changer_combo->get_value() == 3)
                            provider->time(0700u);

                        auto preference_data = provider->preference_data();

                        if (preference_data)
                        {

                            auto viewmodel_data = preference_data->viewmodel_data();

                            if (viewmodel_data)
                            {

                                if (widgets::viewmodel_changer_checkbox->get_value())
                                {

                                    viewmodel_data->aim_fov(widgets::viewmodel_aim_fov_slider->get_value());

                                    viewmodel_data->hip_fov(widgets::viewmodel_hip_fov_slider->get_value());

                                    viewmodel_data->x_offset(widgets::viewmodel_x_pos_slider->get_value());

                                    viewmodel_data->y_offset(widgets::viewmodel_y_pos_slider->get_value());

                                    viewmodel_data->z_offset(widgets::viewmodel_z_pos_slider->get_value());

                                }

                            }

                        }

                        g_item_esp->run();

                        g_player_esp->run(g_snapshots);

                        g_aimbot->collect_targets();

                        g_aimbot->run(alpha_animator.get_value() > 0.f);

#ifndef NO_SECURITY
                        VIRTUALIZER_MUTATE_ONLY_END;
#endif

                        if (cheat::local_gun && cheat::local_gun->type() == EItemType::GUN)
                        {
#ifndef NO_SECURITY
                            VIRTUALIZER_MUTATE_ONLY_START;
#endif

                            if (cheat::local_player.equipment && widgets::instant_equip_checkbox->get_value())
                            {

                                cheat::local_player.equipment->equip_anim_length_frames(0u);
                                cheat::local_player.equipment->last_equip(0.09f);

                            }

                            g_no_sway->run();

                            g_no_weapon_animations->run();

                            g_no_spread->run();

                            g_no_recoil->run();

                            g_no_shake->run();

                            g_no_drop->run();

                            g_fast_bullet->run();

                            g_instant_aim->run();

                            if (g_aimbot->m_fire_full_auto && !g_aimbot->m_has_target)
                            {

                                g_aimbot->set_mouse(false);
                                g_aimbot->m_fire_full_auto = false;

                            }

#ifndef NO_SECURITY
                            VIRTUALIZER_MUTATE_ONLY_END;
#endif
                        }

#ifndef NO_SECURITY
                        VIRTUALIZER_MUTATE_ONLY_START;
#endif

                        g_bullet_tracers->run();

#ifndef NO_SECURITY
                        VIRTUALIZER_MUTATE_ONLY_END;
#endif

                        static auto center = ImVec2{ ImGui::GetIO().DisplaySize.x / 2.f, ImGui::GetIO().DisplaySize.y / 2.f };

                        float zoom_multiplier = 1.f;

                        if (widgets::fov_changer_checkbox->get_value() && widgets::fov_factor_bind->get_bind()->get_enabled())
                            zoom_multiplier = widgets::fov_factor_key_slider->get_value();
                        else if (widgets::fov_changer_checkbox->get_value())
                            zoom_multiplier = widgets::fov_factor_slider->get_value();

                        if (zoom_multiplier > 1.f)
                            zoom_multiplier *= 0.35f;

                        auto final_radius = widgets::aimbot_fov_slider->get_value() * zoom_multiplier;

                        if (GetAsyncKeyState(VK_RBUTTON))
                            final_radius *= 1.05f;

                        if (widgets::aimbot_draw_fov_checkbox->get_value())
                            render.circle(center, widgets::aimbot_fov_color->get_value().u32(), final_radius);

                        if (widgets::weapon_info_panel_checkbox->get_value() && cheat::local_player.player && cheat::local_player.equipment && cheat::local_gun)
                        {

                            auto usable = cheat::local_player.equipment->usable();

                            if (usable)
                            {

                                auto usable_gun = (usable_gun_t*)usable;
                                auto is_gun = cheat::local_gun->type() == EItemType::GUN;

                                static item_gun_asset_t* last_gun = nullptr;
                                static BYTE last_ammo = NULL;

                                if (last_gun != cheat::local_gun)
                                {

                                    auto weapon_name = cheat::local_gun->item_name();

                                    std::transform(weapon_name.begin(), weapon_name.end(), weapon_name.begin(), [](unsigned char c) { return std::tolower(c); });

                                    weapon_name_label->set_text(weapon_name);

                                    auto weapon_range = static_cast<int>(cheat::local_gun->range() * 3.281f);

                                    weapon_range_label->set_text("range: " + std::to_string(weapon_range) + "ft");

                                    weapon_ammo_progress_bar->set_visible(is_gun);
                                    weapon_ammo_progress_bar->reset();

                                    last_gun = cheat::local_gun;
                                    last_ammo = NULL;

                                }

                                if (is_gun)
                                {

                                    auto third_attachments = usable_gun->third_attachments();

                                    if (third_attachments)
                                    {

                                        auto magazine_asset = third_attachments->magazine_asset();

                                        // extra read can be optimized
                                        if (magazine_asset)
                                            weapon_ammo_progress_bar->set_max_value(magazine_asset->amount());

                                    }

                                    auto ammo = usable_gun->ammo();

                                    if (ammo != last_ammo)
                                    {

                                        auto current_value = weapon_ammo_progress_bar->get_value();
                                        auto delta = ammo - current_value;

                                        weapon_ammo_progress_bar->increment_value(delta);

                                        weapon_ammo_progress_bar->set_label(std::to_string(ammo) + " / " + std::to_string(static_cast<int>(weapon_ammo_progress_bar->get_max_value())));

                                        last_ammo = ammo;

                                    }

                                }

                                ImGui::GetStyle().Alpha = 1.f;

                                weapon_panel_window.render();

                            }

                        }
                        else
                            weapon_ammo_progress_bar->reset();

                        for (auto& script : g_script_engine->get_scripts())
                            for (auto& callback : script.callbacks)
                                if (callback.callback_name == "update")
                                    g_script_engine->run_function(callback.function);

                    }

                }
                else
                    clear_cache();

            }
            else
                clear_cache();

        }
        else
            clear_cache();

    }

    if (finished_start)
        alpha_animator.update_animation();

    preview_animator.update_animation();
    loading_alpha_animator.update_animation();

    if (widgets::menu_key.get_enabled() && alpha_animator.start_animation(!menu_open))
        menu_open ^= 1;

    ImGui::GetIO().MouseDrawCursor = menu_open;

    child_alpha_animator.update_animation();

    if (switched_tabs && child_alpha_animator.start_animation(false))
        switched_tabs = false;
    else if (!switched_tabs && child_alpha_animator.get_value() == 0.f)
        child_alpha_animator.start_animation(true);

    if (alpha_animator.get_value() > 0.f)
        ImGui::GetStyle().Alpha = alpha_animator.get_value();

    auto window_size = ImGui::GetIO().DisplaySize;
    auto cheat_window_size = cheat_window.get_scaled_size();

    ImGui::SetNextWindowPos({ (window_size.x - cheat_window_size.x) / 2.f, (window_size.y - cheat_window_size.y) / 2.f }, ImGuiCond_Once);

    for (auto& script : g_script_engine->get_scripts())
        for (auto& callback : script.callbacks)
            if (callback.callback_name == "render")
                g_script_engine->run_function(callback.function);

    if (init_hook && !finished_start)
    {

        finished_start = true;
        loading_alpha_animator.start_animation(false);

    }

#ifndef NO_SECURITY
    if (loading_alpha_animator.get_value() > 0.f)
    {

        std::string loading_text = "loading annihilate (";

        loading_text += std::to_string(items_through) + std::string("/") + std::string("17)");

        auto size = ImGui::GetIO().DisplaySize;
        auto rect = ImRect({ 0.f, 0.f }, size);

        auto draw = ImGui::GetForegroundDrawList();
        auto imgui_alpha = loading_alpha_animator.get_value();

        draw->AddRectFilledMultiColor(rect.Min, rect.Max, IM_COL32(37, 37, 37, 200 * imgui_alpha), IM_COL32(37, 37, 37, 200 * imgui_alpha), IM_COL32(17, 17, 17, 100 * imgui_alpha), IM_COL32(17, 17, 17, 100 * imgui_alpha));

        ImGui::RenderColorRectWithAlphaCheckerboard(draw, rect.Min, rect.Max, ImColor(25, 25, 25, static_cast<int>(200 * imgui_alpha)), 4.f * cheat_window.get_dpi_scale(), ImVec2(6, 6) * cheat_window.get_dpi_scale(), loading_alpha_animator.get_value());

        auto font = framework::g_scaled_fonts.at(4);
        auto text_sz = font.font->CalcTextSizeA(font.size, FLT_MAX, 0.f, loading_text.c_str());

        auto text_pos = (rect.Max - text_sz) * 0.5f;

        constexpr auto alpha_freq = 255 / 0.8f;
        static float alpha = 0;
        static bool should_reverse = false;

        if (!should_reverse)
            alpha += alpha_freq * ImGui::GetIO().DeltaTime;
        else
            alpha -= alpha_freq * ImGui::GetIO().DeltaTime;

        alpha = std::clamp(alpha, 0.f, 255.f);

        if (alpha >= 255.f)
            should_reverse = true;
        else if (alpha <= 0.f)
            should_reverse = false;

        imgui_alpha *= (alpha / 255.f);

        draw->AddText(font.font, font.size, text_pos - ImVec2(4.f, -4.f), IM_COL32(0, 0, 0, 255 * imgui_alpha), loading_text.c_str());

        ImGui::PushFont(font.font);
        ImGui::TextGradiented(loading_text.c_str(), draw, text_pos, IM_COL32(95, 92, 103, 255 * imgui_alpha), IM_COL32(200, 200, 200, 200 * imgui_alpha), 255.f * imgui_alpha, imgui_alpha * 255.f);
        ImGui::PopFont();
    }
#endif

    cheat_window.render();

    if (current_tab == 1)
    {

        auto window_pos = cheat_window.get_pos();
        auto window_sz = cheat_window.get_scaled_size();

        ImGui::SetNextWindowPos({ (window_pos.x + window_sz.x) + (20.f * cheat_window.get_dpi_scale()), window_pos.y });
        preview_window.set_dpi_scale(cheat_window.get_dpi_scale());
        preview_window.render();

        if (preview_animator.get_value() == 0.f)
            preview_animator.start_animation(true);

    }
    else if (preview_animator.get_value() == 1.f)
        preview_animator.start_animation(false);

	ImGui::Render();

	pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return oPresent(pSwapChain, SyncInterval, Flags);

}

uintptr_t original_hitmark{ NULL };
static void __stdcall hk_hitmark(vec3_t point, bool world_space, int new_hit)
{

    if (widgets::hitsound_combo->get_value() > 0 && (new_hit == 1 || new_hit == 2))
    {

        static std::string sounds_path = g_config_manager->get_documents_path() + std::string(("\\ann_sounds\\"));
        std::string sound_name = "";

        if (widgets::hitsound_combo->get_value() == 1)
            sound_name = "bell.wav";
        else if (widgets::hitsound_combo->get_value() == 2)
            sound_name = "cod.wav";
        else if (widgets::hitsound_combo->get_value() == 3)
            sound_name = "stapler.wav";
        else if (widgets::hitsound_combo->get_value() == 4)
            sound_name = widgets::custom_hitsound_name_input->get_value();

        PlaySoundA((sounds_path + sound_name).c_str(), NULL, SND_ASYNC | SND_FILENAME);

        auto volume_percent = widgets::hitsound_volume_slider->get_value();

        if (volume_percent >= 0 && volume_percent <= 100)
        {

            DWORD new_volume = ((DWORD)volume_percent * 65535) / 100;

            DWORD leftVolume = (new_volume & 0xFFFF) | ((new_volume & 0xFFFF) << 16);
            DWORD rightVolume = (new_volume & 0xFFFF) | ((new_volume & 0xFFFF) << 16);

            waveOutSetVolume(NULL, MAKELONG(leftVolume, rightVolume));

        }

    }

    reinterpret_cast<decltype(&hk_hitmark)>(original_hitmark)(point, widgets::force_3d_hitmarkers_checkbox->get_value() ? true : world_space, new_hit);

}

enum ERaycastInfoUsage
{

    Punch,
    ConsumeableAid,
    Melee,
    Gun,
    Bayonet,
    Refill,
    Tire,
    Battery,
    Detonator,
    Carlockpick,
    Fuel,
    Carjack,
    Grower,
    ArrestStart,
    ArrestEnd

};

uintptr_t original_send_raycast{ NULL };
void __cdecl hk_send_raycast(void* thisptr, raycast_info_t* raycast_info, ERaycastInfoUsage usage)
{

    if (widgets::force_hitbox_combo->get_value() > 0)
    {

        if (raycast_info->player() || raycast_info->zombie())
            raycast_info->limb(widgets::force_hitbox_combo->get_value() == 1 ? ELimb::SKULL : ELimb::SPINE);

    }

    reinterpret_cast<decltype(&hk_send_raycast)>(original_send_raycast)(thisptr, raycast_info, usage);

}

shader_t* standard_shader = nullptr;
shader_t* flat_shader = nullptr;

enum CompareFunction
{

    Disabled,
    Never,
    Less,
    Equal,
    LessEqual,
    Greater,
    NotEqual,
    GreaterEqual,
    Always

};

__forceinline bool apply_chams(skinned_mesh_renderer_t* renderer, bool is_local = false, bool is_hands = false)
{

    auto first_renderer = renderer;

    if (!first_renderer)
        return false;

    auto material = first_renderer->get_material();

    if (!material)
        return false;

    auto current_shader = material->get_shader();

    shader_t* target_shader = nullptr;

    if (widgets::chams_combo->get_value() == 1)
        target_shader = flat_shader;

    if (current_shader == target_shader)
    {

        if (current_shader == flat_shader)
        {

            auto chams_col = widgets::chams_colorpicker->get_value();

            if (is_local)
                chams_col = widgets::local_player_chams_colorpicker->get_value();
            else if (is_hands)
                chams_col = widgets::hand_chams_colorpicker->get_value();

            material->set_color(std::string("_Color"), { { chams_col.r, chams_col.g, chams_col.b, chams_col.a } });

        }

        auto ztest = widgets::chams_through_walls_checkbox->get_value() ? CompareFunction::Always : CompareFunction::LessEqual;

        if (is_local)
            ztest = CompareFunction::Always;
        else if (is_hands)
            ztest = CompareFunction::LessEqual;

        material->set_int(std::string("_ZTest"), ztest);

        return false;

    }

    material->set_shader(target_shader);

    return true;

}

__forceinline void unapply_chams(skinned_mesh_renderer_t* renderer)
{

    auto material = renderer->get_material();

    if (!material)
        return;

    if (material->get_shader() == flat_shader)
        material->set_shader(standard_shader);

}

uintptr_t original_player_update{ NULL };
void __cdecl hk_player_update(player_t* thisptr)
{

    reinterpret_cast<decltype(&hk_player_update)>(original_player_update)(thisptr);

    if (!cheat::local_player.player || thisptr == cheat::local_player.player)
    {

        auto animator = thisptr->animator();

        if (animator)
        {

            auto first_renderer = animator->first_renderer();

            auto second_renderer = animator->third_renderer_1();

            auto third_renderer = animator->third_renderer_2();

            if (widgets::local_player_chams_checkbox->get_value())
            {

                if (second_renderer)
                    apply_chams(second_renderer, true);

                if (third_renderer)
                    apply_chams(third_renderer, true);

            }
            else
            {

                if (second_renderer)
                    unapply_chams(second_renderer);

                if (third_renderer)
                    unapply_chams(third_renderer);

            }

            if (widgets::hand_chams_checkbox->get_value())
                if (first_renderer)
                    apply_chams(first_renderer, false, true);
            else
                if (first_renderer)
                    unapply_chams(first_renderer);

        }

        auto look = thisptr->look();

        return;

    }

    if (!flat_shader)
        flat_shader = shader_t::find(std::string("Hidden/Internal-Colored"));

    if (!standard_shader)
        standard_shader = shader_t::find(std::string("Standard"));

    auto look = thisptr->look();
    auto local_look = cheat::local_player.playerlook;

    if (look && local_look)
    {

        auto ptr = look->aim();
        auto ptr2 = local_look->aim();

        if (ptr)
        {

            auto local_aim_ptr = driver.read<uintptr_t>(ptr + 0x10);

            if (local_aim_ptr)
            {

                auto transform = TransformInternal(local_aim_ptr);

                bool in_list = false;

                g_vis_mtx.lock();

                for (int i = 0; i < g_vis_data.size(); i++)
                {

                    auto& data = g_vis_data.at(i);

                    if (!data.ptr || (data.vis_time + 1500) < GetTickCount64())
                    {

                        g_vis_data.erase(g_vis_data.begin() + i);
                        continue;

                    }

                }

                g_vis_mtx.unlock();

                g_vis_mtx.lock();

                for (auto& data : g_vis_data)
                {

                    if (data.ptr == thisptr)
                    {

                        data.visible = game::is_visible(game::camera.position, transform.position());
                        data.vis_time = GetTickCount64();
                        in_list = true;

                    }

                }

                g_vis_mtx.unlock();

                g_vis_mtx.lock();

                if (!in_list)
                    g_vis_data.emplace_back<player_visibility_data_t>({ thisptr, GetTickCount64(), game::is_visible(game::camera.position, transform.position()) });

                g_vis_mtx.unlock();

            }

        }

    }

    auto animator = thisptr->animator();

    if (animator)
    {

        auto second_renderer = animator->third_renderer_1();

        auto third_renderer = animator->third_renderer_2();

        if (widgets::chams_combo->get_value() > 0)
        {

            if (second_renderer)
                apply_chams(second_renderer);

            if (third_renderer)
                apply_chams(third_renderer);

        }
        else
        {

            if (second_renderer)
                unapply_chams(second_renderer);

            if (third_renderer)
                unapply_chams(third_renderer);

        }

    }

}

uintptr_t original_damage_raycast{ NULL };
static raycast_info_t* __stdcall hk_damage_raycast(void* ray, float range, int mask, player_t* ignore_player)
{

    if (range == 1.75f && widgets::extend_range_combo->get_value(0))
        range = 8.f;

    auto raycast_info = reinterpret_cast<decltype(&hk_damage_raycast)>(original_damage_raycast)(ray, range, mask, ignore_player);

    uintptr_t aim_transform_ptr = cheat::local_player.playerlook == nullptr ? NULL : cheat::local_player.playerlook->aim();

    if (aim_transform_ptr)
    {

        auto local_aim_ptr = driver.read<uintptr_t>(aim_transform_ptr + 0x10);

        if (local_aim_ptr)
        {

            auto aim_transform = TransformInternal(local_aim_ptr);
            auto origin_point = aim_transform.position();

            if (g_aimbot->m_last_target_time && g_aimbot->m_last_target.player.player)
            {

                auto target_transform = (uintptr_t)g_aimbot->m_last_target.player.player->get_transform();

                auto visible = game::is_visible(game::camera.position, g_aimbot->m_last_aim_pos);
                auto check_visible = !widgets::aimbot_ignore_occlusion_checkbox->get_value();

                if (check_visible && visible || !check_visible)
                {

                    raycast_info->transform(target_transform);

                    raycast_info->point(g_aimbot->m_last_aim_pos);

                    raycast_info->direction(aim_transform.forward());

                    auto limb_override = ELimb::LIMB_MAX;

                    if (widgets::force_hitbox_combo->get_value() > 0)
                        if (widgets::force_hitbox_combo->get_value() == 1)
                            limb_override = ELimb::SKULL;
                        else
                            limb_override = ELimb::SPINE;

                    raycast_info->limb(limb_override != ELimb::LIMB_MAX ? limb_override : g_aimbot->m_target_limb);

                    raycast_info->material(NULL);

                    raycast_info->player((uintptr_t)g_aimbot->m_last_target.player.player);

                    raycast_info->distance(origin_point.distance(g_aimbot->m_last_aim_pos));

                }

                g_aimbot->m_last_target_time = NULL;
                g_aimbot->m_last_aim_pos = vec3_t{};

            }

            auto final_point = raycast_info->point();

            if (widgets::bullet_tracer_checkbox->get_value() && final_point.non_zero())
                g_bullet_tracers->tracer_list.emplace_back<tracer_info_t>({ GetTickCount64(), origin_point, final_point});

        }

    }

    return raycast_info;

}

uintptr_t original_melee_fire{ NULL };
void __cdecl hk_melee_fire(usable_t* thisptr)
{

    static NaOrganization::MidTerm::NaResolver::Class player_caller = NaOrganization::MidTerm::UnityResolver.GetClass(NaOrganization::assembly_path, "SDG.Unturned", "PlayerCaller");
    static auto player_caller_get_player = NaOrganization::MidTerm::UnityResolver.GetMethod(player_caller, "SDG.Unturned.Player", "get_player", { });

    if (widgets::extend_range_combo->get_value(1))
    {

        auto player = NaOrganization::MidTerm::NaMethodInvoker<player_t*, void*>(player_caller_get_player.method.GetInvokeAddress()).Invoke(thisptr);

        if (player)
        {

            auto equipment = player->equipment();

            if (equipment)
            {

                auto asset = (item_weapon_asset_t*)equipment->asset();

                if (asset)
                    asset->range(8.f);

            }

        }

    }

    reinterpret_cast<decltype(&hk_melee_fire)>(original_melee_fire)(thisptr);

}

uintptr_t original_set_sky_color{ NULL };
static void hk_set_sky_color(UnityEngine_Color_o color)
{

    if (widgets::sky_color_checkbox->get_value())
    {
        
        auto col = widgets::sky_color_picker->get_value();

        color.fields.r = col.r;
        color.fields.g = col.g;
        color.fields.b = col.b;
        color.fields.a = col.a;

    }

    reinterpret_cast<decltype(&hk_set_sky_color)>(original_set_sky_color)(color);

}

uintptr_t original_set_equator_color{ NULL };
static void hk_set_equator_color(UnityEngine_Color_o color)
{

    if (widgets::sky_color_checkbox->get_value())
    {

        auto col = widgets::sky_color_picker->get_value();

        color.fields.r = col.r;
        color.fields.g = col.g;
        color.fields.b = col.b;
        color.fields.a = col.a;

    }

    reinterpret_cast<decltype(&hk_set_equator_color)>(original_set_equator_color)(color);

}

uintptr_t original_pain{ NULL };
static void hk_pain(float amount)
{

    if (widgets::disable_pain_checkbox->get_value())
        amount = 0.f;

    reinterpret_cast<decltype(&hk_pain)>(original_pain)(amount);

}

uintptr_t original_stun{ NULL };
static void hk_stun(UnityEngine_Color_o color, float amount)
{

    if (widgets::disable_stun_checkbox->get_value())
        amount = 0.f;

    reinterpret_cast<decltype(&hk_stun)>(original_stun)(color, amount);

}

uintptr_t original_is_hallucinating{ NULL };
static void hk_is_hallucinating(player_ui_t* thisptr, bool active)
{

    if (widgets::disable_hallucinations_checkbox->get_value())
        active = false;

    reinterpret_cast<decltype(&hk_is_hallucinating)>(original_is_hallucinating)(thisptr, active);

}

//uintptr_t original_layerer_viewmodel{ NULL };
//static void hk_viewmodel(mono_behaviour_t* target)
//{
//
//    static auto renderer_class = NaOrganization::MidTerm::UnityResolver.GetClass(NaOrganization::core_module_path, "UnityEngine", "Renderer");
//    auto type = mono::mono_type_get_object((MonoDomain*)mono::get_root_domain(), renderer_class.klass);
//
//    std::cout << "[+] type ptr: " << type << "\n";
//
//    auto renderer = target->get_component(type);
//
//    if (renderer)
//        std::cout << "[+] got viewmodel renderer / " << renderer << "\n";
//    else
//        std::cout << "[-] failed to get viewmodel renderer\n";
//
//    reinterpret_cast<decltype(&hk_viewmodel)>(original_layerer_viewmodel)(target);
//
//}

ULONGLONG last_fire_time = NULL;

int retry_attempts{ 0 };

__forceinline void stripExtension(std::string& path)
{
    int dot = path.rfind("Unturned.exe");
    if (dot != std::string::npos)
    {
        path.resize(dot);
    }
}

DWORD WINAPI MainThread(LPVOID lpReserved)
{
#ifndef NO_SECURITY
    VIRTUALIZER_FISH_RED_START;
#endif

    DEBUG_LOG("YAA! WE ENTEROD THE MAIN THREAD");

    while (GetModuleHandleA("GameAssembly.dll") == INVALID_HANDLE_VALUE)
    {

        Sleep(350);
        continue;

    }

    DEBUG_LOG("We gooot game assembly!");

	do
	{
		if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
		{
            DEBUG_LOG("KIERO INITIALIZED SUCCESSFULLY!");

			kiero::bind(8, (void**)& oPresent, hkPresent);

            items_through++;

#ifndef NO_SECURITY
            while (!client.create())
            {

                Sleep(100);

                ++retry_attempts;

                if (retry_attempts >= 3)
                {

                    auto last_error = WSAGetLastError();

                    std::stringstream ss;

                    ss << "failed to connect to main server | err: " << last_error;

                    MessageBoxA(NULL, ss.str().c_str(), NULL, NULL);

                    return EXIT_FAILURE;

                }

            }
#endif

            items_through++;

#ifndef NO_SECURITY
            mylar::network::packet_t info_request{mylar::constants::MODULE_GET_USER_INFO};

            if (!client.out(info_request))
            {

                MessageBoxA(NULL, "failed to request server", NULL, NULL);

                exit(0);

                return EXIT_FAILURE;

            }
#endif

            items_through++;

#ifndef NO_SECURITY
            mylar::network::packet_t info_response{};

            if (!client.in(info_response) || info_response.m_type != mylar::constants::MODULE_GET_USER_INFO_ACK)
            {

                MessageBoxA(NULL, "failed to recieve response", NULL, NULL);

                exit(0);

                return EXIT_FAILURE;

            }
#endif

            items_through++;

#ifndef NO_SECURITY
            if (info_response.m_client_heartbeat_time > 0)
                user_name = std::string(info_response.m_username);
            else
                return EXIT_FAILURE;
#endif

#ifdef _DEBUG

            AllocConsole();

            if (!freopen(("CONOUT$"), ("w"), stdout))
            {

                FreeConsole();
                return NULL;

            }

#endif

            static_modules::game_assembly = (uintptr_t)GetModuleHandleA("UnityPlayer.dll");

            if (static_modules::game_assembly == NULL)
            {
                MessageBoxA(NULL, "failed to initialize (1)", NULL, NULL);
                return EXIT_FAILURE;
            }

            items_through++;

            static_modules::mono = (uintptr_t)GetModuleHandleA("mono-2.0-bdwgc.dll");

            if (static_modules::mono == NULL)
            {
                MessageBoxA(NULL, "failed to initialize (2)", NULL, NULL);
                return EXIT_FAILURE;
            }

            items_through++;

            mono::init();

            items_through++;

            unity_classes::init();

            items_through++;

            class_offsets::init();

            items_through++;

            if (!NaOrganization::MidTerm::UnityResolver.Setup())
            {

                LI_FN(MessageBoxA).safe()(NULL, "failed to initialize (3)", NULL, NULL);
                return NULL;

            }

            items_through++;

            HMODULE hExe = GetModuleHandleW(NULL);
            WCHAR fullPath[MAX_PATH]{ 0 };
            GetModuleFileNameW(hExe, fullPath, MAX_PATH);
            std::filesystem::path path(fullPath);
            std::filesystem::path filename = path.filename();

            if (path.empty())
            {

                LI_FN(MessageBoxA).safe()(NULL, "failed to initialize (4)", NULL, NULL);
                return NULL;

            }

            items_through++;

            std::wstring wide_path = std::wstring(fullPath);
            std::string real_path = std::string(wide_path.begin(), wide_path.end());

            stripExtension(real_path);

            std::string core_module_path = real_path + "Unturned_Data\\Managed\\UnityEngine.CoreModule.dll";
            std::string asset_bundle_path = real_path + "Unturned_Data\\Managed\\UnityEngine.AssetBundleModule.dll";
            std::string physics_path = real_path + "Unturned_Data\\Managed\\UnityEngine.PhysicsModule.dll";

            real_path += "Unturned_Data\\Managed\\Assembly-CSharp.dll";

            NaOrganization::assembly_path = real_path;
            NaOrganization::core_module_path = core_module_path;
            NaOrganization::asset_bundle_path = asset_bundle_path;
            NaOrganization::physics_module = physics_path;

            NaOrganization::MidTerm::NaResolver::Class player_ui = NaOrganization::MidTerm::UnityResolver.GetClass(real_path, "SDG.Unturned", "PlayerUI");
            auto hitmark_ptr = NaOrganization::MidTerm::UnityResolver.GetMethod(player_ui, "System.Void", "hitmark", { "UnityEngine.Vector3", "System.Boolean", "SDG.Unturned.EPlayerHit" });
            auto pain_ptr = NaOrganization::MidTerm::UnityResolver.GetMethod(player_ui, "System.Void", "pain", { "System.Single" });
            auto stun_ptr = NaOrganization::MidTerm::UnityResolver.GetMethod(player_ui, "System.Void", "stun", { "UnityEngine.Color", "System.Single" });
            auto is_hallucinating = NaOrganization::MidTerm::UnityResolver.GetMethod(player_ui, "System.Void", "setIsHallucinating", { "System.Boolean" });

            if (!hitmark_ptr.method.GetInvokeAddress() || !pain_ptr.method.GetInvokeAddress() || !stun_ptr.method.GetInvokeAddress() || !is_hallucinating.method.GetInvokeAddress())
            {

                LI_FN(MessageBoxA).safe()(NULL, "failed to initialize (h:1)", NULL, NULL);
                return EXIT_FAILURE;

            }

            NaOrganization::MidTerm::NaResolver::Class player = NaOrganization::MidTerm::UnityResolver.GetClass(real_path, "SDG.Unturned", "Player");
            auto player_update = NaOrganization::MidTerm::UnityResolver.GetMethod(player, "System.Void", "Update", {  });

            if (!player_update.method.GetInvokeAddress())
            {

                LI_FN(MessageBoxA).safe()(NULL, "failed to initialize (h:2)", NULL, NULL);
                return EXIT_FAILURE;

            }

            NaOrganization::MidTerm::NaResolver::Class player_input = NaOrganization::MidTerm::UnityResolver.GetClass(real_path, "SDG.Unturned", "PlayerInput");
            auto send_raycast_ptr = NaOrganization::MidTerm::UnityResolver.GetMethod(player_input, "System.Void", "sendRaycast", { "SDG.Unturned.RaycastInfo", "SDG.Unturned.ERaycastInfoUsage" });

            if (!send_raycast_ptr.method.GetInvokeAddress())
            {

                LI_FN(MessageBoxA).safe()(NULL, "failed to initialize (h:3)", NULL, NULL);
                return EXIT_FAILURE;

            }

            NaOrganization::MidTerm::NaResolver::Class useable_melee = NaOrganization::MidTerm::UnityResolver.GetClass(real_path, "SDG.Unturned", "UseableMelee");
            auto fire_ptr = NaOrganization::MidTerm::UnityResolver.GetMethod(useable_melee, "System.Void", "fire", { });

            if (!fire_ptr.method.GetInvokeAddress())
            {

                LI_FN(MessageBoxA).safe()(NULL, "failed to initialize (h:4)", NULL, NULL);
                return EXIT_FAILURE;

            }

            NaOrganization::MidTerm::NaResolver::Class damage_tool = NaOrganization::MidTerm::UnityResolver.GetClass(real_path, "SDG.Unturned", "DamageTool");
            auto damage_raycast_ptr = NaOrganization::MidTerm::UnityResolver.GetMethod(damage_tool, "SDG.Unturned.RaycastInfo", "raycast", { "UnityEngine.Ray", "System.Single", "System.Int32", "SDG.Unturned.Player" });

            if (!damage_raycast_ptr.method.GetInvokeAddress())
            {

                LI_FN(MessageBoxA).safe()(NULL, "failed to initialize (h:5)", NULL, NULL);
                return EXIT_FAILURE;

            }

            NaOrganization::MidTerm::NaResolver::Class level_lighting = NaOrganization::MidTerm::UnityResolver.GetClass(real_path, "SDG.Unturned", "LevelLighting");
            auto set_sky_color = NaOrganization::MidTerm::UnityResolver.GetMethod(level_lighting, "System.Void", "setSkyColor", { "UnityEngine.Color" });
            auto set_equator_color = NaOrganization::MidTerm::UnityResolver.GetMethod(level_lighting, "System.Void", "setEquatorColor", { "UnityEngine.Color" });

            if (!set_sky_color.method.GetInvokeAddress() || !set_equator_color.method.GetInvokeAddress())
            {

                LI_FN(MessageBoxA).safe()(NULL, "failed to initialize (h:6)", NULL, NULL);
                return EXIT_FAILURE;

            }

            //NaOrganization::MidTerm::NaResolver::Class layerer = NaOrganization::MidTerm::UnityResolver.GetClass(real_path, "SDG.Unturned", "Layerer");
            //auto viewmodel = NaOrganization::MidTerm::UnityResolver.GetMethod(layerer, "System.Void", "viewmodel", { "UnityEngine.Transform" });

            items_through++;

            MH_CreateHook((uintptr_t*)hitmark_ptr.method.GetInvokeAddress(), hk_hitmark, (LPVOID*)&original_hitmark);
            MH_CreateHook((uintptr_t*)pain_ptr.method.GetInvokeAddress(), hk_pain, (LPVOID*)&original_pain);
            MH_CreateHook((uintptr_t*)stun_ptr.method.GetInvokeAddress(), hk_stun, (LPVOID*)&original_stun);
            MH_CreateHook((uintptr_t*)is_hallucinating.method.GetInvokeAddress(), hk_is_hallucinating, (LPVOID*)&original_is_hallucinating);
            MH_CreateHook((uintptr_t*)send_raycast_ptr.method.GetInvokeAddress(), hk_send_raycast, (LPVOID*)&original_send_raycast);
            MH_CreateHook((uintptr_t*)player_update.method.GetInvokeAddress(), hk_player_update, (LPVOID*)&original_player_update);
            MH_CreateHook((uintptr_t*)fire_ptr.method.GetInvokeAddress(), hk_melee_fire, (LPVOID*)&original_melee_fire);
            MH_CreateHook((uintptr_t*)damage_raycast_ptr.method.GetInvokeAddress(), hk_damage_raycast, (LPVOID*)&original_damage_raycast);
            MH_CreateHook((uintptr_t*)set_sky_color.method.GetInvokeAddress(), hk_set_sky_color, (LPVOID*)&original_set_sky_color);
            MH_CreateHook((uintptr_t*)set_equator_color.method.GetInvokeAddress(), hk_set_equator_color, (LPVOID*)&original_set_equator_color);
            //MH_CreateHook((uintptr_t*)viewmodel.method.GetInvokeAddress(), hk_viewmodel, (LPVOID*)&original_layerer_viewmodel);

            items_through++;

            if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
            {

                LI_FN(MessageBoxA).safe()(NULL, "failed to initialize (5)", NULL, NULL);
                return EXIT_FAILURE;

            }

            items_through++;

            auto documents_folder = g_config_manager->get_documents_path();

            LI_FN(CreateDirectoryA).safe()(std::string(documents_folder + "\\ann_configs").c_str(), NULL);
            LI_FN(CreateDirectoryA).safe()(std::string(documents_folder + "\\ann_scripts").c_str(), NULL);

            items_through++;

            setup_sounds(documents_folder);

            items_through++;

            if (!g_script_engine->initialize())
            {

                LI_FN(MessageBoxA).safe()(NULL, "failed to initialize (6)", NULL, NULL);
                return EXIT_FAILURE;

            }

            items_through++;

			init_hook = true;
		}
        else
        {
            LI_FN(MessageBoxA).safe()(NULL, "failed to initialize (0)", NULL, NULL);

            exit(0);
        }

	} while (!init_hook);

#ifndef NO_SECURITY
    VIRTUALIZER_FISH_RED_END;
#endif

	return TRUE;
}

HMODULE g_module{ NULL };
BOOL WINAPI DllMain(HMODULE module_instance, DWORD call_reason, LPVOID lpReserved)
{

	if (call_reason != DLL_PROCESS_ATTACH)
		return false;

    	g_module = module_instance;
        auto handle = CreateThread(nullptr, 0, &MainThread, nullptr, 0, nullptr);

	return true;

}