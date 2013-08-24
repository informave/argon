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
    ARGON_ICERR(! this->getNode()->id.str().empty(), "invalid id");

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
    ARGON_ICERR(elem, "invalid element");
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
 
    ARGON_ICERR(this->getNode() == NULL, "invalid node");

    Connection *dbc = NULL;

    ARGON_ICERR(args.size() > 0, "invalid args count");
    ArgumentList::const_iterator i = args.begin();
    dbc = i->cast<Connection>();    
    ARGON_ICERR(dbc, "invalid dbc");

    tmp = Table::newInstance(this->proc(), args, dbc, NULL_NODE, mode);
    ARGON_ICERR(tmp, "invalid table instance");
    return tmp;
}




//..............................................................................
/////////////////////////////////////////////////////////////////// GenRangeType



 /// @details
/// 
Object*
GenRangeType::newInstance(const ArgumentList &args, Type::mode_t mode)
{
    Object *tmp = 0;
 
    ARGON_ICERR(this->getNode() == NULL, "invalid node");


    ARGON_ICERR(args.size() > 0, "invalid child count");

    tmp = GenRange::newInstance(this->proc(), args, NULL_NODE, mode);
    ARGON_ICERR(tmp, "invalid range obj");
    return tmp;

/*
    ArgumentList::const_iterator i = args.begin();
    dbc = i->cast<Connection>();    
    //assert(dbc);

    tmp = Table::newInstance(this->proc(), args, dbc, NULL_NODE, mode);
    //assert(tmp);
    return tmp;
*/
}



//..............................................................................
///////////////////////////////////////////////////////////////////// ExpandType



 /// @details
/// 
Object*
ExpandType::newInstance(const ArgumentList &args, Type::mode_t mode)
{
    Object *tmp = 0;
 
    ARGON_ICERR(this->getNode() == NULL, "invalid node");


    ARGON_ICERR(args.size() > 0, "invalid size");

    tmp = Expand::newInstance(this->proc(), args, NULL_NODE, mode);
    ARGON_ICERR(tmp, "invalid Expand obj");
    return tmp;

/*
    ArgumentList::const_iterator i = args.begin();
    dbc = i->cast<Connection>();    
    //assert(dbc);

    tmp = Table::newInstance(this->proc(), args, dbc, NULL_NODE, mode);
    //assert(tmp);
    return tmp;
*/
}


//..............................................................................
//////////////////////////////////////////////////////////////////// CompactType



 /// @details
/// 
Object*
CompactType::newInstance(const ArgumentList &args, Type::mode_t mode)
{
    Object *tmp = 0;
 
    ARGON_ICERR(this->getNode() == NULL, "invalid node");


    ARGON_ICERR(args.size() > 0, "invalid args count");

    tmp = Compact::newInstance(this->proc(), args, NULL_NODE, mode);
    ARGON_ICERR(tmp, "invalid compact obj");
    return tmp;

/*
    ArgumentList::const_iterator i = args.begin();
    dbc = i->cast<Connection>();    
    //assert(dbc);

    tmp = Table::newInstance(this->proc(), args, dbc, NULL_NODE, mode);
    //assert(tmp);
    return tmp;
*/
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
    ARGON_ICERR(n, "invalid node");
    ArgumentsNode *argsNode = find_node<ArgumentsNode>(n);
    ARGON_ICERR(argsNode, "invalid args node");
    ARGON_ICERR(argsNode->getChilds().size() > 0, "invalid child count");
    Node *x = argsNode->getChilds()[0];
    IdNode *idNode = node_cast<IdNode>(x);
    ARGON_ICERR(idNode, "invalid node");
    Identifier id = idNode->data();
    
    try
    {
        Connection *dbc = this->proc().getGlobalContext().getSymbols().find<Connection>(id);
        return dbc;
    }
    catch(...) /// @bug check this?!
    {
        ARGON_ICERR(args.size() > 0, "invalid args count");

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

    ARGON_ICERR(this->getNode(), "invalid node");

    Connection *dbc = this->findConnection(args);
    ARGON_ICERR(dbc, "invalid dbc");


    tmp = Table::newInstance(this->proc(), args, dbc, this->getNode(), mode);
    ARGON_ICERR(tmp, "invalid table obj");
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
 
    ARGON_ICERR(this->getNode() == NULL, "invalid node");

    Connection *dbc = NULL;

    ARGON_ICERR(args.size() > 0, "invalid args count");
    ArgumentList::const_iterator i = args.begin();
    dbc = i->cast<Connection>();    
    ARGON_ICERR(dbc, "invalid dbc");

    tmp = Sql::newInstance(this->proc(), args, dbc, NULL_NODE, mode);
    ARGON_ICERR(tmp, "invalid sql obj");
    return tmp;

}





/// @details
/// 
Object*
CustomSqlType::newInstance(const ArgumentList &args, Type::mode_t mode)
{
    Object *tmp = 0;

    ARGON_ICERR(this->getNode(), "invalid node");

    Connection *dbc = this->findConnection(args);
    ARGON_ICERR(dbc, "invalid dbc");


    tmp = Sql::newInstance(this->proc(), args, dbc, this->getNode(), mode);
    ARGON_ICERR(tmp, "invalid sql obj");
    return tmp;
}




Element*
CustomExceptionType::newInstance(const ArgumentList &args, Type::mode_t mode)
{
    throw std::runtime_error("do not use");
}

void
CustomExceptionType::throwException(const ArgumentList &args) const
{
    ARGON_ICERR(args.size() == 0 || args.size() == 1, "invalid args count");
    if(args.size() == 1)
    {
        Value v = args.at(0)->_value();
        throw CustomException(*this, v);
    }
    else
        throw CustomException(*this);
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
