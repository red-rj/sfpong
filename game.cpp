#include <string>
#include <algorithm>
#include <random>
#include <bitset>
#include <fmt/format.h>
#include <imgui.h>
#include <boost/property_tree/ptree.hpp>
#include "common.h"
#include "game.h"
#include "rng.h"
#include "menu.h"
#include "gvar.h"

const char pong::version[] = "0.8.1";

using namespace std::literals;


namespace
{
	auto rnd_dev = std::random_device();
	auto rnd_eng = std::default_random_engine(rnd_dev());

	// TODO: classe para score, expor court
	//pong_court Court{ Playarea, { Playarea.width * 0.95f, 25 } };

	//sf::Font font_mono;
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


void pong::constrain_pos(pos& p)
{
	using gvar::playarea;

	while (p.x >= playarea.width)	p.x -= playarea.width;
	while (p.y >= playarea.height)	p.y -= playarea.height;
	while (p.x < playarea.left)		p.x += playarea.left;
	while (p.y < playarea.top)		p.x += playarea.top;
}


//void pong::overrideGuts(const cfgtree& guts)
//{
//	using namespace engine;
//
//	paddle_kb_speed = guts.get("paddle.kb_speed", paddle_kb_speed);
//	paddle_max_speed = guts.get("paddle.max_speed", paddle_max_speed);
//	paddle_size.x = guts.get("paddle.width", paddle_size.x);
//	paddle_size.y = guts.get("paddle.height", paddle_size.y);
//
//	ball_speed = guts.get("ball.speed", ball_speed);
//	ball_acceleration = guts.get("ball.acceleration", ball_acceleration);
//	ball_max_speed = guts.get("ball.max_speed", ball_max_speed);
//	ball_radius = guts.get("ball.radius", ball_radius);
//}
//
//pong::cfgtree pong::createGuts()
//{
//	using namespace engine;
//	
//	auto guts = cfgtree();
//	guts.put("version", version);
//
//	guts.put("paddle.kb_speed", paddle_kb_speed);
//	guts.put("paddle.max_speed", paddle_max_speed);
//	guts.put("paddle.width", paddle_size.x);
//	guts.put("paddle.height", paddle_size.y);
//
//	guts.put("ball.speed", ball_speed);
//	guts.put("ball.acceleration", ball_acceleration);
//	guts.put("ball.max_speed", ball_max_speed);
//	guts.put("ball.radius", ball_radius);
//
//	return guts;
//}



pong::game::game(gamemode mode_, game_settings* sett)
	: Player1(playerid::one), Player2(playerid::two)
	, Court({ gvar::playarea.width, gvar::playarea.height }, { gvar::playarea.width * .95f, 25 })
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

	devEvents(event);

	switch (event.type)
	{
	case Event::KeyReleased:
	{
		if (menu::rebinding_popup_open())
			break;

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
		}

		break;
	}
	}
}

void pong::game::update()
{
	if (!paused)
	{
		updateBall();

		if (runTime.asMilliseconds() % 20 == 0) {
			if (updateScore())
			{
				reset(Player1);
				reset(Player2);
				reset(Ball);
			}
		}

		updatePlayer(Player1);
		updatePlayer(Player2);
	}
}

// scale 2 fit, center, preserve aspect ratio
static sf::View get_play_view(float target_width)
{
	float to_w = target_width;
	float from_w = gvar::playarea.width;

	if (from_w > to_w) {
		std::swap(from_w, to_w);
	}

	pong::rect vp;
	vp.width = from_w / to_w;
	vp.height = 1;
	auto space_ratio = (to_w - from_w) / to_w;
	vp.left = space_ratio / 2;

	auto view = sf::View(gvar::playarea);
	view.setViewport(vp);

	return view;
}

void pong::game::draw(sf::RenderWindow& window)
{
	const auto play_view = get_play_view((float)window.getSize().x);

	window.clear();
	window.setView(play_view);

	window.draw(Court);

	window.draw(Ball);
	window.draw(Player1);
	window.draw(Player2);

	window.setView(window.getDefaultView());
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

	if (player.ai)
	{
		using gvar::paddle_max_speed;

		static sf::Clock AIClock;
		static const sf::Time AITime = sf::seconds(0.1f);

		if (AIClock.getElapsedTime() > AITime) {
			AIClock.restart();
			
			const auto offset = Ball.getPosition() - player.getPosition();
			const auto y_diff = abs(offset.y);
			const auto ai_speed = 1;

			float mov = velocity;

			if (offset.y > 0)
				mov += ai_speed;
			else if (offset.y < 0)
				mov -= ai_speed;

			velocity = std::clamp(mov, -paddle_max_speed, paddle_max_speed);

			if (y_diff < 50) {
				velocity /= 2;
			}
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
			using gvar::paddle_max_speed;
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
		velocity.y;
		if (player->velocity != 0) {
			velocity.y = player->velocity * 0.75f + random_num(-2, 2);
			//velocity.y += player->velocity.y * 0.5 + random_num(-2, 2);
		}

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
}

bool pong::game::updateScore()
{
	if (!gvar::playarea.intersects(Ball.getGlobalBounds()))
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
		Court.set_score(score.first, score.second);
		log::info("score: {}x{} ; serve: {}", score.first, score.second, to_string(resume_serve_dir));

		return true;
	}
	else return false;
}

void pong::game::reset(ball& b)
{
	b.velocity = vel();
	b.setPosition(gvar::playarea.width / 2, gvar::playarea.height / 2);
}

void pong::game::reset(paddle& p)
{
	const auto margin = gvar::playarea.width * .05f;
	const auto center = pos(gvar::playarea.width / 2, gvar::playarea.height / 2);

	if (p.id == playerid::one) {
		p.setPosition(margin - gvar::paddle_size.x, center.y);
	}
	else if (p.id == playerid::two) {
		p.setPosition(gvar::playarea.width - margin, center.y);
	}
	p.velocity = 0;
}

bool pong::game::waiting_to_serve() const noexcept
{
	return !paused 
		&& Ball.velocity == vel() 
		&& Ball.getPosition() == pos(gvar::playarea.width / 2, gvar::playarea.height / 2);
}

void pong::game::serve(dir direction)
{
	auto mov = gvar::ball_speed;
	if (direction == dir::left) {
		mov = -mov;
	}

	Ball.setPosition(gvar::playarea.width / 2, gvar::playarea.height / 2);
	Ball.velocity = { mov, 0 };
}
