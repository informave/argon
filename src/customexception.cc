//
// customexception.cc - CustomException (definition)
//
// Copyright (C)         informave.org
//   2013,               Daniel Vogelbacher <daniel@vogelbacher.name>
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
/// @brief CustomException (definition)
/// @author Daniel Vogelbacher
/// @since 0.1


#include "argon/dtsengine.hh"
#include "argon/exceptions.hh"
#include "argon/ast.hh"
#include "argon/rtti.hh"
#include "debug.hh"
#include "visitors.hh"

#include <iostream>
#include <sstream>
#include <list>

ARGON_NAMESPACE_BEGIN


    CustomException::CustomException(const ExceptionType &type)
    : m_type(type), m_value()
    {}

    CustomException::CustomException(const ExceptionType &type, const Value& val)
        : m_type(type), m_value(val)
    {}

CustomException::~CustomException() throw()
        {}

const char*
CustomException::what(void) const throw()
    {
    	try
	{
        	return m_value.str().utf8();
		}
	catch(...)
	{
		return this->getType().id().str().utf8(); /// @bug local what string with description
	}
    }

const ExceptionType&
CustomException::getType(void) const
    {
        return this->m_type;
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
