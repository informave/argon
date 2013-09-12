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
#include "debug.hh"
#include "visitors.hh"

#include <iostream>
#include <functional>
#include <sstream>

ARGON_NAMESPACE_BEGIN





    ScopedStackFrame::ScopedStackFrame(Processor &proc)
        : m_stack(proc.m_stack),
          m_saved_size(m_stack.size())
    {
	//        m_pos = m_stack.size();
    }

    ScopedStackFrame::~ScopedStackFrame(void)
    {
        //assert(!m_stack.empty());
	ARGON_ICERR(m_stack.size() >= m_saved_size, "Stack error");

	while((!m_stack.empty()) && m_stack.size() > m_saved_size)
	{
		Element *ptr = *m_stack.rbegin();
		m_stack.pop_back();
		delete ptr;
	}

/*
		while((!m_stack.empty()) && m_stack.begin() != m_pos)
		{
			Element *ptr = *m_stack.begin();
			m_stack.pop_front();
			delete ptr;
		}
		*/
			/*
        for(Processor::stack_type::iterator i = m_stack.begin();
            (!m_stack.empty()) && i != m_pos;
             ++i )
        {
			Element *ptr = m_stack.pop_front();
			delete (*i);
			*i = 0;
			m_stack.pop_front();

			m_stack.erase(i);
			
			i = m_stack.begin();


		
            delete (*i);
			Processor::stack_type::iterator j = i;
			++i;
            m_stack.erase(j);
			
        }
		*/
    }



//..............................................................................
///////////////////////////////////////////////////////////////////////// Column

/// @details
/// 
const db::Value&
Column::getFrom(db::Resultset &rs, Context &context)
{
    using informave::db::ex::not_found;
    try
    {
        if(Column::by_number == mode())
            return rs.column(getNum());
        else
            return rs.column(getName());
    }
    catch(not_found &e)
    {
        throw FieldNotFound(context, e.what()); /// @bug replace what with colum name
    }
}



//..............................................................................
////////////////////////////////////////////////////// SqlObjectColAssignVisitor
///
/// @since 0.0.1
/// @brief Object Child Visitor
struct ObjectRulesVisitor : public Visitor
{
public:
    ObjectRulesVisitor(Processor &proc, Context &context, Object &obj)
        : Visitor(Visitor::ignore_none),
          m_proc(proc),
          m_context(context),
          m_obj(obj)
    {}


    virtual void visit(LogNode *node)
    {
        LogCmd cmd(this->m_proc, m_context, node);
        this->m_proc.call(cmd);
    }


    virtual void visit(ColumnAssignNode *node)
    {
        ARGON_ICERR(node->getChilds().size() == 2, "invalid child count");
        
        //ColumnAssignOp op(this->m_proc, m_context, node);
        //this->m_proc.call(op);
        
        Column col;
        LValueColumnVisitor(this->m_proc, this->m_context, col)(node->getChilds()[0]);
        
        Value val;
        EvalExprVisitor eval(this->m_proc, this->m_context, val);
        eval(node->getChilds()[1]);

        //this->m_context.getMainObject()->setColumn(col, val);

        this->m_obj.setColumn(col, val);


/*
//Column col(dynamic_cast<ColumnNode*>(node->getChilds()[0]));
        
Value val;
EvalExprVisitor eval(this->m_proc, this->m_context, val);
eval(node->getChilds()[1]);
        
this->m_context.getDestObject()->setColumn(col, val);
*/      
        
        //this->m_context.getMainObject()->setColumn(Column("id"), Value(23));
        //this->m_context.getMainObject()->setColumn(Column("name"), Value(23));

        // ARGON_ICERR_CTX.., assign not allowed?
    }
    
private:
    Processor &m_proc;
    Context &m_context;
    //SourceTable      &m_sourcetable;
    Object &m_obj;
};





//..............................................................................
///////////////////////////////////////////////////////////////////////// Object

/// @details
/// 
Object::Object(Processor &proc, DeclNode *node, const ArgumentList &args)
    : Context(proc, args),
      m_column_mappings(),
      m_node(node),
      m_init_nodes(),
      m_reading_nodes(),
      m_writing_nodes(),
      m_final_nodes()
      
