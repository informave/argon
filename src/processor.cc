//
// processor.cc - Processor (definition)
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
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/// @file
/// @brief Processor (definition)
/// @author Daniel Vogelbacher
/// @since 0.1


#include "argon/dtsengine.hh"

#include <iostream>
#include <stack>

ARGON_NAMESPACE_BEGIN


//--------------------------------------------------------------------------
/// Scoped stack-push
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
    Connection *elem = this->proc().toHeap( new Connection(this->proc(), node, this->m_proc.getConnections()) );
    this->proc().addSymbol(node->id, elem);

    if(! elem->getDbc().isConnected())
    {
        throw std::runtime_error("dbc is not connected");
    }

    this->m_proc.getSymbol<Element>(node->id);

    //this->m_proc.getSymbol<Task>(id).exec(argumentlist);
}


/// @details
/// 
void
ProcTreeWalker::visit(TaskNode *node)
{
    /// @bug is this good style?
    Task *elem = this->proc().toHeap( new Task(this->proc(), node) );
    this->proc().addSymbol(node->id, elem);

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
      m_symbols(),
      m_heap()
{}


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
    foreach_node(this->m_tree, PrintTreeVisitor(*this, std::wcout), 1);

    foreach_node( this->m_tree, ProcTreeWalker(*this), 2); // only deep 2
}


/// @details
/// 
Value
Processor::call(Element *obj, const ArgumentList &args)
{
    ScopedStackPush _ssp(this->m_stack, obj);
    return obj->run(args);
}


/// @details
/// 
void Processor::run(void)
{
    std::cout << std::endl << "Running script.." << std::endl;

    Task *task = this->getSymbol<Task>(Identifier("main"));
    Value v = this->call(task, ArgumentList());

    assert(this->m_stack.size() == 0);

    //this->call( this->getSymbol<Connection>(Identifier("c1")) );

}


/// @details
/// 
void
Processor::addSymbol(Identifier name, Element *symbol)
{
    assert(name.str().length() > 0);
            
    element_map::iterator i = this->m_symbols.find(name);
    if(i != this->m_symbols.end())
        throw std::runtime_error("duplicated symbol error: " + std::string(name.str()));
    this->m_symbols[name] = symbol;
}


/// @details
/// 
const Processor::stack_type&
Processor::getStack(void)
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
