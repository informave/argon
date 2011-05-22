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
#include "debug.hh"
#include "visitors.hh"

#include <iostream>
#include <sstream>
#include <list>
#include <set>

ARGON_NAMESPACE_BEGIN



typedef TemplateVisitor       StoreTemplateVisitor;
typedef TemplateArgVisitor    StoreTemplateArgVisitor;



//..............................................................................
////////////////////////////////////////////////////////////////////// StoreTask

/// @details
/// 
StoreTask::StoreTask(Processor &proc, TaskNode *node)
    : Task(proc, node),
      m_destobject()
{}


/// @details
/// 
Object*
StoreTask::getMainObject(void) 
{                 
    return this->m_destobject.get(); 
}


/// @details
/// 
Object*
StoreTask::getResultObject(void) 
{ 
    return this->m_destobject.get();
}


/// @details
/// 
Object*
StoreTask::getDestObject(void)
{
    return this->m_destobject.get();
}


/// @details
/// 
Value
StoreTask::run(const ArgumentList &args)
{
    ARGON_DPRINT(ARGON_MOD_PROC, "Running task " << this->id());

    Task::run(args);

    // Get template arguments
    safe_ptr<TmplArgumentsNode> tmplArgNode = find_node<TmplArgumentsNode>(this->m_node);

    ARGON_ICERR_CTX(!!tmplArgNode, *this,
                "TASK does not have any template arguments");

    ARGON_ICERR_CTX(tmplArgNode->getChilds().size() == 1, *this,
                "wrong template argument count");
        

    ObjectInfo *destInfoObj = 0;


    // Create destination object
    Node *destArgNode = tmplArgNode->getChilds().at(0);
    foreach_node(destArgNode, StoreTemplateVisitor(this->proc(), *this, destInfoObj), 1);
    ARGON_ICERR_CTX(destInfoObj != 0, *this,
                "destination information is not valid");

    this->m_destobject.reset(destInfoObj->newInstance(Object::ADD_MODE));
    ARGON_ICERR_CTX(this->m_destobject.get() != 0, *this,
                "Main object allocation failed");

    // Handle destination object arguments
    ArgumentList destArgs;
    foreach_node(destArgNode, StoreTemplateArgVisitor(this->proc(), *this, destArgs), 1);



    // Get a list of the left and right columns
    ColumnList lclist, rclist;
    foreach_node( this->m_node->getChilds(), ColumnVisitor(this->proc(), *this, lclist, rclist), 1);
    this->m_destobject->setColumnList(lclist);

    assert(rclist.size() == 0); // STORE tasks can't contain any column identifiers on right side.

    
    // Get result columns
    ColumnList reslist;
    foreach_node( this->m_node->getChilds(), ResColumnVisitor(this->proc(), *this, reslist));
    this->m_destobject->setResultList(reslist);

    
    // Call object to setup initial environment
    // This prepares the SQL statement etc.
    this->proc().call(this->getMainObject(), destArgs);

   // IMPORTANT: destArgs may be used as values, Too!!

    {
        // Executes all pre-instructions
        foreach_node( this->m_pre_nodes, TaskChildVisitor(this->proc(), *this), 1);
        // Executes all column-assign instructions
        foreach_node( this->m_colassign_nodes, TaskChildVisitor(this->proc(), *this), 1);
        // Executes the destination object (put data to object)
        this->getDestObject()->execute();
        // Executes all post-instructions
        foreach_node( this->m_post_nodes, TaskChildVisitor(this->proc(), *this), 1);
    }

    
    this->m_destobject.reset(0); // workaround
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
