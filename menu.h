#pragma once
#include "game_config.h"
#include <array>

namespace sf {
	class Window;
	class Event;
}

namespace pong
{
	struct game;

	struct menu_t
	{
		void update(game& ctx);
		void init();

		void processEvent(sf::Event& event);


		bool rebinding = false;
		
	private:
		// windows
		void optionsWin(game& ctx);
		void gameStatsWin(game& ctx);
		void aboutSfPongWin();

		// ui
		void controlsUi();
		int joystickCombobox(const char* label, int current_joyid);

		// show flags
		struct {
			bool options = false;
			bool game_stats = false;
			bool about = false;

			bool imgui_demo = false, imgui_about = false;
		} show;

		// copia de trabalho das configs de input
		std::array<player_input_cfg, 2> input_settings;
	};

	// shared
	extern menu_t game_menu;
}