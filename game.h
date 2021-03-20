#pragma once
#include <utility>
#include "common.h"
#include "game_config.h"
#include "game_ents.h"
#include "SFML/Graphics.hpp"

namespace pong
{
	bool collision(const sf::Shape& a, const sf::Shape& b);
	bool collision(const sf::Shape& a, const rect& b);
	bool collision(const rect& a, const rect& b);

	void constrain_pos(pos& p);


	enum struct gamemode { singleplayer, multiplayer, aitest };
	
	struct game
	{
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
		void change_mode(gamemode m) noexcept;

		void pause() noexcept { paused = true; }
		void unpause() noexcept { paused = false; }
		auto is_paused() const noexcept { return paused; }

		sf::Time ellapsed_time() const {
			return clock.getElapsedTime();
		}
		sf::Time restart_clock() {
			auto elapsed = clock.restart();
			runTime += elapsed;
			return elapsed;
		}

		void newGame(gamemode m) {
			restart();
			change_mode(m);
		}

		void restart();

	private:

		void devEvents(const sf::Event& event);
		void updatePlayer(paddle& player);
		void updateBall();
		bool updateScore();

		void reset(ball& b);
		void reset(paddle& p);

		bool waiting_to_serve() const noexcept;

		bool paused = true;
		gamemode currentMode;
		sf::Clock clock;
		sf::Time runTime;


		//sf::RenderWindow window;
		game_settings* settings;

		paddle Player1{ playerid::one }, Player2{ playerid::two };
		ball Ball;
		pair<short> score;
		dir resume_serve_dir = dir::left;
		pong_area Court;
	};
}
