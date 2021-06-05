#include "gvar.h"


//pong::paddle::paddle(playerid pid) : base_type(gvar::paddle_size), id(pid)
//{
//	setOrigin(0, gvar::paddle_size.y / 2);
//
//	//const sf::Int32 lightgray = 0x45 << 24 | 0x45 << 16 | 0x45 << 8 | 127;
//	setOutlineColor(sf::Color::Black);
//	setOutlineThickness(1.5f);
//}
//
//pong::ball::ball() : base_type(gvar::ball_radius)
//{
//	setOrigin(gvar::ball_radius, gvar::ball_radius);
//	setFillColor(sf::Color::Red);
//}

auto pong::pong_area::getBounds() const noexcept -> rect
{
	return getTransform().transformRect(rect({}, size));
}

bool pong::pong_area::border_collision(const rect& bounds) const
{
	auto top = getTransform().transformRect(top_rect.getGlobalBounds());
	auto bottom = getTransform().transformRect(bottom_rect.getGlobalBounds());
	return bounds.intersects(top) or bounds.intersects(bottom);
}