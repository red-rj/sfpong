#pragma once
#include "game_config.h"

namespace sf { class Window; }

namespace pong
{
	struct game;

	struct menu_state
	{
		void draw(game* ctx, sf::Window* window);

		// options
		bool show_options = false, rebinding = false;
		config_t config;
		bool show_stats = false;

	private:
		void guiOptions(game* ctx);
		void guiStats(game* ctx);
	};

}