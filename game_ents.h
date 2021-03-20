#pragma once
#include <memory>
#include "common.h"
#include "SFML/Graphics.hpp"

namespace pong
{
	// TODO: não herdar de shape
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

	//---

	struct pong_area : sf::Drawable, sf::Transformable
	{
		pong_area(size2d area, size2d border_size);

		auto& get_size() const { return size; }

		void setup_score(sf::Font const& font, unsigned charSize);
		void set_score(short p1, short p2);

		bool border_collision(const rect& bounds) const {
			return bounds.intersects(top_rect.getGlobalBounds()) 
				or bounds.intersects(bottom_rect.getGlobalBounds());
		}

	private:
		void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

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
