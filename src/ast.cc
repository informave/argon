//
// ast.cc - Abstract Syntax Tree
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
/// @brief Abstract Syntax Tree (nodes and tree classes) (definition)
/// @author Daniel Vogelbacher
/// @since 0.1


#include "argon/ast.hh"

#include "argon/dtsengine.hh"
#include "argon/token.hh"
#include "argon/exceptions.hh"
#include "debug.hh"


#include <cstdlib>
#include <cstdio>
#include <memory>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <typeinfo>

#define PDEBUG(name) std::cout << name << std::endl


ARGON_NAMESPACE_BEGIN


void TokenNode::accept(Visitor &visitor)    {}
void ConnSpec::accept(Visitor &visitor)     {}

void ConnNode::accept(Visitor &visitor)           { visitor.visit(this); }
void TaskNode::accept(Visitor &visitor)           { visitor.visit(this); }
void ParseTree::accept(Visitor &visitor)          { visitor.visit(this); }
void LogNode::accept(Visitor &visitor)            { visitor.visit(this); }
void LiteralNode::accept(Visitor &visitor)        { visitor.visit(this); }
void IdNode::accept(Visitor &visitor)             { visitor.visit(this); }
void TaskExecNode::accept(Visitor &visitor)       { visitor.visit(this); }
void ColumnNode::accept(Visitor &visitor)         { visitor.visit(this); }
void SqlExecNode::accept(Visitor &visitor)        { visitor.visit(this); }
void TableNode::accept(Visitor &visitor)          { visitor.visit(this); }
void ArgumentsNode::accept(Visitor &visitor)      { visitor.visit(this); }
void ArgumentsSpecNode::accept(Visitor &visitor)  { visitor.visit(this); }
void TmplArgumentsNode::accept(Visitor &visitor)  { visitor.visit(this); }
void IdCallNode::accept(Visitor &visitor)         { visitor.visit(this); }
void ColumnAssignNode::accept(Visitor &visitor)   { visitor.visit(this); }
void ColumnNumNode::accept(Visitor &visitor)      { visitor.visit(this); }
void NumberNode::accept(Visitor &visitor)         { visitor.visit(this); }
void ExprNode::accept(Visitor &visitor)           { visitor.visit(this); }
void FuncCallNode::accept(Visitor &visitor)       { visitor.visit(this); }
void ResColumnNode::accept(Visitor &visitor)      { visitor.visit(this); }
void ResColumnNumNode::accept(Visitor &visitor)   { visitor.visit(this); }
void ResIdNode::accept(Visitor &visitor)          { visitor.visit(this); }

String ConnNode::str(void) const              { return "ConnNode"; }
String ConnSpec::str(void) const              { return "ConnSpec"; }
String TaskNode::str(void) const              { return this->id.str(); }
String ParseTree::str(void) const             { return "ParseTree"; }
String LogNode::str(void) const               { return "LogNode"; }
String LiteralNode::str(void) const           { return this->m_data; }
String IdNode::str(void) const                { return String("IdNode: ") + this->data().str(); }
String TaskExecNode::str(void) const          { return "taskexecnode"; }
String ColumnNode::str(void) const            { return "columnnode"; }
String TokenNode::str(void) const             { return "tokennode"; }
String SqlExecNode::str(void) const           { return "sqlexecnode"; }
String TableNode::str(void) const             { return String("tablenode ") + this->data().str(); }
String ArgumentsNode::str(void) const         { return "ArgumentsNode"; }
String ArgumentsSpecNode::str(void) const     { return "ArgumentsSpecNode"; }
String TmplArgumentsNode::str(void) const     { return "TmplArgumentsNode"; }
String IdCallNode::str(void) const            { return "IdCallNode"; }
String ColumnAssignNode::str(void) const      { return "ColumnAssignNode"; }
String ColumnNumNode::str(void) const         { return "ColumnNumNode"; }
String FuncCallNode::str(void) const          { return "FuncCallNode"; }
String ResColumnNode::str(void) const         { return "ResColumnNode"; }
String ResColumnNumNode::str(void) const      { return "ResColumnNumNode"; }
String ResIdNode::str(void) const             { return "ResIdNode"; }

String NumberNode::str(void) const
{
    std::wstringstream ss;
    ss << "NumberNode " << this->data();
    return String(ss.str());
}


