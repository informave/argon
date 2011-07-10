//
// ast.hh - Abstract Syntax Tree
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
/// @brief Abstract Syntax Tree (nodes and tree classes)
/// @author Daniel Vogelbacher
/// @since 0.1


#ifndef INFORMAVE_ARGON_AST_HH
#define INFORMAVE_ARGON_AST_HH

#include "argon/fwd.hh"
#include "argon/token.hh"
#include "argon/semantic.hh"

#include <list>
#include <deque>

ARGON_NAMESPACE_BEGIN

struct Node;
struct ConnNode;
struct ConnSpec;
struct TaskNode;
struct Identifier;
struct IdNode;
struct SqlExecNode;
struct LogNode;
struct TaskExecNode;
struct LiteralNode;
struct ColumnNode;
struct ObjectNode;

struct TableNode;
struct SqlNode;

struct ArgumentsNode;
struct ArgumentsSpecNode;
struct TmplArgumentsNode;
struct IdCallNode;
struct ColumnAssignNode;
struct ColumnNumNode;
struct NumberNode;
struct ExprNode;
struct FuncCallNode;
struct ResColumnNode;
struct ResColumnNumNode;
struct ResIdNode;

class Visitor;
class ParseTree;

typedef std::deque<Node*> NodeList;


String gen_anonymous_id(void);


//..............................................................................
/////////////////////////////////////////////////////////////////////////// Node
///
/// @since 0.0.1
/// @brief Node base class
class Node
{
public:
    typedef NodeList nodelist_type;

    Node(void);

    virtual ~Node(void);

    /// @brief Get all child nodes
    inline nodelist_type& getChilds(void)
    {
        return this->m_childs; 
    }

    /// @brief Add a new child
    void addChild(Node *child);

    /// @brief Add new childs
    void addChilds(const NodeList *list);

    /// @brief Get Source information
    SourceInfo getSourceInfo(void) const;

    /// @brief Update information
    void updateSourceInfo(const SourceInfo &info);

    /// @brief String representation (used for debugging)
    virtual String str(void) const = 0;

    /// @brief Visitor function
    virtual void accept(Visitor &visitor) = 0;

    virtual String nodetype(void) const = 0;

    static String name(void) { return "node"; }

    /// @brief Called by the semantic checker
    /// @details
    /// Each node may provide their semantic checks.
    virtual void semanticCheck(SemanticCheck &sc) { }

protected:
    nodelist_type m_childs;
    SourceInfo m_sinfo;
};



//..............................................................................
//////////////////////////////////////////////////////////////////////// Visitor
///
/// @since 0.0.1
/// @brief Visitor base class
class Visitor
{
public:
    typedef enum {
        ignore_none = 0,
        ignore_other = 1
    } visitor_mode;

    Visitor(visitor_mode mode = ignore_none);

    virtual void visit(ConnNode *node);
    virtual void visit(TaskNode *node);
    virtual void visit(ParseTree *node);
    virtual void visit(IdNode *node);
    virtual void visit(LiteralNode *node);
    virtual void visit(LogNode *node);
    virtual void visit(TaskExecNode *node);
    virtual void visit(ColumnNode *node);
    virtual void visit(SqlExecNode *node);
    virtual void visit(ArgumentsNode *node);
    virtual void visit(ArgumentsSpecNode *node);
    virtual void visit(TmplArgumentsNode *node);
    virtual void visit(IdCallNode *node);
    virtual void visit(ColumnAssignNode *node);
    virtual void visit(ColumnNumNode *node);

    virtual void visit(TableNode *node);
    virtual void visit(SqlNode *node);

    virtual void visit(NumberNode *node);
    virtual void visit(ExprNode *node);
    virtual void visit(FuncCallNode *node);
    virtual void visit(ResColumnNode *node);
    virtual void visit(ResColumnNumNode *node);
    virtual void visit(ResIdNode *node);

    void operator()(Node *node);

protected:
    /// @brief This method is called by each visit method defined
    /// in this base class.
    /// It throws an exception, but derived visitor implementations
    /// can override this method to provide a new fallback action.
    virtual void fallback_action(Node *node);

    visitor_mode m_mode;
};



//..............................................................................
///////////////////////////////////////////////////////////////////// Identifier
///
/// @since 0.0.1
/// @brief Identifier
struct Identifier
{
    Identifier() : m_name() { }

    Identifier(const String& name) : m_name(name)
    { }

    bool operator==(const Identifier &o) const
    { return this->m_name == o.m_name; }

    bool operator!=(const Identifier &o) const
    { return ! operator==(o); }

