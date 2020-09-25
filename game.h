#pragma once
#include "common.h"
#include <utility>
#include <filesystem>

#include "SFML/Graphics.hpp"

namespace pong
{
	template<typename T, typename E = T>
	using pair = std::pair<T, E>;

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
	
	struct score : public sf::Drawable
	{
		score() = default;

		score(rect playarea, std::filesystem::path const& fontfile, unsigned size)
		{
			create(playarea, fontfile, size);
		}

		void create(rect playarea, std::filesystem::path const& fontfile, unsigned size)
		{
			font.loadFromFile(fontfile.string());
			text = sf::Text("", font, size);
			text.setPosition(playarea.width / 2 - 100, 50);
			update();
		}

		void update();

		// increment score
		void add(short p1, short p2) {
			val.first += p1;
			val.second += p2;
			update();
		}
		void add(pair<short> value) {
			add(value.first, value.second);
		}

		void set(pair<short> newscore) noexcept { val = newscore; }
		constexpr auto& get() const { return val; }


	private:
		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			target.draw(text, states);
		}

		pair<short> val{};
		sf::Text text;
		sf::Font font;
	};

	struct court : public sf::Drawable
	{
		court() = default;

		court(rect playarea, float heigth, sf::Vector2f margin) : m_playarea(playarea), m_hOffset(margin.y+heigth)
		{
			top = bottom = sf::RectangleShape({ playarea.width - margin.x * 2, heigth });
			top.setPosition(margin);
			bottom.setOrigin(0, heigth);
			bottom.setPosition(margin + sf::Vector2f(0, playarea.height - margin.y * 2));
			net.setPosition(playarea.width / 2, 20);
		}

		auto getCenter() const noexcept {
			auto center = m_playarea;
			center.top += m_hOffset;
			center.height -= m_hOffset * 2;
			return center;
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

		rect m_playarea;
		float m_hOffset;
	};


	struct paddle : sf::RectangleShape
	{
		bool ai = false;
		playerid id = playerid::invalid;
		vel velocity;

		void update();
	};

	struct ball : sf::CircleShape
	{
		explicit ball(float radius = 0) : CircleShape(radius)
		{
			setOrigin(radius, radius);
			setFillColor(sf::Color::Red);
		}

		void update();

		vel velocity;
	};


	bool collision(const sf::Shape& a, const sf::Shape& b);
	bool collision(const sf::Shape& a, const rect& b);
	bool border_collision(const sf::Shape& p);

	void constrain_pos(pos& p);
	
	struct menu_state;

	struct game
	{
		friend menu_state;

		game(sf::RenderWindow& window);

		void serve(dir direction);
		void update(sf::RenderWindow& window);

	private:

		void resetState();
		void pollEvents(sf::RenderWindow& window);
		void updatePlayer(paddle& player);
		void updateBall();
		void generateLevel(rect area);


		bool paused = true;
		uint64_t tickcount = 0;

		score Score;

		paddle Player1, Player2;
		ball Ball;
	};


}