String ConnNode::nodetype(void) const                { return ConnNode::name();            }
String ConnSpec::nodetype(void) const                { return ConnSpec::name();            }
String TaskNode::nodetype(void) const                { return TaskNode::name();            }
String ParseTree::nodetype(void) const               { return ParseTree::name();           }
String LogNode::nodetype(void) const                 { return LogNode::name();             }
String LiteralNode::nodetype(void) const             { return LiteralNode::name();         }
String IdNode::nodetype(void) const                  { return IdNode::name();              }
String TaskExecNode::nodetype(void) const            { return TaskExecNode::name();        }
String ColumnNode::nodetype(void) const              { return ColumnNode::name();          }
String TokenNode::nodetype(void) const               { return TokenNode::name();           }
String SqlExecNode::nodetype(void) const             { return SqlExecNode::name();         }
String TableNode::nodetype(void) const               { return TableNode::name();           }
String ArgumentsNode::nodetype(void) const           { return ArgumentsNode::name();       }
String ArgumentsSpecNode::nodetype(void) const       { return ArgumentsSpecNode::name();   }
String TmplArgumentsNode::nodetype(void) const       { return TmplArgumentsNode::name();   }
String IdCallNode::nodetype(void) const              { return IdCallNode::name();          }
String ColumnAssignNode::nodetype(void) const        { return ColumnAssignNode::name();    }
String ColumnNumNode::nodetype(void) const           { return ColumnNumNode::name();       }
String NumberNode::nodetype(void) const              { return NumberNode::name();          }
String ExprNode::nodetype(void) const                { return ExprNode::name();            }
String FuncCallNode::nodetype(void) const            { return FuncCallNode::name();            }
String ResColumnNode::nodetype(void) const            { return ResColumnNode::name();            }
String ResColumnNumNode::nodetype(void) const            { return ResColumnNumNode::name();            }
String ResIdNode::nodetype(void) const            { return ResIdNode::name();            }

//..............................................................................
/////////////////////////////////////////////////////////////////////////// Node

/// @details
/// 
Node::Node(void)
    : m_childs(),
      m_sinfo()
{}


/// @details
/// 
Node::~Node(void)
{}


/// @details
/// 
SourceInfo
Node::getSourceInfo(void) const
{
    return this->m_sinfo;
}


/// @details
/// 
void
Node::addChilds(const NodeList *list)
{
    assert(list && "Nodelist is null");
    std::copy(list->begin(), list->end(), std::back_insert_iterator<NodeList>(this->m_childs));

    for(NodeList::const_iterator i = list->begin();
        i != list->end();
        ++i)
    {
        this->updateSourceInfo((*i)->getSourceInfo());
    }
}


/// @details
/// 
void 
Node::addChild(Node *child)
{
    ARGON_ICERR(!!child,
                "child node is null");

    this->m_childs.push_back(child);
    this->updateSourceInfo(child->getSourceInfo());
}


/// @details
/// 
void
Node::updateSourceInfo(const SourceInfo &info)
{
    m_sinfo.expand(info);
}



//..............................................................................
//////////////////////////////////////////////////////////////////////// Visitor

/// @details
/// 
Visitor::Visitor(visitor_mode mode)
    : m_mode(mode)
{}


void
Visitor::fallback_action(Node *node)
{
    if(this->m_mode == ignore_none)
    {
        std::string  s(std::string("Visitor for ") +typeid(*node).name()+ " is not implemented in <");
        s.append(typeid(*this).name());
        s.append(">");
        throw std::runtime_error(s);
    }
}

#define DEFAULT_VISIT(type)                     \
    void                                        \
    Visitor::visit(type *node)                  \
    {                                           \
        this->fallback_action(node);            \
    }

DEFAULT_VISIT(ConnNode)
DEFAULT_VISIT(TaskNode)
DEFAULT_VISIT(ParseTree)
DEFAULT_VISIT(LogNode)
DEFAULT_VISIT(IdNode)
DEFAULT_VISIT(LiteralNode)
DEFAULT_VISIT(TaskExecNode)
DEFAULT_VISIT(ColumnNode)
DEFAULT_VISIT(SqlExecNode)
DEFAULT_VISIT(TableNode)
DEFAULT_VISIT(ArgumentsNode)
DEFAULT_VISIT(ArgumentsSpecNode)
DEFAULT_VISIT(TmplArgumentsNode)
DEFAULT_VISIT(IdCallNode)
DEFAULT_VISIT(ColumnAssignNode)
DEFAULT_VISIT(ColumnNumNode)
DEFAULT_VISIT(NumberNode)
DEFAULT_VISIT(ExprNode)
DEFAULT_VISIT(FuncCallNode)
DEFAULT_VISIT(ResColumnNode)
DEFAULT_VISIT(ResColumnNumNode)
DEFAULT_VISIT(ResIdNode)

/// @details
/// 
void
Visitor::operator()(Node *node)
{
    node->accept(*this);
}



//..............................................................................
////////////////////////////////////////////////////////////////////// TokenNode

/// @details
/// 
TokenNode::TokenNode(Token *tok)
    : m_token(tok)
{
    m_sinfo = tok->getSourceInfo();
}



//..............................................................................
///////////////////////////////////////////////////////////////////// ColumnNode

