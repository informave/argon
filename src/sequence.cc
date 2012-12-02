//
// sequence.cc - Sequence (definition)
//
// Copyright (C)         informave.org
//   2012,               Daniel Vogelbacher <daniel@vogelbacher.name>
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
/// @brief Sequence (definition)
/// @author Daniel Vogelbacher
/// @since 0.1

#include "argon/dtsengine.hh"
#include "argon/exceptions.hh"
#include "debug.hh"
#include "visitors.hh"

#include <iostream>
#include <functional>
#include <sstream>

ARGON_NAMESPACE_BEGIN


//..............................................................................
/////////////////////////////////////////////////////////////////////// Sequence

/// @details
/// 
Sequence::Sequence(Processor &proc, const Value &val)
    : Element(proc),
      m_value(val)
{
}


/// @details
/// 
String
Sequence::str(void) const
{
    String s;
    //s.append(this->id().str());
    s.append("Value: ");
    //s.append(this->getValue().data().asStr());
    if(!this->m_value.data().isnull())
    	s.append(this->m_value.data().asStr());
    else
    	s.append("<NULL>");
    
    return s;
}


/// @details
/// Sequence does not contains any source information.
SourceInfo
Sequence::getSourceInfo(void) const
{
    return SourceInfo();
}


/// @details
/// 
String
Sequence::name(void) const
{
    if(!this->m_value.data().isnull())
    	return this->m_value.data().asStr();
    else
    	return "<NULL>";
}


/// @details
/// 
String
Sequence::type(void) const
{
    return "VALUE";
}


/// @details
/// 
Value
Sequence::_value(void) const
{
    return this->m_value;
}

/// @details
/// 
String
Sequence::_string(void) const
{
    return this->_value().data().asStr();
}

/// @details
/// 
String
Sequence::_name(void) const
{
    //return String("unknown-id") + String(" (function)");
    return "VALUE";
}

/// @details
/// 
String
Sequence::_type(void) const
{
    return "VALUE";
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
