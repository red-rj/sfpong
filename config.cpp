#include <string>
#include <string_view>
#include <sstream>
#include <fstream>
#include <filesystem>

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Joystick.hpp>
#include <SFML/System/Vector2.hpp>
#include <fmt/ostream.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "common.h"
#include "ci_string.h"
#include "game_config.h"
#include "convert.h"

using sf::Keyboard;
using sf::Joystick;
using sf::Mouse;
using namespace std::literals;
using iof = std::ios_base;


namespace
{
    std::string_view serialize(sf::Keyboard::Key key) noexcept;
    std::string_view serialize(sf::Mouse::Button btn) noexcept;

    Keyboard::Key parse_kbkey(std::string_view text) noexcept;
    Mouse::Button parse_mousebtn(std::string_view text) noexcept;

}

template<class E, class Traits = std::char_traits<char>, class Alloc = std::allocator<typename Traits::char_type>>
using iostream_translator = boost::property_tree::stream_translator<typename Traits::char_type, Traits, Alloc, E>;

namespace boost::property_tree
{
#define TRAITS(traits) traits ## ::char_type, traits
#define STDTRAITS(c) TRAITS(std::char_traits<c>)

    template<>
    struct customize_stream<STDTRAITS(char), Keyboard::Key>
    {
        static void insert(std::ostream& os, Keyboard::Key key)
        {
            os << serialize(key);
        }

        static void extract(std::istream& is, Keyboard::Key& key)
        {
            std::string token;
            is >> token;
            key = parse_kbkey(token);
        }
    };

    template<>
    struct customize_stream<STDTRAITS(char), Mouse::Button>
    {
        static auto insert(std::ostream& os, Mouse::Button btn)
        {
            os << serialize(btn);
        }
        static void extract(std::istream& is, Mouse::Button& btn)
        {
            std::string token;
            is >> token;
            btn = parse_mousebtn(token);
        }

    };

#undef TRAITS
#undef STDTRAITS
}

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

// convert
std::ostream& operator<<(std::ostream& os, sf::Keyboard::Key key)
{
    return os << serialize(key);
}
std::ostream& operator<<(std::ostream& os, sf::Mouse::Button btn)
{
    return os << serialize(btn);
}

std::istream& operator>>(std::istream& is, sf::Keyboard::Key& key)
{
    std::string token;
    is >> token;
    key = parse_kbkey(token);
    return is;
}
std::istream& operator>>(std::istream& is, sf::Mouse::Button& btn)
{
    std::string token;
    is >> token;
    btn = parse_mousebtn(token);
    return is;
}


void pong::game_settings::set_joystick(playerid pid, int joyid) noexcept
{
    if (joyid != njoystick) {
        // nao deixar o mesmo joystick para os 2 jogadores
        auto& otherJs = pid == playerid::one ? player_joystick[1] : player_joystick[0];
        if (otherJs == joyid) {
            otherJs = njoystick;
        }
    }

    player_joystick[int(pid)] = joyid;
}

void pong::game_settings::load_tree(const cfgtree& tree)
{
    using namespace ckey;

    // player one
    player_keys[0] = {
        tree.get(P1_UP, Keyboard::W),
        tree.get(P1_DOWN, Keyboard::S),
        tree.get(P1_FAST, Keyboard::LShift)
    };
    player_joystick[0] = tree.get(P1_JOYSTICK, njoystick, joyid_translator());
    player_deadzone[0] = tree.get(P1_JSDEADZONE, 10.f);

    // player two
    player_keys[1] = {
        tree.get(P2_UP, Keyboard::Up),
        tree.get(P2_DOWN, Keyboard::Down),
        tree.get(P2_FAST, Keyboard::RControl)
    };
    player_joystick[1] = tree.get(P2_JOYSTICK, njoystick, joyid_translator());
    player_deadzone[1] = tree.get(P2_JSDEADZONE, 10.f);

    // game
    resolution.x = tree.get(RESOLUTION_X, 1280u);
    resolution.y = tree.get(RESOLUTION_Y, 1024u);
    fullscreen = tree.get(FULLSCREEN, false);
}

void pong::game_settings::load_file(std::filesystem::path const& iniPath)
{
    const auto ini = iniPath.string();
    cfgtree cfg;
    spdlog::info("loading config file: {}", ini);
    read_ini(ini, cfg);
    load_tree(cfg);
}

void pong::game_settings::save_tree(cfgtree& tree) const
{
    using namespace ckey;
    
    // player one
    tree.put(P1_UP, player_keys[0].up);
    tree.put(P1_DOWN, player_keys[0].down);
    tree.put(P1_FAST, player_keys[0].fast);
    tree.put(P1_JOYSTICK, player_joystick[0], joyid_translator());
    tree.put(P1_JSDEADZONE, player_deadzone[0]);

    // player two
    tree.put(P2_UP, player_keys[1].up);
    tree.put(P2_DOWN, player_keys[1].down);
    tree.put(P2_FAST, player_keys[1].fast);
    tree.put(P2_JOYSTICK, player_joystick[1], joyid_translator());
    tree.put(P2_JSDEADZONE, player_deadzone[1]);

    // game
    tree.put(RESOLUTION_X, resolution.x);
    tree.put(RESOLUTION_Y, resolution.y);
    tree.put(FULLSCREEN, fullscreen);
}

