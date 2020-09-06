#pragma once
#include "common.h"
#include <string_view>
#include <vector>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Joystick.hpp>

namespace pong
{
    enum class Player
    {
        One, Two
    };

    struct keyboard_ctrls
    {
        sf::Keyboard::Key up, down, fast;

        bool operator== (const keyboard_ctrls& rhs) const noexcept;
    };

    // representação de um input de joystick
    struct joy_input
    {
        enum input_type { invalid=-1, button, axis } type = invalid;
        int btn_number;
        int axis_id;
        dir axis_dir;
    };

    auto parse_joyinput(std::string_view text)->joy_input;

    keyboard_ctrls get_controls(Player pl) noexcept;
    void set_controls(keyboard_ctrls ctrls, Player pl) noexcept;

    unsigned get_joystick_for(Player pl) noexcept;
    void set_joystick_for(Player pl, unsigned joyid) noexcept;

    auto get_joystick_names()->std::vector<std::string> const&;
    void refresh_joystick_names();
}
