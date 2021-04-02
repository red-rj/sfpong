#pragma once
#include "game_config.h"
#include <array>

namespace sf {
	class Event;
	class Window;
}

namespace pong
{
	struct game;

namespace menu {

	void init(game_settings* gs);

	void update(game& ctx, sf::Window& window);
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

	inline bool rebinding_popup_open() noexcept {
		return is_open(win::rebiding_popup);
	}

} // menu
}