#pragma once
#include <utility>
#include "common.h"
#include "game_config.h"
#include "SFML/Graphics.hpp"

namespace pong
{
	bool collision(const sf::Shape& a, const sf::Shape& b);
	bool collision(const sf::Shape& a, const rect& b);
	bool collision(const rect& a, const rect& b);

	void constrain_pos(pos& p);

	enum struct gamemode { singleplayer, multiplayer, aitest };
	
	struct arguments_t
	{
		std::string configFile = "game.cfg";
		bool showHelp = false;
	};

	struct player_t
	{
		player_t(playerid pid);

		void update()
		{
			shape.move(velocity);
		}

		sf::RectangleShape shape;
		sf::Vector2f velocity;
		playerid id;
		bool ai = false;
	};

	struct ball_t
	{
		ball_t();

		void update()
		{
			shape.move(velocity);
		}

		sf::CircleShape shape;
		sf::Vector2f velocity;
	};

	/*
	struct pong_area : sf::Drawable, sf::Transformable
	{
		pong_area(size2d area, size2d border_size);

		auto& get_size() const { return size; }

		void set_score(short p1, short p2);
		void set_score(pair<short> s) {
			set_score(s.first, s.second);
		}

		bool border_collision(const rect& bounds) const;

		rect getBounds() const noexcept;

	private:
		void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

		void init_net();

		// court
		sf::RectangleShape top_rect, bottom_rect;
		size2d size;
		// net
		sf::VertexArray net_verts{ sf::Triangles };
		sf::Transform net_transform;

		// score
		sf::Text scoreTxt;
		sf::Font scoreFont;
	};
	*/

	struct background : sf::Drawable, sf::Transformable
	{
		explicit background(size2d area);

		void update_score(int p1, int p2);

		auto size() const { return mySize; }
		void size(size2d value);

		bool border_collision(const rect& bounds) const;


	private:
		size2d mySize, borderSize;

		sf::RectangleShape top, bottom;
		struct {
			sf::VertexArray verts{ sf::Triangles };
			sf::Transform transform;
		} net;
		struct {
			sf::Text text;
			sf::Font font;
		} score;

		void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	};

	class game_instance
	{
	public:
		explicit game_instance(arguments_t params);
		~game_instance();

		sf::RenderWindow window;
		arguments_t params;
		game_settings settings;
		sf::Clock clock;

		void processEvent(sf::Event& event);

		void update();
		void reset();
		void render();

		sf::Time restartClock() {
			auto elapsed = clock.restart();
			runTime += elapsed;
			return elapsed;
		}

		// entities
		player_t player1{ playerid::one }, player2{ playerid::two };
		ball_t ball;
		background bg;

		// controls
		void serve(dir direction);

		// status
		bool paused = true;
		pair<int> score;
		dir serveDir = dir::left;
		sf::Time runTime;
		gamemode mode;
		
		void changeMode(gamemode m) noexcept;

		bool waiting_to_serve() const noexcept;

		void newGame(gamemode m) {
			reset();
			changeMode(m);
			paused = false;
		}

	private:
		void reset(player_t& player);
		void reset(ball_t& ball);

		void updatePlayer(player_t& player);
		void updateBall();
		bool updateScore();

	};

	extern game_instance* G;

	int main();
}
