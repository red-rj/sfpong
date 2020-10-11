#pragma once
// common stuff

#include "SFML/Graphics/Rect.hpp"
#include "SFML/System/Vector2.hpp"
#include "spdlog/spdlog.h"

#define RED_TU_MAIN struct __red_tu_main_t__ { __red_tu_main_t__(); } static __tu_main__; \
    __red_tu_main_t__::__red_tu_main_t__

namespace pong
{
    namespace log = spdlog;

    // position type
    using pos = sf::Vector2f;
    // velocity type
    using vel = sf::Vector2f;
    // bounds type
    using rect = sf::FloatRect;
    // direction type
    enum class dir { up, down, left, right };

    template<class T>
    using size_2d = sf::Vector2<T>;

    using size2d = sf::Vector2f;

    enum class playerid { invalid=-1, one, two };

    template<typename T, typename E = T>
    using pair = std::pair<T, E>;

}
