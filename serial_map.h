#pragma once
#include <utility>
#include <map>

namespace red 
{
    template<typename Stringish, typename Value>
    class serial_map
    {
    public:

        using string_type = Stringish;
        using value_type = Value;

        serial_map(std::initializer_list<std::pair<string_type, value_type>> init)
            : m_name_to_value(init.begin(), init.end())
        {
            for (auto& p : init)
            {
                m_value_to_name[p.second] = p.first;
            }
        }

        auto at(const string_type& key) const {
            return m_name_to_value.at(key);
        }
        auto at(const value_type& key) const {
            return m_value_to_name.at(key);
        }

        auto operator[] (const value_type& key) const {
            return this->at(key);
        }
        auto operator[] (const string_type& key) const {
            return this->at(key);
        }

    private:
        std::map<string_type, value_type> m_name_to_value;
        std::map<value_type, string_type> m_value_to_name;
    };


}