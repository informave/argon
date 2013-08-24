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
#include "argon/rtti.hh"
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
TaskChildVisitor::visit(ConditionCmdNode *node)
{
    ARGON_ICERR(node, "invalid node");
    ARGON_ICERR(node->getChilds().size() == 1, "invalid child size");
    Value val;
    apply_visitor(node->getChilds()[0], EvalExprVisitor(m_proc, m_context, val));
    if(val.data().isnull() || val.data().get<bool>() == false)
    {
        throw ConditionControlException();
    }
}


void
TaskChildVisitor::visit(ThrowNode *node)
{
    ARGON_ICERR(node, "invalid node");
    if(node->getChilds().size() == 2) // throw id;
    {
        ArgumentList al;
        Identifier id = find_node<IdNode>(node)->data();
        ArgumentsNode *argsnode = find_node<ArgumentsNode>(node);
        ARGON_ICERR(argsnode, "invalid node");

        // Fill argument list with the result of each argument node
        foreach_node(argsnode->getChilds(), ArgumentsVisitor(m_proc, m_context, al), 1);

        //
        ExceptionType *type  = this->m_proc.getTypes().find<ExceptionType>(id);

        type->throwException(al);
    }
    else // throw; (re-throw current exception)
    {
        throw RethrowControlException();
    }

    //LogCmd cmd(this->m_proc, m_context, node);
    //this->m_proc.call(cmd);
}




void
TaskChildVisitor::visit(ColumnAssignNode *node)
{
    ARGON_ICERR(node->getChilds().size() == 2, "invalid child count");

    //ColumnAssignOp op(this->m_proc, m_context, node);
    //this->m_proc.call(op);

    Column col;
    LValueColumnVisitor(this->m_proc, this->m_context, col)(node->getChilds()[0]);

    //Column col(dynamic_cast<ColumnNode*>(node->getChilds()[0]));

    Value val;

    Node *value_node = node->getChilds()[1];

    if(is_nodetype<LambdaFuncNode*>(value_node))
    {
        //assert(1==2);

        Element *elem = new Lambdafunction(this->m_proc, node_cast<LambdaFuncNode>(value_node), this->m_context);
        ARGON_SCOPED_STACKPUSH(this->m_proc, elem);

        val.data() = m_proc.call(*elem).data();

/*
  Element *elem = 0;

  elem = this->m_proc.getTypes().find<FunctionType>(id)->newInstance(al);

  //assert(elem);

  ARGON_SCOPED_STACKPUSH(this->m_proc, elem);


  {
  /// @bug just call m_proc.call(<function-id>, args)
  // m_value.data() = this->m_proc.call(id, al).data();
  m_value.data() = m_proc.call(*elem).data();
  }
*/

    }
    else
    {
        EvalExprVisitor eval(this->m_proc, this->m_context, val);
        eval(node->getChilds()[1]);
    }

    this->m_context.getDestObject()->setColumn(col, val);


    //this->m_context.getMainObject()->setColumn(Column("id"), Value(23));
    //this->m_context.getMainObject()->setColumn(Column("name"), Value(23));

    // ARGON_ICERR_CTX.., assign not allowed?
}


void
TaskChildVisitor::visit(TaskExecNode *node)
{
    ARGON_DPRINT(ARGON_MOD_PROC, "Calling task " << node->taskid().str());

    safe_ptr<ArgumentsNode> argsnode = find_node<ArgumentsNode>(node);

    this->m_proc.call(node->data(), argsnode.get(), this->m_context);
}



