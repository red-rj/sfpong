#include "game_ents.h"
#include "gvar.h"


pong::paddle::paddle(playerid pid) : base_t(gvar::paddle_size), id(pid)
{
	setOrigin(0, gvar::paddle_size.y / 2);
}

pong::ball::ball() : base_t(gvar::ball_radius)
{
	setOrigin(gvar::ball_radius, gvar::ball_radius);
	setFillColor(sf::Color::Red);
}

//void pong::ball::update()
//{
//	move(velocity);
//
//	if (border_collision(*this))
//	{
//		velocity.y = -velocity.y;
//	}
//}
//
//void pong::paddle::update()
//{
//	move(0, velocity);
//
//	if (border_collision(*this))
//	{
//		move(0, -velocity);
//		velocity = 0;
//	}
//}

pong::pong_area::pong_area(size2d area, size2d border_size)
	: top_rect(border_size), bottom_rect(border_size)
	, size(area), net({20,20}, 20, area.y)
{
	auto origin = point(border_size.x / 2, 0);

	top_rect.setOrigin(origin);
	top_rect.setPosition(size.x / 2, 5);

	origin.y = border_size.y;
	bottom_rect.setOrigin(origin);
	bottom_rect.setPosition(size.x / 2, size.y - 5);

	net.setRotation(90);
	net.setPosition(size.x / 2, 20);

	scoreTxt.setPosition(size.x / 2 - 100, border_size.y + 5);
	scoreTxt.setCharacterSize(55);

	scoreFont.loadFromFile(files::mono_tff);

	scoreTxt.setFont(scoreFont);

}

void pong::pong_area::setup_score(sf::Font const& font, unsigned charSize)
{
	scoreTxt.setFont(font);
	scoreTxt.setCharacterSize(charSize);
}

void pong::pong_area::set_score(short p1, short p2)
{
	scoreTxt.setString(fmt::format("{}    {}", p1, p2));
}

void pong::pong_area::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();
	target.draw(top_rect, states);
	target.draw(bottom_rect, states);
	target.draw(net, states);
	target.draw(scoreTxt, states);
}

pong::dashed_line::dashed_line(size2d pieceSize, float gapLen, float maxLen)
{
	point current;
	bool gap{};

	// creat verts alongside the X axis
	for (int count = 1; (pieceSize.x + gapLen) * count < maxLen; gap = !gap)
	{
		if (gap) {
			current.x += pieceSize.x + gapLen;
		}
		else {
			// rect from 2 triangles
			sf::Vertex v = current;
			// t1
			verts.append(v);
			v.position.x += pieceSize.x;
			verts.append(v);
			v.position.y += pieceSize.y;
			verts.append(v);
			// t2
			verts.append(v);
			v.position.x -= pieceSize.x;
			verts.append(v);
			v.position.y -= pieceSize.y;
			verts.append(v);

			count++;
		}
	}
}


void pong::dashed_line::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();
	target.draw(verts, states);
}
