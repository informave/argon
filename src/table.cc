//
// table.cc - Table (definition)
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
/// @brief Table (definition)
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
///////////////////////////////////////////////////////////// ObjectChildVisitor
///
/// @since 0.0.1
/// @brief Object Child Visitor
struct ObjectChildVisitor : public Visitor
{
public:
    ObjectChildVisitor(Processor &proc, Context &context, /*SourceTable*/ Object &task)
        : Visitor(Visitor::ignore_none),
          m_proc(proc),
          m_context(context),
          m_sourcetable(task)
    {}


    virtual void visit(LogNode *node)
    {
        LogCmd cmd(this->m_proc, m_context, node);
        this->m_proc.call(cmd);
    }

    virtual void visit(ArgumentsSpecNode *node)
    {
        /// @bug it seems we must do nothing here, because tcv is only
        /// for looping thorugh instruction commands.
    }

    virtual void visit(ArgumentsNode *node)
    {
        /// @bug it seems we must do nothing here, because tcv is only
        /// for looping thorugh instruction commands.
    }


private:
    Processor &m_proc;
    Context &m_context;
    //SourceTable      &m_sourcetable;
    Object &m_sourcetable;
};




//..............................................................................
//////////////////////////////////////////////////////////////////// SourceTable

/// @details
/// 
Table::Table(Processor &proc, ObjectNode *node, Object::mode mode)
    : Object(proc, node),
      m_stmt(),
      m_conn(),
      m_columns(),
      m_result_columns(),
      m_objname(),
      m_mode(mode)
{}


/// @details
/// The column list is a list of column names or position numbers. These columns
/// are used in the TASK as source columns (we must build up a SQL SELECT statement)
/// or as destination columns (we must build up a SQL INSERT statement).
void
Table::setColumnList(const ColumnList &list)
{
    this->m_columns = list;
}


/// @details
/// The column list is a list fo column names or position numbers. These columns
/// are used in the TASK as result columns (% operator).
void
Table::setResultList(const ColumnList &list)
{
	assert(this->m_mode == Object::ADD_MODE);
    this->m_result_columns = list;
}


/// @details
/// This method can be used after a call to run() (which prepares the statement)
/// to set a column value. This values is directly bound to the statement.
void
Table::setColumn(const Column &col, const Value &v)
{
	assert(this->m_mode == Object::ADD_MODE);
    int i = this->getBindPosition(col);

    assert(i > 0);

    this->m_stmt->bind(i, v.data());

	//this->m_column_values[col.getName()] = v;
//	this->m_column_values[c
}


/// @details
/// Returns a resultset column from the table
const db::Value&
Table::getColumn(Column col)
{
    return col.getFrom(m_stmt->resultset(), *this);
}




/// @details
///
Value
Table::lastInsertRowId(void)
{
	return m_stmt->lastInsertRowId();
}


/// @details
/// 
bool
Table::next(void)
{
    m_stmt->resultset().next();

    return !m_stmt->resultset().eof();
    
    //std::cout << m_stmt->resultset().column(1) << std::endl;
}


/// @details
/// 
bool
Table::eof(void) const
{
    return m_stmt->resultset().eof();
}





/// @details
/// 
Value
Table::run(const ArgumentList &args)
{
    String tableName, schemaName, dbName;

    Object::run(args);

    foreach_node( this->m_node->getChilds(), ObjectChildVisitor(this->proc(), *this, *this), 1);

    safe_ptr<ArgumentsNode> argNode = find_node<ArgumentsNode>(this->m_node);

    safe_ptr<ArgumentsSpecNode> argSpecNode = find_node<ArgumentsSpecNode>(this->m_node);
    
    ARGON_ICERR_CTX(!!argNode, *this,
                "table node does not contains an argument node");

    ARGON_ICERR_CTX(argNode->getChilds().size() >= 2 && argNode->getChilds().size() <= 4, *this,
                "table argument count is invalid");

    /// @bug add argSpecNode !! test

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




    String sql_query;
    

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
    
    
//    if(m_mode == Object::READ_MODE)
//        m_stmt->execute();
        
    //this->setColumn(Column(1), Value(23)); /// @bug just for debugging

    return Value();
}


