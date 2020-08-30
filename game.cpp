#include <string>
#include <algorithm>
#include <random>

#include <fmt/format.h>
#include "spdlog/sinks/stdout_color_sinks.h"
#include "imgui_ext.h"
#include <imgui-SFML.h>

#include "common.h"
#include "game.h"
#include "rng.h"
#include "menu.h"


using namespace std::literals;
namespace fs = std::filesystem;


namespace
{
	auto rnd_dev = std::random_device();
	auto rnd_eng = std::default_random_engine(rnd_dev());

	pong::menu_state Menu;
	sf::FloatRect Playarea;
	sf::FloatRect Borders[2];

	auto create_level(sf::FloatRect area)
	{
		Playarea = area;
		auto lvl = pong::court(Playarea, 25, { 15, 20 });
		Borders[0] = lvl.top.getGlobalBounds();
		Borders[1] = lvl.bottom.getGlobalBounds();

		return lvl;
	}
}

int pong::random_num(int min, int max)
{
	auto dist = std::uniform_int_distribution(min, max);
	return dist(rnd_eng);
}

bool pong::coin_flip()
{
	static std::bernoulli_distribution dist;
	return dist(rnd_eng);
}

pong::net_shape::net_shape(float pieceSize_, int pieceCount_) : m_piece_size(pieceSize_), m_piece_count(pieceCount_)
{
	m_net.clear();

	auto piece_size = sf::Vector2f(m_piece_size, m_piece_size);
	auto current_pt = sf::Vector2f();

	for (int gap = false, p = 0; p < m_piece_count; gap = !gap)
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

bool pong::check_collision(const sf::Shape &a, const sf::Shape &b)
{
    return a.getGlobalBounds().intersects(b.getGlobalBounds());
}


bool pong::border_collision(const sf::Shape& p)
{
	bool result = false;
	for (auto const& bord : Borders)
	{
		if (result = bord.intersects(p.getGlobalBounds()))
			break;
	}
	return result;
}


void pong::score::update()
{
	text.setString(fmt::format("{}    {}", val.first, val.second));
}

using namespace pong;

pong::game::game(config_t cfg, sf::RenderWindow& window) : Config(std::move(cfg))
{
	// pong court
	auto area = sf::FloatRect{ {0.f,0.f}, static_cast<sf::Vector2f>(window.getSize()) };
	Court = create_level(area);

	Menu.config = Config;

	Score.create(area, R"(C:\Windows\Fonts\LiberationMono-Regular.ttf)", 55);

	resetState();
}

void game::pollEvents(sf::RenderWindow& window)
{
	sf::Event event;
	while (window.pollEvent(event)) {

		ImGui::SFML::ProcessEvent(event);

		switch (event.type)
		{
		case sf::Event::Closed:
			window.close();
			break;

		case sf::Event::KeyReleased:
		{
			if (Menu.rebinding)
				break;

			switch (event.key.code)
			{
			case sf::Keyboard::F1:
				Player1.ai = !Player1.ai;
				break;
			case sf::Keyboard::F2:
				Player2.ai = !Player2.ai;
				break;
			case sf::Keyboard::Enter:
				serve(dir::left);
				gamelog()->info("manual ball serve");
				break;
			case sf::Keyboard::F12:
				resetState();
				break;
			case sf::Keyboard::Escape:
				paused = !paused;
				// imgui deve capturar input só com o jogo pausado
				auto& io = ImGui::GetIO();
				io.WantCaptureKeyboard = paused;
				io.WantCaptureMouse = paused;
				break;
			}
		} break;

		case sf::Event::Resized:
		{
			sf::FloatRect visibleArea{ 0, 0, (float)event.size.width, (float)event.size.height };
			window.setView(sf::View(visibleArea));
			Court = create_level(visibleArea);
		} break;

		}
	}

}

void pong::game::update(sf::RenderWindow& window)
{
	window.clear();
	window.draw(Court);
	window.draw(Score);
	window.draw(Ball);
	window.draw(Player1);
	window.draw(Player2);

	if (!paused)
	{
		updateBall();
		updatePlayer(Player1);
		updatePlayer(Player2);

		if (tickcount % 30 == 0) {
			// check score
			if (!Playarea.intersects(Ball.getGlobalBounds()))
			{
				// ponto!
				if (Ball.getPosition().x < 0)
				{
					// indo p/ direita, ponto player 1
					Score.add(1, 0);
					serve(dir::left);
				}
				else
				{
					// indo p/ esquerda, ponto player 2
					Score.add(0, 1);
					serve(dir::right);
				}
			}
		}
		tickcount++;
	}

	Menu.draw(this, &window);
}

void pong::game::resetState()
{
	auto area = Playarea;

	Player1.id = 0;
	Player1.setSize(Config.paddle.size);
	Player1.setOrigin(Config.paddle.size.x / 2, Config.paddle.size.y / 2);
	Player1.setPosition(20, area.height / 2);
	Player1.pcfg = &Config;

	Player2 = Player1;
	Player2.ai = true;
	Player2.id = 1;
	Player2.setPosition(area.width - 20, area.height / 2);

	Ball.setPosition(area.width / 2, area.height / 2);
	Ball.setRadius(Config.ball.radius);
	Ball.pcfg = &Config;
}


void pong::game::updatePlayer(paddle& player)
{
	auto& cfg = player.pcfg->paddle;
	auto& controls = player.pcfg->controls;
	auto& velocity = player.velocity;
	auto ball_bounds = Ball.getGlobalBounds();

	if (player.ai)
	{
		auto myPos = player.getPosition();
		auto ball_pos = pos(ball_bounds.left, ball_bounds.top);
		auto reaction = random_num(20, 85);
		auto diff = (myPos - ball_pos).y;
		auto spd = diff < 0 ? -diff + reaction : diff - reaction;

		auto ySpeed = spd / 50.0f;

		if (spd > ball_bounds.height)
		{
			if (ball_pos.y < myPos.y)
			{
				velocity.y = std::clamp(-ySpeed, -cfg.base_speed * 2, 0.f);
			}
			else if (ball_pos.y > myPos.y)
			{
				velocity.y = std::clamp(ySpeed, 0.f, cfg.base_speed * 2);
			}
		}
		else
		{
			velocity.y = 0;
		}
	}
	else // player
	{
		auto offset = cfg.base_speed * cfg.accel;
		auto const& c = controls[player.id];

		if (sf::Keyboard::isKeyPressed(c.up))
		{
			velocity.y -= offset;
		}
		else if (sf::Keyboard::isKeyPressed(c.down))
		{
			velocity.y += offset;
		}
		else
		{
			velocity.y *= 0.5f;
		}

		bool moving = velocity != sf::Vector2f();
		if (moving && sf::Keyboard::isKeyPressed(c.fast))
		{
			velocity.y *= 1.25f;
		}
	}

	player.update();
}

void pong::game::updateBall()
{
	auto const& cfg = Config.ball;

	paddle* player = nullptr;
	if (check_collision(Ball, Player1)) {
		player = &Player1;
	}
	else if (check_collision(Ball, Player2)) {
		player = &Player2;
	}

	if (player)
	{
		const auto vX = Ball.velocity.x * (1.f + cfg.accel);
		const auto vY = (player->velocity.y != 0 ? player->velocity.y * 0.75f : Ball.velocity.y) + random_num(-2, 2);

		Ball.velocity = {
			-std::clamp(vX, -cfg.max_speed, cfg.max_speed),
			 std::clamp(vY, -cfg.max_speed, cfg.max_speed)
		};

		do
		{
			Ball.update();
		} while (check_collision(*player, Ball));
	}
	else Ball.update();
}

void game::serve(dir direction)
{
	auto mov = Config.ball.base_speed;
	if (direction == dir::left) {
		mov = -mov;
	}

	Ball.setPosition(Playarea.width / 2, Playarea.height / 2);
	Ball.velocity = { mov, 0 };
}


void pong::ball::update()
{
	move(velocity);

	if (border_collision(*this))
	{
		velocity.y = -velocity.y;
	}
}

void pong::paddle::update()
{
	move(velocity);

	if (border_collision(*this))
	{
		move(-velocity);
		velocity.y = 0;
	}
}
