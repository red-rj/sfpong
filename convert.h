#pragma once
#include <iosfwd>
#include <string_view>

#include <fmt/core.h>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>


std::ostream& operator<< (std::ostream& os, sf::Keyboard::Key key);
std::ostream& operator<< (std::ostream& os, sf::Mouse::Button btn);

std::istream& operator>> (std::istream& is, sf::Keyboard::Key& key);
std::istream& operator>> (std::istream& is, sf::Mouse::Button& btn);

