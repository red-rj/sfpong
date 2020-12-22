#pragma once
#include <utility>
#include <tuple>

#include "common.h"
#include "SFML/Graphics.hpp"

namespace pong
{
	// up, down, fast
	//using input_record = std::tuple<bool, bool, bool>;

	// Y offset, fast
	//using input_result = std::tuple<float, bool>;


	struct paddle : sf::RectangleShape
	{
		void update();

		bool ai = false;
		playerid id = playerid(-1);
		vel velocity;
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

		void update(sf::Time delta);
		void draw();

		void serve(dir direction);
		void processEvent(sf::Event& event);

		static void setup(sf::RenderWindow& window);

	private:

		void resetState();
		void devEvents(const sf::Event& event);
		float aiMove(paddle const& pad);
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
