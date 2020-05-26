#include <string>
#include <algorithm>
#include <random>

#include <fmt/format.h>

#include "common.h"
#include "game.h"
#include "rng.h"

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
			auto reaction = random_num(20, 150);
			auto ballPos = ball.getPosition();
			auto paddlePos = p.getPosition();
			auto diff = (paddlePos - ballPos).y;
			auto spd = diff < 0 ? -diff + reaction : diff - reaction;

			auto ySpeed = spd / 100.0f;

			if (spd > ball.getGlobalBounds().height)
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

#include "imgui_ext.h"
#include <imgui-SFML.h>

pong::game::game(sf::RenderWindow& win, config_t& cfg, sf::Text const& txt)
	: window(win), config(cfg), txtScore(txt)
{
	//auto margin = sf::Vector2f(15, 20);

	playable_area = sf::FloatRect(0,0, (float)win.getSize().x, (float)win.getSize().y);
	
	// pong court
	topBorder = bottomBorder = sf::RectangleShape({ playable_area.width - 30, 25 });
	topBorder.setPosition(15, 20);
	bottomBorder.setOrigin(0, 25.f);
	bottomBorder.setPosition(sf::Vector2f(15, 20) + sf::Vector2f(0, playable_area.height - 40));
	net.setPosition(playable_area.width / 2.f, 20);

	// score
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

	tmp_config = cfg;
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
	sf::Event& event = curEvent;
	while (window.pollEvent(event)) {

		ImGui::SFML::ProcessEvent(event);

		switch (event.type)
		{
		case sf::Event::Closed:
			window.close();
			break;

		case sf::Event::KeyReleased:
		{
			if (!binding)
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


void pong::game::drawGui()
{
	// sub items primeiro
	if (show_options)
		guiOptions();

	// main menu
	guiMainmenu();
}

void pong::game::drawScore()
{
	auto Str = fmt::format("{}    {}", score.first, score.second);
	txtScore.setString(Str);
}

void pong::game::resetState()
{
	game(window, config, txtScore).swap(*this);
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

	swap(txtScore, other.txtScore);
	swap(score, other.score);

	swap(topBorder, other.topBorder);
	swap(bottomBorder, other.bottomBorder);
	swap(net, other.net);

	swap(config, other.config);
	swap(p1, other.p1);
	swap(p2, other.p2);
	swap(ball, other.ball);
}

// --- imgui funcs

// helpers
#include "convert.h"
#include <sstream>


void pong::game::guiMainmenu()
{
	// main menu
	using namespace ImScoped;
	Window menu("Main Menu", &paused);
	auto btnSize = sf::Vector2i(100, 30);

	if (ImGui::Button("Jogar", btnSize)) {
		paused = false;
	}
	if (ImGui::Button("Opcoes", btnSize)) {
		show_options = true;
	}
	if (ImGui::Button("Sair", btnSize)) {
		window.close();
		gamelog()->info("ate a proxima! ;D");
	}
}

void pong::game::guiOptions()
{
	namespace im = ImGui;
	using namespace ImScoped;

	ImGuiWindowFlags wflags = config_dirty() ? ImGuiWindowFlags_UnsavedDocument : 0;
	ImGui::SetNextWindowSize({ 500, 400 }, ImGuiCond_FirstUseEver);
	auto lastPos = im::GetWindowPos();
	im::SetNextWindowPos({ lastPos.x + 10, lastPos.y + 10 }, ImGuiCond_FirstUseEver);

	Window guiwindow("Config.", &show_options, wflags);
	if (!guiwindow)
		return;

	if (auto tabbar = TabBar("##Tabs"))
	{
		if (auto gametab = TabBarItem("Game"))
		{
			{
				ID _id_ = "paddle";
				im::Text("Paddle vars");
				im::InputFloat("Base speed", &tmp_config.paddle.base_speed);
				im::InputFloat("Acceleration", &tmp_config.paddle.accel);

				static float vec[2] = { tmp_config.paddle.size.x, tmp_config.paddle.size.y };
				if (im::InputFloat2("Size", vec)) {
					tmp_config.paddle.size.x = vec[0];
					tmp_config.paddle.size.y = vec[1];
				}
			}
			{
				ID _id_ = "ball";
				im::Text("Ball vars");
				im::InputFloat("Base speed", &tmp_config.ball.base_speed);
				im::InputFloat("Acceleration", &tmp_config.ball.accel);
				im::InputFloat("Max speed", &tmp_config.ball.max_speed);
				im::InputFloat("Radius", &tmp_config.ball.radius);
			}
			
			im::Text("Misc");
			static int fr;
			if (im::SliderInt("Framerate", &fr, 15, 144)) {
				tmp_config.framerate = (unsigned)fr;
			}
			else {
				fr = (int)tmp_config.framerate;
			}
		}
		if (auto ctrltab=TabBarItem("Controls"))
		{
			auto controlInput = [this,id=0](const char* label, sf::Keyboard::Key& curKey) mutable
			{
				std::stringstream ss;
				ss << std::setw(8) << label << ":\t" << curKey;
				auto str = ss.str();

				ID _id_ = id++;
				auto constexpr popup_id = "Rebind popup";
				ImGui::Text("%s", str.c_str());
				ImGui::SameLine(200);
				if (ImGui::Button("trocar")) {
					ImGui::OpenPopup(popup_id);
					binding = true;
				}

				if (auto popup = PopupModal(popup_id, &binding, ImGuiWindowFlags_NoMove))
				{
					ImGui::Text("Pressione uma nova tecla para '%s', ou Esc para cancelar.", label);
					if (curEvent.type == sf::Event::KeyReleased)
					{
						if (curEvent.key.code != sf::Keyboard::Escape)
						{
							curKey = curEvent.key.code;
						}

						ImGui::CloseCurrentPopup();
					}
				}
			};
			auto playerInputUI = [&](const char* player, pong::kb_keys& keys) {
				im::Text("%s:", player);
				ID _id_ = player;
				Indent _ind_{ 5.f };

				controlInput("Up", keys.up);
				controlInput("Down", keys.down);
				controlInput("Fast", keys.fast);
			};
			// ---
			auto& ctrls = tmp_config.controls;
			playerInputUI("Player 1", ctrls[0]);
			playerInputUI("Player 2", ctrls[1]);
		}
	}

	im::Spacing();
	im::Separator();

	if (im::Button("Discard")) {
		tmp_config = config;
	}
	im::SameLine();
	if (im::Button("Save") && config_dirty()) {
		config = tmp_config;
	}
}

void pong::game::guiStats()
{
}
