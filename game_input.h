#pragma once
#include <set>
#include <string>
#include <string_view>
#include "SFML/Window/Keyboard.hpp"
//#include "SFML/Window/Joystick.hpp"
//#include "SFML/Window/Mouse.hpp"

namespace red {

	struct game_entity;

	class control_bind
	{
	public:
		using checker_func = void(*)(game_entity const*);


	private:
		std::string m_name;
	};

}