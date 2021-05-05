#include <spdlog/sinks/stdout_color_sinks.h>
#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <lyra/lyra.hpp>
#include <fmt/format.h>
#include <filesystem>

#include "game.h"
#include "menu.h"
#include "common.h"

namespace fs = std::filesystem;
namespace ckey = pong::ckey;
using namespace std::literals;
using fmt::print;


int main(int argc, const char* argv[])
{
	//fs::path guts_file = "guts.info";
	fs::path config_file = "game.cfg";
	bool show_help=false, guts_arg = false;

	auto cli = lyra::cli()
		| lyra::help(show_help).description("sfPong cmd options")
		| lyra::opt(config_file, "game.cfg")["--config"]("arquivo config.")
		;

	auto cli_result = cli.parse({ argc, argv });
	if (!cli_result) {
		print(stderr, "CLI error: {}\n", cli_result.errorMessage());
		return 5;
	}
	
	if (show_help) {
		std::cout << cli << '\n';
		return 0;
	}

	auto logger_ = spdlog::stdout_color_st("sfPong");
	spdlog::set_default_logger(logger_);
	namespace log = spdlog;
#ifndef NDEBUG
	spdlog::set_level(spdlog::level::debug);
#endif // !NDEBUG

	log::debug("CWD: {}", fs::current_path().string());

	pong::game_settings gamecfg;
	try
	{
		gamecfg.load_file(config_file);
	}
	catch (const std::exception& e)
	{
		log::error("config: {}", e.what());
	}

	sf::RenderWindow window;

	try
	{
		log::info("Setting up...");

		sf::VideoMode vidmode;
		vidmode.width = gamecfg.resolution().x;
		vidmode.height = gamecfg.resolution().y;
		vidmode.bitsPerPixel = 32;

		window.create(vidmode, "Sf Pong!");
		window.setFramerateLimit(60u);
	}
	catch (std::exception& e)
	{
		log::error("Game setup failed! {}", e.what());
		return 5;
	}

	ImGui::SFML::Init(window);
	pong::menu::init(&gamecfg);

	// game instance
	auto vg = pong::game(pong::gamemode::singleplayer, gamecfg);

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			// global events
			switch (event.type)
			{
			case sf::Event::Closed:
				window.close();
				break;
			}

			ImGui::SFML::ProcessEvent(event);
			pong::menu::processEvent(event);
			vg.processEvent(event);
		}

		auto dt = vg.restart_clock();
		ImGui::SFML::Update(window, dt);

		vg.update();
		vg.draw(window);

		pong::menu::update(vg, window);
		ImGui::SFML::Render(window);
		
		window.display();
	}

	ImGui::SFML::Shutdown();
	
	gamecfg.save_file(config_file);

	return 0;
}