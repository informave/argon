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
String TableNode::str(void) const             { return String("tablenode ") + this->id.str(); }
String ArgumentsNode::str(void) const         { return "ArgumentsNode"; }
String ArgumentsSpecNode::str(void) const     { return "ArgumentsSpecNode"; }
String TmplArgumentsNode::str(void) const     { return "TmplArgumentsNode"; }
String IdCallNode::str(void) const            { return "IdCallNode"; }
String ColumnAssignNode::str(void) const      { return "ColumnAssignNode"; }
String ColumnNumNode::str(void) const         { return "ColumnNumNode"; }


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

#define DEFAULT_VISIT(type)                                         \
    void                                                            \
    Visitor::visit(type *node)                                      \
    {                                                               \
        this->fallback_action(node);                                \
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
    : m_data()
{}


/// @details
/// 
void
ColumnNode::init(String name)
{
    this->m_data = name;
}


/// @details
/// 
String
ColumnNode::data(void) const
{
    return this->m_data;
}


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
/////////////////////////////////////////////////////////////////// TaskExecNode

/// @details
/// 
TaskExecNode::TaskExecNode(void)
    : Node(),
      m_taskid()
{}


/// @details
/// 
void
TaskExecNode::init(Identifier id)
{
    this->m_taskid = id;
}


/// @details
/// 
Identifier
TaskExecNode::taskid(void) const
{
    return this->m_taskid;
}


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
////////////////////////////////////////////////////////////////////// TableNode


/// @details
/// 
ObjectNode::ObjectNode(void)
    : Node(),
      id()
{}

/// @details
/// 
void
ObjectNode::init(Identifier _id)
{
    this->id = _id;
}



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
    : Node(),
      m_data()
{}


/// @details
/// 
void
LiteralNode::init(String data)
{
    this->m_data = data;
}

//..............................................................................
///////////////////////////////////////////////////////////////////////// IdNode

/// @details
/// 
IdNode::IdNode(void)
    : Node(),
      m_data()
{}


/// @details
/// 
void
IdNode::init(Identifier _id)
{
    m_data = _id;
}


/// @details
/// 
Identifier
IdNode::data(void) const
{
    return m_data;
}

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
    id = _id;
    type = _type;
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
    Token *t = *this->m_tokens.rbegin();
    throw SyntaxError(t);
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

       


void
PrintTreeVisitor::visit(ConnNode *node)
{
    m_stream << this->m_indent << "ConnNode: " << node->str() << std::endl;
    next(node);
}


void
PrintTreeVisitor::visit(TaskNode *node)
{
    m_stream << this->m_indent << "TaskNode: " << node->str() << std::endl;
    next(node);
}

void
PrintTreeVisitor::visit(ParseTree *node)
{
    m_stream << this->m_indent << "ROOT NODE" << std::endl;
    next(node);
}


void
PrintTreeVisitor::visit(LogNode *node)
{
    m_stream << this->m_indent << "LogNode: " << node->str() << std::endl;
    next(node);
}

void
PrintTreeVisitor::visit(TaskExecNode *node)
{
    m_stream << this->m_indent << "TaskExecNode: " << node->str() << std::endl;
    next(node);
}


void
PrintTreeVisitor::visit(ColumnNode *node)
{
    m_stream << this->m_indent << "ColumnNode: " << node->data() << std::endl;
    next(node);
}

void
PrintTreeVisitor::visit(LiteralNode *node)
{
    m_stream << this->m_indent << "LiteralNode: " << node->str() << std::endl;
    next(node);
}


void
PrintTreeVisitor::visit(SqlExecNode *node)
{
    m_stream << this->m_indent << "SqlExecNode: " << node->str() << std::endl;
    next(node);
}



void
PrintTreeVisitor::visit(TableNode *node)
{
    m_stream << this->m_indent << "TableNode: " << node->str()  << std::endl;
    next(node);
}



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
