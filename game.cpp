#include <string>
#include <algorithm>
#include <random>

#include <fmt/format.h>

#include "common.h"
#include "game.h"
#include "rng.h"
#include "serial_map.h"

using namespace std::literals;
using pong::gamelog;

namespace
{
	auto rnd_dev = std::random_device();
	auto rnd_eng = std::default_random_engine(rnd_dev());
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

void pong::game::updatePlayers()
{
	auto players = std::array<paddle*, 2>{ &p1, &p2 };
	auto const& cfg = config.paddle;

	for (auto player : players)
	{
		auto& p = *player;
		auto& velocity = p.velocity;

		if (p.ai)
		{
			auto reaction = random_num(20, 250);
			auto ballPos = ball.getPosition();
			auto paddlePos = p.getPosition();
			auto d = (paddlePos - ballPos).y;
			auto diff = d < 0 ? -d + reaction : d - reaction;

			auto ySpeed = diff / 100.0f;

			if (diff > ball.getGlobalBounds().height)
			{
				if (ballPos.y < paddlePos.y)
				{
					velocity.y = std::clamp(-ySpeed, -cfg.base_speed * 2, 0.f);
				}
				else if (ballPos.y > paddlePos.y)
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
			auto const& c = config.controls[p.id];

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


		p.move(velocity);

		if (!p.ai)
		{
			// check colisão com bordas
			if (check_collision(&p, &topBorder) || check_collision(&p, &bottomBorder))
			{
				p.move(-velocity);
				velocity.y = 0;
			}
		}
	}

}

void pong::game::updateBall()
{
	paddle* paddle = nullptr;
	auto bounds = ball.getGlobalBounds();
	auto& velocity = ball.velocity;
	auto const& cfg = config.ball;

	
	if (bounds.intersects(topBorder.getGlobalBounds()) || bounds.intersects(bottomBorder.getGlobalBounds()))
	{
		velocity.y = -velocity.y;
	}

	if (bounds.intersects(p1.getGlobalBounds()))
		paddle = &p1;
	else if (bounds.intersects(p2.getGlobalBounds()))
		paddle = &p2;

	if (paddle)
	{
		const auto vX = velocity.x * (1.f + cfg.accel);
		const auto vY = (paddle->velocity.y != 0 ? paddle->velocity.y*0.75f : velocity.y) + random_num(-2, 2);

		velocity = {
			-std::clamp(vX, -cfg.max_speed, cfg.max_speed),
			 std::clamp(vY, -cfg.max_speed, cfg.max_speed)
		};
	}

	ball.move(velocity);

	while (paddle && check_collision(paddle, &ball))
	{
		ball.move(velocity);
	}
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

bool pong::check_collision(const sf::Shape *a, const sf::Shape *b)
{
    return a->getGlobalBounds().intersects(b->getGlobalBounds());
}

#include <imgui.h>
#include <imgui-SFML.h>

pong::game::game(sf::RenderWindow& win, config_t cfg) : window(win), config(cfg)
{
	//auto margin = sf::Vector2f(15, 20);

	playable_area = sf::FloatRect(0,0,win.getSize().x,win.getSize().y);
	
	// pong court
	topBorder = bottomBorder = sf::RectangleShape({ playable_area.width - 30, 25 });
	topBorder.setPosition(15, 20);
	bottomBorder.setOrigin(0, 25.f);
	bottomBorder.setPosition(sf::Vector2f(15, 20) + sf::Vector2f(0, playable_area.height - 40));
	net.setPosition(playable_area.width / 2.f, 20);

	// score
	ftScore.loadFromFile("C:/windows/fonts/LiberationMono-Regular.ttf");
	txtScore.setFont(ftScore);
	txtScore.setCharacterSize(55);
	txtScore.setFillColor(sf::Color::Red);
	txtScore.setPosition(playable_area.width / 2 - 100, 50);
	drawScore();

	p1.id = 0;
	p1.setSize(config.paddle.size);
	p1.setOrigin(config.paddle.size.x / 2, config.paddle.size.y / 2);
	p1.setPosition(15, playable_area.height / 2);

	p2 = p1;
	p2.ai = true;
	p2.id = 1;
	p2.setPosition(playable_area.width - 15, playable_area.height / 2);

	ball.setPosition(playable_area.width / 2, playable_area.height / 2);
	ball.setRadius(config.ball.radius);

	menu.tmp_config = cfg;
	menu.active_config = &config;
}

int pong::game::run()
{
	while (window.isOpen())
	{
		pollEvents();

		drawGame();

		if (!paused)
		{
			updatePlayers();
			updateBall();

			// check score
			if (tickcount % 30 == 0)
			{
				if (!playable_area.intersects(ball.getGlobalBounds()))
				{
					// ponto!
					if (ball.getPosition().x < 0)
					{
						// indo p/ direita, ponto player 1
						score.first++;
						serve(dir::left);
					}
					else
					{
						// indo p/ esquerda, ponto player 2
						score.second++;
						serve(dir::right);
					}

					drawScore();
				}
			}

			tickcount++;
		}
		else
		{
			ImGui::SFML::Update(window, deltaClock.restart());
			drawGui();
			ImGui::SFML::Render(window);
		}

		// display
		window.display();
	}

	return 0;
}

void pong::game::pollEvents()
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
			switch (event.key.code)
			{
			case sf::Keyboard::F1:
				p1.ai = !p1.ai;
				break;
			case sf::Keyboard::F2:
				p2.ai = !p2.ai;
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
		} break;

		}
	}

}

void pong::game::drawGame()
{
	window.clear(sf::Color::Black);
	window.draw(net);
	window.draw(topBorder);
	window.draw(bottomBorder);
	window.draw(ball);
	window.draw(p1);
	window.draw(p2);
	window.draw(txtScore);
}

static void showOptionsApp(pong::menu_state& st);

void pong::game::drawGui()
{
	// sub items primeiro
	if (menu.show_options) {
		showOptionsApp(menu);
	}

	// main menu
	ImGui::SetNextWindowSize({ 250, 0 }, ImGuiCond_FirstUseEver);
	ImGui::Begin("Main Menu", &paused);
	ImGui::PushItemWidth(ImGui::GetFontSize() * -15);

	if (ImGui::Button("Jogar")) {
		paused = false;
	}
	if (ImGui::Button("Opcoes")) {
		menu.show_options = true;
	}
	if (ImGui::Button("Sair")) {
		window.close();
		gamelog()->info("ate a proxima! ;D");
	}
	ImGui::End();
}

void pong::game::drawScore()
{
	auto Str = fmt::format("{}    {}", score.first, score.second);
	txtScore.setString(Str);
}

void pong::game::resetState()
{
	game{ window, config }.swap(*this);
	gamelog()->info("State reset");
}

void pong::game::serve(dir direction)
{
	auto mov = config.ball.base_speed;
	if (direction == dir::left) {
		mov = -mov;
	}

	ball.setPosition(playable_area.width / 2, playable_area.height / 2);
	ball.velocity = { mov, 0 };
}

void pong::game::swap(game& other) noexcept
{
	using std::swap;

	swap(paused, other.paused);
	swap(playable_area, other.playable_area);
	swap(deltaClock, other.deltaClock);
	swap(tickcount, other.tickcount);

	swap(ftScore, other.ftScore);
	swap(txtScore, other.txtScore);
	swap(score, other.score);

	swap(topBorder, other.topBorder);
	swap(bottomBorder, other.bottomBorder);
	swap(net, other.net);

	swap(config, other.config);
	swap(p1, other.p1);
	swap(p2, other.p2);
	swap(ball, other.ball);
	swap(menu, other.menu);
}

// --- imgui funcs

// helpers
#include "at_scope.h"

#define IMGUI_AREA(begin, end, ...) begin; __VA_ARGS__  end

#define UI_ID(id, ...) IMGUI_AREA(ImGui::PushID(id), ImGui::PopID(), __VA_ARGS__)
#define UI_GROUP(...) IMGUI_AREA(ImGui::BeginGroup(), ImGui::EndGroup(), __VA_ARGS__)


static void showOptionsApp(pong::menu_state& st)
{
	namespace im = ImGui;

	auto& config = st.tmp_config;
	auto p_open = &st.show_options;
	auto isDirty = [&] { return *st.active_config != st.tmp_config; };

	ImGuiWindowFlags wflags = isDirty() ? ImGuiWindowFlags_UnsavedDocument : 0;

	ImGui::SetNextWindowSize({ 500, 400 }, ImGuiCond_FirstUseEver);
	auto lastPos = im::GetWindowPos();
	im::SetNextWindowPos({ lastPos.x + 10, lastPos.y + 10 }, ImGuiCond_FirstUseEver);

	AT_SCOPE(im::End());
	if (!ImGui::Begin("Config.", p_open, wflags)) {
		return;
	}

	if (im::BeginTabBar("##Tabs"))
	{
		AT_SCOPE(im::EndTabBar());

		if (im::BeginTabItem("Game"))
		{
			AT_SCOPE(im::EndTabItem());
			
			UI_ID("paddle", {
				im::Text("Paddle vars");
				im::InputFloat("Base speed", &config.paddle.base_speed);
				im::InputFloat("Acceleration", &config.paddle.accel);
				static float vec[2] = { config.paddle.size.x, config.paddle.size.y };
				if (im::InputFloat2("Size", vec)) {
					config.paddle.size.x = vec[0];
					config.paddle.size.y = vec[1];
				}
			});

			UI_ID("ball", {
				im::Text("Ball vars");
				im::InputFloat("Base speed", &config.ball.base_speed);
				im::InputFloat("Acceleration", &config.ball.accel);
				im::InputFloat("Max speed", &config.ball.max_speed);
				im::InputFloat("Radius", &config.ball.radius);
			});

			im::Text("Misc");
			static int fr = (int)config.framerate;
			if (im::SliderInt("Framerate", &fr, 15, 144)) {
				config.framerate = (unsigned)fr;
			}
		}
		if (im::BeginTabItem("Controls"))
		{
			AT_SCOPE(im::EndTabItem());

			// TODO
			static char temp[32] = "TODO";

			UI_ID("p1", {
				im::Text("Player 1");
				if (im::InputText("Up", temp, 32))
				{
				}
				if (im::InputText("Down", temp, 32))
				{
				}
				if (im::InputText("Fast", temp, 32))
				{
				}
			});

			im::Spacing();

			UI_ID("p2", {
				im::Text("Player 2");
				if (im::InputText("Up", temp, 32))
				{
				}
				if (im::InputText("Down", temp, 32))
				{
				}
				if (im::InputText("Fast", temp, 32))
				{
				}
			});
		}
	}

	im::Separator();

	if (im::Button("Discard")) {
		config = *st.active_config;
	}
	im::SameLine();
	if (im::Button("Save") && isDirty()) {
		*st.active_config = config;
	}
}
