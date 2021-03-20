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

void pong::ball::update()
{
	move(velocity);

	/*if (border_collision(*this))
	{
		velocity.y = -velocity.y;
	}*/
}

void pong::paddle::update()
{
	move(0, velocity);

	/*if (border_collision(*this))
	{
		move(0, -velocity);
		velocity = 0;
	}*/
}

pong::pong_area::pong_area(size2d area, size2d border_size)
	: top_rect(border_size), bottom_rect(border_size)
	, size(area) //, net({20,20}, 20, area.y)
{
	auto origin = point(border_size.x / 2, 0);

	top_rect.setOrigin(origin);
	top_rect.setPosition(size.x / 2, 5);

	origin.y = border_size.y;
	bottom_rect.setOrigin(origin);
	bottom_rect.setPosition(size.x / 2, size.y - 5);

	//net.setRotation(90);
	//net.setPosition(size.x / 2, 20);

	// build net alongside the X axis
	const size2d pieceSize = { 20,20 };
	const float gapLen = 20;
	point current;
	bool gap{};

	for (int count = 1; (pieceSize.x + gapLen) * count < size.y; gap = !gap)
	{
		if (gap) {
			current.x += pieceSize.x + gapLen;
		}
		else {
			sf::Vertex v{ current, sf::Color::Magenta };

			// triangle1
			net_verts.append(v);
			v.position = current + pos(pieceSize.x, 0);
			net_verts.append(v);
			v.position = current + pieceSize;
			net_verts.append(v);
			// triangle2
			v.position = current;
			net_verts.append(v);
			v.position = current + pos(0, pieceSize.y);
			net_verts.append(v);
			v.position = current + pieceSize;
			net_verts.append(v);

			count++;
		}
	}

	// nessa ordem
	net_transform.translate(size.x / 2, 20).rotate(90);

	scoreFont.loadFromFile(files::mono_tff);
	scoreTxt.setPosition(size.x / 2 - 100, border_size.y + 5);
	scoreTxt.setCharacterSize(55);
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
	states.transform *= getTransform() * net_transform;
	target.draw(net_verts, states);
	
	states.transform *= net_transform.getInverse() * getTransform();
	target.draw(top_rect, states);
	target.draw(bottom_rect, states);
	target.draw(scoreTxt, states);
}
