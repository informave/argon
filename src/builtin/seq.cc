

#include "seq.hh"

#include <iterator>

ARGON_NAMESPACE_BEGIN

namespace seq
{


#define ARGON_FUNCTION_DEF(name) Value func_##name::run(void)


    ARGON_FUNCTION_DEF(next)
    {
        assert(m_args.size() == 1);

        Ref x = m_args[0];
        Sequence *p = x.cast<Sequence>();
        assert(p);
        p->nextValue();
        return p->_value();
    }



}


template<typename T>
static Function* factory_function(Processor &proc, const ArgumentList &args)
{
    return new T(proc, args);
}



builtin_func_def table_seq_funcs[] =
{
    { "seq.next",  factory_function<seq::func_next>, 1, 1 },
    { NULL, NULL, 0, 0 }
};


//bbb = { 2 };

ARGON_NAMESPACE_END

