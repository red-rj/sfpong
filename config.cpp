#include <string>
#include <string_view>
#include <map>
#include <sstream>
#include <fstream>

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/System/Vector2.hpp>
#include <fmt/ostream.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "ci_string.h"
#include "common.h"
#include "serial_map.h"


using KbKey = sf::Keyboard::Key;
using serial_key_map = red::serial_map<red::ci_string_view, int>;

template<typename E>
using enum_name_table = red::serial_map<red::ci_string_view, E>;

static const serial_key_map kb_serialmap {
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

static const serial_key_map mouse_serialmap {
    {"MouseLeft", sf::Mouse::Left}, {"Mouse1", sf::Mouse::Left},
    {"MouseRight", sf::Mouse::Right}, {"Mouse2", sf::Mouse::Right},
    {"MouseMiddle", sf::Mouse::Middle}, {"Mouse3", sf::Mouse::Right},
    {"Mouse4", sf::Mouse::XButton1},
    {"Mouse5", sf::Mouse::XButton2},
    {"MouseWheel", sf::Mouse::VerticalWheel},
    {"MouseHWheel", sf::Mouse::HorizontalWheel}
};

#include "game_config.h"

// config keys
constexpr auto
    CFG_P1_UP = "controls.player1.up",
    CFG_P1_DOWN = "controls.player1.down",
    CFG_P1_FAST = "controls.player1.fast",
    CFG_P2_UP = "controls.player2.up",
    CFG_P2_DOWN = "controls.player2.down",
    CFG_P2_FAST = "controls.player2.fast",
    CFG_PADDLE_SPEED = "game.paddle.base_speed",
    CFG_PADDLE_ACCEL = "game.paddle.accel",
    CFG_PADDLE_SIZE = "game.paddle.size",
    CFG_BALL_SPEED = "game.ball.base_speed",
    CFG_BALL_MAXSPEED = "game.ball.max_speed",
    CFG_BALL_ACCEL = "game.ball.accel",
    CFG_BALL_RADIUS = "game.ball.radius",
    CFG_FRAMERATE = "game.framerate"
;

static void lippincott()
{
    using red::gamelog;
    using boost::property_tree::ini_parser_error;

    try { throw; }
    catch (const ini_parser_error& e)
    {
        gamelog()->error("ini_parser_error {}:{} - '{}'", e.filename(), e.line(), e.what());
    }
    catch (const std::exception& e)
    {
        gamelog()->error("{}", e.what());
    }
}

struct enum_translator
{
    auto get_value(std::string const& v) -> boost::optional<sf::Keyboard::Key>
    {
        try
        {
            auto vv = red::ci_string_view(v.data(), v.size());
            auto value = (sf::Keyboard::Key) kb_serialmap[vv];
            return value;
        }
        catch (const std::exception&)
        {
            return boost::none;
        }
    }

    auto put_value(sf::Keyboard::Key const& v) -> boost::optional<std::string>
    {
        try
        {
            auto value = kb_serialmap[v];
            return std::string{ value.data(), value.size() };
        }
        catch (const std::exception&)
        {
            return boost::none;
        }
    }

};

struct sfVec_translator
{
    template<typename T>
    auto get_value(std::string const& v)->boost::optional<sf::Vector2<T>>
    {
        sf::Vector2<T> value;

        try
        {
            std::istringstream ss{ v };
            ss.imbue(std::locale::classic());

            ss >> value.x >> value.y;
            return value;
        }
        catch (const std::exception&)
        {
            return boost::none;
        }
    }

    template<typename T>
    auto put_value(sf::Vector2<T> const& v)->boost::optional<std::string>
    {
        std::ostringstream ss;
        ss.imbue(std::locale::classic());

        ss << v.x << ", " << v.y;
        return ss.str();
    }
};



void red::pong::config_t::load(std::filesystem::path filepath)
{
    using namespace boost::property_tree;

    std::ifstream file{ filepath };
    ptree tree;
    try
    {
        read_ini(file, tree);
    }
    catch (...)
    {
        lippincott();
        return;
    }

    ;
}

void red::pong::config_t::save(std::filesystem::path filepath)
{
    using namespace boost::property_tree;

    ptree tree;
    { // controls
        enum_translator tr;
        tree.put(CFG_P1_UP, controls[0].up, tr);
        tree.put(CFG_P1_DOWN, controls[0].down, tr);
        tree.put(CFG_P1_FAST, controls[0].fast, tr);
        tree.put(CFG_P2_UP, controls[1].up, tr);
        tree.put(CFG_P2_DOWN, controls[1].down, tr);
        tree.put(CFG_P2_FAST, controls[1].fast, tr);
    }
    
    // paddle
    tree.put(CFG_PADDLE_SPEED, paddle.base_speed);
    tree.put(CFG_PADDLE_ACCEL, paddle.accel);
    tree.put(CFG_PADDLE_SIZE, paddle.size, sfVec_translator{});

    // ball
    tree.put(CFG_BALL_MAXSPEED, ball.max_speed);
    tree.put(CFG_BALL_SPEED, ball.base_speed);
    tree.put(CFG_BALL_ACCEL, ball.accel);
    tree.put(CFG_BALL_RADIUS, ball.radius);

    auto file = std::ofstream(filepath, std::ios::trunc);
    file << "# sfPong configuration\n\n";

    try
    {
        write_ini(file, tree);
    }
    catch (...)
    {
        lippincott();
    }

}
