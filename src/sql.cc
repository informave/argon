//
// sql.cc - SQL (definition)
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
/// @brief SQL (definition)
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
////////////////////////////////////////////////////////// SqlObjectChildVisitor
///
/// @since 0.0.1
/// @brief Object Child Visitor
struct SqlObjectChildVisitor : public Visitor
{
public:
    SqlObjectChildVisitor(Processor &proc, Context &context, Sql &sqlobj)
        : Visitor(Visitor::ignore_none),
          m_proc(proc),
          m_context(context),
          m_sqlobj(sqlobj)
    {}


    virtual void visit(LogNode *node)
    {
        LogCmd cmd(this->m_proc, m_context, node);
        this->m_proc.call(cmd);
    }

    virtual void visit(ArgumentsSpecNode *node)
    {
        // This visitor only handles instructions, so it's save to ignore this node.
    }

    virtual void visit(ArgumentsNode *node)
    {
        // This visitor only handles instructions, so it's save to ignore this node.
    }
    
private:
    Processor &m_proc;
    Context &m_context;
    //SourceTable      &m_sourcetable;
    Sql &m_sqlobj;
};




//..............................................................................
////////////////////////////////////////////////////// SqlObjectColAssignVisitor
///
/// @since 0.0.1
/// @brief Object Child Visitor
struct SqlObjectColAssignVisitor : public Visitor
{
public:
    SqlObjectColAssignVisitor(Processor &proc, Context &context, Sql &sqlobj)
        : Visitor(Visitor::ignore_none),
          m_proc(proc),
          m_context(context),
          m_sqlobj(sqlobj)
    {}


    virtual void visit(ColumnAssignNode *node)
    {
        assert(node->getChilds().size() == 2);
        
        //ColumnAssignOp op(this->m_proc, m_context, node);
        //this->m_proc.call(op);
        
        Column col;
        LValueColumnVisitor(this->m_proc, this->m_context, col)(node->getChilds()[0]);
        
        Value val;
        EvalExprVisitor eval(this->m_proc, this->m_context, val);
        eval(node->getChilds()[1]);

        //this->m_context.getMainObject()->setColumn(col, val);

        this->m_sqlobj.setColumn(col, val);


/*
        //Column col(dynamic_cast<ColumnNode*>(node->getChilds()[0]));
        
        Value val;
        EvalExprVisitor eval(this->m_proc, this->m_context, val);
        eval(node->getChilds()[1]);
        
        this->m_context.getDestObject()->setColumn(col, val);
*/      
        
        //this->m_context.getMainObject()->setColumn(Column("id"), Value(23));
        //this->m_context.getMainObject()->setColumn(Column("name"), Value(23));

        // ARGON_ICERR_CTX.., assign not allowed?
    }
    
private:
    Processor &m_proc;
    Context &m_context;
    //SourceTable      &m_sourcetable;
    Sql &m_sqlobj;
};




//..............................................................................
//////////////////////////////////////////////////////////////////// SourceTable

/// @details
/// 
Sql::Sql(Processor &proc, ObjectNode *node, Object::mode mode)
    : Object(proc, node),
      m_stmt(),
      m_conn(),
      m_columns(),
      m_result_columns(),
      m_objname(),
      m_mode(mode),
      m_prepost_nodes(),
      m_colassign_nodes()
{
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
}


/// @details
/// The column list is a list of column names or position numbers. These columns
/// are used in the TASK as source columns (we must build up a SQL SELECT statement)
/// or as destination columns (we must build up a SQL INSERT statement).
void
Sql::setColumnList(const ColumnList &list)
{
    this->m_columns = list;
}


/// @details
/// The column list is a list fo column names or position numbers. These columns
/// are used in the TASK as result columns (% operator).
void
Sql::setResultList(const ColumnList &list)
{
	assert(this->m_mode == Object::ADD_MODE);
    this->m_result_columns = list;
}


/// @details
/// This method can be used after a call to run() (which prepares the statement)
/// to set a column value. This values is directly bound to the statement.
void
Sql::setColumn(const Column &col, const Value &v)
{
    // removed because param binding works for both moded
	//assert(this->m_mode == Object::ADD_MODE);
    
    assert(col.mode() == Column::by_number);

    // can be used later if engine can tell name of parameters
    //int i = this->getBindPosition(col);
    //assert(i > 0);

    this->m_stmt->bind(col.getNum(), v.data());

	//this->m_column_values[col.getName()] = v;
//	this->m_column_values[c
}


/// @details
/// Returns a resultset column from the table
const db::Value&
Sql::getColumn(Column col)
{
    return col.getFrom(m_stmt->resultset(), *this);
}




/// @details
///
Value
Sql::lastInsertRowId(void)
{
	return m_stmt->lastInsertRowId();
}


