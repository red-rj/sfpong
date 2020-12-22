#pragma once
#include "common.h"
#include <utility>

#include "SFML/Graphics.hpp"

namespace pong
{

	struct paddle : sf::RectangleShape
	{
		bool ai = false;
		playerid id = playerid(-1);
		vel velocity;

		void update();
	};

	struct ball : sf::CircleShape
	{
		void update();

		vel velocity;
	};


	bool collision(const sf::Shape& a, const sf::Shape& b);
	bool collision(const sf::Shape& a, const rect& b);
	bool border_collision(const sf::Shape& p);

	void constrain_pos(pos& p);
	
	struct menu_t;

	struct game
	{
		friend menu_t;
		enum mode { singleplayer, multiplayer };

		game(mode mode_);

		void update();
		void draw();

		void serve(dir direction);
		void processEvent(sf::Event& event);

		static void setup(sf::RenderWindow& window);

	private:

		void resetState();
		void devEvents(const sf::Event& event);
		void updatePlayer(paddle& player);
		void updateBall();

		void resetPos(ball& b);
		void resetPos(paddle& p);

		bool waiting_to_serve() const noexcept;

		bool paused = true;
		uint64_t tickcount = 0;
		mode currentMode;

		paddle Player1, Player2;
		ball Ball;
		pair<short> score;
		dir resume_serve_dir = dir::left;
	};

	// shared
	extern sf::Window* game_window;
}
