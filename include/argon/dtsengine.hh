//
// dtsengine.hh - DTS Engine
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


class RuntimeError;

ARGON_NAMESPACE_BEGIN



#define ARGON_ICERR(checkExpr, context, msg)                            \
    if(!( checkExpr ))                                                  \
    {                                                                   \
        throw InternalError(context, #checkExpr, msg, __FILE__, __LINE__); \
    }



//..............................................................................
////////////////////////////////////////////////////////////////////////// Value
///
/// @since 0.0.1
/// @brief Value
class Value
{
public:
    bool isVoid(void) const { return true; }

    // add value token
    // IVariant?

    template<typename T>
    Value(const T &val) : m_var(val)
    {}

    Value(void) : m_var()
    {}

    Value(const db::Value & ref) : m_var()
    {
        m_var.assign(ref);
    }

    Value(const Value &val) : m_var()
    {
        m_var.assign(val.m_var);
    }

    Value& operator=(Value &val)
    {
        m_var.assign(val.m_var);
        return *this;
    }

    String str(void) const
    {
        return data().asStr();
    }

    db::Variant& data(void)             { return this->m_var; }
    const db::Variant& data(void) const { return this->m_var; }

protected:
    db::Variant m_var;

};

typedef std::list<Value> ArgumentList;


//..............................................................................
/////////////////////////////////////////////////////////////////////// safe_ptr
///
/// @since 0.0.1
/// @brief Safe pointer
template<typename T>
class safe_ptr
{
public:
    safe_ptr(T* p = 0) : m_pointee(p)
    {}

    safe_ptr& operator=(T *p)
    {
        this->m_pointee = p;
        return *this;
    }

    operator bool(void) const
    {
        return m_pointee != 0;
    }

    T* operator->(void)
    {
        assert(m_pointee);
        return m_pointee;
    }

    T& operator*(void)
    {
        return *this->m_pointee;
    }

    T* get(void)
    {
        return this->m_pointee;
    }

private:
    T *m_pointee;
};




//..............................................................................
//////////////////////////////////////////////////////////////////// SymbolTable
///
/// @since 0.0.1
/// @brief Symbol table
class SymbolTable
{
public:
    typedef std::deque<Element*>            stack_type;
    typedef std::map<Identifier, Element*>  element_map;
    typedef std::list<Element*>             heap_type;

    /// @brief Constructor
    /// @param[in] parent Pointer to a parent symbol table or NULL
    SymbolTable(SymbolTable *parent = 0);

    ~SymbolTable(void);

    /// @brief Adds a new symbol to the symbol table
    void add(Identifier name, Element *symbol);

    /// @brief Get a symbol by identifier
    /// Throws if symbol is not of type T
    template<typename T> T* find(Identifier name);

    /// @brief Dumps the symbol table as printable string
    String str(void) const;

    /// @brief Adds a pointer to the symbol table's memory manager
    template<typename T>
    inline T* addPtr(T* elem)
    {
        m_heap.push_back(elem);
        return elem;
    }

    /// @brief Clear out all entries and release memory
    inline void reset(void)
    {
        this->m_symbols.clear();
        this->freeHeap();
    }

protected:
    void freeHeap(void);

    Element* find_element(Identifier id);

    /// @brief Symbol list
    element_map             m_symbols;

    /// @brief Allocated elements
    heap_type                m_heap;

    /// @brief Parent table
    safe_ptr<SymbolTable>   m_parent;


private:
    SymbolTable(const SymbolTable &);
    SymbolTable& operator=(const SymbolTable &);
};



//..............................................................................
//////////////////////////////////////////////////////////////////////// Element
///
/// @since 0.0.1
/// @brief Element base class
class Element
{
public:
    friend class Executor;

    virtual ~Element(void)
    {}

    virtual String str(void) const = 0;
    virtual String name(void) const = 0;
    virtual String type(void) const = 0;

    virtual SourceInfo getSourceInfo(void) const = 0;

protected:
    /// @brief Constructs a new element
    Element(Processor &proc);

    /// @brief Gives the control to the element.
    /// The default behavior is to raise an exception.
    /// Derived classes must implement this method.
    virtual Value run(const ArgumentList &args);

    /// @brief Access to the processor
    inline Processor& proc(void) { return this->m_proc; }

private:
    Processor &m_proc;

    Element(void); // not implemented
    Element(const Element&); // not implemented
    Element& operator=(const Element&); // not implemented
};




//..............................................................................
/////////////////////////////////////////////////////////////////////// Executor
///
/// @since 0.0.1
/// @brief Element Executor
class Executor
{
public:
    Executor(const ArgumentList &args) : m_list(args)
    {}

    inline Value operator()(Element &elem) const
    {
        return elem.run(m_list);
    }

private:
    ArgumentList m_list;
};




//..............................................................................
///////////////////////////////////////////////////////////////////// Connection
///
/// @since 0.0.1
/// @brief Connection element
class Connection : public Element
{
public:
    Connection(Processor &proc, ConnNode *node, db::ConnectionMap &userConns);

    virtual ~Connection(void)
    {}
    
    db::Connection& getDbc(void);

    inline Identifier id(void) const { return m_node->id; }

    virtual String str(void) const;

    virtual String name(void) const;

    virtual String type(void) const;

    virtual SourceInfo getSourceInfo(void) const;


protected:
    ConnNode             *m_node;
    db::Connection       *m_dbc;

    // keep correct order for destruction
    db::Env::ptr          m_alloc_env;
    db::Connection::ptr   m_alloc_dbc;

private:
    Connection(const Connection &);
    Connection& operator=(const Connection &);
};




//..............................................................................
//////////////////////////////////////////////////////////////////////// Context
///
/// @since 0.0.1
/// @brief Context elements
class Context : public Element
{
public:
    friend class RuntimeError;

    virtual ~Context(void)
    {}

    virtual SymbolTable& getSymbols(void);

    virtual const SymbolTable& getSymbols(void) const;

    /// @brief Returns the main object or raises an exception if
    /// there is no main object in this context.
    virtual Object* getMainObject(void) = 0;

    /// @brief Returns the object containing the "results" after
    /// an successful task execution. This is used for the % operator.
    virtual Object* getResultObject(void) = 0;

    /// @brief Returns the object to which data should be written.
    /// Used for STORE and TRANSFER templates by the ColAssignOp.
    virtual Object* getDestObject(void) = 0;

    /// @brief Resolved the value of the given column
    /// or throws an exception if this context does not
    /// have a main object and/or open resultset.
    virtual Value resolve(const Column &col) = 0;


protected:
    /// @brief Hidden constructor, only derived classes can be instantiated
    Context(Processor &proc);

    /// @brief Context-related symbol table
    SymbolTable m_symbols;

private:
    Context(void); // not implemented
    Context(const Context&); // not implemented
    Context& operator=(const Context&); // not implemented
};



//..............................................................................
/////////////////////////////////////////////////////////////////////////// Task
///
/// @since 0.0.1
/// @brief Task base class
class Task : public Context
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

protected:
    /// Tasks must be runnable
    virtual Value run(const ArgumentList &args);

    TaskNode *m_node;

private:
    Task(const Task&);
    Task& operator=(const Task&);
};


