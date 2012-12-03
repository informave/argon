//
// processor.cc - Processor (definition)
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
/// @brief Processor (definition)
/// @author Daniel Vogelbacher
/// @since 0.1


#include "argon/dtsengine.hh"
#include "argon/exceptions.hh"
#include "argon/semantic.hh"
#include "argon/rtti.hh"
#include "helpers.hh"
#include "debug.hh"
#include "visitors.hh"

#include <iostream>
#include <stack>

ARGON_NAMESPACE_BEGIN


//..............................................................................
/////////////////////////////////////////////////////////////////// Pass2Visitor

/// @details
/// 
Pass2Visitor::Pass2Visitor(Processor &proc, Context &context)
    : Visitor(ignore_none),
      m_proc(proc),
      m_context(context)
{}


void
Pass2Visitor::visit(DeclNode *node)
{
}


void
Pass2Visitor::visit(ExceptionDeclNode *node)
{
}



/// @details
/// 
void
Pass2Visitor::visit(VarNode *node)
{
    ArgumentsNode *argsNode = find_node<ArgumentsNode>(node);
    assert(argsNode);
    assert(argsNode->getChilds().size() == 1);

    Node *data = argsNode->getChilds()[0];

    Value value;
    apply_visitor(data, EvalExprVisitor(proc(), context(), value));

    ValueElement *elem = 0;
    elem = new ValueElement(this->proc(), value);

    this->proc().stackPush(elem);
    this->proc().getGlobalContext().getSymbols().add(node->data(), Ref(elem));
}


/// @details
/// 
void
Pass2Visitor::visit(SequenceNode *node)
{
    ArgumentsNode *argsNode = find_node<ArgumentsNode>(node);
    assert(argsNode);
    assert(argsNode->getChilds().size() == 1 || argsNode->getChilds().size() == 2);

    Node *data = argsNode->getChilds()[0];

    Value value, inc(int(1));
    apply_visitor(data, EvalExprVisitor(proc(), context(), value));

    if(argsNode->getChilds().size() == 2)
    {
    	Node *incby = argsNode->getChilds()[1];
    	apply_visitor(incby, EvalExprVisitor(proc(), context(), inc));
    }

    Sequence *elem = 0;
    elem = new Sequence(this->proc(), value, inc);

    this->proc().stackPush(elem);
    this->proc().getGlobalContext().getSymbols().add(node->data(), Ref(elem));
}



/// @details
/// 
void
Pass2Visitor::visit(ConnNode *node)
{

    /// getType("connection").newInstance().construct(node);

    /// @bug use ConnectionType::newInstance()
    Connection *elem = new Connection(this->proc(), node, this->m_proc.getConnections());

	this->m_proc.stackPush(elem);

    //this->proc().addtoHeap(elem);

    this->proc().getGlobalContext().getSymbols().add(node->data(), Ref(elem));

/*
    Connection *elem = this->proc().getSymbols().addPtr(
        new Connection(this->proc(), node, this->m_proc.getConnections()) );
    this->proc().getSymbols().add(node->data(), elem);
*/

    if(! elem->getDbc().isConnected())
    {
        throw std::runtime_error("dbc is not connected");
    }

    this->m_proc.getGlobalContext().getSymbols().find<Element>(node->data());

    //this->m_proc.getSymbol<Task>(id).exec(argumentlist);
}




/// @details
/// 
void
Pass2Visitor::visit(TaskNode *node)
{
    // nothing to do
}


/// @details
/// 
void
Pass2Visitor::visit(FunctionNode *node)
{
    // nothing to do
}


/// @details
/// 
void
Pass2Visitor::visit(LogNode *node)
{
    //std::cout << "visit log node" << std::endl;
}


/// @details
/// 
void
Pass2Visitor::visit(IdNode *node)
{
    //std::cout << "visit id node: " << node->str() << std::endl;
}


/// @details
/// 
void
Pass2Visitor::visit(LiteralNode *node)
{
    //std::cout << "visit literal node: " << node->str() << std::endl;
}


/// @details
/// 
/*
void
Pass2Visitor::visit(ParseTree *node)
{
    //std::cout << "Visit tree" << std::endl;
}
*/






ARGON_NAMESPACE_END



//
// Local Variables:
// mode: C++
// c-file-style: "bsd"
// c-basic-offset: 4
// indent-tabs-mode: nil
// End:
//
