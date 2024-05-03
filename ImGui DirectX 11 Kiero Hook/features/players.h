#pragma once

#include "feature.h"

namespace widgets
{

    c_keybind* player_esp_keybind = nullptr; c_binding player_esp_key { "player esp", VK_XBUTTON1, keybind_interaction_type::ALWAYS };
    c_combo* box_combo = nullptr; c_colorpicker* box_color = nullptr;
    c_checkbox* name_checkbox = nullptr; c_colorpicker* name_color = nullptr;
    c_checkbox* weapon_checkbox = nullptr; c_colorpicker* weapon_color = nullptr;
    c_checkbox* trail_checkbox = nullptr;
    c_checkbox* trail_esp_checkbox = nullptr;
    c_colorpicker* trail_colorpicker = nullptr;
    c_slider_int* trail_length_slider = nullptr;
    c_checkbox* snaplines_checkbox = nullptr; c_colorpicker* snaplines_color = nullptr;
    c_combo* snaplines_start_combo = nullptr; c_combo* snaplines_end_combo = nullptr;
    c_slider_float* esp_max_distance_slider = nullptr;
    c_checkbox* distance_checkbox = nullptr;
    c_colorpicker* distance_colorpicker = nullptr;
    //c_checkbox* out_of_fov_arrows_checkbox = nullptr;
    //c_slider_int* fov_arrow_distance_slider = nullptr;
    //c_slider_int* fov_arrow_size_slider = nullptr;
    //c_colorpicker* fov_arrow_colorpicker = nullptr;

    c_checkbox* friend_override_checkbox = nullptr;
    c_keybind* friend_override_add_keybind = nullptr; c_binding friend_override_add_key{ "friend override add", VK_XBUTTON1, keybind_interaction_type::HELD };
    c_keybind* friend_override_remove_keybind = nullptr; c_binding friend_override_remove_key{ "friend override remove", VK_XBUTTON1, keybind_interaction_type::HELD };
    c_colorpicker* friend_override_color = nullptr;

    c_checkbox* only_visible_checkbox = nullptr;
    c_checkbox* visible_color_override_checkbox = nullptr;
    c_colorpicker* visible_override_colorpicker = nullptr;

}

class c_player_esp
{

private:

public:

