#pragma once
#include <utility>
#include <tuple>

#include "common.h"
#include "SFML/Graphics.hpp"

namespace pong
{
	struct paddle : sf::RectangleShape
	{
		using base_t = sf::RectangleShape;

		paddle(playerid pid);
		void update();

		bool ai = false;
		playerid id;
		float velocity;
	};

	struct ball : sf::CircleShape
	{
		using base_t = sf::CircleShape;
		
		ball();
		void update();

		vel velocity;
	};


	bool collision(const sf::Shape& a, const sf::Shape& b);
	bool collision(const sf::Shape& a, const rect& b);
	bool border_collision(const sf::Shape& p);

	void constrain_pos(pos& p);
	
	struct game
	{
		enum mode { singleplayer, multiplayer, aitest };

		game(mode mode_);

		void update(sf::Time delta);
		void draw();

		void serve(dir direction);
		void processEvent(sf::Event& event);

		static void setup(sf::RenderWindow& window);

	//private:

		void resetState();
		void devEvents(const sf::Event& event);
		void updatePlayer(paddle& player);
		void updateBall();
		bool updateScore();

		void resetPos(ball& b);
		void resetPos(paddle& p);

		bool waiting_to_serve() const noexcept;

		bool paused = true;
		mode currentMode;
		sf::Time runTime;

		paddle Player1, Player2;
		ball Ball;
		pair<short> score;
		dir resume_serve_dir = dir::left;
	};

	// shared
	extern sf::Window* game_window;
}
