#include <string>
#include <algorithm>
#include <random>
#include <array>
#include <stack>
#include <optional>

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



void pong::paddle::update(game_objs& go)
{

	if (ai)
	{
        auto reaction = random_num(20, 250);
        auto ballPos = go.ball->getPosition();
        auto paddlePos = getPosition();
        auto d = (paddlePos - ballPos).y;
        auto diff = d < 0 ? -d + reaction : d - reaction;

        auto ySpeed = diff / 100.0f;

        if (diff > go.ball->getGlobalBounds().height)
        {
            if (ballPos.y < paddlePos.y)
            {
                velocity.y = std::clamp(-ySpeed, -base_speed * 2, 0.f);
            }
            else if (ballPos.y > paddlePos.y)
            {
                velocity.y = std::clamp(ySpeed, 0.f, base_speed * 2);
            }
        }
        else
        {
            velocity.y = 0;
        }
    }
	else // player
	{
		auto offset = base_speed * accel;


		if (sf::Keyboard::isKeyPressed(up_key))
		{
            velocity.y -= offset;
		}
		else if (sf::Keyboard::isKeyPressed(down_key))
		{
            velocity.y += offset;
        }
        else
        {
            velocity.y *= 0.5f;
        }

		bool moving = velocity != sf::Vector2f();

        if (moving && sf::Keyboard::isKeyPressed(fast_key))
        {
            velocity.y *= 1.25f;
        }
	}

    auto oldPos = getPosition();

    move(velocity);

    // check colisão com bordas
    if (check_collision(this, go.court))
    {
        velocity.y = 0;
        setPosition(oldPos);
    }
}

void pong::ball::update(game_objs& go)
{
	paddle* paddle = nullptr;
	auto bounds = getGlobalBounds();

	if (bounds.intersects(go.players.first->getGlobalBounds()))
	{
		paddle = go.players.first;
	}
	else if (bounds.intersects(go.players.second->getGlobalBounds()))
	{
		paddle = go.players.second;
	}
	
	if (bounds.intersects(go.court->top.getGlobalBounds()) || bounds.intersects(go.court->bottom.getGlobalBounds()))
	{
		velocity.y = -velocity.y;
	}

	if (paddle)
	{
		const auto vX = velocity.x * (1.f + accel);
		const auto vY = (paddle->velocity.y != 0 ? paddle->velocity.y*0.75f : velocity.y) + random_num(-2, 2);

		velocity = {
			-std::clamp(vX, -max_speed, max_speed),
			 std::clamp(vY, -max_speed, max_speed)
		};
		
		const auto desloc = (velocity.x < 0 ? -paddle->getSize().x : paddle->getSize().x) * 0.15f;

		// mover bola até não intersectar mais no paddle, isso evita q
		// ela fique presa
		while (check_collision(paddle, this)) {
			move(desloc, 0);
		}
	}

	move(velocity);
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

bool pong::check_collision(const sf::Shape * a, const sf::Shape * b)
{
    return a->getGlobalBounds().intersects(b->getGlobalBounds());
}

#include <imgui.h>
#include <imgui-SFML.h>

pong::game::game(sf::RenderWindow& win, config_t cfg) : window(win), config(cfg)
{
	//auto margin = sf::Vector2f(15, 20);

	playable_area = sf::FloatRect(0, 0, 1280, 1024);
	
	// pong court
	topBorder = bottomBorder = sf::RectangleShape({ playable_area.width - 15 * 2, 25 });
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

	p1.setSize(config.paddle.size);
	p1.setOrigin(config.paddle.size.x / 2, config.paddle.size.y / 2);
	p1.setPosition(15, playable_area.height / 2);

	p2 = p1;
	p2.ai = true;
	p2.setPosition(playable_area.width - 15, playable_area.height / 2);

	ball.setPosition(playable_area.width / 2, playable_area.height / 2);
	ball.setRadius(config.ball.radius);

	// imgui menu (branch imguifix no vcpkg)
	ImGui::SFML::Init(window);
	auto& io = ImGui::GetIO();
	io.IniFilename = nullptr;
}

int pong::game::run()
{
	while (window.isOpen())
	{
		pollEvents();

		if (!paused)
		{
			ball.update();
			p1.update();
			p2.update();

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
						ball.velocity = { -config.ball.accel, 0 };
					}
					else
					{
						// indo p/ esquerda, ponto player 2
						score.second++;
						ball.velocity = { config.ball.accel, 0 };
					}

					ball.setPosition(playable_area.width / 2, playable_area.height / 2);
				}
			}

			drawGame();
			tickcount++;
		}
		else
		{
			ImGui::SFML::Update(window, deltaClock.restart());
			drawGui();
		}

		// display
		ImGui::SFML::Render(window);
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


