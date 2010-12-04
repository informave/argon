//
// ast.hh - Abstract Syntax Tree
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
/// @brief Abstract Syntax Tree (nodes and tree classes)
/// @author Daniel Vogelbacher
/// @since 0.1


#ifndef INFORMAVE_ARGON_AST_HH
#define INFORMAVE_ARGON_AST_HH

#include "argon/fwd.hh"
#include "argon/token.hh"

#include <list>

ARGON_NAMESPACE_BEGIN

struct Node;
struct ConnNode;
struct ConnSpec;
struct TaskNode;
struct Identifier;
struct IdNode;
struct LogNode;
struct TaskExecNode;
struct LiteralNode;
struct ColumnNode;
class Visitor;
class ParseTree;

typedef std::list<Node*> NodeList;




//--------------------------------------------------------------------------
/// Node base class
///
/// @since 0.0.1
/// @brief Node base class
class Node
{
public:
    typedef NodeList nodelist_type;

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


protected:
    nodelist_type m_childs;
    SourceInfo m_sinfo;
};



//--------------------------------------------------------------------------
/// Visitor base class
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

    void operator()(Node *node);

protected:
    visitor_mode m_mode;
};






struct Identifier
{
    Identifier() { }

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



struct TaskExecNode : public Node
{
    TaskExecNode(void);
    void init(Identifier id);

    virtual void accept(Visitor &visitor);
    virtual ~TaskExecNode(void) {}

    virtual String str(void) const;

    Identifier taskid(void) const;

    Identifier m_taskid;

};



struct TokenNode : public Node
{
    TokenNode(Token *tok);
    virtual void accept(Visitor &visitor);
    virtual ~TokenNode(void) {}
    virtual String str(void) const;
    
protected:
    Token *m_token;
};

struct IdNode : public Node
{
    IdNode(void);
    void init(Identifier _id);
    virtual void accept(Visitor &visitor);
    virtual ~IdNode(void) {}

    virtual String str(void) const;

    Identifier data(void) const;

    Identifier m_data;
};


struct LiteralNode : public Node
{
    LiteralNode(void);

    void init(String data);

    virtual void accept(Visitor &visitor);
    virtual ~LiteralNode(void) {}

    virtual String str(void) const;

    String m_data;
};


struct ColumnNode : public Node
{
    ColumnNode(void);

    void init(String data);

    virtual void accept(Visitor &visitor);
    virtual ~ColumnNode(void) {}

    virtual String str(void) const;

    virtual String colname(void) const;

    String m_data;
};


struct ConnNode : public Node
{
    ConnNode(void);

    void init(Identifier _id, ConnSpec *_spec);

    virtual void accept(Visitor &visitor);

    Identifier id;
    ConnSpec *spec;
    
    virtual ~ConnNode(void)
    {}


    virtual String str(void) const;
};


struct ConnSpec : public Node
{
    ConnSpec(void);

    void init(String _type, String _dbcstr);

    virtual void accept(Visitor &visitor);

    String type;
    String dbcstr;


    virtual ~ConnSpec(void)
    {}

    virtual String str(void) const;
};



struct TaskNode : public Node
{
    TaskNode(void);

    void init(Identifier _id);

    virtual void accept(Visitor &visitor);

    Identifier id;

    virtual ~TaskNode(void)
    {}

    virtual String str(void) const;
};




struct LogNode : public Node
{
    LogNode(void);

    virtual void accept(Visitor &visitor);
    
    virtual ~LogNode(void)
    {}

    virtual String str(void) const;



};




//--------------------------------------------------------------------------
/// Parse Tree
///
/// @since 0.0.1
/// @brief Prase Tree
class ParseTree : public Node
{
public:
    virtual ~ParseTree(void);

    virtual void accept(Visitor &visitor);

    virtual String str(void) const;


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
};




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





struct PrintTreeVisitor : public Visitor
{
public:
    PrintTreeVisitor(Processor &proc, std::wostream &stream);


    PrintTreeVisitor(const PrintTreeVisitor& pt);

    mutable Processor &m_proc;
    String m_indent;
    mutable std::wostream &m_stream;
    

    void next(Node *node);
       
    virtual void visit(ConnNode *node);
    virtual void visit(TaskNode *node);
    virtual void visit(ParseTree *node);
    virtual void visit(IdNode *node);
    virtual void visit(LiteralNode *node);
    virtual void visit(LogNode *node);
    virtual void visit(TaskExecNode *node);
    virtual void visit(ColumnNode *node);


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
