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



//..............................................................................
/////////////////////////////////////////////////////////////// StoreTaskVisitor
///
/// @since 0.0.1
/// @brief Visitor
/// @details
/// Prepares the task template argument list
/// 
/// TASK foo() AS STORE [ obj1(x) ]
///                       ^^^^
/// STORE tasks only takes exactly one template argument.
/// This visitor must be called directly on the template argument node.
/// 


struct StoreTaskVisitor : public Visitor
{
public:
    StoreTaskVisitor(Processor &proc, Context &context, ObjectInfo *&obj)
        : Visitor(Visitor::ignore_none),
          m_proc(proc),
          m_context(context),
          m_objinfo(obj)
    {}


    virtual void visit(IdNode *node)
    {
        Identifier id = node->data();
        this->m_objinfo = this->m_proc.getSymbols().find<ObjectInfo>(id); /// @bug replace with context?
    }
    
    
    virtual void visit(IdCallNode *node)
    {
        ARGON_ICERR_CTX(node->getChilds().size() >= 1, this->m_context,
                    "IdCallNode does not contains any subnodes");

        IdNode *idnode = node_cast<IdNode>(node->getChilds().at(0));

        Identifier id = idnode->data();

        this->m_objinfo = this->m_proc.getSymbols().find<ObjectInfo>(id);
    }

    virtual void visit(TableNode *node)
    {
        ObjectInfo *elem = this->m_context.getSymbols().addPtr( new ObjectInfo(this->m_proc, node) );
        this->m_context.getSymbols().add(node->id, elem); // @bug using anonymous id

        m_objinfo = elem;
    }


protected:
    Processor &m_proc;
    Context &m_context;
    ObjectInfo *&m_objinfo;
        
};








struct ResColumnVisitor : public Visitor
{
public:
    ResColumnVisitor(Processor &proc, Context &context, ColumnList &list)
        : Visitor(Visitor::ignore_other),
          m_proc(proc),
          m_context(context),
          m_list(list)
    {}

    
    virtual void visit(ResColumnNode *node)
    {
        m_list.insert(Column(node));
    }

    virtual void visit(ResColumnNumNode *node)
    {
        m_list.insert(Column(node));
    }

protected:
    Processor &m_proc;
    Context &m_context;
    ColumnList &m_list;
};



struct DeepColumnVisitor : public Visitor
{
public:
    DeepColumnVisitor(Processor &proc, Context &context, ColumnList &list)
        : Visitor(Visitor::ignore_other),
          m_proc(proc),
          m_context(context),
          m_list(list)
    {}

    
    virtual void visit(ColumnNode *node)
    {
        m_list.insert(Column(node));
    }

    virtual void visit(ColumnNumNode *node)
    {
        m_list.insert(Column(node));
    }

    virtual void visit(ColumnAssignNode *node)
    {
        ARGON_ICERR_CTX(false, this->m_context,
                    "Column assign expression is not allowed in DeepColumnVisitor");
    }


protected:
    Processor &m_proc;
    Context &m_context;
    ColumnList &m_list;
};




struct ColumnVisitor : public Visitor
{
public:
    ColumnVisitor(Processor &proc, Context &context, ColumnList &left_list, ColumnList &right_list)
        : Visitor(Visitor::ignore_none),
          m_proc(proc),
          m_context(context),
          m_left_list(left_list),
          m_right_list(right_list)
    {}


    virtual void visit(ColumnAssignNode *node)
    {
        ARGON_ICERR_CTX(node->getChilds().size() == 2, this->m_context,
                    "Column assign expression must contain exactly two childs.");
        
        foreach_node( node->getChilds()[0], DeepColumnVisitor(this->m_proc, this->m_context, m_left_list) );
        foreach_node( node->getChilds()[1], DeepColumnVisitor(this->m_proc, this->m_context, m_right_list) );
    }

protected:

    virtual void fallback_action(Node *node)
    {
        foreach_node( node, DeepColumnVisitor(this->m_proc, this->m_context, m_right_list) );
    }


protected:
    Processor &m_proc;
    Context &m_context;
    ColumnList &m_left_list;
    ColumnList &m_right_list;
        
};






