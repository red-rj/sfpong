#include <string>
#include <algorithm>
#include <random>
#include <filesystem>
#include <fmt/format.h>
#include <imgui.h>
#include <imgui-SFML.h>
#include "common.h"
#include "game.h"
#include "rng.h"
#include "menu.h"
#include "gvar.h"

const char pong::version[] = "0.9.0";

using namespace std::literals;

namespace pong { game_instance* G = nullptr; }

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


pong::player_t::player_t(playerid pid) 
	: id(pid), shape({gvar::paddle_width, gvar::paddle_height})
{
	shape.setOrigin({ 0, gvar::paddle_height / 2 });
	//const sf::Int32 lightgray = 0x45 << 24 | 0x45 << 16 | 0x45 << 8 | 127;
	shape.setOutlineColor(sf::Color::Black);
	shape.setOutlineThickness(1.5f);
}

pong::ball_t::ball_t() : shape(gvar::ball_radius)
{
	shape.setOrigin(gvar::ball_radius, gvar::ball_radius);
	shape.setFillColor(sf::Color::Red);
}

pong::background::background(size2d area)
{
	size(area);

	// init net
	// build alongside the X axis
	const size2d pieceSize = { 20,20 };
	const float gapLen = 20;
	point current;
	bool gap{};

	for (int count = 1; (pieceSize.x + gapLen) * count < mySize.y; gap = !gap)
	{
		if (gap) {
			current.x += gapLen;
		}
		else {
			sf::Vertex v{ current, sf::Color(200,200,200) };

			// triangle1
			net.verts.append(v);
			v.position.x += pieceSize.x;
			net.verts.append(v);
			v.position.y += pieceSize.y;
			net.verts.append(v);
			// triangle2
			v.position = current;
			net.verts.append(v);
			v.position.y += pieceSize.y;
			net.verts.append(v);
			v.position.x += pieceSize.x;
			net.verts.append(v);

			current.x += pieceSize.x;
			count++;
		}
	}

	// nessa ordem
	net.transform.translate(mySize.x / 2, 20).rotate(90);

	score.font.loadFromFile(files::mono_tff);
	score.text.setPosition(mySize.x / 2 - 100, borderSize.y);
	score.text.setCharacterSize(55);
	score.text.setFont(score.font);
}

void pong::background::update_score(int p1, int p2)
{
	score.text.setString(fmt::format("{}    {}", p1, p2));
}

void pong::background::size(size2d value)
{
	mySize = value;
	borderSize = { value.x * .95f, 25 };

	auto origin = point(borderSize.x / 2, 0);
	// margin = 6

	top.setSize(borderSize);
	top.setOrigin(origin);
	top.setPosition(mySize.x / 2, 0 + 6);

	origin.y = borderSize.y;
	bottom.setSize(borderSize);
	bottom.setOrigin(origin);
	bottom.setPosition(mySize.x / 2, mySize.y - 6);
}

void pong::background::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform = getTransform() * net.transform;
	target.draw(net.verts, states);

	states.transform = sf::Transform::Identity * getTransform();
	target.draw(top, states);
	target.draw(bottom, states);
	target.draw(score.text, states);
}

bool pong::background::border_collision(const rect& bounds) const
{
	rect R[] = {
		getTransform().transformRect(top.getGlobalBounds()),
		getTransform().transformRect(bottom.getGlobalBounds())
	};
	return bounds.intersects(R[0]) or bounds.intersects(R[1]);
}

pong::point pong::background::getPoint(size_t i) const
{
	switch (i)
	{
	default:
	case 0: return top.getTransform().transformPoint(top.getPoint(3));
	case 1: return top.getTransform().transformPoint(top.getPoint(2));
	case 2: return bottom.getTransform().transformPoint(bottom.getPoint(1));
	case 3: return bottom.getTransform().transformPoint(bottom.getPoint(0));
	}
}


pong::game_instance::game_instance(arguments_t params_)
	: bg({gvar::playarea_width, gvar::playarea_height})
	, params(params_)
{
	try
	{
		settings.load_file(params.configFile);
	}
	catch (std::exception& e)
	{
		spdlog::error("config load error: {}", e.what());
	}

	try
	{
		auto vidmode = sf::VideoMode{
			settings.resolution.x,
			settings.resolution.y
		};
		window.create(vidmode, "Sf Pong!");
		window.setFramerateLimit(60u);
	}
	catch (std::exception& e)
	{
		spdlog::error("failed to create window! {}", e.what());
		throw 5;
	}

	changeMode(gamemode::singleplayer);
	reset();
}

