#include "game.h"
#include "game_config.h"
#include "common.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/info_parser.hpp>

namespace fs = std::filesystem;
using namespace std::literals;


int main()
{
	auto logger = spdlog::stdout_color_st(pong::LOGGER_NAME);

	pong::cfgtree guts, gamecfg;
	try
	{
		read_ini("game.cfg", gamecfg);
	}
	catch (const std::exception& e)
	{
		logger->error("Failed to load config: {}", e.what());
		logger->info("Using defaults");
	}

	auto gutsfile = gamecfg.get(pong::ckey::GUTSFILE, "guts.info");

	try
	{
		read_info(gutsfile, guts);
	}
	catch (const std::exception& e)
	{
		logger->info("No guts loaded: {}", e.what());
	}


	if (!gamecfg.empty()) try
	{
		pong::applyConfig(gamecfg);
	}
	catch (std::exception& e)
	{
		logger->error("applyConfig failed! {}", e.what());
		return 5;
	}

	if (!guts.empty())
		pong::overrideGuts(guts);


	// ---
	unsigned const framelimit = guts.get("framerate_limit", 60u);
	// ---
	sf::RenderWindow window{ sf::VideoMode(1280, 1024), "Sf Pong!" };
	window.setFramerateLimit(framelimit);
	ImGui::SFML::Init(window);

	auto vg = pong::game(window);
	sf::Clock deltaClock;

	while (window.isOpen())
	{
		ImGui::SFML::Update(window, deltaClock.restart());

		vg.update(window);

		ImGui::SFML::Render(window);
		window.display();
	}

	ImGui::SFML::Shutdown();
	
	auto vgconfig = vg.getConfig();
	if (vgconfig != config)
		pong::save_config(vgconfig, "game.cfg");

	return 0;
}