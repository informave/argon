


#ifndef INFORMAVE_ARGON_BUILTIN_NUMERIC_HH
#define INFORMAVE_ARGON_BUILTIN_NUMERIC_HH

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



namespace numeric
{
    ARGON_SIMPLE_FUNCTION(format);
    ARGON_SIMPLE_FUNCTION(from_string);
    ARGON_SIMPLE_FUNCTION(cast);
}




extern builtin_func_def table_numeric_funcs[];



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