//
// dtsengine.cc - DTS Engine (definition)
//
// Copyright (C)         informave.org
//   2010,               Daniel Vogelbacher <daniel@vogelbacher.name>
// 
// GPL 3.0 License
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

/// @file
/// @brief DTS Engine and elements (definition)
/// @author Daniel Vogelbacher
/// @since 0.1


///
/// @mainpage
/// @section intro What is the Argon library?
/// Nulla a porta lacus. Nunc neque elit, rutrum at scelerisque et,
/// fermentum vel nisl. Pellentesque faucibus scelerisque
/// imperdiet. Curabitur tristique egestas odio a ultricies. Sed risus
/// massa, vulputate sit amet semper vitae, gravida ac
/// metus. Phasellus erat sem, dapibus sed adipiscing pellentesque,
/// mattis consequat velit. Vivamus suscipit nisl non urna suscipit
/// rutrum. Integer lacinia pulvinar pulvinar. Cras non cursus
/// felis. Nulla vitae blandit ante. Curabitur dapibus dapibus est id
/// imperdiet. Donec id pretium lacus. Cras nec dictum orci.
///
/// @section install Installation
///
/// Nulla a porta lacus. Nunc neque elit, rutrum at scelerisque et,
/// fermentum vel nisl. Pellentesque faucibus scelerisque
/// imperdiet. Curabitur tristique egestas odio a ultricies. Sed risus
/// massa, vulputate sit amet semper vitae, gravida ac
/// metus. Phasellus erat sem, dapibus sed adipiscing pellentesque,
/// mattis consequat velit. Vivamus suscipit nisl non urna suscipit
/// rutrum. Integer lacinia pulvinar pulvinar. Cras non cursus
/// felis. Nulla vitae blandit ante. Curabitur dapibus dapibus est id
/// imperdiet. Donec id pretium lacus. Cras nec dictum orci.
///

#include "argon/dtsengine.hh"

#include "parserapi.hh"
#include "tokenizer.hh"

#include <cstdlib>
#include <cstdio>
#include <memory>
#include <iostream>

ARGON_NAMESPACE_BEGIN

//..............................................................................
////////////////////////////////////////////////////////////////////// DTSEngine

/// @details
/// 
DTSEngine::DTSEngine(void)
    : m_tree(),
      m_connections(),
      m_tasks(),
      m_userConns()
{}

/// @details
/// 
void 
DTSEngine::addConnection(String name, db::Connection *dbc)
{
    this->m_userConns[Identifier(name)] = dbc;
}


/// @details
/// 
Connection&
DTSEngine::getConn(Identifier id)
{
    if(this->m_connections.find(id) == this->m_connections.end())
        throw std::runtime_error("Connection not found: " + std::string(id.name()));
    else
        return *this->m_connections[id];
}


/// @details
/// 
Task&
DTSEngine::getTask(Identifier id)
{
    if(this->m_tasks.find(id) == this->m_tasks.end())
        throw std::runtime_error("Task not found: " + std::string(id.name()));
    else
        return *this->m_tasks[id];
}


/// @details
/// 
db::ConnectionMap&
DTSEngine::getConnections(void)
{
    return this->m_userConns;
}


/// @details
/// 
void
DTSEngine::exec(void)
{
    Processor proc(*this);

    // this adds included scripts to the tree
    proc.compile(this->m_tree.get());
    proc.run();
}


/// @details
/// 
void
DTSEngine::load(std::istreambuf_iterator<wchar_t> in)
{
    Token t;
    Parser p;
    Tokenizer<wchar_t> tz(in);
    tz.setSourceName(String("<unknown>"));

    this->m_tree.reset(new ParseTree);

    //p.trace(stdout, "[LEMON] ");

    do
    {
        t = tz.next();
        //std::cout << "found token: " << t.getSourceInfo() << std::endl;
        try
        {
            if(t.id() != 0)
            {
                Token *tp = this->m_tree->newToken(t);
                p.parse(t.id(), tp, this->m_tree.get());
            }
            else
                p.parse(0, NULL, this->m_tree.get());
        }
        catch(int) /// @bug fix lexical exceptions
        {
            throw std::runtime_error(t.str());
        }
    }
    while(t != Token::eof());
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
