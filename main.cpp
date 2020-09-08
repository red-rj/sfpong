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
#include <lyra/lyra.hpp>
#include <fmt/format.h>

namespace fs = std::filesystem;
namespace ckey = pong::ckey;
using namespace std::literals;
using fmt::print;


int main(int argcount, const char* args[])
{
	fs::path guts_file, config_file = "game.cfg";
	bool show_help=false;

	auto cli = lyra::cli()
		| lyra::help(show_help)
		| lyra::opt(config_file, "arquivo")["--config"]("arquivo config (padrão: game.cfg).")
		| lyra::opt(guts_file, "arquivo")["--guts"]("usar arquivo GUTS.")
		;

	auto cli_result = cli.parse({ argcount, args });
	if (!cli_result) {
		print(stderr, "CLI error: {}\n", cli_result.errorMessage());
	}
	else if (show_help) {
		std::cout << cli << '\n';
		return 0;
	}

	auto logger = spdlog::stdout_color_st(pong::LOGGER_NAME);

	pong::cfgtree guts, gamecfg;
	try
	{
		if (fs::exists(config_file))
			read_ini(config_file.string(), gamecfg);
	}
	catch (const std::exception& e)
	{
		logger->error("Failed to load config: {}", e.what());
	}

	try
	{
		pong::applyConfig(gamecfg);
	}
	catch (std::exception& e)
	{
		logger->error("applyConfig failed! {}", e.what());
		return 5;
	}


	try
	{
		if (fs::exists(guts_file))
			read_info(guts_file.string(), guts);
	}
	catch (const std::exception& e)
	{
		logger->info("Failed to load guts: {}", e.what());
	}

	// ---
	if (!guts.empty())
		pong::overrideGuts(guts);

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
	
	auto config = pong::getGameConfig();
	if (gamecfg != config) {
		write_ini(config_file.string(), config);
	}

	return 0;
}