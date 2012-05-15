//
// storetask.cc - STORE TASK (definition)
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
/// @brief STORE TASK (definition)
/// @author Daniel Vogelbacher
/// @since 0.1

#include "argon/dtsengine.hh"
#include "argon/exceptions.hh"
#include "argon/ast.hh"
#include "argon/rtti.hh"
#include "debug.hh"
#include "visitors.hh"

#include <iostream>
#include <sstream>
#include <list>
#include <set>

ARGON_NAMESPACE_BEGIN



//typedef TemplateVisitor       StoreTemplateVisitor;
typedef TemplateArgVisitor    StoreTemplateArgVisitor;



//..............................................................................
////////////////////////////////////////////////////////////////////// StoreTask

/// @details
/// 
StoreTask::StoreTask(Processor &proc, TaskNode *node, const ArgumentList &args)
    : Task(proc, node, args),
      m_destobject()
{}


/// @details
/// 
Object*
StoreTask::getMainObject(void) 
{                 
    return this->m_destobject; 
}


/// @details
/// 
Object*
StoreTask::getResultObject(void) 
{ 
    return this->m_destobject;
}


/// @details
/// 
Object*
StoreTask::getDestObject(void)
{
    return this->m_destobject;
}


/// @details
/// 
Value
StoreTask::run(void)
{
    ARGON_DPRINT(ARGON_MOD_PROC, "Running task " << this->id());

    ARGON_SCOPED_STACKFRAME(this->proc());

    Task::run();

    // Get template arguments
    safe_ptr<TmplArgumentsNode> tmplArgNode = find_node<TmplArgumentsNode>(this->m_node);

    ARGON_ICERR_CTX(!!tmplArgNode, *this,
                "TASK does not have any template arguments");

    ARGON_ICERR_CTX(tmplArgNode->getChilds().size() == 1, *this,
                "wrong template argument count");
        


    // Create destination object
    IdCallNode *destArgNode = node_cast<IdCallNode>(tmplArgNode->getChilds().at(0));
    IdNode *destIdNode = find_node<IdNode>(destArgNode);
    ObjectType* destType = this->proc().getTypes().find<ObjectType>(destIdNode->data());

    // Handle destination object arguments
    ArgumentList destArgs;
    foreach_node(destArgNode, StoreTemplateArgVisitor(this->proc(), *this, destArgs), 1);

    ObjectSmartPtr set_destobj(&this->m_destobject, destType->newInstance(destArgs, Type::INSERT_MODE));

    //this->m_destobject.reset(destType->newInstance(Type::INSERT_MODE));
    
    ARGON_ICERR_CTX(this->m_destobject != 0, *this,
                "Destination object allocation failed");


    // Get a list of the left and right columns
    ColumnList lclist, rclist;
    foreach_node( this->m_rules_nodes, ColumnVisitor(this->proc(), *this, lclist, rclist), 1);
    this->m_destobject->setColumnList(lclist);

    assert(rclist.size() == 0); // STORE tasks can't contain any column identifiers on right side.

    
    // Get result columns
    ColumnList reslist;
    foreach_node( this->m_after_nodes, ResColumnVisitor(this->proc(), *this, reslist));
    foreach_node( this->m_final_nodes, ResColumnVisitor(this->proc(), *this, reslist));
    this->m_destobject->setResultList(reslist);

    
    // Call object to setup initial environment
    // This prepares the SQL statement etc.
    this->proc().call(this->getMainObject());

   // IMPORTANT: destArgs may be used as values, Too!!


    // Executes all init-instructions
    foreach_node( this->m_init_nodes, TaskChildVisitor(this->proc(), *this), 1);

    {
        // Executes all before instructions
        foreach_node( this->m_before_nodes, TaskChildVisitor(this->proc(), *this), 1);
        // Executes all rules instructions
        foreach_node( this->m_rules_nodes, TaskChildVisitor(this->proc(), *this), 1);

        // Executes the destination object (put data to object)
        this->getDestObject()->execute();

        // Executes all after instructions
        foreach_node( this->m_after_nodes, TaskChildVisitor(this->proc(), *this), 1);
    }

    // Executes all finalize-instructions
    foreach_node( this->m_final_nodes, TaskChildVisitor(this->proc(), *this), 1);
    

    //this->m_destobject.reset(0); // workaround
    return Value();
}


/// @details
/// 
/*
Value
StoreTask::resolveColumn(const Column &col)
{
    return Value(this->getMainObject()->getColumn(col));
}
*/


/// @details
/// 
Value
StoreTask::_value(void) const
{
    return this->_name();
}

/// @details
/// 
String
StoreTask::_string(void) const
{
    return this->_value().data().asStr();
}

/// @details
/// 
String
StoreTask::_name(void) const
{
    return this->id().str() + String(" (task:store)");
}

/// @details
/// 
String
StoreTask::_type(void) const
{
    return "STORETASK";
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
