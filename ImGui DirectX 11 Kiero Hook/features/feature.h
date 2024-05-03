#pragma once

#include "../sdk/game.h"
#include "../render.h"

class c_feature
{

	friend class c_aimbot;
	friend class c_player_esp;
	friend class c_item_esp;

	friend class c_no_sway;
	friend class c_no_recoil;
	friend class c_no_spread;
	friend class c_no_drop;
	friend class c_instant_aim;
	friend class c_no_shake;
	friend class c_fast_bullet;
	friend class c_no_weapon_animations;

private:

	bool m_enabled{};

public:

	virtual void run() = 0;

	bool get_enabled() const { return m_enabled; }

};

inline std::vector<std::string> g_friendly_players{};

namespace widgets
{

    c_combo* distance_units_combo = nullptr;

}

// vec3_t.distance returns meters
__forceinline float get_unit_multiplier()
{

    float ret = 3.281f;

    switch (widgets::distance_units_combo->get_value())
    {

    case 1:
        ret = 1.f;
        break;
    case 2:
        ret = 1.094;
        break;


    }

    return ret;

}

__forceinline auto get_unit_text()
{

    std::string ret = "ft";

    switch (widgets::distance_units_combo->get_value())
    {

    case 1:
        ret = "m";
        break;
    case 2:
        ret = "yd";
        break;


    }

    return ret;

}