/// @details
/// 
bool
Sql::next(void)
{
    m_stmt->resultset().next();

    return !m_stmt->resultset().eof();
    
    //std::cout << m_stmt->resultset().column(1) << std::endl;
}


/// @details
/// 
bool
Sql::eof(void) const
{
    return m_stmt->resultset().eof();
}





/// @details
/// 
Value
Sql::run(const ArgumentList &args)
{
    String tableName, schemaName, dbName;

    Object::run(args);

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

    ARGON_ICERR_CTX((argSpecNode->getChilds().size() == args.size()), *this,
                "argument count mismatch");

    // Prepare connection
    {
        Node::nodelist_type &args = argNode->getChilds();
        safe_ptr<IdNode> node = node_cast<IdNode>(args.at(0));
        this->m_conn = this->proc().getSymbols().find<Connection>(node->data());
    }

    // Prepare table name, schema etc.
    {
        Node::nodelist_type &args = argNode->getChilds();
        safe_ptr<LiteralNode> node;


        /// @todo only literal strings supported for now. This
        /// can be extended to identifiers later.
        node = node_cast<LiteralNode>(args.at(1));
        tableName = node->data();

        if(args.size() > 2)
        {
            node = node_cast<LiteralNode>(args.at(2));
            schemaName = node->data();
        }

        if(args.size() > 3)
        {
            node = node_cast<LiteralNode>(args.at(3));
            dbName = node->data();
        }
    }

    ARGON_ICERR_CTX(tableName.length() > 0, *this,
                "table name is empty");


    // Generate object name


    if(dbName.length() > 0)
        m_objname.append(dbName);

    if(schemaName.length() > 0)
        m_objname.append( (m_objname.length() > 0 ? String(".") : String("")) + schemaName);

    if(tableName.length() > 0)
        m_objname.append( (m_objname.length() > 0 ? String(".") : String("")) + tableName);




    String sql_query = m_objname;
    
/*
    switch(this->m_mode)
    {
    case Object::READ_MODE:
        sql_query = this->generateSelect(m_objname);
        break;
    case Object::ADD_MODE:
        sql_query = this->generateInsert(m_objname);
        break;
    default:
        throw std::runtime_error("object mode not handled");
    }
*/

/*
    String s("SELECT * FROM ");
    //String objname;

    if(dbName.length() > 0)
        objname.append(dbName);

    if(schemaName.length() > 0)
        objname.append( (objname.length() > 0 ? String(".") : String("")) + schemaName);

    if(tableName.length() > 0)
        objname.append( (objname.length() > 0 ? String(".") : String("")) + tableName);


    s.append(objname);

*/

    ARGON_DPRINT(ARGON_MOD_PROC, "RUN QUERY: " << sql_query);




    this->m_stmt.reset( m_conn->getDbc().newStatement() );
    
    //m_stmt->prepare("foo"); // invalid read, FIX THIS!
    
    m_stmt->prepare(sql_query);
    
    foreach_node( this->m_colassign_nodes, SqlObjectColAssignVisitor(this->proc(), *this, *this), 1);
    
//    if(m_mode == Object::READ_MODE)
//        m_stmt->execute();
        

    return Value();
}



void
Sql::execute(void)
{
    m_stmt->execute();
}



/// @details
/// 
String
Sql::str(void) const
{
    String s;
    //s.append(this->id().str());
    s.append("[TASK]");
    return s;
}


/// @details
/// 
String
Sql::name(void) const
{
    String s = this->id().str();
    return s.empty() ? "<anonymous>" : s;
}


/// @details
/// 
String
Sql::type(void) const
{
    return "TABLE";
}


/// @details
/// 
SourceInfo
Sql::getSourceInfo(void) const
{
    assert(this->m_node);
    return this->m_node->getSourceInfo();
}


/// @details
/// 
Value
Sql::_value(void) const
{
    return String("TABLE");
}

/// @details
/// 
String
Sql::_string(void) const
{
    return this->_value().data().asStr();
}

/// @details
/// 
String
Sql::_name(void) const
{
    return this->id().str() + String(" (table)");
}

/// @details
/// 
String
Sql::_type(void) const
{
    return "TABLE";
}


/// @details
/// 
Sql*
Sql::newInstance(Processor &proc, SqlNode *node, Object::mode mode)
{
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
        conn = proc.getSymbols().find<Connection>(node->data());
    }


    return new Sql(proc, node, mode);
/*    
    switch(conn->getEnv().getEngineType())
    {
    case informave::db::dal::DAL_ENGINE_SQLITE:
        return new TableSqlite(proc, node, mode);        
    default:
    ARGON_ICERR(false,
                "Unknown engine type in Sql::newInstance()");
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
