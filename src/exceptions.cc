//
// exceptions.cc - Exceptions (definition)
//
// Copyright (C)         informave.org
//   2010,               Daniel Vogelbacher <daniel@vogelbacher.name>
// 
// GPL 3.0 License
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

/// @file
/// @brief Exceptions (definition)
/// @author Daniel Vogelbacher
/// @since 0.1

#include "argon/exceptions.hh"
#include "argon/token.hh"

#include <sstream>
#include <iostream>

ARGON_NAMESPACE_BEGIN


//..............................................................................
/////////////////////////////////////////////////////////////////// RuntimeError

/// @details
/// 
RuntimeError::RuntimeError(const Processor::stack_type &stack)
    : m_stack(stack)
{
    std::wstringstream ss;

    ss << L"Stack trace:" << std::endl;

    for(Processor::stack_type::const_iterator i = m_stack.begin();
        i != m_stack.end();
        ++i)
    {
        ss << (*i)->type() << L" " << (*i)->name()
           << L" (" << (*i)->getSourceInfo().sourceName() << L":"
           << (*i)->getSourceInfo().linenum() << L")" << std::endl;
    }

    this->m_what = ss.str();
}


//..............................................................................
//////////////////////////////////////////////////////////////////// SyntaxError

/// @details
/// The given token can be safety freed
SyntaxError::SyntaxError(Token *t)
    
{
    std::wstringstream ss;
    ss << t->getSourceInfo().sourceName() << L":" << t->getSourceInfo().linenum() << L": "
       << L"(AEC5001) "
       << L"Syntax error "
       << L"near token "
       << t->data();

    this->m_what = ss.str();
}


/// @details
/// 
const char*
Exception::what(void) const throw()
{
    m_tmp = this->m_what;

    return m_tmp.c_str();
}


ARGON_NAMESPACE_END


//
// Local Variables:
// mode: C++
// c-file-style: "bsd"
// c-basic-offset: 4
// indent-tabs-mode: nil
// End:
//
