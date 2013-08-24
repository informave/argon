//
// sqlexec.cc - EXEC SQL Command (definition)
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
/// @brief EXEC SQL Command (definition)
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
//////////////////////////////////////////////////////////// SqlExecChildVisitor
///
/// @since 0.0.1
/// @brief SqlExec Child Visitor
struct SqlExecChildVisitor : public Visitor
{
public:
    SqlExecChildVisitor(Processor &proc, Context &context, SqlExecCmd &cmd, int &pnum)
        : Visitor(ignore_none),
          m_proc(proc),
          m_cmd(cmd),
          m_context(context),
          m_pnum(pnum)
    {}

    /// @todo visitor for columns may speed up the processing of large columns

    void fallback_action(Node *node)
    {
        Value val;
        EvalExprVisitor eval(this->m_proc, this->m_context, val);
        eval(node);

        this->m_cmd.bindParam(m_pnum++, val);
    }


private:
    Processor          &m_proc;
    SqlExecCmd         &m_cmd;
    Context            &m_context;
    int                &m_pnum;

};




//..............................................................................
///////////////////////////////////////////////////////////////////// SqlExecCmd

/// @details
/// 
SqlExecCmd::SqlExecCmd(Processor &proc, Context &context, SqlExecNode *node)
    : Element(proc),
      m_context(context),
      m_node(node),
      m_stmt()
{}


/// @details
/// 
String
SqlExecCmd::str(void) const
{
    return "[EXECSQL: %s]";
}


/// @details
/// 
String
SqlExecCmd::name(void) const
{
    return ARGON_UNNAMED_ELEMENT;
}

/// @details
/// 
String
SqlExecCmd::type(void) const
{
    return "EXECSQL";
}


/// @details
/// 
SourceInfo
SqlExecCmd::getSourceInfo(void) const
{
    return this->m_node->getSourceInfo();
}


/// @details
/// 
void
SqlExecCmd::bindParam(int pnum, Value value)
{
    this->m_stmt->bind(pnum, value.data());

    ARGON_DPRINT(ARGON_MOD_PROC, "Bind param: " << pnum << " to " << value.data());
}



/// @details
/// 
Value
SqlExecCmd::run(void)
{
    ARGON_ICERR(! this->m_node->sql().empty(), "sql is empty");

    //assert(this->getCallArgs().size() == 0);

    Connection *con = this->m_context.resolve<Connection>(this->m_node->connid());
    db::Connection& dbc = con->getDbc();

    this->m_stmt.reset( dbc.newStatement() );
    this->m_stmt->prepare(this->m_node->sql());

    //foreach_node(this->m_node->getChilds(), PrintTreeVisitor(this->proc(), std::wcout));

    safe_ptr<ArgumentsNode> argsnode = find_node<ArgumentsNode>(this->m_node);

    int pnum = 1;
    foreach_node(argsnode->getChilds(), SqlExecChildVisitor(this->proc(), m_context, *this, pnum), 1);

    ARGON_DPRINT(ARGON_MOD_PROC, "Exec SQL on " << this->m_node->connid().str());

    this->m_stmt->execute();
    this->m_stmt->close();
    this->m_stmt.reset(0);

    return Value();
}



/// @details
/// 
Value
SqlExecCmd::_value(void) const
{
    return String("EXECSQL");
}

/// @details
/// 
String
SqlExecCmd::_string(void) const
{
    return "EXECSQL";
}

/// @details
/// 
String
SqlExecCmd::_name(void) const
{
    return "EXECSQL";
}

/// @details
/// 
String
SqlExecCmd::_type(void) const
{
    return "EXECSQL";
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
