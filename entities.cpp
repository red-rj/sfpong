#include "entities.h"
#include <string>
#include <algorithm>
#include <random>

namespace
{
	auto rnd_dev = std::random_device();
	auto rnd_eng = std::default_random_engine(rnd_dev());
}

int random_num(int min, int max)
{
	static auto dist = std::uniform_int_distribution(min, max);
	return dist(rnd_eng);
}

bool coin_flip()
{
	static std::bernoulli_distribution dist;
	return dist(rnd_eng);
}




void red::score::set_padding(short p)
{
	if (p != m_padding)
	{
		m_padding = p;
		format_score_txt();
	}
}

void red::score::format_score_txt()
{
	auto str = std::to_string(m_p1_score) + std::string(m_padding, ' ') + std::to_string(m_p2_score);
	m_text.setString(str);
}

void red::paddle::update(game_objs& go)
{
	if (m_ai) 
	{
		auto reaction = random_num(20, 250);
		auto ballbounds = go.ball->getGlobalBounds();
		auto ballPos = go.ball->getPosition();
		auto paddlePos = getPosition();
		auto d = (paddlePos - ballPos).y;
		auto diff = d < 0 ? -d + reaction : d - reaction;

		auto ySpeed = diff / 100.0f;

		if (diff > ballbounds.height)
		{
			if (ballPos.y < paddlePos.y)
			{
				velocity.y = std::clamp(-ySpeed, -MAX_VELOCITY * 2, 0.f);
			}
			else if (ballPos.y > paddlePos.y)
			{
				velocity.y = std::clamp(ySpeed, 0.f, MAX_VELOCITY * 2);
			}
		}
		else
		{
			velocity.y = 0;
		}
	}
	else // player
	{
		const auto vy = velocity.y;
		bool moving = false;

		if (sf::Keyboard::isKeyPressed(up_key))
		{
			velocity.y = std::clamp(vy - ACCEL_FACTOR, -MAX_VELOCITY, 0.f);
			moving = true;
		}
		else if (sf::Keyboard::isKeyPressed(down_key))
		{
			velocity.y = std::clamp(vy + ACCEL_FACTOR, 0.f, MAX_VELOCITY);
			moving = true;
		}
		else
		{
			velocity.y /= 2;
		}
		
		if (sf::Keyboard::isKeyPressed(fast_key) && moving)
		{
			velocity.y *= 2;
		}

	}

	auto bounds = getGlobalBounds();

	if (bounds.intersects(go.court->top.getGlobalBounds()))
	{
		velocity.y = 0;
		move(0, ACCEL_FACTOR);
	}
	else if (bounds.intersects(go.court->bottom.getGlobalBounds()))
	{
		velocity.y = 0;
		move(0, -ACCEL_FACTOR);
	}

	move(velocity);
}

void red::ball::update(game_objs& go)
{
	paddle* paddle = nullptr;
	auto bounds = getGlobalBounds();

	if (bounds.intersects(go.players.first->getGlobalBounds()))
	{
		paddle = go.players.first;
	}
	else if (bounds.intersects(go.players.second->getGlobalBounds()))
	{
		paddle = go.players.second;
	}
	
	if (bounds.intersects(go.court->top.getGlobalBounds()) || bounds.intersects(go.court->bottom.getGlobalBounds()))
	{
		velocity.y = -velocity.y;
	}

	if (paddle)
	{
		const auto vX = velocity.x * (1.f + paddle::ACCEL_FACTOR);
		const auto vY = paddle->velocity.y != 0 ? paddle->velocity.y * 0.75f : velocity.y;

		velocity = {
			-std::clamp(vX, -MAX_VELOCITY, MAX_VELOCITY), 
			 std::clamp(vY, -MAX_VELOCITY, MAX_VELOCITY)
		};
		
		const auto desloc = velocity.x < 0 ? -paddle->getSize().x : paddle->getSize().x;

		// mover bola até não intersectar mais no paddle, isso evita q
		// ela fique presa
		while (paddle->getGlobalBounds().intersects(getGlobalBounds())) {
			move(desloc * 0.1f, 0);
		}
	}

	if (!go.playable_bounds->intersects(bounds))
	{
		// ponto!
		if (getPosition().x < 0)
		{
			// indo p/ dir, ponto p1
			go.score->add_scores(1, 0);
			velocity = { -SERVE_SPEED, 0 };
		}
		else
		{
			// indo p/ esq, ponto p2
			go.score->add_scores(0, 1);
			velocity = { SERVE_SPEED, 0 };
		}

		setPosition(go.playable_bounds->width / 2, go.playable_bounds->height / 2);
	}

	move(velocity);
}

void red::net_shape::setup()
{
	m_net.clear();

	auto piece_size = sf::Vector2f(m_piece_size, m_piece_size);
	auto current_pt = sf::Vector2f();

	int p = 0;
	for (bool gap = false; p < m_piece_count; gap = !gap)
	{
		if (gap)
		{
			current_pt.y += piece_size.y * 2;
			continue;
		}

		m_net.append({ current_pt });
		m_net.append(sf::Vector2f{ current_pt.x + piece_size.x, current_pt.y });
		m_net.append({ piece_size + current_pt });
		m_net.append(sf::Vector2f{ current_pt.x, current_pt.y + piece_size.y });
		p++;
	}

	setOrigin(piece_size.x / 2, 0);
}
