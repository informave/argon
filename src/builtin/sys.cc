

#include "sys.hh"

#include <iterator>

ARGON_NAMESPACE_BEGIN

namespace sys
{


#define ARGON_FUNCTION_DEF(name) Value func_##name::run(void)



    ARGON_FUNCTION_DEF(terminate)
    {
        ARGON_ICERR(m_args.size() <= 1, "invalid args count");
        if(m_args.size())
        {
            throw TerminateControlException((*m_args.begin())->_value());
        }

        throw TerminateControlException(Value(int(0)));
    }

    ARGON_FUNCTION_DEF(isnull)
    {
        ARGON_ICERR(m_args.size() == 1, "invalid args count");
        return m_args[0]->_value().data().isnull();
    }

    ARGON_FUNCTION_DEF(charseq)
    {
        ARGON_ICERR(m_args.size() >= 1, "invalid args count");
        
        std::wstring s;
        s.resize(m_args.size());
        std::wstring::iterator i = s.begin();

        std::for_each(m_args.begin(), m_args.end(), [&i](Ref ref)
                      {
                          *i++ = ref->_value().data().get<unsigned int>();
                      });

        return String(s);
    }

    ARGON_FUNCTION_DEF(byteseq)
    {
        ARGON_ICERR(m_args.size() >= 1, "invalid args count");
        
        std::basic_string<unsigned char> s;
        s.resize(m_args.size());
        std::basic_string<unsigned char>::iterator i = s.begin();

        std::for_each(m_args.begin(), m_args.end(), [&i](Ref ref)
                      {
                          *i++ = ref->_value().data().get<unsigned char>();
                      });

        return db::Variant(informave::db::TVarbinary(s.data(), s.size()));
    }

    ARGON_FUNCTION_DEF(newline)
    {
        ARGON_ICERR(m_args.size() == 0, "invalid args count");
        return String(std::string("\r\n")); /// @bug fixme
    }


}

template<typename T>
static Function* factory_function(Processor &proc, const ArgumentList &args)
{
    return new T(proc, args);
}




builtin_func_def table_sys_funcs[] =
{
    { "isnull", factory_function<sys::func_isnull>, 1, 1},
    { "sys.terminate",  factory_function<sys::func_terminate>, 0, 1 },
    { "sys.charseq", factory_function<sys::func_charseq>, 1, -1},
    { "sys.byteseq", factory_function<sys::func_byteseq>, 1, -1},
    { "sys.newline", factory_function<sys::func_newline>, 0, 0},
    { NULL, NULL, 0, 0 }
};



//bbb = { 2 };

ARGON_NAMESPACE_END

