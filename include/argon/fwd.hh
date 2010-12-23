//
// fwd.hh - Forward declarations
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
/// @brief Typedefs and forward declarations
/// @author Daniel Vogelbacher
/// @since 0.1


#ifndef INFORMAVE_ARGON_FWD_HH
#define INFORMAVE_ARGON_FWD_HH



#define ARGON_NAMESPACE_BEGIN namespace informave { namespace argon {
#define ARGON_NAMESPACE_END } }

#include <dbwtl/dbobjects>
#include <dbwtl/dal/engines/generic>


#include <tr1/memory>


ARGON_NAMESPACE_BEGIN

typedef informave::db::String String;

#define ARGON_UNNAMED_ELEMENT "foo"

class Element;
class Token;
class DTSEngine;
class ParseTree;
class Identifier;
class Processor;
class Exception;
class Executor;
class Value;
class Column;
class Object;
class LastError;
template<typename T> class safe_ptr;


typedef std::tr1::shared_ptr<Element> ElementPtr;


namespace db
{
    
    typedef informave::db::dal::IDbc                    Connection;
    typedef informave::db::dal::IStmt                   Stmt;
    typedef informave::db::dal::IEnv                    Env;
    typedef informave::db::dal::IResult                 Resultset;
    typedef std::map<Identifier, Connection*>           ConnectionMap;
    
    typedef informave::db::Database<informave::db::dal::generic> Database;

    typedef Database::Variant Variant;
    typedef Database::Value   Value;
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
