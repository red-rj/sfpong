#pragma once
#include <boost/property_tree/ptree_fwd.hpp>
#include <SFML/Window/Keyboard.hpp>

#include "common.h"

namespace std::filesystem {
    class path;
}

namespace pong
{
    using cfgtree = boost::property_tree::ptree;

    /*void overrideGuts(const cfgtree& tree);
    cfgtree createGuts();*/

// config keys
namespace ckey
{
    // game.cfg
    constexpr auto
        P1_UP = "player1.up",
        P1_DOWN = "player1.down",
        P1_FAST = "player1.fast",
        P1_JOYSTICK = "player1.joystick",
        P1_JSDEADZONE = "player1.joystick_deadzone",
        
        P2_UP = "player2.up",
        P2_DOWN = "player2.down",
        P2_FAST = "player2.fast",
        P2_JOYSTICK = "player2.joystick",
        P2_JSDEADZONE = "player2.joystick_deadzone",

        RESOLUTION_X = "game.resolution_x",
        RESOLUTION_Y = "game.resolution_y",
        FULLSCREEN = "game.fullscreen"
        ;
}

// config structs

    struct keyboard_ctrls
    {
        sf::Keyboard::Key up, down, fast;

        bool operator== (const keyboard_ctrls& rhs) const noexcept;
        bool operator!= (const keyboard_ctrls& rhs) const noexcept {
            return !(*this == rhs);
        }
    };
    
    // modelo de game.cfg
    class game_settings
    {
        keyboard_ctrls player_keys[2];
        int player_joystick[2];
        float player_deadzone[2];

        sf::Vector2u win_resolution;
        bool win_fullscreen;

    public:

        auto& keyboard_keys(playerid pid) noexcept { return player_keys[int(pid)]; }
        auto& get_keyboard_keys(playerid pid) const noexcept { return player_keys[int(pid)]; }
        void set_keyboard_keys(playerid pid, keyboard_ctrls ctrls) noexcept { player_keys[int(pid)] = ctrls; }

        static const int njoystick = -1;
        auto get_joystick(playerid pid) const noexcept { return player_joystick[int(pid)]; }
        void set_joystick(playerid pid, int joyid) noexcept;

        void unset_joystick(playerid pid) noexcept {
            player_joystick[int(pid)] = njoystick;
        }
        bool using_joystick(playerid pid) const noexcept {
            return player_joystick[int(pid)] != njoystick;
        }

        auto& joystick_deadzone(playerid pid) noexcept { return player_deadzone[int(pid)]; }

        auto& resolution() noexcept { return win_resolution; }
        auto& fullscreen() noexcept { return win_fullscreen; }

        // IO
        void load_tree(const cfgtree& cfg);
        void load_file(std::filesystem::path const& iniPath);

        void save_tree(cfgtree& cfg) const;
        void save_file(std::filesystem::path const& iniPath) const;


        bool operator== (const game_settings& rhs) const noexcept;
        bool operator!= (const game_settings& rhs) const noexcept { return !(*this == rhs); }

    };


}