

#include "numeric.hh"

#include "../utils.hh"

#include <dbwtl/dbobjects>

#include <iterator>
#include <functional>
#include <algorithm>
#include <locale>

ARGON_NAMESPACE_BEGIN

namespace numeric
{


    using namespace informave::db;

#define ARGON_FUNCTION_DEF(name) Value func_##name::run(void)

    ARGON_FUNCTION_DEF(format)
    {
        ARGON_ICERR(BETWEEN(m_args.size(), 1, 4) , "invalid args count");

        Variant val, dec, thousand_sep, dec_sep;

        val = this->m_args[0]->_value().data();
        if(val.isnull())
            return Value();
        if(m_args.size() >= 2)
            dec = this->m_args[1]->_value().data();
        if(m_args.size() >= 3)
            thousand_sep = this->m_args[2]->_value().data();
        if(m_args.size() >= 4)
            dec_sep = this->m_args[3]->_value().data();

        TNumeric x = val.get<TNumeric>();


        std::string s;
        std::transform(x.nibbles().begin(), x.nibbles().end(),
                       std::back_inserter(s), [&](int v){return v+0x30;});

        std::string lpart;
        std::string rpart;


        std::locale loc("");
        std::numpunct<std::string::value_type> const &n = std::use_facet<std::numpunct<std::string::value_type> >(loc);

        if(x.scale())
        {
            lpart = s.substr(0, s.size() - x.scale());
            rpart = s.substr(s.size() - x.scale());
        }

        if(!dec.isnull())
        {
            rpart.resize(dec.get<int>());
        }

        std::string tsep(1, n.thousands_sep());
        if(!thousand_sep.isnull())
        {
            tsep = thousand_sep.get<String>();
        }

        std::string dsep(1, n.decimal_point());
        if(!dec_sep.isnull())
        {
            dsep = dec_sep.get<String>();
        }


        s.clear();
        for(auto i = lpart.rbegin();
            i != lpart.rend();
            ++i)
        {
            if(s.size() && std::distance(i, lpart.rbegin()) % 3 == 0)
                s = tsep + s;
            s = *i + s;
        }

        if(rpart.size())
            s += dsep + rpart;

        if(x.sign() == false)
            s = "-" + s;

        return Value(String(s, "US-ASCII"));
    }


    ARGON_FUNCTION_DEF(from_string)
    {
        ARGON_ICERR(m_args.size() == 1 , "invalid args count");

        return Value(Variant(TNumeric(this->m_args[0]->_value().data().get<String>())));
    }

    ARGON_FUNCTION_DEF(cast)
    {
        ARGON_ICERR(m_args.size() == 1 , "invalid args count");

        if(m_args[0]->_value().data().isnull())
            return Value();

        return Variant(m_args[0]->_value().data().get<TNumeric>());
    }


}


template<typename T>
static Function* factory_function(Processor &proc, const ArgumentList &args)
{
    return new T(proc, args);
}



builtin_func_def table_numeric_funcs[] =
{
    { "numeric.format",         factory_function<numeric::func_format>,         1,  4 },
    { "numeric.from_string",    factory_function<numeric::func_from_string>,    1,  1 },
    { "numeric.cast",           factory_function<numeric::func_cast>,           1,  1 },
    { NULL, NULL, 0, 0 }
};


//bbb = { 2 };

ARGON_NAMESPACE_END