//..............................................................................
///////////////////////////////////////////////////////////////////// FETCH Task
///
/// @since 0.0.1
/// @brief FETCH Task
class FetchTask : public Task
{
public:
    FetchTask(Processor &proc, TaskNode *node);

    virtual ~FetchTask(void)
    {}

    virtual Object* getMainObject(void);
    virtual Object* getResultObject(void);
    virtual Object* getDestObject(void);

    virtual Value resolve(const Column &col);

protected:
    virtual Value run(const ArgumentList &args);

    /// @brief Main object
    /// This object only exists while task is running
    std::auto_ptr<Object> m_mainobject;
};


//..............................................................................
////////////////////////////////////////////////////////////////////// VOID Task
///
/// @since 0.0.1
/// @brief VOID Task
class VoidTask : public Task
{
public:
    VoidTask(Processor &proc, TaskNode *node);

    virtual ~VoidTask(void)
    {}

    virtual Object* getMainObject(void);
    virtual Object* getResultObject(void);
    virtual Object* getDestObject(void);

    virtual Value resolve(const Column &col);

protected:
    virtual Value run(const ArgumentList &args);
};



//..............................................................................
///////////////////////////////////////////////////////////////////////// Object
///
/// @since 0.0.1
/// @brief Object base
class Object : public Context
{
public:
    typedef enum
    {
        READ_MODE,
        ADD_MODE,
        UPDATE_MODE
    } mode;

