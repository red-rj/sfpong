#pragma once
#include "common.h"
#include <string_view>
#include <vector>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Joystick.hpp>

namespace pong
{
    struct keyboard_ctrls
    {
        sf::Keyboard::Key up, down, fast;

        bool operator== (const keyboard_ctrls& rhs) const noexcept;

        bool operator!= (const keyboard_ctrls& rhs) const noexcept {
            return !(*this == rhs);
        }
    };

    // representação de um input de joystick
    struct joy_input
    {
        enum input_type { invalid=-1, button, axis } type = invalid;
        int btn_number;
        int axis_id;
        dir axis_dir;
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


    auto parse_joyinput(std::string_view text)->joy_input;

    // getters e setters para as configurações ativas de input

    auto get_keyboard_controls(playerid pl) noexcept -> keyboard_ctrls;
    void set_keyboard_controls(playerid pl, keyboard_ctrls ctrls) noexcept;

    unsigned get_joystick(playerid pl) noexcept;
    void set_joystick(playerid pl, unsigned joyid) noexcept;

    inline void unset_joystick(playerid pl) noexcept {
        set_joystick(pl, unsigned(-1));
    }

    auto get_input_cfg(playerid player) noexcept ->player_input_cfg;
    void set_input_cfg(player_input_cfg input, playerid player) noexcept;

    auto get_joystick_names()->std::vector<std::string> const&;
    void refresh_joystick_names();
}
