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
/////////////////////////////////////////////////////////////////////// Function

/// @details
/// 
Function::Function(Processor &proc)
    : Context(proc)
{}




/// @details
/// 
String
Function::str(void) const
{
    return "[FUNCTION]";
}


/// @details
/// 
String
Function::name(void) const
{
    return ARGON_UNNAMED_ELEMENT;
}

/// @details
/// 
String
Function::type(void) const
{
    return "Function";
}


/// @details
/// 
SourceInfo
Function::getSourceInfo(void) const
{
    /// @bug implement me
    throw std::runtime_error("Function::getSourceInfo");
    //return this->m_node->getSourceInfo();
}



/// @details
/// 
/*
Value
Function::resolveColumn(const Column &col)
{
    throw std::runtime_error("resolve() not allowed on functions");
}
*/


/// @details
/// 
Object*
Function::getMainObject(void)
{
    ARGON_ICERR_CTX(false, *this,
                "A function does not contains a result object.");
}


/// @details
/// 
Object*
Function::getResultObject(void) 
{ 
    ARGON_ICERR_CTX(false, *this,
                "A function does not contains a result object.");
}


/// @details
/// 
Object*
Function::getDestObject(void)
{
    ARGON_ICERR_CTX(false, *this,
                "A function does not contains a destination object.");
}



/// @details
/// 
Value
Function::_value(void) const
{
    return String("FUNCTION"); /// @todo return function name
}

/// @details
/// 
String
Function::_string(void) const
{
    return this->_value().data().asStr();
}

/// @details
/// 
String
Function::_name(void) const
{
    return String("unknown-id") + String(" (function)");
    //return this->id().str() + String(" (function)");
}

/// @details
/// 
String
Function::_type(void) const
{
    return "FUNCTION";
}



//..............................................................................
///////////////////////////////////////////////////////////////////////// Object

/// @details
/// 
Object::Object(Processor &proc, ObjectNode *node)
    : Context(proc),
      m_column_mappings(),
      m_node(node)
{}


/// @details
/// This runs some code that is required by all objects like Arguments->Symboltable
Value
Object::run(const ArgumentList &args)
{
    // Load OBJECTs arguments to local symbol table
    this->getSymbols().reset();
    safe_ptr<ArgumentsSpecNode> argsSpecNode = find_node<ArgumentsSpecNode>(this->m_node);

    ARGON_ICERR_CTX(argsSpecNode.get() != 0, *this,
                "no argument specification");
    
    ARGON_ICERR_CTX(argsSpecNode->getChilds().size() == args.size(), *this,
                "Argument count mismatch");
    
    ArgumentList::const_iterator i = args.begin();
    foreach_node(argsSpecNode->getChilds(), Arg2SymVisitor(this->proc(), *this, i), 1);
    
    return Value();
}



/// @details
/// 
/*
Value
Object::resolveColumn(const Column &col)
{
    throw std::runtime_error("resolve() not allowed on objects");
}
*/


/// @details
 ///
Object*
Object::getMainObject(void)
{
    ARGON_ICERR_CTX(false, *this,
                "An object does not contains a main object.");
}


/// @details
/// 
Object*
Object::getResultObject(void) 
{ 
    ARGON_ICERR_CTX(false, *this,
                "An object does not contains a result object.");
}


/// @details
/// 
Object*
Object::getDestObject(void)
{
    ARGON_ICERR_CTX(false, *this,
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



//..............................................................................
////////////////////////////////////////////////////////////////// ObjectCreator
///
/// @since 0.0.1
/// @brief Object Creator
struct ObjectCreator : public Visitor
{
public:
    ObjectCreator(Processor &proc, Object::mode mode, Object *&out)
        : Visitor(Visitor::ignore_none),
          m_proc(proc),
          m_mode(mode),
          m_object(out)
    {}


    virtual void visit(TableNode *node)
    {
        this->m_object = Table::newInstance(m_proc, node, m_mode);
    }


private:
    Processor &m_proc;
    Object::mode m_mode;
    Object *&m_object;
};



/// @details
/// 
Object*
ObjectInfo::newInstance(Object::mode mode)
{
    Object *tmp = 0;
    apply_visitor(this->m_node, ObjectCreator(this->proc(), mode, tmp));

    ARGON_ICERR(!!tmp, "ObjectInfo was unable to create a new instance.");
    return tmp;
}


/// @details
/// 
Value
ObjectInfo::_value(void) const
{
    return this->_name();
}

/// @details
/// 
String
ObjectInfo::_string(void) const
{
    return this->_value().data().asStr();
}

/// @details
/// 
String
ObjectInfo::_name(void) const
{
    return this->id().str() + String(" (object)");
}

/// @details
/// 
String
ObjectInfo::_type(void) const
{
    return "OBJINFO";
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
/// ValueElement does not contains any source information.
SourceInfo
ValueElement::getSourceInfo(void) const
{
    return SourceInfo();
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


/// @details
/// 
Value
ValueElement::_value(void) const
{
    return this->m_value;
}

/// @details
/// 
String
ValueElement::_string(void) const
{
    return this->_value().data().asStr();
}

/// @details
/// 
String
ValueElement::_name(void) const
{
    //return String("unknown-id") + String(" (function)");
    return "VALUE";
}

/// @details
/// 
String
ValueElement::_type(void) const
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