    Object(Processor &proc, ObjectNode *node);

    virtual ~Object(void) 
    {}
    
    /// @brief Fetches the next record
    /// @return true if there was a new record
    virtual bool next(void) = 0;

    /// @return true if end of resultset is reached
    virtual bool eof(void) const = 0;

    virtual const db::Value& getColumn(Column col) = 0;

    virtual Value resolve(const Column &col);

    virtual Object* getMainObject(void);
    virtual Object* getResultObject(void);
    virtual Object* getDestObject(void);


protected:
    /// Objects must be runnable
    virtual Value run(const ArgumentList &args);

    ObjectNode *m_node;

private:
    Object(const Object&);
    Object& operator=(const Object&);
};




//..............................................................................
///////////////////////////////////////////////////////////////////// ObjectInfo
///
/// @since 0.0.1
/// @brief Object info
class ObjectInfo : public Element
{
public:
    ObjectInfo(Processor &proc, ObjectNode *node);

    virtual ~ObjectInfo(void)
    {}

    inline Identifier id(void) const { return m_node->id; } /// @bug is this correct?

    virtual String str(void) const;

    virtual String name(void) const;

    virtual String type(void) const;

    virtual SourceInfo getSourceInfo(void) const;

    /// @brief Creates a new object based on the object information
    virtual Object* newInstance(Object::mode mode);
    
protected:
    ObjectNode *m_node;

private:
    ObjectInfo(const ObjectInfo&);
    ObjectInfo& operator=(const ObjectInfo&);
};



//..............................................................................
//////////////////////////////////////////////////////////////////// SourceTable
///
/// @since 0.0.1
/// @brief Source Table
class SourceTable : public Object
{
public:
    SourceTable(Processor &proc, ObjectNode *node); // change node

    virtual ~SourceTable(void) 
    {}

    virtual String str(void) const;

    virtual SourceInfo getSourceInfo(void) const;

    inline Identifier id(void) const { return m_node->id; } /// @bug is this correct?

    virtual String name(void) const;

    virtual String type(void) const;

    virtual void setColumn(Column col, Value v);

    virtual const db::Value& getColumn(Column col);

    virtual bool next(void);

    virtual bool eof(void) const;

protected:
    virtual Value run(const ArgumentList &args);

    db::Stmt::ptr  m_stmt;

private:
    SourceTable(const SourceTable&);
    SourceTable& operator=(const SourceTable&);
};



//..............................................................................
////////////////////////////////////////////////////////////////////// DestTable
///
/// @since 0.0.1
/// @brief Destination Table
class DestTable : public Object
{
public:
    DestTable(Processor &proc, ObjectNode *node); // change node

    virtual ~DestTable(void) 
    {}

    virtual String str(void) const;

    virtual SourceInfo getSourceInfo(void) const;

    inline Identifier id(void) const { return m_node->id; } /// @bug is this correct?

    virtual String name(void) const;

    virtual String type(void) const;

    virtual void setColumn(Column col, Value v);

    virtual const db::Value& getColumn(Column col);

    virtual bool next(void);

    virtual bool eof(void) const;

protected:
    virtual Value run(const ArgumentList &args);

    db::Stmt::ptr   m_stmt;

private:
    DestTable(const DestTable&);
    DestTable& operator=(const DestTable&);
};




//..............................................................................
///////////////////////////////////////////////////////////////////////// LogCmd
///
/// @since 0.0.1
/// @brief LOG Command
class LogCmd : public Element
{
public:
    LogCmd(Processor &proc, Context &context, LogNode *node);

    virtual ~LogCmd(void) 
    {}

    virtual String str(void) const;

    virtual SourceInfo getSourceInfo(void) const;

    virtual String name(void) const;

    virtual String type(void) const;

protected:
    virtual Value run(const ArgumentList &args);

    Context &m_context;
    LogNode *m_node;

private:
    LogCmd(const LogCmd&);
    LogCmd& operator=(const LogCmd&);
};



