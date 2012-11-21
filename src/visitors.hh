//
// visitors.hh - Visitors
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
/// @brief Visitors
/// @author Daniel Vogelbacher
/// @since 0.1


#ifndef INFORMAVE_ARGON_VISITORS_HH
#define INFORMAVE_ARGON_VISITORS_HH

#include "argon/dtsengine.hh"
#include "argon/exceptions.hh"
#include "argon/ast.hh"
#include "debug.hh"

#include <iostream>
#include <sstream>
#include <list>

ARGON_NAMESPACE_BEGIN


//..............................................................................
/////////////////////////////////////////////////////////////////// BlockVisitor
///
/// @since 0.0.1
/// @brief Processor initial tree walker
class BlockVisitor : public CVisitor<Context>
{
public:
    BlockVisitor(Processor &proc, Context &ctx, Value &returnVal);

    virtual void visit(VarNode *node);
    //virtual void visit(BinaryExprNode *node);
    //virtual void visit(AssignNode *node);
    virtual void visit(WhileNode *node);
    virtual void visit(RepeatNode *node);
    virtual void visit(ForNode *node);
    virtual void visit(ContinueNode *node);
    virtual void visit(BreakNode *node);
    virtual void visit(ReturnNode *node);
    virtual void visit(IfelseNode *node);
    virtual void visit(AssertNode *node);

    virtual void visit(CompoundNode *node);

protected:
    virtual void fallback_action(Node *node);

	Value &m_returnVal;
};



//..............................................................................
///////////////////////////////////////////////////////////////// ProcTreeWalker
///
/// @since 0.0.1
/// @brief Processor initial tree walker
class Pass1Visitor : public Visitor
{
public:
    Pass1Visitor(Processor &proc);

    virtual void visit(ConnNode *node);
    virtual void visit(VarNode *node);
    virtual void visit(TaskNode *node);
    virtual void visit(FunctionNode *node);
    //virtual void visit(ParseTree *node);
    virtual void visit(LogNode *node);
    virtual void visit(IdNode *node);
    virtual void visit(LiteralNode *node);

    virtual void visit(DeclNode *node);

protected:
    inline Processor& proc(void) { return m_proc; }
    Processor &m_proc;
};

//..............................................................................
///////////////////////////////////////////////////////////////// ProcTreeWalker
///
/// @since 0.0.1
/// @brief Processor initial tree walker
class Pass2Visitor : public Visitor
{
public:
    Pass2Visitor(Processor &proc, Context &context);

    virtual void visit(ConnNode *node);
    virtual void visit(VarNode *node);
    virtual void visit(TaskNode *node);
    virtual void visit(FunctionNode *node);
    //virtual void visit(ParseTree *node);
    virtual void visit(LogNode *node);
    virtual void visit(IdNode *node);
    virtual void visit(LiteralNode *node);


    virtual void visit(DeclNode *node);

protected:
    inline Processor& proc(void) { return m_proc; }
    inline Context& context(void) { return m_context; }
    Processor &m_proc;
    Context &m_context;
};



//..............................................................................
//////////////////////////////////////////////////////////////// EvalExprVisitor
///
/// @since 0.0.1
/// @brief Evaluates each node and writes the result to the given value reference
///
/// @note
/// EvalExprVisitor calls itself to evaluate subnodes
struct EvalExprVisitor : public CVisitor<Context>
{
    EvalExprVisitor(Processor &proc, Context &context, Value &value);
    
    virtual void visit(ExprNode *node);
    virtual void visit(NullNode *node);
    virtual void visit(NumberNode *node);
    virtual void visit(IdNode *node);
    virtual void visit(LiteralNode *node);
    virtual void visit(ColumnNode *node);
    virtual void visit(ColumnNumNode *node);
    virtual void visit(FuncCallNode *node);
    virtual void visit(ResColumnNode *node);
    virtual void visit(ResColumnNumNode *node);
    virtual void visit(ResIdNode *node);

    //virtual void visit(VarNode *node);
    virtual void visit(AssignNode *node);
    virtual void visit(BinaryExprNode *node);
    virtual void visit(UnaryExprNode *node);

protected:
    Processor &m_proc;
    Context &m_context;
    Value   &m_value;
};




//..............................................................................
/////////////////////////////////////////////////////////////// ArgumentsVisitor
///
/// @since 0.0.1
/// @brief Evaluates each argument and writes the value into the argument list
/// @warning Remember to use ScopedStackFrame before using this visitor
/// because it may create new values on the stack!
struct ArgumentsVisitor : public Visitor
{
public:
    ArgumentsVisitor(Processor &proc, Context &context, ArgumentList &list);

    virtual void visit(IdNode *node);

protected:
    virtual void fallback_action(Node *node);

    Processor &m_proc;
    Context &m_context;
    ArgumentList &m_list;
};



