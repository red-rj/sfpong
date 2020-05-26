#pragma once
#include <string_view>
#include <filesystem>
#include <array>

#include <SFML/Window/Keyboard.hpp>


namespace pong
{
    struct kb_keys { sf::Keyboard::Key up, down, fast; };

    struct config_t
    {
        using Keyboard = sf::Keyboard;

        void load(std::filesystem::path filepath);
        void save(std::filesystem::path filepath);

        // ----
        std::array<kb_keys, 2> controls;

        struct paddle_cfg {
            float base_speed, accel;
            sf::Vector2f size;
        } paddle;

        struct ball_cfg {
            float base_speed, accel, max_speed;
            float radius;
        } ball;

        unsigned framerate=0;

        
        bool operator== (const config_t& rhs) const noexcept;
    };

    // operators
    using std::rel_ops::operator!=;

    constexpr bool operator==(const kb_keys& lhs, const kb_keys& rhs)
    {
        return std::tie(lhs.up, lhs.down, lhs.fast) == std::tie(rhs.up, rhs.down, rhs.fast);
    }

    constexpr bool operator== (config_t::paddle_cfg const& lhs, config_t::paddle_cfg const& rhs)
    {
        return std::tie(lhs.accel, lhs.base_speed, lhs.size) ==
               std::tie(rhs.accel, rhs.base_speed, rhs.size);
    }
    constexpr bool operator== (config_t::ball_cfg const& lhs, config_t::ball_cfg const& rhs)
    {
        return std::tie(lhs.accel, lhs.base_speed, lhs.max_speed, lhs.radius) ==
               std::tie(rhs.accel, rhs.base_speed, rhs.max_speed, rhs.radius);
    }

}