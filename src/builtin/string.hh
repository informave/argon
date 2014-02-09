


#ifndef INFORMAVE_ARGON_BUILTIN_STRING_HH
#define INFORMAVE_ARGON_BUILTIN_STRING_HH

#include "argon/dtsengine.hh"
#include "argon/exceptions.hh"
#include "argon/ast.hh"
#include "../debug.hh"

#include <iostream>
#include <sstream>
#include <list>

ARGON_NAMESPACE_BEGIN

#define ARGON_SIMPLE_FUNCTION(name)                                     \
    class func_##name : public Function                                 \
    {                                                                   \
public:                                                                 \
                                                                        \
        func_##name(Processor &proc, const ArgumentList &args)          \
            : Function(proc, NULL_NODE, args)                           \
        {}                                                              \
                                                                        \
        virtual ~func_##name(void)                                      \
        {}                                                              \
                                                                        \
    protected:                                                          \
        virtual Value run(void);                                        \
    }



namespace string
{
    ARGON_SIMPLE_FUNCTION(concat);
    ARGON_SIMPLE_FUNCTION(len);

    ARGON_SIMPLE_FUNCTION(truncate);
    ARGON_SIMPLE_FUNCTION(substr);
    ARGON_SIMPLE_FUNCTION(find);
    //ARGON_SIMPLE_FUNCTION(find_nth);
    ARGON_SIMPLE_FUNCTION(char);
    ARGON_SIMPLE_FUNCTION(contains);
    ARGON_SIMPLE_FUNCTION(lfill);
    ARGON_SIMPLE_FUNCTION(rfill);
    ARGON_SIMPLE_FUNCTION(numeric);

    ARGON_SIMPLE_FUNCTION(merge);
    ARGON_SIMPLE_FUNCTION(merge_if);

    ARGON_SIMPLE_FUNCTION(isset);

    ARGON_SIMPLE_FUNCTION(debug1);
}




extern builtin_func_def table_string_funcs[];



ARGON_NAMESPACE_END


#endif


//
// Local Variables:
// mode: C++
// c-file-style: "bsd"
// c-basic-offset: 4
// indent-tabs-mode: nil
// End:
//
