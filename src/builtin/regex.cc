

#include "regex.hh"

#include <iterator>

#include <boost/regex.hpp>

ARGON_NAMESPACE_BEGIN

namespace regex
{


#define ARGON_FUNCTION_DEF(name) Value func_##name::run(void)


    ARGON_FUNCTION_DEF(match)
    {
        ARGON_ICERR(m_args.size() == 2, "invalid args count");

        String val = m_args[0]->_value().str();
        String pattern = m_args[1]->_value().str();

        std::wstring w_val = val;
        std::wstring w_pattern = pattern;

        boost::wsmatch what;

        try
        {
            boost::wregex re(w_pattern, boost::regex_constants::perl);
            
            bool w_result = boost::regex_search(w_val, what, re);
            
            return db::Variant(w_result);
            
        }
        catch(boost::bad_expression &e)
        {
            ARGON_ICERR_CTX(false, *this, "bad regular expression");
        }
    }


    ARGON_FUNCTION_DEF(search_n)
    {
        ARGON_ICERR(m_args.size() == 3, "invalid args count");

        String val = m_args[0]->_value().str();
        String pattern = m_args[1]->_value().str();
        int num = m_args[2]->_value().data().get<int>();
        ARGON_ICERR_CTX(num > 0, *this, "argument #3 must be > 0");

        std::wstring w_val = val;
        std::wstring w_pattern = pattern;

        boost::wsmatch what;

        try
        {
            boost::wregex re(w_pattern, boost::regex_constants::perl);
            
            bool w_result = boost::regex_search(w_val, what, re);

            
            if(w_result && what.size() >= num)
                return String(std::wstring(what[num-1]));
            else
                return Value();
            
        }
        catch(boost::bad_expression &e)
        {
            ARGON_ICERR_CTX(false, *this, "bad regular expression");
        }
    }


    ARGON_FUNCTION_DEF(replace)
    {
        ARGON_ICERR(m_args.size() == 3, "invalid args count");

        String val = m_args[0]->_value().str();
        String pattern = m_args[1]->_value().str();
        String replace = m_args[2]->_value().str();

        std::wstring w_val = val;
        std::wstring w_pattern = pattern;
        std::wstring w_replace = replace;

        try
        {

            boost::wregex re(w_pattern, boost::regex_constants::perl);

            std::wstring w_result = boost::regex_replace(w_val, re, w_replace);

            return String(w_result);
        }
        catch(boost::bad_expression &e)
        {
            ARGON_ICERR_CTX(false, *this, "bad regular expression");
        }
    }



}


template<typename T>
static Function* factory_function(Processor &proc, const ArgumentList &args)
{
    return new T(proc, args);
}



builtin_func_def table_regex_funcs[] =
{
    { "regex.match",  factory_function<regex::func_match>, 2, 2 },
    { "regex.search_n",  factory_function<regex::func_search_n>, 3, 3 },
    { "regex.replace",  factory_function<regex::func_replace>, 3, 3 },
    { NULL, NULL, 0, 0 }
};


//bbb = { 2 };

ARGON_NAMESPACE_END

