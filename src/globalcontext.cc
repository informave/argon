//
// globalcontext.cc - Global context (definition)
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
/// @brief Global context (definition)
/// @author Daniel Vogelbacher
/// @since 0.1

#include "argon/dtsengine.hh"
#include "argon/exceptions.hh"
#include "debug.hh"
#include "visitors.hh"

#include "argon/rtti.hh"
#include "builtin/functions.hh"


#include <iostream>
#include <functional>
#include <sstream>

ARGON_NAMESPACE_BEGIN


//..............................................................................
////////////////////////////////////////////////////////////////// GlobalContext



/// @details
/// 
String
GlobalContext::str(void) const
{
    return "[GlobalContext]";
}


/// @details
/// 
String
GlobalContext::name(void) const
{
    return ARGON_UNNAMED_ELEMENT;
}

/// @details
/// 
String
GlobalContext::type(void) const
{
    return "GlobalContext";
}


/// @details
/// 
SourceInfo
GlobalContext::getSourceInfo(void) const
{
    /// @bug implement me
    return SourceInfo();
}



/// @details
/// 
GlobalContext::GlobalContext(Processor &proc, Node *tree)
    : Context(proc, 0, ArgumentList()), // set parent symboltable to NULL
      m_tree(tree)
{}



/// @details
/// 
Value
GlobalContext::run(void)
{
    ARGON_DPRINT(ARGON_MOD_PROC, "Running script");

    ARGON_SCOPED_STACKFRAME(this->proc());
    Node::nodelist_type childs = this->m_tree->getChilds();

    // Phase 2
    // for each module and the progam
    std::for_each(childs.begin(), childs.end(),
                  [this](Node::nodelist_type::value_type node){
                      foreach_node( node->getChilds(), Pass2Visitor(this->proc(), *this), 1);
                  });

    return this->proc().call(Identifier("main"));
}



/// @details
/// 
Object*
GlobalContext::getMainObject(void)
{
    ARGON_ICERR_CTX(false, *this,
                "global-context does not contains a result object.");
}


/// @details
/// 
Object*
GlobalContext::getResultObject(void) 
{ 
    ARGON_ICERR_CTX(false, *this,
                "global-context does not contains a result object.");
}


/// @details
/// 
Object*
GlobalContext::getDestObject(void)
{
    ARGON_ICERR_CTX(false, *this,
                "global-context does not contains a destination object.");
}



/// @details
/// 
Value
GlobalContext::_value(void) const
{
    return String("GLOBALCONTEXT"); /// @todo return function name
}

/// @details
/// 
String
GlobalContext::_string(void) const
{
    return this->_value().data().asStr();
}

/// @details
/// 
String
GlobalContext::_name(void) const
{
    return String("unknown-id") + String(" (globalcontext)");
    //return this->id().str() + String(" (function)");
}

/// @details
/// 
String
GlobalContext::_type(void) const
{
    return "GLOBALCONTEXT";
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
