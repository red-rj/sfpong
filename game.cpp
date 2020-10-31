#include <string>
#include <algorithm>
#include <random>

#include <fmt/format.h>
#include <imgui.h>
#include <imgui-SFML.h>
#include <boost/property_tree/ptree.hpp>

#include "common.h"
#include "game.h"
#include "rng.h"
#include "menu.h"
#include "game_config.h"

const char pong::version[] = "0.2.2";

using namespace std::literals;

namespace
{
	auto rnd_dev = std::random_device();
	auto rnd_eng = std::default_random_engine(rnd_dev());

	pong::rect Playarea;
	pong::court Court;

	struct movement
	{
		float speed;
		float max_speed;
		float acceleration;
	};

	struct paddle_cfg {
		movement move;
		pong::size2d size;
	};

	struct ball_cfg {
		movement move;
		float radius;
	};

	paddle_cfg CfgPaddle{
		/*speed*/		10,
		/*max speed*/	30,
		/*accel*/		0.1f,

		/*size {x,y}*/	pong::size2d(25, 150)
	};
	ball_cfg CfgBall{
		/*speed*/		5,
		/*max speed*/	20,
		/*accel*/		0.1f,

		/*radius*/		20,
	};

	sf::Font font_sans, font_mono;
	sf::RenderWindow* sfwindow;

	struct drawable_message : sf::Drawable
	{
		drawable_message(const sf::Font& font, unsigned chSize)
			: text("", font, chSize)
		{
			bg.setFillColor(sf::Color(0, 0, 0, 128));
			bg.setOutlineColor(sf::Color::White);
			bg.setOutlineThickness(5);
		}

		void write(const std::string& message, pong::size2d coords)
		{
			text.setString(message);

			pong::size_2d<float> bgsize;
			const auto margin = pong::size2d{ 10.f, 10.f };
			
			bgsize.x = (text.getCharacterSize() + text.getLetterSpacing()) * message.length() / 2;
			bgsize.y = (text.getCharacterSize() + text.getLineSpacing()) * 3;

			bg.setSize(bgsize);
			bg.setPosition(coords);
			text.setPosition(coords + pong::size2d{ 10.f, 10.f });
		}
		void clear() {
			text.setString("");
		}

	private:
		void draw(sf::RenderTarget& target, sf::RenderStates states) const override
		{
			if (!text.getString().isEmpty()) {
				target.draw(bg);
				target.draw(text, states);
			}
		}

