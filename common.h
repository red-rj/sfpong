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

    template<class T>
    using point = sf::Vector2<T>;

    template<typename T, typename E = T>
    using pair = std::pair<T, E>;

    // position type
    using pos = sf::Vector2f;
    // velocity type
    using vel = sf::Vector2f;
    // bounds type
    using rect = sf::FloatRect;
    // direction type
    enum class dir { up, down, left, right };

    using size2d = sf::Vector2<float>;


    enum class playerid { one, two };

namespace files
{
    constexpr auto
        sans_tff = "support/liberation-sans.ttf",
        mono_tff = "support/liberation-mono.ttf"
        ;
}

    extern const char version[];

    constexpr auto to_string(playerid pl) noexcept
    {
        switch (pl)
        {
        case playerid::one: return "Player 1";
        case playerid::two: return "Player 2";
        default: return "Player ???";
        }
    }

    constexpr auto to_string(dir d) noexcept {
        switch (d)
        {
        case dir::up: return "up";
        case dir::down: return "down";
        case dir::left: return "left";
        case dir::right: return "right";
        default: return "???";
        }
    }

}
