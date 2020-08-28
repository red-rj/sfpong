#pragma once
#include <string_view>
#include <filesystem>
#include <array>

#include <SFML/Window/Keyboard.hpp>
#include "SFML/System/Vector2.hpp"

namespace pong
{

    struct paddle_cfg {
        float base_speed = 10, accel = 0.1f;
        sf::Vector2f size = { 25.f, 150.f };
    };

    struct ball_cfg {
        float base_speed = 5, accel = 0.1f, max_speed = 20;
        float radius = 10;
    };
    
    struct kb_keys { sf::Keyboard::Key up, down, fast; };
    
    struct config_t
    {
        using Keyboard = sf::Keyboard;

        // ----
        std::array<kb_keys, 2> controls = {{
            {Keyboard::W, Keyboard::S, Keyboard::LShift},
            {Keyboard::Up, Keyboard::Down, Keyboard::RControl}
        }};

        paddle_cfg paddle;
        ball_cfg ball;

        unsigned framerate=60;
        
        bool operator== (const config_t& rhs) const noexcept;
    };

    config_t load_config(std::filesystem::path cfgfile);
    bool save_config(config_t const& cfg, std::filesystem::path cfgfile);


    // operators
    using std::rel_ops::operator!=;

    constexpr bool operator==(const kb_keys& lhs, const kb_keys& rhs)
    {
        return std::tie(lhs.up, lhs.down, lhs.fast) == std::tie(rhs.up, rhs.down, rhs.fast);
    }

    constexpr bool operator== (paddle_cfg const& lhs, paddle_cfg const& rhs)
    {
        return std::tie(lhs.accel, lhs.base_speed, lhs.size) ==
               std::tie(rhs.accel, rhs.base_speed, rhs.size);
    }
    constexpr bool operator== (ball_cfg const& lhs, ball_cfg const& rhs)
    {
        return std::tie(lhs.accel, lhs.base_speed, lhs.max_speed, lhs.radius) ==
               std::tie(rhs.accel, rhs.base_speed, rhs.max_speed, rhs.radius);
    }

    // config keys
    inline constexpr auto
        CFG_P1_UP         = "player1.up",
        CFG_P1_DOWN       = "player1.down",
        CFG_P1_FAST       = "player1.fast",
        CFG_P2_UP         = "player2.up",
        CFG_P2_DOWN       = "player2.down",
        CFG_P2_FAST       = "player2.fast",
        CFG_PADDLE_SPEED  = "game.paddle_base_speed",
        CFG_PADDLE_ACCEL  = "game.paddle_accel",
        CFG_PADDLE_SIZE_X = "game.paddle_size_x",
        CFG_PADDLE_SIZE_Y = "game.paddle_size_y",
        CFG_BALL_SPEED    = "game.ball_base_speed",
        CFG_BALL_MAXSPEED = "game.ball_max_speed",
        CFG_BALL_ACCEL    = "game.ball_accel",
        CFG_BALL_RADIUS   = "game.ball_radius",
        CFG_FRAMERATE     = "game.framerate"
    ;



}