#pragma once
#include <string>
#include <string_view>
#include <locale>

namespace util
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
                char_type uc1 = to_upper(*s1), uc2 = to_upper(*s2);
                if (uc1 < uc2) return -1;
                if (uc1 > uc2) return 1;
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
        static T to_upper(T c)
        {
            return toupper(c, std::locale::classic());
        }

    };

    using ci_string_view = std::basic_string_view<char, ci_char_traits<char>>;

    inline constexpr int ci_compare(std::string_view lhs, std::string_view rhs)
    {
        return ci_string_view(lhs.data(), lhs.size()).compare(ci_string_view(rhs.data(), rhs.size()));
    }
}
