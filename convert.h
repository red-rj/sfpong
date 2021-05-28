#pragma once
#include <iosfwd>
#include <string_view>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

std::ostream& operator<< (std::ostream& os, sf::Keyboard::Key key);
std::ostream& operator<< (std::ostream& os, sf::Mouse::Button btn);

std::istream& operator>> (std::istream& is, sf::Keyboard::Key& key);
std::istream& operator>> (std::istream& is, sf::Mouse::Button& btn);

#include <fmt/format.h>
#include <SFML/System/Vector2.hpp>

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