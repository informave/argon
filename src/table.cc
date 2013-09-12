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
#include "table_firebird.hh"

#include <iostream>
#include <sstream>
#include <list>

ARGON_NAMESPACE_BEGIN


//..............................................................................
//////////////////////////////////////////////////////// TableObjectChildVisitor
///
/// @since 0.0.1
/// @brief Object Child Visitor
struct TableObjectChildVisitor : public Visitor
{
public:
    TableObjectChildVisitor(Processor &proc, Context &context, /*SourceTable*/ Object &task)
        : Visitor(Visitor::ignore_none),
          m_proc(proc),
          m_context(context),
          m_sourcetable(task)
    {}

    virtual void visit(IdNode *node)
    {
        // The ID node refers to the object base.
    }

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
    Object &m_sourcetable;
};




//..............................................................................
//////////////////////////////////////////////////////////////////// SourceTable

/// @details
/// 
Table::Table(Processor &proc, const ArgumentList &args, DeclNode *node, Type::mode_t mode)
    : Object(proc, node, args),
      m_stmt(),
      m_conn(),
      m_columns(),
      m_result_columns(),
      m_objname(),
      m_mode(mode)
{}


Table::~Table(void)
{
    if(this->m_stmt.get())
        this->m_stmt->close();
}

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
    ARGON_ICERR(this->m_mode == Type::INSERT_MODE, "invalid mode");
    this->m_result_columns = list;
}


/// @details
/// This method can be used after a call to run() (which prepares the statement)
/// to set a column value. This values is directly bound to the statement.
void
Table::setColumn(const Column &col, const Value &v)
{
    ARGON_ICERR(this->m_mode == Type::INSERT_MODE, "invalid mode");
    int i = this->getBindPosition(col);

    ARGON_ICERR(i > 0, "invalid bind position");

    this->m_stmt->bind(i, v.data());

    //this->m_column_values[col.getName()] = v;
//  this->m_column_values[c
}


/// @details
/// Returns a resultset column from the table
const db::Value&
Table::getColumn(Column col)
{
    if(this->m_mode == Type::INSERT_MODE)
    {
        // we assume that that SQL was something like SELECT * FROM X RETURNING A, B
        if(this->m_result_columns.size() > 0)
        {
            return col.getFrom(m_stmt->resultset(), *this);
        }
        else
        {
            ARGON_ICERR(false, "getColumn() bug");
        }
    }
    else
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
void
Table::first(void)
{
    m_stmt->resultset().first();
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
Table::run(void)
{
    String tableName, schemaName, dbName;

    Object::run();

    if(!this->m_node)
    {
        ArgumentList::const_iterator i = this->getCallArgs().begin();

        this->m_conn = i->cast<Connection>();

        ++i; /// @bug no check for end()

        /// @todo only literal strings supported for now. This
        /// can be extended to identifiers later.

        
        tableName = (*i++)->_value().data().asStr();
        if(i != this->getCallArgs().end())
        {
            schemaName = (*i++)->_value().data().asStr();
        }

    

/*
  try
  {
  node = node_cast<LiteralNode>(args.at(1));
  tableName = node->data();
  }
  catch(...)
  {
  IdNode *node = node_cast<IdNode>(args.at(1));
  tableName = this->getSymbols().find<Element>(node->data())->_value().data().asStr();
  }

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
*/
    }
    else
    {

        foreach_node( this->m_node->getChilds(), TableObjectChildVisitor(this->proc(), *this, *this), 1);

        safe_ptr<ArgumentsNode> argNode = find_node<ArgumentsNode>(this->m_node);
    
        ARGON_ICERR_CTX(!!argNode, *this,
                        "table node does not contains an argument node");

        ARGON_ICERR_CTX(argNode->getChilds().size() >= 2 && argNode->getChilds().size() <= 4, *this,
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
            /// @bug Needs review
            //this->m_conn = this->proc().getSymbols().find<Connection>(node->data());
            this->m_conn = this->getSymbols().find<Connection>(node->data());
        }



        // Prepare table name, schema etc.
        {
            Node::nodelist_type &args = argNode->getChilds();
            safe_ptr<LiteralNode> node;


            /// @todo only literal strings supported for now. This
            /// can be extended to identifiers later.

            try
            {
                node = node_cast<LiteralNode>(args.at(1));
                tableName = node->data();
            }
            catch(...)
            {
                IdNode *node = node_cast<IdNode>(args.at(1));
                tableName = this->getSymbols().find<Element>(node->data())->_value().data().asStr();
            }

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
    case Type::READ_MODE:
        sql_query = this->generateSelect(m_objname);
        break;
    case Type::INSERT_MODE:
        sql_query = this->generateInsert(m_objname);
        break;
    default:
        throw std::runtime_error("object mode not handled");
    }


    this->m_stmt.reset( m_conn->getDbc().newStatement() );
    
    try
    {
        ARGON_DPRINT(ARGON_MOD_PROC, "RUN QUERY: " << sql_query);
        m_stmt->prepare(sql_query);
    }
    catch(informave::db::ex::exception &e)
    {
        /// @bug Workaround, we need a ARGON_ERR_CTX(*this, e) ma
        ARGON_ICERR_CTX(false, *this, e.what());
    }
    
    return Value();
}


String
Table::generateSelect(String objname)
{
    String s("SELECT * FROM ");

    s.append(objname);

    return s;
}




String
Table::generateInsert(String objname)
{
    String s("INSERT INTO ");
    s.append(objname);

    //this->m_column_values["number"] = Value(23);
    //this->m_column_values["value"] = Value(String("foo"));

    ARGON_ICERR(! this->m_columns.empty(), "columns empty");

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
    m_stmt->execute();
    m_stmt->resultset().first();
    //assert(! m_stmt->resultset().eof());
    //std::cout << "Data:" << m_stmt->resultset().column("NAME") << std::endl;
}



/// @details
/// 
String
Table::str(void) const
{
    String s;
    //s.append(this->id().str());
    s.append("[TABLE]");
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
    ARGON_ICERR(this->m_node, "invalid node");
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
Table*
Table::newInstance(Processor &proc, const ArgumentList &args, Connection *dbc, DeclNode *node, Type::mode_t mode)
{
    ARGON_ICERR(dbc, "invalid dbc");
   

    switch(dbc->getEnv().getEngineType())
    {
    case informave::db::DAL_ENGINE_SQLITE:
        return new TableSqlite(proc, args, node, mode);
    case informave::db::DAL_ENGINE_FIREBIRD:
        return new TableFirebird(proc, args, node, mode);
    default:
        return new Table(proc, args, node, mode);
    }
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
