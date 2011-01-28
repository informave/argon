//
// table_sqlite.hh - SQLite table
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
/// @brief SQLite table
/// @author Daniel Vogelbacher
/// @since 0.1

#ifndef INFORMAVE_ARGON_TABLE_SQLITE_HH
#define INFORMAVE_ARGON_TABLE_SQLITE_HH

#include "argon/dtsengine.hh"
#include "argon/fwd.hh"
#include "argon/ast.hh"
#include "argon/token.hh"

#include <iterator>
#include <map>
#include <deque>
#include <vector>
#include <list>
#include <set>

#include <dbwtl/dbobjects>
#include <dbwtl/dal/engines/sqlite>




ARGON_NAMESPACE_BEGIN



class TableSqlite : public Table
{
public:
    TableSqlite(Processor &proc, ObjectNode *node, Object::mode mode); // change node

    virtual ~TableSqlite(void)
    {}

    virtual Value run(const ArgumentList &args);


    virtual void execute(void);


    virtual const db::Value& getColumn(Column col);

protected:
    

    db::Stmt::ptr  m_result_stmt;

};


ARGON_NAMESPACE_END


#endif

//
// Local Variables:
// mode: C++
// c-file-style: "bsd"
// c-basic-offset: 4
// indent-tabs-mode: nil
// End:
//
