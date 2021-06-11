#pragma once
#include <string_view>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/System/Vector2.hpp>
#include <fmt/format.h>

namespace conv
{
	auto to_string_view(sf::Keyboard::Key key)->std::string_view;
	auto to_string_view(sf::Mouse::Button btn)->std::string_view;

	bool parse(std::string_view text, sf::Keyboard::Key& key);
	bool parse(std::string_view text, sf::Mouse::Button& btn);
}

template <class T>
struct fmt::formatter<sf::Vector2<T>> : formatter<T>
{
	using base = formatter<T>;
	char field_sep = ';';

	template<class Ctx>
	auto format(sf::Vector2f const& vec, Ctx& ctx) {
		base::format(vec.x, ctx);
		format_to(ctx.out(), "{} ", field_sep);
		return base::format(vec.y, ctx);
	}
};