String
Table::generateSelect(String objname)
{
    String s("SELECT * FROM ");

    s.append(objname);

    return s;
}




int
Object::getBindPosition(const Column &col)
{
    std::map<Column, int>::iterator i = this->m_column_mappings.find(col);
    if(i == this->m_column_mappings.end())
        throw std::runtime_error(String("unknown column: ") + col.getName());
    else
        return i->second;
}

void
Object::addBindPosition(const Column &col, int pos)
{
    this->m_column_mappings[col] = pos;
}



String
Table::generateInsert(String objname)
{
    String s("INSERT INTO ");
    s.append(objname);

    //this->m_column_values["number"] = Value(23);
    //this->m_column_values["value"] = Value(String("foo"));

	assert(! this->m_columns.empty());

	String column_list, param_list;

    int bindpos = 1;

    for(ColumnList::iterator i = this->m_columns.begin();
    	i != this->m_columns.end();
        ++i)
	{
		if(! column_list.empty())
			column_list.append(", ");
		column_list.append(i->getName()); /// @bug
		if(! param_list.empty())
			param_list.append(", ");
		param_list.append("?");
        this->addBindPosition(*i, bindpos++);
	}

    s.append("(");
    s.append(column_list);
    s.append(") VALUES(");
    s.append(param_list);
    s.append(")");



    return s;
}


void
Table::execute(void)
{
    m_stmt->execute(); /// @bug do same as desttable
}



/// @details
/// 
String
Table::str(void) const
{
    String s;
    //s.append(this->id().str());
    s.append("[TASK]");
    return s;
}


/// @details
/// 
String
Table::name(void) const
{
    String s = this->id().str();
    return s.empty() ? "<anonymous>" : s;
}


/// @details
/// 
String
Table::type(void) const
{
    return "TABLE";
}


/// @details
/// 
SourceInfo
Table::getSourceInfo(void) const
{
    assert(this->m_node);
    return this->m_node->getSourceInfo();
}


/// @details
/// 
Value
Table::_value(void) const
{
    return String("TABLE");
}

/// @details
/// 
String
Table::_string(void) const
{
    return this->_value().data().asStr();
}

/// @details
/// 
String
Table::_name(void) const
{
    return this->id().str() + String(" (table)");
}

/// @details
/// 
String
Table::_type(void) const
{
    return "TABLE";
}

/// @details
/// 
/// @bug Check connection type
Table*
Table::newInstance(Processor &proc, TableNode *node, Object::mode mode)
{
    return new TableSqlite(proc, node, mode);
}



