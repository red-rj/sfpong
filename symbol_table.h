#pragma once
#include <algorithm>
#include <utility>
#include <set>
#include <stdexcept>


template<typename Stringish, typename Value, typename Compare = std::less<>>
class symbol_table : std::pmr::set<std::pair<Stringish, Value>, Compare>
{
    using base_type = std::pmr::set<std::pair<Stringish, Value>, Compare>;
public:
    using typename base_type::allocator_type;
    using string_type = Stringish;
    using value_type = Value;
    using pair_type = typename base_type::key_type;

    using base_type::set;
    using base_type::size;
    using base_type::empty;
    using base_type::swap;
    using base_type::begin;
    using base_type::end;
    //---

    auto find(value_type const& val) const {
        return std::find_if(begin(), end(), [&](pair_type const& item) {
            return item.second == val;
        });
    }
    auto find(string_type const& name) const {
        return base_type::find(name);
    }

    auto get_name(value_type const& val) const -> string_type const&
    {
        auto it = find(val);
        if (it != end()) {
            return it->first;
        }
        // not found
        throw std::out_of_range("no name associated with this value");
    }

    auto get_value(string_type const& name) const -> value_type const&
    {
        auto it = find(name);
        if (it != end()) {
            return it->second;
        }
        // not found
        throw std::out_of_range("no value associated with this name");
    }

    auto& operator[] (value_type const& val) const {
        return get_name(val);
    }
    auto& operator[] (string_type const& name) const {
        return get_value(name);
    }

private:
};

