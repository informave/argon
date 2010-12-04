//
// exceptions.cc - Exceptions
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


//--------------------------------------------------------------------------
/// Exception base class
///
/// @since 0.0.1
/// @brief Exception base class
class Exception : public std::exception
{
public:
    virtual const char* what(void) const throw();

    virtual ~Exception(void) throw()
    {}

    mutable std::string m_tmp;
    String m_what;
};


//--------------------------------------------------------------------------
/// Not-declared exception
///
/// @since 0.0.1
/// @brief Not-declared exception
class NotDeclared : public Exception
{
};


//--------------------------------------------------------------------------
/// connection-error exception
///
/// @since 0.0.1
/// @brief connection-error exception
class ConnectionErr : public Exception
{
};


//--------------------------------------------------------------------------
/// Syntax-error exception
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


//--------------------------------------------------------------------------
/// parse-error exception
///
/// @since 0.0.1
/// @brief parse-error exception
class ParseError : public Exception
{
};


//--------------------------------------------------------------------------
/// Runtime-error exception
///
/// @since 0.0.1
/// @brief Runtime-error exception
class RuntimeError : public Exception
{
public:
    RuntimeError(const Processor::stack_type &stack);

    virtual ~RuntimeError(void) throw()
    {}

protected:
    const Processor::stack_type m_stack;
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
