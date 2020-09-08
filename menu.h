#pragma once
#include "game_config.h"
#include <array>

namespace sf { class Window; }

namespace pong
{
	struct game;

	struct menu_state
	{
		void draw(game* ctx, sf::Window* window);

		void init();

		// options
		bool show_options = false;
		bool rebinding = false;
		bool show_stats = false;


	private:
		void guiOptions(game* ctx);
		void guiStats(game* ctx);

		//std::array<player_input_cfg, 2> input_settings;

		union input_u
		{
			std::array<player_input_cfg, 2> settings;
			struct
			{
				player_input_cfg player1, player2;
			};

		} input;
	};

}