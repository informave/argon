//
// elements.cc - Elements (definition)
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
/// @brief Elements (definition)
/// @author Daniel Vogelbacher
/// @since 0.1

#include "argon/dtsengine.hh"
#include "argon/exceptions.hh"
#include "debug.hh"
#include "visitors.hh"

#include "argon/rtti.hh"
#include "builtin/functions.hh"


#include <iostream>
#include <functional>
#include <sstream>

ARGON_NAMESPACE_BEGIN



BlockVisitor::BlockVisitor(Processor &proc, Context &ctx, Value &returnVal)
	: CVisitor(proc, ctx, ignore_none), m_returnVal(returnVal)
{
}


void
BlockVisitor::visit(AssertNode *node)
{
    ArgumentList al;
    //Identifier id = node->data();
    ArgumentsNode *argsnode = find_node<ArgumentsNode>(node);
    assert(argsnode);

    // Fill argument list with the result of each argument node
    foreach_node(argsnode->getChilds(), ArgumentsVisitor(proc(), context(), al), 1);


    ARGON_ICERR_CTX(al.size() > 0 && al.size() <= 2, context(), "Invalid assert() argument count");


    if(! al[0]->_value().data().get<bool>())
    {
        SourceInfo si = node->getSourceInfo();

        if(al.size() == 2)
            throw AssertControlException(node, al[1]->_value().data().asStr());
        else
            throw AssertControlException(node);
    }
}


void
BlockVisitor::visit(WhileNode *node)
{
	ARGON_ICERR_CTX(node->getChilds().size() == 2, context(), "Wrong child-node count");

	Node *checkexpr = node->getChilds().at(0);
	Node *block = node->getChilds().at(1);
	Value checkresult;
	apply_visitor(checkexpr, EvalExprVisitor(this->proc(), context(), checkresult));
	while(checkresult.data().asBool())
	{
        try
        {
            //apply_visitor(block->getChilds(), BlockVisitor(this->proc(), context(), m_returnVal));
            apply_visitor(block, BlockVisitor(this->proc(), context(), m_returnVal));
        }
        catch(ContinueControlException&)
        {
            // do nothing, end of function evaluates check expr.
        }
        catch(BreakControlException&)
        {
            break;
        }

        apply_visitor(checkexpr, EvalExprVisitor(this->proc(), context(), checkresult)); // check again
	}
}



void
BlockVisitor::visit(RepeatNode *node)
{
	ARGON_ICERR_CTX(node->getChilds().size() == 2, context(), "Wrong child-node count");

	Node *checkexpr = node->getChilds().at(1);
	Node *block = node->getChilds().at(0);
	Value checkresult;

    do
    {
        try
        {
            //apply_visitor(block->getChilds(), BlockVisitor(this->proc(), context(), m_returnVal));
            apply_visitor(block, BlockVisitor(this->proc(), context(), m_returnVal));
        }
        catch(ContinueControlException&)
        {
            // do nothing, end of function evaluates check expr.
        }
        catch(BreakControlException&)
        {
            break;
        }

        apply_visitor(checkexpr, EvalExprVisitor(this->proc(), context(), checkresult)); // check
    }
	while(! checkresult.data().asBool()); // negate expr because we want "UNTIL"
}



void
BlockVisitor::visit(ForNode *node)
{
	ARGON_ICERR_CTX(node->getChilds().size() == 4, context(), "Wrong child-node count");

    Node *init = node->getChilds().at(0);
	Node *checkexpr = node->getChilds().at(1);
	Node *step = node->getChilds().at(2);
	Node *block = node->getChilds().at(3);
	Value checkresult, initresult, stepresult;

    // init
	apply_visitor(init, EvalExprVisitor(this->proc(), context(), initresult));

    // check
	apply_visitor(checkexpr, EvalExprVisitor(this->proc(), context(), checkresult));
	while(checkresult.data().asBool())
	{
        try
        {
            //apply_visitor(block->getChilds(), BlockVisitor(this->proc(), context(), m_returnVal));
            apply_visitor(block, BlockVisitor(this->proc(), context(), m_returnVal));
        }
        catch(ContinueControlException&)
        {
            // do nothing, end of function evaluates check expr.
        }
        catch(BreakControlException&)
        {
            break;
        }

        apply_visitor(step, EvalExprVisitor(this->proc(), context(), stepresult));
        apply_visitor(checkexpr, EvalExprVisitor(this->proc(), context(), checkresult)); // check again
	}
}




