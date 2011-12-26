//
// fetchtask.cc - FETCH TASK (definition)
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
/// @brief FETCH TASK (definition)
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

ARGON_NAMESPACE_BEGIN



typedef TemplateVisitor       FetchTemplateVisitor;
typedef TemplateArgVisitor    FetchTemplateArgVisitor;




//..............................................................................
////////////////////////////////////////////////////////////////////// FetchTask

/// @details
/// 
FetchTask::FetchTask(Processor &proc, TaskNode *node)
    : Task(proc, node),
      m_mainobject()
{}


/// @details
/// 
Object*
FetchTask::getMainObject(void) 
{ 
    return this->m_mainobject.get(); 
}


/// @details
/// 
Object*
FetchTask::getResultObject(void) 
{ 
    ARGON_ICERR_CTX(false, *this,
                "A FETCH task does not contains a result object.");
}


/// @details
/// 
Object*
FetchTask::getDestObject(void)
{
    ARGON_ICERR_CTX(false, *this,
                "A FETCH task does not contains a destination object.");
}


/// @details
/// 
Value
FetchTask::run(const ArgumentList &args)
{
    ARGON_DPRINT(ARGON_MOD_PROC, "Running task " << this->id());

    Task::run(args);

    // Get template arguments
    safe_ptr<TmplArgumentsNode> tmplArgNode = find_node<TmplArgumentsNode>(this->m_node);

    ARGON_ICERR_CTX(!!tmplArgNode, *this,
                "TASK does not have any template arguments");

    ARGON_ICERR_CTX(tmplArgNode->getChilds().size() == 1, *this,
                "wrong template argument count");
        

    ObjectInfo *sourceInfoObj = 0;


    // Create source object
    Node *sourceArgNode = tmplArgNode->getChilds().at(0);
    foreach_node(sourceArgNode, FetchTemplateVisitor(this->proc(), *this, sourceInfoObj), 1);
    ARGON_ICERR_CTX(sourceInfoObj != 0, *this,
                "source information is not valid");

    this->m_mainobject.reset(sourceInfoObj->newInstance(Object::READ_MODE));
    ARGON_ICERR_CTX(this->m_mainobject.get() != 0, *this,
                "Source object allocation failed");


    // Handle source object arguments
    ArgumentList sourceArgs;
    foreach_node(sourceArgNode, FetchTemplateArgVisitor(this->proc(), *this, sourceArgs), 1);


    // Get a list of the left and right columns
    ColumnList lclist, rclist;
    foreach_node( this->m_before_nodes, ColumnVisitor(this->proc(), *this, lclist, rclist), 1);
    foreach_node( this->m_rules_nodes, ColumnVisitor(this->proc(), *this, lclist, rclist), 1);
    foreach_node( this->m_after_nodes, ColumnVisitor(this->proc(), *this, lclist, rclist), 1);
    this->m_mainobject->setColumnList(rclist);

    assert(lclist.size() == 0); // FETCH tasks can't contain any column identifiers on left side.


    // Get result columns
    ColumnList reslist;
    foreach_node( this->m_after_nodes, ResColumnVisitor(this->proc(), *this, reslist));
    foreach_node( this->m_final_nodes, ResColumnVisitor(this->proc(), *this, reslist));
    
    assert(reslist.size() == 0); // FETCH tasks can't contain any result columns


    // Call object to setup initial environment
    // This prepares the SQL statement etc.
    this->proc().call(this->getMainObject(), sourceArgs);
    this->getMainObject()->execute();


    // Executes all init-instructions
    foreach_node( this->m_init_nodes, TaskChildVisitor(this->proc(), *this), 1);

    while(! this->m_mainobject->eof())
    {
        // Executes all before instructions
        foreach_node( this->m_before_nodes, TaskChildVisitor(this->proc(), *this), 1);

        // Executes all rules instructions
        foreach_node( this->m_rules_nodes, TaskChildVisitor(this->proc(), *this), 1);

        this->m_mainobject->next();


        // at this point, no after nodes should exists...
        assert(this->m_after_nodes.size() == 0);
        // Executes all after instructions
        foreach_node( this->m_after_nodes, TaskChildVisitor(this->proc(), *this), 1);
    }

    // Executes all finalize-instructions
    foreach_node( this->m_final_nodes, TaskChildVisitor(this->proc(), *this), 1);



    this->m_mainobject.reset(0); // workaround

    return Value();
}


/// @details
/// 
/*
Value
FetchTask::resolveColumn(const Column &col)
{
    return Value(this->getMainObject()->getColumn(col));
}
*/


/// @details
/// 
Value
FetchTask::_value(void) const
{
    return this->_name();
}

/// @details
/// 
String
FetchTask::_string(void) const
{
    return this->_value().data().asStr();
}

/// @details
/// 
String
FetchTask::_name(void) const
{
    return this->id().str() + String(" (task:fetch)");
}

/// @details
/// 
String
FetchTask::_type(void) const
{
    return "FETCHTASK";
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
