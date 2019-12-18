#pragma once

#include "SFML/Window/Keyboard.hpp"
#include "common.h"

namespace red::pong
{
    struct config_t
    {
        using Keyboard = sf::Keyboard;

        struct kb_controls { Keyboard::Key up, down, fast; };
        enum player_id : short { player_1, player_2 };

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
    };


    config_t load_config();
}