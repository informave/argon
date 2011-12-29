//
// processor.cc - Processor (definition)
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
/// @brief Processor (definition)
/// @author Daniel Vogelbacher
/// @since 0.1


#include "argon/dtsengine.hh"
#include "argon/exceptions.hh"
#include "argon/semantic.hh"
#include "helpers.hh"
#include "debug.hh"
#include "visitors.hh"

#include <iostream>
#include <stack>

ARGON_NAMESPACE_BEGIN

//..............................................................................
//////////////////////////////////////////////////////////////// ScopedStackPush
///
/// @since 0.0.1
/// @brief Scoped stack-push
class ScopedStackPush
{
public:
    ScopedStackPush(Processor::stack_type &stack, Element *elem)
        : m_stack(stack)
    {
        m_stack.push_front(elem);
    }

    ~ScopedStackPush(void)
    {
        m_stack.pop_front();
    }

protected:
    Processor::stack_type &m_stack;
};



//..............................................................................
///////////////////////////////////////////////////////////////// ProcTreeWalker

/// @details
/// 
ProcTreeWalker::ProcTreeWalker(Processor &proc)
    : m_proc(proc)
{}


/// @details
/// 
void
ProcTreeWalker::visit(ConnNode *node)
{
    Connection *elem = this->proc().getSymbols().addPtr(
        new Connection(this->proc(), node, this->m_proc.getConnections()) );
    this->proc().getSymbols().add(node->data(), elem);

    if(! elem->getDbc().isConnected())
    {
        throw std::runtime_error("dbc is not connected");
    }

    this->m_proc.getSymbols().find<Element>(node->data());

    //this->m_proc.getSymbol<Task>(id).exec(argumentlist);
}



/// @details
/// 
void
ProcTreeWalker::visit(TableNode *node)
{
/*
    Connection *elem = this->proc().toHeap( new Connection(this->proc(), node, this->m_proc.getConnections()) );
    this->proc().addSymbol(node->id, elem);

    if(! elem->getDbc().isConnected())
    {
        throw std::runtime_error("dbc is not connected");
    }
*/

    safe_ptr<Element> elem = SYMBOL_CREATE_ON(this->proc(), node->data(), new Table::Spec(this->proc(), node) );

//    ObjectSepc *elem = this->proc().getSymbols().addPtr( new Table::Specification(this->proc(), node) );

//    ObjectInfo *elem = this->proc().getSymbols().addPtr( new ObjectInfo(this->proc(), node) );
//    this->proc().getSymbols().add(node->id, elem);

    this->m_proc.getSymbols().find<Element>(node->data());

    //this->m_proc.getSymbol<Task>(id).exec(argumentlist);
}



/// @details
/// 
void
ProcTreeWalker::visit(SqlNode *node)
{
/*
    Connection *elem = this->proc().toHeap( new Connection(this->proc(), node, this->m_proc.getConnections()) );
    this->proc().addSymbol(node->id, elem);

    if(! elem->getDbc().isConnected())
    {
        throw std::runtime_error("dbc is not connected");
    }
*/

    safe_ptr<Element> elem = SYMBOL_CREATE_ON(this->proc(), node->data(), new Sql::Spec(this->proc(), node) );

//    ObjectSepc *elem = this->proc().getSymbols().addPtr( new Table::Specification(this->proc(), node) );

//    ObjectInfo *elem = this->proc().getSymbols().addPtr( new ObjectInfo(this->proc(), node) );
//    this->proc().getSymbols().add(node->id, elem);

    this->m_proc.getSymbols().find<Element>(node->data());

    //this->m_proc.getSymbol<Task>(id).exec(argumentlist);
}


/// @details
/// 
void
ProcTreeWalker::visit(TaskNode *node)
{
    Task *elem = 0;

    assert(! node->id.str().empty());

    switch(node->type)
    {
    case ARGON_TASK_VOID:
        elem = this->proc().getSymbols().addPtr( new VoidTask(this->proc(), node) );
        break;
    case ARGON_TASK_FETCH:
        elem = this->proc().getSymbols().addPtr( new FetchTask(this->proc(), node) );
        break;
    case ARGON_TASK_STORE:
    	elem = this->proc().getSymbols().addPtr( new StoreTask(this->proc(), node) );
        break;
    case ARGON_TASK_TRANSFER:
    	elem = this->proc().getSymbols().addPtr( new TransferTask(this->proc(), node) );
        break;
    }
    this->proc().getSymbols().add(node->id, elem);
}


