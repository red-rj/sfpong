#pragma once
#include <string_view>
#include <filesystem>

#include <SFML/Window/Keyboard.hpp>


namespace red::pong
{

namespace player_id {
    enum : uint8_t {
        player_1, player_2
    };
}

    struct kb_controls { sf::Keyboard::Key up, down, fast; };

    struct config_t
    {
        using Keyboard = sf::Keyboard;

        void load(std::filesystem::path filepath);
        void save(std::filesystem::path filepath);

        // ----
        kb_controls controls[2];

        struct paddle_cfg {
            float base_speed, accel;
            sf::Vector2f size;
        } paddle;

        struct ball_cfg {
            float base_speed, accel, max_speed;
            float radius;
        } ball;

        unsigned framerate;
    };


    //auto load_settings(std::filesystem::path filepath)->boost::property_tree::ptree;

    //void save_settings(boost::property_tree::ptree const& cfg, std::filesystem::path filepath);



}