//..............................................................................
///////////////////////////////////////////////////////////////// Arg2SymVisitor
///
/// @since 0.0.1
/// @brief Writes each argument into the symbol table of the context
struct Arg2SymVisitor : public Visitor
{
public:
    Arg2SymVisitor(Processor &proc, SymbolTable &symboltable,
                   ArgumentList::const_iterator &i, ArgumentList::const_iterator end);

    // only IdNode support required, because argument declaration only contains IDs.
    virtual void visit(IdNode *node);

protected:
    Processor &m_proc;
    //Context &m_context;
    SymbolTable &m_symboltable;
    ArgumentList::const_iterator &m_arg_iterator;
    ArgumentList::const_iterator m_end_iterator;
};



//..............................................................................
/////////////////////////////////////////////////////////////// TaskChildVisitor
///
/// @since 0.0.1
/// @brief Task Child Visitor
struct TaskChildVisitor : public Visitor
{
public:
    TaskChildVisitor(Processor &proc, Context &context);

    virtual void visit(LogNode *node);
    
    virtual void visit(TaskExecNode *node);

    virtual void visit(SqlExecNode *node);

    virtual void visit(ArgumentsSpecNode *node);

    virtual void visit(TmplArgumentsNode *node);

    virtual void visit(ColumnAssignNode *node);


private:
    Processor &m_proc;
    Context &m_context;
    //Task      &m_task;
};




//..............................................................................
/////////////////////////////////////////////////////////////// ResColumnVisitor
///
/// @since 0.0.1
/// @brief Result Column Visitor
struct ResColumnVisitor : public Visitor
{
public:
    ResColumnVisitor(Processor &proc, Context &context, ColumnList &list);
    
    virtual void visit(ResColumnNode *node);

    virtual void visit(ResColumnNumNode *node);

protected:
    Processor &m_proc;
    Context &m_context;
    ColumnList &m_list;
};



//..............................................................................
////////////////////////////////////////////////////////////// DeepColumnVisitor
///
/// @since 0.0.1
/// @brief Deep Column Visitor
struct DeepColumnVisitor : public Visitor
{
public:
    DeepColumnVisitor(Processor &proc, Context &context, ColumnList &list);

    
    virtual void visit(ColumnNode *node);

    virtual void visit(ColumnNumNode *node);

    virtual void visit(ColumnAssignNode *node);

protected:
    Processor &m_proc;
    Context &m_context;
    ColumnList &m_list;
};



//..............................................................................
////////////////////////////////////////////////////////////////// ColumnVisitor
///
/// @since 0.0.1
/// @brief Column Visitor
struct ColumnVisitor : public Visitor
{
public:
    ColumnVisitor(Processor &proc, Context &context, ColumnList &left_list, ColumnList &right_list);

    virtual void visit(ColumnAssignNode *node);

protected:

    virtual void fallback_action(Node *node);

protected:
    Processor &m_proc;
    Context &m_context;
    ColumnList &m_left_list;
    ColumnList &m_right_list;
        
};



//..............................................................................
//////////////////////////////////////////////////////////////// TemplateVisitor
///
/// @since 0.0.1
/// @brief Visitor for template arguments
/// @details
/// Prepares the task template argument list
/// 
/// TASK foo() AS TRANSFER [ obj1(x) ]
///                          ^^^^
/// 
/*
struct TemplateVisitor : public Visitor
{
public:
    TemplateVisitor(Processor &proc, Context &context, ObjectInfo *&obj);

    virtual void visit(IdNode *node);
    
    virtual void visit(IdCallNode *node);

    virtual void visit(TableNode *node);

    virtual void visit(SqlNode *node);

protected:
    Processor &m_proc;
    Context &m_context;
    ObjectInfo *&m_objinfo;
        
};
*/



//..............................................................................
///////////////////////////////////////////////////////////// TemplateArgVisitor
///
/// @since 0.0.1
/// @brief Visitor for arguments of template arguments
/// @details
///
/// TASK foo() AS TRANSFER [ obj1(x) ]
///                               ^
///
struct TemplateArgVisitor : public Visitor
{
public:
    TemplateArgVisitor(Processor &proc, Context &context, ArgumentList &list);

    virtual void visit(IdNode *node);
    
    virtual void visit(IdCallNode *node);


protected:
    Processor &m_proc;
    Context &m_context;
    ArgumentList &m_list;
        
};



//..............................................................................
//////////////////////////////////////////////////////////// LValueColumnVisitor
///
/// @since 0.0.1
/// @brief Object Child Visitor
struct LValueColumnVisitor : public Visitor
{
public:
    LValueColumnVisitor(Processor &proc, Context &context, Column &c);

    virtual void visit(ColumnNode *node);

    virtual void visit(ColumnNumNode *node);
 
private:
    Processor &m_proc;
    Context &m_context;
    //SourceTable      &m_sourcetable;
    Column &m_column;
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
