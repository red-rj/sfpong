#pragma once
#include <utility>
#include <map>

// Data structure for converting Keys to Values and vice-versa
template<typename Key, typename Value, typename Compare = std::less<>>
class symbol_table
{
    template<class K, class V>
    using storage_t = std::pmr::map<K, V, Compare>;
public:
    using key_type = Key;
    using value_type = Value;
    using key_value_compare = Compare;

    //---
    symbol_table(std::initializer_list<std::pair<key_type, value_type>> init)
        : m_keys_to_values(init.begin(), init.end())
    {
        for (auto& [key, value] : init)
        {
            m_values_to_keys[value] = key;
        }
    }

    auto& operator[] (value_type const& val) const {
        return m_values_to_keys.at(val);
    }
    auto& operator[] (key_type const& name) const {
        return m_keys_to_values.at(name);
    }

private:
    storage_t<key_type, value_type> m_keys_to_values;
    storage_t<value_type, key_type> m_values_to_keys;
};
