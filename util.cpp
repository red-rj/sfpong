#include <string>

#include "util.h"


auto red::parse_kb_key(red::ci_string_view sv) -> sf::Keyboard::Key {
    using Key = sf::Keyboard::Key;

    int key = Key::Unknown;

    if (sv.length() == 1) {
        const char c = sv[0];
        if (c >= '0' && c <= '9') {
            key = (c - '0') + Key::Num0;
        }
        else if (auto uc = toupper(c); uc >= 'A' && uc <= 'Z') {
            key = uc - 'A';
        }
        else switch (c)
        {
        case '[': key = Key::LBracket; break;
        case ']': key = Key::RBracket; break;
        case ';': key = Key::Semicolon; break;
        case ',': key = Key::Comma; break;
        case '.': key = Key::Period; break;
        case '\'': key = Key::Quote; break;
        case '/': key = Key::Slash; break;
        case '\\': key = Key::Backslash; break;
        case '~': key = Key::Tilde; break;
        case '=': key = Key::Equal; break;
        case '-': key = Key::Hyphen; break;
        case '+': key = Key::Add; break;
        //case '-': key = Key::Subtract; break;
        case '*': key = Key::Multiply; break;
        //case '/': key = Key::Divide; break;
        }
    }
    else if (sv.length() == 7 && sv.compare(0, 6, "numpad") == 0) {
        const char num = sv[6];
        if (num >= '0' && num <= '9') {
            key = (num - '0') + Key::Numpad0;
        }
    }
    else if (sv.length() <= 3 && sv.compare(0, 1, "F") == 0) {
        sv.remove_prefix(1);
        int fnum = std::stoi(std::string(sv.data(), sv.size()));
        key = fnum - 1 + Key::F1;
    }
    else {
        if (sv == "Escape" || sv == "Esc") {
            key = Key::Escape;
        } else if (sv == "LControl") {
            key = Key::LControl;
        } else if (sv == "LAlt") {
            key = Key::LAlt;
        } else if (sv == "LShift") {
            key = Key::LShift;
        } else if (sv == "RControl") {
            key = Key::RControl;
        } else if (sv == "RShift") {
            key = Key::RShift;
        } else if (sv == "RAlt") {
            key = Key::RAlt;
        } else if (sv == "Menu") {
            key = Key::Menu;
        } else if (sv == "Space") {
            key = Key::Space;
        } else if (sv == "Enter") {
            key = Key::Enter;
        } else if (sv == "Backspace") {
            key = Key::Backspace;
        } else if (sv == "Tab") {
            key = Key::Tab;
        } else if (sv == "PageUp") {
            key = Key::PageUp;
        } else if (sv == "PageDown") {
            key = Key::PageDown;
        } else if (sv == "End") {
            key = Key::End;
        } else if (sv == "Home") {
            key = Key::Home;
        } else if (sv == "Insert") {
            key = Key::Insert;
        } else if (sv == "Delete") {
            key = Key::Delete;
        } else if (sv == "Left") {
            key = Key::Left;
        } else if (sv == "Right") {
            key = Key::Right;
        } else if (sv == "Up") {
            key = Key::Up;
        } else if (sv == "Down") {
            key = Key::Down;
        } else if (sv == "Pause") {
            key = Key::Pause;
        }
    }

    if (key == Key::Unknown)
        throw std::runtime_error("Unknown key");

    return Key(key);
}

