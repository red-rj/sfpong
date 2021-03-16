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

	struct dashed_line : sf::Drawable, sf::Transformable
	{
		dashed_line(size2d pieceSize, float gapLen, float maxLen);
		//dashed_line(std::vector<float> const& pattern, float thickness);

	private:
		void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

		sf::VertexArray verts{ sf::Triangles };
	};

	struct pong_area : sf::Drawable, sf::Transformable
	{
		pong_area(size2d area, size2d border_size);

		auto& get_size() const { return size; }

		void setup_score(sf::Font const& font, unsigned charSize);
		void set_score(short p1, short p2);

		auto& topBorder() const { return top_rect; }
		auto& bottomBorder() const { return bottom_rect; }

		bool border_collision(const rect& bounds) const {
			return bounds.intersects(top_rect.getGlobalBounds()) 
				or bounds.intersects(bottom_rect.getGlobalBounds());
		}

	private:
		void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

		// court
		sf::RectangleShape top_rect, bottom_rect;
		dashed_line net;
		size2d size;

		// score
		sf::Text scoreTxt;
		sf::Font scoreFont;
		//std::shared_ptr<sf::Font> scoreFont;
	};
}
