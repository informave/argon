//
// exceptions.cc - Exceptions
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
/// @brief Exceptions
/// @author Daniel Vogelbacher
/// @since 0.1

#ifndef INFORMAVE_ARGON_EXCEPTIONS_HH
#define INFORMAVE_ARGON_EXCEPTIONS_HH

#include "argon/fwd.hh"
#include "argon/dtsengine.hh"

#include <dbwtl/dbobjects>
#include <dbwtl/dal/engines/generic>

#include <iterator>
#include <map>
#include <vector>
#include <stdexcept>


ARGON_NAMESPACE_BEGIN



//..............................................................................
////////////////////////////////////////////////////////////////////// Exception
///
/// @since 0.0.1
/// @brief Exception base class
class Exception : public std::exception
{
public:
    Exception(void) : m_tmp(), m_what()
    {}

    virtual ~Exception(void) throw()
    {}

    virtual const char* what(void) const throw();

    mutable std::string m_tmp;
    String m_what;
};



//..............................................................................
//////////////////////////////////////////////////////////////////// NotDeclared
///
/// @since 0.0.1
/// @brief Not-declared exception
class NotDeclared : public Exception
{
};



//..............................................................................
////////////////////////////////////////////////////////////////// ConnextionErr
///
/// @since 0.0.1
/// @brief connection-error exception
class ConnectionErr : public Exception
{
};



//..............................................................................
//////////////////////////////////////////////////////////////////// SyntaxError
///
/// @since 0.0.1
/// @brief Syntax-error exception
class SyntaxError : public Exception
{
public:
    SyntaxError(Token *t);

    virtual ~SyntaxError(void) throw()
    {}

};


//..............................................................................
///////////////////////////////////////////////////////////////////// ParseError
///
/// @since 0.0.1
/// @brief parse-error exception
class ParseError : public Exception
{
};



//..............................................................................
/////////////////////////////////////////////////////////////////// RuntimeError
///
/// @since 0.0.1
/// @brief Runtime-error exception
class RuntimeError : public Exception
{
public:
    RuntimeError(Context &context);

    RuntimeError(void);

    //RuntimeError(void) { }
    
    void addSourceInfo(const SourceInfo &info);

    virtual ~RuntimeError(void) throw()
    {}

};


//..............................................................................
////////////////////////////////////////////////////////////////// FieldNotFound
///
/// @since 0.0.1
/// @brief Field not found exception
class FieldNotFound : public RuntimeError
{
public:
    FieldNotFound(Context &context, String fname);

    virtual ~FieldNotFound(void) throw()
    {}
};


//..............................................................................
////////////////////////////////////////////////////////////////// InternalError
///
/// @since 0.0.1
/// @brief Internal-error exception
class InternalError : public RuntimeError
{
public:
    InternalError(Context &context, const char* expr, const char *what, const char *file, int line);

    InternalError(const char* expr, const char *what, const char *file, int line);

    virtual ~InternalError(void) throw()
    {}

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