void
TaskChildVisitor::visit(ExecFunctionCmdNode *node)
{
    //ARGON_DPRINT(ARGON_MOD_PROC, "Calling function " << node->taskid().str());

    safe_ptr<IdNode> idnode = find_node<IdNode>(node);

    safe_ptr<ArgumentsNode> argsnode = find_node<ArgumentsNode>(node);

    this->m_proc.call(idnode->data(), argsnode.get(), this->m_context);
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
Task::run(void)
{
    ARGON_DPRINT(ARGON_MOD_PROC, "Running task " << this->id());

    // Load TASK arguments to local symbol table
    this->getSymbols().reset();
    safe_ptr<ArgumentsSpecNode> argsSpecNode = find_node<ArgumentsSpecNode>(this->m_node);

    ARGON_ICERR_CTX(argsSpecNode.get() != 0, *this,
                    "no argument specification");

    ARGON_ICERR_CTX(argsSpecNode->getChilds().size() == this->getCallArgs().size(), *this,
                    "Argument count mismatch");

    ArgumentList::const_iterator i = this->getCallArgs().begin();
    foreach_node(argsSpecNode->getChilds(),
                 Arg2SymVisitor(this->proc(), this->getSymbols(), i, this->getCallArgs().end()), 1);

    return Value();
}




/// @details
///
void
Task::processData(void)
{
    try
    {
        // implemented by derived task types.
        this->do_processData();
    }
    catch(ConditionControlException &e)
    {
        // nothing to do
    }
    catch(informave::db::SqlstateException &e)
    {
        this->setCurrentException(new ExceptionInfo<informave::db::SqlstateException>(e));
        ARGON_SCOPED_RELEASE_EXCEPTION(*this);
        bool rethrow = false;

        try
        {
            std::string state = e.diag().getSqlstate().asStr();
            Node *exhn = this->m_exh_sqlstates[state];
            if(exhn) // exception gets handled
            {
                foreach_node(exhn->getChilds(), TaskChildVisitor(this->proc(), *this), 1);
            }
            else
            {
                if(this->m_exh_catchall)
                {
                    foreach_node(this->m_exh_catchall->getChilds(),
                                 TaskChildVisitor(this->proc(), *this), 1);
                }
                else // unable to handle exception here...
                    throw e;
            }
        }
        catch(RethrowControlException &)
        {
            rethrow = true;
        }
        if(rethrow) throw;
    }
    catch(CustomException &e)
    {
        this->setCurrentException(new ExceptionInfo<CustomException>(e));
        ARGON_SCOPED_RELEASE_EXCEPTION(*this);
        bool rethrow = false;

        try
        {
            Identifier id = e.getType().id();
            Node *exhn = this->m_exh_ids[id];
            if(exhn) // exception gets handled
            {
                foreach_node(exhn->getChilds(), TaskChildVisitor(this->proc(), *this), 1);
            }
            else // maybe catchall?
            {
                if(this->m_exh_catchall)
                {
                    foreach_node(this->m_exh_catchall->getChilds(),
                                 TaskChildVisitor(this->proc(), *this), 1);
                }
                else // unable to handle exception here...
                    throw e;
            }
        }
        catch(RethrowControlException &)
        {
            rethrow = true;
        }
        if(rethrow) throw;
    }
    catch(informave::db::Exception &e)
    {
        this->setCurrentException(new ExceptionInfo<informave::db::Exception>(e));
        ARGON_SCOPED_RELEASE_EXCEPTION(*this);
        bool rethrow = false;

        try
        {
            if(this->m_exh_catchall)
            {
                foreach_node(this->m_exh_catchall->getChilds(),
                             TaskChildVisitor(this->proc(), *this), 1);
            }
            else // unable to handle exception here...
                throw e;
        }
        catch(RethrowControlException &)
        {
            rethrow = true;
        }
        if(rethrow) throw;
    }
}




/// @details
///
Task::Task(Processor &proc, TaskNode *node, const ArgumentList &args)
    : Context(proc, args),
      m_node(node),
      m_init_nodes(),
      m_before_nodes(),
      m_rules_nodes(),
      m_after_nodes(),
      m_final_nodes(),
      m_exh_sqlstates(),
      m_exh_ids(),
      m_exh_catchall(NULL_NODE)

{
    NodeList childs = node->getChilds();

    ARGON_ICERR(childs.size() >= 2, "invalid child count");

    {
        Node *n = find_node<TaskInitNode>(this->m_node);
        if(n && (n = find_node<TaskRuleBlockNode>(n)))
        {
            this->m_init_nodes = n->getChilds();
            ARGON_ICERR(find_nodes<ColumnNode>(n).empty(), "column node not empty");
            ARGON_ICERR(find_nodes<ColumnNumNode>(n).empty(), "columnnum node not empty");
            ARGON_ICERR(find_nodes<ResColumnNode>(n).empty(), "columnres node not empty");
            ARGON_ICERR(find_nodes<ResColumnNumNode>(n).empty(), "columnresnum node not empty");
            ARGON_ICERR(find_nodes<ColumnAssignNode>(n).empty(), "columnassign node not empty");
        }

    }

    {
        Node *n = find_node<TaskBeforeNode>(this->m_node);
        if(n && (n = find_node<TaskRuleBlockNode>(n)))
        {
            this->m_before_nodes = n->getChilds();
            ARGON_ICERR(find_nodes<ResColumnNode>(n).empty(), "rescolumn node not empty");
            ARGON_ICERR(find_nodes<ResColumnNumNode>(n).empty(), "rescolumnnum node not empty");
            ARGON_ICERR(find_nodes<ColumnAssignNode>(n).empty(), "columnassign node not empty");
        }
    }

    {
        Node *n = find_node<TaskRulesNode>(this->m_node);
        if(n && (n = find_node<TaskRuleBlockNode>(n)))
        {
            this->m_rules_nodes = n->getChilds();
            ARGON_ICERR(find_nodes<ResColumnNode>(n).empty(), "rescolumn node not empty");
            ARGON_ICERR(find_nodes<ResColumnNumNode>(n).empty(), "rescolumnnum node not empty");
        }
    }

    {
        Node *n = find_node<TaskAfterNode>(this->m_node);
        if(n && (n = find_node<TaskRuleBlockNode>(n)))
            this->m_after_nodes = n->getChilds();
    }

    {
        std::list<TaskExceptLiteralNode*> a = find_nodes<TaskExceptLiteralNode>(this->m_node);

        std::for_each(a.begin(), a.end(), [this](Node* n){
                TaskRuleBlockNode *rbn = 0;
                if(n && (rbn = find_node<TaskRuleBlockNode>(n)))
                {
                    String sqlstate = node_cast<LiteralNode>(n->getChilds().at(0))->data();
                    std::string s = sqlstate;
                    this->m_exh_sqlstates[s] = rbn;
                }
            });
    }

    {
        std::list<TaskExceptIdNode*> a = find_nodes<TaskExceptIdNode>(this->m_node);

        std::for_each(a.begin(), a.end(), [this](Node* n){
                TaskRuleBlockNode *rbn = 0;
                if(n && (rbn = find_node<TaskRuleBlockNode>(n)))
                {
                    Identifier id = node_cast<IdNode>(n->getChilds().at(0))->data();

                    /// @bug make sure it's an exception type
                    this->proc().getTypes().find<ExceptionType>(id);

                    this->m_exh_ids[id] = rbn;
                }
            });
    }

    {
        TaskRuleBlockNode *rbn = 0;
        Node *n = find_node<TaskExceptNode>(this->m_node);
        if(n && (rbn = find_node<TaskRuleBlockNode>(n)))
            this->m_exh_catchall = rbn;
    }



    {
        Node *n = find_node<TaskFinalNode>(this->m_node);
        if(n && (n = find_node<TaskRuleBlockNode>(n)))
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
//assert(c > childs.size());
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
