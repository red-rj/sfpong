#include <string>
#include <array>
#include <vector>
#include <algorithm>
#include "fmt/format.h"
#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include "../input.h"


TEST_CASE("Joystick parse")
{
    auto valid_btns = std::array{
        "JoyB1", "JoyB2", "JoyB3", "JoyB4", "JoyB5"
    };
    auto valid_axis = std::array{
        "JoyX+" , "JoyX-" , "JoyY+" , "JoyY-"
    };
    auto valid_pov = std::array{
        "JoyPovX+", "JoyPovY+", "JoyPovX-", "JoyPovY-"
    };
    //auto const total_count = valid_btns.size() + valid_axis.size() + valid_pov.size();

    auto invalid_inputs = std::array{
        "Joy","Joy1", "JoyZ", "JoyPov", "JoyPovX",
        "X+", "Y-", "loren Joy ipsum veridr"
    };

    pong::joy_input result;

    SECTION("Valid")
    {
        for (auto input : valid_btns)
        {
            result = pong::parse_joyinput(input);
            CAPTURE(input);
            REQUIRE(result.type == result.button);
        }
        for (auto input : valid_axis)
        {
            result = pong::parse_joyinput(input);
            CAPTURE(input);
            REQUIRE(result.type == result.axis);
        }
        for (auto input : valid_pov)
        {
            result = pong::parse_joyinput(input);
            CAPTURE(input);
            REQUIRE(result.type == result.axis);
        }
    }
    SECTION("Invalid")
    {
        for (auto input : invalid_inputs)
        {
            result = pong::parse_joyinput(input);
            CAPTURE(input);
            REQUIRE(result.type == result.invalid);
        }
    }
}