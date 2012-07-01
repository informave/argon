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

#include "argon/rtti.hh"

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
    ARGON_ICERR_CTX(node->getChilds().size() == 2, m_context,
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
        catch(...) /// @bug handle conversion error for math expressions
        {
            m_value.data() = val1.data().asStr() + val2.data().asStr();
        }
        break;
    case ExprNode::concat_expr:
        try
        {
            m_value.data() = val1.data().asStr() + val2.data().asStr();
            break;
        }
        catch(informave::db::ex::null_value &err)
        {
            m_value.data().set<String>( (val1.data().isnull() ? "<null>" : val1.data().asStr()) );
            m_value.data().set<String>( m_value.data().asStr() + (val2.data().isnull() ? "<null>" : val2.data().asStr()) );
            break;
        }
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



    // // create new function in current context
    // std::auto_ptr<Function> fun( m_proc.createFunction(id) );

    // m_value.data() = m_proc.call(fun.get(), al).data();


    // resolve<> better?


    Element *elem = 0;
    
    elem = this->m_proc.getTypes().find<FunctionType>(id)->newInstance(al);

    assert(elem);

    ARGON_SCOPED_STACKPUSH(this->m_proc, elem);


    {
        /// @bug just call m_proc.call(<function-id>, args)
        // m_value.data() = this->m_proc.call(id, al).data();
        m_value.data() = m_proc.call(*elem).data();
    }

    //Function *f = this->m_proc.getSymbols().find<Function>(id);


    //m_value.data() = m_proc.call(f, al).data();

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
        Value val = this->m_context.getMainObject()->getColumn(Column(node));
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
EvalExprVisitor::visit(ResColumnNode *node)
{
    try
    {
        Value val = this->m_context.getResultObject()->getColumn(Column(node));
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
EvalExprVisitor::visit(ResColumnNumNode *node)
{
    try
    {
	//ResolveColumn ist unsinn, das muss mit context.getMainObject und ResultObject() gehen.
        Value val = this->m_context.getResultObject()->getColumn(Column(node));
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
EvalExprVisitor::visit(ResIdNode *node)
{
    try
    {
        //Value val = this->m_context.getMainObject()->lastInsertRowId();
        Value val = this->m_context.getDestObject()->lastInsertRowId();
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
        Value val = this->m_context.getMainObject()->getColumn(Column(node));
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
ArgumentsVisitor::visit(IdNode *node)
{
    Identifier id = node->data();

    Element *elem = m_context.resolve<Element>(id);

    m_list.push_back(Ref(elem));

//    m_value.data() = elem->_value().data();
/*
    Value val;
    EvalExprVisitor eval(this->m_proc, this->m_context, val);
    eval(node);
    m_list.push_back(val);
*/
}


/// @details
/// 
void
ArgumentsVisitor::fallback_action(Node *node)
{
    Value val;
    EvalExprVisitor eval(this->m_proc, this->m_context, val);
    eval(node);
    ValueElement *elem = new ValueElement(m_proc, val);
    this->m_proc.stackPush(elem);
    m_list.push_back(Ref(elem));
}




//..............................................................................
///////////////////////////////////////////////////////////////// Arg2SymVisitor

/// @details
/// 
Arg2SymVisitor::Arg2SymVisitor(Processor &proc, SymbolTable &symboltable, 
                               ArgumentList::const_iterator &i, ArgumentList::const_iterator end)
    : Visitor(Visitor::ignore_none),
      m_proc(proc),
    m_symboltable(symboltable),
//m_context(context),
    m_arg_iterator(i),
    m_end_iterator(end)
{}


/// @details
/// 
void
Arg2SymVisitor::visit(IdNode *node)
{
    Identifier argumentId = node->data();

    //m_context.getSymbols().add(argumentId, *m_arg_iterator);
    this->m_symboltable.add(argumentId, *m_arg_iterator);

/*
    Element *tmp = m_context.getSymbols().addPtr( new ValueElement(m_proc, *m_arg_iterator));
    m_context.getSymbols().add(node->data(),
                               tmp);
*/
    ++m_arg_iterator;
}





//..............................................................................
/////////////////////////////////////////////////////////////// ResColumnVisitor

/// @details
/// 
ResColumnVisitor::ResColumnVisitor(Processor &proc, Context &context, ColumnList &list)
    : Visitor(Visitor::ignore_other),
      m_proc(proc),
      m_context(context),
      m_list(list)
{}


/// @details
/// 
void
ResColumnVisitor::visit(ResColumnNode *node)
{
    m_list.insert(Column(node));
}


/// @details
/// 
void
ResColumnVisitor::visit(ResColumnNumNode *node)
{
    m_list.insert(Column(node));
}


//..............................................................................
////////////////////////////////////////////////////////////// DeepColumnVisitor

/// @details
/// 
DeepColumnVisitor::DeepColumnVisitor(Processor &proc, Context &context, ColumnList &list)
    : Visitor(Visitor::ignore_other),
      m_proc(proc),
      m_context(context),
      m_list(list)
{}


/// @details
///     
void
DeepColumnVisitor::visit(ColumnNode *node)
{
    m_list.insert(Column(node));
}


/// @details
/// 
void
DeepColumnVisitor::visit(ColumnNumNode *node)
{
    m_list.insert(Column(node));
}


/// @details
/// 
void
DeepColumnVisitor::visit(ColumnAssignNode *node)
{
    ARGON_ICERR_CTX(false, this->m_context,
                    "Column assign expression is not allowed in DeepColumnVisitor");
}


//..............................................................................
////////////////////////////////////////////////////////////////// ColumnVisitor

/// @details
/// 
ColumnVisitor::ColumnVisitor(Processor &proc, Context &context, ColumnList &left_list, ColumnList &right_list)
    : Visitor(Visitor::ignore_none),
      m_proc(proc),
      m_context(context),
      m_left_list(left_list),
      m_right_list(right_list)
{}


/// @details
/// 
void
ColumnVisitor::visit(ColumnAssignNode *node)
{
    ARGON_ICERR_CTX(node->getChilds().size() == 2, this->m_context,
                    "Column assign expression must contain exactly two childs.");
        
    foreach_node( node->getChilds()[0], DeepColumnVisitor(this->m_proc, this->m_context, m_left_list) );
    foreach_node( node->getChilds()[1], DeepColumnVisitor(this->m_proc, this->m_context, m_right_list) );
}


/// @details
/// 
void
ColumnVisitor::fallback_action(Node *node)
{
    foreach_node( node, DeepColumnVisitor(this->m_proc, this->m_context, m_right_list) );
}



//..............................................................................
///////////////////////////////////////////////////////////// TemplateArgVisitor

/// @details
/// 
TemplateArgVisitor::TemplateArgVisitor(Processor &proc, Context &context, ArgumentList &list)
    : Visitor(Visitor::ignore_none),
      m_proc(proc),
      m_context(context),
      m_list(list)
{}


/// @details
/// 
void
TemplateArgVisitor::visit(IdNode *node)
{
    // no args
}


/// @details
/// 
void
TemplateArgVisitor::visit(IdCallNode *node)
{
    ARGON_ICERR_CTX(node->getChilds().size() >= 1, this->m_context,
                    "IdCallNode does not contains any subnodes");

    ArgumentsNode *argsnode = node_cast<ArgumentsNode>(node->getChilds().at(1));

    foreach_node(argsnode->getChilds(), ArgumentsVisitor(this->m_proc, m_context, this->m_list), 1);
}



//..............................................................................
//////////////////////////////////////////////////////////// LValueColumnVisitor

/// @details
/// 
LValueColumnVisitor::LValueColumnVisitor(Processor &proc, Context &context, Column &c)
    : Visitor(Visitor::ignore_none),
      m_proc(proc),
      m_context(context),
      m_column(c)
{}

void
LValueColumnVisitor::visit(ColumnNode *node)
{
    m_column = Column(node);
    //m_list.insert(Column(node));
}


void
LValueColumnVisitor::visit(ColumnNumNode *node)
{
    m_column = Column(node);
    //m_list.insert(Column(node));
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