    bool operator<(const Identifier &o) const
    { return this->m_name < o.m_name; }

    String name(void) const
    {
        return this->m_name;
    }

    String str(void) const
    {
        return this->m_name;
    }

protected:
    String m_name;
};


//..............................................................................
/////////////////////////////////////////////////////////////////// SimpleNode<>
///
/// @since 0.0.1
/// @brief Base class for nodes which only have one data attribute
template<class T>
class SimpleNode : public Node
{
public:
    virtual ~SimpleNode(void)
    {}

    virtual void init(const T &data)
    {
        this->m_data = data;
    }

    virtual const T& data(void) const
    {
        return this->m_data;
    }

    virtual T& data(void)
    {
        return this->m_data;
    }

protected:
    SimpleNode(void) : Node(), m_data()
    {}

    T m_data;
};



//..............................................................................
/////////////////////////////////////////////////////////////////// TaskExecNode
///
/// @since 0.0.1
/// @brief Node for EXEC TASK Command
struct TaskExecNode : public SimpleNode<Identifier>
{
    TaskExecNode(void);

    virtual ~TaskExecNode(void) {}

    virtual void accept(Visitor &visitor);

    virtual String str(void) const;

    virtual String nodetype(void) const;

    static String name(void) { return "EXEC"; }

    virtual void semanticCheck(SemanticCheck &sc);
};



//..............................................................................
//////////////////////////////////////////////////////////////////// SqlExecNode
///
/// @since 0.0.1
/// @brief Node for EXEC SQL Command
struct SqlExecNode : public Node
{
    SqlExecNode(void);

    virtual ~SqlExecNode(void) {}

    void init(String sql, Identifier conn_id);

    virtual void accept(Visitor &visitor);

    virtual String str(void) const;

    Identifier connid(void) const;

    String sql(void) const;

    virtual String nodetype(void) const;

    static String name(void) { return "EXEC SQL"; }

protected:
    Identifier m_connid;
    String     m_sql;
};


//..............................................................................
////////////////////////////////////////////////////////////////////// TokenNode
///
/// @since 0.0.1
/// @brief Node for Tokens
struct TokenNode : public Node
{
    TokenNode(Token *tok);

    virtual ~TokenNode(void) {}

    virtual void accept(Visitor &visitor);

    virtual String str(void) const;
    
    virtual String nodetype(void) const;

    static String name(void) { return "TOKEN"; }

protected:
    Token *m_token;

private:
    TokenNode(const TokenNode&);
    TokenNode& operator=(const TokenNode&);
};


//..............................................................................
///////////////////////////////////////////////////////////////////////// IdNode
///
/// @since 0.0.1
/// @brief Node for Identifiers
struct IdNode : public SimpleNode<Identifier>
{
    IdNode(void);

    virtual ~IdNode(void) {}

    virtual void accept(Visitor &visitor);

    virtual String str(void) const;

    virtual String nodetype(void) const;

    static String name(void) { return "Identifier"; }
};





//..............................................................................
/////////////////////////////////////////////////////////////////// FuncCallNode
///
/// @since 0.0.1
/// @brief Node for function calls
struct FuncCallNode : public SimpleNode<Identifier>
{
    FuncCallNode(void);

    virtual ~FuncCallNode(void)
    {}

    virtual void accept(Visitor &visitor);

    virtual String str(void) const;

    virtual String nodetype(void) const;

    static String name(void) { return "FuncCall"; }
};



//..............................................................................
//////////////////////////////////////////////////////////////////// LiteralNode
///
/// @since 0.0.1
/// @brief Node for Literals
struct LiteralNode : public SimpleNode<String>
{
    LiteralNode(void);

    virtual ~LiteralNode(void) {}

    virtual void accept(Visitor &visitor);

    virtual String str(void) const;

    virtual String nodetype(void) const;

    static String name(void) { return "Literal"; }
};


//..............................................................................
///////////////////////////////////////////////////////////////////// NumberNode
///
/// @since 0.0.1
/// @brief Node for Numbers
struct NumberNode : public Node
{
    NumberNode(void);

    virtual ~NumberNode(void) {}

    void init(String data);

    virtual void accept(Visitor &visitor);

    virtual String str(void) const;

    inline int data(void) const { return m_data; } /// @todo replace with TNumeric

    virtual String nodetype(void) const;

    static String name(void) { return "Number"; }

protected:
    int m_data;
};


//..............................................................................
///////////////////////////////////////////////////////////////////// ColumnNode
///
/// @since 0.0.1
/// @brief Node for Column names
struct ColumnNode : public SimpleNode<String>
{
    ColumnNode(void);

