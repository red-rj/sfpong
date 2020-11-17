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

		enum mode
		{
			singleplayer, multiplayer
		};

		void serve(dir direction);
		void update();
		void processEvent(sf::Event& event);

		static void setup(sf::RenderWindow& window);

		game(mode mode_);
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
	};
}
