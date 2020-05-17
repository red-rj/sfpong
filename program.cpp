#include <string>
#include <string_view>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <imgui.h>
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
        config.load("game.cfg");
    }
    catch (const std::exception& e)
    {
        logger->error("Failed to load config '{}'", e.what());
        return 5;
    }
    // ----
    auto scoreFont = sf::Font();
    scoreFont.loadFromFile("C:/windows/fonts/LiberationMono-Regular.ttf");
    auto scoreTxt = sf::Text("", scoreFont, 55);

    sf::RenderWindow window(sf::VideoMode(1280, 1024), "Sf Pong!");
    window.setFramerateLimit(config.framerate);

    // imgui menu (branch imguifix no vcpkg)
    ImGui::SFML::Init(window);
    auto& gui_io = ImGui::GetIO();
    //gui_io.IniFilename = nullptr;

    pong::game vg{ window, config, scoreTxt };
    auto result = vg.run();

    ImGui::SFML::Shutdown();
    config.save("game.cfg");

    return result;
}
