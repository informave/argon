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
///////////////////////////////////////////////////////////////// Pass1Visitor

/// @details
/// 
Pass1Visitor::Pass1Visitor(Processor &proc)
    : Visitor(ignore_none),
      m_proc(proc)
{}


/// @details
/// 
void
Pass1Visitor::visit(ConnNode *node)
{
    // Nothing to do
}


/// @details
/// 
void
Pass1Visitor::visit(DeclNode *node)
{
    Identifier x = find_node<IdNode>(node)->data();

    if(Identifier("table") == x)
        this->proc().getTypes().add(new CustomTableType(this->proc(), node->data(), node));
    else if(Identifier("sql") == x)
        this->proc().getTypes().add(new CustomSqlType(this->proc(), node->data(), node));
    else if(Identifier("procedure") == x)
    {
        FIXME();
    }
    else if(Identifier("view") == x)
    {
        FIXME();
    }
    else
    {
        FIXME();
    }
        
}



/// @details
/// 
void
Pass1Visitor::visit(ExceptionDeclNode *node)
{
    //Identifier x = find_node<IdNode>(node)->data();

    this->proc().getTypes().add(new CustomExceptionType(this->proc(), node->data(), node));
}




/// @details
/// 
void
Pass1Visitor::visit(VarNode *node)
{
    // nothing to do
}


/// @details
/// 
void
Pass1Visitor::visit(SequenceNode *node)
{
    // nothing to do
}



/// @details
/// 
void
Pass1Visitor::visit(TaskNode *node)
{
    this->proc().getTypes().add(new TaskType(this->proc(), node->id, node));
}


/// @details
/// 
void
Pass1Visitor::visit(FunctionNode *node)
{
    this->proc().getTypes().add(new CustomFunctionType(this->proc(), node->data(), node));
}


/// @details
/// 
void
Pass1Visitor::visit(LogNode *node)
{
    //std::cout << "visit log node" << std::endl;
}


/// @details
/// 
void
Pass1Visitor::visit(IdNode *node)
{
    //std::cout << "visit id node: " << node->str() << std::endl;
}


/// @details
/// 
void
Pass1Visitor::visit(LiteralNode *node)
{
    //std::cout << "visit literal node: " << node->str() << std::endl;
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
