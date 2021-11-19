#include "convert.h"
#include "ci_string.h"
#include <algorithm>
#include <spdlog/spdlog.h>

struct enumname
{
    int value;
    const char* name;

    constexpr auto ci_name() const {
        return util::ci_string_view(name);
    }

    constexpr bool operator== (int v) const { return v == value; }
    constexpr bool operator== (std::string_view n) const { return n == name; }
    constexpr bool operator== (util::ci_string_view n) const { return n == ci_name(); }
};

// name-value tables
namespace
{
    // gerado por codegen-v2.py
    const enumname sfkeyboard_table[] = {
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

    const enumname sfmouse_table[] = {
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
    };

    const enumname sfmousewheel_table[] = {
        { sf::Mouse::VerticalWheel, "MouseWheel" },
        { sf::Mouse::HorizontalWheel, "MouseHWheel" },
    };
}

namespace conv
{
    auto to_string_view(sf::Keyboard::Key key)->std::string_view {
        const auto End = std::end(sfkeyboard_table);
        auto it = std::find(std::begin(sfkeyboard_table), End, key);
        return it != End ? it->name : "???";
    }
    auto to_string_view(sf::Mouse::Button btn)->std::string_view {
        const auto End = std::end(sfmouse_table);
        auto it = std::find(std::begin(sfmouse_table), End, btn);
        return it != End ? it->name : "???";
    }

    bool parse(std::string_view text, sf::Keyboard::Key& key) {
        const auto End = std::end(sfkeyboard_table);
        auto citext = util::ci_string_view(text.data(), text.size());
        auto it = std::find(std::begin(sfkeyboard_table), End, citext);

        key = it != End ? sf::Keyboard::Key(it->value) : sf::Keyboard::Unknown;
        return it != End;
    }
    bool parse(std::string_view text, sf::Mouse::Button& btn) {
        const auto End = std::end(sfmouse_table);
        auto citext = util::ci_string_view(text.data(), text.size());
        auto it = std::find(std::begin(sfmouse_table), End, citext);

        btn = it != End ? sf::Mouse::Button(it->value) : sf::Mouse::Button(-1);
        return it != End;
    }

    auto to_string_view(pong::playerid pid) noexcept -> std::string_view {
        switch (pid)
        {
        case pong::playerid::one: return "Player 1";
        case pong::playerid::two: return "Player 2";
        default: return "Player ???";
        }
    }

    auto to_string_view(pong::dir d) noexcept ->std::string_view {
        switch (d)
        {
        case pong::dir::up: return "up";
        case pong::dir::down: return "down";
        case pong::dir::left: return "left";
        case pong::dir::right: return "right";
        default: return "???";
        }
    }
}