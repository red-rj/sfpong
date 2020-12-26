#include "joyinput.h"
#include "ci_string.h"
#include <regex>
#include <SFML/Window/Joystick.hpp>


using sf::Joystick;
using util::ci_string_view;

template<class Sv, class Iter>
static Sv mk_strview(Iter begin, Iter end) {
	return { std::addressof(*begin), size_t(std::distance(begin, end)) };
}

static auto parse_axis(char ch, bool povhat)->int
{
	const ci_string_view axis_letters = "XYZRUV"; // mesma ordem do enum
	auto pos = axis_letters.find(ch);

	if (pos < Joystick::AxisCount) {
		auto axis = povhat ? pos + Joystick::PovX : pos;
		return axis;
	}
	else
		throw std::invalid_argument("invalid axis");
}
static auto parse_dir(char ch)->pong::dir
{
	using pong::dir;

	if (ch == '-') {
		return dir::up;
	}
	else if (ch == '+') {
		return dir::down;
	}
	else
		throw std::invalid_argument("invalid dir");
}

auto pong::parse_joyinput(std::string_view input) -> joy_input
{
	namespace rxc = std::regex_constants;
	using std::string_view;

	// https://regex101.com/r/Y0JvkF/1
	auto joy_rx = std::regex("Joy(B(\\d+)|[XYZRUV][+-]|Pov([XY][+-]))", rxc::icase);
	enum { InputID=1, BtnNum, PovAxis };

	joy_input js;
	std::cmatch match;

	if (regex_match(input.data(), input.data() + input.length(), match, joy_rx)) {
		auto input_id = ci_string_view(match[InputID].first, match[InputID].length());
		auto starts_with = [=](string_view sv) {
			return input_id.compare(0, sv.length(), sv.data()) == 0;
		};

		try
		{
			if (starts_with("B")) {
				// button
				js.btn_number = std::stoi(match[BtnNum].str());
				js.type = js.button;
			}
			else if (starts_with("Pov")) {
				// povhat
				auto povhat = ci_string_view(match[PovAxis].first, 2);

				js.axis_id = parse_axis(povhat.front(), true);
				js.axis_dir = parse_dir(povhat[1]);

				js.type = js.axis;
			}
			else {
				// axis
				js.axis_id = parse_axis(input_id.front(), false);
				js.axis_dir = parse_dir(input_id[1]);

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
