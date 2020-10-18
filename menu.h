#pragma once
#include "game_config.h"
#include <array>

namespace sf { class Window; }

namespace pong
{
	struct game;

	struct menu_t
	{
		void draw(game& ctx, sf::Window& window);
		void init();

		bool rebinding = false;
		
		void refresh_joystick_list() const;

	private:
		// windows
		void guiOptions(game& ctx);
		void guiStats(game& ctx);
		void aboutSfPong();

		// ui
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

	extern menu_t game_menu;
}