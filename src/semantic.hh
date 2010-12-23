//
// semantic.hh - Semantic checker
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
/// @brief Semantic checker
/// @author Daniel Vogelbacher
/// @since 0.1


#ifndef INFORMAVE_ARGON_SEMANTIC_HH
#define INFORMAVE_ARGON_SEMANTIC_HH

#include "argon/fwd.hh"
#include "argon/token.hh"
#include "argon/ast.hh"

#include <list>


ARGON_NAMESPACE_BEGIN


//..............................................................................
////////////////////////////////////////////////////////////////// SemanticCheck
///
/// @since 0.0.1
/// @brief Semantic check
class SemanticCheck
{
public:
    SemanticCheck(ParseTree *tree);

    void check(void);

protected:

    ParseTree *m_tree;
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
