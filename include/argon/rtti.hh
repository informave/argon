//
// rtti.hh - Type system
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
/// @brief Type system
/// @author Daniel Vogelbacher
/// @since 0.1


#ifndef INFORMAVE_ARGON_RTTI_HH
#define INFORMAVE_ARGON_RTTI_HH

#include "argon/fwd.hh"
#include "argon/token.hh"
#include "argon/semantic.hh"

//#include <list>
//#include <deque>

ARGON_NAMESPACE_BEGIN


//..............................................................................
///////////////////////////////////////////////////////////////// ConnectionType
///
/// @since 0.0.1
/// @brief ConnectionType
class ConnectionType : public Type
{
public:
    ConnectionType(Processor &proc, Identifier type_id) : Type(proc, type_id) {}

    virtual String name(void) const
    {
        return "connection";
    }

    virtual Element* newInstance(const ArgumentList &args, Type::mode_t mode = DEFAULT_MODE)
    {
        return 0;
    }

    virtual Node* getNode(void) const { return NULL_NODE; }

};

//..............................................................................
//////////////////////////////////////////////////////////////////////// VarType
///
/// @since 0.0.1
/// @brief VarType
class VarType : public Type
{
};



//..............................................................................
///////////////////////////////////////////////////////////////////// ObjectType
///
/// @since 0.0.1
/// @brief ObjectType
class ObjectType : public Type
{
public:
    ObjectType(Processor &proc, Identifier type_id)
        : Type(proc, type_id)
    {}

    virtual Object* newInstance(const ArgumentList &args, Type::mode_t mode = DEFAULT_MODE) = 0;

    Connection* findConnection(const ArgumentList &args);

    virtual DeclNode* getNode(void) const = 0;

private:
    ObjectType(const ObjectType &);
    ObjectType& operator=(const ObjectType &);
};


//..............................................................................
////////////////////////////////////////////////////////////////////// TableType
///
/// @since 0.0.1
/// @brief TableType
class TableType : public ObjectType
{
public:
    TableType(Processor &proc, Identifier type_id, DeclNode *node)
        : ObjectType(proc, type_id),
          m_node(node)
    {}

    virtual String name(void) const
    {
        return "table";
    }


    virtual Object* newInstance(const ArgumentList &args, Type::mode_t mode = DEFAULT_MODE);


    virtual DeclNode* getNode(void) const { return this->m_node; }

protected:
    DeclNode *m_node;

private:
    TableType(const TableType &);
    TableType& operator=(const TableType &);
};



//..............................................................................
/////////////////////////////////////////////////////////////// CustomTableType
///
/// @since 0.0.1
/// @brief CustomTableType
class CustomTableType : public TableType
{
public:
    CustomTableType(Processor &proc, Identifier type_id, DeclNode *node)
        : TableType(proc, type_id, node)
    {}

    virtual String name(void) const
    {
        return "table";
    }


    virtual Object* newInstance(const ArgumentList &args, Type::mode_t mode = DEFAULT_MODE);



private:
    CustomTableType(const CustomTableType &);
    CustomTableType& operator=(const CustomTableType &);
};



//..............................................................................
//////////////////////////////////////////////////////////////////////// SqlType
///
/// @since 0.0.1
/// @brief SqlType
class SqlType : public ObjectType
{
public:
    SqlType(Processor &proc, Identifier type_id, DeclNode *node)
        : ObjectType(proc, type_id),
          m_node(node)
    {}

    virtual String name(void) const
    {
        return "sql";
    }


    virtual Object* newInstance(const ArgumentList &args, Type::mode_t mode = DEFAULT_MODE);


    virtual DeclNode* getNode(void) const { return this->m_node; }

protected:
    DeclNode *m_node;

private:
    SqlType(const SqlType &);
    SqlType& operator=(const SqlType &);
};



//..............................................................................
////////////////////////////////////////////////////////////////// CustomSqlType
///
/// @since 0.0.1
/// @brief CustomSqlType
class CustomSqlType : public SqlType
{
public:
    CustomSqlType(Processor &proc, Identifier type_id, DeclNode *node)
        : SqlType(proc, type_id, node)
    {}

    virtual String name(void) const
    {
        return "sql";
    }

    virtual Object* newInstance(const ArgumentList &args, Type::mode_t mode = DEFAULT_MODE);


private:
    CustomSqlType(const CustomSqlType &);
    CustomSqlType& operator=(const CustomSqlType &);
};


//..............................................................................
/////////////////////////////////////////////////////////////////////// TaskType
///
/// @since 0.0.1
/// @brief TaskType
class TaskType : public Type
{
public:
    TaskType(Processor &proc, Identifier type_id, TaskNode *node)
        : Type(proc, type_id),
          m_node(node)
    {}

    virtual String name(void) const
    {
        return "task";
    }


    virtual Task* newInstance(const ArgumentList &args, Type::mode_t mode = DEFAULT_MODE);


    virtual TaskNode* getNode(void) const { return this->m_node; }

protected:
    TaskNode *m_node;

private:
    TaskType(const TaskType &);
    TaskType& operator=(const TaskType &);
};


//..............................................................................
/////////////////////////////////////////////////////////////////// FunctionType
///
/// @since 0.0.1
/// @brief FunctionType
class FunctionType : public Type
{
public:
    FunctionType(Processor &proc, Identifier type_id, FunctionNode *node)
        : Type(proc, type_id),
          m_node(node)
    {}

    virtual String name(void) const
    {
        return "function";
    }


    virtual Function* newInstance(const ArgumentList &args, Type::mode_t mode = DEFAULT_MODE) = 0;


    virtual FunctionNode* getNode(void) const { return this->m_node; }

protected:
    FunctionNode *m_node;

private:
    FunctionType(const FunctionType &);
    FunctionType& operator=(const FunctionType &);
};



//..............................................................................
//////////////////////////////////////////////////////////// BuiltinFunctionType
/// 
/// @since 0.0.1
/// @brief BuiltinFunctionType
class BuiltinFunctionType : public FunctionType
{
public:
    BuiltinFunctionType(Processor &proc, builtin_func_def &def)
        : FunctionType(proc, Identifier(def.name), NULL_NODE),
          m_def(def)
    {}

    virtual Function* newInstance(const ArgumentList &args, Type::mode_t mode = DEFAULT_MODE);

protected:
    builtin_func_def &m_def;

private:
    BuiltinFunctionType(const BuiltinFunctionType &);
    BuiltinFunctionType& operator=(const BuiltinFunctionType &);
};



//..............................................................................
///////////////////////////////////////////////////////////// CustomFunctionType
///
/// @since 0.0.1
/// @brief CustomFunctionType
class CustomFunctionType : public FunctionType
{
public:
    CustomFunctionType(Processor &proc, Identifier type_id, FunctionNode *node)
        : FunctionType(proc, type_id, node)
    {}

    virtual String name(void) const
    {
        return "function";
    }


    virtual Function* newInstance(const ArgumentList &args, Type::mode_t mode = DEFAULT_MODE);


private:
    CustomFunctionType(const CustomFunctionType &);
    CustomFunctionType& operator=(const CustomFunctionType &);
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