/// @details
/// 
ColumnNode::ColumnNode(void)
    : SimpleNode<String>()
{}


//..............................................................................
////////////////////////////////////////////////////////////////// ResColumnNode

/// @details
///
ResColumnNode::ResColumnNode(void)
    : SimpleNode<String>()
{}



//..............................................................................
////////////////////////////////////////////////////////////////// ColumnNumNode

/// @details
/// 
ColumnNumNode::ColumnNumNode(void)
    : m_data()
{}


/// @details
/// 
void
ColumnNumNode::init(String name)
{
    assert(name.length() > 0);
    db::Variant v(name);
    this->m_data = v.asInt();
}


/// @details
/// 
int
ColumnNumNode::data(void) const
{
    return this->m_data;
}


//..............................................................................
/////////////////////////////////////////////////////////////// ResColumnNumNode

/// @details
///
ResColumnNumNode::ResColumnNumNode(void)
    : m_data()
{}


/// @details
///
void
ResColumnNumNode::init(String name)
{
    assert(name.length() > 0);
    db::Variant v(name);
    this->m_data = v.asInt();
}


/// @details
///
int
ResColumnNumNode::data(void) const
{
    return this->m_data;
}

//..............................................................................
////////////////////////////////////////////////////////////////////// ResIdNode

/// @details
///
ResIdNode::ResIdNode(void)
    : m_data()
{}


/// @details
///
void
ResIdNode::init(String name)
{
    assert(name.length() > 0);
    db::Variant v(name);
    this->m_data = v;
}


/// @details
///
String
ResIdNode::data(void) const
{
    return this->m_data;
}


//..............................................................................
/////////////////////////////////////////////////////////////////// TaskExecNode

/// @details
/// 
TaskExecNode::TaskExecNode(void)
    : SimpleNode<Identifier>()
{}



//..............................................................................
////////////////////////////////////////////////////////////////// ArgumentsNode

/// @details
/// 
ArgumentsNode::ArgumentsNode(void)
    : Node()
{}


//..............................................................................
/////////////////////////////////////////////////////////////// ColumnAssignNode

/// @details
/// 
ColumnAssignNode::ColumnAssignNode(void)
    : Node()
{}




//..............................................................................
////////////////////////////////////////////////////////////// ArgumentsSpecNode

/// @details
/// 
ArgumentsSpecNode::ArgumentsSpecNode(void)
    : Node()
{}



//..............................................................................
////////////////////////////////////////////////////////////// TmplArgumentsNode

/// @details
/// 
TmplArgumentsNode::TmplArgumentsNode(void)
    : Node()
{}


//..............................................................................
/////////////////////////////////////////////////////////////// ArgumentItemNode

/// @details
/// 
/*
  ArgumentItemNode::ArgumentItemNode(void)
  : Node()
  {}
*/


//..............................................................................
///////////////////////////////////////////////////////////////////// IdCallNode

/// @details
/// 
IdCallNode::IdCallNode(void)
    : Node()
{}



//..............................................................................
/////////////////////////////////////////////////////////////////// FuncCallNode


/// @details
/// 
FuncCallNode::FuncCallNode(void)
    : SimpleNode<Identifier>()
{}


//..............................................................................
/////////////////////////////////////////////////////////////////////// ExprNode


/// @details
/// 
ExprNode::ExprNode(void)
    : Node(),
      m_mode()
{}

/// @details
/// 
void
ExprNode::init(ExprNode::mode m, Node *left, Node *right)
{
    this->m_mode = m;
    this->addChild(left);
    this->addChild(right);
}


/// @details
/// 
ExprNode::mode
ExprNode::data(void) const
{
    return this->m_mode;
}


String
ExprNode::str(void) const
{
    String s("ExprNode ");
    switch(m_mode)
    {
    case ExprNode::plus_expr:
        s.append("+"); break;
    case ExprNode::minus_expr:
        s.append("-"); break;
    case ExprNode::mul_expr:
        s.append("*"); break;
    case ExprNode::div_expr:
        s.append("/"); break;
    case ExprNode::concat_expr:
        s.append("||"); break;
    };
    return s;
}

//..............................................................................
///////////////////////////////////////////////////////////////////// ObjectNode


/// @details
/// 
ObjectNode::ObjectNode(void)
    : SimpleNode<Identifier>()
{}



//..............................................................................
////////////////////////////////////////////////////////////////////// TableNode


/// @details
/// 
TableNode::TableNode(void)
    : ObjectNode()
{}


/// @details
/// 
void
TableNode::init(Identifier id)
{
    ObjectNode::init(id);
}





//..............................................................................
//////////////////////////////////////////////////////////////////// SqlExecNode

/// @details
/// 
SqlExecNode::SqlExecNode(void)
    : Node(),
      m_connid(),
      m_sql()
{}


