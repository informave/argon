//
// parserapi.hh - C++ Wrapper from Lemon API
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
/// @brief C++ wrapper for Lemon API
/// @author Daniel Vogelbacher
/// @since 0.1


#ifndef INFORMAVE_ARGON_PARSERAPI_HH
#define INFORMAVE_ARGON_PARSERAPI_HH

#include <iosfwd>
#include <cstdlib>

#include "parser.h"
#include "argon/dtsengine.hh"
#include "argon/token.hh"

#define ParseTOKENTYPE informave::argon::Token*
#define ParseARG_PDECL ,informave::argon::ParseTree *tree


void Parse(void *,               // The parser
           int,                  // The major token code number
           ParseTOKENTYPE        // The value for the token
           ParseARG_PDECL);      // Optional %extra_argument parameter


void ParseFree(void *p,                    // The parser to be deleted
               void (*freeProc)(void*) );  // Function used to reclaim memory


void *ParseAlloc(void *(*mallocProc)(size_t));

void ParseTrace(FILE *stream, char *zPrefix);


ARGON_NAMESPACE_BEGIN



//--------------------------------------------------------------------------
/// Lemon Parser Wrapper
///
/// @since 0.0.1
/// @brief Lemon Parser Wrapper
class Parser
{
public:
    Parser(void) : m_parser(0)
    {
        m_parser = ParseAlloc(::malloc);
    }

    inline void parse(int tid, ParseTOKENTYPE token, ParseTree *tree)
    {
        Parse(m_parser, tid, token, tree);
    }

    inline void trace(FILE *stream, const char *prefix)
    {
        ParseTrace(stream, const_cast<char*>(prefix));
    }

    ~Parser(void)
    {
        if(m_parser)
        {
            ParseFree(m_parser, ::free);
            m_parser = 0;
        }
    }

protected:
    void *m_parser;

private:
    Parser(const Parser&);
    Parser& operator=(const Parser&);
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
