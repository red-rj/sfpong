#pragma once
#include "game_config.h"
#include <array>

namespace sf { class Window; }

namespace pong
{
	struct game;

	struct menu_state
	{
		void draw(game& ctx, sf::Window& window);

		void init();

		bool rebinding = false;

	private:
		struct {
			bool options = false;
			bool game_stats = false;

			bool imgui_demo = false; 
		//---
		} show;

		void guiOptions(game& ctx);
		void guiStats(game& ctx);

		struct input_t
		{
			std::array<player_input_cfg, 2> settings;
			player_input_cfg& player1 = settings[0];
			player_input_cfg& player2 = settings[1];

		} input;
	};

}