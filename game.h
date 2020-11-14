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
		explicit ball(float radius = 0) : CircleShape(radius)
		{
			setOrigin(radius, radius);
			setFillColor(sf::Color::Red);
		}

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

		enum class mode
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

		bool paused = true;
		uint64_t tickcount = 0;
		mode currentMode;

		//score Score;

		paddle Player1, Player2;
		ball Ball;
		pair<short> score;
	};
}
