#pragma once
#include <string_view>

#include "SFML/Window/Keyboard.hpp"
#include "boost/program_options/value_semantic.hpp"

namespace red
{
    template<class T>
    struct ci_char_traits : public std::char_traits<T> {
        using typename std::char_traits<T>::char_type;

        static char to_upper(char ch) {
            return static_cast<char>(toupper(static_cast<unsigned char>(ch)));
        }
        static wchar_t to_upper(wchar_t ch) {
            return towupper(ch);
        }

        static bool eq(char_type c1, char_type c2) {
            return to_upper(c1) == to_upper(c2);
        }
        static bool lt(char_type c1, char_type c2) {
            return to_upper(c1) < to_upper(c2);
        }
        static int compare(const char_type* s1, const char_type* s2, size_t n) {
            while (n-- != 0) {
                if (to_upper(*s1) < to_upper(*s2)) return -1;
                if (to_upper(*s1) > to_upper(*s2)) return 1;
                ++s1; ++s2;
            }
            return 0;
        }
        static const char_type* find(const char_type* s, int n, char_type a) {
            auto const ua(to_upper(a));
            while (n-- != 0)
            {
                if (to_upper(*s) == ua)
                    return s;
                s++;
            }
            return nullptr;
        }
    };

    using ci_string_view = std::basic_string_view<char, ci_char_traits<char>>;
    using ci_wstring_view = std::basic_string_view<wchar_t, ci_char_traits<wchar_t>>;

    std::string_view nameof(sf::Keyboard::Key key);

    class keybordkey_value : public boost::program_options::typed_value<sf::Keyboard::Key>
    {
    public:
        using typed_value::typed_value;

        bool apply_default(boost::any& storage) const {
            storage = sf::Keyboard::Unknown;
            return true;
        }
    };
}