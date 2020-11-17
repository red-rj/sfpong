#include "joyinput.h"
#include "ci_string.h"
#include <vector>
#include <SFML/Window/Joystick.hpp>


using sf::Joystick;
using ci_string_view = std::basic_string_view<char, pong::util::ci_char_traits<char>>;

auto pong::parse_joyinput(std::string_view arg) -> joy_input
{
	constexpr auto npos = std::string::npos;
	constexpr auto INPUT_ID = "B" "P" "XYZRUV";
	constexpr auto AXIS_ID = INPUT_ID+2;
	constexpr auto AXIS_DIR = "+-";

	joy_input js;

	if (arg.length() < 5) {
		js.type = js.invalid;
		return js;
	}

	auto text = ci_string_view(arg.data(), arg.length());

	auto pos = text.find("Joy");
	pos = text.find_first_of(INPUT_ID, pos + 3);
	if (pos == npos) {
		js.type = js.invalid;
		return js;
	}

	const auto input_id = text[pos];

	if (input_id == 'B') { // joybtn
		js.type = js.button;

		try
		{
			auto num = std::string(arg.substr(pos+1));
			js.btn_number = std::stoi(num);
		}
		catch (const std::invalid_argument&)
		{
			js.type = js.invalid;
		}

		return js;
	}
	else if (input_id == 'P') { // joy povhat
		js.type = js.axis;

		try
		{
			const auto axis = text.substr(pos+1, 1);
			const auto dir = text.substr(pos+2, 1);

			if (axis=="X") {
				js.axis_id = Joystick::Axis::PovX;
			}
			else if (axis=="Y") {
				js.axis_id = Joystick::Axis::PovY;
			}
			else {
				js.type = js.invalid;
			}

			if (dir=="-") {
				js.axis_dir = dir::up;
			}
			else if (dir=="+") {
				js.axis_dir = dir::down;
			}
			else {
				js.type = js.invalid;
			}
		}
		catch (const std::exception&)
		{
			js.type = js.invalid;
		}

		return js;
	}
	else { // joyaxis
		auto const axis = input_id;
		js.type = js.axis;

		try
		{
			auto dir = text.substr(pos + 1, 1);

			if (dir == "-") {
				js.axis_dir = dir::up;
			}
			else if (dir == "+") {
				js.axis_dir = dir::down;
			}
			else {
				js.type = js.invalid;
			}
		}
		catch (const std::exception&)
		{
			js.type = js.invalid;
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

		return js;
	}
}
