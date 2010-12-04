//
// dtsengine.hh - DTS Engine
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
/// @brief DTS Engine and elements
/// @author Daniel Vogelbacher
/// @since 0.1

#ifndef INFORMAVE_ARGON_DTSENGINE_HH
#define INFORMAVE_ARGON_DTSENGINE_HH

#include "argon/fwd.hh"
#include "argon/ast.hh"
#include "argon/token.hh"

#include <iterator>
#include <map>
#include <deque>
#include <vector>
#include <list>

#include <dbwtl/dbobjects>
#include <dbwtl/dal/engines/generic>


ARGON_NAMESPACE_BEGIN


class Value
{
public:
    bool isVoid(void) const { return true; }

    // add value token
    // IVariant?

};

typedef std::list<Value> ArgumentList;


//--------------------------------------------------------------------------
/// Element base class
///
/// @since 0.0.1
/// @brief Element base class
class Element
{
public:
    virtual ~Element(void)
    {}


    /// @bug add return type and argument list type
    virtual Value run(const ArgumentList &args) { throw std::runtime_error("is not callabled"); }

    virtual String str(void) const = 0;
    virtual String name(void) const = 0;
    virtual String type(void) const = 0;

    virtual SourceInfo getSourceInfo(void) const = 0;

protected:
    /// @brief Constructs a new element
    Element(Processor &proc);


    inline Processor& proc(void)
    { return this->m_proc; }


private:
    Processor &m_proc;

    Element(void); // not implemented
    Element(const Element&); // not implemented
    Element& operator=(const Element&); // not implemented
};



//--------------------------------------------------------------------------
/// CONNECTION Command
///
/// @since 0.0.1
class Connection : public Element
{
public:
    Connection(Processor &proc, ConnNode *node, db::ConnectionMap &userConns);
    
    db::Connection& getDbc(void);

    inline Identifier id(void) const { return m_node->id; }

    virtual String str(void) const;


    virtual String name(void) const;
    virtual String type(void) const;

    virtual SourceInfo getSourceInfo(void) const;

    virtual ~Connection(void)
    {}

protected:
    ConnNode      *m_node;
    db::Connection  *m_dbc;

    // keep correct order for destruction
    db::Env::ptr m_alloc_env;
    db::Connection::ptr m_alloc_dbc;

private:
    Connection(const Connection &);
    Connection& operator=(const Connection &);
};




//--------------------------------------------------------------------------
/// TASK Command
///
/// @since 0.0.1
class Task : public Element
{
public:
    Task(Processor &proc, TaskNode *node);

    virtual ~Task(void)
    {}

    inline Identifier id(void) const { return m_node->id; }

    virtual String str(void) const;

    virtual String name(void) const;
    virtual String type(void) const;

    virtual SourceInfo getSourceInfo(void) const;

    virtual Value run(const ArgumentList &args);

protected:
    TaskNode *m_node;

private:
    Task(const Task&);
    Task& operator=(const Task&);
};



//--------------------------------------------------------------------------
/// LOG Command
///
/// @since 0.0.1
class LogCmd : public Element
{
public:
    LogCmd(Processor &proc, LogNode *node);

    virtual ~LogCmd(void) 
    {}

    virtual String str(void) const;

    // what does exec?
    void exec(void);


    virtual SourceInfo getSourceInfo(void) const;

    virtual String name(void) const;
    virtual String type(void) const;


protected:
    LogNode *m_node;

private:
    LogCmd(const LogCmd&);
    LogCmd& operator=(const LogCmd&);
};





//--------------------------------------------------------------------------
/// Processor initial tree walker
///
/// @since 0.0.1
class ProcTreeWalker : public Visitor
{
public:
    ProcTreeWalker(Processor &proc);

    virtual void visit(ConnNode *node);
    virtual void visit(TaskNode *node);
    virtual void visit(ParseTree *node);
    virtual void visit(LogNode *node);
    virtual void visit(IdNode *node);
    virtual void visit(LiteralNode *node);

protected:
    inline Processor& proc(void) { return m_proc; }
    Processor &m_proc;
};




//--------------------------------------------------------------------------
/// DTS Processor
///
/// @since 0.0.1
/// @brief DTS Processor
class Processor
{
    friend class ProcTreeWalker;

public:
    typedef std::deque<Element*>            stack_type;
    typedef std::map<Identifier, Element*>  element_map;


    Processor(DTSEngine &engine);

    /// @brief Compile the parse tree
    void compile(ParseTree *tree);

    /// @brief Run compiled structure
    void run(void);

    /// @brief Get call stack
    const stack_type& getStack(void);

    /// @brief Adds a new symbol to the symbol table
    inline void addSymbol(Identifier name, Element *symbol);

    /// @brief Get a symbol by identifier
    /// Throws if symbol is not of type T
    template<typename T> T* getSymbol(Identifier name);


    /// @bug remove me - NOT!
    Value call(Element *obj, const ArgumentList &args);


protected:
    db::ConnectionMap& getConnections(void);

    template<typename T>
    inline T* toHeap(T* elem)
    {
        m_heap.push_back(ElementPtr(elem));
        return elem;
    }

    DTSEngine    &m_engine;
    stack_type    m_stack;
    ParseTree    *m_tree;
    element_map   m_symbols;

private:
    /// @brief Allocated elements
    std::list<ElementPtr> m_heap;

    Processor(const Processor&);
    Processor& operator=(const Processor&);
};




//--------------------------------------------------------------------------
/// DTS Engine
///
/// @since 0.0.1
/// @brief DTS Engine
class DTSEngine
{
    friend class Processor;

public:
    DTSEngine(void);

    /// @brief Adds a new connection to the internal connection list
    void addConnection(String name, db::Connection *dbc);
 

    /// @bug fixme
    void load(std::istreambuf_iterator<wchar_t> in);

    /// @brief Execute the loaded script
    void exec(void);

    /// @brief Get connection by identifier
    Connection& getConn(Identifier id);

    /// @brief Get task by identifier
    Task& getTask(Identifier id);


protected:
    typedef std::map<Identifier, Connection*>   connection_map;
    typedef std::map<Identifier, Task*>         task_map;

    db::ConnectionMap& getConnections(void);

    std::auto_ptr<ParseTree>    m_tree;
    connection_map              m_connections;
    task_map                    m_tasks;
    db::ConnectionMap           m_userConns;

private:
    DTSEngine(const DTSEngine&);
    DTSEngine& operator=(const DTSEngine&);
};




//
//
template<typename T> T*
Processor::getSymbol(Identifier name)
{
    element_map::iterator i = this->m_symbols.find(name);
    if(i != this->m_symbols.end())
    {
        T* ptr = dynamic_cast<T*>(i->second);
        if(!ptr)
            throw std::runtime_error("invalid element type");
        else
            return ptr;
    }
    else
        throw std::runtime_error("element not found");
}



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