    virtual ~ColumnNode(void) {}

    virtual void accept(Visitor &visitor);

    virtual String str(void) const;

    virtual String nodetype(void) const;

    static String name(void) { return "Column"; }
};


//..............................................................................
////////////////////////////////////////////////////////////////// ResColumnNode
///
/// @since 0.0.1
/// @brief Node for result column names
struct ResColumnNode : public SimpleNode<String>
{
    ResColumnNode(void);

    virtual ~ResColumnNode(void) {}

    virtual void accept(Visitor &visitor);

    virtual String str(void) const;

    virtual String nodetype(void) const;

    static String name(void) { return "ResColumn"; }
};



//..............................................................................
////////////////////////////////////////////////////////////////// ColumnNumNode
///
/// @since 0.0.1
/// @brief Node for Column numbers
struct ColumnNumNode : public Node
{
    ColumnNumNode(void);

    virtual ~ColumnNumNode(void) {}

    void init(String data);

    virtual void accept(Visitor &visitor);

    virtual String str(void) const;

    virtual int data(void) const;

    virtual String nodetype(void) const;

    static String name(void) { return "Column"; }

protected:
    int m_data;
};


//..............................................................................
/////////////////////////////////////////////////////////////// ResColumnNumNode
///
/// @since 0.0.1
/// @brief Node for result column numbers
struct ResColumnNumNode : public Node
{
    ResColumnNumNode(void);

    virtual ~ResColumnNumNode(void) {}

    void init(String data);

    virtual void accept(Visitor &visitor);

    virtual String str(void) const;

    virtual int data(void) const;

    virtual String nodetype(void) const;

    static String name(void) { return "ResColumn"; }

protected:
    int m_data;
};



//..............................................................................
////////////////////////////////////////////////////////////////////// ResIdNode
///
/// @since 0.0.1
/// @brief Node for result IDs
struct ResIdNode : public Node
{
    ResIdNode(void);

    virtual ~ResIdNode(void) {}

    void init(String data);

    virtual void accept(Visitor &visitor);

    virtual String str(void) const;

    virtual String data(void) const;

    virtual String nodetype(void) const;

    static String name(void) { return "ResId"; }

protected:
    String m_data;
};

//..............................................................................
/////////////////////////////////////////////////////////////////////// ConnNode
///
/// @since 0.0.1
/// @brief Node for CONNECTION command
struct ConnNode : public Node
{
    ConnNode(void);

    virtual ~ConnNode(void) {}

    void init(Identifier _id, ConnSpec *_spec);

    virtual void accept(Visitor &visitor);
    
    virtual String str(void) const;

    virtual String nodetype(void) const;
    
    static String name(void) { return "Connection"; }

    Identifier id;
    ConnSpec *spec;

private:
    ConnNode(const ConnNode&);
    ConnNode& operator=(const ConnNode&);
};


struct ConnSpec : public Node
{
    ConnSpec(void);

    virtual ~ConnSpec(void)
    {}

    void init(String _type, String _dbcstr);

    virtual void accept(Visitor &visitor);

    virtual String str(void) const;

    virtual String nodetype(void) const;

    static String name(void) { return "Connspec"; }

    String type;
    String dbcstr;
};


//..............................................................................
////////////////////////////////////////////////////////////////// ArgumentsNode
///
/// @since 0.0.1
/// @brief Node for Arguments
struct ArgumentsNode : public Node
{
    ArgumentsNode(void);

    virtual ~ArgumentsNode(void)
    {}

    virtual void accept(Visitor &visitor);

    virtual String str(void) const;

    virtual String nodetype(void) const;

    static String name(void) { return "Argument list"; }
};


//..............................................................................
/////////////////////////////////////////////////////////////// ColumnAssignNode
///
/// @since 0.0.1
/// @brief Node for Column Assignment
struct ColumnAssignNode : public Node
{
    ColumnAssignNode(void);

    virtual ~ColumnAssignNode(void)
    {}

    virtual void accept(Visitor &visitor);

    virtual String str(void) const;

    virtual String nodetype(void) const;

    static String name(void) { return "Column assign node"; }
};



//..............................................................................
////////////////////////////////////////////////////////////// ArgumentsSpecNode
///
/// @since 0.0.1
/// @brief Node for Argument specifications
struct ArgumentsSpecNode : public Node
{
    ArgumentsSpecNode(void);

    virtual ~ArgumentsSpecNode(void)
    {}

    virtual void accept(Visitor &visitor);

    virtual String str(void) const;

    virtual String nodetype(void) const;

    static String name(void) { return "Argument specification list"; }
};



