#pragma once

#include "feature.h"

namespace widgets
{

    c_checkbox* bullet_tracer_checkbox = nullptr;
    c_colorpicker* bullet_tracer_colorpicker = nullptr;
    c_slider_float* tracer_lifetime_slider = nullptr;
    c_slider_float* tracer_line_thickness = nullptr;
    c_combo* tracer_line_style_combo = nullptr;

    c_checkbox* item_esp_checkbox = nullptr;
    c_colorpicker* item_esp_color = nullptr;
    c_checkbox* item_distance_checkbox = nullptr;
    c_slider_float* item_max_distance_slider = nullptr;

    c_checkbox* zombie_esp_checkbox = nullptr;
    

}

struct tracer_info_t
{

    ULONG64 time;
    vec3_t origin;
    vec3_t pos;

};

class c_bullet_tracers : c_feature
{

private:

public:

    std::vector<tracer_info_t> tracer_list = std::vector<tracer_info_t>{};

	virtual void run()
	{

        if (!widgets::bullet_tracer_checkbox->get_value())
            return;

        for (int i = 0; i < tracer_list.size(); i++)
        {

            auto tracer = tracer_list.at(i);

            auto origin_screen = ImVec2{};
            auto pos_screen = ImVec2{};

            if (!game::to_screen(tracer.pos, pos_screen) || !game::to_screen(tracer.origin, origin_screen))
                continue;

            auto lifetime = GetTickCount64() - tracer.time;
            auto remaining_time = static_cast<int>((widgets::tracer_lifetime_slider->get_value() * 1000.f) - lifetime);
            auto float_time = std::clamp(remaining_time / 1000.f, 0.f, widgets::tracer_lifetime_slider->get_value());

            static auto scale_number = [](double number, double minInput, double maxInput, int minOutput, int maxOutput) -> auto {

                double inputRange = maxInput - minInput;
                double outputRange = maxOutput - minOutput;
                double scaledNumber = (number - minInput) / inputRange * outputRange + minOutput;

                return round(scaledNumber);
            };

            if (lifetime >= (widgets::tracer_lifetime_slider->get_value() * 1000.f))
            {

                tracer_list.erase(tracer_list.begin() + i);

                continue;

            }

            auto alpha = static_cast<int>(scale_number(float_time, 0.f, widgets::tracer_lifetime_slider->get_value(), 0, static_cast<int>(widgets::bullet_tracer_colorpicker->get_value().a * 255.f)));

            if (alpha <= 0)
            {

                tracer_list.erase(tracer_list.begin() + i);
                continue;

            }

            if (widgets::tracer_line_style_combo->get_value() == 0)
                render.line(origin_screen, pos_screen, widgets::bullet_tracer_colorpicker->get_value().u32(alpha), widgets::tracer_line_thickness->get_value());
            else if (widgets::tracer_line_style_combo->get_value() == 1)
                render.line_segment(origin_screen, pos_screen, widgets::bullet_tracer_colorpicker->get_value().u32(alpha), widgets::tracer_line_thickness->get_value(), (tracer.origin.distance(tracer.pos) * (10.f + (widgets::tracer_line_thickness->get_value() * 0.01f))));
            else
                render.line_segment(origin_screen, pos_screen, widgets::bullet_tracer_colorpicker->get_value().u32(alpha), widgets::tracer_line_thickness->get_value(), tracer.origin.distance(tracer.pos));

        }

	}

}; inline c_bullet_tracers* g_bullet_tracers = new c_bullet_tracers();

class c_item_esp : c_feature
{

private:

    std::string rarity_to_string(EItemRarity arg)
    {
        switch (arg)
        {
        case COMMON:
            return "common";
        case UNCOMMON:
            return "uncommon";
        case RARE:
            return "rare";
        case EPIC:
            return "epic";
        case LEGENDARY:
            return "legendary";
        case MYTHICAL:
            return "mythical";
        }
    }

