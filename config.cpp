#include <string>
#include <string_view>
#include <sstream>
#include <fstream>

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Joystick.hpp>
#include <SFML/System/Vector2.hpp>
#include <fmt/ostream.h>
#include <boost/property_tree/ptree.hpp>

#include "common.h"
#include "ci_string.h"
#include "symbol_table.h"
#include "game_config.h"
#include "convert.h"


using sf::Keyboard;
using sf::Joystick;
using sf::Mouse;
using namespace std::literals;

namespace // user config vars
{
    int player_joystick[2] = { -1, -1 };
    pong::keyboard_ctrls player_keyboard_controls[2];
    float player_deadzone[2];

    pong::size_2d<unsigned> game_resolution = { 1024, 768 };
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

/*
* enum tables
*/
auto sf_keyboard_table()->enum_name_table const&;
auto sf_mouse_table()->enum_name_table const&;

using iof = std::ios_base;


// convert

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

template<class E, class Traits = std::char_traits<char>>
using iostream_translator = boost::property_tree::stream_translator<typename Traits::char_type, Traits, std::allocator<char>, E>;

template<>
struct boost::property_tree::customize_stream<char, std::char_traits<char>, Keyboard::Key>
{
    static void insert(std::ostream& os, Keyboard::Key key)
    {
        auto const& table = sf_keyboard_table();
        os << table[key];
    }

    static void extract(std::istream& is, Keyboard::Key& key)
    {
        auto const& table = sf_keyboard_table();
        std::string token; is >> token;
        key = Keyboard::Key(table[token]);
    }
};

class joyid_translator : iostream_translator<int>
{
    using base = iostream_translator<int>;
public:
    using base::get_value;

    auto put_value(int id)
    {
        return id < 0 ? ""s : base::put_value(id);
    }
};


void pong::set_user_config(const cfgtree& tree)
{
    using namespace ckey;

    player_input_cfg P1, P2;
    joyid_translator joyid_tr;

    // player one
    P1.keyboard_controls.up = tree.get(P1_UP, Keyboard::W);
    P1.keyboard_controls.down = tree.get(P1_DOWN, Keyboard::S);
    P1.keyboard_controls.fast = tree.get(P1_FAST, Keyboard::LShift);
    P1.joystickId = tree.get(P1_JOYSTICK, -1, joyid_tr);
    P1.joystick_deadzone = tree.get(P1_JSDEADZONE, 10.f);
    set_input_cfg(P1, playerid::one);
    
    // player two
    P2.keyboard_controls.up = tree.get(P2_UP, Keyboard::Up);
    P2.keyboard_controls.down = tree.get(P2_DOWN, Keyboard::Down);
    P2.keyboard_controls.fast = tree.get(P2_FAST, Keyboard::RControl);
    P2.joystickId = tree.get(P2_JOYSTICK, -1, joyid_tr);
    P2.joystick_deadzone = tree.get(P2_JSDEADZONE, 10.f);
    set_input_cfg(P2, playerid::two);
}

pong::cfgtree pong::get_user_config()
{
    using namespace ckey;
    auto tree = cfgtree();

    player_input_cfg P1 = get_input_cfg(playerid::one), P2 = get_input_cfg(playerid::two);
    joyid_translator joyid_tr;

    tree.put(P1_UP, P1.keyboard_controls.up);
    tree.put(P1_DOWN, P1.keyboard_controls.down);
    tree.put(P1_FAST, P1.keyboard_controls.fast);
    tree.put(P1_JOYSTICK, P1.joystickId, joyid_tr);
    tree.put(P1_JSDEADZONE, P1.joystick_deadzone);
    
    tree.put(P2_UP, P2.keyboard_controls.up);
    tree.put(P2_DOWN, P2.keyboard_controls.down);
    tree.put(P2_FAST, P2.keyboard_controls.fast);
    tree.put(P2_JOYSTICK, P2.joystickId, joyid_tr);
    tree.put(P2_JSDEADZONE, P2.joystick_deadzone);

    return tree;
}


pong::keyboard_ctrls pong::get_keyboard_controls(playerid pl) noexcept
{
    return player_keyboard_controls[int(pl)];
}

void pong::set_keyboard_controls(playerid pl, keyboard_ctrls ctrls) noexcept
{
    player_keyboard_controls[int(pl)] = ctrls;
}

int pong::get_joystick(playerid pl) noexcept
{
    return player_joystick[int(pl)];
}

void pong::set_joystick(playerid pl, int joyid) noexcept
{
    player_joystick[int(pl)] = joyid;
}

auto pong::get_input_cfg(playerid player) noexcept -> player_input_cfg
{
    const auto p = int(player);
    player_input_cfg cfg;
    cfg.joystickId = player_joystick[p];
    cfg.joystick_deadzone = player_deadzone[p];
    cfg.keyboard_controls = player_keyboard_controls[p];
    return cfg;
}

void pong::set_input_cfg(player_input_cfg input, playerid player) noexcept
{
    const auto p = int(player);
    player_keyboard_controls[p] = input.keyboard_controls;
    player_joystick[p] = input.joystickId;
    player_deadzone[p] = input.joystick_deadzone;
}


bool pong::keyboard_ctrls::operator==(const keyboard_ctrls& rhs) const noexcept
{
    using std::tie;
    return tie(up, down, fast) == tie(rhs.up, rhs.down, rhs.fast);
}

bool pong::player_input_cfg::operator==(const player_input_cfg& rhs) const noexcept
{
    using std::tie;
    return tie(keyboard_controls, joystickId, joystick_deadzone) ==
        tie(rhs.keyboard_controls, rhs.joystickId, rhs.joystick_deadzone);
}


// enum name tables

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
