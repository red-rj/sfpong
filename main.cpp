#include "game.h"
#include "game_config.h"
#include "common.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>


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

	sf::RenderWindow window{ sf::VideoMode(1280, 1024), "Sf Pong!" };
	window.setFramerateLimit(config.framerate);
	ImGui::SFML::Init(window);

	auto vg = pong::game(config, window);
	sf::Clock deltaClock;

	while (window.isOpen())
	{
		ImGui::SFML::Update(window, deltaClock.restart());
		vg.update(window);

		ImGui::SFML::Render(window);
		window.display();
	}

	ImGui::SFML::Shutdown();
	pong::save_config(vg.getConfig(), "game.cfg");
	return 0;
}