//
// rtti.cc - Type system (definition)
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
/// @brief Type system (definition)
/// @author Daniel Vogelbacher
/// @since 0.1


#include "argon/fwd.hh"
#include "argon/exceptions.hh"
#include "argon/rtti.hh"
#include "visitors.hh"
#include "debug.hh"

#include "builtin/functions.hh"

#include <iostream>
#include <list>

ARGON_NAMESPACE_BEGIN


//..............................................................................
/////////////////////////////////////////////////////////////////////// TaskType


/// @details
/// 
Task*
TaskType::newInstance(const ArgumentList &args, Type::mode_t mode)
{
    assert(! this->getNode()->id.str().empty());

    Task *elem = 0;

    switch(this->getNode()->type)
    {
    case ARGON_TASK_VOID:
        elem = new VoidTask(this->proc(), this->getNode(), args);
        break;
    case ARGON_TASK_FETCH:
        elem = new FetchTask(this->proc(), this->getNode(), args);
        break;
    case ARGON_TASK_STORE:
    	elem = new StoreTask(this->proc(), this->getNode(), args);
        break;
    case ARGON_TASK_TRANSFER:
    	elem = new TransferTask(this->proc(), this->getNode(), args);
        break;
    }
    assert(elem);
    return elem;
}





//..............................................................................
//////////////////////////////////////////////////////////// BuiltinFunctionType


/// @details
/// 
Function*
BuiltinFunctionType::newInstance(const ArgumentList &args, Type::mode_t mode)
{
    return this->m_def.factory(this->proc(), args);
}





//..............................................................................
///////////////////////////////////////////////////////////// CustomFunctionType


/// @details
/// 
Function*
CustomFunctionType::newInstance(const ArgumentList &args, Type::mode_t mode)
{
    return new Function(this->proc(), this->getNode(), args);

    //return this->m_def.factory(this->proc(), args);
}





//..............................................................................
////////////////////////////////////////////////////////////////////// TableType



 /// @details
/// 
Object*
TableType::newInstance(const ArgumentList &args, Type::mode_t mode)
{
    Object *tmp = 0;
 
    assert(this->getNode() == NULL);

    Connection *dbc = NULL;

    assert(args.size() > 0);
    ArgumentList::const_iterator i = args.begin();
    dbc = i->cast<Connection>();    
    assert(dbc);

    tmp = Table::newInstance(this->proc(), args, dbc, NULL_NODE, mode);
    assert(tmp);
    return tmp;
}




//..............................................................................
///////////////////////////////////////////////////////////////////// ObjectType



/// @details
/// This method helps to find out the connection reference.
/// 
/// declare foo(dbc, tablename) : table(dbc, tablename)
/// 
/// In this declaration, the table() arguments are values passed to foo().
/// Because we need to know the connection before be can construct a new
/// Table element, the first step is to resolve the table() arguments.
Connection*
ObjectType::findConnection(const ArgumentList &args)
{
    DeclNode *n = this->getNode();
    assert(n);
    ArgumentsNode *argsNode = find_node<ArgumentsNode>(n);
    assert(argsNode);
    assert(argsNode->getChilds().size() > 0);
    Node *x = argsNode->getChilds()[0];
    IdNode *idNode = node_cast<IdNode>(x);
    assert(idNode);
    Identifier id = idNode->data();
    
    try
    {
        Connection *dbc = this->proc().getGlobalContext().getSymbols().find<Connection>(id);
        return dbc;
    }
    catch(...)
    {
        assert(args.size() > 0);

        //ArgumentList::const_iterator i = args.begin();


        SymbolTable st;

        safe_ptr<ArgumentsSpecNode> argsSpecNode = find_node<ArgumentsSpecNode>(n);

        ArgumentList::const_iterator i = args.begin();

        foreach_node(argsSpecNode->getChilds(),
                     Arg2SymVisitor(this->proc(), st,
                                    i, args.end()), 1);


        return st.find<Connection>(id);

        //return i->cast<Connection>();
        
    }
}



//..............................................................................
//////////////////////////////////////////////////////////////// CustomTableType




/// @details
/// 
Object*
CustomTableType::newInstance(const ArgumentList &args, Type::mode_t mode)
{
    Object *tmp = 0;

    assert(this->getNode());

    Connection *dbc = this->findConnection(args);
    assert(dbc);


    tmp = Table::newInstance(this->proc(), args, dbc, this->getNode(), mode);
    assert(tmp);
    return tmp;
}





//..............................................................................
//////////////////////////////////////////////////////////////////////// SqlType


/// @details
/// 
Object*
SqlType::newInstance(const ArgumentList &args, Type::mode_t mode)
{
    Object *tmp = 0;
 
    assert(this->getNode() == NULL);

    Connection *dbc = NULL;

    assert(args.size() > 0);
    ArgumentList::const_iterator i = args.begin();
    dbc = i->cast<Connection>();    
    assert(dbc);

    tmp = Sql::newInstance(this->proc(), args, dbc, NULL_NODE, mode);
    assert(tmp);
    return tmp;

}





/// @details
/// 
Object*
CustomSqlType::newInstance(const ArgumentList &args, Type::mode_t mode)
{
    Object *tmp = 0;

    assert(this->getNode());

    Connection *dbc = this->findConnection(args);
    assert(dbc);


    tmp = Sql::newInstance(this->proc(), args, dbc, this->getNode(), mode);
    assert(tmp);
    return tmp;
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
