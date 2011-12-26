//
// task.cc - Task (definition)
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
/// @brief Elements (definition)
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
/////////////////////////////////////////////////////////////// TaskChildVisitor


TaskChildVisitor::TaskChildVisitor(Processor &proc, Context &context)
    : Visitor(Visitor::ignore_none),
      m_proc(proc),
      m_context(context)
      //m_task(task)
{}

void
TaskChildVisitor::visit(LogNode *node)
{
    LogCmd cmd(this->m_proc, m_context, node);
    this->m_proc.call(cmd);
}

void
TaskChildVisitor::visit(ColumnAssignNode *node)
{
    assert(node->getChilds().size() == 2);

    //ColumnAssignOp op(this->m_proc, m_context, node);
    //this->m_proc.call(op);

    Column col;
    LValueColumnVisitor(this->m_proc, this->m_context, col)(node->getChilds()[0]);

    //Column col(dynamic_cast<ColumnNode*>(node->getChilds()[0]));

    Value val;
    EvalExprVisitor eval(this->m_proc, this->m_context, val);
    eval(node->getChilds()[1]);

    this->m_context.getDestObject()->setColumn(col, val);


    //this->m_context.getMainObject()->setColumn(Column("id"), Value(23));
    //this->m_context.getMainObject()->setColumn(Column("name"), Value(23));

    // ARGON_ICERR_CTX.., assign not allowed?
}


void
TaskChildVisitor::TaskChildVisitor::visit(TaskExecNode *node)
{
    ARGON_DPRINT(ARGON_MOD_PROC, "Calling task " << node->taskid().str());
    //foreach_node(node->getChilds(), PrintTreeVisitor(this->m_proc, std::wcout), 1);
    
    Task* task = this->m_proc.getSymbols().find<Task>(node->data()); // search task global
    
    /// @bug fix this
    ArgumentList al;
    
    
    //ARGON_ICERR_CTX(node->getChilds().size() >= 1, this->m_proc,
    //"IdCallNode does not contains any subnodes");
        
        //ArgumentsNode *argsnode = node_cast<ArgumentsNode>(node->getChilds().at(1));
        //safe_ptr<ArgumentsNode>
        ArgumentsNode *argsnode = find_node<ArgumentsNode>(node);
        assert(argsnode);
        
        foreach_node(argsnode->getChilds(), ArgumentsVisitor(this->m_proc, m_context, al), 1);
        /*
        al.push_back(Value(String("foo1")));
        al.push_back(Value(String("foo2")));
        */

        // get arguments
        this->m_proc.call(task, al);
}


void
TaskChildVisitor::visit(SqlExecNode *node)
{
    SqlExecCmd cmd(this->m_proc, m_context, node);
    this->m_proc.call(cmd);
}

void
TaskChildVisitor::visit(ArgumentsSpecNode *node)
{
    // This visitor only handles instructions, so it's save to ignore this node.
}

void
TaskChildVisitor::visit(TmplArgumentsNode *node)
{
    // This visitor only handles instructions, so it's save to ignore this node.
}



//..............................................................................
/////////////////////////////////////////////////////////////////////////// Task


/// @details
/// This runs some code that is required by all tasks like Arguments->Symboltable
Value
Task::run(const ArgumentList &args)
{
    ARGON_DPRINT(ARGON_MOD_PROC, "Running task " << this->id());

    // Load TASK arguments to local symbol table
    this->getSymbols().reset();
    safe_ptr<ArgumentsSpecNode> argsSpecNode = find_node<ArgumentsSpecNode>(this->m_node);

    ARGON_ICERR_CTX(argsSpecNode.get() != 0, *this,
                "no argument specification");

    ARGON_ICERR_CTX(argsSpecNode->getChilds().size() == args.size(), *this,
                "Argument count mismatch");
    
    ArgumentList::const_iterator i = args.begin();
    foreach_node(argsSpecNode->getChilds(), Arg2SymVisitor(this->proc(), *this, i), 1);

    return Value();
}



/// @details
/// 
Task::Task(Processor &proc, TaskNode *node)
    : Context(proc),
      m_node(node),
      m_init_nodes(),
      m_before_nodes(),
      m_rules_nodes(),
      m_after_nodes(),
      m_final_nodes()
{
    NodeList childs = node->getChilds();

    assert(childs.size() >= 2);

    {
        Node *n = find_node<TaskInitNode>(this->m_node);
        if(n)
	{
            this->m_init_nodes = n->getChilds();
	    assert(find_nodes<ColumnNode>(n).empty());
	    assert(find_nodes<ColumnNumNode>(n).empty());
	    assert(find_nodes<ResColumnNode>(n).empty());
	    assert(find_nodes<ResColumnNumNode>(n).empty());
	    assert(find_nodes<ColumnAssignNode>(n).empty());
	}

    }

    {
        Node *n = find_node<TaskBeforeNode>(this->m_node);
        if(n)
	{
            this->m_before_nodes = n->getChilds();
	    assert(find_nodes<ResColumnNode>(n).empty());
	    assert(find_nodes<ResColumnNumNode>(n).empty());
	    assert(find_nodes<ColumnAssignNode>(n).empty());
	}
    }

    {
        Node *n = find_node<TaskRulesNode>(this->m_node);
        if(n)
	{
            this->m_rules_nodes = n->getChilds();
	    assert(find_nodes<ResColumnNode>(n).empty());
	    assert(find_nodes<ResColumnNumNode>(n).empty());
	}
    }

    {
        Node *n = find_node<TaskAfterNode>(this->m_node);
        if(n)
            this->m_after_nodes = n->getChilds();
    }

    {
        Node *n = find_node<TaskFinalNode>(this->m_node);
        if(n)
            this->m_final_nodes = n->getChilds();
    }


/*

    // Skip first two arguments. Checked by semantic checker, too.
    size_t c = 3;
    
    while(c <= childs.size() && !is_nodetype<ColumnAssignNode*>(childs[c-1]))
    {
        m_pre_nodes.push_back(childs[c-1]);
        ++c;
    }
    
    while(c <= childs.size() && is_nodetype<ColumnAssignNode*>(childs[c-1]))
    {
        m_colassign_nodes.push_back(childs[c-1]);
        ++c;
    }
    
    while(c <= childs.size() && !is_nodetype<ColumnAssignNode*>(childs[c-1]))
    {
        m_post_nodes.push_back(childs[c-1]);
        ++c;
    }
    
    // All childs must been processed.
    assert(c > childs.size());
*/
}



/// @details
/// 
String
Task::str(void) const
{
    String s;
    s.append(this->id().str());
    s.append("[TASK]");
    return s;
}


/// @details
/// 
String
Task::name(void) const
{
    return this->id().str();
}


/// @details
/// 
String
Task::type(void) const
{
    return "TASK";
}


/// @details
/// 
SourceInfo
Task::getSourceInfo(void) const
{
    return this->m_node->getSourceInfo();
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
