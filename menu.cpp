#include "menu.h"
#include "convert.h"
#include "game.h"
#include "common.h"

#include <algorithm>

#include "imgui_ext.h"
#include <imgui-SFML.h>
#include <fmt/ostream.h>
#include <SFML/Window/Window.hpp>


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
	if (Button("Opcoes", btnSize)) {
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
	namespace im = ImGui;
	using namespace ImScoped;

	struct {
		float paddle_size[2]{};
		int framerate = 0;
	} static model;

	auto& active_config = ctx->Config;

	{
		ImGui::SetNextWindowSize({ 500, 400 }, ImGuiCond_FirstUseEver);
		auto lastPos = im::GetWindowPos();
		im::SetNextWindowPos({ lastPos.x + 10, lastPos.y + 10 }, ImGuiCond_FirstUseEver);
	}

	auto wflags = active_config != config ? ImGuiWindowFlags_UnsavedDocument : 0;
	Window guiwindow("Config.", &show_options, wflags);
	if (!guiwindow)
		return;

	if (auto tabbar = TabBar("##Tabs"))
	{
		if (auto ctrltab = TabBarItem("Controls"))
		{

			auto InputControl = [id = 0, this, ctx](const char* label, sf::Keyboard::Key& curKey) mutable
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

					auto& io = ImGui::GetIO();
					auto const keysEnd = io.KeysDown + Key::KeyCount;
					auto it = std::find(io.KeysDown, keysEnd, true);

					if (it != keysEnd) {
						auto key = Key( it - io.KeysDown );
						if (key != Key::Escape) {
							curKey = key;
						}

						CloseCurrentPopup();
					}
				}
			};
			#pragma endregion

			auto InputPlayerCtrls = [&](const char* player, pong::kb_keys& keys) {
				im::Text("%s:", player);
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
			{
				ID _id_ = "paddle";
				im::Text("Paddle vars");
				im::InputFloat("Base speed", &config.paddle.base_speed);
				im::InputFloat("Acceleration", &config.paddle.accel);

				if (im::InputFloat2("Size", model.paddle_size)) {
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
				im::Text("Ball vars");
				im::InputFloat("Base speed", &config.ball.base_speed);
				im::InputFloat("Acceleration", &config.ball.accel);
				im::InputFloat("Max speed", &config.ball.max_speed);
				im::InputFloat("Radius", &config.ball.radius);
			}

			im::Text("Misc");
			if (im::SliderInt("Framerate", &model.framerate, 15, 144)) {
				config.framerate = (unsigned)model.framerate;
			}
			else {
				model.framerate = (int)config.framerate;
			}
		}
	}

	im::Spacing();
	im::Separator();

	if (im::Button("Discard")) {
		config = active_config;
	}
	im::SameLine();
	if (im::Button("Save") && active_config != config) {
		active_config = config;
	}
}

void pong::menu_state::guiStats(game* ctx)
{
	using namespace ImScoped;

	ImGuiIO& io = ImGui::GetIO();
	pos winpos = { io.DisplaySize.x - 10.f, 15.f };
	ImGui::SetNextWindowBgAlpha(0.5f);
	ImGui::SetNextWindowPos(winpos, ImGuiCond_Always, { 1.f, 0 });

	auto constexpr wflags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing;
	Window overlay("Stats", &show_stats, wflags);

	auto p1b = ctx->Player1.getPosition();
	auto p2b = ctx->Player2.getPosition();
	auto bb = ctx->Ball.getPosition();
	auto text = fmt::format("P1: [{:.2f}, {:.2f}]\nP2: [{:.2f}, {:.2f}]\nBall: [{:.2f}, {:.2f}]",
		p1b.x, p1b.y, p2b.x, p2b.y, bb.x, bb.y);

	ImGui::Text("%s:\n%s", "Game state", text.c_str());
}
