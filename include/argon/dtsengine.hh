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
#include <stack>
#include <set>

#include <dbwtl/dbobjects>
#include <dbwtl/dal/engines/generic>


namespace strutils
{
    template<typename T> class split;
}




ARGON_NAMESPACE_BEGIN


#define ARGON_EXIT_PARSER_ERROR     1
#define ARGON_EXIT_ASSERT           2
#define ARGON_EXIT_SUCCESS          0

class RuntimeError;

class ExceptionType;

class ControlException;
class ReturnControlException;
class TerminateControlException;
class ContinueControlException;
class BreakControlException;
class AssertControlException;

class ValueElement;

typedef std::set<Column> ColumnList;


#define ARGON_ICERR_CTX(checkExpr, context, msg)                        \
    if(!( checkExpr ))                                                  \
    {                                                                   \
        throw InternalError(context, #checkExpr, msg, __FILE__, __LINE__); \
    }

#define ARGON_ICERR(checkExpr, msg)                                     \
    if(!( checkExpr ))                                                  \
    {                                                                   \
        throw InternalError(#checkExpr, msg, __FILE__, __LINE__);       \
    }



//..............................................................................
////////////////////////////////////////////////////////////////////////// Value
///
/// @since 0.0.1
/// @brief Value
class ARGON_EXPORT Value
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

    Value& operator=(const Value &val)
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



//..............................................................................
/////////////////////////////////////////////////////////////////////// safe_ptr
///
/// @since 0.0.1
/// @brief Safe pointer
template<typename T>
class ARGON_EXPORT safe_ptr
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
//////////////////////////////////////////////////////////////////////////// Ref
///
/// @since 0.0.1
/// @brief Ref class
class ARGON_EXPORT Ref
{
public:
    Ref(Element *elem);

    Ref(const Ref &orig);

    ~Ref(void);

    template<typename T>
    T* cast(void) const
    {
        T *tmp = dynamic_cast<T*>(m_element);
        assert(tmp);
        return tmp;
    }

    
    Element* operator->(void);

    const Element* operator->(void) const;

    //Element* operator&(void);

    template<typename T>
    bool is(void) const
    {
        return dynamic_cast<T*>(m_element) == 0 ? false : true;
    }

    void mark_dead(void) { this->m_element = 0; }

    inline bool dead(void) const
    {
       return this->m_element == 0;
    }


protected:
    Element *m_element;

//private:
public:
    Ref& operator=(const Ref& r);
};


typedef std::deque<Ref> ArgumentList;




//..............................................................................
//////////////////////////////////////////////////////////////////// SymbolTable
///
/// @since 0.0.1
/// @brief Symbol table
class ARGON_EXPORT SymbolTable
{
public:
    typedef std::map<Identifier, Ref>         map_type;
    typedef std::list<map_type*>              list_map_type;
    //typedef std::list<Element*>             heap_type;

    /// @brief Constructor
    /// @param[in] parent Pointer to a parent symbol table or NULL
    SymbolTable(SymbolTable *parent = 0);

    ~SymbolTable(void);

    /// @brief Adds a new symbol to the symbol table
    void add(Identifier name, const Ref &ref);

    /// @brief Get a symbol by identifier
    /// Throws if symbol is not of type T
    template<typename T> T* find(Identifier name);

    /// @brief Dumps the symbol table as printable string
    String str(void) const;


    /// @brief Clear out all entries and release memory
    void reset(void);

    map_type& getCurrentSub(void);
    const map_type& getCurrentSub(void) const;


    map_type* createSub(void);
    void deleteSub(map_type *ptr);

    void cloneSymbols(const SymbolTable &from);

protected:

    Ref find_element(Identifier id);

    /// @brief Symbol list
    list_map_type           m_sub_symbols;
    //map_type                m_symbols;

    /// @brief Parent table
    safe_ptr<SymbolTable>   m_parent;


private:
    SymbolTable(const SymbolTable &);
    SymbolTable& operator=(const SymbolTable &);
};


//..............................................................................
////////////////////////////////////////////////////////////////////// TypeTable
///
/// @since 0.0.1
/// @brief Type table
class ARGON_EXPORT TypeTable
{
public:
    typedef std::map<Identifier, Type*>  element_map;
    typedef std::list<Type*>             heap_type;

    /// @brief Constructor
    /// @param[in] parent Pointer to a parent symbol table or NULL
    TypeTable(void);

    ~TypeTable(void);

    /// @brief Adds a new type to the type table
    void add(Type *type);

    /// @brief Get a type by identifier
    /// Throws if tyoe is not of type T
    template<typename T> T* find(Identifier name);

    /// @brief Dumps the symbol table as printable string
    String str(void) const;

    /// @brief Adds a pointer to the type table's memory manager
    template<typename T>
    inline T* addPtr(T* elem)
    {
        m_heap.push_back(elem);
        return elem;
    }

    /// @brief Clear out all entries and release memory
    inline void reset(void)
    {
        this->m_types.clear();
        this->freeHeap();
    }

protected:
    void freeHeap(void);

    Type* find_type(Identifier id);

    /// @brief Symbol list
    element_map             m_types;

    /// @brief Allocated elements
    heap_type                m_heap;



private:
    TypeTable(const TypeTable &);
    TypeTable& operator=(const TypeTable &);
};


//..............................................................................
//////////////////////////////////////////////////////////////////////// Element
///
/// @since 0.0.1
/// @brief Element base class
class ARGON_EXPORT Element
{
public:
    friend class Executor;

    typedef std::list<Ref*> reflist_type;

    virtual ~Element(void);


    virtual SourceInfo getSourceInfo(void) const = 0;


    virtual String str(void) const = 0;
    virtual String name(void) const = 0;
    virtual String type(void) const = 0;

    virtual Value    _value(void) const = 0;
    virtual String   _string(void) const = 0;
    virtual String   _name(void) const = 0;
    virtual String   _type(void) const = 0;

    void registerRef(Ref *p);

    void unregisterRef(Ref *p);

protected:
    /// @brief Constructs a new element
    Element(Processor &proc);

    /// @brief Gives the control to the element.
    /// The default behavior is to raise an exception.
    /// Derived classes must implement this method.
    virtual Value run(void);

    /// @brief Access to the processor
    inline Processor& proc(void) { return this->m_proc; }

    reflist_type m_references;

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
class ARGON_EXPORT Executor
{
public:
    Executor(void)
    {}

    inline Value operator()(Element &elem) const
    {
        return elem.run();
    }

};



//..............................................................................
///////////////////////////////////////////////////////////////////// Connection
///
/// @since 0.0.1
/// @brief Connection element
class ARGON_EXPORT Connection : public Element
{
public:
    Connection(Processor &proc, ConnNode *node, db::ConnectionMap &userConns);

    virtual ~Connection(void)
    {}
    
    db::Connection& getDbc(void);
    db::Env& getEnv(void);

    inline Identifier id(void) const { return m_node->data(); }

    virtual String str(void) const;

    virtual String name(void) const;

    virtual String type(void) const;

    virtual SourceInfo getSourceInfo(void) const;


    virtual Value    _value(void) const;
    virtual String   _string(void) const;
    virtual String   _name(void) const;
    virtual String   _type(void) const;

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
/////////////////////////////////////////////////////////////////////////// Type
///
/// @since 0.0.1
/// @brief Type element
class ARGON_EXPORT Type : public Element
{
public:
    Type(Processor &proc, Identifier type_id) : Element(proc),
                                                m_type_id(type_id) {}


    enum mode_t { DEFAULT_MODE, READ_MODE, INSERT_MODE };


    virtual Identifier id(void) const { return this->m_type_id; }


    virtual Node* getNode(void) const = 0;

    virtual bool builtin(void) const
        {
            return 0 == this->getNode();
        }

    virtual Element* newInstance(const ArgumentList &args, Type::mode_t mode = DEFAULT_MODE) = 0;


//    Identifier        id (void) const = 0;
    virtual String      str (void) const  { return "implme"; }

virtual String  type (void) const  { return "implme"; }
virtual Value   _value (void) const  { throw 0; }
virtual String  _string (void) const  { return name(); }
virtual String  _name (void) const  { return name(); }
virtual String  _type (void) const  { return name(); }
virtual SourceInfo      getSourceInfo (void) const   { throw 0; }

protected:
    /* const */ Identifier m_type_id;
};


class ARGON_EXPORT IExceptionInfo
{
public:
    virtual ~IExceptionInfo(void)
    {}
};

template<typename T>
class ARGON_EXPORT ExceptionInfo : public IExceptionInfo
{
public:
    ExceptionInfo(T &exception)
    {}

    virtual ~ExceptionInfo(void)
    {}
};


//..............................................................................
//////////////////////////////////////////////////////////////////////// Context
///
/// @since 0.0.1
/// @brief Context elements
class ARGON_EXPORT Context : public Element
{
public:
    friend class RuntimeError;

    virtual ~Context(void)
    {}

    virtual SymbolTable& getSymbols(void);

    virtual const SymbolTable& getSymbols(void) const;


    template<typename T>
    T* resolve(const Identifier &id);

    template<typename T>
    T* resolve(const String &name);

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
    //virtual Value resolveColumn(const Column &col) = 0;

    inline const ArgumentList& getCallArgs(void) const
    {
        return this->m_args;
    }


    void setCurrentException(IExceptionInfo *info);
    void releaseCurrentException(void);
    IExceptionInfo& getCurrentException(void);
    bool hasCurrentException(void) const;

protected:
    /// @brief Hidden constructor, only derived classes can be instantiated
    Context(Processor &proc, const ArgumentList &args = ArgumentList());
    Context(Processor &proc, SymbolTable *parentptr, const ArgumentList &args = ArgumentList());

    /// @brief Context-related symbol table
    SymbolTable m_symbols;

    /// @brief Arguments for this context, used when run() is called
    ArgumentList m_args;

    IExceptionInfo *m_exception_info;

private:
    Context(void); // not implemented
    Context(const Context&); // not implemented
    Context& operator=(const Context&); // not implemented
};



//..............................................................................
/////////////////////////////////////////////////////////////////////// Function
///
/// @since 0.0.1
/// @brief Function base class
class ARGON_EXPORT Function : public Context
{
public:
    Function(Processor &proc, FunctionNode *node, const ArgumentList &args);

    virtual ~Function(void)
    {}

    //inline Identifier id(void) const { return m_node->id; }

    virtual String str(void) const;

    virtual String name(void) const;

    virtual String type(void) const;

    virtual SourceInfo getSourceInfo(void) const;


    virtual Value    _value(void) const;
    virtual String   _string(void) const;
    virtual String   _name(void) const;
    virtual String   _type(void) const;

    virtual Object* getMainObject(void);
    virtual Object* getResultObject(void);
    virtual Object* getDestObject(void);

    //virtual Value resolveColumn(const Column &col);


protected:
    /// Functions must be runnable
    virtual Value run(void);

    FunctionNode *m_node;

private:
    Function(const Function&);
    Function& operator=(const Function&);
};



//..............................................................................
///////////////////////////////////////////////////////////////// Lambdafunction
///
/// @since 0.0.1
/// @brief Lambda function class
class ARGON_EXPORT Lambdafunction : public Context
{
public:
    Lambdafunction(Processor &proc, LambdaFuncNode *node, Context &parent_context);

    virtual ~Lambdafunction(void)
    {}

    //inline Identifier id(void) const { return m_node->id; }

    virtual String str(void) const;

    virtual String name(void) const;

    virtual String type(void) const;

    virtual SourceInfo getSourceInfo(void) const;


    virtual Value    _value(void) const;
    virtual String   _string(void) const;
    virtual String   _name(void) const;
    virtual String   _type(void) const;

    virtual Object* getMainObject(void);
    virtual Object* getResultObject(void);
    virtual Object* getDestObject(void);

    //virtual Value resolveColumn(const Column &col);


protected:
    /// Functions must be runnable
    virtual Value run(void);

    LambdaFuncNode *m_node;
    Context        &m_parent_context;

private:
    Lambdafunction(const Lambdafunction&);
    Lambdafunction& operator=(const Lambdafunction&);
};



//..............................................................................
///////////////////////////////////////////////////////////////// Lambdafunction
///
/// @since 0.0.1
/// @brief Lambda function class
class ARGON_EXPORT GlobalContext : public Context
{
public:
    GlobalContext(Processor &proc, Node *tree);

    virtual ~GlobalContext(void)
    {}

    //inline Identifier id(void) const { return m_node->id; }

    virtual String str(void) const;

    virtual String name(void) const;

    virtual String type(void) const;

    virtual SourceInfo getSourceInfo(void) const;


    virtual Value    _value(void) const;
    virtual String   _string(void) const;
    virtual String   _name(void) const;
    virtual String   _type(void) const;

    virtual Object* getMainObject(void);
    virtual Object* getResultObject(void);
    virtual Object* getDestObject(void);

    //virtual Value resolveColumn(const Column &col);


protected:
    /// Functions must be runnable
    virtual Value run(void);

    Node *m_tree;

private:
    GlobalContext(const GlobalContext&);
    GlobalContext& operator=(const GlobalContext&);
};




//..............................................................................
/////////////////////////////////////////////////////////////////////////// Task
///
/// @since 0.0.1
/// @brief Task base class
class ARGON_EXPORT Task : public Context
{
public:
    Task(Processor &proc, TaskNode *node, const ArgumentList &args);

    virtual ~Task(void)
    {}

    inline Identifier id(void) const { return m_node->id; }

    virtual String str(void) const;

    virtual String name(void) const;

    virtual String type(void) const;

    virtual SourceInfo getSourceInfo(void) const;


protected:
    /// Tasks must be runnable
    virtual Value run(void);

    virtual void processData(void);
    virtual void do_processData(void) = 0;

    TaskNode *m_node;

    NodeList m_init_nodes,
        m_before_nodes,
        m_rules_nodes,
        m_after_nodes,
        m_final_nodes;

typedef std::map<std::string, TaskRuleBlockNode*> exh_sqlstate_nodelist_type;
typedef std::map<Identifier, TaskRuleBlockNode*>      exh_id_nodelist_type; // @todo String -> Identifier?


    exh_sqlstate_nodelist_type  m_exh_sqlstates;
    exh_id_nodelist_type        m_exh_ids;
    TaskRuleBlockNode          *m_exh_catchall;

    /*
    NodeList  m_pre_nodes;
    NodeList  m_colassign_nodes;
    NodeList  m_post_nodes;
    */
    

private:
    Task(const Task&);
    Task& operator=(const Task&);
};


//..............................................................................
///////////////////////////////////////////////////////////////////// FETCH Task
///
/// @since 0.0.1
/// @brief FETCH Task
class ARGON_EXPORT FetchTask : public Task
{
public:
    FetchTask(Processor &proc, TaskNode *node, const ArgumentList &args);

    virtual ~FetchTask(void)
    {}

    virtual Object* getMainObject(void);
    virtual Object* getResultObject(void);
    virtual Object* getDestObject(void);

    //virtual Value resolveColumn(const Column &col);


    virtual Value    _value(void) const;
    virtual String   _string(void) const;
    virtual String   _name(void) const;
    virtual String   _type(void) const;


protected:
    virtual Value run(void);

    virtual void do_processData(void);

    /// @brief Main object
    /// This object only exists while task is running
    Object *m_mainobject;

private:
    FetchTask(const FetchTask&);
    FetchTask& operator=(const FetchTask&);
};



//..............................................................................
///////////////////////////////////////////////////////////////////// STORE Task
///
/// @since 0.0.1
/// @brief STORE Task
class ARGON_EXPORT StoreTask : public Task
{
public:
    StoreTask(Processor &proc, TaskNode *node, const ArgumentList &args);

    virtual ~StoreTask(void)
    {}

    virtual Object* getMainObject(void);
    virtual Object* getResultObject(void);
    virtual Object* getDestObject(void);

    //virtual Value resolveColumn(const Column &col);


    virtual Value    _value(void) const;
    virtual String   _string(void) const;
    virtual String   _name(void) const;
    virtual String   _type(void) const;


protected:
    virtual Value run(void);

    virtual void do_processData(void);

    /// @brief Main object
    /// This object only exists while task is running
    Object *m_destobject;

private:
    StoreTask(const StoreTask&);
    StoreTask& operator=(const StoreTask&);
};


//..............................................................................
////////////////////////////////////////////////////////////////// TRANSFER Task
///
/// @since 0.0.1
/// @brief TRANSFER Task
class ARGON_EXPORT TransferTask : public Task
{
public:
    TransferTask(Processor &proc, TaskNode *node, const ArgumentList &args);
    
    virtual ~TransferTask(void)
    {}

    virtual Object* getMainObject(void);
    virtual Object* getResultObject(void);
    virtual Object* getDestObject(void);

    //virtual Value resolveColumn(const Column &col);


    virtual Value    _value(void) const;
    virtual String   _string(void) const;
    virtual String   _name(void) const;
    virtual String   _type(void) const;


protected:
    virtual Value run(void);

    virtual void do_processData(void);

    /// This object only exists while task is running
    Object *m_srcobject;

    Object *m_destobject;

private:
    TransferTask(const TransferTask&);
    TransferTask& operator=(const TransferTask&);
};




//..............................................................................
////////////////////////////////////////////////////////////////////// VOID Task
///
/// @since 0.0.1
/// @brief VOID Task
class ARGON_EXPORT VoidTask : public Task
{
public:
    VoidTask(Processor &proc, TaskNode *node, const ArgumentList &args);

    virtual ~VoidTask(void)
    {}

    virtual Object* getMainObject(void);
    virtual Object* getResultObject(void);
    virtual Object* getDestObject(void);

    //virtual Value resolveColumn(const Column &col);

    virtual Value    _value(void) const;
    virtual String   _string(void) const;
    virtual String   _name(void) const;
    virtual String   _type(void) const;

protected:
    virtual Value run(void);

    virtual void do_processData(void);
};



//..............................................................................
///////////////////////////////////////////////////////////////////////// Object
///
/// @since 0.0.1
/// @brief Object base
class ARGON_EXPORT Object : public Context
{
public:

    Object(Processor &proc, DeclNode *node, const ArgumentList &args);

    virtual ~Object(void) 
    {}
    

    virtual void execute(void) = 0;

    virtual void doInit(void);
    virtual void doReading(void);
    virtual void doWriting(void);
    virtual void doFinal(void);

    /// @brief Fetches the next record
    /// @return true if there was a new record
    virtual bool next(void) = 0;

	virtual void first(void) = 0;

    /// @return true if end of resultset is reached
    virtual bool eof(void) const = 0;

    virtual void setColumn(const Column &col, const Value &v) = 0;

    virtual const db::Value& getColumn(Column col) = 0;


    virtual Value lastInsertRowId(void) = 0;

    // only dummies, because an object can itself be a context
    virtual Object* getMainObject(void);
    virtual Object* getResultObject(void);
    virtual Object* getDestObject(void);

    virtual void setColumnList(const ColumnList &list) = 0;

    virtual void setResultList(const ColumnList &list) = 0;

protected:
    /// Objects must be runnable
    virtual Value run(void);

    int getBindPosition(const Column &col);

    void addBindPosition(const Column &col, int pos);

    std::map<Column, int> m_column_mappings;


    DeclNode *m_node;

    NodeList m_init_nodes,
        m_reading_nodes,
        m_writing_nodes,
        m_final_nodes;

private:
    Object(const Object&);
    Object& operator=(const Object&);
};




//..............................................................................
////////////////////////////////////////////////////////////////////////// Table
///
/// @since 0.0.1
/// @brief Table element
class ARGON_EXPORT Table : public Object
{
public:
    static Table* newInstance(Processor &proc, const ArgumentList &args, Connection *dbc, DeclNode *node, Type::mode_t mode);

    Table(Processor &proc, const ArgumentList &args, DeclNode *node, Type::mode_t mode); // change node

    virtual ~Table(void);

    virtual String str(void) const;

    virtual SourceInfo getSourceInfo(void) const;

    inline Identifier id(void) const { return m_node->data(); } /// @bug is this correct?

    virtual String name(void) const;

    virtual String type(void) const;

    virtual Value    _value(void) const;
    virtual String   _string(void) const;
    virtual String   _name(void) const;
    virtual String   _type(void) const;


    virtual void setColumn(const Column &col, const Value &v);

    virtual const db::Value& getColumn(Column col);

    virtual void execute(void);

	virtual void first(void);

    virtual bool next(void);

    virtual bool eof(void) const;

    virtual void setColumnList(const ColumnList &list);

    virtual void setResultList(const ColumnList &list);

    virtual Value lastInsertRowId(void);

    String generateSelect(String objname);

    virtual String generateInsert(String objname);
    

protected:
    virtual Value run(void);

    db::Stmt::ptr  m_stmt;
    Connection *m_conn;

    ColumnList m_columns;
    ColumnList m_result_columns;

    String m_objname;

    Type::mode_t m_mode;

private:
    Table(const Table&);
    Table& operator=(const Table&);
};



//..............................................................................
//////////////////////////////////////////////////////////////////////////// Sql
///
/// @since 0.0.1
/// @brief SQL
class ARGON_EXPORT Sql : public Object
{
public:
    static Sql* newInstance(Processor &proc, const ArgumentList &args, Connection *dbc, DeclNode *node, Type::mode_t mode);

    Sql(Processor &proc, const ArgumentList &args, DeclNode *node, Type::mode_t mode); // change node

    virtual ~Sql(void) 
    {}

    virtual String str(void) const;

    virtual SourceInfo getSourceInfo(void) const;

    inline Identifier id(void) const { return m_node->data(); } /// @bug is this correct?

    virtual String name(void) const;

    virtual String type(void) const;

    virtual Value    _value(void) const;
    virtual String   _string(void) const;
    virtual String   _name(void) const;
    virtual String   _type(void) const;


    virtual void setColumn(const Column &col, const Value &v);

    virtual const db::Value& getColumn(Column col);

    virtual void execute(void);

	virtual void first(void);

    virtual bool next(void);

    virtual bool eof(void) const;

    virtual void setColumnList(const ColumnList &list);

    virtual void setResultList(const ColumnList &list);

    virtual Value lastInsertRowId(void);

/*
    String generateSelect(String objname);

    String generateInsert(String objname);
*/  

protected:
    virtual Value run(void);

    db::Stmt::ptr  m_stmt;
    Connection *m_conn;

    ColumnList m_columns;
    ColumnList m_result_columns;

    String m_objname;

    Type::mode_t m_mode;

    //NodeList  m_prepost_nodes;
    //NodeList  m_colassign_nodes;

private:
    Sql(const Sql&);
    Sql& operator=(const Sql&);
};




//..............................................................................
/////////////////////////////////////////////////////////////////////// GenRange
///
/// @since 0.0.1
/// @brief GEN_RANGE
class ARGON_EXPORT GenRange : public Object
{
public:
    static GenRange* newInstance(Processor &proc, const ArgumentList &args, DeclNode *node, Type::mode_t mode);

    GenRange(Processor &proc, const ArgumentList &args, DeclNode *node, Type::mode_t mode); // change node

    virtual ~GenRange(void) 
    {}

    virtual String str(void) const;

    virtual SourceInfo getSourceInfo(void) const;

    inline Identifier id(void) const { return m_node->data(); } /// @bug is this correct?

    virtual String name(void) const;

    virtual String type(void) const;

    virtual Value    _value(void) const;
    virtual String   _string(void) const;
    virtual String   _name(void) const;
    virtual String   _type(void) const;


    virtual void setColumn(const Column &col, const Value &v);

    virtual const db::Value& getColumn(Column col);

    virtual void execute(void);

	virtual void first(void);

    virtual bool next(void);

    virtual bool eof(void) const;

    virtual void setColumnList(const ColumnList &list);

    virtual void setResultList(const ColumnList &list);

    virtual Value lastInsertRowId(void);

/*
    String generateSelect(String objname);

    String generateInsert(String objname);
*/  

protected:
    virtual Value run(void);

    String m_objname;
    db::Value m_start;
    signed int m_stop;
    signed int m_step;

    Type::mode_t m_mode;

    //NodeList  m_prepost_nodes;
    //NodeList  m_colassign_nodes;

private:
    GenRange(const GenRange&);
    GenRange& operator=(const GenRange&);
};



//..............................................................................
/////////////////////////////////////////////////////////////////////// Expand
///
/// @since 0.0.1
/// @brief GEN_RANGE
class ARGON_EXPORT Expand : public Object
{
public:
    static Expand* newInstance(Processor &proc, const ArgumentList &args, DeclNode *node, Type::mode_t mode);

    Expand(Processor &proc, const ArgumentList &args, DeclNode *node, Type::mode_t mode); // change node

    virtual ~Expand(void);

    virtual String str(void) const;

    virtual SourceInfo getSourceInfo(void) const;

    inline Identifier id(void) const { return m_node->data(); } /// @bug is this correct?

    virtual String name(void) const;

    virtual String type(void) const;

    virtual Value    _value(void) const;
    virtual String   _string(void) const;
    virtual String   _name(void) const;
    virtual String   _type(void) const;


    virtual void setColumn(const Column &col, const Value &v);

    virtual const db::Value& getColumn(Column col);

    virtual void execute(void);

	virtual void first(void);

    virtual bool next(void);

    virtual bool eof(void) const;

    virtual void setColumnList(const ColumnList &list);

    virtual void setResultList(const ColumnList &list);

    virtual Value lastInsertRowId(void);

/*
    String generateSelect(String objname);

    String generateInsert(String objname);
*/  

protected:
    virtual Value run(void);

    String m_objname;
    std::wstring m_value;
    std::wstring m_sep;

    std::auto_ptr<strutils::split<std::wstring> > m_splitter;

    bool          m_eof;
    db::Value     m_curval;
    Type::mode_t  m_mode;

    //NodeList  m_prepost_nodes;
    //NodeList  m_colassign_nodes;

private:
    Expand(const Expand&);
    Expand& operator=(const Expand&);
};



//..............................................................................
/////////////////////////////////////////////////////////////////////// Compact
///
/// @since 0.0.1
/// @brief COMPACT
class ARGON_EXPORT Compact : public Object
{
public:
    static Compact* newInstance(Processor &proc, const ArgumentList &args, DeclNode *node, Type::mode_t mode);

    Compact(Processor &proc, const ArgumentList &args, DeclNode *node, Type::mode_t mode); // change node

    virtual ~Compact(void) 
    {}

    virtual String str(void) const;

    virtual SourceInfo getSourceInfo(void) const;

    inline Identifier id(void) const { return m_node->data(); } /// @bug is this correct?

    virtual String name(void) const;

    virtual String type(void) const;

    virtual Value    _value(void) const;
    virtual String   _string(void) const;
    virtual String   _name(void) const;
    virtual String   _type(void) const;


    virtual void setColumn(const Column &col, const Value &v);

    virtual const db::Value& getColumn(Column col);

    virtual void execute(void);

	virtual void first(void);

    virtual bool next(void);

    virtual bool eof(void) const;

    virtual void setColumnList(const ColumnList &list);

    virtual void setResultList(const ColumnList &list);

    virtual Value lastInsertRowId(void);

/*
    String generateSelect(String objname);

    String generateInsert(String objname);
*/  

protected:
    virtual Value run(void);

    String m_objname;
    db::Value m_value;
    ValueElement *m_ref;
    String m_sep;

    Type::mode_t m_mode;

    //NodeList  m_prepost_nodes;
    //NodeList  m_colassign_nodes;

private:
    Compact(const Compact&);
    Compact& operator=(const Compact&);
};




//..............................................................................
///////////////////////////////////////////////////////////////////////// LogCmd
///
/// @since 0.0.1
/// @brief LOG Command
class ARGON_EXPORT LogCmd : public Element
{
public:
    LogCmd(Processor &proc, Context &context, LogNode *node);

    virtual ~LogCmd(void) 
    {}

    virtual String str(void) const;

    virtual SourceInfo getSourceInfo(void) const;

    virtual String name(void) const;

    virtual String type(void) const;


    virtual Value    _value(void) const;
    virtual String   _string(void) const;
    virtual String   _name(void) const;
    virtual String   _type(void) const;

protected:
    virtual Value run(void);

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
class ARGON_EXPORT SqlExecCmd : public Element
{
public:
    SqlExecCmd(Processor &proc, Context &context, SqlExecNode *node);

    virtual ~SqlExecCmd(void) 
    {}

    virtual String str(void) const;

    virtual SourceInfo getSourceInfo(void) const;

    virtual String name(void) const;

    virtual String type(void) const;

    virtual Value    _value(void) const;
    virtual String   _string(void) const;
    virtual String   _name(void) const;
    virtual String   _type(void) const;

    void bindParam(int pnum, Value value);

protected:
    virtual Value run(void);

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
class ARGON_EXPORT ValueElement : public Element
{
public:
    ValueElement(Processor &proc, const Value& value);

    virtual ~ValueElement(void)
    {}

    virtual Value& getValue(void)
    {
        return this->m_value;
    }

    virtual void setValue(const Value& value)
    {
        this->m_value = value;
    }
    
    virtual SourceInfo getSourceInfo(void) const;
    virtual String str(void) const;

    virtual String name(void) const;

    virtual String type(void) const;

    virtual Value    _value(void) const;
    virtual String   _string(void) const;
    virtual String   _name(void) const;
    virtual String   _type(void) const;

protected:
    Value m_value;
};


template<class T>
inline Value enter_element(const T &f, Element &elem)
{
    return f(elem);
}



//..............................................................................
/////////////////////////////////////////////////////////////////////// Sequence
///
/// @since 0.0.1
/// @brief Sequence Element
class ARGON_EXPORT Sequence : public Element
{
public:
    Sequence(Processor &proc, const Value& value, const Value &inc_by);

    virtual ~Sequence(void)
    {}

    void nextValue(void);

    virtual Value& getValue(void)
    {
        return this->m_value;
    }
    
    virtual SourceInfo getSourceInfo(void) const;
    virtual String str(void) const;

    virtual String name(void) const;

    virtual String type(void) const;

    virtual Value    _value(void) const;
    virtual String   _string(void) const;
    virtual String   _name(void) const;
    virtual String   _type(void) const;

protected:
    Value m_value;
    Value m_inc_by;
};



//..............................................................................
///////////////////////////////////////////////////////////////////////// Column
///
/// @since 0.0.1
/// @brief Column
class ARGON_EXPORT Column
{
public:
    enum selection_mode {
        by_number,
        by_name,
        invalid_column
    };

    Column(void) : m_mode(invalid_column), m_name(), m_num()
    {}

    Column(ColumnNode *node) : m_mode(by_name), m_name(), m_num()
    {
        assert(node);
        m_name = node->data();
    }

    Column(ResColumnNode *node) : m_mode(by_name), m_name(), m_num()
    {
        assert(node);
        m_name = node->data();
    }
    

    Column(ColumnNumNode *node) : m_mode(by_number), m_name(), m_num()
    {
        assert(node);
        m_num = node->data();
    }


    Column(ResColumnNumNode *node) : m_mode(by_number), m_name(), m_num()
    {
        assert(node);
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

    bool operator<(const Column &col) const
    {
        switch(m_mode)
        {
        case by_number:
            return m_num < col.m_num;
        case by_name:
            return m_name < col.m_name;
        default:
            throw std::runtime_error("invalid column");
        }
    }

    bool operator==(const Column &col) const
    {
        return (m_mode == col.m_mode) && (m_name == col.m_name) && (m_num == col.m_num);
    }

protected:
    enum selection_mode  m_mode;
    String               m_name;
    int                  m_num;
};


class ScopedStackPush;
class ScopedStackFrame;
class ScopedSubSymbols;
class ScopedReleaseException;


/// @bug add documentation
struct builtin_func_def
{
    const char *name;
    Function*(*factory)(Processor &, const ArgumentList &);
    signed int min_arg;
    signed int max_arg;
};



//..............................................................................
////////////////////////////////////////////////////////////////////// Processor
///
/// @since 0.0.1
/// @brief DTS Processor
class ARGON_EXPORT Processor
{
    friend class Pass1Visitor;
    friend class Pass2Visitor;
    friend class ScopedStackPush;
    friend class ScopedStackFrame;

public:
    typedef std::deque<Element*>   stack_type;
    typedef std::list<Element*>    heap_type;

    /// @brief Constructor
    Processor(DTSEngine &engine);

    /// @brief Compile the parse tree
    void compile(ParseTree *tree);

    /// @brief Run compiled structure
    Value run(void);

    /// @brief Get call stack
    const stack_type& getStack(void);

//    void addtoHeap(Element *elem);

    void stackPush(Element *elem);

    Value call(Element &localObj);

    Value call(const Identifier &id, const ArgumentList &args = ArgumentList());

    Value call(const Identifier &id, ArgumentsNode *argsNode, Context &ctx);

    //SymbolTable& getSymbols(void);

    GlobalContext& getGlobalContext(void);

    TypeTable& getTypes(void);

    Function* createFunction(const Identifier &id);

    DTSEngine& getEngine(void);
    const DTSEngine& getEngine(void) const;

    void setGlobalMapValue(const std::wstring &key, const db::Variant &value);
    db::Variant getGlobalMapValue(const std::wstring &key);

protected:
    db::ConnectionMap& getConnections(void);
    void addBuiltinFunctiontable(builtin_func_def *ptr);

    DTSEngine    &m_engine;
    stack_type    m_stack;
    heap_type     m_heap;
    ParseTree    *m_tree;
    //SymbolTable   m_symbols;
    TypeTable     m_types;
    GlobalContext *m_globalcontext;

    typedef std::map<std::wstring, db::Variant> globalmap_type;
    globalmap_type m_globalmap;

private:
    Processor(const Processor&);
    Processor& operator=(const Processor&);
};






//..............................................................................
//////////////////////////////////////////////////////////////// ScopedStackPush
///
/// @since 0.0.1
/// @brief Scoped stack push
class ScopedStackPush
{
public:
    inline ScopedStackPush(Processor &proc, Element *elem)
        : m_ptr(elem),
          m_stack(proc.m_stack)
    {
        m_stack.push_back(elem);
    }

    ~ScopedStackPush(void);


protected:
    Element *m_ptr;
    Processor::stack_type &m_stack;

private:
    ScopedStackPush(const ScopedStackPush&);
    ScopedStackPush& operator=(const ScopedStackPush&);
};

#define ARGON_SCOPED_STACKPUSH(proc, elem) ScopedStackPush ssp__autogen_##__LINE__ (proc, elem)


//..............................................................................
/////////////////////////////////////////////////////////////// ScopedStackFrame
///
/// @since 0.0.1
/// @brief Scoped stack frame
class ScopedStackFrame
{
public:
    ScopedStackFrame(Processor &proc);

    ~ScopedStackFrame(void);

protected:
    Processor::stack_type &m_stack;
    Processor::stack_type::size_type m_saved_size;
};


#define ARGON_SCOPED_STACKFRAME(proc) ScopedStackFrame ssf__autogen_##__LINE__ (proc)



//..............................................................................
/////////////////////////////////////////////////////////////// ScopedSubSymbols
///
/// @since 0.0.1
/// @brief Scoped sub symbols
class ScopedSubSymbols
{
public:
    ScopedSubSymbols(SymbolTable &st);

    ~ScopedSubSymbols(void);

protected:
    SymbolTable::map_type *m_subptr;
    SymbolTable           &m_symbols;
/*
    Processor::stack_type &m_stack;
    Processor::stack_type::iterator m_pos;
*/

private:
    ScopedSubSymbols(const ScopedSubSymbols&);
    ScopedSubSymbols& operator=(const ScopedSubSymbols&);
};


#define ARGON_SCOPED_SUBSYMBOLS(st) ScopedSubSymbols sss__autogen_##__LINE__ (st)



//..............................................................................
///////////////////////////////////////////////////////// ScopedReleaseException
///
/// @since 0.0.1
/// @brief Scoped Release Exception
class ARGON_EXPORT ScopedReleaseException
{
public:
    ScopedReleaseException(Context &ctx);

    ~ScopedReleaseException(void);

protected:
    Context &m_ctx;
};


#define ARGON_SCOPED_RELEASE_EXCEPTION(ctx) ScopedReleaseException sre__autogen_##__LINE__ (ctx)





//..............................................................................
///////////////////////////////////////////////////////////////// ObjectSmartPtr
///
/// @since 0.0.1
/// @brief Smart pointer for Objects
/// Used in task implementations to hold an object pointer
class ARGON_EXPORT ObjectSmartPtr
{
public:
    ObjectSmartPtr(Object **destptr, Object *ptr) : m_ptr(destptr)
    {
        *m_ptr = ptr;
    }

    ~ObjectSmartPtr(void)
    {
        delete *m_ptr;
		*m_ptr = 0;
    }

protected:
    Object **m_ptr;

private:
    ObjectSmartPtr(const ObjectSmartPtr&);
    ObjectSmartPtr& operator=(const ObjectSmartPtr&);

};





//..............................................................................
////////////////////////////////////////////////////////////////////// LastError
///
/// @since 0.0.1
/// @brief Last Error
class ARGON_EXPORT LastError
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
//////////////////////////////////////////////////////////////// CustomException
///
/// @since 0.0.1
/// @brief Control exception
class ARGON_EXPORT CustomException : public std::exception
{
public:
    CustomException(const ExceptionType &type);

    CustomException(const ExceptionType &type, const Value& val);

	virtual ~CustomException() throw();

    virtual const char* what(void) const throw();

    virtual const ExceptionType& getType(void) const;

protected:
    const ExceptionType &m_type;
    Value m_value;
};



//..............................................................................
/////////////////////////////////////////////////////////////// ControlException
///
/// @since 0.0.1
/// @brief Control exception
class ARGON_EXPORT ControlException
{
public:
	virtual ~ControlException()
	{}
};



//..............................................................................
///////////////////////////////////////////////////////// ReturnControlException
///
/// @since 0.0.1
/// @brief Control exception for return statement
class ARGON_EXPORT ReturnControlException : public ControlException
{
public:
	ReturnControlException(Value retvalue);

	virtual ~ReturnControlException(void);

	virtual Value getValue(void) const;

protected:
	Value m_value;
};


//..............................................................................
/////////////////////////////////////////////////////// ContinueControlException
///
/// @since 0.0.1
/// @brief Control exception for continue statement
class ARGON_EXPORT ContinueControlException : public ControlException
{
public:
	ContinueControlException(void);

	virtual ~ContinueControlException(void);
};



//..............................................................................
////////////////////////////////////////////////////// ConditionControlException
///
/// @since 0.0.1
/// @brief Control exception condition command
class ARGON_EXPORT ConditionControlException : public ControlException
{
public:
	ConditionControlException(void);

	virtual ~ConditionControlException(void);
};



//..............................................................................
////////////////////////////////////////////////////////// BreakControlException
///
/// @since 0.0.1
/// @brief Control exception for break statement
class ARGON_EXPORT BreakControlException : public ControlException
{
public:
	BreakControlException(void);

	virtual ~BreakControlException(void);
};



//..............................................................................
//////////////////////////////////////////////////////// RethrowControlException
///
/// @since 0.0.1
/// @brief Control exception for break statement
class ARGON_EXPORT RethrowControlException : public ControlException
{
public:
	RethrowControlException(void);

	virtual ~RethrowControlException(void);
};



//..............................................................................
////////////////////////////////////////////////////// TerminateControlException
///
/// @since 0.0.1
/// @brief Control exception for program termination
class ARGON_EXPORT TerminateControlException : public ControlException
{
public:
	TerminateControlException(Value exitcode);

	virtual ~TerminateControlException(void);

	virtual Value getExitcode(void) const;

protected:
	Value m_exitcode;
};


//..............................................................................
///////////////////////////////////////////////////////// AssertControlException
///
/// @since 0.0.1
/// @brief Control exception for asserts
class ARGON_EXPORT AssertControlException : public ControlException
{
public:
	AssertControlException(AssertNode *node);
    AssertControlException(AssertNode *node, const String &comment);

	virtual ~AssertControlException(void);

	virtual String getComment(void) const;

    virtual String message(void) const;

protected:
    SourceInfo m_sourceinfo;
    String     m_comment;
};






//..............................................................................
////////////////////////////////////////////////////////////////////// DTSEngine
///
/// @since 0.0.1
/// @brief DTS Engine
class ARGON_EXPORT DTSEngine
{
    friend class Processor;

    typedef void (*logger_callback_t)(const String &msg, void *arg);

public:
    DTSEngine(void);

    /// @brief Adds a new connection to the internal connection list
    void addConnection(String name, db::Connection *dbc);

    void registerLogger(logger_callback_t callback, void *arg);

    void writeLog(const String &what) const;

    void load(std::istreambuf_iterator<wchar_t> in, String sourcename = String("<input>"));

    void load(const char* file);


    /// @brief Execute the loaded script
    int exec(void);

    /// @brief Get connection by identifier
    Connection& getConn(Identifier id);

    /// @brief Get task by identifier
    Task& getTask(Identifier id);


protected:
    typedef Function* (*FunctionCreator)(Processor &proc);

    typedef std::map<Identifier, Connection*>       connection_map;
    typedef std::map<Identifier, Task*>             task_map;
    typedef std::map<Identifier, FunctionCreator>   function_map;

    db::ConnectionMap& getConnections(void);

    std::auto_ptr<ParseTree>    m_tree;
    connection_map              m_connections;
    task_map                    m_tasks;
    db::ConnectionMap           m_userConns;
    Processor                   m_proc;
    function_map                m_functions;
    logger_callback_t           m_logger_callback;
        void                       *m_logger_cb_arg;


private:
    DTSEngine(const DTSEngine&);
    DTSEngine& operator=(const DTSEngine&);
};





//
//
template<typename T> T*
SymbolTable::find(Identifier name)
{
    Ref ref = this->find_element(name);
    
    T* ptr = ref.cast<T>();
    if(!ptr)
        throw std::runtime_error("found element, but it has a different type");
    else
        return ptr;
}



//
//
template<typename T> T*
TypeTable::find(Identifier name)
{
    Type *elem = this->find_type(name);
    
    T* ptr = dynamic_cast<T*>(elem);
    if(!ptr)
        /// @bug fix message
        throw std::runtime_error("found type, but it has a different type");
    else
        return ptr;
}


/// @details
/// resolve<T> returns an element of type T* if the given id
/// is an element of type T or a ValueElement that contains the name
/// of an element of type T.
template<typename T>
T* Context::resolve(const Identifier &id)
{
    Element* elem = this->getSymbols().find<Element>(id);

    T* ret = dynamic_cast<T*>(elem);
    if(ret)
        return ret;
    else
    {
        ValueElement* val = dynamic_cast<ValueElement*>(elem);
        if(val)
        {
            return this->getSymbols().find<T>(val->_string());
        }
    }
    // fallback
    throw std::runtime_error("found element, but it has a different type");
}




template<typename T>
T* Context::resolve(const String &name)
{
    return this->resolve<T>(Identifier(name));
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
