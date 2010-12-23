//
// elements.cc - Elements (definition)
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
/// @brief Elements (definition)
/// @author Daniel Vogelbacher
/// @since 0.1

#include "argon/dtsengine.hh"
#include "argon/exceptions.hh"
#include "debug.hh"
#include "visitors.hh"

#include <iostream>
#include <sstream>

ARGON_NAMESPACE_BEGIN


//..............................................................................
///////////////////////////////////////////////////////////////////////// Column

/// @details
/// 
const db::Value&
Column::getFrom(db::Resultset &rs, Context &context)
{
    using informave::db::ex::not_found;
    try
    {
        if(Column::by_number == mode())
            return rs.column(getNum());
        else
            return rs.column(getName());
    }
    catch(not_found &e)
    {
        throw FieldNotFound(context, e.what()); /// @bug replace what with colum name
    }
}



//..............................................................................
///////////////////////////////////////////////////////////////////////// Object

/// @details
/// 
Object::Object(Processor &proc, ObjectNode *node)
    : Context(proc),
      m_node(node)
{}


/// @details
/// This runs some code that is required by all objects like Arguments->Symboltable
Value
Object::run(const ArgumentList &args)
{
    ARGON_DPRINT(ARGON_MOD_PROC, "Running task " << this->id());

    // Load OBJECTs arguments to local symbol table
    this->getSymbols().reset();
    safe_ptr<ArgumentsSpecNode> argsSpecNode = find_node<ArgumentsSpecNode>(this->m_node);

    ARGON_ICERR(argsSpecNode->getChilds().size() == args.size(), *this,
                "Argument count mismatch");
    
    ArgumentList::const_iterator i = args.begin();
    foreach_node(argsSpecNode->getChilds(), Arg2SymVisitor(this->proc(), *this, i), 1);

    return Value();
}



/// @details
/// 
Value
Object::resolve(const Column &col)
{
    throw std::runtime_error("resolve() not allowed on objects");
}


/// @details
/// 
Object*
Object::getMainObject(void)
{
    return this; /// @bug is this ok? maybe we need this for query() calls from object bodies
    // we should check the object mode
}


/// @details
/// 
Object*
Object::getResultObject(void) 
{ 
    ARGON_ICERR(false, *this,
                "An object does not contains a result object.");
}


/// @details
/// 
Object*
Object::getDestObject(void)
{
    ARGON_ICERR(false, *this,
                "An object does not contains a destination object.");
}




//..............................................................................
//////////////////////////////////////////////////////////////////////// Element

/// @details
/// 
Element::Element(Processor &proc)
    : m_proc(proc)
{}


//..............................................................................
//////////////////////////////////////////////////////////////////////// Context

/// @details
/// 
Context::Context(Processor &proc)
    : Element(proc),
      m_symbols(&proc.getSymbols()) /// @bug good style??
{}


/// @details
/// 
SymbolTable&
Context::getSymbols(void)
{
    return this->m_symbols;
}


/// @details
/// 
const SymbolTable&
Context::getSymbols(void) const
{
    return this->m_symbols;
}


/// @details
/// 
Value
Element::run(const ArgumentList &args)
{ 
    throw std::runtime_error("is not callabled");
}




//..............................................................................
///////////////////////////////////////////////////////////////////// ObjectInfo

/// @details
/// 
ObjectInfo::ObjectInfo(Processor &proc, ObjectNode *node)
    : Element(proc),
      m_node(node)
{}


/// @details
/// 
String
ObjectInfo::str(void) const
{
    return "[OBJINFO]";
}


/// @details
/// 
String
ObjectInfo::name(void) const
{
    return ARGON_UNNAMED_ELEMENT;
}

/// @details
/// 
String
ObjectInfo::type(void) const
{
    return "OBJECT";
}


/// @details
/// 
SourceInfo
ObjectInfo::getSourceInfo(void) const
{
    return this->m_node->getSourceInfo();
}


/// @details
/// 
Object*
ObjectInfo::newInstance(Object::mode mode)
{
    switch(mode)
    {
    case Object::READ_MODE:
        return new SourceTable(this->proc(), this->m_node);
    case Object::ADD_MODE:
        return new DestTable(this->proc(), this->m_node);
    default:
        throw std::runtime_error("object mode not handled");
    }
}




//..............................................................................
/////////////////////////////////////////////////////////////////// ValueElement

/// @details
/// 
ValueElement::ValueElement(Processor &proc, const Value &val)
    : Element(proc),
      m_value(val)
{
}


/// @details
/// 
String
ValueElement::str(void) const
{
    String s;
    //s.append(this->id().str());
    s.append("Value: ");
    //s.append(this->getValue().data().asStr());
    s.append(this->m_value.data().asStr());
    
    return s;
}


/// @details
/// 
SourceInfo
ValueElement::getSourceInfo(void) const
{
    return SourceInfo(); /// @bug fixme
    //return this->m_node->getSourceInfo();
}


/// @details
/// 
String
ValueElement::name(void) const
{
    return this->m_value.data().asStr();
}


/// @details
/// 
String
ValueElement::type(void) const
{
    return "VALUE";
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
