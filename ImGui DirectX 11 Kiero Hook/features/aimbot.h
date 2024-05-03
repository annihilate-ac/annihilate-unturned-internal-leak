#pragma once

#include "feature.h"

namespace widgets
{

	c_checkbox* aimbot_checkbox = nullptr;
	c_keybind* aimbot_keybind = nullptr; c_binding aimbot_key{ "aimbot" };
	c_checkbox* aimbot_only_visible = nullptr;
	c_slider_float* aimbot_fov_slider = nullptr;
	c_checkbox* aimbot_draw_fov_checkbox = nullptr;
	c_colorpicker* aimbot_fov_color = nullptr;
	c_slider_float* aimbot_max_distance_slider = nullptr;
	c_combo* aimbot_sorting_combo = nullptr;
	c_combo* aimbot_hitbox_combo = nullptr;
	c_combo* aimbot_hitbox_override_combo = nullptr;
	c_keybind* aimbot_hitbox_override_keybind = nullptr; c_binding aimbot_hitbox_override_key{ "hitbox override" };
	c_checkbox* aimbot_avoid_friendly_checkbox = nullptr;
	c_combo* aimbot_smoothing_combo = nullptr;
	c_slider_float* aimbot_smoothing_slider = nullptr;
	c_combo* aimbot_type_combo = nullptr;
	c_checkbox* aimbot_target_color_override_checkbox = nullptr;
	c_colorpicker* aimbot_target_color_override_color = nullptr;
	c_multi_combo* aimbot_show_target_combo = nullptr; c_colorpicker* aimbot_target_color = nullptr;
	c_slider_float* aimbot_show_target_size = nullptr;
	c_checkbox* aimbot_ignore_occlusion_checkbox = nullptr;
	c_checkbox* aimbot_auto_shoot_checkbox = nullptr;
	c_multi_combo* extend_range_combo = nullptr;

}

struct aimbot_target_t
{

	cached_player_t player;
	float fov, distance;
	vec3_t origin, head_pos;
	ImVec2 head_screen;
	vec3_t aimed_pos;
	bool off_screen;

};

class c_aimbot
{

private:

	std::vector<aimbot_target_t> m_targets{};

	__forceinline ImVec2 smooth_angle(ImVec2 angles, ImVec2 localViewAngles)
	{

		ImVec2 delta = ImVec2(angles.x - localViewAngles.x, angles.y - localViewAngles.y);

		delta.normalize();

		float smoothing_value = 5.5f * (widgets::aimbot_smoothing_slider->get_value()) / 100.f;
		float smooth = powf(0.81f + smoothing_value, 0.4f);

		smooth = min(0.98f, smooth);

		ImVec2 toChange = ImVec2();

		float coeff = (1.0f - smooth) / delta.length() * 4.f;

		if (widgets::aimbot_smoothing_combo->get_value() == 1)
			coeff = powf(coeff, 2.f) * 80.f / widgets::aimbot_smoothing_slider->get_value();
		else if (widgets::aimbot_smoothing_combo->get_value() == 2)
			coeff = powf(coeff * coeff * 80.f / widgets::aimbot_smoothing_slider->get_value(), 1.5f);

		coeff = min(1.f, coeff);
		toChange = delta * coeff;

		return ImVec2(localViewAngles.x, localViewAngles.y) + toChange;
	}

public:

	ULONGLONG m_last_target_time{};
	ULONGLONG m_last_fire_time{};
	vec3_t m_last_aim_pos{};
	aimbot_target_t m_last_target{};
	ELimb m_target_limb{};
	bool m_fire_full_auto{};
	bool m_has_target{};

	__forceinline void set_mouse(bool val) { mouse_event(val ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP, NULL, NULL, 0, 0); }

