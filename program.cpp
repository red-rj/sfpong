#include <string>
#include <string_view>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <imgui-SFML.h>

#include "common.h"
#include "game.h"

namespace fs = std::filesystem;

int main()
{
    auto logger = spdlog::stderr_color_st(pong::LOGGER_NAME);

    pong::config_t config;
    try
    {
        config = pong::load_config("game.cfg");
    }
    catch (const std::exception& e)
    {
        logger->error("Failed to load config '{}'", e.what());
        logger->info("Using defaults");
    }

    // ----
    sf::RenderWindow window(sf::VideoMode(1280, 1024), "Sf Pong!");
    window.setFramerateLimit(config.framerate);

    // imgui menu
    ImGui::SFML::Init(window);

    auto result = pong::run_game(&window, &config);

    ImGui::SFML::Shutdown();

    pong::save_config(config, "game.cfg");
    return result;
}
