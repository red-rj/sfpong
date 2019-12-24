#pragma once

#include <utility>
#include "SFML/Graphics.hpp"

namespace red::pong
{
	struct score : public sf::Drawable, public sf::Transformable
	{
	 	explicit score(const sf::Text& text_, short padding = 4) : m_text(text_), m_padding(padding)
		{
			format_score_txt();
		}

		
		const auto get_scores() const
		{
			return std::make_pair(m_p1_score, m_p2_score);
		}

		void set(short p1, short p2)
		{
			m_p1_score = p1; m_p2_score = p2;
			format_score_txt();
		}

		void add(short p1, short p2)
		{
			m_p1_score += p1; m_p2_score += p2;
			format_score_txt();
		}

		short get_padding() const { return m_padding; }
		void set_padding(short p);

		const sf::Text& get_text() const { return m_text; }
		void set_text(const sf::Text& txt) 
		{
			m_text = txt;
			format_score_txt(); 
		}

	private:
		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			states.transform *= getTransform();
			target.draw(m_text, states);
		}

		void format_score_txt();

		sf::Text m_text;
		short m_padding = 4, m_p1_score = 0, m_p2_score = 0;
	};

	struct net_shape : public sf::Drawable, public sf::Transformable
	{
        net_shape(float pieceSize_ = 20.f, int pieceCount_ = 25);

	private:
		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			states.transform *= getTransform();
			target.draw(m_net, states);
		}

		
		float m_piece_size = 20.f;
		int m_piece_count = 50;
		sf::VertexArray m_net{ sf::Quads };
	};

	struct court : public sf::Drawable
	{
		court(const sf::Vector2f& topSize_, const sf::Vector2f& bottomSize_) 
			: top(topSize_), bottom(bottomSize_)
		{
		}

        explicit court(const sf::Vector2f& size_) : court(size_, size_) {}

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
	
	
	struct game_objs;
	
	struct game_entity
	{
		virtual void update(game_objs& go) = 0;

		virtual ~game_entity() = default;
		
		sf::Vector2f velocity = {};
	};

	struct paddle : sf::RectangleShape, game_entity
	{

		void update(game_objs& go) override;

		sf::Keyboard::Key up_key, down_key, fast_key;


		bool ai = false;
        float accel = 1.f;
        float base_speed = 500.f;

	private:
	};

	struct ball : sf::CircleShape, game_entity
	{
		explicit ball(float radius) : CircleShape(radius) {
			setOrigin(radius / 2, radius / 2);
			setFillColor(sf::Color::Red);
		}

		virtual void update(game_objs& go) override;

        float max_speed, serve_speed, accel;
	};


	struct game_objs
	{
		std::pair<paddle*, paddle*> players = {};
		ball*	ball = nullptr;
		score*	score = nullptr;
		court*	court = nullptr;
		sf::FloatRect* playable_bounds = nullptr;
		uint64_t tickcount = 0;
	};

    bool check_collision(const sf::Shape* a, const sf::Shape* b);
    inline bool check_collision(const sf::Shape* a, const court* court) {
        return check_collision(a, &court->top) || check_collision(a, &court->bottom);
    }
}
