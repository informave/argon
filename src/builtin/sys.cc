

#include "sys.hh"

#include <iterator>

ARGON_NAMESPACE_BEGIN

namespace sys
{


#define ARGON_FUNCTION_DEF(name) Value func_##name::run(void)



    ARGON_FUNCTION_DEF(terminate)
    {
        assert(m_args.size() <= 1);
        if(m_args.size())
        {
            throw TerminateControlException((*m_args.begin())->_value());
        }

        throw TerminateControlException(Value(int(0)));
    }


}

template<typename T>
static Function* factory_function(Processor &proc, const ArgumentList &args)
{
    return new T(proc, args);
}




builtin_func_def table_sys_funcs[] =
{
    { "sys.terminate",  factory_function<sys::func_terminate>, 0, 1 },
    { NULL, NULL, 0, 0 }
};



//bbb = { 2 };

ARGON_NAMESPACE_END

