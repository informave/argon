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


ARGON_NAMESPACE_BEGIN

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
    
    ValueElement* elem = m_context.getSymbols().find<ValueElement>(id);
    
    m_list.push_back(Value(elem->getValue()));
}


/// @details
/// 
void
ArgumentsVisitor::visit(LiteralNode *node)
{
    m_list.push_back(Value(node->data()));
}


/// @details
/// 
void
ArgumentsVisitor::visit(NumberNode *node)
{
    m_list.push_back(Value(node->data()));
}



void
ArgumentsVisitor::visit(ColumnNumNode *node)
{
    try
    {
        Value val = this->m_context.resolve(Column(node));
        m_list.push_back(val);
    }
    catch(RuntimeError &err)
    {
        err.addSourceInfo(node->getSourceInfo());
        throw;
    }
}


void
ArgumentsVisitor::visit(ColumnNode *node)
{
    try
    {
        Value val = this->m_context.resolve(Column(node));
        m_list.push_back(val);
    }
    catch(RuntimeError &err)
    {
        err.addSourceInfo(node->getSourceInfo());
        throw;
    }
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
