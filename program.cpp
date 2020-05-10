#include <string>
#include <string_view>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <fstream>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "boost/program_options.hpp"
#include <imgui.h>
#include "imgui-SFML.h"

#include "common.h"
#include "game.h"
#include "ci_string.h"
#include "game_config.h"

namespace po = boost::program_options;

int main()
{
    auto logger = spdlog::stderr_color_st(red::LOGGER_NAME);

    red::pong::config_t config;
    try
    {
        config = red::pong::load_config();
    }
    catch (const std::exception& err)
    {
        logger->error("failed to load game config: {}", err.what());
        return 5;
    }

    // ----
    auto win_size = sf::Vector2u(1280, 1024);
    auto win_bounds = sf::FloatRect({ 0, 0 }, static_cast<sf::Vector2f>(win_size));
	auto margin = sf::Vector2f(15, 20);
    bool isPlaying = false;

    sf::RenderWindow window({ win_size.x, win_size.y }, "Sf Pong!");

	// pong court
	red::pong::court court{ { (float)win_size.x - margin.x * 2, 25.f } };

	court.top.setPosition(margin);
	court.bottom.setOrigin(0, 25.f);
	court.bottom.setPosition(margin + sf::Vector2f{0, win_size.y - margin.y * 2});
	court.net.setPosition({ win_size.x / 2.f, margin.y });

    sf::Font score_font;
	if (!score_font.loadFromFile("resources/LiberationMono-Regular.ttf"))
	{
		logger->error("Font not found");
		return EXIT_FAILURE;
	}

    // placar
	sf::Text score_txt;
    score_txt.setFont(score_font);
    score_txt.setCharacterSize(55);
    score_txt.setFillColor(sf::Color::Red);
    
	red::pong::score scores{ score_txt };
	scores.setPosition(win_size.x / 2 - 100.f, margin.y + 30);

    // jogadores
    red::pong::paddle p1;

    p1.setSize(config.paddle.size);
    p1.setOrigin(config.paddle.size.x / 2, config.paddle.size.y / 2);
    
    p1.accel = config.paddle.accel;
    p1.base_speed = config.paddle.base_speed;

    auto p2 = p1;
    p2.ai = true;

	p1.setPosition(margin.x, win_size.y / 2.f);
	p2.setPosition(win_size.x - margin.x, win_size.y / 2.f);

    using namespace red::pong::player_id;
    p1.up_key   = config.controls[player_1].up;
	p1.down_key = config.controls[player_1].down;
	p1.fast_key = config.controls[player_1].fast;
	p2.up_key	= config.controls[player_2].up;
	p2.down_key = config.controls[player_2].down;
	p2.fast_key = config.controls[player_2].fast;


    red::pong::ball ball{ config.ball.radius };
	ball.setPosition(win_size.x / 2.f, win_size.y / 2.f);
    ball.max_speed = config.ball.max_speed;
    ball.serve_speed = config.ball.base_speed;
    ball.accel = config.ball.accel;

    window.setFramerateLimit(config.framerate);
    auto& playable_bounds = win_bounds;

    // padroes
	const auto def_players = std::make_pair(p1, p2);
	const auto def_ball = ball;

    // imgui menu (branch imguifix no vcpkg)
    sf::Clock clock;
    ImGui::SFML::Init(window);
    auto& gui_io = ImGui::GetIO();
    gui_io.IniFilename = nullptr;


	red::pong::game_objs go = {
		{ &p1, &p2 }, &ball, &scores, &court, &win_bounds
	};

    while(window.isOpen()) {
        
        sf::Event event;
        while(window.pollEvent(event)) {
            
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
							logger->info("P1 Ai={}", p1.ai);
							break;
						case sf::Keyboard::F2:
                            p2.ai = !p2.ai;
							logger->info("P2 Ai={}", p2.ai);
							break;
						case sf::Keyboard::Enter:
							ball = def_ball;
							ball.velocity = { -ball.serve_speed, 0 };
							logger->info("Ball serve");
							break;
						case sf::Keyboard::F12:
							p1 = def_players.first;
							p2 = def_players.second;
							ball = def_ball;
							scores.set(0, 0);
							logger->info("State reset");
							break;
                        case sf::Keyboard::Escape:
                            isPlaying = !isPlaying;
                            gui_io.WantCaptureKeyboard = !isPlaying;
                            gui_io.WantCaptureMouse = !isPlaying;
                            break;
					}
				} break;

				case sf::Event::Resized:
                {
                    sf::FloatRect visibleArea{ 0, 0, (float)event.size.width, (float)event.size.height };
					window.setView(sf::View(visibleArea));
                } break;

                case sf::Event::MouseMoved:
                {
                } break;

                case sf::Event::MouseButtonReleased:
                {
                } break;
            }            
        }

        window.clear(sf::Color::Black);
        window.draw(court);
        window.draw(ball);
        window.draw(p1);
        window.draw(p2);
        window.draw(scores);

        ImGui::SFML::Update(window, clock.restart());

        if (isPlaying)
        {
            ball.update(go);
            p1.update(go);
            p2.update(go);

            // check score
            if (go.tickcount % 30 == 0)
            {
                if (!playable_bounds.intersects(ball.getGlobalBounds()))
                {
                    // ponto!
                    if (ball.getPosition().x < 0)
                    {
                        // indo p/ direita, ponto player 1
                        scores.add(1, 0);
                        ball.velocity = { -config.ball.accel, 0 };
                    }
                    else
                    {
                        // indo p/ esquerda, ponto player 2
                        scores.add(0, 1);
                        ball.velocity = { config.ball.accel, 0 };
                    }

                    ball.setPosition(playable_bounds.width / 2, playable_bounds.height / 2);
                }
            }
            go.tickcount++;
        }
        else
        {
            // menu
            ImGui::Begin("sfPong menu");
            ImGui::SetWindowSize({150, 0}, ImGuiCond_FirstUseEver);
            ImGui::PushItemWidth(ImGui::GetFontSize() * -15);

            if (ImGui::Button("Jogar")) {
                isPlaying = true;
            }
            if (ImGui::Button("Opcoes")) {
                logger->warn("Nao implementado...");
            }
            if (ImGui::Button("Sair")) {
                window.close();
            }
            ImGui::End();
        }


        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();

    return EXIT_SUCCESS;
}