pong::game_instance::~game_instance()
{
	spdlog::info("Tchau! ;D");
	settings.save_file(params.configFile);
}

void pong::game_instance::changeMode(gamemode m) noexcept
{
	switch (m)
	{
	default:
	case pong::gamemode::singleplayer:
		player1.ai = false;
		player2.ai = true;
		break;
	case pong::gamemode::multiplayer:
		player1.ai = player2.ai = false;
		break;
	case pong::gamemode::aitest:
		player1.ai = player2.ai = true;
		break;
	}

	mode = m;
}


void pong::game_instance::processEvent(sf::Event& event)
{
	using sf::Event;
	using sf::Keyboard;
	using mwin = menu::win::Id;

#ifndef NDEBUG
	// devEvents
	switch (event.type)
	{
	case sf::Event::KeyReleased:
	{
		switch (event.key.code)
		{
		case sf::Keyboard::F1:
			player1.ai = !player1.ai;
			spdlog::debug("DEV. Player1 Ai = {}", player1.ai);
			break;
		case sf::Keyboard::F2:
			player2.ai = !player2.ai;
			spdlog::debug("DEV. Player2 Ai = {}", player2.ai);
			break;
		}

		break;
	}
	}
#endif // !NDEBUG

	if (!menu::is_open(mwin::rebiding_popup)) {
		switch (event.type)
		{
		case Event::KeyReleased:
		{
			switch (event.key.code)
			{
			case Keyboard::Enter:
				if (waiting_to_serve()) {
					serve(serveDir);
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

bool pong::game_instance::waiting_to_serve() const noexcept
{
	return !paused
		&& ball.velocity == vec2()
		&& ball.shape.getPosition() == point(gvar::playarea_width / 2, gvar::playarea_height / 2);
}

void pong::game_instance::serve(dir direction)
{
	auto mov = gvar::ball_speed;
	if (direction == dir::left) {
		mov = -mov;
	}

	ball.shape.setPosition(gvar::playarea_width / 2, gvar::playarea_height / 2);
	ball.velocity = { mov, 0 };
}

void pong::game_instance::updatePlayer(player_t& player)
{
	using gvar::paddle_max_speed;
	bool turbo = false;
	auto mom = player.velocity;

	if (player.ai)
	{
		// TODO
		static sf::Clock AIClock;
		static const sf::Time AITime = sf::seconds(0.1f);

		if (AIClock.getElapsedTime() > AITime) {
			AIClock.restart();

			const auto offset = ball.shape.getPosition() - player.shape.getPosition();
			const auto ai_speed = 1;
			const auto ydiff = abs(offset.y);

			if (ydiff >= ball.shape.getRadius()) {
				mom.y += std::copysign(ai_speed, offset.y);
				turbo = ydiff > 99;
			}
		}
	}
	else // player
	{
		using sf::Keyboard;
		using sf::Joystick;

		// keyboard
		auto& kb_controls = settings.get_keyboard_keys(player.id);
		bool gofast_kb = Keyboard::isKeyPressed(kb_controls.fast);

		if (Keyboard::isKeyPressed(kb_controls.up))
			mom.y -= gvar::paddle_kb_speed;
		else if (Keyboard::isKeyPressed(kb_controls.down))
			mom.y += gvar::paddle_kb_speed;

		// joystick
		bool gofast_js = false;
		if (settings.using_joystick(player.id))
		{
			auto joyid = settings.get_joystick(player.id);
			auto deadzone = settings.joystick_deadzone(player.id);

			auto axis = Joystick::getAxisPosition(joyid, Joystick::Y);
			// deadzone
			if (abs(axis) > deadzone)
				mom.y = axis / 3;

			gofast_js = Joystick::isButtonPressed(joyid, 0);
		}

		turbo = gofast_kb || gofast_js;
	}

	// TODO: mover pra player.update()
	mom.y = std::clamp(mom.y, -paddle_max_speed, paddle_max_speed);

	if (turbo)
	{
		mom.y *= 1.25f;
	}
	else if (!player.ai && player.velocity == mom) {
		mom.y *= 0.6f;
	}

	player.velocity = mom;
	player.update();

	if (bg.border_collision(player.shape.getGlobalBounds())) {
		player.velocity = {};
		auto position = player.shape.getPosition();

		if (collision(player.shape, bg.topBorder())) {
			position.y = bg.getPoint(0).y + player.shape.getOrigin().y + 2;
			//	p.y = 108;
		}
		else {
			position.y = bg.getPoint(2).y - player.shape.getOrigin().y - 2;
			//	p.y = 916;
		}

		player.shape.setPosition(position);
	}

}

void pong::game_instance::updateBall()
{
	player_t* player = nullptr;
	auto mom = ball.velocity;

	if (collision(ball.shape, player1.shape)) {
		player = &player1;
	}
	else if (collision(ball.shape, player2.shape)) {
		player = &player2;
	}

	if (player)
	{
		using namespace gvar;

		mom.x += ball_acceleration;
		mom.y += player->velocity.y * 0.5f;

		ball.velocity = {
			-std::clamp(mom.x, -ball_max_speed, ball_max_speed),
			 std::clamp(mom.y, -ball_max_speed, ball_max_speed)
		};

		do
		{
			ball.update();
		} while (collision(player->shape, ball.shape));
	}
	else ball.update();

	if (bg.border_collision(ball.shape.getGlobalBounds()))
	{
		ball.velocity.y = -ball.velocity.y;
	}
}

bool pong::game_instance::updateScore()
{
	auto bounds = rect({}, bg.size());

	if (!bounds.intersects(ball.shape.getGlobalBounds()))
	{
		// ponto!
		if (ball.velocity.x < 0)
		{
			// indo p/ direita, ponto player 1, saque player 2
			score.first++;
			serveDir = dir::left;
		}
		else
		{
			// indo p/ esquerda, ponto player 2, saque player 1
			score.second++;
			serveDir = dir::right;
		}
		bg.update_score(score.first, score.second);
		spdlog::info("score: {}x{} ; serve: {}", score.first, score.second, nameof(serveDir));

		return true;
	}
	else return false;
}

void pong::game_instance::update()
{
	if (!paused)
	{
		updatePlayer(player1);
		updatePlayer(player2);
		updateBall();

		if (runTime.asMilliseconds() % 20 == 0) {
			if (updateScore())
			{
				reset(player1);
				reset(player2);
				reset(ball);
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

void pong::game_instance::render()
{
	window.clear();

	window.draw(bg);
	window.draw(ball.shape);
	window.draw(player1.shape);
	window.draw(player2.shape);
}


void pong::game_instance::reset()
{
	reset(player1);
	reset(player2);
	reset(ball);

	score = {};
	bg.update_score(0, 0);
}

void pong::game_instance::reset(ball_t& b)
{
	b.velocity = {};
	b.shape.setPosition(gvar::playarea_width / 2, gvar::playarea_height / 2);
}

void pong::game_instance::reset(player_t& p)
{
	const auto center = point(gvar::playarea_width / 2, gvar::playarea_height / 2);

	if (p.id == playerid::one) {
		//p.shape.setPosition(margin - gvar::paddle_width, center.y);
		auto margin = bg.getPoint(0).x;
		p.shape.setPosition(margin - gvar::paddle_width, center.y);
	}
	else if (p.id == playerid::two) {
		auto margin = bg.getPoint(2).x;
		p.shape.setPosition(gvar::playarea_width - margin, center.y);
	}

	p.velocity = {};
}


int pong::main()
{
	while (G->window.isOpen())
	{
		sf::Event event;
		while (G->window.pollEvent(event))
		{
			// global events
			switch (event.type)
			{
			case sf::Event::Closed:
				G->window.close();
				break;
			}

			ImGui::SFML::ProcessEvent(event);
			menu::processEvent(event);
			G->processEvent(event);
		}

		auto dt = G->restartClock();
		ImGui::SFML::Update(G->window, dt);

		G->update();
		G->render();
		menu::update();

		ImGui::SFML::Render(G->window);

		G->window.display();
	}

	return 0;
}
