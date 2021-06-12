#include <string>
#include <string_view>
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

#define TRAITS(traits) typename traits ## ::char_type, traits

template<class E, class Tr = std::char_traits<char>, class Alloc = std::allocator<typename Tr::char_type>>
using iostream_translator = boost::property_tree::stream_translator<TRAITS(Tr), Alloc, E>;

namespace boost::property_tree
{
#define STDTRAITS(c) TRAITS(std::char_traits<c>)

    template<>
    struct customize_stream<STDTRAITS(char), Keyboard::Key>
    {
        static void insert(std::ostream& os, Keyboard::Key key)
        {
            os << conv::to_string_view(key);
        }

        static void extract(std::istream& is, Keyboard::Key& key)
        {
            std::string token;
            is >> token;
            conv::parse(token, key);
        }
    };

    template<>
    struct customize_stream<STDTRAITS(char), Mouse::Button>
    {
        static auto insert(std::ostream& os, Mouse::Button btn)
        {
            os << conv::to_string_view(btn);
        }
        static void extract(std::istream& is, Mouse::Button& btn)
        {
            std::string token;
            is >> token;
            conv::parse(token, btn);
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
