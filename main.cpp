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


int main(int argcount, const char* args[])
{
	fs::path guts_file, config_file = "game.cfg";
	bool show_help=false;

	auto cli = lyra::cli()
		| lyra::help(show_help)
		| lyra::opt(config_file, "arquivo")["--config"]("arquivo config (padrão: game.cfg).")
		| lyra::opt(guts_file, "arquivo")["--guts"]("usar arquivo GUTS.")
		;

	auto logger = spdlog::stdout_color_st("sfPong");
	spdlog::set_default_logger(logger);
#ifndef NDEBUG
	spdlog::set_level(spdlog::level::debug);
#endif // DEBUG

	auto cli_result = cli.parse({ argcount, args });
	if (!cli_result) {
		print("CLI error: {}\n", cli_result.errorMessage());
	}
	else if (show_help) {
		std::cout << cli << '\n';
		return 0;
	}

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
		
		if (fs::exists(guts_file)) {
			logger->debug("GUTS file: {}", guts_file.string());
			read_info(guts_file.string(), guts);
			pong::overrideGuts(guts);
		}
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
	sansFont.loadFromFile(pong::files::sans_tff);
	monoFont.loadFromFile(pong::files::mono_tff);
	
	{
		auto& io = ImGui::GetIO();
		io.Fonts->Clear();
		const auto ui_font_size = 18.f;
		io.Fonts->AddFontFromFileTTF(pong::files::sans_tff, ui_font_size);
		io.Fonts->AddFontFromFileTTF(pong::files::mono_tff, ui_font_size);
		
		ImGui::SFML::UpdateFontTexture();
	}

	pong::setup_game();

	auto vg = pong::game(window);
	sf::Clock deltaClock;

	while (window.isOpen())
	{
		vg.pollEvents(window, deltaClock.restart());
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