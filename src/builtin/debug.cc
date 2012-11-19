

#include "debug.hh"

#include <iterator>

ARGON_NAMESPACE_BEGIN

namespace debug
{


#define ARGON_FUNCTION_DEF(name) Value func_##name::run(void)



    ARGON_FUNCTION_DEF(echo)
    {

        assert(m_args.size() == 1);
        std::cerr << "[debug():] " << "{"
                  << (*m_args.begin())->_value().data().datatype() << "} "
                  << (*m_args.begin())->_value().str() << std::endl;
        return (*m_args.begin())->_value();
    }


    ARGON_FUNCTION_DEF(symbol_exists)
    {
        /// @bug this function is buggy. We search for the ID, but
        /// with this->getSymbols() we dont get the table of the caller,
        /// we get just our own internal table which is empty!
        assert(m_args.size() == 1);

        Identifier id = (*m_args.begin())->_value().data().get<String>();
        
        try
        {
            Element* elem = this->getSymbols().find<Element>(id);
        }
        catch(std::runtime_error&) /// @bug custom exception
        {
            return Value(bool(false));
        }
        return Value(bool(true));
    }


}

template<typename T>
static Function* factory_function(Processor &proc, const ArgumentList &args)
{
    return new T(proc, args);
}




builtin_func_def table_debug_funcs[] =
{
    { "debug.echo",  factory_function<debug::func_echo>, 1, 1 },
    { "debug.symbol_exists",  factory_function<debug::func_symbol_exists>, 1, 1 },
    { NULL, NULL, 0, 0 }
};



//bbb = { 2 };

ARGON_NAMESPACE_END