void pong::game_settings::save_file(std::filesystem::path const& iniPath) const
{
    const auto ini = iniPath.string();
    cfgtree cfg;
    spdlog::info("saving config file: {}", ini);
    save_tree(cfg);
    write_ini(ini, cfg);
}

bool pong::game_settings::operator==(const game_settings& rhs) const noexcept
{
    using std::tie;
    return
        tie(fullscreen, resolution, player_keys, player_joystick, player_deadzone)
        ==
        tie(rhs.fullscreen, rhs.resolution, rhs.player_keys, rhs.player_joystick, rhs.player_deadzone)
    ;
}

bool pong::keyboard_ctrls::operator==(const keyboard_ctrls& rhs) const noexcept
{
    using std::tie;
    return tie(up, down, fast) == tie(rhs.up, rhs.down, rhs.fast);
}


namespace // name-value tables
{

struct enumname
{
    int value;
    std::string_view name;

    constexpr auto ci_name() const {
        return util::ci_string_view(name.data(), name.size());
    }

    constexpr bool operator== (int v) const { return v == value; }
    constexpr bool operator== (std::string_view n) const { return n == name; }
    constexpr bool operator== (util::ci_string_view n) const { return n == ci_name(); }
};

// gerado por codegen-v2.py
const enumname
sfkeyboard_table[] = {
    // enum func: sfkeyboard_enum
    // format func: pr_multimap
    { sf::Keyboard::A, "A" },
    { sf::Keyboard::Add, "+" },
    { sf::Keyboard::Add, "Add" },
    { sf::Keyboard::B, "B" },
    { sf::Keyboard::Backslash, "\\" },
    { sf::Keyboard::Backslash, "Backslash" },
    { sf::Keyboard::Backspace, "Backspace" },
    { sf::Keyboard::C, "C" },
    { sf::Keyboard::Comma, "," },
    { sf::Keyboard::Comma, "Comma" },
    { sf::Keyboard::D, "D" },
    { sf::Keyboard::Delete, "Delete" },
    { sf::Keyboard::Divide, "Divide" },
    { sf::Keyboard::Divide, "Numpad/" },
    { sf::Keyboard::Down, "Down" },
    { sf::Keyboard::Down, "DownArrow" },
    { sf::Keyboard::E, "E" },
    { sf::Keyboard::End, "End" },
    { sf::Keyboard::Enter, "Enter" },
    { sf::Keyboard::Equal, "=" },
    { sf::Keyboard::Equal, "Equal" },
    { sf::Keyboard::Escape, "Esc" },
    { sf::Keyboard::Escape, "Escape" },
    { sf::Keyboard::F, "F" },
    { sf::Keyboard::F1, "F1" },
    { sf::Keyboard::F10, "F10" },
    { sf::Keyboard::F11, "F11" },
    { sf::Keyboard::F12, "F12" },
    { sf::Keyboard::F13, "F13" },
    { sf::Keyboard::F14, "F14" },
    { sf::Keyboard::F15, "F15" },
    { sf::Keyboard::F2, "F2" },
    { sf::Keyboard::F3, "F3" },
    { sf::Keyboard::F4, "F4" },
    { sf::Keyboard::F5, "F5" },
    { sf::Keyboard::F6, "F6" },
    { sf::Keyboard::F7, "F7" },
    { sf::Keyboard::F8, "F8" },
    { sf::Keyboard::F9, "F9" },
    { sf::Keyboard::G, "G" },
    { sf::Keyboard::H, "H" },
    { sf::Keyboard::Home, "Home" },
    { sf::Keyboard::Hyphen, "-" },
    { sf::Keyboard::Hyphen, "Hyphen" },
    { sf::Keyboard::I, "I" },
    { sf::Keyboard::Insert, "Insert" },
    { sf::Keyboard::J, "J" },
    { sf::Keyboard::K, "K" },
    { sf::Keyboard::L, "L" },
    { sf::Keyboard::LAlt, "LAlt" },
    { sf::Keyboard::LBracket, "[" },
    { sf::Keyboard::LBracket, "LBracket" },
    { sf::Keyboard::LControl, "LControl" },
    { sf::Keyboard::LControl, "LCtrl" },
    { sf::Keyboard::Left, "Left" },
    { sf::Keyboard::Left, "LeftArrow" },
    { sf::Keyboard::LShift, "LShift" },
    { sf::Keyboard::LSystem, "LSystem" },
    { sf::Keyboard::M, "M" },
    { sf::Keyboard::Menu, "Menu" },
    { sf::Keyboard::Multiply, "*" },
    { sf::Keyboard::Multiply, "Multiply" },
    { sf::Keyboard::N, "N" },
    { sf::Keyboard::Num0, "Num0" },
    { sf::Keyboard::Num1, "Num1" },
    { sf::Keyboard::Num2, "Num2" },
    { sf::Keyboard::Num3, "Num3" },
    { sf::Keyboard::Num4, "Num4" },
    { sf::Keyboard::Num5, "Num5" },
    { sf::Keyboard::Num6, "Num6" },
    { sf::Keyboard::Num7, "Num7" },
    { sf::Keyboard::Num8, "Num8" },
    { sf::Keyboard::Num9, "Num9" },
    { sf::Keyboard::Numpad0, "Numpad0" },
    { sf::Keyboard::Numpad1, "Numpad1" },
    { sf::Keyboard::Numpad2, "Numpad2" },
    { sf::Keyboard::Numpad3, "Numpad3" },
    { sf::Keyboard::Numpad4, "Numpad4" },
    { sf::Keyboard::Numpad5, "Numpad5" },
    { sf::Keyboard::Numpad6, "Numpad6" },
    { sf::Keyboard::Numpad7, "Numpad7" },
    { sf::Keyboard::Numpad8, "Numpad8" },
    { sf::Keyboard::Numpad9, "Numpad9" },
    { sf::Keyboard::O, "O" },
    { sf::Keyboard::P, "P" },
    { sf::Keyboard::PageDown, "PageDown" },
    { sf::Keyboard::PageUp, "PageUp" },
    { sf::Keyboard::Pause, "Pause" },
    { sf::Keyboard::Period, "." },
    { sf::Keyboard::Period, "Period" },
    { sf::Keyboard::Q, "Q" },
    { sf::Keyboard::Quote, "'" },
    { sf::Keyboard::Quote, "Quote" },
    { sf::Keyboard::R, "R" },
    { sf::Keyboard::RAlt, "RAlt" },
    { sf::Keyboard::RBracket, "]" },
    { sf::Keyboard::RBracket, "RBracket" },
    { sf::Keyboard::RControl, "RControl" },
    { sf::Keyboard::RControl, "RCtrl" },
    { sf::Keyboard::Right, "Right" },
    { sf::Keyboard::Right, "RightArrow" },
    { sf::Keyboard::RShift, "RShift" },
    { sf::Keyboard::RSystem, "RSystem" },
    { sf::Keyboard::S, "S" },
    { sf::Keyboard::Semicolon, ";" },
    { sf::Keyboard::Semicolon, "Semicolon" },
    { sf::Keyboard::Slash, "/" },
    { sf::Keyboard::Slash, "Slash" },
    { sf::Keyboard::Space, "Space" },
    { sf::Keyboard::Subtract, "Numpad-" },
    { sf::Keyboard::Subtract, "Subtract" },
    { sf::Keyboard::T, "T" },
    { sf::Keyboard::Tab, "Tab" },
    { sf::Keyboard::Tilde, "~" },
    { sf::Keyboard::Tilde, "Tilde" },
    { sf::Keyboard::U, "U" },
    { sf::Keyboard::Up, "Up" },
    { sf::Keyboard::Up, "UpArrow" },
    { sf::Keyboard::V, "V" },
    { sf::Keyboard::W, "W" },
    { sf::Keyboard::X, "X" },
    { sf::Keyboard::Y, "Y" },
    { sf::Keyboard::Z, "Z" },
};

const enumname
sfmouse_table[] = {
    { sf::Mouse::Left, "Mouse1" },
    { sf::Mouse::Left, "MouseLeft" },
    { sf::Mouse::Right, "Mouse2" },
    { sf::Mouse::Right, "MouseRight" },
    { sf::Mouse::Middle, "Mouse3" },
    { sf::Mouse::Middle, "MouseMiddle" },
    { sf::Mouse::XButton1, "Mouse4" },
    { sf::Mouse::XButton1, "XButton1" },
    { sf::Mouse::XButton2, "Mouse5" },
    { sf::Mouse::XButton2, "XButton2" },
    //{ sf::Mouse::VerticalWheel, "MouseWheel" },
    //{ sf::Mouse::HorizontalWheel, "MouseHWheel" },
};


std::string_view serialize(sf::Keyboard::Key key) noexcept
{
    const auto End = std::end(sfkeyboard_table);
    auto it = std::find(std::begin(sfkeyboard_table), End, key);

    if (it != End) {
        return { it->name.data(), it->name.size() };
    }
    else return "???";
}
std::string_view serialize(Mouse::Button btn) noexcept
{
    const auto End = std::end(sfmouse_table);
    auto it = std::find(std::begin(sfmouse_table), End, btn);

    if (it != End) {
        return { it->name.data(), it->name.size() };
    }
    else return "???";
}

Keyboard::Key parse_kbkey(std::string_view text) noexcept
{
    const auto End = std::end(sfkeyboard_table);
    auto citext = util::ci_string_view(text.data(), text.size());
    auto it = std::find(std::begin(sfkeyboard_table), End, citext);
    return it != End ? Keyboard::Key(it->value) : Keyboard::Unknown;
}

Mouse::Button parse_mousebtn(std::string_view text) noexcept
{
    const auto End = std::end(sfmouse_table);
    auto citext = util::ci_string_view(text.data(), text.size());
    auto it = std::find(std::begin(sfmouse_table), End, citext);
    return it != End ? Mouse::Button(it->value) : Mouse::Button(-1);
}

} // unnamed ns