/// @details
/// 
void
ProcTreeWalker::visit(LogNode *node)
{
    //std::cout << "visit log node" << std::endl;
}


/// @details
/// 
void
ProcTreeWalker::visit(IdNode *node)
{
    //std::cout << "visit id node: " << node->str() << std::endl;
}


/// @details
/// 
void
ProcTreeWalker::visit(LiteralNode *node)
{
    //std::cout << "visit literal node: " << node->str() << std::endl;
}


/// @details
/// 
void
ProcTreeWalker::visit(ParseTree *node)
{
    //std::cout << "Visit tree" << std::endl;
}



//..............................................................................
////////////////////////////////////////////////////////////////////// Processor

/// @details
/// 
Processor::Processor(DTSEngine &engine)
    : m_engine(engine),
      m_stack(),
      m_tree(0),
      m_symbols()
{}


/// @details
/// 
Function*
Processor::createFunction(const Identifier &id)
{
    DTSEngine::function_map::iterator i = this->m_engine.m_functions.find(id);
    if(i == this->m_engine.m_functions.end())
        throw std::runtime_error(String("Unknown function ").append(id.str()));
    else
        return i->second(*this);
}


/// @details
/// 
db::ConnectionMap&
Processor::getConnections(void)
{
    return this->m_engine.getConnections();
}


/// @details
/// 
void
Processor::compile(ParseTree *tree)
{
    this->m_tree = tree;

    // print node tree
#if defined(ARGON_DEV_DEBUG) || 1
    foreach_node(this->m_tree, PrintTreeVisitor(*this, std::wcout), 1);
    #endif


    SemanticCheck sc(this->m_tree, *this);

    sc.check();

    //::abort();

    // Generate connections, objects etc...
    foreach_node( this->m_tree, ProcTreeWalker(*this), 2); // only deep 2
}


/// @details
/// 
Value
Processor::call(Element *obj, const ArgumentList &args)
{
    ScopedStackPush _ssp(this->m_stack, obj);

    return enter_element(Executor(args), *obj);
}



/// @details
/// 
Value
Processor::call(Element &obj)
{
    return enter_element(Executor(ArgumentList()), obj);
}



/// @details
/// 
void Processor::run(void)
{
    ARGON_DPRINT(ARGON_MOD_PROC, "Running script");
   


    Task *task = this->getSymbols().find<Task>(Identifier("main"));

    // RUN TASK
    Value v = this->call(task, ArgumentList());

    assert(this->m_stack.size() == 0);

    //this->call( this->getSymbol<Connection>(Identifier("c1")) );

}


/// @details
/// 
SymbolTable&
Processor::getSymbols(void)
{
    return this->m_symbols;
}


/// @details
/// 
const Processor::stack_type&
Processor::getStack(void)
{
    return this->m_stack;
}


DTSEngine&
Processor::getEngine(void)
{
	return this->m_engine;
}


const DTSEngine&
Processor::getEngine(void) const
{
	return this->m_engine;
}




//..............................................................................
////////////////////////////////////////////////////////////////////// LastError

/// @details
/// 
String
LastError::str(void) const
{
    std::wstringstream ss;

    ss << L"Stack trace:" << std::endl;

    for(Processor::stack_type::const_iterator i = m_stack.begin();
        i != m_stack.end();
        ++i)
    {
        ss << (*i)->type() << L" " << (*i)->name()
           << L" (" << (*i)->getSourceInfo().sourceName() << L":"
           << (*i)->getSourceInfo().linenum() << L")" << std::endl;
    }

    return ss.str();
}


/// @details
/// 
const Processor::stack_type&
LastError::getStack(void) const
{
    return this->m_stack;
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
