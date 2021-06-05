#pragma once
#include "game_config.h"
#include <array>

namespace sf {
	class Event;
	class Window;
}

namespace pong
{
namespace menu {

	void init();

	void update();
	void processEvent(sf::Event& event);

	// window ids
	namespace win { enum Id {
		options,
		game_stats,
		about,
		imgui_demo,
		imgui_about,
		rebiding_popup,

		Count
	};}


	bool is_open(win::Id id) noexcept;

} // menu
}