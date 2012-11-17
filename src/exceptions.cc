//
// exceptions.cc - Exceptions (definition)
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
/// @brief Exceptions (definition)
/// @author Daniel Vogelbacher
/// @since 0.1

#include "argon/exceptions.hh"
#include "argon/token.hh"

#include <sstream>
#include <iostream>

ARGON_NAMESPACE_BEGIN


//..............................................................................
////////////////////////////////////////////////////////////////// InternalError

/// @details
/// 
InternalError::InternalError(Context &context, const char* expr, const char *what, const char *file, int line)
    : RuntimeError(context)
{
    std::wstringstream ss;
    
    ss << L"Internal compiler error: "
       << String(what)
       << L" at " << String(file) << L":" << line
       << L" - expr: " << String(expr)
       << std::endl
       << L"Please report this BUG on <http://argon.informave.org/bugtracker> or send a PATCH to argon-list@informave.org"
       << std::endl;
    

    this->m_what = String(ss.str()) + this->m_what;
}



InternalError::InternalError(const char* expr, const String &what, const char *file, int line)
    : RuntimeError()
{
    std::wstringstream ss;
    
    ss << L"Internal compiler error: "
       << String(what)
       << L" at " << String(file) << L":" << line
       << L" - expr: " << String(expr)
       << std::endl
       << L"Please report this BUG on <http://argon.informave.org/bugtracker> or send a PATCH to argon-list@informave.org"
       << std::endl;
    

    this->m_what = String(ss.str()) + this->m_what;
}



//..............................................................................
/////////////////////////////////////////////////////////////////// RuntimeError

/// @details
/// 
RuntimeError::RuntimeError(Context &context)
	: Exception()
{
    LastError e(context.proc().getStack());
    m_what = e.str();
}


RuntimeError::RuntimeError(void)
	: Exception()
{
    //LastError e(context.proc().getStack());
    //m_what = e.str();
}


/// @details
/// 
void
RuntimeError::addSourceInfo(const SourceInfo &info)
{
    std::wstringstream ss;
    ss << L"In file " << info.str() << std::endl;
    m_what = String(ss.str()) + m_what;
}


//..............................................................................
////////////////////////////////////////////////////////////////// FieldNotFound

/// @details
/// 
FieldNotFound::FieldNotFound(Context &context, String fname)
	: RuntimeError(context)
{
    //String s("Field not found: ");
    //s.append(fname);
    std::wstringstream ss;
    ss << fname << std::endl;
    m_what = String(ss.str()) + m_what;
}



//..............................................................................
//////////////////////////////////////////////////////////////////// SyntaxError

/// @details
/// The given token can be safety freed
SyntaxError::SyntaxError(Token *t)
    : CompileError()
{
    std::wstringstream ss;
    if(t)
    {
        ss << t->getSourceInfo().sourceName() << L":" << t->getSourceInfo().linenum() << L": "
           << L"(AEC5001) "
           << L"Syntax error "
           << L"near token "
           << t->data();
    }
    else // no tokens so far
    {
        ss << L"(AEC5002) "
           << L"Empty input stream!";
    }

    this->m_what = ss.str();
}


//..............................................................................
////////////////////////////////////////////////////////////////////// Exception

/// @details
/// 
const char*
Exception::what(void) const throw()
{
    m_tmp = this->m_what;

    return m_tmp.c_str();
}






//..............................................................................
////////////////////////////////////////////////////////////////// SemanticError

/// @details
/// 
SemanticError::SemanticError(const SemanticCheckList &sclist)
    : CompileError(),
      m_sclist(sclist)
{
    std::wstringstream ss;

    for(SemanticCheckList::const_iterator i = sclist.begin();
        i != sclist.end();
        ++i)
    {
        ss << i->_str() << std::endl;
    }

    this->m_what = ss.str();
}



/// @details
/// 
const SemanticCheckList&
SemanticError::getCheckResults(void) const
{
    return this->m_sclist;
}



//..............................................................................
/////////////////////////////////////////////////////////////////// LexicalError

/// @details
/// 
LexicalError::LexicalError(String what, SourceInfo info)
    : CompileError()
{
    std::wstringstream ss;

    ss << info.str() << " Error: " << what;
    this->m_what = ss.str();
}


ReturnControlException::ReturnControlException(Value retvalue)
	: ControlException(),
	m_value(retvalue)
{}

ReturnControlException::~ReturnControlException(void)
{}

Value
ReturnControlException::getValue(void) const
{
	return this->m_value;
}







TerminateControlException::TerminateControlException(Value exitcode)
	: ControlException(),
      m_exitcode(exitcode)
{}

TerminateControlException::~TerminateControlException(void)
{}

Value
TerminateControlException::getExitcode(void) const
{
	return this->m_exitcode;
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