void
BlockVisitor::visit(ContinueNode *node)
{
    throw ContinueControlException();
}

void
BlockVisitor::visit(BreakNode *node)
{
    throw BreakControlException();
}




void
BlockVisitor::visit(IfelseNode *node)
{
	assert(node->getChilds().size() >= 2);
	Node *checkexpr = node->getChilds().at(0);
	Node *ifblock = node->getChilds().at(1);
	Node *elseblock = 0;
	if(node->getChilds().size() == 3)
		elseblock = node->getChilds().at(2);
	
	Value checkresult;

	apply_visitor(checkexpr, EvalExprVisitor(this->proc(), context(), checkresult));
	if(checkresult.data().asBool())
	{
        //ARGON_ICERR(is_nodetype<CompoundNode*>(ifblock), "Invalid node");
		//apply_visitor(ifblock->getChilds(), BlockVisitor(this->proc(), context(), m_returnVal));
        apply_visitor(ifblock, BlockVisitor(this->proc(), context(), m_returnVal));
	}
	else if(elseblock)
	{
        //ARGON_ICERR(is_nodetype<CompoundNode*>(elseblock), "Invalid node");
		//apply_visitor(elseblock->getChilds(), BlockVisitor(this->proc(), context(), m_returnVal));
        apply_visitor(elseblock, BlockVisitor(this->proc(), context(), m_returnVal));
	}
}



void
BlockVisitor::visit(CompoundNode *node)
{
    ARGON_SCOPED_STACKFRAME(this->proc());
    ARGON_SCOPED_SUBSYMBOLS(this->context().getSymbols());
    
    apply_visitor(node->getChilds(), BlockVisitor(this->proc(),
                                                  this->context(),
                                                  this->m_returnVal));
}



/// @detail
/// All node types not handled by this visitor assumed to be expression nodes.
/// The return value is ignored, this example should explain why:
///
/// function foo()
/// begin
///    1 + 2;
/// end;
/// 
void
BlockVisitor::fallback_action(Node *node)
{
    Value value;
    apply_visitor(node, EvalExprVisitor(proc(), context(), value));
}


void
BlockVisitor::visit(ReturnNode *node)
{
    ARGON_ICERR_CTX(node->getChilds().size() <= 1, context(),
                "ReturnNode must have 0 or 1 childs.");

    if(node->getChilds().size() == 1)
    {
        Node *op0 = node->getChilds().at(0);
        apply_visitor(op0, EvalExprVisitor(proc(), context(), this->m_returnVal));
        throw ReturnControlException(this->m_returnVal);
    }
    else
    {
        this->m_returnVal.data().setNull();
        throw ReturnControlException(this->m_returnVal);
    }
}

void
BlockVisitor::visit(VarNode *node)
{
    ArgumentsNode *argsNode = find_node<ArgumentsNode>(node);
    assert(argsNode);
    assert(argsNode->getChilds().size() == 1);

    Node *data = argsNode->getChilds()[0];

    Value value;
    apply_visitor(data, EvalExprVisitor(proc(), context(), value));

    ValueElement *elem = 0;
    elem = new ValueElement(this->proc(), value);

    this->proc().stackPush(elem);
    this->context().getSymbols().add(node->data(), Ref(elem));
    this->m_returnVal.data() = value.data();
}


 /*
void
BlockVisitor::visit(AssignNode *node)
{
	assert(node->getChilds().size() == 2);
	Node *dest = node->getChilds()[0];
	Node *expr = node->getChilds()[1];

	IdNode *id = node_cast<IdNode>(dest);
	ValueElement *elem = this->context().resolve<ValueElement>(id->data());
	elem->getValue().data() = 45;
}
 */

