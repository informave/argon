


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

#define ARGON_SIMPLE_FUNCTION(name)                     \
    class func_##name : public Function                 \
    {                                                   \
    public:                                             \
                                                        \
        func_##name(Processor &proc) : Function(proc)   \
        {}                                              \
                                                        \
        virtual ~func_##name(void)                      \
        {}                                              \
                                                        \
    protected:                                          \
        virtual Value run(const ArgumentList &args);    \
    }



namespace string
{
    ARGON_SIMPLE_FUNCTION(concat);
    ARGON_SIMPLE_FUNCTION(len);
}


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
