//
// expand.cc - EXPAND (definition)
//
// Copyright (C)         informave.org
//   2012,               Daniel Vogelbacher <daniel@vogelbacher.name>
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
/// @brief EXPAND (definition)
/// @author Daniel Vogelbacher
/// @since 0.1

#include "argon/dtsengine.hh"
#include "argon/exceptions.hh"
#include "argon/ast.hh"
#include "debug.hh"
#include "visitors.hh"

#include "table_sqlite.hh"

#include "strutils.hh"

#include <iostream>
#include <sstream>
#include <list>

ARGON_NAMESPACE_BEGIN



//..............................................................................
///////////////////////////////////////////////////////////////////////// Expand

/// @details
/// 
Expand::Expand(Processor &proc, const ArgumentList &args, DeclNode *node, Type::mode_t mode)
    : Object(proc, node, args),
      m_objname(),
      m_value(),
      m_sep(),
      m_splitter(),
      m_eof(false),
      m_curval(),
      m_mode(mode)
      //m_prepost_nodes(),
      //m_colassign_nodes()
{

    if(node)
    {
/*
    NodeList childs = node->getChilds();

    assert(childs.size() >= 2);

    // Skip first two arguments. Checked by semantic checker, too. TODO!
    size_t c = 3;
    
    while(c <= childs.size() && !is_nodetype<ColumnAssignNode*>(childs[c-1]))
    {
        m_prepost_nodes.push_back(childs[c-1]);
        ++c;
    }
    
    while(c <= childs.size() && is_nodetype<ColumnAssignNode*>(childs[c-1]))
    {
        m_colassign_nodes.push_back(childs[c-1]);
        ++c;
    }
    
    while(c <= childs.size() && !is_nodetype<ColumnAssignNode*>(childs[c-1]))
    {
        m_prepost_nodes.push_back(childs[c-1]);
        ++c;
    }
    
    // All childs must been processed.
    assert(c > childs.size());
*/
    }
}


/// @details
/// The column list is a list of column names or position numbers. These columns
/// are used in the TASK as source columns (we must build up a SQL SELECT statement)
/// or as destination columns (we must build up a SQL INSERT statement).
void
Expand::setColumnList(const ColumnList &list)
{
    //this->m_columns = list;
}


/// @details
/// The column list is a list fo column names or position numbers. These columns
/// are used in the TASK as result columns (% operator).
void
Expand::setResultList(const ColumnList &list)
{
	//assert(this->m_mode == Type::INSERT_MODE);
    //this->m_result_columns = list;
}


/// @details
/// This method can be used after a call to run() (which prepares the statement)
/// to set a column value. This values is directly bound to the statement.
void
Expand::setColumn(const Column &col, const Value &v)
{
    // removed because param binding works for both moded
	//assert(this->m_mode == Object::ADD_MODE);
    
    //assert(col.mode() == Column::by_number);

    // can be used later if engine can tell name of parameters
    //int i = this->getBindPosition(col);
    //assert(i > 0);

    //this->m_stmt->bind(col.getNum(), v.data());

	//this->m_column_values[col.getName()] = v;
//	this->m_column_values[c
}


/// @details
/// Returns a resultset column from the table
const db::Value&
Expand::getColumn(Column col)
{
    return this->m_curval; /// @bug only for column 1
    //return col.getFrom(m_stmt->resultset(), *this);
}




/// @details
///
Value
Expand::lastInsertRowId(void)
{
	return -1;
}


/// @details
/// 
bool
Expand::next(void)
{
    m_eof = !this->m_splitter->next();
    if(!m_eof)
    {
        m_curval = String(this->m_splitter->get_string());
    }
    return m_eof;
}


/// @details
/// 
bool
Expand::eof(void) const
{
    return this->m_eof;
}



