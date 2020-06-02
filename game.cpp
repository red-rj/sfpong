#include <string>
#include <algorithm>
#include <random>

#include <fmt/format.h>

#include "common.h"
#include "game.h"
#include "rng.h"

#include "imgui_ext.h"
#include <imgui-SFML.h>

using namespace std::literals;

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

// GAME
using namespace pong;

namespace
{
	pong::game_state G;
	pong::menu_state M;

	// game entities
	// score
	struct {
		sf::Text text; sf::Font font;
		std::pair<short, short>* const value = &G.score;
	} Score;

	// court
	struct {
		sf::RectangleShape top, bottom;
		pong::net_shape net;
	} Court;
	
	// moving parts
	pong::paddle Player1, Player2;
	pong::ball Ball;
}

bool menu_state::configDirty() noexcept
{
	return tmp_config != *G.config;
}



void pollEvents();
void updatePlayer(pong::paddle& p);
void updateBall();

void drawGui();
void guiOptions();
void guiStats();

void serve(dir direction);
void resetState();

void checkScore() {
	// check score
	if (!G.playable_area.intersects(Ball.getGlobalBounds()))
	{
		// ponto!
		if (Ball.getPosition().x < 0)
		{
			// indo p/ direita, ponto player 1
			G.score.first++;
			serve(dir::left);
		}
		else
		{
			// indo p/ esquerda, ponto player 2
			G.score.second++;
			serve(dir::right);
		}

		auto Str = fmt::format("{}    {}", G.score.first, G.score.second);
		Score.text.setString(Str);
	}
}
// ---

int pong::run_game(sf::RenderWindow* win, config_t* cfg, cmdline_options const&)
{
	// init globais
	G.window = win;
	G.config = cfg;
	M.tmp_config = *cfg;

	G.playable_area = sf::FloatRect(0, 0, (float)G.window->getSize().x, (float)G.window->getSize().y);

	resetState();

	// locais
	sf::Clock deltaClock;
	uint64_t tickcount = 0;

	auto& io = ImGui::GetIO();
	io.WantCaptureKeyboard = G.paused;
	io.WantCaptureMouse = G.paused;

	while (G.window->isOpen())
	{
		pollEvents();
		
		G.window->clear();
		G.window->draw(Ball);
		G.window->draw(Player1);
		G.window->draw(Player2);
		G.window->draw(Court.net);
		G.window->draw(Court.top);
		G.window->draw(Court.bottom);
		G.window->draw(Score.text);

		ImGui::SFML::Update(*G.window, deltaClock.restart());

		if (!G.paused)
		{
			updatePlayer(Player1);
			updatePlayer(Player2);
			updateBall();

			if (tickcount % 30 == 0) {
				checkScore();
			}
			tickcount++;
		}
		else
		{
			drawGui();
		}

		if (M.show_stats) {
			guiStats();
		}
		ImGui::SFML::Render(*G.window);

		G.window->display();
	}

	return 0;
}

void updatePlayer(paddle& p)
{
	auto const& cfg = G.config->paddle;
	auto& velocity = p.velocity;

	if (p.ai)
	{
		auto reaction = random_num(20, 150);
		auto ballPos = Ball.getPosition();
		auto paddlePos = p.getPosition();
		auto diff = (paddlePos - ballPos).y;
		auto spd = diff < 0 ? -diff + reaction : diff - reaction;

		auto ySpeed = spd / 100.0f;

		if (spd > Ball.getGlobalBounds().height)
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
		auto const& c = G.config->controls[p.id];

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
		if (check_collision(&p, &Court.top) || check_collision(&p, &Court.bottom))
		{
			p.move(-velocity);
			velocity.y = 0;
		}
	}
}

