

#include "trx.hh"

#include <iterator>

#include <boost/regex.hpp>

ARGON_NAMESPACE_BEGIN

namespace trx
{


#define ARGON_FUNCTION_DEF(name) Value func_##name::run(void)


    ARGON_FUNCTION_DEF(start)
    {
        ARGON_ICERR_CTX(m_args.size() == 1, *this, "invalid args count");
        
        Ref dbc_ref = m_args[0];

        ARGON_ICERR_CTX(dbc_ref.is<Connection>(), *this,
                        "trx.start() arg #1 must be of type connection");

        Connection *dbc = dbc_ref.cast<Connection>();

        dbc->getDbc().beginTrans(informave::db::trx_read_committed);

        return Value();
    }


    ARGON_FUNCTION_DEF(commit)
    {
        ARGON_ICERR_CTX(m_args.size() == 1, *this, "invalid args count");
        
        Ref dbc_ref = m_args[0];

        ARGON_ICERR_CTX(dbc_ref.is<Connection>(), *this,
                        "trx.commit() arg #1 must be of type connection");

        Connection *dbc = dbc_ref.cast<Connection>();

        dbc->getDbc().commit();

        return Value();
    }


    ARGON_FUNCTION_DEF(rollback)
    {
        ARGON_ICERR_CTX(m_args.size() == 1 || m_args.size() == 2, *this, "invalid args count");
        
        Ref dbc_ref = m_args[0];
        String savepoint;

        if(m_args.size() == 2)
            savepoint = m_args[1]->_value().str();


        ARGON_ICERR_CTX(savepoint.length() > 0, *this, "trx.rollback() arg #2 requires a savepoint name");
        ARGON_ICERR_CTX(dbc_ref.is<Connection>(), *this,
                        "trx.rollback() arg #1 must be of type connection");

        Connection *dbc = dbc_ref.cast<Connection>();

        if(savepoint.empty())
            dbc->getDbc().rollback();
        else
            dbc->getDbc().rollback(savepoint);

        return Value();
    }


    ARGON_FUNCTION_DEF(savepoint)
    {
        ARGON_ICERR_CTX(m_args.size() == 2, *this, "invalid args count");
        
        Ref dbc_ref = m_args[0];
        String savepoint = m_args[1]->_value().str();

        ARGON_ICERR_CTX(savepoint.length() > 0, *this, "trx.savepoint() arg #2 requires a savepoint name");
        ARGON_ICERR_CTX(dbc_ref.is<Connection>(), *this,
                        "trx.savepoint() arg #1 must be of type connection");

        Connection *dbc = dbc_ref.cast<Connection>();

        dbc->getDbc().savepoint(savepoint);

        return Value();
    }



}


template<typename T>
static Function* factory_function(Processor &proc, const ArgumentList &args)
{
    return new T(proc, args);
}



builtin_func_def table_trx_funcs[] =
{
    { "trx.start",  factory_function<trx::func_start>, 1, 1 },
    { "trx.commit",  factory_function<trx::func_commit>, 1, 1 },
    { "trx.rollback",  factory_function<trx::func_rollback>, 1, 2 },
    { "trx.savepoint",  factory_function<trx::func_savepoint>, 2, 2 },
    { NULL, NULL, 0, 0 }
};


//bbb = { 2 };

ARGON_NAMESPACE_END

