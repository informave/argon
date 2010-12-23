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
#include "semantic.hh"
#include "debug.hh"

#include <iostream>
#include <list>

ARGON_NAMESPACE_BEGIN


//..............................................................................
//////////////////////////////////////////////////////////////// SemanticChecker
///
/// @since 0.0.1
/// @brief Semantic checker
class SemanticWalker : public Visitor
{
public:
    SemanticWalker(SemanticCheck &check)
    {
    }


    virtual void visit(ConnNode *node)
    {
        ARGON_DPRINT(ARGON_MOD_SEM, "checking connection node");
    }

    virtual void visit(TaskNode *node)
    {
        ARGON_DPRINT(ARGON_MOD_SEM, "checking task node");
    }

    virtual void visit(ParseTree *node)
    {
        ARGON_DPRINT(ARGON_MOD_SEM, "checking parsetree node");
    }


    virtual void visit(TableNode *node)
    {
        ARGON_DPRINT(ARGON_MOD_SEM, "checking table node");

        // Argument nodes check
        {
            ArgumentsNode* args = find_node<ArgumentsNode>(node);
            assert(args);
            assert(args->getChilds().size() >= 2 && args->getChilds().size() <= 4);

            IdNode      *node1 = node_cast<IdNode>(args->getChilds().at(0));
            LiteralNode *node2 = node_cast<LiteralNode>(args->getChilds().at(1));
        }

        //node_cast<LiteralNode>(arg_node->getChilds().at(0));

        //assert(arg_node->getChilds().size() >= 2 && arg_node->getChilds().size() <= 4);

    }

};


//..............................................................................
////////////////////////////////////////////////////////////////// SemanticCheck


/// @details
/// 
SemanticCheck::SemanticCheck(ParseTree *tree)
    : m_tree(tree)
{
}


/// @details
/// 
void
SemanticCheck::check(void)
{
    ARGON_DPRINT(ARGON_MOD_SEM, "Checking semantic");

    foreach_node( this->m_tree, SemanticWalker(*this), 2); // only deep 1
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