{
    if(this->m_node) // only process custom objects
    {
        {
            Node *n = find_node<DeclInitNode>(this->m_node);
            if(n && (n = find_node<DeclRuleBlockNode>(n)))
            {
                this->m_init_nodes = n->getChilds();
                ARGON_ICERR(find_nodes<ColumnNode>(n).empty(), "column node not empty");
                ARGON_ICERR(find_nodes<ColumnNumNode>(n).empty(), "columnnum node not empty");
                ARGON_ICERR(find_nodes<ResColumnNode>(n).empty(), "columnres node not empty");
                ARGON_ICERR(find_nodes<ResColumnNumNode>(n).empty(), "columnresnum node not empty");

                // this is allowed for sql object which prepares SELECT statements
                // with parameter assignment
                //ARGON_ICERR(find_nodes<ColumnAssignNode>(n).empty(), "columnassign node not empty");
            }
        }


        {
            Node *n = find_node<DeclReadingNode>(this->m_node);
            if(n && (n = find_node<DeclRuleBlockNode>(n)))
            {
                this->m_reading_nodes = n->getChilds();
                ARGON_ICERR(find_nodes<ResColumnNode>(n).empty(), "rescolumn node not empty");
                ARGON_ICERR(find_nodes<ResColumnNumNode>(n).empty(), "rescolumnnum node not empty");
                ARGON_ICERR(find_nodes<ColumnAssignNode>(n).empty(), "columnassign node not empty");
            }
        }

        {
            Node *n = find_node<DeclWritingNode>(this->m_node);
            if(n && (n = find_node<DeclRuleBlockNode>(n)))
            {
                this->m_writing_nodes = n->getChilds();
                ARGON_ICERR(find_nodes<ResColumnNode>(n).empty(), "rescolumn node not empty");
                ARGON_ICERR(find_nodes<ResColumnNumNode>(n).empty(), "rescolumnnum node not empty");
            }
        }

        {
            Node *n = find_node<DeclFinalNode>(this->m_node);
            if(n && (n = find_node<DeclRuleBlockNode>(n)))
                this->m_final_nodes = n->getChilds();
        }
    }
}


/// @details
/// This runs some code that is required by all objects like Arguments->Symboltable
Value
Object::run(void)
{
    if(this->m_node)
    {
    // Load OBJECTs arguments to local symbol table
    this->getSymbols().reset();
    safe_ptr<ArgumentsSpecNode> argsSpecNode = find_node<ArgumentsSpecNode>(this->m_node);

    ARGON_ICERR_CTX(argsSpecNode.get() != 0, *this,
                "no argument specification");
    
    ARGON_ICERR_CTX(argsSpecNode->getChilds().size() == this->getCallArgs().size(), *this,
                "Argument count mismatch");
    
    ArgumentList::const_iterator i = this->getCallArgs().begin();
    foreach_node(argsSpecNode->getChilds(),
                 Arg2SymVisitor(this->proc(), this->getSymbols(), i, this->getCallArgs().end()), 1);
    }


    


    return Value();
}


void
Object::doInit(void)
{
    foreach_node( this->m_init_nodes, ObjectRulesVisitor(this->proc(), *this, *this), 1);
}

void
Object::doReading(void)
{
    foreach_node( this->m_reading_nodes, ObjectRulesVisitor(this->proc(), *this, *this), 1);
}

void
Object::doWriting(void)
{
    foreach_node( this->m_writing_nodes, ObjectRulesVisitor(this->proc(), *this, *this), 1);
}

void
Object::doFinal(void)
{
    foreach_node( this->m_final_nodes, ObjectRulesVisitor(this->proc(), *this, *this), 1);
}



/// @details
/// 
/*
Value
Object::resolveColumn(const Column &col)
{
    throw std::runtime_error("resolve() not allowed on objects");
}
*/


/// @details
 ///
Object*
Object::getMainObject(void)
{
    // we return the object itself. This is a good workaround
    // for requesting the object by visitors when processing custom
    // object rules with column identifiers.
    return this;
}


/// @details
/// 
Object*
Object::getResultObject(void) 
{ 
    ARGON_ICERR_CTX(false, *this,
                "An object does not contains a result object.");
}


/// @details
/// 
Object*
Object::getDestObject(void)
{
    ARGON_ICERR_CTX(false, *this,
                "An object does not contains a destination object.");
}



/// @details
/// 
int
Object::getBindPosition(const Column &col)
{
    std::map<Column, int>::iterator i = this->m_column_mappings.find(col);
    if(i == this->m_column_mappings.end())
        throw std::runtime_error(String("unknown column: ") + col.getName());
    else
        return i->second;
}


/// @details
/// 
void
Object::addBindPosition(const Column &col, int pos)
{
    this->m_column_mappings[col] = pos;
}





Ref::Ref(Element *elem) : m_element(elem)
{
    ARGON_ICERR(elem, "invalid element");
    this->m_element->registerRef(this);
}

Ref::Ref(const Ref &orig) : m_element(orig.m_element)
{
    ARGON_ICERR(m_element, "Invalid element");
    this->m_element->registerRef(this);
}

