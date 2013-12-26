

#include "seq.hh"

#include <iterator>

ARGON_NAMESPACE_BEGIN

namespace seq
{


#define ARGON_FUNCTION_DEF(name) Value func_##name::run(void)


    ARGON_FUNCTION_DEF(next)
    {
        ARGON_ICERR(m_args.size() == 1, "invalid args count");

        Ref x = m_args[0];
        Sequence *p = x.cast<Sequence>();
        ARGON_ICERR(p, "invalid seq ptr");
        p->nextValue();
        return p->_value();
    }



    ARGON_FUNCTION_DEF(reset)
    {
        ARGON_ICERR(m_args.size() == 2, "invalid args count");

        Ref x = m_args[0];
        Sequence *p = x.cast<Sequence>();
        ARGON_ICERR(p, "invalid seq ptr");

        int newVal = m_args[1]->_value().data().get<int>();

        Value oldVal = p->_value();

        p->resetValue(db::Variant(newVal));
        return oldVal;
    }



    ARGON_FUNCTION_DEF(current)
    {
        ARGON_ICERR(m_args.size() == 1, "invalid args count");

        Ref x = m_args[0];
        Sequence *p = x.cast<Sequence>();
        ARGON_ICERR(p, "invalid seq ptr");
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
    { "seq.next",     factory_function<seq::func_next>,    1, 1 },
    { "seq.reset",    factory_function<seq::func_reset>,   2, 2 },
    { "seq.current",  factory_function<seq::func_current>, 1, 1 },
    { NULL, NULL, 0, 0 }
};


//bbb = { 2 };

ARGON_NAMESPACE_END

