#include "game.h"
#include "game_config.h"
#include "common.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
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

#ifndef NDEBUG
	spdlog::set_level(spdlog::level::debug);
#endif // DEBUG
	auto logger = spdlog::stdout_color_st(pong::LOGGER_NAME);


	pong::cfgtree guts, gamecfg;
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
		pong::applyConfig(gamecfg);
		guts = pong::overrideGuts(guts_file);
	}
	catch (std::exception& e)
	{
		logger->error("Game setup failed! {}", e.what());
		return 5;
	}

	unsigned const framelimit = guts.get("framerate_limit", 60u);
	// ---
	sf::RenderWindow window{ sf::VideoMode(1280, 1024), "Sf Pong!" };
	window.setFramerateLimit(framelimit);
	ImGui::SFML::Init(window);
	
	logger->debug("PWD: {}", fs::current_path().string());

	logger->info("carregando tff");
	sf::Font sansFont, monoFont;
	sansFont.loadFromFile("support\\liberation-sans.ttf");
	monoFont.loadFromFile("support\\liberation-mono.ttf");
	pong::sansFont = &sansFont;
	pong::monoFont = &monoFont;

	auto& io = ImGui::GetIO();
	io.Fonts->Clear();
	auto* imfont = io.Fonts->AddFontFromFileTTF("support\\liberation-sans.ttf", 18.f);
	io.Fonts->AddFontDefault();

	ImGui::SFML::UpdateFontTexture();

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