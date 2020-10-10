#include "input.h"
#include "ci_string.h"

#include <tuple>
#include <vector>

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

using sf::Keyboard;
using sf::Joystick;
using sf::Mouse;

namespace
{
	unsigned player_joystick[2] = { -1, -1 };
	pong::keyboard_ctrls player_keyboard_controls[2] = {
			{Keyboard::W, Keyboard::S, Keyboard::LShift},
			{Keyboard::Up, Keyboard::Down, Keyboard::RControl}
	};
	float player_deadzone[2];

	bool refresh_jsinfo = true;
	std::vector<std::string> list_of_joysticks;
}


auto pong::parse_joyinput(std::string_view arg) -> joy_input
{
	constexpr auto npos = std::string_view::npos;
	constexpr auto INPUT_ID = "B" "P" "XYZRUV";
	constexpr auto AXIS = INPUT_ID+2;
	constexpr auto AXIS_DIR = "+-";

	auto text = red::to_ci(arg);

	joy_input js;

	auto p = text.find("Joy");
	p = text.find_first_of(INPUT_ID, p+3);
	if (p == npos) {
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
		p = text.find_first_of(AXIS, p, 2);
		auto d = text.find_first_of(AXIS_DIR, p);

		if (p == npos or d == npos) {
			return js;
		}
		else js.type = js.axis;

		const auto axis = text[p];
		const auto axd = text[d];

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
		p = text.find_first_of(AXIS_DIR, p, 2);
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

pong::keyboard_ctrls pong::get_keyboard_controls(playerid pl) noexcept
{
	return player_keyboard_controls[int(pl)];
}

void pong::set_keyboard_controls(playerid pl, keyboard_ctrls ctrls) noexcept
{
	player_keyboard_controls[int(pl)] = ctrls;
}

unsigned pong::get_joystick_for(playerid pl) noexcept
{
	return player_joystick[int(pl)];
}

void pong::set_joystick_for(playerid pl, unsigned joyid) noexcept
{
	player_joystick[int(pl)] = joyid;
}

auto pong::get_input_cfg(playerid player) noexcept -> player_input_cfg
{
	const auto p = int(player);
	player_input_cfg cfg;
	cfg.joystickId = player_joystick[p];
	cfg.joystick_deadzone = player_deadzone[p];
	cfg.keyboard_controls = player_keyboard_controls[p];
	return cfg;
}

void pong::set_input_cfg(player_input_cfg input, playerid player) noexcept
{
	const auto p = int(player);
	player_keyboard_controls[p] = input.keyboard_controls;
	player_joystick[p] = input.joystickId;
	player_deadzone[p] = input.joystick_deadzone;
}

auto pong::get_joystick_names() -> const std::vector<std::string>&
{
	if (refresh_jsinfo)
	{
		list_of_joysticks.reserve(Joystick::Count);
		list_of_joysticks.clear();

		for (unsigned id = 0; id < Joystick::Count; id++)
		{
			if (Joystick::isConnected(id)) {
				auto info = Joystick::getIdentification(id);
				list_of_joysticks.emplace_back(info.name);
			}
		}

		refresh_jsinfo = false;
	}

	return list_of_joysticks;
}

void pong::refresh_joystick_names()
{
	refresh_jsinfo = true;
}


bool pong::keyboard_ctrls::operator==(const keyboard_ctrls& rhs) const noexcept
{
	return std::tie(up,down,fast) == std::tie(rhs.up,rhs.down,rhs.fast);
}

bool pong::player_input_cfg::operator==(const player_input_cfg& rhs) const noexcept
{
	using std::tie;
	return tie(keyboard_controls, joystickId, joystick_deadzone) == 
		tie(rhs.keyboard_controls, rhs.joystickId, rhs.joystick_deadzone);
}