Ref::~Ref(void)
{
    if(!this->dead())
      this->m_element->unregisterRef(this);
}


Ref&
Ref::operator=(const Ref& r)
	{
		if(!this->dead())
		{
			this->m_element->unregisterRef(this);
			this->m_element = 0;
		}
		if(!r.dead())
		{
			this->m_element = r.m_element;
			this->m_element->registerRef(this);
		}
		return *this;
	}


Element*
Ref::operator->(void)
{
    ARGON_ICERR(this->m_element, "Dead reference, the element did not exist!");
    return this->m_element;
}

const Element*
Ref::operator->(void) const
{
    ARGON_ICERR(this->m_element, "Dead reference, the element did not exist!");
    return this->m_element;
}


/*
Element*
Ref::operator&(void)
{
    ARGON_ICERR(this->m_element, "Dead reference, the element did not exist!");
    return this->m_element;
}
*/



//..............................................................................
//////////////////////////////////////////////////////////////////////// Element

/// @details
/// 
Element::Element(Processor &proc)
    : m_references(),
      m_proc(proc)
{}


Element::~Element(void)
{
    if(this->m_references.size())
    {
    std::cerr << "Element '" << typeid(this).name() << "' has " << this->m_references.size()
       << " active references, now marked as dead!" << std::endl;


    std::for_each(this->m_references.begin(), this->m_references.end(), std::mem_fun(&Ref::mark_dead));
    }

    /// @bug better error message
    //ARGON_ICERR(this->m_references.empty(), ss.str());
}


void
Element::registerRef(Ref *p)
{
    this->m_references.push_back(p);
}



void
Element::unregisterRef(Ref *p)
{
    this->m_references.remove(p);
}




//..............................................................................
//////////////////////////////////////////////////////////////////////// Context

/// @details
/// 
Context::Context(Processor &proc, const ArgumentList &args)
    : Element(proc),
      m_symbols(&proc.getGlobalContext().getSymbols()),
      m_args(args),
      m_exception_info(0)
{}


/// @details
///
Context::Context(Processor &proc, SymbolTable *parentptr, const ArgumentList &args)
    : Element(proc),
      m_symbols(parentptr),
      m_args(args),
      m_exception_info(0)
{}



/// @details
/// 
void
Context::setCurrentException(IExceptionInfo *info)
{
    ARGON_ICERR(this->m_exception_info == 0, "exception info is null");
    this->m_exception_info = info;
}


/// @details
/// 
void
Context::releaseCurrentException(void)
{
    ARGON_ICERR(this->m_exception_info != 0, "exception info");
    delete this->m_exception_info;
    this->m_exception_info = 0;
}


IExceptionInfo&
Context::getCurrentException(void)
{
    if(this->m_exception_info)
        return *this->m_exception_info;
    else
        throw std::runtime_error("no exception info set");
}


bool
Context::hasCurrentException(void) const
{
    return this->m_exception_info != 0;
}


/// @details
/// 
SymbolTable&
Context::getSymbols(void)
{
    return this->m_symbols;
}


/// @details
/// 
const SymbolTable&
Context::getSymbols(void) const
{
    return this->m_symbols;
}


/// @details
/// 
Value
Element::run()
{ 
    throw std::runtime_error("is not callabled");
}



ScopedReleaseException::ScopedReleaseException(Context &ctx)
    : m_ctx(ctx)
{}

ScopedReleaseException::~ScopedReleaseException(void)
{
    this->m_ctx.releaseCurrentException();
}


//..............................................................................
/////////////////////////////////////////////////////////////////// ValueElement

/// @details
/// 
ValueElement::ValueElement(Processor &proc, const Value &val)
    : Element(proc),
      m_value(val)
{
}


/// @details
/// 
String
ValueElement::str(void) const
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
/// ValueElement does not contains any source information.
SourceInfo
ValueElement::getSourceInfo(void) const
{
    return SourceInfo();
}


/// @details
/// 
String
ValueElement::name(void) const
{
    if(!this->m_value.data().isnull())
    	return this->m_value.data().asStr();
    else
    	return "<NULL>";
}


/// @details
/// 
String
ValueElement::type(void) const
{
    return "VALUE";
}


/// @details
/// 
Value
ValueElement::_value(void) const
{
    return this->m_value;
}

/// @details
/// 
String
ValueElement::_string(void) const
{
    return this->_value().data().asStr();
}

/// @details
/// 
String
ValueElement::_name(void) const
{
    //return String("unknown-id") + String(" (function)");
    return "VALUE";
}

/// @details
/// 
String
ValueElement::_type(void) const
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
