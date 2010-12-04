//
// elements.cc - Elements (definition)
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
/// @brief Elements (definition)
/// @author Daniel Vogelbacher
/// @since 0.1

#include "argon/dtsengine.hh"
#include "argon/exceptions.hh"


#include <iostream>
#include <sstream>

ARGON_NAMESPACE_BEGIN


//--------------------------------------------------------------------------
/// Task Child Visitor
///
/// @since 0.0.1
/// @brief Task Child Visitor
struct TaskChildVisitor : public Visitor
{
public:
    TaskChildVisitor(Processor &proc, Task &task)
        : Visitor(Visitor::ignore_none),
          m_proc(proc),
          m_task(task)
    {}


    virtual void visit(LogNode *node)
    {
        LogCmd cmd(this->m_proc, node);
        cmd.exec();
    }

    
    virtual void visit(TaskExecNode *node)
    {
        std::cout << "calling task: " << node->taskid().str() << std::endl;
        //foreach_node(node->getChilds(), PrintTreeVisitor(this->m_proc, std::wcout), 1);

        Task* task = this->m_proc.getSymbol<Task>(node->taskid());

        // get arguments
        this->m_proc.call(task, ArgumentList());
    }

private:
    Processor &m_proc;
    Task      &m_task;
};




//--------------------------------------------------------------------------
/// Log Child Visitor
///
/// @since 0.0.1
/// @brief Log Child Visitor
struct LogChildVisitor : public Visitor
{
public:
    LogChildVisitor(Processor &proc, std::wstringstream &stream)
        : Visitor(),
          m_proc(proc),
          m_stream(stream)
    {}


    virtual void visit(IdNode *node)
    {
        Element* elem = this->m_proc.getSymbol<Element>(node->data());
        m_stream << elem->str();
    }


    virtual void visit(LiteralNode *node)
    {
        m_stream << node->str();
    }

private:
    Processor          &m_proc;
    std::wstringstream &m_stream;

};


//..............................................................................
//////////////////////////////////////////////////////////////////////// Element

/// @details
/// 
Element::Element(Processor &proc)
    : m_proc(proc)
{}



//..............................................................................
///////////////////////////////////////////////////////////////////////// LogCmd

/// @details
/// 
LogCmd::LogCmd(Processor &proc, LogNode *node)
    : Element(proc),
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
void
LogCmd::exec(void)
{
    std::cout << "exec log" << std::endl;

    std::wstringstream ss;

    foreach_node(this->m_node->getChilds(), LogChildVisitor(this->proc(), ss), 1);

    std::wcout << L"[LOG]: " << ss.str() << std::endl;
}



//..............................................................................
/////////////////////////////////////////////////////////////////////////// Task

/// @details
/// 
Task::Task(Processor &proc, TaskNode *node)
    : Element(proc),
      m_node(node)
{
    std::cout << "Processing task: " << node->id << std::endl;
}


/// @details
/// 
String
Task::str(void) const
{
    String s;
    s.append(this->id().str());
    s.append("[TASK]");
    return s;
}


/// @details
/// 
String
Task::name(void) const
{
    return this->id().str();
}


/// @details
/// 
String
Task::type(void) const
{
    return "TASK";
}


/// @details
/// 
SourceInfo
Task::getSourceInfo(void) const
{
    return this->m_node->getSourceInfo();
}


/// @details
/// 
Value
Task::run(const ArgumentList &args)
{
    std::cout << "running task: " << this->id() << std::endl;

//    std::cout << debug::ArgsPrinter(args) << std::endl;

    foreach_node( this->m_node->getChilds(), TaskChildVisitor(this->proc(), *this), 1);


    if(this->id() != Identifier("main"))
        throw RuntimeError(this->proc().getStack());


    return Value();

    //this->proc().call(t);


    //this->m_script.getTask(Identifier("foo"));
}



//..............................................................................
///////////////////////////////////////////////////////////////////// Connection

/// @details
/// 
Connection::Connection(Processor &proc, ConnNode *node, db::ConnectionMap &userConns)
    : Element(proc),
      m_node(node),
      m_dbc(0),
      m_alloc_env(),
      m_alloc_dbc()
{
    std::cout << "Processing connection: " << node->id << std::endl;

    if(userConns[node->id])
    {
        std::cout << "Using user-supplied connection: " << node->id << std::endl;
        this->m_dbc = userConns[node->id];
    }
    else
    {
        if(node->spec->type.empty())
        {
            throw std::runtime_error("no dbc type given");
        }
        this->m_alloc_env.reset(new db::Database::Environment(node->spec->type));
        this->m_alloc_dbc.reset(this->m_alloc_env->newConnection());
        this->m_dbc = this->m_alloc_dbc.get();
        this->m_dbc->connect(node->spec->dbcstr);
    }
}


/// @details
/// 
String
Connection::str(void) const
{
    String s;
    s.append(this->id().str());
    s.append("[CONNECTION]");
    return s;
}


/// @details
/// 
SourceInfo
Connection::getSourceInfo(void) const
{
    return this->m_node->getSourceInfo();
}


/// @details
/// 
String
Connection::name(void) const
{
    return this->id().str();
}


/// @details
/// 
String
Connection::type(void) const
{
    return "CONNECTION";
}


/// @details
/// 
db::Connection&
Connection::getDbc(void)
{
    assert(this->m_dbc);
    return *this->m_dbc;
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
