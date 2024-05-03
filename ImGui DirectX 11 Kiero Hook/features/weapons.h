#pragma once

#include "feature.h"

namespace widgets
{

	c_checkbox* no_sway_checkbox = nullptr;
	c_checkbox* no_recoil_checkbox = nullptr;
	c_slider_float* no_recoil_percentage_slider = nullptr;
	c_checkbox* no_spread_checkbox = nullptr;
	c_checkbox* no_shake_checkbox = nullptr;
	c_checkbox* no_drop_checkbox = nullptr;
	c_checkbox* fast_bullet_checkbox = nullptr;
	c_checkbox* instant_aim_checkbox = nullptr;
	c_checkbox* no_animations_checkbox = nullptr;

}

class c_no_sway : c_feature
{

private:

public:

	virtual void run()
	{

		auto enabled = widgets::no_sway_checkbox->get_value();

		if (enabled)
		{

			if (!cheat::local_player.player)
				return;

			auto animator = cheat::local_player.animator;

			if (!animator)
				return;

			animator->scope_sway({ 0.f, 0.f, 0.f });

			m_enabled = true;

		}
		else
			m_enabled = false;

	}

}; inline c_no_sway* g_no_sway = new c_no_sway();

static bool force_recoil_update = false;

class c_no_recoil : c_feature
{

private:

	game::vec4 m_previous_recoil{};
	item_gun_asset_t* m_previous_weapon{};

public:

	virtual void run()
	{

		if (!widgets::no_recoil_checkbox->get_value() || !cheat::local_player.player || !cheat::local_gun)
			return;

		auto recoil_per = widgets::no_recoil_percentage_slider->get_value() / 100.f;

		cheat::local_gun->recoil(game::vec4{ recoil_per, recoil_per, recoil_per, recoil_per });

	}

}; inline c_no_recoil* g_no_recoil = new c_no_recoil();

class c_no_spread : c_feature
{

private:

	float m_previous_spread{};
	item_gun_asset_t* m_previous_weapon{};

public:

	virtual void run()
	{

		auto enabled = widgets::no_spread_checkbox->get_value();
		auto needs_update = cheat::local_gun != m_previous_weapon;

		if ((enabled && !m_enabled) || needs_update)
		{

			if (!cheat::local_player.player || !cheat::local_gun)
				return;

			m_previous_spread = cheat::local_gun->base_spread();

			cheat::local_gun->base_spread(0.f);

			m_enabled = true;
			m_previous_weapon = cheat::local_gun;

		}
		else if (!enabled && m_enabled)
		{

			if (!cheat::local_player.player || !cheat::local_gun)
				return;

			cheat::local_gun->base_spread(m_previous_spread);

			m_enabled = false;

		}

	}

}; inline c_no_spread* g_no_spread = new c_no_spread();

class c_no_shake : c_feature
{

private:

	gun_shake_data_t m_previous_shake{};
	item_gun_asset_t* m_previous_weapon{};

public:

	virtual void run()
	{

		auto enabled = widgets::no_shake_checkbox->get_value();
		auto needs_update = cheat::local_gun != m_previous_weapon;

		if ((enabled && !m_enabled) || needs_update)
		{

			if (!cheat::local_player.player || !cheat::local_gun)
				return;

			m_previous_shake = cheat::local_gun->shake_data();

			cheat::local_gun->shake_data({ 0.f, 0.f, 0.f, 0.f, 0.f, 0.f });

			m_enabled = true;
			m_previous_weapon = cheat::local_gun;

		}
		else if (!enabled && m_enabled)
		{

			if (!cheat::local_player.player || !cheat::local_gun)
				return;

			cheat::local_gun->shake_data(m_previous_shake);

			m_enabled = false;

		}

	}

}; inline c_no_shake* g_no_shake = new c_no_shake();

class c_no_drop : c_feature
{

private:

	float m_previous_drop{};
	item_gun_asset_t* m_previous_weapon{};

public:

	virtual void run()
	{

		auto enabled = widgets::no_drop_checkbox->get_value();
		auto needs_update = cheat::local_gun != m_previous_weapon;

		if ((enabled && !m_enabled) || needs_update)
		{

			if (!cheat::local_player.player || !cheat::local_gun)
				return;

			m_previous_drop = cheat::local_gun->bullet_gravity_modifier();

			cheat::local_gun->bullet_gravity_modifier(0.f);

			m_enabled = true;
			m_previous_weapon = cheat::local_gun;

		}
		else if (!enabled && m_enabled)
		{

			if (!cheat::local_player.player || !cheat::local_gun)
				return;

			cheat::local_gun->bullet_gravity_modifier(m_previous_drop);

			m_enabled = false;

		}

	}

}; inline c_no_drop* g_no_drop = new c_no_drop();

class c_fast_bullet : c_feature
{

public:

	virtual void run()
	{

		auto enabled = widgets::fast_bullet_checkbox->get_value();

		if (enabled)
		{

			if (!cheat::local_player.player || !cheat::local_gun)
				return;

			cheat::local_gun->muzzle_velocity(600.f);

			m_enabled = true;

		}
		else
			m_enabled = false;

	}

}; inline c_fast_bullet* g_fast_bullet = new c_fast_bullet();

class c_instant_aim : c_feature
{

public:

	virtual void run()
	{

		auto enabled = widgets::instant_aim_checkbox->get_value();

		if (enabled)
		{

			if (!cheat::local_player.player || !cheat::local_gun)
				return;

			cheat::local_gun->aim_duration(0.f);

			m_enabled = true;

		}
		else
			m_enabled = false;

	}

}; inline c_instant_aim* g_instant_aim = new c_instant_aim();

class c_no_weapon_animations : c_feature
{

private:

public:

	virtual void run()
	{

		auto enabled = widgets::no_animations_checkbox->get_value();

		if (enabled)
		{

			if (!cheat::local_player.animator)
				return;

			cheat::local_player.animator->viewmodel_sway_multiplier(0.f);

			m_enabled = true;

		}
		else
			m_enabled = false;

	}

}; inline c_no_weapon_animations* g_no_weapon_animations = new c_no_weapon_animations();