//
// semantic.cc - Semantic checker (definition)
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
/// @brief Semantic checker (definition)
/// @author Daniel Vogelbacher
/// @since 0.1


#include "argon/fwd.hh"
#include "argon/exceptions.hh"
#include "argon/semantic.hh"
#include "debug.hh"

#include <iostream>
#include <list>

ARGON_NAMESPACE_BEGIN


#define ARGON_SC_ADD(sc, type, what, srcinfo)                           \
    sc.addCheckEntry(SemanticCheckEntry(SemanticCheck::type, what, srcinfo))



//..............................................................................
//////////////////////////////////////////////////////////// SemanticCheckRunner
///
/// @since 0.0.1
/// @brief Semantic checker
class SemanticCheckRunner : public Visitor
{
public:
    SemanticCheckRunner(SemanticCheck &check)
        : Visitor(ignore_none),
          m_check(check)
    {}


protected:

    virtual void fallback_action(Node *node)
    {
        node->semanticCheck(m_check);
    }

    SemanticCheck &m_check;
};




//..............................................................................
/////////////////////////////////////////////////////////////////// TaskExecNode


/// @details
/// 
void
TaskExecNode::semanticCheck(SemanticCheck &sc)
{
    TaskNode *task = find_node_byid<TaskNode>(sc.ast(), Identifier(this->data()));

    if(!task)
    {
        ARGON_SC_ADD(sc, SC_ERROR, String("Task not defined: ").append(this->data().str()), this->getSourceInfo());
        return;
    }


    ArgumentsNode *argsnode = find_node<ArgumentsNode>(this);
    assert(argsnode);

    if(argsnode->getChilds().size() != node_cast<ArgumentsSpecNode>(task->getChilds()[0])->getChilds().size())
        ARGON_SC_ADD(sc, SC_ERROR, "Argument count mismatch", this->getSourceInfo());

}




//..............................................................................
/////////////////////////////////////////////////////////////////////// TaskNode

/// @details
/// 
void
TaskNode::semanticCheck(SemanticCheck &sc)
{
    NodeList childs = this->getChilds();       

    ARGON_ICERR(childs.size() >= 2,
                "Task node requires at least 2 child nodes");

    ARGON_ICERR(is_nodetype<ArgumentsSpecNode*>(childs[0]),
                "Task node requires that the first child node is a ArgumentsSpecNode");

    ARGON_ICERR(is_nodetype<TmplArgumentsNode*>(childs[1]),
                "Task node requires that the second child node is a TmplArgumentsNode");

        
    size_t c = 3; // skip ArgumentSpec and TmpArguments nodes (start with 1)

    while(c <= childs.size() && !is_nodetype<ColumnAssignNode*>(childs[c-1]))
    {
        /// @todo check that there is no % operator node, but only in STORE and TRANSFER if there
        /// are no colassign expressions.
        ++c;
    }

    while(c <= childs.size() && is_nodetype<ColumnAssignNode*>(childs[c-1]))
    {
        /// @todo check that there is no % operator node
        ++c;
    }

    while(c <= childs.size() && !is_nodetype<ColumnAssignNode*>(childs[c-1]))
        ++c;


    if(c <= childs.size())
        ARGON_SC_ADD(sc, SC_ERROR, "Column assignment not allowed here", this->getSourceInfo());
}



//..............................................................................
////////////////////////////////////////////////////////////////////// TableNode

/// @details
/// 
void
TableNode::semanticCheck(SemanticCheck &sc)
{
    ARGON_DPRINT(ARGON_MOD_SEM, "checking table node");

    // Argument nodes check
    {
        ArgumentsNode* args = find_node<ArgumentsNode>(this);
        assert(args);
        assert(args->getChilds().size() >= 2 && args->getChilds().size() <= 4);

        IdNode      *node1 = node_cast<IdNode>(args->getChilds().at(0));
        LiteralNode *node2 = node_cast<LiteralNode>(args->getChilds().at(1));
    }
}



