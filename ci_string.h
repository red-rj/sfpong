#pragma once
#include <string>
#include <string_view>
#include <iostream>

#include <fmt/core.h>


namespace red
{
    template<class T>
    struct ci_char_traits : public std::char_traits<T> {
        using typename std::char_traits<T>::char_type;

        static bool eq(char_type c1, char_type c2) {
            std::locale l;
            return toupper(c1, l) == toupper(c2, l);
        }
        static bool lt(char_type c1, char_type c2) {
            std::locale l;
            return toupper(c1, l) < toupper(c2, l);
        }
        static int compare(const char_type* s1, const char_type* s2, size_t n) {
            std::locale l;
            while (n-- != 0) {
                char const uch[] = { toupper(*s1, l), toupper(*s2, l) };
                if (uch[0] < uch[1]) return -1;
                if (uch[0] > uch[1]) return 1;
                ++s1; ++s2;
            }
            return 0;
        }
        static const char_type* find(const char_type* s, int n, char_type a) {
            std::locale l;
            auto const ua = toupper(a, l);
            while (n-- != 0)
            {
                if (toupper(*s, l) == ua)
                    return s;
                s++;
            }
            return nullptr;
        }
    };

    using ci_string_view = std::basic_string_view<char, ci_char_traits<char>>;
    using ci_string = std::basic_string<char, ci_char_traits<char>>;

    constexpr ci_string_view to_ci(std::string_view sv) {
        return { sv.data(), sv.size() };
    }
    constexpr std::string_view to_string_view(ci_string_view sv) {
        return { sv.data(), sv.size() };
    }

    inline auto& operator<< (std::ostream& os, ci_string_view ciview) noexcept {
        return os << to_string_view(ciview);
    }

}