//..............................................................................
////////////////////////////////////////////////////////////// TmplArgumentsNode
///
/// @since 0.0.1
/// @brief Node for template arguments
struct TmplArgumentsNode : public Node
{
    TmplArgumentsNode(void);

    virtual ~TmplArgumentsNode(void)
    {}

    virtual void accept(Visitor &visitor);

    virtual String str(void) const;

    virtual String nodetype(void) const;

    static String name(void) { return "Tmpl Argument list"; }
};



//..............................................................................
///////////////////////////////////////////////////////////////////// IdCallNode
///
/// @since 0.0.1
/// @brief Node for identifier calls used in template arguments
struct IdCallNode : public Node
{
    IdCallNode(void);

    virtual ~IdCallNode(void)
    {}

    virtual void accept(Visitor &visitor);

    virtual String str(void) const;

    virtual String nodetype(void) const;

    static String name(void) { return "ID call node"; }
};


enum tasktype
{
    ARGON_TASK_VOID,
    ARGON_TASK_TRANSFER,
    ARGON_TASK_FETCH,
    ARGON_TASK_STORE
};

//..............................................................................
/////////////////////////////////////////////////////////////////////// TaskNode
///
/// @since 0.0.1
/// @brief Node for TASK definitions
struct TaskNode : public Node
{
    TaskNode(void);

    virtual ~TaskNode(void)
    {}

    void init(Identifier _id, String type);

    virtual void accept(Visitor &visitor);

    virtual String str(void) const;

    virtual String nodetype(void) const;

    static String name(void) { return "Task"; }

    virtual void semanticCheck(SemanticCheck &sc);

    Identifier id;
    enum tasktype type;
};



//..............................................................................
/////////////////////////////////////////////////////////////////////// ExprNode
///
/// @since 0.0.1
/// @brief Node for Expressions
struct ExprNode : public Node
{
    enum mode { plus_expr, minus_expr, mul_expr, div_expr, concat_expr };

    ExprNode(void);

    virtual ~ExprNode(void)
    {}

    void init(enum mode m, Node *left, Node *right);

    virtual void accept(Visitor &visitor);

    virtual String str(void) const;

    enum mode data(void) const;

    virtual String nodetype(void) const;

    static String name(void) { return "Expr"; }

protected:
    enum mode m_mode;
};




//..............................................................................
///////////////////////////////////////////////////////////////////// ObjectNode
///
/// @since 0.0.1
/// @brief Node for Object (base class)
struct ObjectNode : public SimpleNode<Identifier>
{
    ObjectNode(void);

    virtual ~ObjectNode(void)
    {}


    static String name(void) { return "Object"; }
};



//..............................................................................
////////////////////////////////////////////////////////////////////// TableNode
///
/// @since 0.0.1
/// @brief Node for TABLE()
struct TableNode : public ObjectNode
{
    TableNode(void);

    virtual ~TableNode(void)
    {}

    void init(Identifier _id);

    virtual void accept(Visitor &visitor);
  
    virtual String str(void) const;

    virtual String nodetype(void) const;

    static String name(void) { return "Table"; }

    virtual void semanticCheck(SemanticCheck &sc);
};



//..............................................................................
//////////////////////////////////////////////////////////////////////// SqlNode
///
/// @since 0.0.1
/// @brief Node for SQL()
struct SqlNode : public ObjectNode
{
    SqlNode(void);

    virtual ~SqlNode(void)
    {}

    void init(Identifier _id);

    virtual void accept(Visitor &visitor);
  
    virtual String str(void) const;

    virtual String nodetype(void) const;

    static String name(void) { return "SQL"; }

    virtual void semanticCheck(SemanticCheck &sc);
};



//..............................................................................
//////////////////////////////////////////////////////////////////////// LogNode
///
/// @since 0.0.1
/// @brief Node for LOG command
struct LogNode : public Node
{
    LogNode(void);

    virtual ~LogNode(void)
    {}

    virtual void accept(Visitor &visitor);
    
    virtual String str(void) const;

    virtual String nodetype(void) const;
    
    static String name(void) { return "Log"; }
};



//..............................................................................
////////////////////////////////////////////////////////////////////// ParseTree
///
/// @since 0.0.1
/// @brief Prase Tree
class ParseTree : public Node
{
public:
    ParseTree(void)
        : m_nodelists(),
          m_nodes(),
          m_tokens(),
          m_anonymous_id_counter(0)
    {}

    virtual ~ParseTree(void);

    virtual void accept(Visitor &visitor);

    virtual String str(void) const;

    virtual String nodetype(void) const;
    static String name(void) { return "Parsetree"; }

