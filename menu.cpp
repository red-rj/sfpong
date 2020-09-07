#include "menu.h"
#include "convert.h"
#include "game.h"
#include "common.h"
#include "input.h"
#include "imgui_ext.h"

#include <algorithm>
#include <optional>
#include <tuple>

#include <imgui-SFML.h>
#include <fmt/ostream.h>
#include <SFML/Window/Window.hpp>
#include <boost/range/algorithm.hpp>

static auto scan_kb() noexcept -> std::optional<sf::Keyboard::Key>
{
	using sf::Keyboard;
	
	auto& io = ImGui::GetIO();
	auto const begin = io.KeysDown;
	auto const end = begin + Keyboard::KeyCount;
	auto it = std::find(begin, end, true);

	if (it != end) {
		return Keyboard::Key(it - begin);
	}
	else return {};
}

static auto scan_mouse_btn() noexcept -> std::optional<sf::Mouse::Button>
{
	using sf::Mouse;
	constexpr auto Count = Mouse::ButtonCount;

	auto& io = ImGui::GetIO();
	auto const begin = io.MouseDown;
	auto const end = begin + Count;
	auto it = std::find(begin, end, true);

	if (it != end) {
		return Mouse::Button(it - begin);
	}
	else return {};
}

static auto scan_joy_btn() noexcept
{
	using sf::Joystick;

	for (unsigned id = 0; id < Joystick::Count; id++)
	{
		const auto btnCount = Joystick::getButtonCount(id);
		for (unsigned btn = 0; btn < btnCount; btn++)
		{
			if (Joystick::isButtonPressed(id, btn))
			{
				return std::make_tuple(btn, id);
			}
		}
	}
}

static void selectJoystick(pong::Player player, int& joyid);



void pong::menu_state::draw(game* ctx, sf::Window* window)
{
	if (show_options)
		guiOptions(ctx);

	if (show_stats)
		guiStats(ctx);

	if (!ctx->paused) return;

	using namespace ImGui;
	using namespace ImScoped;

	Window menu("Menu", &ctx->paused);
	auto btnSize = sf::Vector2i(100, 30);

	if (Button("Jogar", btnSize)) {
		ctx->paused = false;
	}
	if (Button(u8"Op��es", btnSize)) {
		show_options = true;
	}
	if (Button("DEV", btnSize)) {
		show_stats = true;
	}
	if (Button("Sair", btnSize)) {
		window->close();
		gamelog()->info("ate a proxima! ;D");
	}

}

