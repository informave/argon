

#include "string.hh"

#include <iterator>

ARGON_NAMESPACE_BEGIN

namespace string
{


#define ARGON_FUNCTION_DEF(name) Value func_##name::run(void)


    ARGON_FUNCTION_DEF(concat)
    {
        std::wstringstream ss;

        for(ArgumentList::const_iterator i = m_args.begin();
            i != m_args.end();
            ++i)
        {
            //ss << (*i)->str();
            /// @bug requires better solution since we have ValueElement Refs as args
            ss << (*i)->_value().str();
        }

        return Value(String(ss.str()));
    }



    ARGON_FUNCTION_DEF(len)
    {
        std::wstringstream ss;

        for(ArgumentList::const_iterator i = m_args.begin();
            i != m_args.end();
            ++i)
        {
            ss << (*i)->_value().str();
        }
    
        return Value((int)ss.str().length()); /// @bug remove cast
    }


    ARGON_FUNCTION_DEF(debug1)
    {
        //std::wstringstream ss;

	assert(m_args.size() == 1);
	std::cerr << "[debug():] " << "{" << (*m_args.begin())->_value().data().datatype() << "} " <<
	(*m_args.begin())->_value().str() << std::endl;
/*
        for(ArgumentList::const_iterator i = m_args.begin();
            i != m_args.end();
            ++i)
        {
            ss << (*i)->_value().str();
        }
*/
        //return Value((int)ss.str().length()); /// @bug remove cast
	return Value();
    }

}


template<typename T>
static Function* factory_function(Processor &proc, const ArgumentList &args)
{
    return new T(proc, args);
}



builtin_func_def table_string_funcs[] =
{
    { "string.concat",  factory_function<string::func_concat>, 2, -1 },
    { "string.len",     factory_function<string::func_len>,    1,  1 },
    { "debug",		factory_function<string::func_debug1>,	1, 1 },
    { NULL, NULL, 0, 0 }
};


//bbb = { 2 };

ARGON_NAMESPACE_END