/*



//..............................................................................
////////////////////////////////////////////////////////////////////// DestTable

/// @details
/// 
DestTable::DestTable(Processor &proc, ObjectNode *node)
    : Object(proc, node),
      m_stmt(),
      m_column_values(),
      m_columns()
{}



/// @details
/// 
void
DestTable::setColumnList(const ColumnList &list)
{
    this->m_columns = list;
}



int
Object::getBindPosition(const Column &col)
{
    std::map<Column, int>::iterator i = this->m_column_mappings.find(col);
    if(i == this->m_column_mappings.end())
        throw std::runtime_error(String("unknown column: ") + col.getName());
    else
        return i->second;
}

void
Object::addBindPosition(const Column &col, int pos)
{
    this->m_column_mappings[col] = pos;
}



/// @details
/// 
void
DestTable::setColumn(Column col, Value v)
{
    int i = this->getBindPosition(col);

    assert(i > 0);

    this->m_stmt->bind(i, v.data());

	//this->m_column_values[col.getName()] = v;
//	this->m_column_values[c
}


/// @details
/// 
bool
DestTable::eof(void) const
{
    return m_stmt->resultset().eof();
}


/// @details
/// 
bool
DestTable::next(void)
{
    //m_stmt->resultset().next();

    //return !m_stmt->resultset().eof();
    //
    return false;
    
    //std::cout << m_stmt->resultset().column(1) << std::endl;
}


/// @details
/// 
const db::Value&
DestTable::getColumn(Column col)
{

    // if(Column::by_number == col.mode())
    //     return m_stmt->resultset().column(col.getNum());
    // else
    //     return m_stmt->resultset().column(col.getName());

	throw std::runtime_error("getcolumn");
}


/// @details
/// 
Value
DestTable::run(const ArgumentList &args)
{
    String tableName, schemaName, dbName;
    Connection *conn;

    Object::run(args);

    foreach_node( this->m_node->getChilds(), ObjectChildVisitor(this->proc(), *this, *this), 1);

    safe_ptr<ArgumentsNode> argNode = find_node<ArgumentsNode>(this->m_node);

    safe_ptr<ArgumentsSpecNode> argSpecNode = find_node<ArgumentsSpecNode>(this->m_node);
    
    ARGON_ICERR_CTX(!!argNode, *this,
                "table node does not contains an argument node");

    ARGON_ICERR_CTX(argNode->getChilds().size() >= 2 && argNode->getChilds().size() <= 4, *this,
                "table argument count is invalid");

    /// @bug add argSpecNode !! test

    ARGON_ICERR_CTX((argSpecNode->getChilds().size() == args.size()), *this,
                "argument count mismatch");

    // Prepare connection
    {
        Node::nodelist_type &args = argNode->getChilds();
        safe_ptr<IdNode> node = node_cast<IdNode>(args.at(0));
        conn = this->proc().getSymbols().find<Connection>(node->data());
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


    String s("INSERT INTO ");
    String objname;

    if(dbName.length() > 0)
        objname.append(dbName);

    if(schemaName.length() > 0)
        objname.append( (objname.length() > 0 ? String(".") : String("")) + schemaName);

    if(tableName.length() > 0)
        objname.append( (objname.length() > 0 ? String(".") : String("")) + tableName);


    s.append(objname);

    //this->m_column_values["number"] = Value(23);
    //this->m_column_values["value"] = Value(String("foo"));

	assert(! this->m_columns.empty());

	String column_list, param_list;

    int bindpos = 1;

    for(ColumnList::iterator i = this->m_columns.begin();
    	i != this->m_columns.end();
        ++i)
	{
		if(! column_list.empty())
			column_list.append(", ");
		column_list.append(i->getName()); /// @bug
		if(! param_list.empty())
			param_list.append(", ");
		param_list.append("?");
        this->addBindPosition(*i, bindpos++);
	}

    s.append("(");
    s.append(column_list);
    s.append(") VALUES(");
    s.append(param_list);
    s.append(")");

	std::cout << s << std::endl;

    ARGON_DPRINT(ARGON_MOD_PROC, "RUN INSERT: " << s);


    this->m_stmt.reset( conn->getDbc().newStatement() );
    
    //m_stmt->prepare("foo"); // invalid read, FIX THIS!
    
    m_stmt->prepare(s);
    
    
        
    //this->setColumn(Column(1), Value(23)); /// @bug just for debugging

    return Value();
}

void
DestTable::execute(void)
{
    m_stmt->execute();
}


/// @details
/// 
String
DestTable::str(void) const
{
    String s;
    //s.append(this->id().str());
    s.append("[TASK]");
    return s;
}


/// @details
/// 
String
DestTable::name(void) const
{
    String s = this->id().str();
    return s.empty() ? "<anonymous>" : s;
}


/// @details
/// 
String
DestTable::type(void) const
{
    return "TABLE";
}


/// @details
/// 
SourceInfo
DestTable::getSourceInfo(void) const
{
    assert(this->m_node);
    return this->m_node->getSourceInfo();
}



/// @details
/// 
Value
DestTable::_value(void) const
{
    return String("TABLE");
}

/// @details
/// 
String
DestTable::_string(void) const
{
    return this->_value().data().asStr();
}

/// @details
/// 
String
DestTable::_name(void) const
{
    return this->id().str() + String(" (table)");
}

/// @details
/// 
String
DestTable::_type(void) const
{
    return "TABLE";
}
*/


ARGON_NAMESPACE_END


//
// Local Variables:
// mode: C++
// c-file-style: "bsd"
// c-basic-offset: 4
// indent-tabs-mode: nil
// End:
//
