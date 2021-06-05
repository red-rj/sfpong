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
		std::string configFile;
		bool showHelp;
	};

	struct player_t
	{
		player_t(playerid pid) : id(pid)
		{}

		sf::RectangleShape shape;
		sf::Vector2f velocity, momentum;
		playerid id;
		bool ai = false;
	};

	struct ball_t
	{
		sf::CircleShape shape;
		sf::Vector2f velocity, momentum;
	};

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

	struct background : sf::Drawable, sf::Transformable
	{
		background(size2d area, size2d border_size);

		void update_score(int p1, int p2);

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
		sf::RenderWindow window;
		arguments_t params;
		game_settings settings;
		sf::Clock clock;

		sf::Time restart_clock() {
			auto elapsed = clock.restart();
			runTime += elapsed;
			return elapsed;
		}

		// entities
		player_t player1{ playerid::one }, player2{ playerid::two };
		ball_t ball;

		// status
		bool paused = true;
		pair<int> score;
		dir serveDir = dir::left;
		sf::Time runTime;

		void reset();

		gamemode mode;
		void changeMode(gamemode m) noexcept;

		void newGame(gamemode m) {
			reset();
			changeMode(m);
		}
	};

	extern game_instance* G;


	void setup(arguments_t params);

	int game_main();
}
