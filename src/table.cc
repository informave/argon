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
    ObjectChildVisitor(Processor &proc, Context &context, SourceTable &task)
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
    SourceTable      &m_sourcetable;
};




//..............................................................................
//////////////////////////////////////////////////////////////////// SourceTable

/// @details
/// 
SourceTable::SourceTable(Processor &proc, ObjectNode *node)
    : Object(proc, node),
      m_stmt()
{}


/// @details
/// 
void
SourceTable::setColumn(Column col, Value v)
{
}


/// @details
/// 
bool
SourceTable::next(void)
{
    m_stmt->resultset().next();

    return !m_stmt->resultset().eof();
    
    //std::cout << m_stmt->resultset().column(1) << std::endl;
}


/// @details
/// 
bool
SourceTable::eof(void) const
{
    return m_stmt->resultset().eof();
}


/// @details
/// 
const db::Value&
SourceTable::getColumn(Column col)
{
    return col.getFrom(m_stmt->resultset(), *this);
}


/// @details
/// 
Value
SourceTable::run(const ArgumentList &args)
{
    String tableName, schemaName, dbName;
    Connection *conn;

    Object::run(args);

    foreach_node( this->m_node->getChilds(), ObjectChildVisitor(this->proc(), *this, *this), 1);

    safe_ptr<ArgumentsNode> argNode = find_node<ArgumentsNode>(this->m_node);

    safe_ptr<ArgumentsSpecNode> argSpecNode = find_node<ArgumentsSpecNode>(this->m_node);
    
    ARGON_ICERR(!!argNode, *this,
                "table node does not contains an argument node");

    ARGON_ICERR(argNode->getChilds().size() >= 2 && argNode->getChilds().size() <= 4, *this,
                "table argument count is invalid");

    /// @bug add argSpecNode !! test

    ARGON_ICERR((argSpecNode->getChilds().size() == args.size()), *this,
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

    ARGON_ICERR(tableName.length() > 0, *this,
                "table name is empty");


    String s("SELECT * FROM ");
    s.append(tableName);


    ARGON_DPRINT(ARGON_MOD_PROC, "RUN SELECT: " << s);




    this->m_stmt.reset( conn->getDbc().newStatement() );
    
    //m_stmt->prepare("foo"); // invalid read, FIX THIS!
    
    m_stmt->prepare(s);
    
    m_stmt->execute();
        
    this->setColumn(Column(1), Value(23)); /// @bug just for debugging

    return Value();
}


/// @details
/// 
String
SourceTable::str(void) const
{
    String s;
    //s.append(this->id().str());
    s.append("[TASK]");
    return s;
}


/// @details
/// 
String
SourceTable::name(void) const
{
    String s = this->id().str();
    return s.empty() ? "<anonymous>" : s;
}


/// @details
/// 
String
SourceTable::type(void) const
{
    return "TABLE";
}


/// @details
/// 
SourceInfo
SourceTable::getSourceInfo(void) const
{
    assert(this->m_node);
    return this->m_node->getSourceInfo();
}





//..............................................................................
////////////////////////////////////////////////////////////////////// DestTable

/// @details
/// 
DestTable::DestTable(Processor &proc, ObjectNode *node)
    : Object(proc, node),
      m_stmt()
{}


/// @details
/// 
void
DestTable::setColumn(Column col, Value v)
{
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
    m_stmt->resultset().next();

    return !m_stmt->resultset().eof();
    
    //std::cout << m_stmt->resultset().column(1) << std::endl;
}


/// @details
/// 
const db::Value&
DestTable::getColumn(Column col)
{
    if(Column::by_number == col.mode())
        return m_stmt->resultset().column(col.getNum());
    else
        return m_stmt->resultset().column(col.getName());
}


/// @details
/// 
Value
DestTable::run(const ArgumentList &args)
{
    return Value();
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

ARGON_NAMESPACE_END


//
// Local Variables:
// mode: C++
// c-file-style: "bsd"
// c-basic-offset: 4
// indent-tabs-mode: nil
// End:
//