    std::string type_to_string(EItemType arg)
    {
        switch (arg)
        {
        case HAT:
            return "hat";
        case PANTS:
            return "pants";
        case SHIRT:
            return "shirt";
        case MASK:
            return "mask";
        case BACKPACK:
            return "backpack";
        case VEST:
            return "vest";
        case GLASSES:
            return "glasses";
        case GUN:
            return "gun";
        case SIGHT:
            return "sight";
        case TACTICAL:
            return "tactical";
        case GRIP:
            return "grip";
        case BARREL:
            return "barrel";
        case MAGAZINE:
            return "magazine";
        case FOOD:
            return "food";
        case WATER:
            return "water";
        case MEDICAL:
            return "medical";
        case MELEE:
            return "melee";
        case FUEL:
            return "fuel";
        case TOOL:
            return "tool";
        case BARRICADE:
            return "barricade";
        case STORAGE:
            return "storage";
        case BEACON:
            return "beacon";
        case FARM:
            return "farm";
        case TRAP:
            return "trap";
        case STRUCTURE:
            return "structure";
        case SUPPLY:
            return "supply";
        case THROWABLE:
            return "throwable";
        case GROWER:
            return "grower";
        case OPTIC:
            return "optic";
        case REFILL:
            return "refill";
        case FISHER:
            return "fisher";
        case CLOUD:
            return "cloud";
        case MAP:
            return "map";
        case KEY:
            return "key";
        case BOX:
            return "box";
        case ARREST_START:
            return "arrest start";
        case ARREST_END:
            return "arrest end";
        case TANK:
            return "tank";
        case GENERATOR:
            return "generator";
        case DETONATOR:
            return "detonator";
        case CHARGE:
            return "charge";
        case LIBRARY:
            return "library";
        case FILTER:
            return "filter";
        case SENTRY:
            return "sentry";
        case VEHICLE_REPAIR_TOOL:
            return "vehicle repair tool";
        case TIRE:
            return "tire";
        case COMPASS:
            return "compass";
        case OIL_PUMP:
            return "oil pump";
        }
    }

public:

    virtual void run()
    {

        if (!widgets::item_esp_checkbox->get_value())
            return;

        std::sort(cheat::items.begin(), cheat::items.end(), [](const cached_item_t& a, const cached_item_t& b) -> bool {

            auto a_origin = a.position;
            auto b_origin = b.position;

            auto a_distance = game::camera.position.distance(a_origin);
            auto b_distance = game::camera.position.distance(b_origin);

            return a_distance > b_distance;

        });

        for (auto& item : cheat::items)
        {

            auto screen_pos = ImVec2();
            auto item_pos = item.position;

            if (!game::to_screen(item_pos, screen_pos))
                continue;

            auto text = std::string("");
            auto bottom_text = std::string("");

            //if (item.count > 1)
            //    text += std::to_string(item.count) + "x ";

            text += item.name;
                
            auto distance = floor(game::camera.position.distance(item_pos)) * get_unit_multiplier();

            constexpr float min_distance = 0.f;
            const float max_distance = widgets::item_max_distance_slider->get_value() + 1.f;

            auto normalized_distance = (distance - min_distance) / (max_distance - min_distance);
            auto alpha = 1.f - normalized_distance;

            if (alpha <= 0.f)
                continue;

            if (widgets::item_distance_checkbox->get_value())
                bottom_text = std::to_string(static_cast<int>(distance)) + get_unit_text();

            ImGui::PushFont(framework::g_scaled_fonts.at(3).font);
            auto text_sz = ImGui::CalcTextSize(text.c_str());
            ImGui::PopFont();

            ImGui::PushFont(framework::g_scaled_fonts.at(2).font);
            auto bottom_text_sz = ImGui::CalcTextSize(bottom_text.c_str());
            ImGui::PopFont();

            render.text_shadowed({ screen_pos - (text_sz / 2.f) }, text, widgets::item_esp_color->get_value().u32(widgets::item_esp_color->get_value().a * alpha), &framework::g_scaled_fonts.at(3));
            render.text_shadowed({ (screen_pos - (bottom_text_sz / 2.f)) + ImVec2{ 0.f, bottom_text_sz.y + 2.f } }, bottom_text, widgets::item_esp_color->get_value().u32((widgets::item_esp_color->get_value().a * 0.95f) * alpha), &framework::g_scaled_fonts.at(2));

        }

    }

}; inline c_item_esp* g_item_esp = new c_item_esp();

class c_zombie_esp : c_feature
{

private:

public:

    virtual void run()
    {

        for (auto& zombie : cheat::zombies)
        {

            auto screen_pos = ImVec2();
            auto zombie_pos = zombie.pos;

            auto head_screen = ImVec2();
            auto head_pos = zombie_pos + vec3_t{ 0.f, 2.f, 0.f };

            if (!game::to_screen(zombie_pos, screen_pos) || !game::to_screen(head_pos, head_screen))
                continue;

            zombie.bounds.calculate(screen_pos, head_screen);

            render.text_shadowed(head_screen, "zombie", IM_COL32_WHITE, &framework::g_scaled_fonts.at(3));

        }

    }

}; inline c_zombie_esp* g_zombie_esp = new c_zombie_esp();