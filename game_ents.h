#pragma once
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

	private:
		void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

		// court
		sf::RectangleShape top_rect, bottom_rect;
		dashed_line net;
		size2d size;

		// score
		sf::Text score_txt;
	};
}