    void run(std::vector<player_snapshot_t>& snapshots)
    {

        if (!widgets::player_esp_key.get_enabled())
            return;

        std::vector<cached_player_t> sorted_players{};
        std::vector<player_snapshot_t> sorted_snapshots{};

        std::sort(cheat::players.begin(), cheat::players.end(), [](const cached_player_t& a, const cached_player_t& b) -> bool {
                
            auto a_origin = a.origin;
            auto b_origin = b.origin;

            auto a_distance = game::camera.position.distance(a_origin);
            auto b_distance = game::camera.position.distance(b_origin);

            return a_distance > b_distance;

        });

        for (auto& player : cheat::players)
        {

            if (!player.playerlife || !player.movement || !player.transform)
                continue;

            auto vis_data = player.get_vis_data();

            if (!vis_data.ptr || (!vis_data.visible && widgets::only_visible_checkbox->get_value()))
                continue;

            if (!player.playerlife || player.playerlife->dead())
                continue;

            auto player_height = player.movement->get_height();

            vec3_t origin = player.transform->position();
            vec3_t head_pos = origin + vec3_t{ 0.f, player_height, 0.f };

            float distance = floor(game::camera.position.distance(head_pos)) * get_unit_multiplier();

            constexpr float min_distance = 0.f;
            const float max_distance = widgets::esp_max_distance_slider->get_value() * 1.55f;

            auto normalized_distance = (distance - min_distance) / (max_distance - min_distance);
            auto alpha_factor = (1.f - normalized_distance);

            if (alpha_factor <= 0.f)
                continue;

            auto base_screen = ImVec2{};
            auto head_screen = ImVec2{};

            player_snapshot_t* player_snapshot = nullptr;
            auto found_player = false;

            for (int i = 0; i < snapshots.size(); i++)
            {

                auto& snapshot = snapshots.at(i);

                if (!snapshot.player.player)
                {

                    snapshots.erase(snapshots.begin() + i);
                    continue;

                }

                if (snapshot.player.player == player.player)
                {

                    found_player = true;
                    player_snapshot = &snapshot;

                }

            }

            if (!found_player)
            {

                snapshots.emplace_back<player_snapshot_t>({ player, {} });
                player_snapshot = &snapshots.back();

            }

            constexpr auto snapshot_interval = 15;
            constexpr auto max_snapshots = 128;

            if (player_snapshot->positions.size() < snapshot_interval || player_snapshot->positions.back().time < GetTickCount64())
            {

                player_snapshot->positions.emplace_back<snapshot_info_t>({ origin, head_pos, GetTickCount64() + snapshot_interval });

                if (player_snapshot->positions.size() > max_snapshots)
                    player_snapshot->positions.erase(player_snapshot->positions.begin());

            }

            bool invalid_player = false;

            if (player_snapshot->positions.size() > (max_snapshots - 1))
            {

                std::vector<snapshot_info_t> snapshot_list{};

                for (int i = 4; i < (max_snapshots - 2); i++)
                    snapshot_list.push_back(player_snapshot->positions.at(i));

                int same_count = 0;
                auto last_snap = snapshot_info_t{};

                for (auto snap : snapshot_list)
                {

                    if (last_snap.time == NULL)
                    {

                        last_snap = snap;
                        continue;

                    }

                    if (last_snap.position == snap.position)
                        same_count++;

                    if (same_count > (max_snapshots - 16))
                        player_snapshot->dormant = true;
                    else
                        player_snapshot->dormant = false;

                }

            }

            player.origin = origin;

            if (!game::to_screen(origin, base_screen) || !game::to_screen(head_pos, head_screen))
            {

                //constexpr auto get_point_on_ellipse = [](const ImVec2& radius, const ImVec2& center, const float degree) -> ImVec2
                //{
                //    constexpr auto pi_over_180 = 0.017453292519943295769236907684886;
                //    return { radius.x * cosf(degree * static_cast<float>(pi_over_180)) + center.x,
                //        radius.y * sinf(degree * static_cast<float>(pi_over_180)) + center.y };
                //};

                //constexpr auto rotate_point = [](const ImVec2& point, const ImVec2& center, float degree) -> ImVec2
                //{
                //    degree = DEG2RAD(degree);

                //    const auto cos = cosf(degree);
                //    const auto sin = sinf(degree);

                //    static auto ret = ImVec2();
                //    ret.x = cos * (point.x - center.x) - sin * (point.y - center.y);
                //    ret.y = sin * (point.x - center.x) + cos * (point.y - center.y);

                //    ret += center;

                //    static ImVec2 point_ret;
                //    point_ret.x = ret.x;
                //    point_ret.y = ret.y;

                //    return point_ret;
                //};

                //if (widgets::out_of_fov_arrows_checkbox->get_value())
                //{

                //    // Calculate the relative angle between the player's view and the off-screen player
                //    const auto view_angles = cheat::local_player.playerlook->viewangles();
                //    const auto angle = game::calcangle(cheat::local_player.transform->position(), player.origin);
                //    const auto screen_angle = view_angles.y - angle.y - 90.f;

                //    // Calculate the initial screen position based on the relative angle
                //    const auto dimensions = ImGui::GetIO().DisplaySize;
                //    const auto screen_size = ImVec2(dimensions.x, dimensions.y);
                //    const auto screen_center = ImVec2((screen_size.x) / 2.f, (screen_size.y) / 2.f);
                //    const auto size = widgets::fov_arrow_size_slider->get_value();
                //    const auto offset = (screen_center.x - size) * (widgets::fov_arrow_distance_slider->get_value() * .01f);
                //    const auto screen_pos = get_point_on_ellipse({ offset, offset * (screen_size.y / screen_size.x) }, { screen_center.x, screen_center.y }, screen_angle);

                //    // Draw arrows with fixed position and rotation
                //    static auto triangle = std::array<ImVec2, 3>{ };
                //    triangle.at(1) = { screen_pos.x, screen_pos.y - size };
                //    triangle.at(0) = { screen_pos.x - size * .5f, screen_pos.y };
                //    triangle.at(2) = { screen_pos.x + size * .5f, screen_pos.y };

                //    static ImVec2 one, two, three;
                //    one = rotate_point(triangle.at(0), screen_pos, screen_angle + 90.f);
                //    two = rotate_point(triangle.at(1), screen_pos, screen_angle + 90.f);
                //    three = rotate_point(triangle.at(2), screen_pos, screen_angle + 90.f);

                //    ImGui::GetBackgroundDrawList()->AddTriangleFilled(one, two, three, widgets::fov_arrow_colorpicker->get_value().u32());

                //}

                continue;

            }

            player.bounds.calculate(base_screen, head_screen);

            if (std::isnan(player.bounds.x) || std::isnan(player.bounds.y))
                continue;

            if (widgets::friend_override_checkbox->get_value() && !player.name.empty() && !invalid_player)
                sorted_players.push_back(player);

            bool should_override_color = false;

            c_color override_color = c_color();

            if (widgets::visible_color_override_checkbox->get_value() && player.get_vis_data().visible)
            {

                should_override_color = true;
                override_color = widgets::visible_override_colorpicker->get_value();

            }

            if (widgets::aimbot_target_color_override_checkbox->get_value() && widgets::aimbot_keybind->get_bind()->get_enabled() && g_aimbot->m_has_target)
            {

                auto last_target = g_aimbot->get_last_target();

                if (last_target.player.player == player.player)
                {

                    should_override_color = true;
                    override_color = widgets::aimbot_target_color_override_color->get_value();

                }

            }

            if (widgets::friend_override_checkbox->get_value() && !player.name.empty())
            {

                auto ite = std::find(g_friendly_players.begin(), g_friendly_players.end(), player.name);

                if (ite != g_friendly_players.end())
                {

                    should_override_color = true;
                    override_color = widgets::friend_override_color->get_value();

                }

            }

            if (widgets::snaplines_checkbox->get_value())
            {

                static auto screen_size = ImGui::GetIO().DisplaySize;
                auto screen_center = screen_size * 0.5f;

                auto start_pos = widgets::snaplines_start_combo->get_value();
                auto end_pos = widgets::snaplines_end_combo->get_value();

                auto start_screen = start_pos == 0 ? ImVec2(screen_center.x, 0.f) : start_pos == 1 ? screen_center : start_pos == 2 ? ImVec2(screen_center.x, screen_size.y) : screen_center;
                auto end_screen = end_pos == 0 ? ImVec2(player.bounds.x + (player.bounds.w * 0.5f), player.bounds.y + player.bounds.h) : end_pos == 1 ? head_screen : end_pos == 2 ? ImVec2(player.bounds.x + (player.bounds.w * 0.5f), player.bounds.y) : head_screen;

                auto snapline_alpha = widgets::snaplines_color->get_value().a * alpha_factor;

                render.line(start_screen, end_screen, should_override_color ? override_color.u32(snapline_alpha) : widgets::snaplines_color->get_value().u32(snapline_alpha), 2.f);

            }

            if (widgets::trail_checkbox->get_value() && player_snapshot->positions.size() >= widgets::trail_length_slider->get_value())
            {

                vec3_t last_screen_point{};

                auto first_data = player_snapshot->positions.front();
                auto last_data = player_snapshot->positions.back();

                auto trail_distance = first_data.position.distance(last_data.position);

                if (trail_distance < 76.f)
                {

                    for (int i = player_snapshot->positions.size() - widgets::trail_length_slider->get_value(); i < player_snapshot->positions.size(); i++)
                    {

                        auto info = player_snapshot->positions.at(i);
                        auto snap_pos = ImVec2{};

                        if (game::to_screen(info.position, snap_pos))
                        {

                            if (i == (player_snapshot->positions.size() - widgets::trail_length_slider->get_value()))
                            {

                                last_screen_point = info.position;
                                continue;

                            }

                            auto last_screen_pos = ImVec2{};

                            if (game::to_screen(last_screen_point, last_screen_pos))
                            {

                                float alpha = static_cast<float>(i - (player_snapshot->positions.size() - widgets::trail_length_slider->get_value())) / (widgets::trail_length_slider->get_value() - 1);
                                alpha = widgets::trail_colorpicker->get_value().a * (alpha * 1000.f);

                                alpha = max(0.0f, min(alpha, 255.0f));

                                auto factored_alpha = alpha * alpha_factor;

                                render.line(last_screen_pos, snap_pos, should_override_color ? override_color.u32(static_cast<int>(factored_alpha)) : widgets::trail_colorpicker->get_value().u32(static_cast<int>(factored_alpha)), 3.f);

                                last_screen_point = info.position;

                            }

                        }

                    }

                }

            }

            float box_alpha = widgets::box_color->get_value().a * alpha_factor;

            if (widgets::box_combo->get_value() == 1)
                render.outlined_rect({ player.bounds.x, player.bounds.y }, { player.bounds.w, player.bounds.h }, should_override_color ? override_color.u32(box_alpha) : widgets::box_color->get_value().u32(box_alpha));
            else if (widgets::box_combo->get_value() == 2)
                render.cornered_rect({ player.bounds.x, player.bounds.y }, { player.bounds.w, player.bounds.h }, should_override_color ? override_color.u32(box_alpha) : widgets::box_color->get_value().u32(box_alpha));
            else if (widgets::box_combo->get_value() == 3)
            {

                auto origin = player.origin;
                auto extents = vec3_t{ 0.45f, 0.f, 0.45f };

                auto v000 = origin - extents; extents.y = player.movement->get_height();
                auto v111 = origin + extents;

                const auto ProjectWorldToScreen = [](vec3_t pos) -> ImVec2 {
                    auto screen = ImVec2();
                    auto ret = game::to_screen(pos, screen);

                    if (!ret)
                        return { -1.f, -1.f };

                    return screen;

                };

                auto w1 = ProjectWorldToScreen(vec3_t(v000.x, v000.y, v000.z));
                auto w2 = ProjectWorldToScreen(vec3_t(v111.x, v111.y, v111.z));

                // top box
                auto v010 = vec3_t(v000.x, v111.y, v000.z);
                auto v110 = vec3_t(v111.x, v111.y, v000.z);
                auto v011 = vec3_t(v000.x, v111.y, v111.z);

                // bottom box
                auto v101 = vec3_t(v111.x, v000.y, v111.z);
                auto v100 = vec3_t(v111.x, v000.y, v000.z);
                auto v001 = vec3_t(v000.x, v000.y, v111.z);

                auto s1 = ProjectWorldToScreen(v010);
                auto s2 = ProjectWorldToScreen(v110);
                auto s3 = ProjectWorldToScreen(v011);
                auto s4 = ProjectWorldToScreen(v101);
                auto s5 = ProjectWorldToScreen(v100);
                auto s6 = ProjectWorldToScreen(v001);

                auto color = should_override_color ? override_color.u32(box_alpha) : widgets::box_color->get_value().u32(box_alpha);

                if (w1.valid() && w2.valid() && s1.valid() && s2.valid() && s3.valid() && s4.valid() && s5.valid() && s6.valid()) 
                {

                    render.line(w1, s6, color);
                    render.line(w1, s5, color);
                    render.line(s4, s5, color);
                    render.line(s4, s6, color);

                    render.line(s1, s3, color);
                    render.line(s1, s2, color);
                    render.line(w2, s2, color);
                    render.line(w2, s3, color);

                    render.line(s6, s3, color);
                    render.line(w1, s1, color);
                    render.line(s4, w2, color);
                    render.line(s5, s2, color);

                }

            }

            std::string top_text = "";

            if (widgets::name_checkbox->get_value())
                top_text += player.name;

            int font_scale = 0;

            if (top_text.size() > 0)
            {

                ImGui::PushFont(framework::g_scaled_fonts.at(3).font);
                auto top_text_sz = ImGui::CalcTextSize(top_text.c_str());
                ImGui::PopFont();

                auto text_padding = 14.f;

                if (widgets::box_combo->get_value() == 2)
                    text_padding += 2.f;

                auto name_alpha = widgets::name_color->get_value().a * alpha_factor;

                render.text_shadowed({ player.bounds.x + (player.bounds.w - top_text_sz.x) / 2.f, (player.bounds.y + player.bounds.h) - text_padding }, top_text, should_override_color ? override_color.u32(name_alpha) : widgets::name_color->get_value().u32(name_alpha), &framework::g_scaled_fonts.at(3));

            }

            if (widgets::weapon_checkbox->get_value())
            {

                ImGui::PushFont(framework::g_scaled_fonts.at(2).font);
                auto bot_text_sz = ImGui::CalcTextSize(player.weapon_name.c_str());
                ImGui::PopFont();

                auto weapon_alpha = widgets::weapon_color->get_value().a * alpha_factor;

                render.text_shadowed({ player.bounds.x + (player.bounds.w - bot_text_sz.x) / 2.f, player.bounds.y + 2.f }, player.weapon_name, should_override_color ? override_color.u32(weapon_alpha) : widgets::weapon_color->get_value().u32(weapon_alpha), &framework::g_scaled_fonts.at(2));

            }

            if (widgets::distance_checkbox->get_value())
            {

                std::string distance_text = std::string("");

                distance_text += std::to_string(static_cast<int>(distance)) + get_unit_text();

                ImGui::PushFont(framework::g_scaled_fonts.at(2).font);
                auto bot_text_sz = ImGui::CalcTextSize(distance_text.c_str());
                ImGui::PopFont();

                auto distance_alpha = widgets::distance_colorpicker->get_value().a * alpha_factor;

                render.text_shadowed({ player.bounds.x + (player.bounds.w - bot_text_sz.x) / 2.f, widgets::weapon_checkbox->get_value() ? (player.bounds.y + bot_text_sz.y + 2.f) : player.bounds.y + 2.f}, distance_text.c_str(), should_override_color ? override_color.u32(distance_alpha) : widgets::distance_colorpicker->get_value().u32(distance_alpha), & framework::g_scaled_fonts.at(2));

            }

            if (widgets::trail_esp_checkbox->get_value())
            {

                for (const auto& snapshot : snapshots)
                {

                    if (snapshot.player.player == player.player)
                    {

                        for (int i = player_snapshot->positions.size() - widgets::trail_length_slider->get_value(); i < player_snapshot->positions.size(); i++)
                        {

                            auto position_info = snapshot.positions.at(i);

                            if (!game::to_screen(position_info.position, base_screen) || !game::to_screen(position_info.head_pos, head_screen))
                                continue;

                            player.bounds.calculate(base_screen, head_screen);

                            if (std::isnan(player.bounds.x) || std::isnan(player.bounds.y))
                                continue;

                            float alpha = static_cast<float>(i - (player_snapshot->positions.size() - widgets::trail_length_slider->get_value())) / (widgets::trail_length_slider->get_value() - 1);
                            alpha = widgets::trail_colorpicker->get_value().a * (alpha * 1000.f);

                            alpha = max(0.0f, min(alpha, 255.0f));

                            auto box_alpha = static_cast<int>(alpha * alpha_factor);

                            if (widgets::box_combo->get_value() == 1)
                                render.outlined_rect({ player.bounds.x, player.bounds.y }, { player.bounds.w, player.bounds.h }, should_override_color ? override_color.u32(box_alpha) : widgets::box_color->get_value().u32(box_alpha));
                            else if (widgets::box_combo->get_value() == 2)
                                render.cornered_rect({ player.bounds.x, player.bounds.y }, { player.bounds.w, player.bounds.h }, should_override_color ? override_color.u32(box_alpha) : widgets::box_color->get_value().u32(box_alpha));

                            std::string top_text = "";

                            if (widgets::name_checkbox->get_value())
                                top_text += player.name;

                            auto new_dst = static_cast<int>((position_info.position.distance(game::camera.position) * get_unit_multiplier()));

                            int font_scale = 0;

                            if (top_text.size() > 0)
                            {

                                ImGui::PushFont(framework::g_scaled_fonts.at(2).font);
                                auto top_text_sz = ImGui::CalcTextSize(top_text.c_str());
                                ImGui::PopFont();

                                auto text_padding = 14.f;

                                if (widgets::box_combo->get_value() == 2)
                                    text_padding += 2.f;

                                auto name_alpha = widgets::name_color->get_value().a * alpha_factor;

                                render.text_shadowed({ player.bounds.x + (player.bounds.w - top_text_sz.x) / 2.f, (player.bounds.y + player.bounds.h) - text_padding }, top_text, should_override_color ? override_color.u32(name_alpha) : widgets::name_color->get_value().u32(name_alpha), &framework::g_scaled_fonts.at(2));

                            }

                            auto bottom_offset = 0.f;
                            
                            if (widgets::box_combo->get_value() == 2)
                                bottom_offset = 10.f;

                            if (widgets::weapon_checkbox->get_value())
                            {

                                ImGui::PushFont(framework::g_scaled_fonts.at(3).font);
                                auto bot_text_sz = ImGui::CalcTextSize(player.weapon_name.c_str());
                                ImGui::PopFont();

                                auto weapon_alpha = widgets::weapon_color->get_value().a * alpha_factor;

                                render.text_shadowed({ player.bounds.x + (player.bounds.w - bot_text_sz.x) / 2.f, player.bounds.y + (bottom_offset + 2.f) }, player.weapon_name, should_override_color ? override_color.u32(weapon_alpha) : widgets::weapon_color->get_value().u32(weapon_alpha), &framework::g_scaled_fonts.at(3));

                            }

                            if (widgets::distance_checkbox->get_value())
                            {

                                std::string distance_text = std::string("");

                                distance_text += std::to_string(static_cast<int>(distance)) + get_unit_text();

                                ImGui::PushFont(framework::g_scaled_fonts.at(2).font);
                                auto bot_text_sz = ImGui::CalcTextSize(distance_text.c_str());
                                ImGui::PopFont();

                                auto distance_alpha = widgets::distance_colorpicker->get_value().a * alpha_factor;

                                render.text_shadowed({ player.bounds.x + (player.bounds.w - bot_text_sz.x) / 2.f, widgets::weapon_checkbox->get_value() ? (player.bounds.y + (bot_text_sz.y + 2.f + bottom_offset)) : player.bounds.y + (2.f + bottom_offset) }, distance_text.c_str(), should_override_color ? override_color.u32(distance_alpha) : widgets::distance_colorpicker->get_value().u32(distance_alpha), &framework::g_scaled_fonts.at(2));

                            }

                            if (widgets::snaplines_checkbox->get_value())
                            {

                                static auto screen_size = ImGui::GetIO().DisplaySize;
                                auto screen_center = screen_size * 0.5f;

                                auto start_pos = widgets::snaplines_start_combo->get_value();
                                auto end_pos = widgets::snaplines_end_combo->get_value();

                                auto start_screen = start_pos == 0 ? ImVec2(screen_center.x, 0.f) : start_pos == 1 ? screen_center : start_pos == 2 ? ImVec2(screen_center.x, screen_size.y) : screen_center;
                                auto end_screen = end_pos == 0 ? ImVec2(player.bounds.x + (player.bounds.w * 0.5f), player.bounds.y + player.bounds.h) : end_pos == 1 ? head_screen : end_pos == 2 ? ImVec2(player.bounds.x + (player.bounds.w * 0.5f), player.bounds.y) : head_screen;

                                auto snapline_alpha = widgets::snaplines_color->get_value().a * alpha_factor;

                                render.line(start_screen, end_screen, should_override_color ? override_color.u32(snapline_alpha) : widgets::snaplines_color->get_value().u32(snapline_alpha), 2.f);

                            }

                        }

                    }

                }

            }

        }

        if (widgets::friend_override_checkbox->get_value())
        {

            for (const auto& player : sorted_players)
                for (const auto& snapshot : snapshots)
                    if (snapshot.player.player != nullptr && player.player != nullptr && snapshot.player.player == player.player)
                        sorted_snapshots.push_back(snapshot);

            if (sorted_snapshots.size() > 1 && sorted_players.size() > 1)
            {

                auto local_pos = game::camera.position;

                static auto screen_center = ImVec2(ImGui::GetIO().DisplaySize.x / 2.f, ImGui::GetIO().DisplaySize.y / 2.f);

                auto distance_sort = [local_pos](player_snapshot_t t1, player_snapshot_t t2) -> bool
                {

                    auto t1_dst = t1.positions.front().position.distance(local_pos);
                    auto t2_dst = t2.positions.front().position.distance(local_pos);

                    return t1_dst < t2_dst;

                };

                auto fov_sort = [](player_snapshot_t t1, player_snapshot_t t2) -> bool
                {

                    auto t1_head_screen = ImVec2();
                    auto t2_head_screen = ImVec2();

                    game::to_screen(t1.positions.front().head_pos, t1_head_screen);
                    game::to_screen(t2.positions.front().head_pos, t2_head_screen);

                    auto t1_fov = t1_head_screen.distance(screen_center);
                    auto t2_fov = t2_head_screen.distance(screen_center);

                    return t1_fov < t2_fov;

                };

                std::sort(sorted_snapshots.begin(), sorted_snapshots.end(), distance_sort);

                std::sort(sorted_snapshots.begin(), sorted_snapshots.end(), fov_sort);

                auto final_player = cached_player_t{};
                auto& final_snapshot = sorted_snapshots.front();

                for (const auto& player : sorted_players)
                    if (final_snapshot.player.player == player.player)
                        final_player = player;

                bool is_friend = false;

                for (const auto& name : g_friendly_players)
                    if (name == final_player.name)
                        is_friend = true;

                if (widgets::friend_override_add_keybind->get_bind()->get_enabled() && !is_friend)
                    g_friendly_players.push_back(final_player.name);

                if (widgets::friend_override_remove_keybind->get_bind()->get_enabled())
                {

                    for (int i = 0; i < g_friendly_players.size(); i++)
                    {

                        auto name = g_friendly_players.at(i);

                        if (name == final_player.name)
                            g_friendly_players.erase(g_friendly_players.begin() + i);

                    }

                }

            }

        }

    }

}; inline c_player_esp* g_player_esp = new c_player_esp();