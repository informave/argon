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


//..............................................................................
///////////////////////////////////////////////////////////////// Lambdafunction

/// @details
/// 
Lambdafunction::Lambdafunction(Processor &proc, LambdaFuncNode *node, Context &parent_context)
    : Context(proc, ArgumentList()),
      m_node(node),
      m_parent_context(parent_context)
{
    this->m_symbols.cloneSymbols(parent_context.getSymbols());
}




Value
Lambdafunction::run(void)
{
    //ARGON_DPRINT(ARGON_MOD_PROC, "Running lambda function " << this->id());

    ARGON_ICERR(this->m_node, "invalid node");

/*
    safe_ptr<ArgumentsSpecNode> argsSpecNode = find_node<ArgumentsSpecNode>(this->m_node);

    ARGON_ICERR_CTX(argsSpecNode.get() != 0, *this,
                "no argument specification");


    ARGON_ICERR_CTX(argsSpecNode->getChilds().size() == this->getCallArgs().size(), *this,
                "Argument count mismatch");

    ArgumentList::const_iterator i = this->getCallArgs().begin();
    foreach_node(argsSpecNode->getChilds(),
                 Arg2SymVisitor(this->proc(), this->getSymbols(), i, this->getCallArgs().end()), 1);

*/
    // just for development tests
    //this->resolve<ValueElement>(Identifier("foo"));

    CompoundNode *n = find_node<CompoundNode>(this->m_node);
    ARGON_ICERR(n, "invalid node");

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

    //return db::Variant(String("func-null-value"));

    return returnValue;
}




/// @details
/// 
String
Lambdafunction::str(void) const
{
    return "[LAMBDA]";
}


/// @details
/// 
String
Lambdafunction::name(void) const
{
    return ARGON_UNNAMED_ELEMENT;
}

/// @details
/// 
String
Lambdafunction::type(void) const
{
    return "Lambda";
}


/// @details
/// 
SourceInfo
Lambdafunction::getSourceInfo(void) const
{
    /// @bug implement me
    return SourceInfo();
}



/// @details
/// 
/*
Value
Lambdafunction::resolveColumn(const Column &col)
{
    throw std::runtime_error("resolve() not allowed on functions");
}
*/


/// @details
/// 
Object*
Lambdafunction::getMainObject(void)
{
    return this->m_parent_context.getMainObject();
    /// @bug fixme. "Parent has no main object and lambda functions has no own object..."
    ARGON_ICERR_CTX(false, *this,
                "A lambda-function does not contains a result object.");
}


/// @details
/// 
Object*
Lambdafunction::getResultObject(void) 
{ 
    ARGON_ICERR_CTX(false, *this,
                "A lambda-function does not contains a result object.");
}


/// @details
/// 
Object*
Lambdafunction::getDestObject(void)
{
    ARGON_ICERR_CTX(false, *this,
                "A lambda-function does not contains a destination object.");
}



/// @details
/// 
Value
Lambdafunction::_value(void) const
{
    return String("FUNCTION"); /// @todo return function name
}

/// @details
/// 
String
Lambdafunction::_string(void) const
{
    return this->_value().data().asStr();
}

/// @details
/// 
String
Lambdafunction::_name(void) const
{
    return String("unknown-id") + String(" (lambda)");
    //return this->id().str() + String(" (function)");
}

/// @details
/// 
String
Lambdafunction::_type(void) const
{
    return "LAMBDAFUNCTION";
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
