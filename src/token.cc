//
// token.cc - Token class
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
/// @brief Token class used by the Tokenizer (definition)
/// @author Daniel Vogelbacher
/// @since 0.1

#include "argon/token.hh"

#include <sstream>
#include <cassert>
#include <iostream>


ARGON_NAMESPACE_BEGIN

//..............................................................................
///////////////////////////////////////////////////////////////////// SourceInfo

/// @details
/// 
SourceInfo::SourceInfo(String file, std::streamsize chpos, size_t len, size_t line)
    : m_file(file), 
      m_offset(chpos),
      m_len(len),
      m_line(line)
{}


/// @details
/// 
void
SourceInfo::expand(const SourceInfo &info)
{
    /// @todo uncomment asserts if all AST nodes implemented
    // assert(info.m_line > 0);
    // assert(info.m_len > 0);

    if(m_line == 0 || m_line > info.m_line)
        m_line = info.m_line == 0 ? m_line : info.m_line;

    if(m_file.empty())
        m_file = info.m_file;

    if(m_offset == 0 || info.m_offset < m_offset)
        m_offset = info.m_offset;

    if(m_offset + m_len < info.m_offset + info.m_len)
        this->m_len = (info.m_offset + info.m_len) - m_offset;
}


//..............................................................................
////////////////////////////////////////////////////////////////////////// Token

/// @details
/// 
Token::Token(void)
    : m_id(0), 
      m_info()
{}


/// @details
/// 
Token::Token(int tid, SourceInfo info)
    : m_id(tid),
      m_info(info)
{}


/// @details
/// 
Token::Token(const Token& t)
    : m_id(t.m_id),
      m_data(t.m_data),
      m_info(t.m_info)
{}


/// @details
/// 
void 
Token::setData(const String &str)
{
    this->m_data = str;
}


/// @details
/// 
String 
Token::data(void) const
{
    return this->m_data;
}


/// @details
/// 
int 
Token::id(void) const
{
    return this->m_id;
}


/// @details
/// 
const SourceInfo&
Token::getSourceInfo(void) const
{
    return this->m_info;
}


/// @details
/// 
std::string 
Token::str(void) const
{
    std::stringstream tmp;
          
    tmp << "<"
        << this->m_id
        << "> "
        << "Start: "
        << this->getSourceInfo().offset()
        << " Line: "
        << this->getSourceInfo().linenum()
        << " DATA: "
        << this->m_data;
            
    return tmp.str();
}


/// @details
///  
Token 
Token::eof(void)
{
    return Token(0);
}


/// @details
/// 
bool 
Token::operator !=(const Token &tok) const
{
    return m_id != tok.m_id;
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