/// @details
/// 
void
SqlExecNode::init(String sql, Identifier id)
{
    this->m_sql = sql;
    this->m_connid = id;
}


/// @details
/// 
Identifier
SqlExecNode::connid(void) const
{
    return this->m_connid;
}


/// @details
/// 
String
SqlExecNode::sql(void) const
{
    return this->m_sql;
}



//..............................................................................
//////////////////////////////////////////////////////////////////// LiteralNode

/// @details
/// 
LiteralNode::LiteralNode(void)
    : SimpleNode<String>()
{}



//..............................................................................
///////////////////////////////////////////////////////////////////// NumberNode

/// @details
/// 
NumberNode::NumberNode(void)
    : Node(),
      m_data()
{}


/// @details
/// 
void
NumberNode::init(String data)
{
    assert(data.length() > 0);
    db::Variant v(data);
    this->m_data = v.asInt();
}


//..............................................................................
///////////////////////////////////////////////////////////////////////// IdNode

/// @details
/// 
IdNode::IdNode(void)
    : SimpleNode<Identifier>()
{}


//..............................................................................
/////////////////////////////////////////////////////////////////////// TaskNode

/// @details
/// 
TaskNode::TaskNode(void)
    : id(),
      type()
{}


/// @details
/// 
void
TaskNode::init(Identifier _id, String _type)
{
    this->id = _id;

    if(_type == "VOID")
        this->type = ARGON_TASK_VOID;
    else if(_type == "FETCH")
        this->type = ARGON_TASK_FETCH;
    else if(_type == "STORE")
        this->type = ARGON_TASK_STORE;
    else if(_type == "TRANSFER")
        this->type = ARGON_TASK_TRANSFER;
    else
    {
        ARGON_ICERR(false, String("Unknown task type: ").append(_type).append(" at ").append(this->getSourceInfo().str()));
    }
}



//..............................................................................
/////////////////////////////////////////////////////////////////////// ConnNode

/// @details
/// 
ConnNode::ConnNode(void)
    : id(),
      spec()
{}


/// @details
/// 
void
ConnNode::init(Identifier _id, ConnSpec *_spec)
{
    this->id = _id;
    this->spec = _spec;

    ARGON_DPRINT(ARGON_MOD_CONN, "Connection: " << id << " Spec: " << spec->type << " " << spec->dbcstr);
}


/// @details
/// 
ConnSpec::ConnSpec(void)
    : type(),
      dbcstr()
{}


/// @details
/// 
void
ConnSpec::init(String _type, String _dbcstr)
{
    this->type = _type;
    this->dbcstr = _dbcstr;
}


//..............................................................................
//////////////////////////////////////////////////////////////////////// LogNode

/// @details
/// 
LogNode::LogNode(void)
{}



//..............................................................................
////////////////////////////////////////////////////////////////////// ParseTree


/// @details
/// 
Token*
ParseTree::newToken(const Token& t)
{
    Token* tok = new Token(t);
    this->m_tokens.push_back(tok);
    return tok;
}



struct delete_ptr
{
    template<class T>
    inline void operator()(T& ref)
    {
        delete ref;
    }
};


/// @details
/// 
ParseTree::~ParseTree(void)
{
    std::for_each(m_nodes.begin(), m_nodes.end(), delete_ptr());
    std::for_each(m_tokens.begin(), m_tokens.end(), delete_ptr());
    std::for_each(m_nodelists.begin(), m_nodelists.end(), delete_ptr());
}


/// @details
/// 
void
ParseTree::raiseSyntaxError(void)
{
    if(this->m_tokens.size())
    {
        Token *t = *this->m_tokens.rbegin();
        throw SyntaxError(t);
    }
    else
        throw SyntaxError(0);
}




//..............................................................................
/////////////////////////////////////////////////////////////// PrintTreeVisitor

/// @details
/// 
PrintTreeVisitor::PrintTreeVisitor(Processor &proc, std::wostream &stream)
    : Visitor(Visitor::ignore_none),
      m_proc(proc),
      m_indent(""),
      m_stream(stream)
{}


/// @details
/// 
PrintTreeVisitor::PrintTreeVisitor(const PrintTreeVisitor& pt)
    : Visitor(Visitor::ignore_none),
      m_proc(pt.m_proc),
      m_indent(pt.m_indent),
      m_stream(pt.m_stream)
{
    m_indent.append("   ");
}


/// @details
/// 
void 
PrintTreeVisitor::next(Node *node)
{
    foreach_node(node->getChilds(), PrintTreeVisitor(*this), 1);
}



/// @details
/// 
void
PrintTreeVisitor::fallback_action(Node *node)
{
    m_stream << this->m_indent << node->str()  << std::endl;
    next(node);
}



/// @details
/// 
std::ostream& operator<<(std::ostream &o, const Identifier &i)
{
    o << i.name();
    return o;
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
