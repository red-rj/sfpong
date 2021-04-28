#pragma once
#include <memory>
#include "common.h"
#include "SFML/Graphics.hpp"

namespace pong
{
	// TODO: não herdar de shape
	struct paddle : sf::RectangleShape
	{
		using base_type = sf::RectangleShape;

		paddle(playerid pid);
		void move();
		using base_type::move;

		bool ai = false;
		playerid id;
		float velocity;
	};

	struct ball : sf::CircleShape
	{
		using base_type = sf::CircleShape;

		ball();
		void move();
		using base_type::move;

		vel velocity;
	};

	//---

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
}
