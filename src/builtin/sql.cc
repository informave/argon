

#include "sql.hh"

#include <iterator>

ARGON_NAMESPACE_BEGIN

namespace sql
{


#define ARGON_FUNCTION_DEF(name) Value func_##name::run(void)



    ARGON_FUNCTION_DEF(scalar)
    {
        ARGON_ICERR(m_args.size() >= 2, "invalid args count");
        
        Ref dbc_ref = m_args[0];
        String sql = m_args[1]->_value().str();
        Value val;

        ARGON_ICERR_CTX(sql.length() > 0, *this, "sql.scalar() arg #2 requires a SQL statement");
        ARGON_ICERR_CTX(dbc_ref.is<Connection>(), *this, "sql.scalar() arg #1 must be of type connection");

        Connection *dbc = dbc_ref.cast<Connection>();
        
        informave::db::IStmt::ptr stmt = dbc->getDbc().newStatement();

        stmt->prepare(sql);
        
        for(size_t i = 1; (i+2) <= m_args.size(); ++i)
        {
            stmt->bind(i, m_args[(i+2)-1]->_value().data());
        }
        stmt->execute();
        
        informave::db::IResult &res = stmt->resultset();
        res.first();
        if(res.eof() || res.columnCount() == 0)
        {}
        else
        {
            val = res.column(1);
        }
        stmt->close();
        return val;
    }


    ARGON_FUNCTION_DEF(list)
    {
        ARGON_ICERR(m_args.size() >= 3, "invalid args count");

        Ref dbc_ref = m_args[0];
	String sep = m_args[1]->_value().str();
        String sql = m_args[2]->_value().str();
        std::wstringstream ss;
	bool nullval = true;

        ARGON_ICERR_CTX(sql.length() > 0, *this, "sql.list() arg #3 requires a SQL statement");
        ARGON_ICERR_CTX(dbc_ref.is<Connection>(), *this, "sql.list() arg #1 must be of type connection");

        Connection *dbc = dbc_ref.cast<Connection>();

        informave::db::IStmt::ptr stmt = dbc->getDbc().newStatement();

        stmt->prepare(sql);

        for(size_t i = 1; (i+3) <= m_args.size(); ++i)
        {
            stmt->bind(i, m_args[(i+3)-1]->_value().data());
        }
        stmt->execute();

        informave::db::IResult &res = stmt->resultset();
        res.first();
        if(res.eof() || res.columnCount() == 0)
        {}
        else
        {
	    do
	    {
	    	if(res.column(1).isnull()) continue;
		if(!nullval) ss << sep;
		nullval = false;
		ss << res.column(1).asStr();
	    }
	    while(res.next());
        }
        stmt->close();
	if(nullval) return Value();
	else return String(ss.str());
    }



    ARGON_FUNCTION_DEF(exec)
    {
        ARGON_ICERR(m_args.size() >= 2, "invalid args count");
        
        Ref dbc_ref = m_args[0];
        String sql = m_args[1]->_value().str();

        ARGON_ICERR_CTX(sql.length() > 0, *this, "sql.exec() arg #2 requires a SQL statement");
        ARGON_ICERR_CTX(dbc_ref.is<Connection>(), *this, "sql.exec() arg #1 must be of type connection");

        Connection *dbc = dbc_ref.cast<Connection>();
        
        informave::db::IStmt::ptr stmt = dbc->getDbc().newStatement();

        stmt->prepare(sql);
        
        for(size_t i = 1; (i+2) <= m_args.size(); ++i)
        {
            stmt->bind(i, m_args[(i+2)-1]->_value().data());
        }
        stmt->execute();

        stmt->close();
        
        return Value();
    }



}

template<typename T>
static Function* factory_function(Processor &proc, const ArgumentList &args)
{
    return new T(proc, args);
}




builtin_func_def table_sql_funcs[] =
{
    { "sql.scalar", factory_function<sql::func_scalar>, 2, -1},
    { "sql.exec",   factory_function<sql::func_exec>,   2, -1},
    { "sql.list",   factory_function<sql::func_list>,   3, -1},
    { NULL, NULL, 0, 0 }
};



//bbb = { 2 };

ARGON_NAMESPACE_END