	void run(bool menu_open)
	{

		m_has_target = false;

		if (m_targets.size() < 1)
			return;

		if (!cheat::local_player.player || !cheat::local_player.playerlook || !cheat::local_player.animator)
			return;

		auto compare_target = [&](const aimbot_target_t& t1, const aimbot_target_t& t2)
		{

			if (widgets::aimbot_sorting_combo->get_value() == 0)
				return t1.fov < t2.fov;
			else if (widgets::aimbot_sorting_combo->get_value() == 1)
			{

				auto dst_delta = fabsf(t1.distance - t2.distance);
				auto t1_dst_flag = t1.distance < t2.distance;

				if (dst_delta > 50.f)
				{

					if (t1.off_screen || t2.off_screen)
						return t1_dst_flag;

					return t1.fov < t2.fov;

				}
				else if (!t1.off_screen && !t2.off_screen)
					return t1.fov < t2.fov;

				return t1_dst_flag;

			}

		};

		std::sort(m_targets.begin(), m_targets.end(), compare_target);

		aimbot_target_t& target = m_targets.at(0);

		if (!target.player.player || !target.player.movement || !target.player.transform)
			return;

		if (widgets::aimbot_fov_slider->get_value() != 1001.f)
		{

			if (target.fov > widgets::aimbot_fov_slider->get_value())
				return;

		}

		if (!cheat::local_player.playerlook)
			return;

		auto base_pos = target.origin;
		auto head_pos = target.head_pos - vec3_t{ 0.f, 0.22f, 0.f };

		auto aimbot_hitbox = widgets::aimbot_hitbox_combo->get_value();

		if (widgets::aimbot_hitbox_override_key.get_enabled() && widgets::aimbot_hitbox_override_combo->get_value() > 0)
			aimbot_hitbox = (widgets::aimbot_hitbox_override_combo->get_value() - 1);

		auto height = target.player.movement->get_height();

		if (aimbot_hitbox == 1)
		{

			auto target_aim = target.player.playerlook->aim();

			if (target_aim)
			{

				auto ptr = driver.read<uintptr_t>(target_aim + 0x10);

				if (ptr)
				{

					auto target_aim_transform = TransformInternal(ptr);

					head_pos = target_aim_transform.position();
					m_target_limb = ELimb::SKULL;

				}

			}

		}
		else if (aimbot_hitbox == 2)
		{

			head_pos = vec3_t{ base_pos.x, base_pos.y + (height * 0.82f), base_pos.z };
			m_target_limb = ELimb::SPINE;

		}
		else if (aimbot_hitbox == 3)
		{

			auto decider = rand() % 2;

			if (decider == 0)
			{

				auto target_aim = target.player.playerlook->aim();

				if (target_aim)
				{

					auto ptr = driver.read<uintptr_t>(target_aim + 0x10);

					if (ptr)
					{

						auto target_aim_transform = TransformInternal(ptr);

						head_pos = target_aim_transform.position();
						m_target_limb = ELimb::SKULL;

					}

				}

			}
			else
			{

				head_pos = vec3_t{ base_pos.x, base_pos.y + (height * 0.82f), base_pos.z };
				m_target_limb = ELimb::SPINE;

			}

		}

		if (widgets::aimbot_type_combo->get_value() == 0)
		{

			auto animator = cheat::local_player.animator;

			if (animator)
			{

				auto look = cheat::local_player.playerlook;
				auto ang = game::calculate_angle(game::camera.position, head_pos, {});

				if (look)
				{

					auto viewangles = look->viewangles();
					auto smoothed_angle = ImVec2{};

					if (ang.y <= 90.f && ang.y <= 270.f) ang.y += 90.f;
					else if (ang.y >= 270.f && ang.y <= 360.f) ang.y -= 270.f;

					auto enable_smoothing = widgets::aimbot_smoothing_combo->get_value() > 0;

					if (enable_smoothing)
						smoothed_angle = smooth_angle({ ang.y, ang.x }, viewangles);

					look->viewangles(enable_smoothing ? smoothed_angle : ImVec2{ ang.y, ang.x });

					auto time = GetTickCount64();

					m_last_aim_pos = head_pos;
					m_last_target_time = time;
					m_last_target = target;

				}

			}

		}
		else if (widgets::aimbot_type_combo->get_value() == 1)
		{

			auto time = GetTickCount64();

			m_last_aim_pos = head_pos;
			m_last_target_time = time;
			m_last_target = target;

			if (widgets::aimbot_show_target_combo->get_active_values())
			{

				auto hit_screen = ImVec2{};

				auto screen = game::to_screen(head_pos, hit_screen);

				static auto screen_size = ImGui::GetIO().DisplaySize;
				auto screen_center = screen_size * 0.5f;

				// under crosshair
				if (widgets::aimbot_show_target_combo->get_value(0))
				{

					std::string target_text = "current target: " + target.player.name;

					ImGui::PushFont(framework::g_scaled_fonts.at(2).font);
					auto text_sz = ImGui::CalcTextSize(target_text.c_str());
					ImGui::PopFont();

					render.text_shadowed({ screen_center.x - (text_sz.x * 0.5f), screen_center.y + 24.f }, target_text, widgets::aimbot_target_color->get_value().u32(), &framework::g_scaled_fonts.at(2));

				}

				if (screen)
				{

					// line
					if (widgets::aimbot_show_target_combo->get_value(1))
						render.line(screen_center, hit_screen, widgets::aimbot_target_color->get_value().u32(), 2.f);

					// cross
					if (widgets::aimbot_show_target_combo->get_value(2))
					{

						auto k_size = widgets::aimbot_show_target_size->get_value();

						render.line({ hit_screen.x - k_size, hit_screen.y - k_size }, { hit_screen.x - (k_size / 2.f), hit_screen.y - (k_size / 2.f) }, widgets::aimbot_target_color->get_value().u32(), 2.f);

						render.line({ hit_screen.x - k_size, hit_screen.y + k_size }, { hit_screen.x - (k_size / 2.f), hit_screen.y + (k_size / 2.f) }, widgets::aimbot_target_color->get_value().u32(), 2.f);

						render.line({ hit_screen.x + k_size, hit_screen.y + k_size }, { hit_screen.x + (k_size / 2.f), hit_screen.y + (k_size / 2.f) }, widgets::aimbot_target_color->get_value().u32(), 2.f);

						render.line({ hit_screen.x + k_size, hit_screen.y - k_size }, { hit_screen.x + (k_size / 2.f), hit_screen.y - (k_size / 2.f) }, widgets::aimbot_target_color->get_value().u32(), 2.f);

					}

				}

			}

		}

		auto any_active = (menu_open);

		if (widgets::aimbot_type_combo->get_value() == 1 && !m_last_target_time)
			any_active = true;

		if (GetForegroundWindow() != FindWindowA(NULL, "Unturned"))
			any_active = true;

		if (!any_active && widgets::aimbot_auto_shoot_checkbox->get_value() && m_last_fire_time < GetTickCount64())
		{

			auto firemode = 1;

			if (cheat::local_gun && cheat::local_gun->type() == EItemType::GUN)
				firemode = cheat::local_gun->firemode();

			if (firemode != 0)
			{

				if (firemode == 1 || firemode == 3)
				{

					set_mouse(true);
					set_mouse(false);

					m_last_fire_time = GetTickCount64();

				}
				else if (firemode == 2)
				{
					
					set_mouse(true);

					m_last_fire_time = GetTickCount64();
					m_fire_full_auto = true;

				}

			}

		}

		m_has_target = true;

	}

