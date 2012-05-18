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
#include <functional>
#include <sstream>

ARGON_NAMESPACE_BEGIN





    ScopedStackFrame::ScopedStackFrame(Processor &proc)
        : m_stack(proc.m_stack),
          m_pos()
    {
        m_pos = m_stack.begin();
    }

    ScopedStackFrame::~ScopedStackFrame(void)
    {
        for(Processor::stack_type::iterator i = m_stack.begin();
            (!m_stack.empty()) && i != m_pos;
            ++i)
        {
            delete (*i);
            m_stack.erase(i);
        }
    }



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
Object::Object(Processor &proc, DeclNode *node, const ArgumentList &args)
    : Context(proc, args),
      m_column_mappings(),
      m_node(node)
{}


/// @details
/// This runs some code that is required by all objects like Arguments->Symboltable
Value
Object::run(void)
{
    if(this->m_node)
    {
    // Load OBJECTs arguments to local symbol table
    this->getSymbols().reset();
    safe_ptr<ArgumentsSpecNode> argsSpecNode = find_node<ArgumentsSpecNode>(this->m_node);

    ARGON_ICERR_CTX(argsSpecNode.get() != 0, *this,
                "no argument specification");
    
    ARGON_ICERR_CTX(argsSpecNode->getChilds().size() == this->getCallArgs().size(), *this,
                "Argument count mismatch");
    
    ArgumentList::const_iterator i = this->getCallArgs().begin();
    foreach_node(argsSpecNode->getChilds(),
                 Arg2SymVisitor(this->proc(), this->getSymbols(), i, this->getCallArgs().end()), 1);
    }
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



/// @details
/// 
int
Object::getBindPosition(const Column &col)
{
    std::map<Column, int>::iterator i = this->m_column_mappings.find(col);
    if(i == this->m_column_mappings.end())
        throw std::runtime_error(String("unknown column: ") + col.getName());
    else
        return i->second;
}


/// @details
/// 
void
Object::addBindPosition(const Column &col, int pos)
{
    this->m_column_mappings[col] = pos;
}





Ref::Ref(Element *elem) : m_element(elem)
{
    assert(elem);
    this->m_element->registerRef(this);
}

Ref::Ref(const Ref &orig) : m_element(orig.m_element)
{
    assert(m_element);
    this->m_element->registerRef(this);
}

Ref::~Ref(void)
{
    if(!this->dead())
      this->m_element->unregisterRef(this);
}


Element*
Ref::operator->(void)
{
    ARGON_ICERR(this->m_element, "Dead reference, the element did not exist!");
    return this->m_element;
}

const Element*
Ref::operator->(void) const
{
    ARGON_ICERR(this->m_element, "Dead reference, the element did not exist!");
    return this->m_element;
}


Element*
Ref::operator&(void)
{
    ARGON_ICERR(this->m_element, "Dead reference, the element did not exist!");
    return this->m_element;
}




//..............................................................................
//////////////////////////////////////////////////////////////////////// Element

/// @details
/// 
Element::Element(Processor &proc)
    : m_references(),
      m_proc(proc)
{}


Element::~Element(void)
{
    if(this->m_references.size())
    {
    std::cerr << "Element '" << typeid(this).name() << "' has " << this->m_references.size()
       << " active references, now marked as dead!" << std::endl;


    std::for_each(this->m_references.begin(), this->m_references.end(), std::mem_fun(&Ref::mark_dead));
    }

    /// @bug better error message
    //ARGON_ICERR(this->m_references.empty(), ss.str());
}


void
Element::registerRef(Ref *p)
{
    this->m_references.push_back(p);
}



void
Element::unregisterRef(Ref *p)
{
    this->m_references.remove(p);
}




//..............................................................................
//////////////////////////////////////////////////////////////////////// Context

/// @details
/// 
Context::Context(Processor &proc, const ArgumentList &args)
    : Element(proc),
      m_symbols(&proc.getSymbols()), /// @bug good style??
      m_args(args)
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
Element::run()
{ 
    throw std::runtime_error("is not callabled");
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
