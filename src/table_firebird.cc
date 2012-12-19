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

#include "table_firebird.hh"

#include <iostream>
#include <sstream>
#include <list>

ARGON_NAMESPACE_BEGIN




TableFirebird::TableFirebird(Processor &proc, const ArgumentList &args, DeclNode *node, Type::mode_t mode)
    : Table(proc, args, node, mode)
{}





String
TableFirebird::generateInsert(String objname)
{
    String s = Table::generateInsert(objname);
    
    String rescol_list;

    for(ColumnList::iterator i = this->m_result_columns.begin();
    	i != this->m_result_columns.end();
        ++i)
	{
		if(! rescol_list.empty())
			rescol_list.append(", ");
		rescol_list.append(i->getName()); /// @bug
	}

    if(rescol_list.length())
    {
        s += String(" RETURNING ") + rescol_list;
    }

    std::cerr << s << std::endl;

    return s;
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