	void collect_targets()
	{

		m_targets.clear();

		if (!widgets::aimbot_key.get_enabled() || !widgets::aimbot_checkbox->get_value() || !cheat::local_player.equipment)
			return;

		auto current_asset = cheat::local_player.equipment->asset();

		bool using_silent = widgets::aimbot_type_combo->get_value() == 1;
		bool using_fists = !current_asset;
		bool using_melee = false;
		bool extending_fist_range = widgets::extend_range_combo->get_value(0) > 0;
		bool extending_melee_range = widgets::extend_range_combo->get_value(1) > 0;
		auto weapon_asset = (item_weapon_asset_t*)current_asset;

		if (weapon_asset && weapon_asset->type() == EItemType::MELEE)
			using_melee = true;

		if ((using_fists || using_melee) && !using_silent)
			return;

		auto calc_fov = [&](ImVec2 pos)
		{
			static auto screen_center = ImVec2(ImGui::GetIO().DisplaySize.x / 2.f, ImGui::GetIO().DisplaySize.y / 2.f);
			return pos.distance(screen_center);
		};

		auto weapon_range = 0.f;

		if (cheat::local_gun && widgets::aimbot_max_distance_slider->get_value() == 0.f)
			weapon_range = cheat::local_gun->range();
		else if (using_fists)
			weapon_range = extending_fist_range ? 8.f : 1.75f;
		else if (using_melee)
			weapon_range = extending_melee_range ? 8.f : weapon_asset->range();

		if (weapon_range > 0.f)
			weapon_range = weapon_range * 3.281f;

		auto ignore_fov = widgets::aimbot_fov_slider->get_value() == 1001.f;

		for (auto& player : cheat::players)
		{

			if (player.playerlife && player.playerlife->dead())
				continue;

			bool vis_flag = false;

			if (!player.get_vis_data().visible && widgets::aimbot_only_visible->get_value())
				vis_flag = true;

			if (widgets::aimbot_ignore_occlusion_checkbox->get_value())
				vis_flag = false;

			if (!player.movement || !player.transform || vis_flag)
				continue;

			bool is_friendly = false;

			if (widgets::aimbot_avoid_friendly_checkbox->get_value())
				for (const auto& name : g_friendly_players)
					if (name == player.name)
						is_friendly = true;

			if (is_friendly)
				continue;

			vec3_t origin = player.transform->position();

			auto origin_screen = ImVec2{};

			auto height = player.movement->get_height();

			vec3_t head_pos = origin + vec3_t{ 0.f, height, 0.f };

			float ft_dst = game::camera.position.distance(origin) * get_unit_multiplier();

			auto head_screen = ImVec2{};

			auto off_screen = false;

			if (!game::to_screen(origin, origin_screen) || !game::to_screen(head_pos, head_screen))
				off_screen = true;

			if (off_screen && !ignore_fov)
				continue;

			auto player_fov = calc_fov(head_screen);

			if (!ignore_fov)
			{

				if (player_fov > widgets::aimbot_fov_slider->get_value())
					continue;

			}

			if (widgets::aimbot_max_distance_slider->get_value() == 0.f || using_melee || using_fists)
			{

				if (ft_dst > weapon_range)
					continue;

			}
			else if (ft_dst > widgets::aimbot_max_distance_slider->get_value())
				continue;

			m_targets.emplace_back<aimbot_target_t>({ player, player_fov, ft_dst, origin, head_pos, head_screen, {}, off_screen });

		}

	}

	aimbot_target_t get_last_target() { return m_last_target; }

}; inline c_aimbot* g_aimbot = new c_aimbot();