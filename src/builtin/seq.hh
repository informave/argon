


#ifndef INFORMAVE_ARGON_BUILTIN_SEQ_HH
#define INFORMAVE_ARGON_BUILTIN_SEQ_HH

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



namespace seq
{
    ARGON_SIMPLE_FUNCTION(next);
    ARGON_SIMPLE_FUNCTION(reset);
    ARGON_SIMPLE_FUNCTION(current);
}




extern builtin_func_def table_seq_funcs[];



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
