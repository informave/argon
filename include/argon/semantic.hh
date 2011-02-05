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


typedef std::list<SemanticCheckEntry>  SemanticCheckList;



//..............................................................................
////////////////////////////////////////////////////////////////// SemanticCheck
///
/// @since 0.0.1
/// @brief Semantic check
class SemanticCheck
{
public:
    SemanticCheck(ParseTree *tree, Processor &proc);

    enum sctype { SC_ERROR, SC_WARNING };

    void check(void);

    Processor& proc(void) { return this->m_proc; }

    ParseTree* ast(void) { return this->m_tree; }

    void addCheckEntry(const SemanticCheckEntry &entry);

protected:

    ParseTree *m_tree;
    Processor &m_proc;

    SemanticCheckList m_list;

private:
    SemanticCheck(const SemanticCheck &);
    SemanticCheck& operator=(const SemanticCheck &);
};



//..............................................................................
///////////////////////////////////////////////////////////// SemanticCheckEntry
///
/// @since 0.0.1
/// @brief Represents one error/warning in the check result list
class SemanticCheckEntry
{
public:
    SemanticCheckEntry(SemanticCheck::sctype type, const String &what, const SourceInfo &info);

    SemanticCheck::sctype getType(void) const;

    String _str() const;


protected:
    const SemanticCheck::sctype m_type;
    const String m_what;
    const SourceInfo m_info;

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
