#pragma once
// common stuff

#include "SFML/Graphics/Rect.hpp"
#include "SFML/System/Vector2.hpp"
#include "spdlog/spdlog.h"

#define RED_TU_MAIN struct __red_tu_main_t__ { __red_tu_main_t__(); } static __tu_main__; \
    __red_tu_main_t__::__red_tu_main_t__()

namespace pong
{
    inline auto LOGGER_NAME = "sfPong";
    inline auto gamelog() { return spdlog::get(LOGGER_NAME); }

    // position type
    using pos = sf::Vector2f;
    // velocity type
    using vel = sf::Vector2f;
    // bounds type
    using rect = sf::FloatRect;
}
