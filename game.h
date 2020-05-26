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
			setOrigin(radius, radius);
			setFillColor(sf::Color::Red);
		}
	};


    bool check_collision(const sf::Shape* a, const sf::Shape* b);
	//---

	struct menu_state
	{
		bool show_options = false, rebinding = false;
		bool show_stats = false;
	};

	struct game_state
	{
		bool paused = true;
		uint64_t tickcount = 0;
		sf::Event lastEvent;
		std::pair<short, short> score;
		sf::FloatRect playable_area;
	};

	struct cmdline_options
	{
		// reserved
	};

	int run_game(sf::RenderWindow* win, config_t* cfg, cmdline_options const& cmdline = {});
	
	enum class dir { left, right };

}
