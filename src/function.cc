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


//..............................................................................
///////////////////////////////////////////////////////////////// ProcTreeWalker
///
/// @since 0.0.1
/// @brief Processor initial tree walker
class BlockVisitor : public Visitor
{
public:
    BlockVisitor(Processor &proc, Context &ctx);

    virtual void visit(VarNode *node);
    //virtual void visit(ReturnNode *node);


protected:
    inline Processor& proc(void) { return m_proc; }
    Processor &m_proc;
    Context   &m_context;
};


BlockVisitor::BlockVisitor(Processor &proc, Context &ctx)
	: Visitor(ignore_none),
		m_proc(proc),
		m_context(ctx)
{
}


void
BlockVisitor::visit(VarNode *node)
{
    ArgumentsNode *argsNode = find_node<ArgumentsNode>(node);
    assert(argsNode);
    assert(argsNode->getChilds().size() == 1);

    Node *data = argsNode->getChilds()[0];
    /// @bug Supports only LiteralNode for initialization

    ValueElement *elem = new ValueElement(this->proc(), String(node_cast<LiteralNode>(data)->data()));
    this->m_proc.stackPush(elem); /// @bug check stack scope in functsion
    this->m_context.getSymbols().add(node->data(), Ref(elem));
}


//..............................................................................
/////////////////////////////////////////////////////////////////////// Function

/// @details
/// 
Function::Function(Processor &proc, FunctionNode *node, const ArgumentList &args)
    : Context(proc, args),
      m_node(node)
{}




Value
Function::run(void)
{
    ARGON_DPRINT(ARGON_MOD_PROC, "Running function " << this->id());

    assert(this->m_node);

    safe_ptr<ArgumentsSpecNode> argsSpecNode = find_node<ArgumentsSpecNode>(this->m_node);

    ARGON_ICERR_CTX(argsSpecNode.get() != 0, *this,
                "no argument specification");


    ARGON_ICERR_CTX(argsSpecNode->getChilds().size() == this->getCallArgs().size(), *this,
                "Argument count mismatch");

    ArgumentList::const_iterator i = this->getCallArgs().begin();
    foreach_node(argsSpecNode->getChilds(),
                 Arg2SymVisitor(this->proc(), this->getSymbols(), i, this->getCallArgs().end()), 1);


    // just for development tests
    this->resolve<ValueElement>(Identifier("foo"));

    CompoundNode *n = find_node<CompoundNode>(this->m_node);
    assert(n);

    ARGON_SCOPED_STACKFRAME(this->proc());

    apply_visitor(n->getChilds(), BlockVisitor(this->proc(), *this));

    this->resolve<ValueElement>(Identifier("x"));

    return db::Variant(String("func-null-value"));
}




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
    return SourceInfo();
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



ARGON_NAMESPACE_END


//
// Local Variables:
// mode: C++
// c-file-style: "bsd"
// c-basic-offset: 4
// indent-tabs-mode: nil
// End:
//
