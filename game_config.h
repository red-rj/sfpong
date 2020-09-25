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
    struct movement
    {
        float speed;
        float max_speed;
        float acceleration;
    };

    struct paddle_cfg {
        movement move;
        size2d size;
    };

    struct ball_cfg {
        movement move;
        float radius;
    };


    using cfgtree = boost::property_tree::ptree;

    void applyConfig(const cfgtree& tree);
    cfgtree getGameConfig();

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

        P1_JSDEADZONE = "player1.joystick_deadzone",
        P2_JSDEADZONE = "player2.joystick_deadzone"
        ;
}

    void overrideGuts(const cfgtree& guts);
}