//
// table_sqlite.cc - Sqlite table (definition)
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
/// @brief Sqlite table (definition)
/// @author Daniel Vogelbacher
/// @since 0.1

#include "table_sqlite.hh"

#include <iostream>
#include <sstream>
#include <list>

ARGON_NAMESPACE_BEGIN




TableSqlite::TableSqlite(Processor &proc, const ArgumentList &args, DeclNode *node, Type::mode_t mode)
    : Table(proc, args, node, mode),
      m_result_stmt()
{}


const db::Value&
TableSqlite::getColumn(Column col)
{
    if(this->m_mode == Type::INSERT_MODE)
    {
    	assert(this->m_result_stmt.get());
        return col.getFrom(this->m_result_stmt->resultset(), *this);
    }
    else
        return Table::getColumn(col);
}



Value
TableSqlite::run(void)
{
    Value v = Table::run();

    // prepare

    // only if there are result columns requested
    if(this->m_mode == Type::INSERT_MODE && this->m_result_columns.size() > 0)
    { 
        this->m_result_stmt.reset( m_conn->getDbc().newStatement() );
        
        String q;
        q.append("SELECT * FROM ");
        q.append(m_objname);
        q.append(" WHERE _ROWID_ = ?");
        
        this->m_result_stmt->prepare(q);
    }
    
    return v;
}



void
TableSqlite::execute(void)
{
    Table::execute();

    if(this->m_mode == Type::INSERT_MODE &&  this->m_result_columns.size() > 0)
    {
        this->m_result_stmt->bind(1, this->lastInsertRowId().data());
        this->m_result_stmt->execute();
    }
}

/*
bool
TableSqlite::next(void)
{
    if(this->m_mode == Object::ADD_MODE)
    {
        if(Table::next())
        {
            this->m_result_stmt->bind(1, this->lastInsertRowId().data());
            this->m_result_stmt->execute();
            return true;
        }
        
        return false;
    };
    return Table::next();
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
