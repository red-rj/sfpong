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

        bool operator!= (const keyboard_ctrls& rhs) const noexcept {
            return !(*this == rhs);
        }
    };

    struct player_input_cfg
    {
        keyboard_ctrls keyboard_controls;
        int joystickId;

        bool operator== (const player_input_cfg& rhs) const noexcept
        {
            return keyboard_controls == rhs.keyboard_controls
                && joystickId == rhs.joystickId;
        }
        bool operator!= (const player_input_cfg& rhs) const noexcept
        {
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

    auto parse_joyinput(std::string_view text)->joy_input;

    // getters e setters para as configurações ativas de input

    auto get_keyboard_controls(Player pl) noexcept -> keyboard_ctrls;
    void set_keyboard_controls(Player pl, keyboard_ctrls ctrls) noexcept;

    unsigned get_joystick_for(Player pl) noexcept;
    void set_joystick_for(Player pl, unsigned joyid) noexcept;

    inline void unset_joystick_for(Player pl) noexcept {
        set_joystick_for(pl, unsigned(-1));
    }

    inline auto get_input_cfg(Player player)
    {
        player_input_cfg cfg;
        cfg.keyboard_controls = get_keyboard_controls(player);
        cfg.joystickId = get_joystick_for(player);
        return cfg;
    }

    auto get_joystick_names()->std::vector<std::string> const&;
    void refresh_joystick_names();
}
