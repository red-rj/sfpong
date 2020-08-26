#pragma once
#include <algorithm>
#include <utility>
#include <stdexcept>
#include <map>


template<typename Stringish, typename Value, typename Compare = std::less<>>
class symbol_table
{
    template<class K, class V>
    using storage_t = std::pmr::map<K, V, Compare>;
public:
    using string_type = Stringish;
    using value_type = Value;
    using key_compare = Compare;

    //---
    symbol_table(std::initializer_list<std::pair<string_type, value_type>> init)
        : m_keys_to_values(init.begin(), init.end())
    {
        for (auto& [key, value] : init)
        {
            m_values_to_keys[value] = key;
        }
    }

    auto get_name(value_type const& val) const -> string_type const&
    {
        return m_values_to_keys.at(val);
    }

    auto get_value(string_type const& name) const -> value_type const&
    {
        return m_keys_to_values.at(name);
    }

    auto& operator[] (value_type const& val) const {
        return get_name(val);
    }
    auto& operator[] (string_type const& name) const {
        return get_value(name);
    }

private:
    storage_t<string_type, value_type> m_keys_to_values;
    storage_t<value_type, string_type> m_values_to_keys;
};
