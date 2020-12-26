#include <string>
#include <algorithm>
#include <random>
#include <tuple>

#include <fmt/format.h>
#include <imgui.h>
#include <imgui-SFML.h>
#include <boost/property_tree/ptree.hpp>

#include "common.h"
#include "game.h"
#include "rng.h"
#include "menu.h"
#include "game_config.h"

const char pong::version[] = "0.9.0";

using namespace std::literals;

struct net_shape : public sf::Drawable, public sf::Transformable
{
	explicit net_shape(float pieceSize_ = 20.f, int pieceCount_ = 25);

private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override
	{
		states.transform *= getTransform();
		target.draw(m_net, states);
	}


	float m_piece_size;
	int m_piece_count;
	sf::VertexArray m_net{ sf::Quads };
};

net_shape::net_shape(float pieceSize_, int pieceCount_) : m_piece_size(pieceSize_), m_piece_count(pieceCount_)
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

struct pong_court : public sf::Drawable
{
	pong_court() = default;

	pong_court(pong::rect playarea, pong::size2d border_size) : m_playarea(playarea), top(border_size), bottom(border_size)
	{
		const auto margin = pong::size2d(0, 5);
		auto origin = pong::size2d(border_size.x / 2, 0);

		top.setOrigin(origin);
		top.setPosition(playarea.width / 2, margin.y);

		//bottom = top;
		origin.y = border_size.y;

		bottom.setOrigin(origin);
		bottom.setPosition(playarea.width / 2, playarea.height - margin.y);

		net.setPosition(playarea.width / 2, margin.y);
	}

	auto getCenter() const noexcept {
		auto center = m_playarea;
		center.top += m_hOffset;
		center.height -= m_hOffset * 2;
		return center;
	}

	sf::RectangleShape top, bottom;
	net_shape net;

private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override
	{
		target.draw(top, states);
		target.draw(bottom, states);
		target.draw(net, states);
	}

	pong::rect m_playarea;
	float m_hOffset;
};

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

		const auto margin = pong::size2d{ 10.f, 10.f };

		sf::Vector2<float> bgsize;
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


namespace
{
	auto rnd_dev = std::random_device();
	auto rnd_eng = std::default_random_engine(rnd_dev());

	const pong::rect Playarea = { 0.f, 0.f, 1280.f, 1024.f };
	pong_court Court;

namespace engine
{
	float paddle_kb_speed = 1;
	float paddle_max_speed = 30;
	pong::size2d paddle_size = { 25, 150 };

	float ball_speed = 5;
	float ball_max_speed = 20;
	float ball_acceleration = 0.1f;
	float ball_radius = 20;
};

	sf::Font font_sans, font_mono;
	sf::Text txtScore;

	void set_score_txt(pong::pair<short> const& val)
	{
		txtScore.setString(fmt::format("{}    {}", val.first, val.second));
	}
}

sf::Window* pong::game_window = nullptr;


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
	using namespace engine;

	paddle_kb_speed = guts.get("paddle.kb_speed", paddle_kb_speed);
	paddle_max_speed = guts.get("paddle.max_speed", paddle_max_speed);
	paddle_size.x = guts.get("paddle.width", paddle_size.x);
	paddle_size.y = guts.get("paddle.height", paddle_size.y);

	ball_speed = guts.get("ball.speed", ball_speed);
	ball_acceleration = guts.get("ball.acceleration", ball_acceleration);
	ball_max_speed = guts.get("ball.max_speed", ball_max_speed);
	ball_radius = guts.get("ball.radius", ball_radius);
}

pong::cfgtree pong::createGuts()
{
	using namespace engine;
	
	auto guts = cfgtree();
	guts.put("version", version);

	guts.put("paddle.kb_speed", paddle_kb_speed);
	guts.put("paddle.max_speed", paddle_max_speed);
	guts.put("paddle.width", paddle_size.x);
	guts.put("paddle.height", paddle_size.y);

	guts.put("ball.speed", ball_speed);
	guts.put("ball.acceleration", ball_acceleration);
	guts.put("ball.max_speed", ball_max_speed);
	guts.put("ball.radius", ball_radius);

	return guts;
}

pong::paddle::paddle(playerid pid) : base_t(engine::paddle_size), id(pid)
{
	setOrigin(0, engine::paddle_size.y / 2);
}

pong::ball::ball() : base_t(engine::ball_radius)
{
	setOrigin(engine::ball_radius, engine::ball_radius);
	setFillColor(sf::Color::Red);
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
	move(0, velocity);

	if (border_collision(*this))
	{
		move(0, -velocity);
		velocity = 0;
	}
}

void pong::game::setup(sf::RenderWindow& window)
{
	game_window = &window;
	Court = pong_court(Playarea, { Playarea.width * 0.95f, 25 });

	font_sans.loadFromFile(pong::files::sans_tff);
	font_mono.loadFromFile(pong::files::mono_tff);

	txtScore.setFont(font_mono);
	txtScore.setCharacterSize(55);
	txtScore.setPosition(Playarea.width / 2 - 100, 30);
	set_score_txt({});

	menu::init();
}