static auto keyfrominput(const char* label, char* buff) {
	auto key = sf::Keyboard::Unknown;
	if (ImGui::InputText(label, buff, 32)) {
		key = pong::parseKey(buff);
		if (key == sf::Keyboard::Unknown) {
			gamelog()->warn("Unknown key '{}'", buff);
		}
	}
	return key;
}

static void showOptionsApp(bool* p_open, pong::config_t& cfg)
{
	namespace im = ImGui;

	// cópia de trabalho
	static pong::config_t config;
	static bool apply = false, firstrun=true;

	if (firstrun) {
		config = cfg;
	}

	auto isDirty = [&] { return config != cfg; };
	
	ImGuiWindowFlags wflags = isDirty() ? ImGuiWindowFlags_UnsavedDocument : 0;

	ImGui::SetNextWindowSize({ 500, 400 }, ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Config.", p_open, wflags)) {
		ImGui::End();
		return;
	}

	im::BeginTabBar("##Tabs");
	if (im::BeginTabItem("Game"))
	{
		im::Text("Paddle vars");
		
		im::InputFloat("Base speed", &config.paddle.base_speed);
		im::InputFloat("Acceleration", &config.paddle.accel);

		static float vec[2] = { config.paddle.size.x, config.paddle.size.y };
		if (im::InputFloat2("Size", vec)) {
			config.paddle.size.x = vec[0];
			config.paddle.size.y = vec[1];
		}

		im::Text("Ball vars");
		im::InputFloat("Base speed", &config.ball.base_speed);
		im::InputFloat("Acceleration", &config.ball.accel);
		im::InputFloat("Max speed", &config.ball.max_speed);
		im::InputFloat("Radius", &config.ball.radius);

		im::Text("Misc");
		static int fr = (int)config.framerate;
		if (im::SliderInt("Framerate", &fr, 10, 144)) {
			config.framerate = (unsigned)fr;
		}
	}
	if (im::BeginTabItem("Controls"))
	{
		static char temp[32];

		im::Text("Player 1");

		if (keyfrominput("Up", temp) != sf::Keyboard::Unknown)
		{
		}
		if (keyfrominput("Down", temp) != sf::Keyboard::Unknown)
		{
		}
		if (keyfrominput("Fast", temp) != sf::Keyboard::Unknown)
		{
		}

		im::Spacing();

		im::Text("Player 2");

		if (keyfrominput("Up", temp) != sf::Keyboard::Unknown)
		{
		}
		if (keyfrominput("Down", temp) != sf::Keyboard::Unknown)
		{
		}
		if (keyfrominput("Fast", temp) != sf::Keyboard::Unknown)
		{
		}

	}

	im::EndTabBar();
	im::Separator();

	if (im::Button("Discard")) {
		config = cfg;
	}
	im::SameLine();
	if (im::Button("Save") && isDirty()) {
		cfg = config;
	}

	ImGui::End();
}

void pong::game::drawGui()
{
	// sub items primeiro
	if (menu.show_options) {
		showOptionsApp(&menu.show_options, config);
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
		gamelog()->warn("Nao implementado...");
	}
	if (ImGui::Button("Sair")) {
		window.close();
		gamelog()->info("ate a proxima! ;D");
	}
	ImGui::End();
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

void pong::game::swap(game& other)
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
