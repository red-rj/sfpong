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
#include <boost/operators.hpp>

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
        gamelog()->error("{}", e.what());
    }
    catch (const std::exception& e)
    {
        gamelog()->error("{}", e.what());
    }
}


struct ci_compare
{
    constexpr bool operator()(std::string_view lhs, std::string_view rhs) const
    {
        using red::to_ci;
        return to_ci(lhs) < to_ci(rhs);
    }
    constexpr bool operator()(int lhs, int rhs) const
    {
        return lhs < rhs;
    }
};

using enum_name_table = symbol_table<std::string_view, int, ci_compare>;

using iof = std::ios_base;

/*
* enum tables
*/
auto sf_keyboard_table()->enum_name_table const&;
auto sf_mouse_table()->enum_name_table const&;


std::ostream& operator<<(std::ostream& os, sf::Keyboard::Key key)
{
    auto const& table = sf_keyboard_table();
    try
    {
        auto name = table[key];
        return os << name;
    }
    catch (const std::out_of_range&)
    {
        os << "unknown";
        os.setstate(iof::failbit);
        return os;
    }
}
std::ostream& operator<<(std::ostream& os, sf::Mouse::Button btn)
{
    auto const& table = sf_mouse_table();
    try
    {
        auto name = table[btn];
        return os << name;
    }
    catch (const std::out_of_range&)
    {
        os << "unknown";
        os.setstate(iof::failbit);
        return os;
    }
}

