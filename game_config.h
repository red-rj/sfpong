#pragma once
#include <boost/property_tree/ptree_fwd.hpp>
#include <SFML/Window/Keyboard.hpp>

#include "common.h"

namespace pong
{
    using cfgtree = boost::property_tree::ptree;

    void set_user_config(const cfgtree& tree);
    cfgtree get_user_config();

    void overrideGuts(const cfgtree& tree);
    cfgtree createGuts();

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

// config structs

    struct keyboard_ctrls
    {
        sf::Keyboard::Key up, down, fast;

        bool operator== (const keyboard_ctrls& rhs) const noexcept;

        bool operator!= (const keyboard_ctrls& rhs) const noexcept {
            return !(*this == rhs);
        }
    };

    struct player_input_cfg
    {
        keyboard_ctrls keyboard_controls;

        int joystickId;
        float joystick_deadzone;

        bool use_joystick() const noexcept { return joystickId > -1; }

        bool operator== (const player_input_cfg& rhs) const noexcept;
        bool operator!= (const player_input_cfg& rhs) const noexcept {
            return !(*this == rhs);
        }

    };

// getters e setters para as configurações ativas de usuário

    auto get_keyboard_controls(playerid pl) noexcept -> keyboard_ctrls;
    void set_keyboard_controls(playerid pl, keyboard_ctrls ctrls) noexcept;

    int get_joystick(playerid pl) noexcept;
    void set_joystick(playerid pl, int joyid) noexcept;

    inline void unset_joystick(playerid pl) noexcept {
        set_joystick(pl, -1);
    }

    auto get_input_cfg(playerid player) noexcept ->player_input_cfg;
    void set_input_cfg(player_input_cfg input, playerid player) noexcept;

}