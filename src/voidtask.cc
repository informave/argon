//
// voidtask.cc - VOID TASK (definition)
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
/// @brief VOID TASK (definition)
/// @author Daniel Vogelbacher
/// @since 0.1

#include "visitors.hh"
#include "argon/dtsengine.hh"
#include "argon/exceptions.hh"
#include "argon/ast.hh"
#include "debug.hh"
#include "visitors.hh"

#include <iostream>
#include <sstream>
#include <list>

ARGON_NAMESPACE_BEGIN



//..............................................................................
/////////////////////////////////////////////////////////////////////// VoidTask

/// @details
/// 
VoidTask::VoidTask(Processor &proc, TaskNode *node, const ArgumentList &args)
    : Task(proc, node, args)
{}


/// @details
///
/*
Value
VoidTask::resolveColumn(const Column &col)
{
    throw std::runtime_error("resolve() not allowed on void tasks");
}
*/


/// @details
/// 
Object*
VoidTask::getMainObject(void)
{
    ARGON_ICERR_CTX(false, *this,
                "A VOID task does not contains a main object.");
}


/// @details
/// 
Object*
VoidTask::getResultObject(void) 
{ 
    ARGON_ICERR_CTX(false, *this,
                "A VOID task does not contains a result object.");
}


/// @details
/// 
Object*
VoidTask::getDestObject(void)
{
    ARGON_ICERR_CTX(false, *this,
                "A VOID task does not contains a destination object.");
}


/// @details
/// 
Value
VoidTask::run(void)
{
    ARGON_SCOPED_STACKFRAME(this->proc());

    Task::run();

    // Executes all init-instructions
    foreach_node( this->m_init_nodes, TaskChildVisitor(this->proc(), *this), 1);
    {
        // Executes all before instructions
        foreach_node( this->m_before_nodes, TaskChildVisitor(this->proc(), *this), 1);
        // Executes all rules instructions
        foreach_node( this->m_rules_nodes, TaskChildVisitor(this->proc(), *this), 1);
        // Executes all after instructions
        foreach_node( this->m_after_nodes, TaskChildVisitor(this->proc(), *this), 1);
    }
    // Executes all finalize-instructions
    foreach_node( this->m_final_nodes, TaskChildVisitor(this->proc(), *this), 1);


    return Value();
}




/// @details
/// 
Value
VoidTask::_value(void) const
{
    return this->_name();
}

/// @details
/// 
String
VoidTask::_string(void) const
{
    return this->_value().data().asStr();
}

/// @details
/// 
String
VoidTask::_name(void) const
{
    return this->id().str() + String(" (task:void)");
}

/// @details
/// 
String
VoidTask::_type(void) const
{
    return "VOIDTASK";
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
