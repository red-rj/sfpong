#include <string>
#include <algorithm>
#include <random>

#include <fmt/format.h>
#include "spdlog/sinks/stdout_color_sinks.h"

#include "common.h"
#include "game.h"
#include "rng.h"

#include "imgui_ext.h"
#include <imgui-SFML.h>

using namespace std::literals;
namespace fs = std::filesystem;

int main()
{
	auto logger = spdlog::stdout_color_st(pong::LOGGER_NAME);
	pong::config_t config;
	try
	{
		config = pong::load_config("game.cfg");
	}
	catch (const std::exception& e)
	{
		logger->error("Failed to load config: {}", e.what());
		logger->info("Using defaults");
	}

	pong::game vg{ config };
	vg.run();
	
	return 0;
}

//---

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

bool pong::check_collision(const sf::Shape* a, const court* court)
{
	return check_collision(a, &court->top) || check_collision(a, &court->bottom);
}


void pong::physics::update()
{
	m_position += m_velocity;
}
void pong::physics::update(vel const& accel)
{
	m_velocity += accel;
	update();
}

void pong::controllable_physics::update()
{
	m_phy.update(m_accel);
	m_accel = { 0, 0 };
}

static pong::menu_state Menu;

using namespace pong;

pong::game::game(config_t cfg) : Config(std::move(cfg))
{
	Window.create(sf::VideoMode(1280, 1024), "Sf Pong!");
	Window.setFramerateLimit(Config.framerate);
	//playable_area = { {0.f,0.f}, static_cast<sf::Vector2f>(Window.getSize()) };

	Menu.config = Config;

	resetState();
}