void pong::menu_state::guiOptions(game* ctx)
{
	using namespace ImScoped;

	std::array<player_input_cfg, 2> active_settings = {{
		{get_keyboard_controls(Player::One), get_joystick_for(Player::One)},
		{get_keyboard_controls(Player::Two), get_joystick_for(Player::Two)}
	}};

	{
		ImGui::SetNextWindowSize({ 500, 400 }, ImGuiCond_FirstUseEver);
		auto lastPos = ImGui::GetWindowPos();
		ImGui::SetNextWindowPos({ lastPos.x + 10, lastPos.y + 10 }, ImGuiCond_FirstUseEver);
	}

	auto wflags = input_settings != active_settings ? ImGuiWindowFlags_UnsavedDocument : 0;

	Window guiwindow("Config.", &show_options, wflags);
	if (!guiwindow)
		return;

	if (auto tabbar = TabBar("##Tabs"))
	{
		if (auto ctrltab = TabBarItem("Controls"))
		{

			auto InputControl = [id = 0, this](const char* label, sf::Keyboard::Key& curKey) mutable
			#pragma region Block
			{
				using namespace ImGui;

				ID _id_ = id++;
				auto constexpr popup_id = "Rebind popup";
				auto keystr = fmt::format("{}", curKey);

				Text("%8s:\t%s", label, keystr.c_str());
				SameLine(200);
				if (Button("Trocar")) {
					OpenPopup(popup_id);
					rebinding = true;
				}

				if (auto popup = PopupModal(popup_id, &rebinding, ImGuiWindowFlags_NoMove))
				{
					using Key = sf::Keyboard::Key;
					Text("Pressione uma nova tecla para '%s', ou Esc para cancelar.", label);

					auto key = scan_kb();
					if (key && key.value() != Key::Escape)
					{
						curKey = key.value();
					}
				}
			};
			#pragma endregion

			auto InputPlayerCtrls = [&](pong::Player pl) mutable
			{
				ID _id_ = int(pl);
				auto* title = "Player ???";
				switch (pl)
				{
				case pong::Player::One: title = "Player 1"; break;
				case pong::Player::Two: title = "Player 2"; break;
				}

				ImGui::Text("%s:", title);
				Indent _ind_{ 5.f };

				auto& settings = input_settings[int(pl)];
				auto& player_ctrls = settings.keyboard_controls;

				InputControl("Up", player_ctrls.up);
				InputControl("Down", player_ctrls.down);
				InputControl("Fast", player_ctrls.fast);

				selectJoystick(pl, settings.joystickId);
			};

			// ---
			InputPlayerCtrls(Player::One);
			InputPlayerCtrls(Player::Two);
		}
	}

	ImGui::Spacing();
	ImGui::Separator();

	if (ImGui::Button("Discard")) {
		input_settings = active_settings;
	}
	ImGui::SameLine();
	if (ImGui::Button("Save") && (wflags & ImGuiWindowFlags_UnsavedDocument) != 0) {
		for (auto player : { Player::One, Player::Two })
		{
			auto& setting = input_settings[int(player)];
			set_keyboard_controls(player, setting.keyboard_controls);
			set_joystick_for(player, setting.joystickId);
		}
	}
}

void pong::menu_state::guiStats(game* ctx)
{
	using namespace ImScoped;

	ImGuiIO& io = ImGui::GetIO();
	pos winpos = { io.DisplaySize.x - 10.f, 15.f };
	ImGui::SetNextWindowBgAlpha(0.5f);
	ImGui::SetNextWindowPos(winpos, ImGuiCond_FirstUseEver, { 1.f, 0 });

	auto constexpr wflags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing;
	Window overlay("Stats", &show_stats, wflags);

	auto const& P1 = ctx->Player1;
	auto const& P2 = ctx->Player2;
	auto const& Ball = ctx->Ball;

	auto p1_pos = P1.getPosition();
	auto p2_pos = P2.getPosition();
	auto ball_pos = Ball.getPosition();

	auto text = fmt::format("P1: [{:.2f}, {:.2f}]\nP2: [{:.2f}, {:.2f}]\nBall: [{:.2f}, {:.2f}]",
		p1_pos.x, p1_pos.y, p2_pos.x, p2_pos.y, ball_pos.x, ball_pos.y);

	ImGui::Text("Positions:\n%s", text.c_str());


	text = fmt::format("P1: {:.3f}\nP2: {:.3f}\nBall: [{:.3f}, {:.3f}]", 
		P1.velocity.y, P2.velocity.y, Ball.velocity.x, Ball.velocity.y);

	ImGui::Text("Velocity:\n%s", text.c_str());
}

void selectJoystick(pong::Player player, int& joyid)
{
	using namespace ImGui;
	namespace gui = ImScoped;
	using sf::Joystick;

	auto& jsnames = pong::get_joystick_names();

	auto previewItem = joyid == -1 ? "None" : jsnames[joyid];
	if (auto cb = gui::Combo("Select joystick", previewItem.c_str()))
	{
		if (Selectable("None", joyid==-1)) {
			pong::unset_joystick_for(player);
		}

		auto s=0;
		for (auto& name : jsnames)
		{
			bool is_selected = s == joyid;
			if (Selectable(name.c_str(), is_selected))
				pong::set_joystick_for(player, s);

			if (is_selected)
				SetItemDefaultFocus();

			s++;
		}
	}
}
