//
// symboltable.cc - Symbol Table (definition)
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

#include <iostream>
#include <stack>

ARGON_NAMESPACE_BEGIN

//..............................................................................
//////////////////////////////////////////////////////////////////// SymbolTable

/// @details
/// 
SymbolTable::SymbolTable(SymbolTable *parent)
    : m_symbols(),
      m_heap(),
      m_parent(parent)
{}


/// @details
/// 
SymbolTable::~SymbolTable(void)
{
    this->freeHeap();
}


/// @details
/// During runtime, new objects are insert at the end.
/// Because some object depends on other objects (e.h. Statements -> Connection)
/// we must ensure that the destruction of such objects is in reverse order.
void
SymbolTable::freeHeap(void)
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
SymbolTable::add(Identifier name, Element *symbol)
{
    assert(name.str().length() > 0);
            
    element_map::iterator i = this->m_symbols.find(name);
    if(i != this->m_symbols.end())
        throw std::runtime_error("duplicated symbol error: " + std::string(name.str()));
    this->m_symbols[name] = symbol;
}


/// @details
/// 
String
SymbolTable::str(void) const
{
    std::wstringstream ss;
    ss << L"SYMBOL TABLE OUTPUT: " << std::endl;

    for(element_map::const_iterator i = m_symbols.begin();
        i != m_symbols.end();
        ++i)
    {
        ss << i->first.str() << L"\t\t:\t" << i->second->str() << std::endl;
    }
    return ss.str();
}


/// @details
/// 
Element*
SymbolTable::find_element(Identifier name)
{
    element_map::iterator i = this->m_symbols.find(name);
    if(i != this->m_symbols.end())
    {
        Element* ptr = i->second;
        return ptr;
    }
    else
    {
        if(this->m_parent)
        {
            return this->m_parent->find_element(name);
        }
        else
        {
            String s("element not found: ");
            s.append(name.str());
            throw std::runtime_error(s);
        }
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

