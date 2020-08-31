#include "input.h"
#include "util.h"
#include "ci_string.h"
#include <tuple>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Joystick.hpp>

using sf::Keyboard;
using sf::Joystick;
using sf::Mouse;

namespace
{
	// up, down, fast
	std::tuple<Keyboard::Key, Keyboard::Key, Keyboard::Key>
		keyboard_controls;
}


auto pong::parse_joyinput(std::string_view text) -> joy_input
{
	constexpr auto npos = std::string_view::npos;

	joy_input js;

	auto p = text.find("Joy");
	auto i = text.find_first_of("B" "P" "XYZRUV", p);
	if (p == npos or i == npos) {
		return js;
	}

	const auto input_id = text[p];

	if (input_id == 'B') { // joybtn
		p++;
		auto* begin = &text[p];
		char* end;
		js.btn_number = std::strtol(begin, &end, 10);
		if (begin != end) {
			js.type = js.button;
		}
		return js;
	}
	else if (input_id == 'P') { // joy pov hat
		p = text.find_first_of("XY", p, 3);
		auto d = text.find_first_of("+-", p, 1);

		if (p == npos or d == npos) {
			return js;
		}

		const auto axis = text[p];
		const auto dir = text[d];

		if (axis == 'X') {
			js.axis_id = Joystick::Axis::PovX;
		}
		else {
			js.axis_id = Joystick::Axis::PovY;
		}

		js.axis_dir = dir == '-' ? dir::up : dir::down;
	}
	else { // joyaxis
		auto const axis = input_id;
		p = text.find_first_of("+-", p, 1);
		if (p == npos)
			return js;
		else {
			js.type = js.axis;
			auto const axd = text[p];
			js.axis_dir = axd == '-' ? dir::up : dir::down;
		}

		switch (axis)
		{
		case 'X': {
			js.axis_id = Joystick::Axis::X;
		} break;
		case 'Y': {
			js.axis_id = Joystick::Axis::Y;
		} break;
		case 'Z': {
			js.axis_id = Joystick::Axis::Z;
		} break;
		case 'R': {
			js.axis_id = Joystick::Axis::R;
		} break;
		case 'U': {
			js.axis_id = Joystick::Axis::U;
		} break;
		case 'V': {
			js.axis_id = Joystick::Axis::V;
		} break;
		default:
			js.type = js.invalid;
			break;
		}
	}

	return js;
}
