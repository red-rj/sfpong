#pragma once
#include <string>
#include <string_view>
#include <iostream>
#include <cwctype>

namespace red
{
    template<class T>
    struct ci_char_traits : public std::char_traits<T> {
        using typename std::char_traits<T>::char_type;

        static bool eq(char_type c1, char_type c2) {
            return to_upper(c1) == to_upper(c2);
        }
        static bool lt(char_type c1, char_type c2) {
            return to_upper(c1) < to_upper(c2);
        }
        static int compare(const char_type* s1, const char_type* s2, size_t n) {
            while (n-- != 0) {
                char const uch[] = { to_upper(*s1), to_upper(*s2) };
                if (uch[0] < uch[1]) return -1;
                if (uch[0] > uch[1]) return 1;
                ++s1; ++s2;
            }
            return 0;
        }
        static const char_type* find(const char_type* s, int n, char_type a) {
            auto const ua = to_upper(a);
            while (n-- != 0)
            {
                if (to_upper(*s) == ua)
                    return s;
                s++;
            }
            return nullptr;
        }

    private:
        static T to_upper(T c) noexcept
        {
            using std::make_unsigned_t;
            using std::is_same_v;

            if constexpr (is_same_v<make_unsigned_t<T>, unsigned char>) {
                return static_cast<char>(std::toupper((unsigned char)c));
            }
            else if constexpr (is_same_v<T, wchar_t>) {
                return std::towupper(c);
            }
            else {
                static_assert(false, "Unsupported char type");
            }
        }

    };

    using ci_string_view = std::basic_string_view<char, ci_char_traits<char>>;
    using ci_string = std::basic_string<char, ci_char_traits<char>>;
    using ci_wstring_view = std::basic_string_view<wchar_t, ci_char_traits<wchar_t>>;
    using ci_wstring = std::basic_string<wchar_t, ci_char_traits<wchar_t>>;

    constexpr ci_string_view to_ci(std::string_view sv) {
        return { sv.data(), sv.size() };
    }
    constexpr ci_wstring_view to_ci(std::wstring_view sv) {
        return { sv.data(), sv.size() };
    }


    constexpr std::string_view to_string_view(ci_string_view sv) {
        return { sv.data(), sv.size() };
    }

    inline auto& operator<< (std::ostream& os, ci_string_view ciview) noexcept {
        return os << to_string_view(ciview);
    }

}
