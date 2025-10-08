#pragma once
#include "common.h"
#include <string_view>

namespace pong
{
    // representação de um input de joystick
    struct joy_input
    {
        enum input_type { invalid=-1, button, axis } type = invalid;
        int btn_number;
        int axis_id;
        dir axis_dir;
    };

    auto parse_joyinput(std::string_view text)->joy_input;
}