/// @details
/// 
Value
Expand::run(void)
{
    Object::run();

    if(this->m_node == NULL_NODE)
    {
        ARGON_ICERR_CTX(this->getCallArgs().size() == 2, *this,
                        "expand requires 2 arguments");

        ArgumentList::const_iterator arg = this->getCallArgs().begin();

        this->m_value = (arg++)->cast<ValueElement>()->_value().data().get<String>();
        this->m_sep   = (arg++)->cast<ValueElement>()->_value().data().get<String>();

        ARGON_ICERR_CTX(this->m_sep.size() >= 1, *this,
                        "expand() separator must be of length >= 1");

        this->m_splitter.reset(new strutils::split<std::wstring>(
                                   m_value.begin(), m_value.end(),
                                   m_sep));
        this->next();
    }
    else
    {
        assert(!"bug");

/*
    //foreach_node( this->m_node->getChilds(), SqlObjectChildVisitor(this->proc(), *this, *this), 1);
    foreach_node( this->m_prepost_nodes, SqlObjectChildVisitor(this->proc(), *this, *this), 1);


    safe_ptr<ArgumentsNode> argNode = find_node<ArgumentsNode>(this->m_node);
    
    ARGON_ICERR_CTX(!!argNode, *this,
                "table node does not contains an argument node");

    ARGON_ICERR_CTX(argNode->getChilds().size() == 2, *this,
                "table argument count is invalid");


    safe_ptr<ArgumentsSpecNode> argSpecNode = find_node<ArgumentsSpecNode>(this->m_node);

    ARGON_ICERR_CTX(!!argSpecNode, *this,
                "table node does not contains an argument specification node");

    ARGON_ICERR_CTX((argSpecNode->getChilds().size() == this->getCallArgs().size()), *this,
                "argument count mismatch");

    // Prepare connection
    {
        Node::nodelist_type &args = argNode->getChilds();
        safe_ptr<IdNode> node = node_cast<IdNode>(args.at(0));
        //this->m_conn = this->proc().getSymbols().find<Connection>(node->data());
	this->m_conn = this->resolve<Connection>(node->data());
    }

    // Prepare table name, schema etc.
    {
        Node::nodelist_type &args = argNode->getChilds();
        safe_ptr<LiteralNode> node;


        /// @todo only literal strings supported for now. This
        /// can be extended to identifiers later.
        node = node_cast<LiteralNode>(args.at(1));
        sqlCommand = node->data();

    }
*/

    }
        

    return Value();
}



void
Expand::execute(void)
{
    //m_stmt->execute();
}



/// @details
/// 
String
Expand::str(void) const
{
    String s;
    //s.append(this->id().str());
    s.append("[EXPAND]");
    return s;
}


/// @details
/// 
String
Expand::name(void) const
{
    String s = this->id().str();
    return s.empty() ? "<anonymous>" : s;
}


/// @details
/// 
String
Expand::type(void) const
{
    return "EXPAND";
}


/// @details
/// 
SourceInfo
Expand::getSourceInfo(void) const
{
    assert(this->m_node);
    return this->m_node->getSourceInfo();
}


/// @details
/// 
Value
Expand::_value(void) const
{
    return String("EXPAND");
}

/// @details
/// 
String
Expand::_string(void) const
{
    return this->_value().data().asStr();
}

/// @details
/// 
String
Expand::_name(void) const
{
    return this->id().str() + String(" (expand)");
}

/// @details
/// 
String
Expand::_type(void) const
{
    return "GEN_RANGE";
}


/// @details
/// 
Expand*
Expand::newInstance(Processor &proc, const ArgumentList &args, DeclNode *node, Type::mode_t mode)
{
    /// the dbc argument is currently not used, because there is no
    /// special handling for different db engines.

    if(!node)
    {
        // handle anonymous object
    }
    else
    {
/*
    safe_ptr<ArgumentsNode> argNode = find_node<ArgumentsNode>(node);

    ARGON_ICERR(!!argNode,
                "table node does not contains an argument node");

    ARGON_ICERR(argNode->getChilds().size() >= 2 && argNode->getChilds().size() <= 4,
                "table argument count is invalid");

    Connection* conn;

    // Find connection
    {
        Node::nodelist_type &args = argNode->getChilds();
        safe_ptr<IdNode> node = node_cast<IdNode>(args.at(0));
        conn = proc.getGlobalContext().getSymbols().find<Connection>(node->data());
	//conn = this->resolve<Connection>(node->data());
    }
*/
    }

    return new Expand(proc, args, node, mode);
/*    
    switch(conn->getEnv().getEngineType())
    {
    case informave::db::dal::DAL_ENGINE_SQLITE:
        return new TableSqlite(proc, node, mode);        
    default:
    ARGON_ICERR(false,
                "Unknown engine type in Expand::newInstance()");
    } 
*/
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
