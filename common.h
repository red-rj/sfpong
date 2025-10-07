#pragma once
// common stuff

#include "SFML/Graphics/Rect.hpp"
#include "SFML/System/Vector2.hpp"
#include "spdlog/spdlog.h"


namespace pong
{
    template<typename T, typename E = T>
    using pair = std::pair<T, E>;
    using vec2 = sf::Vector2f;
    // position type
    using pos = sf::Vector2f;
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
    // versão do jogo
    extern const char version[];
}
