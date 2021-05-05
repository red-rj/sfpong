#include <string>
#include <algorithm>
#include <random>
#include <bitset>
#include <fmt/format.h>
#include <imgui.h>
#include "common.h"
#include "game.h"
#include "rng.h"
#include "menu.h"
#include "gvar.h"

const char pong::version[] = "0.8.2";

using namespace std::literals;


namespace
{
	auto rnd_eng = std::default_random_engine(1337);
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


bool pong::collision(const sf::Shape &a, const sf::Shape &b)
{
	return a.getGlobalBounds().intersects(b.getGlobalBounds());
}
bool pong::collision(const sf::Shape& a, const rect& b)
{
	return a.getGlobalBounds().intersects(b);
}
bool pong::collision(const rect& a, const rect& b)
{
	return a.intersects(b);
}

void pong::constrain_pos(pos& p)
{
	using namespace gvar;

	while (p.x >= playarea_width)	p.x -= playarea_width;
	while (p.y >= playarea_height)	p.y -= playarea_height;
	while (p.x < 0)	p.x += playarea_width;
	while (p.y < 0)	p.y += playarea_height;
}


pong::game::game(gamemode mode_, game_settings* sett)
	: Court({ gvar::playarea_width, gvar::playarea_height }, { gvar::playarea_width * .95f, 25 })
	, settings(sett)
{
	change_mode(mode_);
	restart();
}

void pong::game::change_mode(gamemode m) noexcept
{
	currentMode = m;

	if (currentMode == gamemode::singleplayer) {
		Player1.ai = false;
		Player2.ai = true;
	}
	else if (currentMode == gamemode::multiplayer) {
		Player1.ai = Player2.ai = false;
	}
	else if (currentMode == gamemode::aitest) {
		Player1.ai = Player2.ai = true;
	}
}


void pong::game::processEvent(sf::Event& event)
{
	using sf::Event;
	using sf::Keyboard;
	using mwin = menu::win::Id;

	devEvents(event);

	if (!menu::is_open(mwin::rebiding_popup)) {
		switch (event.type)
		{
			case Event::KeyReleased:
			{
				switch (event.key.code)
				{
					case Keyboard::Enter:
						if (waiting_to_serve()) {
							serve(resume_serve_dir);
						}
						break;
					case Keyboard::Escape:
						paused = !paused;
						// imgui deve capturar input só com o jogo pausado
						auto& io = ImGui::GetIO();
						io.WantCaptureKeyboard = paused;
						io.WantCaptureMouse = paused;
						break;
				}
			} break;
		}
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
			log::debug("DEV. Player1 Ai = {}", Player1.ai);
			break;
		case sf::Keyboard::F2:
			Player2.ai = !Player2.ai;
			log::debug("DEV. Player2 Ai = {}", Player2.ai);
			break;
		}

		break;
	}
	}
}

void pong::game::update()
{
	if (!paused)
	{
		updatePlayer(Player1);
		updatePlayer(Player2);
		updateBall();

		if (runTime.asMilliseconds() % 20 == 0) {
			if (updateScore())
			{
				reset(Player1);
				reset(Player2);
				reset(Ball);
			}
		}
	}
}

// scale 2 fit, center, preserve aspect ratio
static sf::View get_play_view(float target_width)
{
	float to_w = target_width;
	float from_w = gvar::playarea_width;

	if (from_w > to_w) {
		std::swap(from_w, to_w);
	}

	pong::rect vp;
	vp.width = from_w / to_w;
	vp.height = 1;
	auto space_ratio = (to_w - from_w) / to_w;
	vp.left = space_ratio / 2;

	auto view = sf::View(pong::rect(0,0, gvar::playarea_width, gvar::playarea_height));
	view.setViewport(vp);

	return view;
}

void pong::game::draw(sf::RenderWindow& window)
{
	window.clear();

	window.draw(Court);
	window.draw(Ball);
	window.draw(Player1);
	window.draw(Player2);
}


void pong::game::restart()
{
	reset(Player1);
	reset(Player2);
	reset(Ball);

	score = {};
	Court.set_score(0, 0);
}

