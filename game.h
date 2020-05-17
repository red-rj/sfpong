#pragma once
#include <utility>
#include <SFML/Graphics.hpp>

#include "game_config.h"

namespace pong
{

	struct net_shape : public sf::Drawable, public sf::Transformable
	{
        net_shape(float pieceSize_ = 20.f, int pieceCount_ = 25);

	private:
		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			states.transform *= getTransform();
			target.draw(m_net, states);
		}

		
		float m_piece_size;
		int m_piece_count;
		sf::VertexArray m_net{ sf::Quads };
	};
	
	
	struct game_entity
	{
		sf::Vector2f velocity = {};
	};

	struct paddle : sf::RectangleShape, game_entity
	{
		bool ai = false;
		int id = -1;
	};

	struct ball : sf::CircleShape, game_entity
	{
		explicit ball(float radius = 0) : CircleShape(radius) {
			setOrigin(radius / 2, radius / 2);
			setFillColor(sf::Color::Red);
		}
	};


    bool check_collision(const sf::Shape* a, const sf::Shape* b);
	//---

	struct game
	{
		game(sf::RenderWindow& win, config_t& cfg);

		int run();
		void pollEvents();

		void drawGame();
		void drawGui();
		void drawScore();

		void resetState();

		void updatePlayers();
		void updateBall();

		enum class dir
		{
			left, right
		};

		void serve(dir direction);

		void swap(game& other) noexcept;

	private:
		sf::RenderWindow& window;

		bool paused = true;
		sf::Clock deltaClock;
		sf::FloatRect playable_area;
		uint64_t tickcount = 0;

		// score
		sf::Font ftScore;
		sf::Text txtScore;
		std::pair<short, short> score;

		// court
		sf::RectangleShape topBorder, bottomBorder;
		net_shape net;

		config_t& config;
		paddle p1, p2;
		ball ball;
	};

}