std::istream& operator>>(std::istream& is, sf::Keyboard::Key& key)
{
    auto const& table = sf_keyboard_table();
    std::string token; is >> token;
    try
    {
        auto val = sf::Keyboard::Key(table[token]);
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
    auto const& table = sf_mouse_table();
    std::string token; is >> token;
    try
    {
        auto val = sf::Mouse::Button(table[token]);
        btn = val;
    }
    catch (const std::out_of_range&)
    {
        is.setstate(iof::failbit);
    }

    return is;
}


template<class Enum>
struct sfenum_translator
{
    using internal_type = std::string;
    using external_type = Enum;

    auto get_value(std::string const& v)->boost::optional<external_type>
    {
        try
        {
            auto ss = std::stringstream(v, iof::in);
            ss.exceptions(iof::failbit);
            external_type i;
            ss >> i;
            return i;
        }
        catch (const std::exception&)
        {
            return boost::none;
        }
    }

    auto put_value(external_type const& v) -> boost::optional<std::string>
    {
        try
        {
            auto ss = std::stringstream(iof::out);
            ss.exceptions(iof::failbit);
            ss << v;
            return ss.str();
        }
        catch (const std::exception&)
        {
            return boost::none;
        }
    }
};
using keyboardkey_translator = sfenum_translator<sf::Keyboard::Key>;


pong::config_t pong::load_config(std::filesystem::path cfgfile)
{
    using namespace boost::property_tree;

    std::ifstream file{ cfgfile };
    ptree tree;
    config_t cfg; // tem valores padrão

    try
    {
        read_ini(file, tree);
    }
    catch (...)
    {
        lippincott();
        return cfg;
    }


    { // player settings
        using Key = sf::Keyboard::Key;
        keyboardkey_translator tr;
        enum { P1, P2 };
        auto dft = cfg.controls;

        cfg.controls[P1].up = tree.get<Key>(CFG_P1_UP, dft[P1].up, tr);
        cfg.controls[P1].down = tree.get<Key>(CFG_P1_DOWN, dft[P1].down, tr);
        cfg.controls[P1].fast = tree.get<Key>(CFG_P1_FAST, dft[P1].fast, tr);
        cfg.controls[P2].up = tree.get<Key>(CFG_P2_UP, dft[P2].up, tr);
        cfg.controls[P2].down = tree.get<Key>(CFG_P2_DOWN, dft[P2].down, tr);
        cfg.controls[P2].fast = tree.get<Key>(CFG_P2_FAST, dft[P2].fast, tr);
    }

    // paddle
    auto def = cfg.paddle;
    cfg.paddle.accel = tree.get(CFG_PADDLE_ACCEL, def.accel);
    cfg.paddle.base_speed = tree.get(CFG_PADDLE_SPEED, def.base_speed);
    cfg.paddle.size.x = tree.get(CFG_PADDLE_SIZE_X, def.size.x);
    cfg.paddle.size.y = tree.get(CFG_PADDLE_SIZE_Y, def.size.y);

    // ball
    auto defb = cfg.ball;
    cfg.ball.accel = tree.get(CFG_BALL_ACCEL, defb.accel);
    cfg.ball.base_speed = tree.get(CFG_BALL_SPEED, defb.base_speed);
    cfg.ball.max_speed = tree.get(CFG_BALL_MAXSPEED, defb.max_speed);
    cfg.ball.radius = tree.get(CFG_BALL_RADIUS, defb.radius);

    cfg.framerate = tree.get(CFG_FRAMERATE, cfg.framerate);

    return cfg;
}

bool pong::save_config(config_t const& cfg, std::filesystem::path cfgfile)
{
    using namespace boost::property_tree;

    ptree tree;
    { // controls
        keyboardkey_translator tr;
        enum { P1, P2 };

        tree.put(CFG_P1_UP, cfg.controls[P1].up, tr);
        tree.put(CFG_P1_DOWN, cfg.controls[P1].down, tr);
        tree.put(CFG_P1_FAST, cfg.controls[P1].fast, tr);
        tree.put(CFG_P2_UP, cfg.controls[P2].up, tr);
        tree.put(CFG_P2_DOWN, cfg.controls[P2].down, tr);
        tree.put(CFG_P2_FAST, cfg.controls[P2].fast, tr);
    }
    
    // paddle
    tree.put(CFG_PADDLE_SPEED, cfg.paddle.base_speed);
    tree.put(CFG_PADDLE_ACCEL, cfg.paddle.accel);
    tree.put(CFG_PADDLE_SIZE_X, cfg.paddle.size.x);
    tree.put(CFG_PADDLE_SIZE_Y, cfg.paddle.size.y);

    // ball
    tree.put(CFG_BALL_MAXSPEED, cfg.ball.max_speed);
    tree.put(CFG_BALL_SPEED, cfg.ball.base_speed);
    tree.put(CFG_BALL_ACCEL, cfg.ball.accel);
    tree.put(CFG_BALL_RADIUS, cfg.ball.radius);

    tree.put(CFG_FRAMERATE, cfg.framerate);

    auto file = std::ofstream(cfgfile, iof::trunc);
    file << "# sfPong configuration\n\n";

    try
    {
        write_ini(file, tree);
        return true;
    }
    catch (...)
    {
        lippincott();
        return false;
    }

}

bool pong::config_t::operator==(const config_t& rhs) const noexcept
{
    return controls == rhs.controls &&
           paddle == rhs.paddle &&
           ball == rhs.ball &&
           framerate == rhs.framerate;
}


auto sf_keyboard_table() ->enum_name_table const&
{
    static enum_name_table names
    {
        { "Escape", sf::Keyboard::Escape },
        { "Esc", sf::Keyboard::Escape },
        { "LControl", sf::Keyboard::LControl },
        { "LCtrl", sf::Keyboard::LControl },
        { "LShift", sf::Keyboard::LShift },
        { "LAlt", sf::Keyboard::LAlt },
        { "LSystem", sf::Keyboard::LSystem },
        { "RControl", sf::Keyboard::RControl },
        { "RCtrl", sf::Keyboard::RControl },
        { "RShift", sf::Keyboard::RShift },
        { "RAlt", sf::Keyboard::RAlt },
        { "RSystem", sf::Keyboard::RSystem },
        { "Menu", sf::Keyboard::Menu },
        { "[", sf::Keyboard::LBracket },
        { "]", sf::Keyboard::RBracket },
        { ";", sf::Keyboard::Semicolon },
        { ",", sf::Keyboard::Comma },
        { ".", sf::Keyboard::Period },
        { "'", sf::Keyboard::Quote },
        { "/", sf::Keyboard::Slash },
        { "\\", sf::Keyboard::Backslash },
        { "~", sf::Keyboard::Tilde },
        { "=", sf::Keyboard::Equal },
        { "-", sf::Keyboard::Hyphen },
        { "Space", sf::Keyboard::Space },
        { "Enter", sf::Keyboard::Enter },
        { "Backspace", sf::Keyboard::Backspace },
        { "Tab", sf::Keyboard::Tab },
        { "PageUp", sf::Keyboard::PageUp },
        { "PageDown", sf::Keyboard::PageDown },
        { "End", sf::Keyboard::End },
        { "Home", sf::Keyboard::Home },
        { "Insert", sf::Keyboard::Insert },
        { "Delete", sf::Keyboard::Delete },
        { "+", sf::Keyboard::Add },
        { "Numpad-", sf::Keyboard::Subtract },
        { "*", sf::Keyboard::Multiply },
        { "Numpad/", sf::Keyboard::Divide },
        { "Left", sf::Keyboard::Left },
        { "LeftArrow", sf::Keyboard::Left },
        { "Right", sf::Keyboard::Right },
        { "RightArrow", sf::Keyboard::Right },
        { "Up", sf::Keyboard::Up },
        { "UpArrow", sf::Keyboard::Up },
        { "Down", sf::Keyboard::Down },
        { "DownArrow", sf::Keyboard::Down },
        { "Pause", sf::Keyboard::Pause },
        { "0", sf::Keyboard::Num0 },
        { "Numpad0", sf::Keyboard::Numpad0 },
        { "1", sf::Keyboard::Num1 },
        { "Numpad1", sf::Keyboard::Numpad1 },
        { "2", sf::Keyboard::Num2 },
        { "Numpad2", sf::Keyboard::Numpad2 },
        { "3", sf::Keyboard::Num3 },
        { "Numpad3", sf::Keyboard::Numpad3 },
        { "4", sf::Keyboard::Num4 },
        { "Numpad4", sf::Keyboard::Numpad4 },
        { "5", sf::Keyboard::Num5 },
        { "Numpad5", sf::Keyboard::Numpad5 },
        { "6", sf::Keyboard::Num6 },
        { "Numpad6", sf::Keyboard::Numpad6 },
        { "7", sf::Keyboard::Num7 },
        { "Numpad7", sf::Keyboard::Numpad7 },
        { "8", sf::Keyboard::Num8 },
        { "Numpad8", sf::Keyboard::Numpad8 },
        { "9", sf::Keyboard::Num9 },
        { "Numpad9", sf::Keyboard::Numpad9 },
        { "A", sf::Keyboard::A },
        { "B", sf::Keyboard::B },
        { "C", sf::Keyboard::C },
        { "D", sf::Keyboard::D },
        { "E", sf::Keyboard::E },
        { "F", sf::Keyboard::F },
        { "G", sf::Keyboard::G },
        { "H", sf::Keyboard::H },
        { "I", sf::Keyboard::I },
        { "J", sf::Keyboard::J },
        { "K", sf::Keyboard::K },
        { "L", sf::Keyboard::L },
        { "M", sf::Keyboard::M },
        { "N", sf::Keyboard::N },
        { "O", sf::Keyboard::O },
        { "P", sf::Keyboard::P },
        { "Q", sf::Keyboard::Q },
        { "R", sf::Keyboard::R },
        { "S", sf::Keyboard::S },
        { "T", sf::Keyboard::T },
        { "U", sf::Keyboard::U },
        { "V", sf::Keyboard::V },
        { "W", sf::Keyboard::W },
        { "X", sf::Keyboard::X },
        { "Y", sf::Keyboard::Y },
        { "Z", sf::Keyboard::Z },
        { "F1", sf::Keyboard::F1 },
        { "F2", sf::Keyboard::F2 },
        { "F3", sf::Keyboard::F3 },
        { "F4", sf::Keyboard::F4 },
        { "F5", sf::Keyboard::F5 },
        { "F6", sf::Keyboard::F6 },
        { "F7", sf::Keyboard::F7 },
        { "F8", sf::Keyboard::F8 },
        { "F9", sf::Keyboard::F9 },
        { "F10", sf::Keyboard::F10 },
        { "F11", sf::Keyboard::F11 },
        { "F12", sf::Keyboard::F12 },
        { "F13", sf::Keyboard::F13 },
        { "F14", sf::Keyboard::F14 },
        { "F15", sf::Keyboard::F15 }
    };

    return names;
}

auto sf_mouse_table()->enum_name_table const&
{
    static enum_name_table table
    {
        {"MouseLeft", sf::Mouse::Left}, {"Mouse1", sf::Mouse::Left},
        {"MouseRight", sf::Mouse::Right}, {"Mouse2", sf::Mouse::Right},
        {"MouseMiddle", sf::Mouse::Middle}, {"Mouse3", sf::Mouse::Middle},
        {"Mouse4", sf::Mouse::XButton1},
        {"Mouse5", sf::Mouse::XButton2},
        {"MouseWheel", sf::Mouse::VerticalWheel},
        {"MouseHWheel", sf::Mouse::HorizontalWheel}
    };

    return table;
}
