#include <string>
#include <string_view>
#include <sstream>
#include <fstream>

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/System/Vector2.hpp>
#include <fmt/ostream.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "common.h"
#include "ci_string.h"
#include "symbol_table.h"
#include "game_config.h"
#include "convert.h"


static void lippincott()
{
    using pong::gamelog;
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

// converters
using red::to_ci;
using enum_name_table = symbol_table<red::ci_string_view, int>;
static auto sf_enums_table()->enum_name_table const&;


std::ostream& operator<<(std::ostream& os, sf::Keyboard::Key key)
{
    auto const& table = sf_enums_table();
    try
    {
        auto name = table.get_name(key);
        return os << name;
    }
    catch (const std::out_of_range&)
    {
        return os << "unknown";
    }
}
std::ostream& operator<<(std::ostream& os, sf::Mouse::Button btn)
{
    auto const& table = sf_enums_table();
    try
    {
        auto name = table.get_name(btn);
        return os << name;
    }
    catch (const std::out_of_range&)
    {
        os << "unknown";
        return os;
    }
}

std::istream& operator>>(std::istream& is, sf::Keyboard::Key& key)
{
    auto const& table = sf_enums_table();
    std::string token; is >> token;
    try
    {
        auto val = static_cast<sf::Keyboard::Key>(table[to_ci(token)]);
        key = val;
    }
    catch (const std::out_of_range&)
    {
        is.setstate(std::ios::failbit);
        key = sf::Keyboard::Unknown;
    }

    return is;
}
std::istream& operator>>(std::istream& is, sf::Mouse::Button& btn)
{
    auto const& table = sf_enums_table();
    std::string token; is >> token;
    try
    {
        auto val = table[to_ci(token)];
        btn = static_cast<sf::Mouse::Button>(val);
    }
    catch (const std::out_of_range&)
    {
        is.setstate(std::ios::failbit);
    }

    return is;
}

struct keyboardkey_translator
{
    using internal_type = std::string;
    using external_type = sf::Keyboard::Key;

    auto get_value(std::string const& v) -> boost::optional<sf::Keyboard::Key>
    {
        auto const& table = sf_enums_table();
        try {
            auto i = table[to_ci(v)];
            return static_cast<sf::Keyboard::Key>(i);
        }
        catch (const std::out_of_range&) {
            return boost::none;
        }
    }

    auto put_value(sf::Keyboard::Key const& v) -> boost::optional<std::string>
    {
        auto const& table = sf_enums_table();
        try
        {
            auto sv = table[v];
            return std::string(sv.begin(), sv.end());
        }
        catch (const std::exception&)
        {
            return boost::none;
        }
    }
};

void pong::config_t::load(std::filesystem::path filepath)
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

    { // controls
        using Key = sf::Keyboard::Key;
        keyboardkey_translator tr;
        
        controls[0].up = tree.get<Key>(CFG_P1_UP, Key::W, tr);
        controls[0].down = tree.get<Key>(CFG_P1_DOWN, Key::S, tr);
        controls[0].fast = tree.get<Key>(CFG_P1_FAST, Key::LShift, tr);
        controls[1].up = tree.get<Key>(CFG_P2_UP, Key::Up, tr);
        controls[1].down = tree.get<Key>(CFG_P2_DOWN, Key::Down, tr);
        controls[1].fast = tree.get<Key>(CFG_P2_FAST, Key::RControl, tr);
    }

    // paddle
    paddle.accel = tree.get<float>(CFG_PADDLE_ACCEL, 0.1f);
    paddle.base_speed = tree.get<float>(CFG_PADDLE_SPEED, 10.f);
    paddle.size.x = tree.get(CFG_PADDLE_SIZE_X, 25.f);
    paddle.size.y = tree.get(CFG_PADDLE_SIZE_Y, 150.f);


    // ball
    ball.accel = tree.get<float>(CFG_BALL_ACCEL, 0.1f);
    ball.base_speed = tree.get<float>(CFG_BALL_SPEED, 5);
    ball.max_speed = tree.get<float>(CFG_BALL_MAXSPEED, 20);
    ball.radius = tree.get<float>(CFG_BALL_RADIUS, 10);

    framerate = tree.get<unsigned>(CFG_FRAMERATE, 60);
}

void pong::config_t::save(std::filesystem::path filepath)
{
    using namespace boost::property_tree;

    ptree tree;
    { // controls
        keyboardkey_translator tr;
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
    //tree.put(CFG_PADDLE_SIZE, paddle.size, sfVec_translator<float>{});
    tree.put(CFG_PADDLE_SIZE_X, paddle.size.x);
    tree.put(CFG_PADDLE_SIZE_Y, paddle.size.y);

    // ball
    tree.put(CFG_BALL_MAXSPEED, ball.max_speed);
    tree.put(CFG_BALL_SPEED, ball.base_speed);
    tree.put(CFG_BALL_ACCEL, ball.accel);
    tree.put(CFG_BALL_RADIUS, ball.radius);

    tree.put(CFG_FRAMERATE, framerate);

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

bool pong::config_t::operator==(const config_t& rhs) const noexcept
{
    return controls == rhs.controls &&
           paddle == rhs.paddle &&
           ball == rhs.ball &&
           framerate == rhs.framerate;
}



// ---
using KbKey = sf::Keyboard::Key;

auto sf_enums_table() ->symbol_table<red::ci_string_view, int> const&
{
    static symbol_table<red::ci_string_view, int> names{
        // keyboard
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

        {"Esc", KbKey::Escape}, {"Escape", KbKey::Escape},
        {"LCtrl", KbKey::LControl},{"LControl", KbKey::LControl},
        {"LAlt", KbKey::LAlt},
        {"LShift", KbKey::LShift},
        {"RCtrl", KbKey::RControl},{"RControl", KbKey::RControl},
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
        {"Z", KbKey::Z},
        // mouse
        {"MouseLeft", sf::Mouse::Left}, {"Mouse1", sf::Mouse::Left},
        {"MouseRight", sf::Mouse::Right}, {"Mouse2", sf::Mouse::Right},
        {"MouseMiddle", sf::Mouse::Middle}, {"Mouse3", sf::Mouse::Right},
        {"Mouse4", sf::Mouse::XButton1},
        {"Mouse5", sf::Mouse::XButton2},
        {"MouseWheel", sf::Mouse::VerticalWheel},
        {"MouseHWheel", sf::Mouse::HorizontalWheel}
    };

    return names;
}

