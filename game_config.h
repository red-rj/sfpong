#pragma once
#include "input.h"
#include <string_view>
#include <filesystem>
#include <tuple>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/System/Vector2.hpp>
#include <boost/property_tree/ptree_fwd.hpp>


namespace pong
{
    struct paddle_cfg {
        float base_speed, accel;
        size2d size;
    };

    struct ball_cfg {
        float base_speed, accel, max_speed;
        float radius;
    };



    using cfgtree = boost::property_tree::ptree;

    void applyConfig(const cfgtree& tree);
    cfgtree getGameConfig();


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
namespace ckey
{
    // game.cfg
    inline constexpr auto
        P1_UP = "player1.up",
        P1_DOWN = "player1.down",
        P1_FAST = "player1.fast",
        P1_JOYSTICK = "player1.joystick",
        P2_UP = "player2.up",
        P2_DOWN = "player2.down",
        P2_FAST = "player2.fast",
        P2_JOYSTICK = "player2.joystick",
        GUTSFILE = "game.guts"
        ;
}

    void overrideGuts(const cfgtree& guts);
}