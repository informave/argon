//
// genrange.cc - GENRANGE (definition)
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
/// @brief GENRANGE (definition)
/// @author Daniel Vogelbacher
/// @since 0.1

#include "argon/dtsengine.hh"
#include "argon/exceptions.hh"
#include "argon/ast.hh"
#include "debug.hh"
#include "visitors.hh"

#include "table_sqlite.hh"

#include <iostream>
#include <sstream>
#include <list>

ARGON_NAMESPACE_BEGIN



//..............................................................................
/////////////////////////////////////////////////////////////////////// GenRange

/// @details
/// 
GenRange::GenRange(Processor &proc, const ArgumentList &args, DeclNode *node, Type::mode_t mode)
    : Object(proc, node, args),
      m_objname(),
      m_start(0),
      m_stop(0),
      m_step(1),
      m_mode(mode)
      //m_prepost_nodes(),
      //m_colassign_nodes()
{

    if(node)
    {
/*
    NodeList childs = node->getChilds();

    //assert(childs.size() >= 2);

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
    //assert(c > childs.size());
*/
    }
}


/// @details
/// The column list is a list of column names or position numbers. These columns
/// are used in the TASK as source columns (we must build up a SQL SELECT statement)
/// or as destination columns (we must build up a SQL INSERT statement).
void
GenRange::setColumnList(const ColumnList &list)
{
    //this->m_columns = list;
}


/// @details
/// The column list is a list fo column names or position numbers. These columns
/// are used in the TASK as result columns (% operator).
void
GenRange::setResultList(const ColumnList &list)
{
	//assert(this->m_mode == Type::INSERT_MODE);
    //this->m_result_columns = list;
}


/// @details
/// This method can be used after a call to run() (which prepares the statement)
/// to set a column value. This values is directly bound to the statement.
void
GenRange::setColumn(const Column &col, const Value &v)
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
GenRange::getColumn(Column col)
{
    return this->m_start;
    //return col.getFrom(m_stmt->resultset(), *this);
}




/// @details
///
Value
GenRange::lastInsertRowId(void)
{
	return -1;
}


/// @details
/// 
bool
GenRange::next(void)
{
    //m_stmt->resultset().next();

    this->m_start = this->m_start.get<int>() + this->m_step;
    
    return (this->m_start.get<int>() <= this->m_stop);

    //return !m_stmt->resultset().eof();
    
    //std::cout << m_stmt->resultset().column(1) << std::endl;
}

/// @details
/// 
void
GenRange::first(void)
{
	// nothing to do
}

/// @details
/// 
bool
GenRange::eof(void) const
{
    return !(this->m_start.get<int>() <= this->m_stop);
}





/// @details
/// 
Value
GenRange::run(void)
{
    Object::run();

    if(this->m_node == NULL_NODE)
    {
        ARGON_ICERR_CTX(this->getCallArgs().size() >= 2, *this,
                        "gen_range requires >= 2 arguments");

        ArgumentList::const_iterator i = this->getCallArgs().begin();
        ArgumentList::const_iterator end = this->getCallArgs().end();
        
        if(i != end) this->m_start = (i++)->cast<ValueElement>()->getValue().data().get<int>();
        if(i != end) this->m_stop = (i++)->cast<ValueElement>()->getValue().data().get<int>();
        if(i != end) this->m_step = (i++)->cast<ValueElement>()->getValue().data().get<int>();
    }
    else
    {

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
GenRange::execute(void)
{
    //m_stmt->execute();
}



/// @details
/// 
String
GenRange::str(void) const
{
    String s;
    //s.append(this->id().str());
    s.append("[GEN_RANGE]");
    return s;
}


/// @details
/// 
String
GenRange::name(void) const
{
    String s = this->id().str();
    return s.empty() ? "<anonymous>" : s;
}


/// @details
/// 
String
GenRange::type(void) const
{
    return "GEN_RANGE";
}


/// @details
/// 
SourceInfo
GenRange::getSourceInfo(void) const
{
    ARGON_ICERR(this->m_node, "invalid node");
    return this->m_node->getSourceInfo();
}


/// @details
/// 
Value
GenRange::_value(void) const
{
    return String("GEN_RANGE");
}

/// @details
/// 
String
GenRange::_string(void) const
{
    return this->_value().data().asStr();
}

/// @details
/// 
String
GenRange::_name(void) const
{
    return this->id().str() + String(" (gen_range)");
}

/// @details
/// 
String
GenRange::_type(void) const
{
    return "GEN_RANGE";
}


/// @details
/// 
GenRange*
GenRange::newInstance(Processor &proc, const ArgumentList &args, DeclNode *node, Type::mode_t mode)
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

    return new GenRange(proc, args, node, mode);
/*    
    switch(conn->getEnv().getEngineType())
    {
    case informave::db::dal::DAL_ENGINE_SQLITE:
        return new TableSqlite(proc, node, mode);        
    default:
    ARGON_ICERR(false,
                "Unknown engine type in GenRange::newInstance()");
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
