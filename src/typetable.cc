//
// typetable.cc - Symbol Table (definition)
//
// Copyright (C)         informave.org
//   2010,               Daniel Vogelbacher <daniel@vogelbacher.name>
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
/// @brief Symbol Table (definition)
/// @author Daniel Vogelbacher
/// @since 0.1


#include "argon/dtsengine.hh"
#include "argon/exceptions.hh"
//#include "semantic.hh"
#include "debug.hh"
#include <iostream>
#include <stack>

ARGON_NAMESPACE_BEGIN

//..............................................................................
//////////////////////////////////////////////////////////////////// TypeTable

/// @details
/// 
TypeTable::TypeTable(void)
    : m_types(),
      m_heap()
{}


/// @details
/// 
TypeTable::~TypeTable(void)
{
    this->freeHeap();
}


/// @details
/// During runtime, new objects are insert at the end.
/// Because some object depends on other objects (e.h. Statements -> Connection)
/// we must ensure that the destruction of such objects is in reverse order.
void
TypeTable::freeHeap(void)
{
    for(heap_type::reverse_iterator i = m_heap.rbegin();
        i != m_heap.rend();
        ++i)
    {
        delete *i;
        *i = 0;
    }
    this->m_heap.clear();
}


/// @details
/// 
void
TypeTable::add(Type *type)
{
    ARGON_ICERR(type->id().str().length() > 0, "empty type id");
            
    element_map::iterator i = this->m_types.find(type->id());
    if(i != this->m_types.end())
        throw std::runtime_error("duplicated type error: " + std::string(type->id().str()));
    this->m_types[type->id()] = type;

    /// @bug is is correct do handle deletion of types in destructor?
    this->m_heap.push_back(type);
}


/// @details
/// 
String
TypeTable::str(void) const
{
    std::wstringstream ss;
    ss << L"SYMBOL TABLE OUTPUT: " << std::endl;

    for(element_map::const_iterator i = m_types.begin();
        i != m_types.end();
        ++i)
    {
        ss << i->first.str() << L"\t\t:\t" << i->second->str() << std::endl;
    }
    return ss.str();
}


/// @details
/// 
Type*
TypeTable::find_type(Identifier name)
{
    element_map::iterator i = this->m_types.find(name);
    if(i != this->m_types.end())
    {
        Type* ptr = i->second;
        return ptr;
    }
    else
    {
        String s("type not found: ");
        s.append(name.str());
        throw std::runtime_error(s);
    }   
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