//..............................................................................
///////////////////////////////////////////////////////////////////// SqlExecCmd
///
/// @since 0.0.1
/// @brief EXEC SQL Command
class SqlExecCmd : public Element
{
public:
    SqlExecCmd(Processor &proc, Context &context, SqlExecNode *node);

    virtual ~SqlExecCmd(void) 
    {}

    virtual String str(void) const;

    virtual SourceInfo getSourceInfo(void) const;

    virtual String name(void) const;

    virtual String type(void) const;

    void bindParam(int pnum, Value value);

protected:
    virtual Value run(const ArgumentList &args);

    Context       &m_context;
    SqlExecNode   *m_node;
    db::Stmt::ptr  m_stmt;

private:
    SqlExecCmd(const SqlExecCmd&);
    SqlExecCmd& operator=(const SqlExecCmd&);
};




//..............................................................................
/////////////////////////////////////////////////////////////////// ValueElement
///
/// @since 0.0.1
/// @brief Value Element
class ValueElement : public Element
{
public:
    ValueElement(Processor &proc, const Value& value);

    virtual ~ValueElement(void)
    {}

    virtual Value& getValue(void)
    {
        return this->m_value;
    }
    
    virtual SourceInfo getSourceInfo(void) const;
    virtual String str(void) const;

    virtual String name(void) const;

    virtual String type(void) const;

protected:
    Value m_value;
};


template<class T>
inline Value enter_element(const T &f, Element &elem)
{
    return f(elem);
}



//..............................................................................
///////////////////////////////////////////////////////////////////////// Column
///
/// @since 0.0.1
/// @brief Column
class Column
{
public:
    enum selection_mode {
        by_number,
        by_name
    };

    Column(ColumnNode *node) : m_mode(by_name), m_name(), m_num()
    {
        m_name = node->data();
    }

    Column(ColumnNumNode *node) : m_mode(by_number), m_name(), m_num()
    {
        m_num = node->data();
    }

    Column(int num) : m_mode(by_number), m_name(), m_num(num)
    {}

    Column(String name) : m_mode(by_name), m_name(name), m_num(0)
    {}

    inline enum selection_mode mode(void) const { return this->m_mode; }

    inline String getName(void) const { return this->m_name; }

    inline int getNum(void) const { return this->m_num; }

    const db::Value& getFrom(db::Resultset &rs, Context &context);

protected:
    enum selection_mode  m_mode;
    String               m_name;
    int                  m_num;
};




//..............................................................................
////////////////////////////////////////////////////////////////////// Processor
///
/// @since 0.0.1
/// @brief DTS Processor
class Processor
{
    friend class ProcTreeWalker;

public:
    typedef std::deque<Element*>   stack_type;

    /// @brief Constructor
    Processor(DTSEngine &engine);

    /// @brief Compile the parse tree
    void compile(ParseTree *tree);

    /// @brief Run compiled structure
    void run(void);

    /// @brief Get call stack
    const stack_type& getStack(void);

    Value call(Element *obj, const ArgumentList &args);

    Value call(Element &localObj);

    SymbolTable& getSymbols(void);


protected:
    db::ConnectionMap& getConnections(void);

    DTSEngine    &m_engine;
    stack_type    m_stack;
    ParseTree    *m_tree;
    SymbolTable   m_symbols;

private:
    Processor(const Processor&);
    Processor& operator=(const Processor&);
};


//..............................................................................
////////////////////////////////////////////////////////////////////// LastError
///
/// @since 0.0.1
/// @brief Last Error
class LastError
{
public:
    LastError(const Processor::stack_type &stack) : m_stack(stack)
    {}

    String str(void) const;

    const Processor::stack_type& getStack(void) const;

protected:
    const Processor::stack_type m_stack;
};


//..............................................................................
////////////////////////////////////////////////////////////////////// DTSEngine
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
    void load(std::istreambuf_iterator<wchar_t> in, String sourcename = String("<input>"));

    void load(const char* file);


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
    Processor                   m_proc;

private:
    DTSEngine(const DTSEngine&);
    DTSEngine& operator=(const DTSEngine&);
};





//
//
template<typename T> T*
SymbolTable::find(Identifier name)
{
    Element *elem = this->find_element(name);
    
    T* ptr = dynamic_cast<T*>(elem);
    if(!ptr)
        throw std::runtime_error("invalid element type");
    else
        return ptr;
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
