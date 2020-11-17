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
#include <filesystem>

namespace fs = std::filesystem;
namespace ckey = pong::ckey;
using namespace std::literals;
using fmt::print;


int main(int argc, const char* argv[])
{
	fs::path guts_file = "guts.info", config_file = "game.cfg";
	bool show_help=false, guts_arg = false;

	auto cli = lyra::cli()
		| lyra::help(show_help).description("sfPong cmd options")
		| lyra::opt(config_file, "game.cfg")["--config"]("arquivo config.")
		| lyra::opt(guts_arg)["--guts"]("usar/criar arquivo GUTS.")
		;

	auto cli_result = cli.parse({ argc, argv });
	if (!cli_result) {
		print("CLI error: {}\n", cli_result.errorMessage());
		return 5;
	}
	else if (show_help) {
		std::cout << cli << '\n';
		return 0;
	}


	auto logger = spdlog::stdout_color_st("sfPong");
	spdlog::set_default_logger(logger);
#ifndef NDEBUG
	spdlog::set_level(spdlog::level::debug);
#endif // !NDEBUG

	logger->debug("CWD: {}", fs::current_path().string());


	pong::cfgtree gamecfg;
	try
	{
		logger->info("loading config file");
		if (fs::exists(config_file))
			read_ini(config_file.string(), gamecfg);
	}
	catch (const std::exception& e)
	{
		logger->error("{}", e.what());
	}

	try
	{
		logger->info("Setting up...");
		pong::set_user_config(gamecfg);
		
		// guts
		pong::cfgtree guts;

		if (guts_arg and fs::exists(guts_file)) {
			read_info(guts_file.string(), guts);
			logger->debug("got GUTS file: {}", guts_file.string());
			pong::overrideGuts(guts);
		}
		else if (guts_arg) {
			guts = pong::createGuts();
			write_info(guts_file.string(), guts);
			logger->debug("GUTS file created: {}", fs::absolute(guts_file).string());
			logger->debug("exiting...");
			return 0;
		}
	}
	catch (std::exception& e)
	{
		logger->error("Game setup failed! {}", e.what());
		return 5;
	}

	// ---
	sf::RenderWindow window{ sf::VideoMode(1920, 1080), "Sf Pong!" };
	window.setFramerateLimit(60u);
	ImGui::SFML::Init(window);
	

	pong::game::setup(window);

	auto vg = pong::game(pong::game::mode::singleplayer);
	sf::Clock deltaClock;

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(event);
			vg.processEvent(event);
		}
		ImGui::SFML::Update(window, deltaClock.restart());

		vg.update();
		ImGui::SFML::Render(window);
		window.display();
	}

	ImGui::SFML::Shutdown();
	
	auto config = pong::get_user_config();
	if (gamecfg != config) {
		write_ini(config_file.string(), config);
	}

	return 0;
}