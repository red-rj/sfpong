#include "menu.h"
#include "convert.h"
#include "game.h"
#include "common.h"

#include <algorithm>
#include <optional>

#include "imgui_ext.h"
#include <imgui-SFML.h>
#include <fmt/ostream.h>
#include <SFML/Window/Window.hpp>

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

static auto scan_mouse() noexcept -> std::optional<sf::Mouse::Button>
{
	using sf::Mouse;
	constexpr auto Count = Mouse::ButtonCount;
	static_assert(Count <= ImGuiMouseButton_COUNT, "Num. btns de Mouse errado!");

	auto& io = ImGui::GetIO();
	auto const begin = io.MouseDown;
	auto const end = begin + Count;
	auto it = std::find(begin, end, true);

	if (it != end) {
		return Mouse::Button(it - begin);
	}
	else return {};
}


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
	if (Button(u8"Opções", btnSize)) {
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

	struct {
		float paddle_size[2]{};
		int framerate = 0;
	} static model;

	auto& active_config = ctx->Config;

	{
		ImGui::SetNextWindowSize({ 500, 400 }, ImGuiCond_FirstUseEver);
		auto lastPos = ImGui::GetWindowPos();
		ImGui::SetNextWindowPos({ lastPos.x + 10, lastPos.y + 10 }, ImGuiCond_FirstUseEver);
	}

	auto wflags = active_config != config ? ImGuiWindowFlags_UnsavedDocument : 0;
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
				using namespace ImScoped;
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

			auto InputPlayerCtrls = [&](const char* player, pong::kb_keys& keys) {
				ImGui::Text("%s:", player);
				ID _id_ = player;
				Indent _ind_{ 5.f };

				InputControl("Up", keys.up);
				InputControl("Down", keys.down);
				InputControl("Fast", keys.fast);
			};

			// ---
			auto& ctrls = config.controls;
			InputPlayerCtrls("Player 1", ctrls[0]);
			InputPlayerCtrls("Player 2", ctrls[1]);
		}
		if (auto gametab = TabBarItem("Game vars"))
		{
			ImGui::Text(u8"É preciso reiniciar o jogo para que as alterações desta aba tenham efeito.");
			{
				ID _id_ = "paddle";
				ImGui::Text("Paddle vars");
				ImGui::InputFloat("Base speed", &config.paddle.base_speed);
				ImGui::InputFloat("Acceleration", &config.paddle.accel);

				if (ImGui::InputFloat2("Size", model.paddle_size)) {
					config.paddle.size.x = model.paddle_size[0];
					config.paddle.size.y = model.paddle_size[1];
				}
				else {
					model.paddle_size[0] = config.paddle.size.x;
					model.paddle_size[1] = config.paddle.size.y;
				}
			}
			{
				ID _id_ = "ball";
				ImGui::Text("Ball vars");
				ImGui::InputFloat("Base speed", &config.ball.base_speed);
				ImGui::InputFloat("Acceleration", &config.ball.accel);
				ImGui::InputFloat("Max speed", &config.ball.max_speed);
				ImGui::InputFloat("Radius", &config.ball.radius);
			}

			ImGui::Text("Misc");
			if (ImGui::SliderInt("Framerate", &model.framerate, 15, 144)) {
				config.framerate = (unsigned)model.framerate;
			}
			else {
				model.framerate = (int)config.framerate;
			}
		}
	}

	ImGui::Spacing();
	ImGui::Separator();

	if (ImGui::Button("Discard")) {
		config = active_config;
	}
	ImGui::SameLine();
	if (ImGui::Button("Save") && active_config != config) {
		active_config = config;
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
