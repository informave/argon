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



//..............................................................................
/////////////////////////////////////////////////////////////// FetchTaskVisitor
///
/// @since 0.0.1
/// @brief Visitor
struct FetchTaskVisitor : public Visitor
{
public:
    FetchTaskVisitor(Processor &proc, Context &context, ObjectInfo *&obj)
        : Visitor(Visitor::ignore_none),
          m_proc(proc),
          m_context(context),
          m_objinfo(obj)
    {}

    virtual void visit(IdNode *node)
    {
        Identifier id = node->data();
        this->m_objinfo = this->m_proc.getSymbols().find<ObjectInfo>(id); // replace with context?
    }
    
    virtual void visit(IdCallNode *node)
    {
        ARGON_ICERR(node->getChilds().size() >= 1, this->m_context,
                    "IdCallNode does not contains any subnodes");

        IdNode *idnode = node_cast<IdNode>(node->getChilds().at(0));

        Identifier id = idnode->data();

        this->m_objinfo = this->m_proc.getSymbols().find<ObjectInfo>(id);
    }

    virtual void visit(TableNode *node)
    {
        //AnonymousObjectInfo *info = *magic*;
    }


protected:
    Processor &m_proc;
    Context &m_context;
    ObjectInfo *&m_objinfo;
        
};



//..............................................................................
//////////////////////////////////////////////////////////// FetchTaskArgVisitor
///
/// @since 0.0.1
/// @brief Visitor
struct FetchTaskArgVisitor : public Visitor
{
public:
    FetchTaskArgVisitor(Processor &proc, Context &context, ArgumentList &list)
        : Visitor(Visitor::ignore_none),
          m_proc(proc),
          m_context(context),
          m_list(list)
    {
    }

    virtual void visit(IdNode *node)
    {
        // no args
    }
    
    virtual void visit(IdCallNode *node)
    {
        ARGON_ICERR(node->getChilds().size() >= 1, this->m_context,
                    "IdCallNode does not contains any subnodes");

        ArgumentsNode *argsnode = node_cast<ArgumentsNode>(node->getChilds().at(1));

        foreach_node(argsnode->getChilds(), ArgumentsVisitor(this->m_proc, m_context, this->m_list), 1);
    }

    virtual void visit(TableNode *node)
    {
        // no args
    }


protected:
    Processor &m_proc;
    Context &m_context;
    ArgumentList &m_list;
        
};




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
    ARGON_ICERR(false, *this,
                "A FETCH task does not contains a result object.");
}


/// @details
/// 
Object*
FetchTask::getDestObject(void)
{
    ARGON_ICERR(false, *this,
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

    ARGON_ICERR(!!tmplArgNode, *this,
                "TASK does not have any template arguments");

    ARGON_ICERR(tmplArgNode->getChilds().size() == 1, *this,
                "wrong template argument count");
        
    //Node *destArgNode = tmplArgNode->getChilds().at(0);
    Node *sourceArgNode = tmplArgNode->getChilds().at(0);

    ObjectInfo *sourceInfoObj = 0;
    //ObjectInfo *destInfoObj = 0;

    foreach_node(sourceArgNode, FetchTaskVisitor(this->proc(), *this, sourceInfoObj), 1);
    //foreach_node(destArgNode, FetchTaskVisitor(this->proc(), destInfoObj), 1);

    ARGON_ICERR(sourceInfoObj != 0, *this,
                "source information is not valid");


    //std::auto_ptr<Object> x(sourceInfoObj->newInstance(Object::READ_MODE));
    this->m_mainobject.reset(sourceInfoObj->newInstance(Object::READ_MODE));

    //std::cout << x->getSourceInfo() << std::endl;

    // Prepare arguments

    ArgumentList sourceArgs;
    //ArgumentList destArgs;

    //this->getSymbols().add(Identifier("blabla"), new ValueElement(this->proc(), Value(String("myvalue"))));

    foreach_node(sourceArgNode, FetchTaskArgVisitor(this->proc(), *this, sourceArgs), 1);
    //foreach_node(destArgNode, FetchTaskArgVisitor(this->proc(), this->getSymbols(), destArgs), 1);



    this->proc().call(this->getMainObject(), sourceArgs); // now sql is prepared and executed

    while(! this->m_mainobject->eof())
    {
        foreach_node( this->m_node->getChilds(), TaskChildVisitor(this->proc(), *this), 1);

#ifdef ARGON_DEV_DEBUG
        std::cout << "FOUND ROW" << std::endl;
        std::cout << this->m_mainobject->getColumn(Column(1)) << std::endl;
        std::cout << this->m_mainobject->getColumn(Column(2)) << std::endl;
#endif

        this->m_mainobject->next();
    }

    
    this->m_mainobject.reset(0); // workaround


/*
// Source
x->addColumn(Column("foo"));
// prepare query and open it
this->proc().call(&*x, ls); // add dest params

// Destination
y->addColumn(Column("bar"));
this->proc().call(&*y, ls);


while(x->next())
{
// handle pre-insert-expressions.. are there any - yes, set options?
        

// column expr eval can be moved to ColumnAssignNode-Visitor
y->setColumn(Column("bar"), x->getColumn(Column("foo")));

y->post();

// handle post-insert-expressions
}
*/

    return Value();
}


/// @details
/// 
Value
FetchTask::resolve(const Column &col)
{
    return Value(this->getMainObject()->getColumn(col));
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
