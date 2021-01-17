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

	void init();

	void update(game& ctx, sf::Window& window);
	void processEvent(sf::Event& event);

	bool rebinding_popup_open() noexcept;

} // menu
}