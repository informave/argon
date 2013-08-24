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


typedef informave::db::IDbc::Options dbcopts_t;

//..............................................................................
//////////////////////////////////////////////////////////// SemanticCheckRunner
///
/// @since 0.0.1
/// @brief Semantic checker
class KeyValueCreator : public Visitor
{
public:
    KeyValueCreator(dbcopts_t &opts)
        : Visitor(ignore_none),
          m_opts(opts)
    {}


    virtual void visit(KeyValueNode *node)
    {
        ARGON_ICERR(node->getChilds().size() == 2, "Invalid child count");

        LiteralNode *key = node_cast<LiteralNode>(node->getChilds()[0]);
        LiteralNode *value = node_cast<LiteralNode>(node->getChilds()[1]);

        m_opts[key->data()] = value->data();
    }

protected:
/*
    virtual void fallback_action(Node *node)
    {
        node->semanticCheck(m_check);
    }
*/
    dbcopts_t &m_opts;
};



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
    ARGON_DPRINT(ARGON_MOD_PROC, "processing connection " << node->data());

    if(userConns[node->data()])
    {
        ARGON_DPRINT(ARGON_MOD_PROC, "Using user-supplied connection " << node->data());
        this->m_dbc = userConns[node->data()];
    }
    else
    {

        if(node->getChilds().size() == 0)
        {
            throw std::runtime_error("no dbc type given");
        }

        informave::db::IDbc::Options opts;

        foreach_node(node->getChilds(), KeyValueCreator(opts), 1); // for each node

//        apply_visitor(node->getChilds(), KeyValueCreator(opts));

        ARGON_ICERR(opts["engine"].length() > 0, "Engine not set");

//        for(dbcopts_t::iterator i = opts.begin(); i != opts.end(); ++i)
//            std::cout << i->first << " " << i->second << std::endl;


        this->m_alloc_env.reset(new db::Database::Environment(opts["engine"]));
        this->m_alloc_dbc.reset(this->m_alloc_env->newConnection());
        this->m_dbc = this->m_alloc_dbc.get();
        this->m_dbc->connect(opts);
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
    ARGON_ICERR(this->m_dbc, "dbc not set");
    return *this->m_dbc;
}


/// @details
/// 
db::Env&
Connection::getEnv(void)
{
	return this->m_dbc->getEnv();
    //assert(this->m_alloc_env.get());
    //return *this->m_alloc_env.get();
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