void pong::game::updatePlayer(paddle& player)
{
	auto velocity = player.velocity;
	using gvar::paddle_max_speed;

	if (player.ai)
	{
		static sf::Clock AIClock;
		static const sf::Time AITime = sf::seconds(0.3f);

		if (AIClock.getElapsedTime() > AITime) {
			AIClock.restart();
			
			const auto offset = Ball.getPosition() - player.getPosition();

			if (abs(offset.y) > 20) {
				//velocity += std::copysign(1, offset.y);
				auto m = offset.y / 20;
				velocity += m;
			}

			velocity = std::clamp(velocity, -paddle_max_speed, paddle_max_speed);
		}
	}
	else // player
	{
		using sf::Keyboard;
		using sf::Joystick;

		// keyboard
		auto& kb_controls = settings->get_keyboard_keys(player.id);
		bool gofast_kb = Keyboard::isKeyPressed(kb_controls.fast);
		
		if (Keyboard::isKeyPressed(kb_controls.up))
			velocity -= gvar::paddle_kb_speed;
		else if (Keyboard::isKeyPressed(kb_controls.down))
			velocity += gvar::paddle_kb_speed;


		// joystick
		bool gofast_js = false;
		if (settings->using_joystick(player.id))
		{
			auto joyid = settings->get_joystick(player.id);
			auto deadzone = settings->joystick_deadzone(player.id);

			auto axis = Joystick::getAxisPosition(joyid, Joystick::Y);
			// deadzone
			if (abs(axis) > deadzone)
				velocity = axis / 3;

			gofast_js = Joystick::isButtonPressed(joyid, 0);
		}

		if (player.velocity != velocity) {
			velocity = std::clamp(velocity, -paddle_max_speed, paddle_max_speed);
			auto gofast = gofast_kb || gofast_js;

			if (velocity != 0 && gofast)
			{
				velocity *= 1.25f;
			}
		}
		else {
			velocity *= 0.6f; // desacelerar
		}
	}

	player.velocity = velocity;
	player.update();

	if (Court.border_collision(player.getGlobalBounds())) {
		player.move(0, -velocity);
		player.velocity = 0;
	}
}

void pong::game::updateBall()
{
	paddle* player = nullptr;

	if (collision(Ball, Player1)) {
		player = &Player1;
	}
	else if (collision(Ball, Player2)) {
		player = &Player2;
	}

	if (player)
	{
		using namespace gvar;
		vel velocity = Ball.velocity;

		velocity.x *= ball_acceleration;
		velocity.y += player->velocity * 0.5f;

		Ball.velocity = {
			-std::clamp(velocity.x, -ball_max_speed, ball_max_speed),
			 std::clamp(velocity.y, -ball_max_speed, ball_max_speed)
		};

		do
		{
			Ball.update();
		} while (collision(*player, Ball));
	}
	else Ball.update();

	if (Court.border_collision(Ball.getGlobalBounds()))
	{
		Ball.velocity.y = -Ball.velocity.y;
	}
}

bool pong::game::updateScore()
{
	auto bounds = Court.getBounds();

	if (!bounds.intersects(Ball.getGlobalBounds()))
	{
		// ponto!
		if (Ball.velocity.x < 0)
		{
			// indo p/ direita, ponto player 1, saque player 2
			score.first++;
			resume_serve_dir = dir::left;
		}
		else
		{
			// indo p/ esquerda, ponto player 2, saque player 1
			score.second++;
			resume_serve_dir = dir::right;
		}
		Court.set_score(score);
		log::info("score: {}x{} ; serve: {}", score.first, score.second, nameof(resume_serve_dir));

		return true;
	}
	else return false;
}

void pong::game::reset(ball& b)
{
	b.velocity = vel();
	b.setPosition(gvar::playarea_width / 2, gvar::playarea_height / 2);
}

void pong::game::reset(paddle& p)
{
	const auto margin = gvar::playarea_width * .05f;
	const auto center = point(gvar::playarea_width / 2, gvar::playarea_height / 2);

	if (p.id == playerid::one) {
		p.setPosition(margin - gvar::paddle_size.x, center.y);
	}
	else if (p.id == playerid::two) {
		p.setPosition(gvar::playarea_width - margin, center.y);
	}
	p.velocity = 0;
}

bool pong::game::waiting_to_serve() const noexcept
{
	return !paused 
		&& Ball.velocity == vel() 
		&& Ball.getPosition() == point(gvar::playarea_width / 2, gvar::playarea_height / 2);
}

void pong::game::serve(dir direction)
{
	auto mov = gvar::ball_speed;
	if (direction == dir::left) {
		mov = -mov;
	}

	Ball.setPosition(gvar::playarea_width / 2, gvar::playarea_height / 2);
	Ball.velocity = { mov, 0 };
}
