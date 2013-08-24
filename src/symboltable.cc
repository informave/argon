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
/////////////////////////////////////////////////////////////// ScopedSubSymbols

/// @details
/// 
ScopedSubSymbols::ScopedSubSymbols(SymbolTable &st)
    : m_subptr(0),
      m_symbols(st)
{
    m_subptr = st.createSub();
}


/// @details
/// 
ScopedSubSymbols::~ScopedSubSymbols(void)
{
    if(m_subptr)
        this->m_symbols.deleteSub(this->m_subptr);
        
}


//..............................................................................
//////////////////////////////////////////////////////////////////// SymbolTable

/// @details
/// 
SymbolTable::SymbolTable(SymbolTable *parent)
    : m_sub_symbols(),
      m_parent(parent)
{
    this->createSub(); // create initial symbol table
}


/// @details
/// 
SymbolTable::~SymbolTable(void)
{
    ARGON_ICERR(this->m_sub_symbols.size() == 1, "reset() can only called when no sub symboltables active.");
    delete *this->m_sub_symbols.begin();
    this->m_sub_symbols.pop_front();
}



/// @details
/// 
SymbolTable::map_type*
SymbolTable::createSub(void)
{
    map_type *p = new map_type();
    this->m_sub_symbols.push_front(p);
    return p;
}


/// @details
/// 
void
SymbolTable::deleteSub(SymbolTable::map_type* ptr)
{
    ARGON_ICERR(*this->m_sub_symbols.begin() == ptr, "deleteSub() can only delete the topmost table");
    delete *this->m_sub_symbols.begin();
    this->m_sub_symbols.pop_front();
}



/// @details
/// 
SymbolTable::map_type&
SymbolTable::getCurrentSub(void)
{
    ARGON_ICERR(this->m_sub_symbols.size() > 0, "sub_symbols = 0");
    return **this->m_sub_symbols.begin();
}


/// @details
/// 
const SymbolTable::map_type&
SymbolTable::getCurrentSub(void) const
{
    ARGON_ICERR(this->m_sub_symbols.size() > 0, "sub_symbols = 0");
    return **this->m_sub_symbols.begin();
}


/// @details
/// 
void
SymbolTable::add(Identifier name, const Ref &ref)
{
    ARGON_ICERR(name.str().length() > 0, "empty id");
            
    map_type::iterator i = this->getCurrentSub().find(name);
    if(i != this->getCurrentSub().end())
        throw std::runtime_error("duplicated symbol error: " + std::string(name.str()));
    //this->m_symbols[name] = ref;
    this->getCurrentSub().insert(map_type::value_type(name, ref));
}


void
SymbolTable::cloneSymbols(const SymbolTable &from)
{
    for(list_map_type::const_iterator j = from.m_sub_symbols.begin();
        j != from.m_sub_symbols.end();
        ++j)
    {
        std::for_each((*j)->begin(), (*j)->end(), [this](map_type::value_type &v){
                try
                {
                    this->add(v.first, v.second);
                }
                catch(std::runtime_error &)
                {
                    /// @bug use custom exception, see add() method
                }
            });
    }
    
}


/// @details
/// @todo iterate over all sub tables
String
SymbolTable::str(void) const
{
    std::wstringstream ss;
    ss << L"SYMBOL TABLE OUTPUT: " << std::endl;

    for(map_type::const_iterator i = getCurrentSub().begin();
        i != getCurrentSub().end();
        ++i)
    {
        ss << i->first.str() << L"\t\t:\t" << i->second->str() << std::endl;
    }
    return ss.str();
}


void
SymbolTable::reset(void)
{
    ARGON_ICERR(this->m_sub_symbols.size() == 1, "reset() can only called when no sub symboltables active.");
    this->getCurrentSub().clear();
}


/// @details
/// 
Ref
SymbolTable::find_element(Identifier name)
{
    for(list_map_type::iterator j = this->m_sub_symbols.begin();
        j != this->m_sub_symbols.end();
        ++j)
    {
        map_type::iterator i = (*j)->find(name);
        if(i != (*j)->end())
        {
            Ref ptr = i->second;
            return ptr;
        }
    }
     
    

    {
        /// @bug really required? no, but rename it to m_global
        if(this->m_parent)
        {
            return this->m_parent->find_element(name);
        }
        else
        {
            String s("element not found: ");
            s.append(name.str());
            throw std::runtime_error(s); /// @bug custom exception
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

