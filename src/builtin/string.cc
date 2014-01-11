

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
        ARGON_ICERR_CTX(m_args.size() == 1, *this, "string.len() req argument count: 1");

		if(m_args[0]->_value().data().isnull())
			return db::Variant();

        String str = m_args[0]->_value().str();
        return str.length();
    }


    ARGON_FUNCTION_DEF(truncate)
    {
        ARGON_ICERR_CTX(m_args.size() == 2, *this, "string.len() req argument count: 2");

		if(m_args[0]->_value().data().isnull())
			return db::Variant();

        std::wstring str = m_args[0]->_value().str();
        int tsize = m_args[1]->_value().data().get<int>();

        ARGON_ICERR_CTX(tsize >= 0, *this, "string.truncate() argument #2 must be >= 0");

        if(str.length() > tsize)
            str.erase(tsize, std::wstring::npos);
        return String(str);
    }


    ARGON_FUNCTION_DEF(substr)
    {
        ARGON_ICERR_CTX(m_args.size() >= 2 && m_args.size() <= 3, *this, "string.substr() req argument count: 2-3");

		if(m_args[0]->_value().data().isnull())
			return db::Variant();

        std::wstring haystack = m_args[0]->_value().str();
        int start = m_args[1]->_value().data().get<int>();
        ARGON_ICERR_CTX(start > 0, *this, "string.substr() arg #2 must be >= 0");
        int c = 0;
        if(m_args.size() == 3)
            c = m_args[2]->_value().data().get<int>();

        if(haystack.length() == 0 || start >= haystack.length())
            return String();

        if(m_args.size() == 2)
            return String(haystack.substr(start-1, std::wstring::npos));
        else
            return String(haystack.substr(start-1, c));
    }


    ARGON_FUNCTION_DEF(find)
    {
        ARGON_ICERR_CTX(m_args.size() == 2, *this, "string.find() req argument count: 2");

		if(m_args[0]->_value().data().isnull())
			return db::Variant();

        std::wstring haystack = m_args[0]->_value().str();
        std::wstring needle = m_args[1]->_value().str();

        size_t a = haystack.find(needle);
        if(a == std::wstring::npos)
            return -1;
        else
            return a+1;
    }


    ARGON_FUNCTION_DEF(char)
    {
        ARGON_ICERR_CTX(m_args.size() == 2, *this, "string.find() req argument count: 2");

        std::wstring haystack = m_args[0]->_value().str();
        int pos = m_args[1]->_value().data().get<int>();
        ARGON_ICERR_CTX(pos >= 1 && pos <= haystack.length(), *this, "position for string.char() is out of range");

        return String(std::wstring(1, haystack.at(pos-1)));
    }


    ARGON_FUNCTION_DEF(contains)
    {
        ARGON_ICERR_CTX(m_args.size() == 2, *this, "string.find() req argument count: 2");

		if(m_args[0]->_value().data().isnull())
			return db::Variant();

        std::wstring haystack = m_args[0]->_value().str();
        std::wstring needle = m_args[1]->_value().str();

        size_t a = haystack.find(needle);
        if(a == std::wstring::npos)
            return db::Variant(bool(false));
        else
            return db::Variant(bool(true));
    }

    ARGON_FUNCTION_DEF(lfill)
    {
        ARGON_ICERR_CTX(m_args.size() == 3, *this, "string.lfill() req argument count: 3");

		if(m_args[0]->_value().data().isnull())
			return db::Variant();

        std::wstring data = m_args[0]->_value().str();
        size_t c = m_args[1]->_value().data().get<int>();
        std::wstring fill = m_args[2]->_value().str();

	/// @bug fill must equal size 1
        while(data.length() < c)
        {
            data = fill + data;
        }
        return db::Variant(String(data));
    }

    ARGON_FUNCTION_DEF(rfill)
    {
        ARGON_ICERR_CTX(m_args.size() == 3, *this, "string.rfill() req argument count: 3");

		if(m_args[0]->_value().data().isnull())
			return db::Variant();

        std::wstring data = m_args[0]->_value().str();
        size_t c = m_args[1]->_value().data().get<int>();
        std::wstring fill = m_args[2]->_value().str();

	/// @bug fill must equal size 1
        while(data.length() < c)
        {
            data += fill;
        }
        return db::Variant(String(data));
    }


	struct no_separator : std::numpunct<char>
    {
    protected:
        virtual string_type do_grouping() const
            { return "\000"; } // groups of 0 (disable)
    };

    ARGON_FUNCTION_DEF(numeric)
    {
        ARGON_ICERR_CTX(m_args.size() == 1, *this, "string.numeric() req argument count: 1");

		if(m_args[0]->_value().data().isnull())
			return db::Variant();

		std::locale loc(std::locale("C"), new no_separator());

        String data = m_args[0]->_value().data().asStr(loc);
        return db::Variant(data);
    }



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

}


template<typename T>
static Function* factory_function(Processor &proc, const ArgumentList &args)
{
    return new T(proc, args);
}



builtin_func_def table_string_funcs[] =
{
    { "string.concat",   factory_function<string::func_concat>,    2, -1 },
    { "string.len",      factory_function<string::func_len>,       1,  1 },
    { "string.truncate", factory_function<string::func_truncate>,  2,  2 },
    { "string.substr",   factory_function<string::func_substr>,    2,  3 },
    { "string.find",     factory_function<string::func_find>,      2,  2 },
    { "string.char",     factory_function<string::func_char>,      2,  2 },
    { "string.contains", factory_function<string::func_contains>,  2,  2 },
    { "string.lfill",    factory_function<string::func_lfill>,     3,  3 },
    { "string.rfill",    factory_function<string::func_rfill>,     3,  3 },
    { "string.numeric",  factory_function<string::func_numeric>,   1,  1 },
    { "debug",		     factory_function<string::func_debug1>,	   1,  1 },
    { NULL, NULL, 0, 0 }
};


//bbb = { 2 };

ARGON_NAMESPACE_END