//..............................................................................
//////////////////////////////////////////////////////////// StoreTaskArgVisitor
///
/// @since 0.0.1
/// @brief Visitor
struct StoreTaskArgVisitor : public Visitor
{
public:
    StoreTaskArgVisitor(Processor &proc, Context &context, ArgumentList &list)
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
        ARGON_ICERR_CTX(node->getChilds().size() >= 1, this->m_context,
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
////////////////////////////////////////////////////////////////////// StoreTask

/// @details
/// 
StoreTask::StoreTask(Processor &proc, TaskNode *node)
    : Task(proc, node),
      m_mainobject()
{}


/// @details
/// 
Object*
StoreTask::getMainObject(void) 
{ 
    return this->m_mainobject.get(); 
}


/// @details
/// 
Object*
StoreTask::getResultObject(void) 
{ 
    return this->m_mainobject.get();
/*
    ARGON_ICERR_CTX(false, *this,
                "A STORE task does not contains a result object.");
*/
}


/// @details
/// 
Object*
StoreTask::getDestObject(void)
{
    ARGON_ICERR_CTX(false, *this,
                "A STORE task does not contains a destination object.");
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
        
    Node *destArgNode = tmplArgNode->getChilds().at(0);
    //Node *sourceArgNode = tmplArgNode->getChilds().at(0);

    //ObjectInfo *sourceInfoObj = 0;
    ObjectInfo *destInfoObj = 0;

    //foreach_node(sourceArgNode, StoreTaskVisitor(this->proc(), *this, sourceInfoObj), 1);
    foreach_node(destArgNode, StoreTaskVisitor(this->proc(), *this, destInfoObj), 1);

    ARGON_ICERR_CTX(destInfoObj != 0, *this,
                "destination information is not valid");


    //std::auto_ptr<Object> x(sourceInfoObj->newInstance(Object::READ_MODE));
    this->m_mainobject.reset(destInfoObj->newInstance(Object::ADD_MODE));

    ARGON_ICERR_CTX(this->m_mainobject.get() != 0, *this,
                "Main object allocation failed");

    //std::cout << x->getSourceInfo() << std::endl;

    // Prepare arguments

    //ArgumentList sourceArgs;
    ArgumentList destArgs;

    //this->getSymbols().add(Identifier("blabla"), new ValueElement(this->proc(), Value(String("myvalue"))));

    //foreach_node(sourceArgNode, StoreTaskArgVisitor(this->proc(), *this, sourceArgs), 1);
    foreach_node(destArgNode, StoreTaskArgVisitor(this->proc(), *this, destArgs), 1);




    // Get a list of the left and right columns
    ColumnList lclist, rclist;
    foreach_node( this->m_node->getChilds(), ColumnVisitor(this->proc(), *this, lclist, rclist), 1);
    this->m_mainobject->setColumnList(lclist);

    assert(rclist.size() == 0); // STORE tasks can't contain any column identifiers on right side.


    ColumnList reslist;
    foreach_node( this->m_node->getChilds(), ResColumnVisitor(this->proc(), *this, reslist));
    this->m_mainobject->setResultList(reslist);

/*
    for(ColumnList::iterator i = rclist.begin();
        i != rclist.end();
        ++i)
    {
        std::cout << i->getName() << std::endl;
    }

    std::cout << "other" << std::endl;

    for(ColumnList::iterator i = lclist.begin();
        i != lclist.end();
        ++i)
    {
        std::cout << i->getName() << std::endl;
    }
*/
    

    //this->proc().call(this->getMainObject(), sourceArgs); // now sql is prepared and executed
    this->proc().call(this->getMainObject(), destArgs); // now sql is prepared

   // IMPORTANT: destArgs may be used as values, Too!!

    {
	// TaskPreCmdVisitor
	// TaskAssignCmdVisitor
	// this->m_mainobject->execute();
	// this->getResultObject() contains now the values and can be used in PostCmdVisitor
	// TaskPostCmdVisitor

        foreach_node( this->m_pre_nodes, TaskChildVisitor(this->proc(), *this), 1);
        foreach_node( this->m_colassign_nodes, TaskChildVisitor(this->proc(), *this), 1);
        this->m_mainobject->execute();
        foreach_node( this->m_post_nodes, TaskChildVisitor(this->proc(), *this), 1);

/*
        foreach_node( this->m_node->getChilds(), TaskChildVisitor(this->proc(), *this), 1);
        this->m_mainobject->execute();
*/
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
StoreTask::resolveColumn(const Column &col)
{
    return Value(this->getMainObject()->getColumn(col));
}


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
