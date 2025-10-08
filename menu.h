#pragma once
#include "game_config.h"
#include <array>

namespace sf {
	class Event;
	class Window;
	class Time;
}

namespace pong {
	class game;
	class game_settings;
};

class ImFont;


class themenu
{
public:
	themenu(pong::game& gameref, float fontsize);
	themenu(const themenu&) = delete;

	~themenu();

	void init();

	enum menuid {
		ui_options,
		ui_game_stats,
		ui_about,
		ui_imgui_demo,
		ui_imgui_about,
		ui_rebiding_popup,

		ui_count
	};

	bool isOpen(menuid mid);

	void update(sf::Time delta);
	void processEvent(sf::Event& event);
	void render();

private:

	void optionsUi();
	void aboutUi();
	void gameStatsUi();
	void controlsUi();

	pong::game& game;
	pong::game_settings work_settings;
	std::array<bool, ui_count> visible;
	enum {
		font_normal,
		font_larger,
		font_monospace,
		font_title,

		font_count
	};
	std::array<ImFont*, font_count> fonts;
	float font_size;
};

