#pragma once
#include <utility>
#include "common.h"
#include "SFML/Graphics.hpp"

namespace pong
{
	// TODO: mover entidades para outro header
	// não herdar de shape
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

	enum struct gamemode { singleplayer, multiplayer, aitest };
	
	struct game
	{
		static void setup();

		game(gamemode mode_, game_settings* sett);

		void update();
		void processEvent(sf::Event& event);
		void draw(sf::RenderWindow& window);

		void serve(dir direction);

		auto get_players() const noexcept -> pair<paddle const&> {
			return { Player1, Player2 };
		}
		auto& get_ball() const noexcept { return Ball; }

		auto mode() const noexcept { return currentMode; }
		void mode(gamemode m) noexcept;

		void pause() noexcept { paused = true; }
		void unpause() noexcept { paused = false; }
		auto is_paused() const noexcept { return paused; }

		sf::Time ellapsed_time() const {
			return clock.getElapsedTime();
		}
		sf::Time restart_clock() {
			return runTime += clock.restart();
		}

		void resetState();

	private:

		void devEvents(const sf::Event& event);
		void updatePlayer(paddle& player);
		void updateBall();
		bool updateScore();

		void resetPos(ball& b);
		void resetPos(paddle& p);

		bool waiting_to_serve() const noexcept;

		bool paused = true;
		gamemode currentMode;
		sf::Clock clock;
		sf::Time runTime;

		//sf::RenderWindow window;

		paddle Player1, Player2;
		ball Ball;
		pair<short> score;
		dir resume_serve_dir = dir::left;

		game_settings* settings;
	};
}
