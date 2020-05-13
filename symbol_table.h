#pragma once
#include <algorithm>
#include <utility>
#include <set>
#include <stdexcept>


template<typename Stringish, typename Value>
class symbol_table : std::set<std::pair<Stringish, Value>>
{
public:
    using string_type = Stringish;
    using value_type = Value;
    using node = std::pair<string_type, value_type>;
    
    using std::set<node>::set;
    using std::set<node>::size;
    using std::set<node>::empty;
    using std::set<node>::swap;
    using std::set<node>::begin;
    using std::set<node>::end;
    //---

    auto find(value_type const& val) const {
        return std::find_if(this->begin(), this->end(), [&](node const& item) {
            return item.second == val;
        });
    }
    auto find(string_type const& name) const {
        return std::find_if(this->begin(), this->end(), [&](node const& item) {
            return item.first == name;
        });
    }

    auto get_name(value_type const& val) const -> string_type
    {
        auto it = find(val);
        if (it != this->end()) {
            return it->first;
        }
        // not found
        throw std::out_of_range("no name associated with this value");
    }

    auto get_value(string_type name) const -> value_type
    {
        auto it = find(name);
        if (it != this->end()) {
            return it->second;
        }
        // not found
        throw std::out_of_range("no value associated with this name");
    }

    auto operator[] (value_type const& val) const {
        return get_name(val);
    }
    auto operator[] (string_type name) const {
        return get_value(name);
    }

private:
};

