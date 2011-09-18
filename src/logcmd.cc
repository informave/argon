//
// logcmd.cc - LOG Command (definition)
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
/// @brief LOG Command (definition)
/// @author Daniel Vogelbacher
/// @since 0.1

#include "argon/dtsengine.hh"
#include "argon/exceptions.hh"
#include "argon/ast.hh"
#include "debug.hh"
#include "visitors.hh"

#include <iostream>
#include <sstream>
#include <list>

ARGON_NAMESPACE_BEGIN


//..............................................................................
//////////////////////////////////////////////////////////////// LogChildVisitor
///
/// @since 0.0.1
/// @brief Log Child Visitor
struct LogChildVisitor : public Visitor
{
public:
    LogChildVisitor(Processor &proc, Context &context, std::wstringstream &stream)
        : Visitor(),
          m_proc(proc),
          m_stream(stream),
          m_context(context)
    {}


    void fallback_action(Node *node)
    {
        try
        {
            Value val;
            EvalExprVisitor eval(this->m_proc, this->m_context, val);
            eval(node);
            
            m_stream << val.data().asStr(m_stream.getloc());
        }
        catch(informave::db::ex::null_value &err)
        {
            m_stream << "<NULL>";
        }
    }


private:
    Processor          &m_proc;
    std::wstringstream &m_stream;
    Context            &m_context;

};



//..............................................................................
///////////////////////////////////////////////////////////////////////// LogCmd

/// @details
/// 
LogCmd::LogCmd(Processor &proc, Context &context, LogNode *node)
    : Element(proc),
      m_context(context),
      m_node(node)
{}


/// @details
/// 
String
LogCmd::str(void) const
{
    return "[LOG]";
}


/// @details
/// 
String
LogCmd::name(void) const
{
    return ARGON_UNNAMED_ELEMENT;
}

/// @details
/// 
String
LogCmd::type(void) const
{
    return "LOG";
}


/// @details
/// 
SourceInfo
LogCmd::getSourceInfo(void) const
{
    return this->m_node->getSourceInfo();
}


/// @details
/// 
Value
LogCmd::run(const ArgumentList &args)
{
    ARGON_DPRINT(ARGON_MOD_PROC, "Run LogCmd");

    std::wstringstream ss;

    ss.imbue(std::wcout.getloc());

    foreach_node(this->m_node->getChilds(), LogChildVisitor(this->proc(), this->m_context, ss), 1);

    //std::wcout << L"[LOG] " << ss.str() << std::endl;

    this->proc().getEngine().writeLog(ss.str());

    //std::cout << String(ss.str()).to("UTF-8") << std::endl;

    return Value();
}




/// @details
/// 
Value
LogCmd::_value(void) const
{
    return String("LOGCMD");
}

/// @details
/// 
String
LogCmd::_string(void) const
{
    return "LOGCMD";
}

/// @details
/// 
String
LogCmd::_name(void) const
{
    return "LOGCMD";
}

/// @details
/// 
String
LogCmd::_type(void) const
{
    return "LOGCMD";
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
