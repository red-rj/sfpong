#include <string>
#include <string_view>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <fstream>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "boost/program_options.hpp"

#include "common.h"
#include "game.h"
#include "menu.h"
#include "util.h"
#include "game_config.h"

namespace po = boost::program_options;

int main()
{
    auto logger = spdlog::stderr_color_st(red::LOGGER_NAME);
    atexit([] {
        std::puts("exit trap");
    });

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
	red::court court{ { (float)win_size.x - margin.x * 2, 25.f } };

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
    
	red::score scores{ score_txt };
	scores.setPosition(win_size.x / 2 - 100.f, margin.y + 30);

    // jogadores
    red::paddle p1;

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


    red::ball ball{ config.ball.radius };
	ball.setPosition(win_size.x / 2.f, win_size.y / 2.f);
    ball.max_speed = config.ball.max_speed;
    ball.serve_speed = config.ball.base_speed;
    ball.accel = config.ball.accel;

    // menu
    auto mainMenu = red::pong::menu(800, 600, { "Jogar", "Opções", "Sair" });
    mainMenu.setFont(score_font);

    // ajustes WIP

    // WIP: velocidade da bola depende da taxa de frames :/
    window.setFramerateLimit(config.framerate);
    auto& playable_bounds = win_bounds;

    // -------

    auto onMainMenuItem = [&](int i) {
        switch (i)
        {
        case 0: // Jogar
            logger->info("Jogar");
            isPlaying = !isPlaying;
            break;
        case 1: // Opções
            logger->info("Opções");
            logger->warn("Não implementado...");
            // WIP
            break;
        case 2: // Sair
            window.close();
            break;
        }
    };
	
    // padrao
	const auto def_players = std::make_pair(p1, p2);
	const auto def_ball = ball;


    sf::Clock clock;
    sf::FloatRect visibleArea;

	red::game_objs go = {
		{ &p1, &p2 }, &ball, &scores, &court, &win_bounds
	};

    while(window.isOpen()) {
        
        sf::Event event;
        while(window.pollEvent(event)) {
            
            switch (event.type)
            {
                case sf::Event::Closed:
                    window.close();
                    break;
				
                case sf::Event::KeyPressed:
				{
                    if (!isPlaying) break;

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
                            break;
					}
				} break;

                case sf::Event::KeyReleased:
                {
                    if (isPlaying) break;

                    switch (event.key.code)
                    {
                    case sf::Keyboard::Up:
                        mainMenu.moveUp();
                        break;
                    case sf::Keyboard::Down:
                        mainMenu.moveDown();
                        break;
                    case::sf::Keyboard::Return:
                        onMainMenuItem(mainMenu.selected());
                    }
                } break;

				case sf::Event::Resized:
                    visibleArea.width = (float)event.size.width;
                    visibleArea.height = (float)event.size.height;
					window.setView(sf::View(visibleArea));
					break;

                case sf::Event::MouseMoved:
                {
                    if (!isPlaying)
                    {
                        mainMenu.deselect();
                        int idx = 0;
                        
                        for (auto& i : mainMenu)
                        {
                            if (i.getGlobalBounds().contains((float)event.mouseMove.x, (float)event.mouseMove.y))
                            {
                                mainMenu.select(idx);
                                break;
                            }
                            
                            idx++;
                        }
                    }
                } break;

                case sf::Event::MouseButtonReleased:
                {
                    if (!isPlaying)
                    {
                        if (event.mouseButton.button == sf::Mouse::Left)
                        {
                            onMainMenuItem(mainMenu.selected());
                        }
                    }
                } break;
            }            
        }

        window.clear(sf::Color::Black);

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

            window.draw(court);
            window.draw(ball);
            window.draw(p1);
            window.draw(p2);
            window.draw(scores);
        }
        else
        {
            window.draw(mainMenu);
        }

		window.display();
        go.tickcount++;
    }

    return EXIT_SUCCESS;
}
