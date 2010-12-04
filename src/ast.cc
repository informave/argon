//
// ast.cc - Abstract Syntax Tree
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
/// @brief Abstract Syntax Tree (nodes and tree classes) (definition)
/// @author Daniel Vogelbacher
/// @since 0.1


#include "argon/ast.hh"

#include "argon/dtsengine.hh"
#include "argon/token.hh"
#include "argon/exceptions.hh"


#include <cstdlib>
#include <cstdio>
#include <memory>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <typeinfo>

#define PDEBUG(name) std::cout << name << std::endl


ARGON_NAMESPACE_BEGIN



void ConnNode::accept(Visitor &visitor)     { visitor.visit(this); }
void ConnSpec::accept(Visitor &visitor)     { /* visitor.visit(*this);*/ }
void TaskNode::accept(Visitor &visitor)     { visitor.visit(this); }
void ParseTree::accept(Visitor &visitor)    { visitor.visit(this); }
void LogNode::accept(Visitor &visitor)      { visitor.visit(this); }
void LiteralNode::accept(Visitor &visitor)  { visitor.visit(this); }
void IdNode::accept(Visitor &visitor)       { visitor.visit(this); }
void TaskExecNode::accept(Visitor &visitor) { visitor.visit(this); }
void ColumnNode::accept(Visitor &visitor)   { visitor.visit(this); }
void TokenNode::accept(Visitor &visitor)    { /* visitor.visit(this); */ }



String ConnNode::str(void) const     { return ""; }
String ConnSpec::str(void) const     { return ""; }
String TaskNode::str(void) const     { return this->id.str(); }
String ParseTree::str(void) const    { return ""; }
String LogNode::str(void) const      { return ""; }
String LiteralNode::str(void) const  { return this->m_data; }
String IdNode::str(void) const       { return String("IdNode: ") + this->data().str(); }
String TaskExecNode::str(void) const       { return "taskexecnode"; }
String ColumnNode::str(void) const       { return "columnnode"; }
String TokenNode::str(void) const       { return "tokennode"; }




//..............................................................................
/////////////////////////////////////////////////////////////////////////// Node

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



#define DEFAULT_VISIT(type)                                         \
    void                                                            \
    Visitor::visit(type *node)                                      \
    {                                                               \
        if(this->m_mode == ignore_none)                             \
        {                                                           \
            std::string                                             \
                s("Visitor for " #type " is not implemented in <"); \
            s.append(typeid(*this).name());                         \
            s.append(">");                                          \
            throw std::runtime_error(s);                            \
        }                                                           \
    }

DEFAULT_VISIT(ConnNode)
DEFAULT_VISIT(TaskNode)
DEFAULT_VISIT(ParseTree)
DEFAULT_VISIT(LogNode)
DEFAULT_VISIT(IdNode)
DEFAULT_VISIT(LiteralNode)
DEFAULT_VISIT(TaskExecNode)
DEFAULT_VISIT(ColumnNode)


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
ColumnNode::colname(void) const
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
    : id()
{}


/// @details
/// 
void
TaskNode::init(Identifier _id)
{
    id = _id;
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

    std::cout << "Connection: " << id << " Spec: " << spec->type << " " << spec->dbcstr
              << std::endl;
}


/// @details
/// 
ConnSpec::ConnSpec(void)
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



PrintTreeVisitor::PrintTreeVisitor(Processor &proc, std::wostream &stream)
    : Visitor(),
      m_proc(proc),
      m_indent(""),
      m_stream(stream)
{}


PrintTreeVisitor::PrintTreeVisitor(const PrintTreeVisitor& pt)
    : Visitor(),
      m_proc(pt.m_proc),
      m_indent(pt.m_indent),
      m_stream(pt.m_stream)
{
    m_indent.append("   ");
}


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
PrintTreeVisitor::visit(IdNode *node)
{
    m_stream << this->m_indent << "IdNode: " << node->str() << std::endl;
    next(node);
}

void
PrintTreeVisitor::visit(ColumnNode *node)
{
    m_stream << this->m_indent << "ColumnNode: " << node->colname() << std::endl;
    next(node);
}

void
PrintTreeVisitor::visit(LiteralNode *node)
{
    m_stream << this->m_indent << "LiteralNode: " << node->str() << std::endl;
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
