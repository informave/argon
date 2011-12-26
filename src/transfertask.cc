//
// transfertask.cc - TRANSFER TASK (definition)
//
// Copyright (C)         informave.org
//   2011,               Daniel Vogelbacher <daniel@vogelbacher.name>
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
/// @brief TRANSFER TASK (definition)
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


typedef TemplateVisitor       TransferTemplateVisitor;
typedef TemplateArgVisitor    TransferTemplateArgVisitor;


//..............................................................................
/////////////////////////////////////////////////////////////////// TransferTask

/// @details
/// 
TransferTask::TransferTask(Processor &proc, TaskNode *node)
    : Task(proc, node),
      m_srcobject(),
      m_destobject()
{}


/// @details
/// 
Object*
TransferTask::getMainObject(void) 
{ 
    assert(this->m_srcobject.get());
    return this->m_srcobject.get(); 
}


/// @details
/// 
Object*
TransferTask::getResultObject(void) 
{ 
    assert(this->m_destobject.get());
    return this->m_destobject.get();
/*
    ARGON_ICERR_CTX(false, *this,
                "A STORE task does not contains a result object.");
*/
}


/// @details
/// 
Object*
TransferTask::getDestObject(void)
{
    assert(this->m_destobject.get());
    return this->m_destobject.get();
}


/// @details
/// 
Value
TransferTask::run(const ArgumentList &args)
{
    ARGON_DPRINT(ARGON_MOD_PROC, "Running task " << this->id());

    Task::run(args);

    // Get template arguments
    safe_ptr<TmplArgumentsNode> tmplArgNode = find_node<TmplArgumentsNode>(this->m_node);

    ARGON_ICERR_CTX(!!tmplArgNode, *this,
                "TASK does not have any template arguments");

    ARGON_ICERR_CTX(tmplArgNode->getChilds().size() == 2, *this,
                "wrong template argument count");


    ObjectInfo *srcInfoObj = 0;
    ObjectInfo *destInfoObj = 0;


    // Create destination object
    Node *destArgNode = tmplArgNode->getChilds().at(0);
    foreach_node(destArgNode, TransferTemplateVisitor(this->proc(), *this, destInfoObj), 1);
    ARGON_ICERR_CTX(destInfoObj != 0, *this,
                "destination information is not valid");

    this->m_destobject.reset(destInfoObj->newInstance(Object::ADD_MODE));
    ARGON_ICERR_CTX(this->m_destobject.get() != 0, *this,
                "Dest object allocation failed");

    // Handle destination object arguments
    ArgumentList destArgs;
    foreach_node(destArgNode, TransferTemplateArgVisitor(this->proc(), *this, destArgs), 1);



    // Create source object
    Node *srcArgNode = tmplArgNode->getChilds().at(1);
    foreach_node(srcArgNode, TransferTemplateVisitor(this->proc(), *this, srcInfoObj), 1);
    ARGON_ICERR_CTX(srcInfoObj != 0, *this,
                "source information is not valid");

    this->m_srcobject.reset(srcInfoObj->newInstance(Object::READ_MODE));
    ARGON_ICERR_CTX(this->m_srcobject.get() != 0, *this,
                "Source object allocation failed");

    // Handle source object arguments
    ArgumentList srcArgs;
    foreach_node(srcArgNode, TransferTemplateArgVisitor(this->proc(), *this, srcArgs), 1);




    // Get a list of the left and right columns
    ColumnList lclist, rclist;
    foreach_node( this->m_before_nodes, ColumnVisitor(this->proc(), *this, lclist, rclist), 1);
    foreach_node( this->m_rules_nodes, ColumnVisitor(this->proc(), *this, lclist, rclist), 1);
    foreach_node( this->m_after_nodes, ColumnVisitor(this->proc(), *this, lclist, rclist), 1);
    this->m_destobject->setColumnList(lclist);
    this->m_srcobject->setColumnList(rclist);


    // Get result columns
    ColumnList reslist;
    foreach_node( this->m_after_nodes, ResColumnVisitor(this->proc(), *this, reslist));
    foreach_node( this->m_final_nodes, ResColumnVisitor(this->proc(), *this, reslist));
    this->m_destobject->setResultList(reslist);


    // Call both objects to setup their initial environment
    // This prepares SQL statements etc.
    this->proc().call(this->getMainObject(), srcArgs);
    this->proc().call(this->getDestObject(), destArgs);

    // IMPORTANT: destArgs may be used as values, Too!!


    // Execute the main object (mostly this runs the SELECT statement)
    this->getMainObject()->execute();

    
    // Executes all init-instructions
    foreach_node( this->m_init_nodes, TaskChildVisitor(this->proc(), *this), 1);

    // Iterate over all records in the main object
    while(! this->getMainObject()->eof())
    {
        // Executes all before instructions
        foreach_node( this->m_before_nodes, TaskChildVisitor(this->proc(), *this), 1);
        // Executes all rules instructions
        foreach_node( this->m_rules_nodes, TaskChildVisitor(this->proc(), *this), 1);

        // Executes the destination object (put data to object)
        this->getDestObject()->execute();

        // Executes all after instructions
        foreach_node( this->m_after_nodes, TaskChildVisitor(this->proc(), *this), 1);


        this->getMainObject()->next();
    }

    // Executes all finalize-instructions
    foreach_node( this->m_final_nodes, TaskChildVisitor(this->proc(), *this), 1);


    
    this->m_destobject.reset(0); // workaround
    this->m_srcobject.reset(0); // workaround

    return Value();
}


/// @details
/// 
/*
Value
TransferTask::resolveColumn(const Column &col)
{
    assert(!"not impl");
    //return Value(this->getMainObject()->getColumn(col));
}
*/


/// @details
/// 
Value
TransferTask::_value(void) const
{
    return this->_name();
}

/// @details
/// 
String
TransferTask::_string(void) const
{
    return this->_value().data().asStr();
}

/// @details
/// 
String
TransferTask::_name(void) const
{
    return this->id().str() + String(" (task:transfer)");
}

/// @details
/// 
String
TransferTask::_type(void) const
{
    return "TRANSFERTASK";
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
