#include "game_ents.h"
#include "gvar.h"


pong::paddle::paddle(playerid pid) : base_type(gvar::paddle_size), id(pid)
{
	setOrigin(0, gvar::paddle_size.y / 2);

	//const sf::Int32 lightgray = 0x45 << 24 | 0x45 << 16 | 0x45 << 8 | 127;
	setOutlineColor(sf::Color::Black);
	setOutlineThickness(1.5f);
}

pong::ball::ball() : base_type(gvar::ball_radius)
{
	setOrigin(gvar::ball_radius, gvar::ball_radius);
	setFillColor(sf::Color::Red);
}


void pong::ball::move()
{
	move(velocity);
}

void pong::paddle::move()
{
	move(0, velocity);
}

pong::pong_area::pong_area(size2d area, size2d border_size)
	: top_rect(border_size), bottom_rect(border_size)
	, size(area)
{
	auto origin = point(border_size.x / 2, 0);

	top_rect.setOrigin(origin);
	top_rect.setPosition(size.x / 2, 0);

	origin.y = border_size.y;
	bottom_rect.setOrigin(origin);
	bottom_rect.setPosition(size.x / 2, size.y);

	init_net();

	scoreFont.loadFromFile(files::mono_tff);
	scoreTxt.setPosition(size.x / 2 - 100, border_size.y);
	scoreTxt.setCharacterSize(55);
	scoreTxt.setFont(scoreFont);

	// margin  
	top_rect.move(0, 6);
	bottom_rect.move(0, -6);
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
	states.transform = getTransform() * net_transform;
	target.draw(net_verts, states);
	
	states.transform = sf::Transform::Identity * getTransform();
	target.draw(top_rect, states);
	target.draw(bottom_rect, states);
	target.draw(scoreTxt, states);
}

void pong::pong_area::init_net()
{
	// build net alongside the X axis
	const size2d pieceSize = { 20,20 };
	const float gapLen = 20;
	point current;
	bool gap{};

	for (int count = 1; (pieceSize.x + gapLen) * count < size.y; gap = !gap)
	{
		if (gap) {
			current.x += gapLen;
		}
		else {
			sf::Vertex v{ current, sf::Color(200,200,200) };

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

			current.x += pieceSize.x;
			count++;
		}
	}

	// nessa ordem
	net_transform.translate(size.x / 2, 20).rotate(90);
}

auto pong::pong_area::getBounds() const noexcept -> rect
{
	auto top = top_rect.getGlobalBounds();
	auto bottom = bottom_rect.getGlobalBounds();

	auto r = rect(getPosition(), size);
	r.top = top.top + top.height;
	r.height = bottom.top;

	return getTransform().transformRect(r);
}

bool pong::pong_area::border_collision(const rect& bounds) const
{
	auto top = getTransform().transformRect(top_rect.getGlobalBounds());
	auto bottom = getTransform().transformRect(bottom_rect.getGlobalBounds());
	return bounds.intersects(top) or bounds.intersects(bottom);
}