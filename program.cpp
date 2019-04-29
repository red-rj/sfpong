#include <string>
#include <string_view>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <fstream>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "boost/program_options.hpp"

#include "game.h"
#include "menu.h"
#include "util.h"

namespace opt = boost::program_options;


int main()
{
    auto logger = spdlog::stderr_color_st("pong");

    // config file options

    // config storage struct
    struct {
        std::string p1_up, p1_down, p1_fast;
        std::string p2_up, p2_down, p2_fast;

        sf::Vector2f paddle_size = { 25.f, 150.f };
        float paddle_basespeed, paddle_accel;

        float ball_radius, ball_maxspeed, ball_servespeed, ball_accel;

    } config;

    opt::options_description desc;
    desc.add_options()
        ("controls.player1.up", opt::value<std::string>(&config.p1_up))
        ("controls.player1.down", opt::value<std::string>(&config.p1_down))
        ("controls.player1.fast", opt::value<std::string>(&config.p1_fast))
        ("controls.player2.up", opt::value<std::string>(&config.p2_up))
        ("controls.player2.down", opt::value<std::string>(&config.p2_down))
        ("controls.player2.fast", opt::value<std::string>(&config.p2_fast))

        ("game.paddle.base_speed", opt::value<float>(&config.paddle_basespeed)->default_value(500.f))
        ("game.paddle.accel", opt::value<float>(&config.paddle_accel)->default_value(1.f))
        //("game.paddle.size", opt::value<sf::Vector2f>(&PaddleSize))

        ("game.ball.max_speed", opt::value<float>(&config.ball_maxspeed)->default_value(5.f))
        ("game.ball.serve_speed", opt::value<float>(&config.ball_servespeed)->default_value(0.1f))
        ("game.ball.accel", opt::value<float>(&config.ball_accel)->default_value(0.05f))
        ("game.ball.radius", opt::value<float>(&config.ball_radius)->default_value(10.f))
        ;

    // read cfg
    auto cfgfile = std::fstream("game.cfg");
    opt::variables_map cfg_vm;

    auto parsed = opt::parse_config_file(cfgfile, desc, true);
    opt::store(parsed, cfg_vm);
    opt::notify(cfg_vm);

    // ----
    auto win_size = sf::Vector2u(1280, 1024);
    auto win_bounds = sf::FloatRect({ 0, 0 }, static_cast<sf::Vector2f>(win_size));
	auto margin = sf::Vector2f(15, 20);
	auto playArea = static_cast<sf::Vector2f>(win_size) - (margin * 2.f);
    bool isPlaying = false;

    sf::RenderWindow window({ win_size.x, win_size.y }, "Sf Pong!");

	// pong court
	red::court court{ { (float)playArea.x, 25.f } };

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
	sf::Text scr_txt;
	scr_txt.setFont(score_font);
	scr_txt.setCharacterSize(55);
	scr_txt.setFillColor(sf::Color::Red);
    
	red::score scores{ scr_txt };
	scores.setPosition(win_size.x / 2 - 100.f, margin.y + 30);

    // jogadores
    red::paddle p1;

    p1.setSize(config.paddle_size);
    p1.setOrigin(config.paddle_size.x / 2, config.paddle_size.y / 2);
    
    p1.accel = config.paddle_accel;
    p1.base_speed = config.paddle_basespeed;

    auto p2 = p1;

    p2.ai = true;

	p1.setPosition(margin.x, win_size.y / 2.f);
	p2.setPosition(win_size.x - margin.x, win_size.y / 2.f);

	p1.up_key	= red::parse_kb_key(config.p1_up);
	p1.down_key = red::parse_kb_key(config.p1_down);
	p1.fast_key = red::parse_kb_key(config.p1_fast);
	p2.up_key	= red::parse_kb_key(config.p2_up);
	p2.down_key = red::parse_kb_key(config.p2_down);
	p2.fast_key = red::parse_kb_key(config.p2_fast);


    red::ball ball{ config.ball_radius };
	ball.setPosition(win_size.x / 2.f, win_size.y / 2.f);
    ball.max_speed = config.ball_maxspeed;
    ball.serve_speed = config.ball_servespeed;
    ball.accel = config.ball_accel;

    // menu
    auto mainMenu = red::pong::menu(800, 600, { "Jogar", "Opções", "Sair" });
    mainMenu.setFont(score_font);

    // ajustes WIP



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
		&clock, { &p1, &p2 }, &ball, &scores, &court, &win_bounds
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
							scores.set_scores(0, 0);
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
            window.setFramerateLimit(144);

            ball.update(go);
            p1.update(go);
            p2.update(go);

            window.draw(court);
            window.draw(ball);
            window.draw(p1);
            window.draw(p2);
            window.draw(scores);
        }
        else
        {
            window.setFramerateLimit(5);
            window.draw(mainMenu);
        }

		window.display();
    }

    return EXIT_SUCCESS;
}
