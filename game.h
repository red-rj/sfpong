#pragma once
#include <utility>
#include <SFML/Graphics.hpp>
#include "game_config.h"

namespace pong
{
	// game entities
	struct net_shape : public sf::Drawable, public sf::Transformable
	{
        explicit net_shape(float pieceSize_ = 20.f, int pieceCount_ = 25);

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

	// TODO: crasha
	/*
	struct score : public sf::Drawable
	{
		score() = default;

		score(sf::FloatRect playarea, std::filesystem::path const& fontfile, unsigned size)
			: text("", font, size)
		{
			font.loadFromFile(fontfile.string());
			text.setPosition(playarea.width / 2 - 100, 50);
			update(0, 0);
		}

		void update(short p1, short p2) {
			text.setString(fmt::format("{}    {}", p1, p2));
		}

	private:
		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			target.draw(text, states);
		}

		sf::Text text;
		sf::Font font;
	};
	*/

	struct court : public sf::Drawable
	{
		court() = default;

		court(sf::FloatRect playarea, float heigth, sf::Vector2f margin)
		{
			top = bottom = sf::RectangleShape({ playarea.width - margin.x * 2, heigth });
			top.setPosition(margin);
			bottom.setOrigin(0, heigth);
			bottom.setPosition(margin + sf::Vector2f(0, playarea.height - margin.y * 2));
			net.setPosition(playarea.width / 2, 20);
		}

		sf::RectangleShape top, bottom;
		net_shape net;

	private:
		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			target.draw(top, states);
			target.draw(bottom, states);
			target.draw(net, states);
		}

	};

	
	struct paddle : sf::RectangleShape
	{
		bool ai = false;
		int id = -1;

		sf::Vector2f velocity = {};
	};

	struct ball : sf::CircleShape
	{
		explicit ball(float radius = 0) : CircleShape(radius) {
			setOrigin(radius, radius);
			setFillColor(sf::Color::Red);
		}

		sf::Vector2f velocity = {};
	};


    bool check_collision(const sf::Shape* a, const sf::Shape* b);
	//---

	struct menu_state
	{
		// options
		bool show_options = false, rebinding = false;
		config_t tmp_config;
		bool configDirty() noexcept;

		bool show_stats = false;
	};

	struct game_state
	{
		bool paused = true;
		std::pair<short, short> score;
		sf::FloatRect playable_area;
		config_t config;

		sf::Event lastEvent;
		sf::RenderWindow window;
	};

	
	
	enum class dir { left, right };

}
