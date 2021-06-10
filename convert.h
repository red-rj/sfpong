#pragma once
#include <iosfwd>
#include <string_view>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/System/Vector2.hpp>
#include <fmt/format.h>


//namespace conv
//{
//	std::string_view serialize(sf::Keyboard::Key key) noexcept;
//	std::string_view serialize(sf::Mouse::Button btn) noexcept;
//
//	sf::Keyboard::Key parse_kbkey(std::string_view text) noexcept;
//	sf::Mouse::Button parse_mousebtn(std::string_view text) noexcept;
//}

std::ostream& operator<< (std::ostream& os, sf::Keyboard::Key key);
std::ostream& operator<< (std::ostream& os, sf::Mouse::Button btn);

std::istream& operator>> (std::istream& is, sf::Keyboard::Key& key);
std::istream& operator>> (std::istream& is, sf::Mouse::Button& btn);

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
