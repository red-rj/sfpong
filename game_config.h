#pragma once
#include "input.h"
#include <string_view>
#include <filesystem>
#include <tuple>
#include <array>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/System/Vector2.hpp>
#include <boost/property_tree/ptree_fwd.hpp>


namespace pong
{
    struct paddle_cfg {
        float base_speed, accel;
        sf::Vector2f size;
    };

    struct ball_cfg {
        float base_speed, accel, max_speed;
        float radius;
    };

    struct player_cfg
    {
        keyboard_ctrls keyboard_controls;
        int joystickId;

        bool operator== (const player_cfg& rhs) const noexcept
        {
            return keyboard_controls == rhs.keyboard_controls
                && joystickId == rhs.joystickId;
        }
    };

    struct config_t
    {
        std::array<player_cfg, 2> player;

        auto const& get_player_cfg(Player pl) const noexcept {
            return player[int(pl)];
        }

        paddle_cfg paddle;
        ball_cfg ball;
        unsigned framerate;
        
        bool operator== (const config_t& rhs) const noexcept;
    };

    struct game_guts
    {
        struct movable { float speed, accel, max_speed; };

        struct : movable
        {
            sf::Vector2f size;

        } paddle;

        struct : movable
        {
            float radius;

        } ball;
    };

    using cfgtree = boost::property_tree::iptree;

    config_t load_config(std::filesystem::path cfgfile);
    bool save_config(config_t const& cfg, std::filesystem::path cfgfile);


    // operators
    using std::rel_ops::operator!=;

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