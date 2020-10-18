#include "joyinput.h"
#include "ci_string.h"
#include <vector>
#include <SFML/Window/Joystick.hpp>


using sf::Joystick;

auto pong::parse_joyinput(std::string_view arg) -> joy_input
{
	constexpr auto npos = std::string::npos;
	constexpr auto INPUT_ID = "B" "P" "XYZRUV";
	constexpr auto AXIS_ID = INPUT_ID+2;
	constexpr auto AXIS_DIR = "+-";

	auto text = red::to_ci(arg);

	joy_input js;

	auto pos = text.find("Joy");
	pos = text.find_first_of(INPUT_ID, pos + 3);
	if (pos == npos) {
		return js;
	}

	const auto input_id = text[pos];

	if (input_id == 'B') { // joybtn
		try
		{
			auto num = std::string(arg.substr(pos+1));
			js.btn_number = std::stoi(num);
			js.type = js.button;
		}
		catch (const std::invalid_argument&)
		{
			js.type = js.invalid;
		}

		return js;
	}
	else if (input_id == 'P') { // joy povhat
		pos = text.find_first_of(AXIS_ID, pos, 2); // "XY"
		auto pos_dir = text.find_first_of(AXIS_DIR, pos);
		if (pos == npos or pos_dir == npos) {
			js.type = js.invalid;
			return js;
		}
		else js.type = js.axis;

		const auto axis = text[pos];
		const auto axd = text[pos_dir];

		if (axis == 'X') {
			js.axis_id = Joystick::Axis::PovX;
		}
		else {
			js.axis_id = Joystick::Axis::PovY;
		}

		js.axis_dir = axd == '-' ? dir::up : dir::down;
	}
	else { // joyaxis
		auto const axis = input_id;
		pos = text.find_first_of(AXIS_DIR, pos, 2);
		if (pos == npos) {
			js.type = js.invalid;
			return js;
		}
		else {
			js.type = js.axis;
			auto const axd = text[pos];
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