//..............................................................................
//////////////////////////////////////////////////////////////////////// SqlNode

/// @details
/// 
void
SqlNode::semanticCheck(SemanticCheck &sc)
{
    ARGON_DPRINT(ARGON_MOD_SEM, "checking sql node");

    // Argument nodes check
    {
        ArgumentsNode* args = find_node<ArgumentsNode>(this);
        assert(args);
        assert(args->getChilds().size() == 2);

        IdNode      *node1 = node_cast<IdNode>(args->getChilds().at(0));
        LiteralNode *node2 = node_cast<LiteralNode>(args->getChilds().at(1));
    }
}



//..............................................................................
//////////////////////////////////////////////////////////////////////// SqlNode

/// @details
/// 
void
IdNode::semanticCheck(SemanticCheck &sc)
{
}


/// @details
/// 
void
FuncCallNode::semanticCheck(SemanticCheck &sc)
{
}

/// @details
/// 
void
LiteralNode::semanticCheck(SemanticCheck &sc)
{
}

/// @details
/// 
void
ColumnNode::semanticCheck(SemanticCheck &sc)
{
}

/// @details
/// 
void
ResColumnNode::semanticCheck(SemanticCheck &sc)
{
}

/// @details
/// 
void
ResIdNode::semanticCheck(SemanticCheck &sc)
{
}

/// @details
/// 
void
ArgumentsNode::semanticCheck(SemanticCheck &sc)
{
}

/// @details
/// 
void
ColumnAssignNode::semanticCheck(SemanticCheck &sc)
{
}

/// @details
/// 
void
ArgumentsSpecNode::semanticCheck(SemanticCheck &sc)
{
}


/// @details
/// 
void
TmplArgumentsNode::semanticCheck(SemanticCheck &sc)
{
}


/// @details
/// 
void
IdCallNode::semanticCheck(SemanticCheck &sc)
{
}


/// @details
/// 
void
LogNode::semanticCheck(SemanticCheck &sc)
{
}

/// @details
/// 
void
TaskInitNode::semanticCheck(SemanticCheck &)
{
}

/// @details
/// 
void
TaskBeforeNode::semanticCheck(SemanticCheck &)
{
}

/// @details
/// 
void
TaskRulesNode::semanticCheck(SemanticCheck &)
{
}

/// @details
/// 
void
TaskAfterNode::semanticCheck(SemanticCheck &)
{
}

/// @details
/// 
void
TaskFinalNode::semanticCheck(SemanticCheck &)
{
}









//..............................................................................
////////////////////////////////////////////////////////////////// SemanticCheck


/// @details
/// 
SemanticCheck::SemanticCheck(ParseTree *tree, Processor &proc)
    : m_tree(tree),
      m_proc(proc),
      m_list()
{}


/// @details
/// 
void
SemanticCheck::check(void)
{
    ARGON_DPRINT(ARGON_MOD_SEM, "Checking semantic");

    foreach_node( this->m_tree, SemanticCheckRunner(*this)); // for each node

    if(this->m_list.size())
        throw SemanticError(this->m_list);
}


/// @details
/// 
void
SemanticCheck::addCheckEntry(const SemanticCheckEntry &entry)
{
    this->m_list.push_back(entry);
}




//..............................................................................
///////////////////////////////////////////////////////////// SemanticCheckEntry


/// @details
/// 
SemanticCheckEntry::SemanticCheckEntry(SemanticCheck::sctype type, const String &what, const SourceInfo &info)
    : m_type(type),
      m_what(what),
      m_info(info)
{}


/// @details
/// 
SemanticCheck::sctype
SemanticCheckEntry::getType(void) const
{
    return this->m_type;
}


/// @details
/// 
String
SemanticCheckEntry::_str(void) const
{
    std::wstringstream ss;
    ss << this->m_info.str() << " Error: " << this->m_what;
    return ss.str();
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
