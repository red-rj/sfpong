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

	bool rebinding_popup_open() noexcept;

} // menu
}