#include <string>
#include <string_view>
#include <algorithm>
#include <numeric>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "game.h"

// CONFIG


int main(int , char const ** )
{
	auto logger = spdlog::stderr_color_st("gamelog");

	sf::RenderWindow window({1280, 1024}, "Sf Pong!");
	auto win_size = window.getSize();
	auto win_bounds = sf::FloatRect({ 0,0 }, static_cast<sf::Vector2f>(win_size));

	auto margin = sf::Vector2f(15, 20);
	auto playArea = win_size - sf::Vector2u{ 30, 40 };
	
	// pong court
	red::court court = {
		{ (float)playArea.x, 25.f },
		{ (float)playArea.x, 25.f },
	};

	court.top.setPosition(margin);
	court.bottom.setOrigin(0, 25.f);
	court.bottom.setPosition(margin + sf::Vector2f{0, win_size.y - margin.y * 2});
	court.net.setPosition({ win_size.x / 2.f, margin.y });

	auto score_font = sf::Font();
	if (!score_font.loadFromFile("resources/LiberationMono-Regular.ttf"))
	{
		logger->error("Font not found");
		return EXIT_FAILURE;
	}

	sf::Text scr_txt;
	scr_txt.setFont(score_font);
	scr_txt.setCharacterSize(55);
	scr_txt.setFillColor(sf::Color::Red);

	red::score scores{ scr_txt };
	scores.set_padding(4);
	scores.setPosition(win_size.x / 2 - 100.f, margin.y + 30);

	red::paddle p1(false);
	red::paddle p2(true);
	red::ball ball;

	p1.setPosition(margin.x, win_size.y / 2.f);
	p2.setPosition(win_size.x - margin.x, win_size.y / 2.f);

	p1.up_key	= sf::Keyboard::W;
	p1.down_key = sf::Keyboard::S;
	p1.fast_key = sf::Keyboard::LShift;
	p2.up_key	= sf::Keyboard::Up;
	p2.down_key = sf::Keyboard::Down;
	p2.fast_key = sf::Keyboard::RControl;

	ball.setPosition(win_size.x / 2.f, win_size.y / 2.f);

	// padrao
	const auto def_players = std::make_pair(p1, p2);
	const auto def_ball = ball;

    // ajustes
    


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
					switch (event.key.code)
					{
						case sf::Keyboard::F1:
							p1.setAi(!p1.isAi());
							logger->info("P1 Ai={}", p1.isAi());
							break;
						case sf::Keyboard::F2:
							p2.setAi(!p2.isAi());
							logger->info("P2 Ai={}", p2.isAi());
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
					}
				} break;
				case sf::Event::Resized:
					sf::FloatRect visibleArea(0, 0, (float)event.size.width, (float)event.size.height);
					window.setView(sf::View(visibleArea));
					break;
            }            
        }

		ball.update(go);
		p1.update(go);
		p2.update(go);

        // drawing
        window.clear(sf::Color::Black);

		window.draw(court);
		window.draw(ball);
		window.draw(p1);
		window.draw(p2);
		window.draw(scores);

		window.display();
    }

    return EXIT_SUCCESS;
}
