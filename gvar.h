#pragma once
#include "common.h"

namespace gvar
{
	constexpr float playarea_width = 1280, playarea_height = 1024;
	//inline const pong::rect playarea = { 0.f, 0.f, playarea_width, playarea_height };

	constexpr float paddle_kb_speed = 1;
	constexpr float paddle_max_speed = 30;
	constexpr float paddle_width = 25, paddle_height = 150;
	inline const pong::size2d paddle_size = { 25, 150 };
	
	constexpr float ball_speed = 5;
	constexpr float ball_max_speed = 20;
	constexpr float ball_acceleration = 1.1f;
	constexpr float ball_radius = 20;
};
