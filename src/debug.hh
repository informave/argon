//
// debug.hh - Debugging macros
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
/// @brief Debugging macros
/// @author Daniel Vogelbacher
/// @since 0.1


#ifndef INFORMAVE_ARGON_DEBUG_HH
#define INFORMAVE_ARGON_DEBUG_HH

#include "argon/fwd.hh"
#include "argon/token.hh"
#include "argon/ast.hh"
#include "argon/dtsengine.hh"

#include <iostream>

ARGON_NAMESPACE_BEGIN

//#define ARGON_DEV_DEBUG

#ifdef ARGON_DEV_DEBUG
#define ARGON_DPRINT(mod, expr) std::cout << "[DEBUG] (" << mod << ") " << expr << std::endl
#else
#define ARGON_DPRINT(mod, expr)
#endif

#define ARGON_MOD_SEM "semantic"
#define ARGON_MOD_PROC "processor"
#define ARGON_MOD_CONN "connection"
#define ARGON_MOD_DD "--*****--"


#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)


#define FIXME() throw std::runtime_error(AT " FIXME")

#define BUG() FIXME("bug")

inline void debug_print_arglist(const ArgumentList &list)
{
#ifdef ARGON_DEV_DEBUG
    for(ArgumentList::const_iterator i = list.begin();
        i != list.end();
        ++i)
    {
        ARGON_DPRINT(ARGON_MOD_PROC, "Argumemnt item: " << i->data());
    }
#endif
}

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