//..............................................................................
/////////////////////////////////////////////////////////////////////// Function

/// @details
/// 
Function::Function(Processor &proc, FunctionNode *node, const ArgumentList &args)
    : Context(proc, args),
      m_node(node)
{}




Value
Function::run(void)
{
    ARGON_DPRINT(ARGON_MOD_PROC, "Running function " << this->id());

    assert(this->m_node);

    safe_ptr<ArgumentsSpecNode> argsSpecNode = find_node<ArgumentsSpecNode>(this->m_node);

    ARGON_ICERR_CTX(argsSpecNode.get() != 0, *this,
                "no argument specification");


    ARGON_ICERR_CTX(argsSpecNode->getChilds().size() == this->getCallArgs().size(), *this,
                "Argument count mismatch");

    ArgumentList::const_iterator i = this->getCallArgs().begin();
    foreach_node(argsSpecNode->getChilds(),
                 Arg2SymVisitor(this->proc(), this->getSymbols(), i, this->getCallArgs().end()), 1);


    // just for development tests
    //this->resolve<ValueElement>(Identifier("foo"));

    CompoundNode *n = find_node<CompoundNode>(this->m_node);
    assert(n);

    ARGON_SCOPED_STACKFRAME(this->proc());

    Value returnValue;

    
    try
    {
        apply_visitor(n->getChilds(), BlockVisitor(this->proc(), *this, returnValue));
    }
    catch(ReturnControlException& e)
    {
        returnValue = e.getValue();
        // ok, catch return expr.
    }

    //this->resolve<ValueElement>(Identifier("x"));

    //this->clearSymbolTable(); // prevent "refs to non-existing element" warnings
    this->getSymbols().reset(); // cleanup symbols (refs) to safety restore stack
/*
    {
    	ARGON_SCOPED_STACKFRAME(this->proc());
	ARGON_SCOPED_SYMBOLTABLE(this->m_context);
	try
	{
		apply_visitor(n->getChilds(), BlockVisitor(this->proc(), *this));
	}
	catch(ReturnValueException &e)
	{
		return db::Variant(String("func-null-value"));
	}
    }
*/
    //return db::Variant(String("func-null-value"));

    return returnValue;
}




/// @details
/// 
String
Function::str(void) const
{
    return "[FUNCTION]";
}


/// @details
/// 
String
Function::name(void) const
{
    return ARGON_UNNAMED_ELEMENT;
}

/// @details
/// 
String
Function::type(void) const
{
    return "Function";
}


/// @details
/// 
SourceInfo
Function::getSourceInfo(void) const
{
    /// @bug implement me
    return SourceInfo();
}



/// @details
/// 
/*
Value
Function::resolveColumn(const Column &col)
{
    throw std::runtime_error("resolve() not allowed on functions");
}
*/


/// @details
/// 
Object*
Function::getMainObject(void)
{
    ARGON_ICERR_CTX(false, *this,
                "A function does not contains a result object.");
}


/// @details
/// 
Object*
Function::getResultObject(void) 
{ 
    ARGON_ICERR_CTX(false, *this,
                "A function does not contains a result object.");
}


/// @details
/// 
Object*
Function::getDestObject(void)
{
    ARGON_ICERR_CTX(false, *this,
                "A function does not contains a destination object.");
}



/// @details
/// 
Value
Function::_value(void) const
{
    return String("FUNCTION"); /// @todo return function name
}

/// @details
/// 
String
Function::_string(void) const
{
    return this->_value().data().asStr();
}

/// @details
/// 
String
Function::_name(void) const
{
    return String("unknown-id") + String(" (function)");
    //return this->id().str() + String(" (function)");
}

/// @details
/// 
String
Function::_type(void) const
{
    return "FUNCTION";
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
