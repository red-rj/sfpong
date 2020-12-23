#include "joyinput.h"
#include "ci_string.h"
#include <vector>
#include <SFML/Window/Joystick.hpp>
#include <regex>


using sf::Joystick;
using util::ci_string_view;

template<class Sv, class Iter>
static Sv mk_strview(Iter begin, Iter end) {
	return { std::addressof(*begin), size_t(std::distance(begin, end)) };
}


auto pong::parse_joyinput(std::string_view input) -> joy_input
{
	namespace rxc = std::regex_constants;
	using svmatch = std::match_results<std::string_view::const_iterator>;

	// https://regex101.com/r/Y0JvkF/1
	auto joy_rx = std::regex("Joy(B(\\d+)|[XYZRUV]([+-])|Pov([XY][+-])", rxc::icase);
	enum { InputID=1, Arg };

	joy_input js;

	svmatch match;
	if (regex_match(input.begin(), input.end(), match, joy_rx)) {
		auto input_id = mk_strview<ci_string_view>(match[InputID].first, match[InputID].second);
		auto id_letter = input_id.substr(0, 1);

		try
		{
			if (id_letter == "B") {
				// button
				js.btn_number = std::stoi(match[Arg].str());
				js.type = js.button;
			}
			else if (id_letter == "P") {
				// povhat
				auto value = mk_strview<ci_string_view>(match[Arg].first, match[Arg].second);
				auto axis = value.substr(0, 1);
				auto dir = value.substr(1);

				if (axis == "X") {
					js.axis_id = Joystick::Axis::PovX;
				}
				else if (axis == "Y") {
					js.axis_id = Joystick::Axis::PovY;
				}

				if (dir == "-") {
					js.axis_dir = pong::dir::up;
				}
				else if (dir == "+") {
					js.axis_dir = pong::dir::down;
				}
			}
			else {
				// axis
				auto axis = id_letter;
				char d = *match[Arg].first;

				if (d == '-') {
					js.axis_dir = pong::dir::up;
				}
				else if (d == '+') {
					js.axis_dir = pong::dir::down;
				}

				if (axis == "X") {
					js.axis_id = Joystick::Axis::X;
				}
				else if (axis == "Y") {
					js.axis_id = Joystick::Axis::Y;
				}
				else if (axis == "Z") {
					js.axis_id = Joystick::Axis::Z;
				}
				else if (axis == "R") {
					js.axis_id = Joystick::Axis::R;
				}
				else if (axis == "U") {
					js.axis_id = Joystick::Axis::U;
				}
				else if (axis == "V") {
					js.axis_id = Joystick::Axis::V;
				}

				js.type = js.axis;
			}
		}
		catch (const std::exception&)
		{
			js.type = js.invalid;
		}
	}

	return js;
}
