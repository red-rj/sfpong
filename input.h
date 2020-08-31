#pragma once
#include <string_view>

namespace pong
{
    struct input_state
    {
        bool up, down, fast;
    };

    using input_handler = bool(*)(input_state&);    

    enum class Player
    {
        One, Two
    };


    struct joystick_input
    {
        enum input_type { invalid=-1, button, axis } type = invalid;
        int btn_number;
        int axis_id;
        bool axis_dir;
    };

    auto parse_jsinput(std::string_view text)->joystick_input;
}
