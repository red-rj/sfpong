#pragma once
// common stuff

#include "SFML/Graphics/Rect.hpp"
#include "SFML/System/Vector2.hpp"
#include "spdlog/spdlog.h"

#define DEF_PROPERTY(getter, setter, type, member) \
    constexpr auto& getter() const noexcept { return member; } \
    constexpr void setter(type&& val) { member = val; }

#define PROPERTY(type, member) DEF_PROPERTY(get_##member, set_##member, type, member)

namespace pong
{
    namespace log = spdlog;

    template<typename T, typename E = T>
    using pair = std::pair<T, E>;

    // position type
    using pos = sf::Vector2f;
    // velocity type
    using vel = sf::Vector2f;
    // bounds type
    using rect = sf::Rect<float>;
    // direction type
    enum class dir { up, down, left, right };

    using size2d = sf::Vector2f;
    using point = sf::Vector2f;

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