    String gen_anonymous_id(void);

    virtual void raiseSyntaxError(void);

    virtual Token* newToken(const Token &t);

    inline NodeList* newNodeList(void)
    {
        NodeList* n = new NodeList();
        m_nodelists.push_back(n);
        return n;
    }

    /// @brief Create a new token node
    inline TokenNode* newTokenNode(Token *t)
    {
        TokenNode *node = new TokenNode(t);
        m_nodes.push_back(node);
        return node;
    }
   

    /// @brief Create new node and register it in the node list
    template<typename T>
    inline T* newNode(void)
    {
        T *tmp = new T();
        m_nodes.push_back(tmp); 
        return tmp;
    }

protected:
    /// @brief All allocated nodelists, cleaned up by the destructor
    std::list<NodeList*>  m_nodelists;

    /// @brief All allocated nodes, cleaned up by the destructor
    NodeList              m_nodes;

    /// @brief All allocated tokens, cleaned up by the destructor
    std::list<Token*>     m_tokens;

    size_t                m_anonymous_id_counter;

private:
    ParseTree(const ParseTree&);
    ParseTree& operator=(const ParseTree&);
};



/// @details
/// 
template<typename Op>
inline void foreach_node(Node::nodelist_type &nodes, Op op, int deep = -1)
{
    if(deep != 0)
    {
        Node::nodelist_type childs = nodes;
        for(Node::nodelist_type::iterator i = childs.begin();
            i != childs.end();
            ++i)
        {
            foreach_node(*i, op, deep);
        }
    }
}


/// @details
/// 
template<typename Op>
inline void foreach_node(Node *node, Op op, int deep = -1)
{
    if(deep != 0)
    {
        op(node);
        if(deep > 0)
            --deep;

        if(deep != 0)
        {
            Node::nodelist_type childs = node->getChilds();
            
            for(Node::nodelist_type::iterator i = childs.begin();
                i != childs.end();
                ++i)
            {
                foreach_node(*i, op, deep);
            }
        }
    }
}


/// @details
/// 
template<class Op>
inline void apply_visitor(Node *node, Op op)
{
    foreach_node(node, op, 1);
}



/// @details
/// 
template<class T>
std::list<T*> find_nodes(Node *node, int deep = -1)
{
    std::list<T*> nlist;

    for(Node::nodelist_type::iterator i = node->getChilds().begin();
        i != node->getChilds().end();
        ++i)
    {
        T* tmp = dynamic_cast<T*>(*i);
        if(tmp)
            nlist.push_back(tmp);
    }

    return nlist;
}


/// @details
/// 
template<class T>
T* find_node(Node *node, int deep = -1)
{
    for(Node::nodelist_type::iterator i = node->getChilds().begin();
        i != node->getChilds().end();
        ++i)
    {
        T* tmp = dynamic_cast<T*>(*i);
        if(tmp)
            return tmp;
    }

    return 0;
}


/// @details
/// 
template<typename T>
T* node_cast(Node *node)
{
    T* tmp = dynamic_cast<T*>(node);
    if(!tmp)
	{
        //throw std::runtime_error("invalid node cast");
        String s = "Type error: expecting `";
		s.append(T::name());
		s.append("` but got `");
        s.append(node->nodetype());
		s.append("` at position ");
		s.append(node->getSourceInfo().str());
		throw std::runtime_error(s);
	}
    return tmp;
}




/// @details
/// 
template<typename T>
inline T*
find_node_byid(Node* node, Identifier id, int deep = -1)
{
    for(Node::nodelist_type::iterator i = node->getChilds().begin();
        i != node->getChilds().end();
        ++i)
    {
        T* tmp = dynamic_cast<T*>(*i);
        if(tmp)
        {
            if(tmp->id == id)
                return tmp;
        }
    }
    return 0;
}




/// @details
///
template<typename T>
inline bool is_nodetype(Node *node)
{
    return dynamic_cast<T>(node) != 0 ? true : false;
}



//..............................................................................
/////////////////////////////////////////////////////////////// PrintTreeVisitor
///
/// @since 0.0.1
/// @brief Pretty printer for AST
struct PrintTreeVisitor : public Visitor
{
public:
    PrintTreeVisitor(Processor &proc, std::wostream &stream);

    PrintTreeVisitor(const PrintTreeVisitor& pt);

    mutable Processor &m_proc;
    String m_indent;
    mutable std::wostream &m_stream;
    
    void next(Node *node);
       
    virtual void fallback_action(Node *node);
};



std::ostream& operator<<(std::ostream &o, const Identifier &i);


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