void updateBall()
{
	using pong::check_collision;
	using pong::random_num;

	pong::paddle* paddle = nullptr;
	auto bounds = Ball.getGlobalBounds();
	auto& velocity = Ball.velocity;
	auto const& cfg = G.config->ball;

	
	if (check_collision(&Ball, &Court.top) || check_collision(&Ball, &Court.bottom))
	{
		velocity.y = -velocity.y;
	}

	if (bounds.intersects(Player1.getGlobalBounds()))
		paddle = &Player1;
	else if (bounds.intersects(Player2.getGlobalBounds()))
		paddle = &Player2;

	if (paddle)
	{
		const auto vX = velocity.x * (1.f + cfg.accel);
		const auto vY = (paddle->velocity.y != 0 ? paddle->velocity.y*0.75f : velocity.y) + random_num(-2, 2);

		velocity = {
			-std::clamp(vX, -cfg.max_speed, cfg.max_speed),
			 std::clamp(vY, -cfg.max_speed, cfg.max_speed)
		};
	}

	Ball.move(velocity);

	while (paddle && check_collision(paddle, &Ball))
	{
		Ball.move(velocity);
	}
}

void pollEvents()
{
	sf::Event& event = G.lastEvent;
	while (G.window->pollEvent(event)) {

		ImGui::SFML::ProcessEvent(event);

		switch (event.type)
		{
		case sf::Event::Closed:
			G.window->close();
			break;

		case sf::Event::KeyReleased:
		{
			if (!M.rebinding)
			{
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
					G.paused = !G.paused;
					// imgui deve capturar input só com o jogo pausado
					auto& io = ImGui::GetIO();
					io.WantCaptureKeyboard = G.paused;
					io.WantCaptureMouse = G.paused;
					break;
				}
			}
		} break;

		case sf::Event::Resized:
		{
			sf::FloatRect visibleArea{ 0, 0, (float)event.size.width, (float)event.size.height };
			G.window->setView(sf::View(visibleArea));
		} break;

		}
	}

}


void resetState()
{
	// drawables
	//auto margin = sf::Vector2f(15, 20);

	// pong court
	Court.top = Court.bottom = sf::RectangleShape({ G.playable_area.width - 30, 25 });
	Court.top.setPosition(15, 20);
	Court.bottom.setOrigin(0, 25.f);
	Court.bottom.setPosition(sf::Vector2f(15, 20) + sf::Vector2f(0, G.playable_area.height - 40));
	Court.net.setPosition(G.playable_area.width / 2.f, 20);

	// score
	Score.font.loadFromFile("C:/windows/fonts/LiberationMono-Regular.ttf");
	Score.text = sf::Text("", Score.font, 55);
	Score.text.setPosition(G.playable_area.width / 2 - 100, 50);

	Player1.id = 0;
	Player1.setSize(G.config->paddle.size);
	Player1.setOrigin(G.config->paddle.size.x / 2, G.config->paddle.size.y / 2);
	Player1.setPosition(15, G.playable_area.height / 2);

	Player2 = Player1;
	Player2.ai = true;
	Player2.id = 1;
	Player2.setPosition(G.playable_area.width - 15, G.playable_area.height / 2);

	Ball.setPosition(G.playable_area.width / 2, G.playable_area.height / 2);
	Ball.setRadius(G.config->ball.radius);
}

void serve(dir direction)
{
	auto mov = G.config->ball.base_speed;
	if (direction == dir::left) {
		mov = -mov;
	}

	Ball.setPosition(G.playable_area.width / 2, G.playable_area.height / 2);
	Ball.velocity = { mov, 0 };
}

// --- imgui funcs

// helpers
#include "convert.h"
#include <sstream>


