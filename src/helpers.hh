

#ifndef CYZ_FOO
#define CYZ_FOO

#include "argon/dtsengine.hh"

ARGON_NAMESPACE_BEGIN

template<typename OnType>
inline safe_ptr<Element> __symbol_create_on(OnType &on, const Identifier id, Element *newsymbol)
{
        Element *elem = on.getSymbols().addPtr(newsymbol);
	        on.getSymbols().add(id, elem);
		        return elem;
			}



#define SYMBOL_CREATE_ON(on, id, newelem) __symbol_create_on(on, id, newelem)


ARGON_NAMESPACE_END


#endif

