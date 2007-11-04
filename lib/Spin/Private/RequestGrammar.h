#ifndef _spin_private_requestgrammar_h
#define _spin_private_requestgrammar_h

#include "Grammar.h"

namespace Spin
{
	namespace Private
	{
		struct RequestGrammar : Grammar< RequestGrammar >
		{
			template < typename Scanner >
			struct definition : public Grammar< RequestGrammar >::definition< Scanner >
			{
				definition(const RequestGrammar & self)
					: Grammar< RequestGrammar >::definition< Scanner >(self)
				{ /* no-op */ }

				const boost::spirit::rule< Scanner > & start() const
				{
					return Request_;
				}
			};
		};
	}
}

#endif
