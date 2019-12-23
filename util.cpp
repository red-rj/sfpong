#include <string>
#include <map>
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Joystick.hpp"
#include "SFML/Window/Mouse.hpp"

#include "util.h"
#include "common.h"
#include "serial_map.h"


using KbKey = sf::Keyboard::Key;
using serial_key_map = red::serial_map<red::ci_string_view, int>;
namespace po = boost::program_options;

static const red::serial_map<red::ci_string_view, KbKey> kb_serialmap {
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

namespace {
    template<class I> I parse(red::ci_string_view) = delete;
    template<class I> I parse(std::string_view view) {
        return parse<I>(red::ci_string_view{ view.data(), view.size() });
    }

    template<class I> red::ci_string_view serialize(I) = delete;

    template<>
    sf::Keyboard::Key parse(red::ci_string_view sv) {
        try
        {
            int code = kb_serialmap[sv];
            return (KbKey)code;
        }
        catch (const std::out_of_range&)
        {
            red::gamelog()->error("invalid key name '{}'", sv);
            return KbKey::Unknown;
        }
    }

    template<>
    red::ci_string_view serialize(sf::Keyboard::Key key) {
        auto str = kb_serialmap[key];
        return str;
    }
}


#include "game_config.h"
#include "boost/program_options.hpp"

// config keys
constexpr auto 
    CFG_P1_UP          = "controls.player1.up", 
    CFG_P1_DOWN        = "controls.player1.down",
    CFG_P1_FAST        = "controls.player2.fast",
    CFG_P2_UP          = "controls.player2.up", 
    CFG_P2_DOWN        = "controls.player2.down",
    CFG_P2_FAST        = "controls.player2.fast",
    CFG_PADDLE_SPEED   = "game.paddle.base_speed",
    CFG_PADDLE_ACCEL   = "game.paddle.accel",
    CFG_BALL_SPEED     = "game.ball.base_speed",
    CFG_BALL_MAXSPEED  = "game.ball.max_speed",
    CFG_BALL_ACCEL     = "game.ball.accel",
    CFG_BALL_RADIUS    = "game.ball.radius",
    CFG_FRAMERATE      = "game.framerate"
;

po::variables_map red::pong::load_config_variables(std::string_view file)
{
    using std::string;

    po::options_description cfg_desc;
    cfg_desc.add_options()
        (CFG_P1_UP, po::value<string>()->default_value("w"))
        (CFG_P1_DOWN, po::value<string>()->default_value("s"))
        (CFG_P1_FAST, po::value<string>()->default_value("Lshift"))
        (CFG_P2_UP, po::value<string>()->default_value("upArrow"))
        (CFG_P2_DOWN, po::value<string>()->default_value("downArrow"))
        (CFG_P2_FAST, po::value<string>()->default_value("Rctrl"))

        (CFG_PADDLE_SPEED, po::value<float>())
        (CFG_PADDLE_ACCEL, po::value<float>())

        (CFG_BALL_MAXSPEED, po::value<float>())
        (CFG_BALL_SPEED, po::value<float>())
        (CFG_BALL_ACCEL, po::value<float>())
        (CFG_BALL_RADIUS, po::value<float>())   
        (CFG_FRAMERATE, po::value<unsigned>()->default_value(60))
    ;

    po::variables_map cfg_vm;
    auto parsed = po::parse_config_file(file.data(), cfg_desc, true);
    po::store(parsed, cfg_vm); po::notify(cfg_vm);

    return cfg_vm;
}

red::pong::config_t red::pong::load_config()
{
    using std::string;
    using namespace red::pong::player_id;

    config_t config;

    auto vmap = load_config_variables("game.cfg");

    config.controls[player_1].up = parse<KbKey>(vmap[CFG_P1_UP].as<string>());
    config.controls[player_1].down = kb_serialmap[vmap[CFG_P1_DOWN].as<string>()];
    config.controls[player_1].fast = kb_serialmap[vmap[CFG_P1_FAST].as<string>()];
    config.controls[player_2].up = kb_serialmap[vmap[CFG_P2_UP].as<string>()];
    config.controls[player_2].down = kb_serialmap[vmap[CFG_P2_DOWN].as<string>()];
    config.controls[player_2].fast = kb_serialmap[vmap[CFG_P2_FAST].as<string>()];

    config.paddle.base_speed = vmap[CFG_PADDLE_SPEED].as<float>();
    config.paddle.accel = vmap[CFG_PADDLE_ACCEL].as<float>();
    config.ball.base_speed = vmap[CFG_BALL_SPEED].as<float>();
    config.ball.accel = vmap[CFG_BALL_ACCEL].as<float>();
    config.ball.max_speed = vmap[CFG_BALL_MAXSPEED].as<float>();
    config.ball.radius = vmap[CFG_BALL_RADIUS].as<float>();
    config.framerate = vmap[CFG_FRAMERATE].as<unsigned>();

    return config;
}