void pong::game::run()
{
	// locais
	sf::Clock deltaClock;

	// imgui
	ImGui::SFML::Init(Window);
	auto& io = ImGui::GetIO();
	io.WantCaptureKeyboard = paused;
	io.WantCaptureMouse = paused;

	while (Window.isOpen())
	{
		pollEvents();
		Window.clear();
		Window.draw(Court);
		Window.draw(Score);
		Window.draw(Ball);
		Window.draw(Player1);
		Window.draw(Player2);

		ImGui::SFML::Update(Window, deltaClock.restart());

		if (!paused)
		{
			for (auto p : { &Player1, &Player2 })
			{
				p->update(Ball.getGlobalBounds());

				// check colisão com bordas
				if (check_collision(&Ball, &Court))
				{
					p->move(-p->velocity);
					p->velocity.y = 0;
				}

				Ball.update(*p);
			}

			// check colisão com bordas
			if (check_collision(&Ball, &Court))
			{
				Ball.velocity.y = -Ball.velocity.y;
			}

			if (tickcount % 30 == 0) {
				// check score
				if (!playable_area.intersects(Ball.getGlobalBounds()))
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

		Menu.draw(this);

		ImGui::SFML::Render(Window);
		Window.display();
	}

	ImGui::SFML::Shutdown();
	pong::save_config(Config, "game.cfg");
}

void game::pollEvents()
{
	sf::Event& event = lastEvent;
	while (Window.pollEvent(event)) {

		ImGui::SFML::ProcessEvent(event);

		switch (event.type)
		{
		case sf::Event::Closed:
			Window.close();
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
			Window.setView(sf::View(visibleArea));
			playable_area = visibleArea;
		} break;

		}
	}

}

void pong::game::resetState()
{
	// pong court
	auto area = sf::FloatRect{ {0.f,0.f}, static_cast<sf::Vector2f>(Window.getSize()) };
	Court = court(area, 25, { 15, 20 });
	playable_area = area;

	// score
	//Score = score(G.playable_area, R"(C:\Windows\Fonts\LiberationMono-Regular.ttf)", 55);
	Score.create(playable_area, R"(C:\Windows\Fonts\LiberationMono-Regular.ttf)", 55);

	Player1.id = 0;
	Player1.setSize(Config.paddle.size);
	Player1.setOrigin(Config.paddle.size.x / 2, Config.paddle.size.y / 2);
	Player1.setPosition(20, playable_area.height / 2);
	Player1.pcfg = &Config;

	Player2 = Player1;
	Player2.ai = true;
	Player2.id = 1;
	Player2.setPosition(playable_area.width - 20, playable_area.height / 2);

	Ball.setPosition(playable_area.width / 2, playable_area.height / 2);
	Ball.setRadius(Config.ball.radius);
	Ball.pcfg = &Config;

}

void pong::paddle::update()
{
	move(velocity);
}
/* precisa:
  x Config
  - Ball pos e global bounds
  - Court
*/
void paddle::update(const bounds_t& ball_bounds)
{
	auto& cfg = pcfg->paddle;

	if (ai)
	{
		auto myPos = getPosition();
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
		auto const& c = pcfg->controls[id];

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

	update();
}

/* precisa:
* x Config.ball
* - Player GlobalBounds
* - Court
*/
void pong::ball::update()
{
	move(velocity);
}

void ball::update(const paddle& player)
{
	auto const& cfg = pcfg->ball;

	if (check_collision(this, &player))
	{
		const auto vX = velocity.x * (1.f + cfg.accel);
		const auto vY = (player.velocity.y != 0 ? player.velocity.y * 0.75f : velocity.y) + random_num(-2, 2);

		velocity = {
			-std::clamp(vX, -cfg.max_speed, cfg.max_speed),
			 std::clamp(vY, -cfg.max_speed, cfg.max_speed)
		};

		do
		{
			move(velocity);
		} while (check_collision(&player, this));
	}

	update();
}

void game::serve(dir direction)
{
	auto mov = Config.ball.base_speed;
	if (direction == dir::left) {
		mov = -mov;
	}

	Ball.setPosition(playable_area.width / 2, playable_area.height / 2);
	Ball.velocity = { mov, 0 };
}



// --- imgui funcs

// helpers
#include "convert.h"
#include <fmt/ostream.h>


void pong::menu_state::draw(game* ctx)
{
	if (show_options)
		guiOptions(ctx);

	if (show_stats)
		guiStats(ctx);

	if (!ctx->paused) return;

	using namespace ImGui;
	using namespace ImScoped;

	Window menu("Menu", &ctx->paused);
	auto btnSize = sf::Vector2i(100, 30);

	if (Button("Jogar", btnSize)) {
		ctx->paused = false;
	}
	if (Button("Opcoes", btnSize)) {
		show_options = true;
	}
	if (Button("DEV", btnSize)) {
		show_stats = true;
	}
	if (Button("Sair", btnSize)) {
		ctx->Window.close();
		gamelog()->info("ate a proxima! ;D");
	}

}

void menu_state::guiOptions(game* ctx)
{
	namespace im = ImGui;
	using namespace ImScoped;

	struct {
		float paddle_size[2]{};
		int framerate=0;
	} static model;

	auto& active_config = ctx->Config;

	{
		ImGui::SetNextWindowSize({ 500, 400 }, ImGuiCond_FirstUseEver);
		auto lastPos = im::GetWindowPos();
		im::SetNextWindowPos({ lastPos.x + 10, lastPos.y + 10 }, ImGuiCond_FirstUseEver);
	}

	auto wflags = active_config != config ? ImGuiWindowFlags_UnsavedDocument : 0;
	Window guiwindow("Config.", &show_options, wflags);
	if (!guiwindow)
		return;

	if (auto tabbar = TabBar("##Tabs"))
	{
		if (auto ctrltab = TabBarItem("Controls"))
		{

			auto InputControl = [id=0,this,ctx](const char* label, sf::Keyboard::Key& curKey) mutable
			#pragma region Block
			{
				using namespace ImScoped;
				using namespace ImGui;

				ID _id_ = id++;
				auto constexpr popup_id = "Rebind popup";
				auto keystr = fmt::format("{}", curKey);

				Text("%8s:\t%s", label, keystr.c_str());
				SameLine(200);
				if (Button("Trocar")) {
					OpenPopup(popup_id);
					rebinding = true;
				}

				if (auto popup = PopupModal(popup_id, &rebinding, ImGuiWindowFlags_NoMove))
				{
					Text("Pressione uma nova tecla para '%s', ou Esc para cancelar.", label);
					if (ctx->lastEvent.type == sf::Event::KeyReleased)
					{
						if (ctx->lastEvent.key.code != sf::Keyboard::Escape)
						{
							curKey = ctx->lastEvent.key.code;
						}

						CloseCurrentPopup();
					}
				}
			};
			#pragma endregion

			auto InputPlayerCtrls = [&](const char* player, pong::kb_keys& keys) {
				im::Text("%s:", player);
				ID _id_ = player;
				Indent _ind_{ 5.f };

				InputControl("Up", keys.up);
				InputControl("Down", keys.down);
				InputControl("Fast", keys.fast);
			};

			// ---
			auto& ctrls = config.controls;
			InputPlayerCtrls("Player 1", ctrls[0]);
			InputPlayerCtrls("Player 2", ctrls[1]);
		}
		if (auto gametab = TabBarItem("Game vars"))
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
	}

	im::Spacing();
	im::Separator();

	if (im::Button("Discard")) {
		config = active_config;
	}
	im::SameLine();
	if (im::Button("Save") && active_config != config) {
		active_config = config;
	}
}

void menu_state::guiStats(game* ctx)
{
	using namespace ImScoped;

	ImGuiIO& io = ImGui::GetIO();
	pos winpos = { io.DisplaySize.x - 10.f, 15.f};
	ImGui::SetNextWindowBgAlpha(0.5f);
	ImGui::SetNextWindowPos(winpos, ImGuiCond_Always, {1.f, 0});

	auto constexpr wflags = ImGuiWindowFlags_AlwaysAutoResize
		| ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing;
	Window overlay("Stats", &show_stats, wflags);

	auto p1b = ctx->Player1.getPosition();
	auto p2b = ctx->Player2.getPosition();
	auto bb = ctx->Ball.getPosition();
	auto text = fmt::format("P1: [{:.2f}, {:.2f}]\nP2: [{:.2f}, {:.2f}]\nBall: [{:.2f}, {:.2f}]", 
		p1b.x, p1b.y, p2b.x, p2b.y, bb.x, bb.y);

	ImGui::Text("%s:\n%s", "Game state", text.c_str());
}
