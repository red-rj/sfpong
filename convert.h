#pragma once
#include <string_view>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/System/Vector2.hpp>
#include <fmt/format.h>
#include "common.h"

namespace conv
{
	auto to_string_view(sf::Keyboard::Key key)->std::string_view;
	auto to_string_view(sf::Mouse::Button btn)->std::string_view;

	bool parse(std::string_view text, sf::Keyboard::Key& key);
	bool parse(std::string_view text, sf::Mouse::Button& btn);

	auto to_string_view(pong::playerid pid) noexcept -> std::string_view;
	auto to_string_view(pong::dir d) noexcept -> std::string_view;
}

template <class T>
class fmt::formatter<sf::Vector2<T>> : public formatter<T>
{
	using base = formatter<T>;
	std::string_view sep = ", ";

public:
	template<class Ctx>
	auto format(sf::Vector2f const& vec, Ctx& ctx) const {
		return format_to(ctx.out(), "(x={0:.1f}{1}y={2:.1f})", vec.x, sep, vec.y);
	}
};
