

#include "string.hh"

#include <iterator>

ARGON_NAMESPACE_BEGIN

namespace string
{


#define ARGON_FUNCTION_DEF(name) Value func_##name::run(const ArgumentList &args)


    ARGON_FUNCTION_DEF(concat)
    {
        std::wstringstream ss;

        for(ArgumentList::const_iterator i = args.begin();
            i != args.end();
            ++i)
        {
            ss << i->data();
        }

        return Value(String(ss.str()));
    }



    ARGON_FUNCTION_DEF(len)
    {
        std::wstringstream ss;

        for(ArgumentList::const_iterator i = args.begin();
            i != args.end();
            ++i)
        {
            ss << i->data();
        }
    
        return Value((int)ss.str().length()); /// @bug remove cast
    }



}

ARGON_NAMESPACE_END