		sf::Text text;
		sf::RectangleShape bg;
	};
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


bool pong::collision(const sf::Shape &a, const sf::Shape &b)
{
    return a.getGlobalBounds().intersects(b.getGlobalBounds());
}
bool pong::collision(const sf::Shape& a, const rect& b)
{
	return a.getGlobalBounds().intersects(b);
}
bool pong::border_collision(const sf::Shape& p)
{
	return collision(p, Court.top) or collision(p, Court.bottom);
}

void pong::constrain_pos(pos& p)
{
	while (p.x >= Playarea.width)	p.x -= Playarea.width;
	while (p.y >= Playarea.height)	p.y -= Playarea.height;
	while (p.x < Playarea.left)		p.x += Playarea.left;
	while (p.y < Playarea.top)		p.x += Playarea.top;
}


void pong::overrideGuts(const cfgtree& guts)
{
	if (auto p = guts.get_child_optional("paddle")) try
	{
		paddle_cfg cfg;
		cfg.move.speed = p->get<float>("speed");
		cfg.move.acceleration = p->get<float>("acceleration");
		cfg.move.max_speed = p->get<float>("max_speed");
		cfg.size.x = p->get<float>("width");
		cfg.size.y = p->get<float>("height");
		CfgPaddle = cfg;
	}
	catch(std::exception& e)
	{
		log::debug("{} paddle error: {}", __func__, e.what());
	}
	log::trace("{} paddle cfg loaded", __func__);

	if (auto node = guts.get_child_optional("ball")) try
	{
		ball_cfg cfg;
		cfg.move.speed = node->get<float>("speed");
		cfg.move.acceleration = node->get<float>("acceleration");
		cfg.move.max_speed = node->get<float>("max_speed");
		cfg.radius = node->get<float>("radius");
		CfgBall = cfg;
	}
	catch (std::exception& e)
	{
		log::debug("{} ball error: {}", __func__, e.what());
	}
	log::trace("{} ball cfg loaded", __func__);

	log::debug("{} success!", __func__);
}

pong::cfgtree pong::createGuts()
{
	auto guts = cfgtree();
	guts.put("version", version);

	guts.put("paddle.speed", CfgPaddle.move.speed);
	guts.put("paddle.acceleration", CfgPaddle.move.acceleration);
	guts.put("paddle.max_speed", CfgPaddle.move.max_speed);
	guts.put("paddle.width", CfgPaddle.size.x);
	guts.put("paddle.height", CfgPaddle.size.y);

	guts.put("ball.speed", CfgBall.move.speed);
	guts.put("ball.acceleration", CfgBall.move.acceleration);
	guts.put("ball.max_speed", CfgBall.move.max_speed);
	guts.put("ball.radius", CfgBall.radius);

	return guts;
}

void pong::score::update()
{
	text.setString(fmt::format("{}    {}", val.first, val.second));
}


void pong::game::setup(sf::RenderWindow& window)
{
	sfwindow = &window;

	font_sans.loadFromFile(pong::files::sans_tff);
	font_mono.loadFromFile(pong::files::mono_tff);

	game_menu.init();
}

pong::game::game(size2d playsize, mode mode_) : currentMode(mode_)
{
	// pong court
	auto area = rect{ {0.f,0.f}, playsize };
	generateLevel(area);
	Score.create(area, font_mono, 55);
	resetState();
}

pong::game::game(mode mode_) : game({Playarea.width, Playarea.height}, mode_) {}


void pong::game::processEvent(sf::Event& event)
{
	devEvents(event);

	switch (event.type)
	{
	case sf::Event::Closed:
		sfwindow->close();
		break;

	case sf::Event::KeyReleased:
	{
		if (game_menu.rebinding)
			break;

		switch (event.key.code)
		{
		case sf::Keyboard::Enter:
			serve(dir::left);
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

	case sf::Event::JoystickConnected:
	case sf::Event::JoystickDisconnected:
		game_menu.refresh_joystick_list();
		break;
	}

}
void pong::game::devEvents(const sf::Event& event)
{
	switch (event.type)
	{
	case sf::Event::KeyReleased:
	{
		switch (event.key.code)
		{
		case sf::Keyboard::F1:
			Player1.ai = !Player1.ai;
			break;
		case sf::Keyboard::F2:
			Player2.ai = !Player2.ai;
			break;
		case sf::Keyboard::F12:
			resetState();
			break;
		}
	} break;
	}
}

void pong::game::update()
{
	auto& window = *sfwindow;

	window.clear();
	window.draw(Court);
	window.draw(Score);

	//auto msg = drawable_message(font_sans, 45);
	//msg.write("Bem-vindo! Isto é um teste...", { 250.f, 150.f });
	//window.draw(msg);

	if (!paused)
	{
		window.draw(Ball);
		window.draw(Player1);
		window.draw(Player2);
		
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

	game_menu.update(*this, window);
}


void pong::game::resetState()
{
	auto area = Playarea;

	Player1.id = playerid::one;
	Player1.setSize(CfgPaddle.size);
	Player1.setOrigin(CfgPaddle.size.x / 2, CfgPaddle.size.y / 2);
	Player1.setPosition(20, area.height / 2);

	Player2 = Player1;
	Player2.id = playerid::two;
	Player2.setPosition(area.width - 20, area.height / 2);

	if (currentMode==mode::singleplayer) {
		Player1.ai = false;
		Player2.ai = true;
	}
	else if (currentMode == mode::multiplayer) {
		Player1.ai = Player2.ai = false;
	}
	//else if (currentMode == mode::aitest) {
	//	Player1.ai = Player2.ai = true;
	//}

	Ball.setPosition(area.width / 2, area.height / 2);
	Ball.setRadius(CfgBall.radius);
}


void pong::game::updatePlayer(paddle& player)
{
	auto& cfg = CfgPaddle;
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
				velocity.y = std::clamp(-ySpeed, -cfg.move.speed * 2, 0.f);
			}
			else if (ball_pos.y > myPos.y)
			{
				velocity.y = std::clamp(ySpeed, 0.f, cfg.move.speed * 2);
			}
		}
		else
		{
			velocity.y = 0;
		}
	}
	else // player
	{
		using sf::Keyboard;
		using sf::Joystick;

		float movement = velocity.y;
		bool gofast = false;
		auto input = pong::get_input_cfg(player.id);
		const auto& controls = input.keyboard_controls;

		if (input.use_joystick()) {
			auto axis = Joystick::getAxisPosition(input.joystickId, Joystick::Y);
			// deadzone
			if (abs(axis) > input.joystick_deadzone)
				movement = axis / 5;

			gofast = Joystick::isButtonPressed(input.joystickId, 0);
		}
		else
		{ // keyboard
			auto offset = cfg.move.speed + cfg.move.speed * cfg.move.acceleration;

			if (Keyboard::isKeyPressed(controls.up))
				movement -= offset;
			else if (Keyboard::isKeyPressed(controls.down))
				movement += offset;
			else
				movement /= 3; // desacelerar

			gofast = Keyboard::isKeyPressed(controls.fast);
		}

		velocity.y = std::clamp(movement, -cfg.move.max_speed, cfg.move.max_speed);

		if (velocity != vel() && gofast)
		{
			velocity.y *= 1.25f;
		}
	}

	player.update();
}

void pong::game::updateBall()
{
	auto const& cfg = CfgBall;

	paddle* player = nullptr;
	if (collision(Ball, Player1)) {
		player = &Player1;
	}
	else if (collision(Ball, Player2)) {
		player = &Player2;
	}

	if (player)
	{
		const auto MAX = cfg.move.max_speed;
		vel velocity = Ball.velocity;

		velocity.x *= 1.0 + cfg.move.acceleration;
		if (player->velocity.y != 0) {
			velocity.y = player->velocity.y * 0.75 + random_num(-2, 2);
			//velocity.y += player->velocity.y * 0.5 + random_num(-2, 2);
		}

		Ball.velocity = {
			-std::clamp(velocity.x, -MAX, MAX),
			 std::clamp(velocity.y, -MAX, MAX)
		};

		do
		{
			Ball.update();
		} while (collision(*player, Ball));
	}
	else Ball.update();
}

void pong::game::generateLevel(rect area)
{
	Playarea = area;
	//Court = pong::court(Playarea, 25, { 15, 20 });
	Court = pong::court(Playarea, { area.width * 0.95f, 25 });
}

void pong::game::serve(dir direction)
{
	auto mov = CfgBall.move.speed;
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