void drawGui()
{
	// sub items primeiro
	if (M.show_options)
		guiOptions();

	// main menu
	using namespace ImScoped;
	Window menu("Main Menu", &G.paused);
	auto btnSize = sf::Vector2i(100, 30);

	if (ImGui::Button("Jogar", btnSize)) {
		G.paused = false;
	}
	if (ImGui::Button("Opcoes", btnSize)) {
		M.show_options = true;
	}
	if (ImGui::Button("DEV", btnSize)) {
		M.show_stats = true;
	}
	if (ImGui::Button("Sair", btnSize)) {
		G.window->close();
		gamelog()->info("ate a proxima! ;D");
	}
}
void guiOptions()
{
	namespace im = ImGui;
	using namespace ImScoped;

	struct {
		float paddle_size[2]{};
		int framerate=0;
	} static model;
	auto& config = M.tmp_config;

	{
		ImGui::SetNextWindowSize({ 500, 400 }, ImGuiCond_FirstUseEver);
		auto lastPos = im::GetWindowPos();
		im::SetNextWindowPos({ lastPos.x + 10, lastPos.y + 10 }, ImGuiCond_FirstUseEver);
	}

	auto wflags = M.configDirty() ? ImGuiWindowFlags_UnsavedDocument : 0;
	Window guiwindow("Config.", &M.show_options, wflags);
	if (!guiwindow)
		return;

	if (auto tabbar = TabBar("##Tabs"))
	{
		if (auto gametab = TabBarItem("Game"))
		{
			{
				ID _id_ = "paddle";
				im::Text("Paddle vars");
				im::InputFloat("Base speed", &config.paddle.base_speed);
				im::InputFloat("Acceleration", &config.paddle.accel);

				if (im::InputFloat2("Size", model.paddle_size)) {
					config.paddle.size.x = model.paddle_size[0];
					config.paddle.size.y = model.paddle_size[1];
				}
				else {
					model.paddle_size[0] = config.paddle.size.x;
					model.paddle_size[1] = config.paddle.size.y;
				}
			}
			{
				ID _id_ = "ball";
				im::Text("Ball vars");
				im::InputFloat("Base speed", &config.ball.base_speed);
				im::InputFloat("Acceleration", &config.ball.accel);
				im::InputFloat("Max speed", &config.ball.max_speed);
				im::InputFloat("Radius", &config.ball.radius);
			}
			
			im::Text("Misc");
			if (im::SliderInt("Framerate", &model.framerate, 15, 144)) {
				config.framerate = (unsigned)model.framerate;
			}
			else {
				model.framerate = (int)config.framerate;
			}
		}
		if (auto ctrltab=TabBarItem("Controls"))
		{
			auto controlInput = [id=0](const char* label, sf::Keyboard::Key& curKey) mutable
			{
				auto str = fmt::format("{:8}:\t{}", label, curKey);

				ID _id_ = id++;
				auto constexpr popup_id = "Rebind popup";
				ImGui::Text("%s", str.c_str());
				ImGui::SameLine(200);
				if (ImGui::Button("trocar")) {
					ImGui::OpenPopup(popup_id);
					M.rebinding = true;
				}

				if (auto popup = PopupModal(popup_id, &M.rebinding, ImGuiWindowFlags_NoMove))
				{
					ImGui::Text("Pressione uma nova tecla para '%s', ou Esc para cancelar.", label);
					if (G.lastEvent.type == sf::Event::KeyReleased)
					{
						if (G.lastEvent.key.code != sf::Keyboard::Escape)
						{
							curKey = G.lastEvent.key.code;
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
			auto& ctrls = config.controls;
			playerInputUI("Player 1", ctrls[0]);
			playerInputUI("Player 2", ctrls[1]);
		}
	}

	im::Spacing();
	im::Separator();

	if (im::Button("Discard")) {
		config = *G.config;
	}
	im::SameLine();
	if (im::Button("Save") && M.configDirty()) {
		*G.config = config;
	}
}

void guiStats()
{
	using namespace ImScoped;

	ImGuiIO& io = ImGui::GetIO();

	auto constexpr wflags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize
		| ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing
		| ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;

	sf::Vector2f winpos = { io.DisplaySize.x - 10.f, 10.f};
	ImGui::SetNextWindowBgAlpha(0.35f);
	ImGui::SetNextWindowPos(winpos, ImGuiCond_Always, {1.f, 0});

	Window overlay("Stats", &M.show_stats, wflags);

	auto p1b = Player1.getPosition();
	auto p2b = Player2.getPosition();
	auto text = fmt::format("P1: [{}, {}]\nP2: [{}, {}]", p1b.x, p1b.y, p2b.x, p2b.y);

	ImGui::Text("%s:\n%s", "Game state", text.c_str());
}
