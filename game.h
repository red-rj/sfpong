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
		
		static void setup(sf::RenderWindow& window);

		void update(sf::Time delta);
		void processEvent(sf::Event& event);
		void draw();

		void serve(dir direction);

		auto get_players() const noexcept -> pair<paddle const&> { return { Player1, Player2 }; }
		auto& get_ball() const noexcept { return Ball; }

		auto get_mode() const noexcept { return currentMode; }

		auto is_paused() const noexcept { return paused; }
		bool toggle_pause() noexcept { return paused = !paused; }
		void unpause() noexcept { paused = false; }

	private:

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
