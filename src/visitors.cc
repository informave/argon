//
// visitors.cc - Visitors (definition)
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
/// @brief Visitors (definition)
/// @author Daniel Vogelbacher
/// @since 0.1

#include "visitors.hh"

#include "builtin/functions.hh"

#include <stdexcept>

ARGON_NAMESPACE_BEGIN


//..............................................................................
//////////////////////////////////////////////////////////////// EvalExprVisitor

/// @details
/// 
EvalExprVisitor::EvalExprVisitor(Processor &proc, Context &context, Value &value)
    : Visitor(ignore_none),
      m_proc(proc),
      m_context(context),
      m_value(value)
{}

    
/// @details
/// 
void
EvalExprVisitor::visit(ExprNode *node)
{
    ARGON_ICERR(node->getChilds().size() == 2, m_context,
                "ExprNode must have exactly two childs.");

    Value val1, val2;
    {
        EvalExprVisitor eval(this->m_proc, this->m_context, val1);
        eval(node->getChilds()[0]);
    }
    {
        EvalExprVisitor eval(this->m_proc, this->m_context, val2);
        eval(node->getChilds()[1]);
    }

    
    switch(node->data())
    {
    case ExprNode::plus_expr:
        try
        {
            m_value.data() = val1.data().asInt() + val2.data().asInt();
        }
        catch(...) /// @bug only for tests!
        {
            m_value.data() = val1.data().asStr() + val2.data().asStr();
        }
        break;
    case ExprNode::concat_expr:
        m_value.data() = val1.data().asStr() + val2.data().asStr();
        break;
    case ExprNode::minus_expr:
        m_value.data() = val1.data().asInt() - val2.data().asInt();
        break;
    case ExprNode::mul_expr:
        m_value.data() = val1.data().asInt() * val2.data().asInt();
        break;
    case ExprNode::div_expr:
        if(val2.data().asInt() == 0)
            throw std::runtime_error("zero division error");
        m_value.data() = val1.data().asInt() / val2.data().asInt();
        break;
    }
}


/// @details
/// 
void
EvalExprVisitor::visit(NumberNode *node)
{
    m_value.data() = node->data();
}



/// @details
/// 
void
EvalExprVisitor::visit(IdNode *node)
{
    Identifier id = node->data();

    Element *elem = m_context.resolve<Element>(id);
    m_value.data() = elem->_value().data();
}



/// @details
/// 
void
EvalExprVisitor::visit(FuncCallNode *node)
{
    ArgumentList al;
    Identifier id = node->data();
    ArgumentsNode *argsnode = find_node<ArgumentsNode>(node);
    assert(argsnode);

    // Fill argument list with the result of each argument node
    foreach_node(argsnode->getChilds(), ArgumentsVisitor(m_proc, m_context, al), 1);

    // create new function in current context
    std::auto_ptr<Function> fun( m_proc.createFunction(id) );

    m_value.data() = m_proc.call(fun.get(), al).data();
}


/// @details
/// 
void
EvalExprVisitor::visit(LiteralNode *node)
{
    m_value.data() = node->data();
}


/// @details
/// 
void
EvalExprVisitor::visit(ColumnNode *node)
{
    try
    {
        Value val = this->m_context.resolveColumn(Column(node));
        m_value.data() = val.data();
    }
    catch(RuntimeError &err)
    {
        err.addSourceInfo(node->getSourceInfo());
        throw;
    }


}


/// @details
/// 
void
EvalExprVisitor::visit(ColumnNumNode *node)
{
    try
    {
        Value val = this->m_context.resolveColumn(Column(node));
        m_value.data() = val.data();
    }
    catch(RuntimeError &err)
    {
        err.addSourceInfo(node->getSourceInfo());
        throw;
    }    
}



//..............................................................................
/////////////////////////////////////////////////////////////// ArgumentsVisitor

/// @details
/// 
ArgumentsVisitor::ArgumentsVisitor(Processor &proc, Context &context, ArgumentList &list)
    : Visitor(Visitor::ignore_none),
      m_proc(proc),
      m_context(context),
      m_list(list)
{}


/// @details
/// 
void
ArgumentsVisitor::fallback_action(Node *node)
{
    Value val;
    EvalExprVisitor eval(this->m_proc, this->m_context, val);
    eval(node);

    /// @bug Fix Expr visitor
    m_list.push_back(val);
}




//..............................................................................
///////////////////////////////////////////////////////////////// Arg2SymVisitor

/// @details
/// 
Arg2SymVisitor::Arg2SymVisitor(Processor &proc, Context &context, ArgumentList::const_iterator &i)
    : Visitor(Visitor::ignore_none),
      m_proc(proc),
      m_context(context),
      m_arg_iterator(i)
{}


/// @details
/// 
void
Arg2SymVisitor::visit(IdNode *node)
{
    Element *tmp = m_context.getSymbols().addPtr( new ValueElement(m_proc, *m_arg_iterator));
    m_context.getSymbols().add(node->data(),
                               tmp);
    ++m_arg_iterator;
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
