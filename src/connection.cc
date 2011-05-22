//
// connection.cc - Connection (definition)
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
/// @brief Connection (definition)
/// @author Daniel Vogelbacher
/// @since 0.1

#include "argon/dtsengine.hh"
#include "argon/exceptions.hh"
#include "argon/ast.hh"
#include "debug.hh"

#include <iostream>
#include <sstream>
#include <list>

ARGON_NAMESPACE_BEGIN



//..............................................................................
///////////////////////////////////////////////////////////////////// Connection

/// @details
/// 
Connection::Connection(Processor &proc, ConnNode *node, db::ConnectionMap &userConns)
    : Element(proc),
      m_node(node),
      m_dbc(0),
      m_alloc_env(),
      m_alloc_dbc()
{
    ARGON_DPRINT(ARGON_MOD_PROC, "processing connection " << node->id);

    if(userConns[node->id])
    {
        ARGON_DPRINT(ARGON_MOD_PROC, "Using user-supplied connection " << node->id);
        this->m_dbc = userConns[node->id];
    }
    else
    {
        if(node->spec->type.empty())
        {
            throw std::runtime_error("no dbc type given");
        }
        this->m_alloc_env.reset(new db::Database::Environment(node->spec->type));
        this->m_alloc_dbc.reset(this->m_alloc_env->newConnection());
        this->m_dbc = this->m_alloc_dbc.get();
        this->m_dbc->connect(node->spec->dbcstr);
    }
}


/// @details
/// 
String
Connection::str(void) const
{
    String s;
    s.append(this->id().str());
    s.append("[CONNECTION]");
    return s;
}


/// @details
/// 
SourceInfo
Connection::getSourceInfo(void) const
{
    return this->m_node->getSourceInfo();
}


/// @details
/// 
String
Connection::name(void) const
{
    return this->id().str();
}


/// @details
/// 
String
Connection::type(void) const
{
    return "CONNECTION";
}


/// @details
/// 
db::Connection&
Connection::getDbc(void)
{
    assert(this->m_dbc);
    return *this->m_dbc;
}


/// @details
/// 
db::Env&
Connection::getEnv(void)
{
    assert(this->m_alloc_env.get());
    return *this->m_alloc_env.get();
}


/// @details
/// 
Value
Connection::_value(void) const
{
    return this->_name();
}

/// @details
/// 
String
Connection::_string(void) const
{
    return this->_value().data().asStr();
}

/// @details
/// 
String
Connection::_name(void) const
{
    return this->id().str() + String(" (connection)");
}

/// @details
/// 
String
Connection::_type(void) const
{
    return "CONNECTION";
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
