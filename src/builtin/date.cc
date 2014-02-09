

#include "date.hh"

#include <iterator>

ARGON_NAMESPACE_BEGIN

namespace date
{


#define ARGON_FUNCTION_DEF(name) Value func_##name::run(void)

    ARGON_FUNCTION_DEF(debug1)
    {
        //std::wstringstream ss;

        ARGON_ICERR(m_args.size() == 1, "invalid args count");
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


    using namespace informave::db;


    ARGON_FUNCTION_DEF(encode)
    {
        ARGON_ICERR(m_args.size() == 3 , "invalid args count");

        if(m_args[0]->_value().data().isnull() ||
           m_args[1]->_value().data().isnull() ||
           m_args[2]->_value().data().isnull())
            return Value();

        int year, month, day;
        year = m_args[0]->_value().data().get<int>();
        month = m_args[1]->_value().data().get<int>();
        day = m_args[2]->_value().data().get<int>();
           
        return Value(Variant(TDate(year, month, day)));
    }

    ARGON_FUNCTION_DEF(year)
    {
        ARGON_ICERR(m_args.size() == 1 , "invalid args count");

        if(m_args[0]->_value().data().isnull())
            return Value();

        return Value(m_args[0]->_value().data().get<TDate>().year());
    }

    ARGON_FUNCTION_DEF(month)
    {
        ARGON_ICERR(m_args.size() == 1 , "invalid args count");

        if(m_args[0]->_value().data().isnull())
            return Value();

        return Value(m_args[0]->_value().data().get<TDate>().month());
    }

    ARGON_FUNCTION_DEF(day)
    {
        ARGON_ICERR(m_args.size() == 1 , "invalid args count");

        if(m_args[0]->_value().data().isnull())
            return Value();

        return Value(m_args[0]->_value().data().get<TDate>().day());
    }



	struct no_separator : std::numpunct<char>
    {
    protected:
        virtual string_type do_grouping() const
            { return "\000"; } // groups of 0 (disable)
    };

    static std::string fmt_int(int val, size_t max_len, char fill)
    {
		std::locale loc(std::locale("C"), new no_separator());
        
        std::string res = Variant(val).asStr(loc);
        
        if(res.size() > max_len)
            return res.substr(res.size()-2);
        else if(res.size() < max_len && fill != '\0')
        {
            while(res.size() < max_len)
                res = std::string(1, fill) + res;
        }
        return res;
    }


    ARGON_FUNCTION_DEF(format)
    {
        ARGON_ICERR(m_args.size() == 2 , "invalid args count");

        if(m_args[0]->_value().data().isnull())
            return Value();

        TDate d = m_args[0]->_value().data().get<TDate>();
        std::string fmt = m_args[1]->_value().data().get<String>();

        auto i = fmt.begin();

        std::string res;

        while(i != fmt.end())
        {
            std::string tmp;
            switch(*i)
            {
            case 'y':
                do tmp += *i++; while(i != fmt.end() && *i == 'y');
                if(tmp == "yyyy")
                    res += fmt_int(d.year(), 4, '0');
                else if(tmp == "yy")
                    res += fmt_int(d.year(), 2, '0');
                else
                    throw std::runtime_error(std::string("invalid format: ") + fmt);
                continue;
            case 'd':
                do tmp += *i++; while(i != fmt.end() && *i == 'd');
                if(tmp == "dd")
                    res += fmt_int(d.day(), 2, '0');
                else if(tmp == "d")
                    res += fmt_int(d.day(), 1, '0');
                else
                    throw std::runtime_error(std::string("invalid format: ") + fmt);
                continue;
            case 'm':
                do tmp += *i++; while(i != fmt.end() && *i == 'm');
                if(tmp == "mm")
                    res += fmt_int(d.month(), 2, '0');
                else if(tmp == "m")
                    res += fmt_int(d.month(), 1, '0');
                else
                    throw std::runtime_error(std::string("invalid format: ") + fmt);
                continue;
            default:
                res += *i;
            }
            ++i;
        }

    return Value(String(res, "US-ASCII"));
    }


}


template<typename T>
static Function* factory_function(Processor &proc, const ArgumentList &args)
{
    return new T(proc, args);
}



builtin_func_def table_date_funcs[] =
{
    { "debug2",		     factory_function<date::func_debug1>,	   1,  1 },

    { "date.encode",		 factory_function<date::func_encode>,	   3,  3 },
    { "date.year",		     factory_function<date::func_year>,	       0,  0 },
    { "date.month",		     factory_function<date::func_month>,	   0,  0 },
    { "date.day",		     factory_function<date::func_day>,	       0,  0 },
    { "date.format",		 factory_function<date::func_format>,	   2,  2 },

    { NULL, NULL, 0, 0 }
};


//bbb = { 2 };

ARGON_NAMESPACE_END

