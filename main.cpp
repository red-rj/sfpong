#include <spdlog/sinks/stdout_color_sinks.h>
#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
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
	using pong::G;
	pong::arguments_t params;

	auto cli = lyra::cli()
		| lyra::help(params.showHelp).description("sfPong cmd options")
		| lyra::opt(params.configFile, "game.cfg")["--config"]("arquivo config.")
		;

	auto cli_result = cli.parse({ argc, argv });
	if (!cli_result) {
		print(stderr, "CLI error: {}\n", cli_result.errorMessage());
		return 5;
	}
	
	if (params.showHelp) {
		std::cout << cli << '\n';
		return 0;
	}

	auto logger_ = spdlog::stdout_color_st("sfPong");
	spdlog::set_default_logger(logger_);
#ifndef NDEBUG
	spdlog::set_level(spdlog::level::debug);
#endif // !NDEBUG

	spdlog::debug("CWD: {}", fs::current_path().string());

	// game instance
	spdlog::info("setting-up game");
	try {
		G = new pong::game_instance(params);
	}
	catch (int e) {
		return e;
	}
	ImGui::SFML::Init(G->window);
	menu::init();

	int ec = pong::main();

	ImGui::SFML::Shutdown();
	delete G;

	return ec;
}