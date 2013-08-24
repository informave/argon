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
#include "argon/rtti.hh"
#include "helpers.hh"
#include "debug.hh"
#include "visitors.hh"

#include "builtin/functions.hh"

#include <iostream>
#include <stack>

ARGON_NAMESPACE_BEGIN


ScopedStackPush::~ScopedStackPush(void)
{
   ARGON_ICERR(m_stack.size() > 0, "stack error");
    Element *elem = *m_stack.rbegin();

    if(this->m_ptr != elem) // something bad happens
    {
        String s("Stack protection error: ");
        s.append(LastError(this->m_stack).str());
        
        // If something very bad happens to the stack,
        // the value we pop is not the same as we pushed before.
        // Main reason is someone has push something to the stack,
        // but has not pop from when finished.
        ARGON_ICERR(m_ptr == elem, s);        
    }
    else
    {
        delete elem;
        m_stack.pop_back();
    }
}



//..............................................................................
////////////////////////////////////////////////////////////////////// Processor

/// @details
///
Processor::Processor(DTSEngine &engine)
    : m_engine(engine),
      m_stack(),
      m_heap(),
      m_tree(0),
      //m_symbols(),
      m_types(),
      m_globalcontext(0)
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

/*
void
Processor::addtoHeap(Element *elem)
{
    this->m_heap.push_back(elem);
}
*/

void
Processor::stackPush(Element *elem)
{
    this->m_stack.push_back(elem);
}




void
Processor::addBuiltinFunctiontable(builtin_func_def *ptr)
{
    builtin_func_def *defp = ptr;
    while(defp->name)
    {
        this->getTypes().add(new BuiltinFunctionType(*this, *defp));
        ++defp;
    }
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

//    ::abort();


    SemanticCheck sc(this->m_tree, *this);
    sc.check();



    // Phase 1
    // Add builtin types, functions etc.
    this->getTypes().add(new ConnectionType(*this, Identifier("connection")));
    this->getTypes().add(new TableType(*this, Identifier("table"), NULL_NODE));
    this->getTypes().add(new SqlType(*this, Identifier("sql"), NULL_NODE));

    this->getTypes().add(new GenRangeType(*this, Identifier("gen_range"), NULL_NODE));
    this->getTypes().add(new ExpandType(*this, Identifier("expand"), NULL_NODE));
    this->getTypes().add(new CompactType(*this, Identifier("compact"), NULL_NODE));



//    this->getTypes().add(new FunctionType(*this, Identifier("string.concat"), NULL_NODE));


    this->addBuiltinFunctiontable(table_string_funcs);
    this->addBuiltinFunctiontable(table_sys_funcs);
    this->addBuiltinFunctiontable(table_debug_funcs);
    this->addBuiltinFunctiontable(table_seq_funcs);



    // for each module and the progam
    for(NodeList::iterator i = this->m_tree->getChilds().begin();
        i != this->m_tree->getChilds().end();
        ++i)
    {
        foreach_node( (*i)->getChilds(), Pass1Visitor(*this), 1); // only first level nodes
    }

    // Phase 2
    // for each module and the progam
/*
    for(NodeList::iterator i = this->m_tree->getChilds().begin();
        i != this->m_tree->getChilds().end();
        ++i)
    {
        foreach_node( (*i)->getChilds(), Pass2Visitor(*this), 1); // only first level nodes
    }
*/
    //::abort();



}




/// @details
///
Value
Processor::call(Element &obj)
{
    // DO NOT USE StackPush!
    // The newer version deletes the element!
    //ScopedStackPush _ssp(*this, &obj);

    // This is for LogCmd & Co., see TaskChildVisitor

    return enter_element(Executor(), obj);
}



/// @details
///
Value
Processor::call(const Identifier &id, const ArgumentList &list)
{
    Element *elem = 0;
    //elem = this->getTypes().find<TaskType>(id)->newInstance(list);
    elem = this->getTypes().find<Type>(id)->newInstance(list);
    ARGON_ICERR(elem, "invalid element");
    ARGON_SCOPED_STACKPUSH(*this, elem);

    return enter_element(Executor(), *elem);
}



/// @details
/// This call() method resolves the arguments from the ArgumentsNode via
/// the given context and calls the <id> with these argument values.
Value
Processor::call(const Identifier &id, ArgumentsNode *argsNode, Context &ctx)
{
    // new stack frame for this call
    ARGON_SCOPED_STACKFRAME(*this);

    ARGON_ICERR(argsNode, "invalid node");

    ArgumentList alist;
    // resolve all arguments
    foreach_node(argsNode->getChilds(), ArgumentsVisitor(*this, ctx, alist), 1);

    const Processor::stack_type::size_type size = this->getStack().size();

    Value v = this->call(id, alist);
    ARGON_ICERR(this->getStack().size() == size, "invalid stack size"); // stack must have same size as saved before call
    return v;
}



/// @details
///
Value Processor::run(void)
{
    ARGON_DPRINT(ARGON_MOD_PROC, "Running script");

    ARGON_SCOPED_STACKFRAME(*this);
    Node::nodelist_type childs = this->m_tree->getChilds();

    try
    {
        try
        {
            GlobalContext *gctx = new GlobalContext(*this, this->m_tree);
            ARGON_ICERR(gctx, "invalid global context");
            this->m_globalcontext = gctx;

            ARGON_SCOPED_STACKPUSH(*this, gctx);
            Value v = this->call(*gctx).data();

            this->m_globalcontext = 0; // reset, autom. destroyed
        }
        catch(...)
        {
            this->m_globalcontext = 0; // reset, autom. destroyed
            throw;
        }
    }
    catch(TerminateControlException& e)
    {
        std::cerr << "Program terminated." << std::endl;
        std::cerr << "Result is: " << e.getExitcode().data() << std::endl;
        return e.getExitcode();
    }
    catch(AssertControlException& e)
    {
        std::cerr << e.message() << std::endl;
        std::cerr << "Program terminated." << std::endl;
        return ARGON_EXIT_ASSERT;
    }
    catch(ControlException& e)
    {
        std::cerr << "Unhandled control exception. Terminated." << std::endl;
        return ARGON_EXIT_PARSER_ERROR;
    }

    ARGON_ICERR(this->m_stack.size() == 0, "invalid stack size");
    return ARGON_EXIT_SUCCESS;
}


/// @details
///
/*
SymbolTable&
Processor::getSymbols(void)
{
    return this->getGlobalContext().getSymbols();
}
*/


GlobalContext&
Processor::getGlobalContext(void)
{
	ARGON_ICERR(this->m_globalcontext, "No global context");
	return *this->m_globalcontext;
}


/// @details
///
TypeTable&
Processor::getTypes(void)
{
	return this->m_types;
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