pong::game::game(mode mode_) 
	: currentMode(mode_), 
	Player1(playerid::one), Player2(playerid::two)
{
	resetPos(Player1);
	resetPos(Player2);
	resetPos(Ball);

	if (currentMode == mode::singleplayer) {
		Player1.ai = false;
		Player2.ai = true;
	}
	else if (currentMode == mode::multiplayer) {
		Player1.ai = Player2.ai = false;
	}
	else if (currentMode == mode::aitest) {
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
	case sf::Event::Closed:
		game_window->close();
		break;

	case sf::Event::KeyReleased:
	{
		if (menu::rebinding_popup_open())
			break;

		switch (event.key.code)
		{
		case sf::Keyboard::Enter:
			if (waiting_to_serve()) {
				serve(resume_serve_dir);
			}
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

void pong::game::update(sf::Time delta)
{
	if (!paused)
	{
		updateBall();

		if (runTime.asMilliseconds() % 30 == 0) {
			if (updateScore())
			{
				resetPos(Player1);
				resetPos(Player2);
				resetPos(Ball);
			}
		}

		updatePlayer(Player1);
		updatePlayer(Player2);
		
		runTime += delta;
	}
}

// scale 2 fit, center, preserve aspect ratio
static sf::View get_play_view(float target_width)
{
	float to_w = target_width;
	float from_w = Playarea.width;
	pong::rect vp;

	if (from_w > to_w) {
		std::swap(from_w, to_w);
	}

	auto space_ratio = (to_w - from_w) / to_w;
	vp.width = from_w / to_w;
	vp.height = 1;
	vp.left = space_ratio / 2;

	auto view = sf::View(Playarea);
	view.setViewport(vp);

	return view;
}

void pong::game::draw()
{
	auto& window = static_cast<sf::RenderWindow&>(*game_window);
	const auto& prev_view = window.getView();
	const auto play_view = get_play_view((float)window.getSize().x);

	window.clear();
	window.setView(play_view);

	window.draw(Court);
	window.draw(txtScore);

	if (!paused) {
		window.draw(Ball);
		window.draw(Player1);
		window.draw(Player2);
	}

	window.setView(prev_view);
}


void pong::game::resetState()
{
	*this = game(currentMode);
}

void pong::game::updatePlayer(paddle& player)
{
	auto& velocity = player.velocity;
	auto ball_bounds = Ball.getGlobalBounds();

	if (player.ai)
	{
		using engine::paddle_max_speed;

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

			if (y_diff < 20) {
				velocity /= 2;
			}
		}
	}
	else // player
	{
		using sf::Keyboard;
		using sf::Joystick;

		float movement = velocity;

		const auto input = pong::get_input_cfg(player.id);
		const auto& kb_controls = input.keyboard_controls;

		// keyboard
		if (Keyboard::isKeyPressed(kb_controls.up))
			movement -= engine::paddle_kb_speed;
		else if (Keyboard::isKeyPressed(kb_controls.down))
			movement += engine::paddle_kb_speed;

		bool gofast_kb = Keyboard::isKeyPressed(kb_controls.fast);

		// joystick
		bool gofast_js = false;
		if (input.use_joystick())
		{
			auto axis = Joystick::getAxisPosition(input.joystick_id, Joystick::Y);
			// deadzone
			if (abs(axis) > input.joystick_deadzone)
				movement = axis / 3;

			gofast_js = Joystick::isButtonPressed(input.joystick_id, 0);
		}

		if (movement != velocity) {
			using engine::paddle_max_speed;
			velocity = std::clamp(movement, -paddle_max_speed, paddle_max_speed);
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
		using namespace engine;
		vel velocity = Ball.velocity;

		velocity.x *= 1.0f + ball_acceleration;
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
	if (!Playarea.intersects(Ball.getGlobalBounds()))
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
		set_score_txt(score);
		log::info("score: {}x{} ; serve: {}", score.first, score.second, to_string(resume_serve_dir));

		return true;
	}
	else return false;
}

void pong::game::resetPos(ball& b)
{
	b.velocity = vel();
	b.setPosition(Playarea.width / 2, Playarea.height / 2);
}

void pong::game::resetPos(paddle& p)
{
	const auto margin = Playarea.width * .05f;
	const auto center = pos(Playarea.width / 2, Playarea.height / 2);

	if (p.id == playerid::one) {
		p.setPosition(margin - engine::paddle_size.x, center.y);
	}
	else if (p.id == playerid::two) {
		p.setPosition(Playarea.width - margin, center.y);
	}
	p.velocity = 0;
}

bool pong::game::waiting_to_serve() const noexcept
{
	return !paused && Ball.velocity == vel() && Ball.getPosition() == pos(Playarea.width / 2, Playarea.height / 2);
}

void pong::game::serve(dir direction)
{
	auto mov = engine::ball_speed;
	if (direction == dir::left) {
		mov = -mov;
	}

	Ball.setPosition(Playarea.width / 2, Playarea.height / 2);
	Ball.velocity = { mov, 0 };
}
