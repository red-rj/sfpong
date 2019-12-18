#pragma once
#include <string_view>

#include "SFML/Window/Keyboard.hpp"
#include "boost/program_options/variables_map.hpp"
#include "common.h"

namespace red::pong
{

namespace player_id {
    enum : uint8_t {
        player_1, player_2
    };
}

    struct config_t
    {
        using Keyboard = sf::Keyboard;

        struct kb_controls { Keyboard::Key up, down, fast; };

        // ----
        kb_controls controls[2] = {
            { Keyboard::W, Keyboard::S, Keyboard::LShift },
            { Keyboard::Up, Keyboard::Down, Keyboard::RControl }
        };

        struct paddle_cfg {
            float base_speed, accel;
            sf::Vector2f size = { 25.f, 150.f };
        } paddle;

        struct ball_cfg {
            float base_speed, accel, max_speed;
            float radius;
        } ball;

        unsigned framerate;
    };

    boost::program_options::variables_map load_config_variables(std::string_view file);

    config_t load_config();
}