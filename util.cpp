#include <string>
#include <map>

#include "util.h"
#include "common.h"
#include "serial_map.h"


using KbKey = sf::Keyboard::Key;
using namekey_map_t = std::map<red::ci_string_view, int>;

static red::serial_map<red::ci_string_view, int> kb_serialmap {
    {"[", KbKey::LBracket}, 
    {"]", KbKey::RBracket},
    {";", KbKey::Semicolon},
    {",", KbKey::Comma},
    {".", KbKey::Period},
    {"'", KbKey::Quote},
    {"/", KbKey::Slash},
    {"\\", KbKey::BackSlash},
    {"~", KbKey::Tilde},
    {"=", KbKey::Equal},
    {"-", KbKey::Hyphen},
    {"+", KbKey::Add},
    {"*", KbKey::Multiply},
    
    {"Escape", KbKey::Escape}, {"Esc", KbKey::Escape},
    {"LControl", KbKey::LControl},{"LCtrl", KbKey::LControl},
    {"LAlt", KbKey::LAlt},
    {"LShift", KbKey::LShift},
    {"RControl", KbKey::RControl}, {"RCtrl", KbKey::RControl},
    {"RAlt", KbKey::RAlt},
    {"RShift", KbKey::RShift},

    {"Menu", KbKey::Menu},
    {"Space", KbKey::Space},
    {"Enter", KbKey::Enter},
    {"Backspace", KbKey::Backspace},
    {"Tab", KbKey::Tab},

    {"PageUp", KbKey::PageUp},
    {"PageDown", KbKey::PageDown},
    {"End", KbKey::End},
    {"Home", KbKey::Home},
    {"Insert", KbKey::Insert},
    {"Delete", KbKey::Delete},

    {"Left", KbKey::Left},   {"LeftArrow", KbKey::Left},
    {"Right", KbKey::Right}, {"RightArrow", KbKey::Right},
    {"Up", KbKey::Up},       {"UpArrow", KbKey::Up},
    {"Down", KbKey::Down},   {"DownArrow", KbKey::Down},

    {"Pause", KbKey::Pause},

    // autogen
    // ----------------
    {"0", KbKey::Num0},
    {"Num0", KbKey::Num0},
    {"Numpad0", KbKey::Numpad0},
    {"1", KbKey::Num1},
    {"Num1", KbKey::Num1},
    {"Numpad1", KbKey::Numpad1},
    {"2", KbKey::Num2},
    {"Num2", KbKey::Num2},
    {"Numpad2", KbKey::Numpad2},
    {"3", KbKey::Num3},
    {"Num3", KbKey::Num3},
    {"Numpad3", KbKey::Numpad3},
    {"4", KbKey::Num4},
    {"Num4", KbKey::Num4},
    {"Numpad4", KbKey::Numpad4},
    {"5", KbKey::Num5},
    {"Num5", KbKey::Num5},
    {"Numpad5", KbKey::Numpad5},
    {"6", KbKey::Num6},
    {"Num6", KbKey::Num6},
    {"Numpad6", KbKey::Numpad6},
    {"7", KbKey::Num7},
    {"Num7", KbKey::Num7},
    {"Numpad7", KbKey::Numpad7},
    {"8", KbKey::Num8},
    {"Num8", KbKey::Num8},
    {"Numpad8", KbKey::Numpad8},
    {"9", KbKey::Num9},
    {"Num9", KbKey::Num9},
    {"Numpad9", KbKey::Numpad9},
    {"A", KbKey::A},
    {"B", KbKey::B},
    {"C", KbKey::C},
    {"D", KbKey::D},
    {"E", KbKey::E},
    {"F", KbKey::F},
    {"G", KbKey::G},
    {"H", KbKey::H},
    {"I", KbKey::I},
    {"J", KbKey::J},
    {"K", KbKey::K},
    {"L", KbKey::L},
    {"M", KbKey::M},
    {"N", KbKey::N},
    {"O", KbKey::O},
    {"P", KbKey::P},
    {"Q", KbKey::Q},
    {"R", KbKey::R},
    {"S", KbKey::S},
    {"T", KbKey::T},
    {"U", KbKey::U},
    {"V", KbKey::V},
    {"W", KbKey::W},
    {"X", KbKey::X},
    {"Y", KbKey::Y},
    {"Z", KbKey::Z}
};


auto red::parse_kb_key(red::ci_string_view sv) -> sf::Keyboard::Key {
    try
    {
        int code = kb_serialmap.at(sv);
        return (KbKey)code;
    }
    catch (const std::out_of_range&)
    {
        gamelog()->error("invalid key name '{}'", sv);
        return KbKey::Unknown;
    }
}

#include "game_config.h"
#include "boost/program_options.hpp"

red::pong::config_t red::pong::load_config() 
{
    using std::string;
    namespace po = boost::program_options;

    // temp pra parsear controles
    enum : short { player_1, player_2 };
    struct { string up, down, fast; } keys[2];

    config_t config;

    po::options_description cfg_desc;
    cfg_desc.add_options()
        ("controls.player1.up", po::value<string>(&keys[player_1].up))
        ("controls.player1.down", po::value<string>(&keys[player_1].down))
        ("controls.player1.fast", po::value<string>(&keys[player_1].fast))
        ("controls.player2.up", po::value<string>(&keys[player_2].up))
        ("controls.player2.down", po::value<string>(&keys[player_2].down))
        ("controls.player2.fast", po::value<string>(&keys[player_2].fast))

        ("game.paddle.base_speed", po::value<float>(&config.paddle.base_speed))
        ("game.paddle.accel", po::value<float>(&config.paddle.accel))

        ("game.ball.max_speed", po::value<float>(&config.ball.max_speed))
        ("game.ball.serve_speed", po::value<float>(&config.ball.base_speed))
        ("game.ball.accel", po::value<float>(&config.ball.accel))
        ("game.ball.radius", po::value<float>(&config.ball.radius))
    ;

    po::variables_map cfg_vm;
    auto parsed = po::parse_config_file("game.cfg", cfg_desc, true);
    po::store(parsed, cfg_vm);
    po::notify(cfg_vm);

    config.controls[player_1].up = red::parse_kb_key(keys[player_1].up);
    config.controls[player_1].down = red::parse_kb_key(keys[player_1].down);
    config.controls[player_1].fast = red::parse_kb_key(keys[player_1].fast);
    config.controls[player_2].up = red::parse_kb_key(keys[player_2].up);
    config.controls[player_2].down = red::parse_kb_key(keys[player_2].down);
    config.controls[player_2].fast = red::parse_kb_key(keys[player_2].fast);

    return config;
}