//
// strutils.hh - String utilities
//
// Copyright (C)         informave.org
//   2012,               Daniel Vogelbacher <daniel@vogelbacher.name>
//
// Lesser GPL 3.0 License
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

/// @file
/// @brief String utilities
/// @author Daniel Vogelbacher
/// @since 0.1

#include <iostream>
#include <string>
#include <iterator>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

#ifndef INFORMAVE_STRUTILS_HH
#define INFORMAVE_STRUTILS_HH


namespace strutils
{
    // Trim left
    template<typename T>
    inline T ltrim(T s)
    {
        s.erase(s.begin(),
                std::find_if(s.begin(), s.end(),
                             std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
    }

    // Trim right
    template<typename T>
    inline T rtrim(T s)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(),
                             std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
    }

    // Trim both
    template<typename T>
    inline T trim(T s)
    {
        return ltrim(rtrim(s));
    }


    // find single char separator
    template<typename T>
    inline std::pair<T, T> find_separator(T start, T end, T search)
    {
        start = std::find(start, end, *search);
        return std::pair<T, T>(start, start != end ? (start+1) : end);
    }


    // find string separator
    template<typename T>
    inline std::pair<T, T> find_separator(T start, T end, T search, T search_end)
    {
        while(start != end)
        {
            if(*start != *search)
                ++start;
            else
            {
                T backup = search;
                T i = start;
                while(start != end && search != search_end && *start == *search)
                {
                    ++start;
                    ++search;
                }
                if(search == search_end)
                    return std::pair<T, T>(i, start);

                search = backup;
            }
        }
        return std::pair<T, T>(start, start);
    }


    // split string
    template<typename T>
    class split
    {
    public:
        typedef T                                         string_type;
        typedef typename T::const_iterator                iterator_type;
        typedef std::pair<iterator_type, iterator_type>   pair_type;

        explicit split(iterator_type begin, iterator_type end, const string_type &separator)
            : m_begin(begin), m_end(end), m_start(), m_stop(), m_next(begin), m_separator(separator)
        {}

        bool next(void)
        {
            if(m_next == m_end && m_stop != m_next)
            {
                m_start = m_next;
                m_stop = m_end;
                return true;
            }
            else
            {
                std::pair<iterator_type, iterator_type> p =
                    m_separator.size() == 1
                    ? find_separator(m_next, m_end, m_separator.begin())
                    : find_separator(m_next, m_end, m_separator.begin(), m_separator.end());
                m_start = m_next;
                m_stop = p.first;
                m_next = p.second;
                return m_start  != m_end;
            }
        }

        inline pair_type get_current(void) const
        {
            return std::pair<iterator_type, iterator_type>(m_start, m_stop);
        }

        string_type get_string(void)
        {
            return string_type(get_current().first, get_current().second);
        }

    private:
        iterator_type m_begin;
        iterator_type m_end;
        iterator_type m_start;
        iterator_type m_stop;
        iterator_type m_next;
        const string_type m_separator;
    };
}



#endif

//
// Local Variables:
// mode: C++
// c-file-style: "bsd"
// c-basic-offset: 4
// indent-tabs-mode: nil
// End:
//
