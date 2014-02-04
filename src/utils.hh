//
// utils.hh - Utilities
//
// Copyright (C)         informave.org
//   2014,               Daniel Vogelbacher <daniel@vogelbacher.name>
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
/// @brief Utility macros
/// @author Daniel Vogelbacher
/// @since 0.1

#ifndef INFORMAVE_ARGON_UTILS_HH
#define INFORMAVE_ARGON_UTILS_HH


#include "argon/dtsengine.hh"


ARGON_NAMESPACE_BEGIN


#define FORMAT1(str, a1) (informave::db::format(str) % a1)
#define FORMAT2(str, a1, a2) (informave::db::format(str) % a1 % a2)
#define FORMAT3(str, a1, a2, a3) (informave::db::format(str) % a1 % a2 % a3)
#define FORMAT4(str, a1, a2, a3, a4) (informave::db::format(str) % a1 % a2 % a3 % a4)
#define FORMAT5(str, a1, a2, a3, a4, a5) (informave::db::format(str) % a1 % a2 % a3 % a4 % a5)


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
