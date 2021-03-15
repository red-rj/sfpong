#pragma once
#include "common.h"

namespace gvar
{
	inline const pong::rect playarea = { 0.f, 0.f, 1280.f, 1024.f };

	constexpr float paddle_kb_speed = 1;
	constexpr float paddle_max_speed = 30;
	inline const pong::size2d paddle_size = { 25, 150 };
	
	constexpr float ball_speed = 5;
	constexpr float ball_max_speed = 20;
	constexpr float ball_acceleration = 1.1f;
	constexpr float ball_radius = 20;
};
