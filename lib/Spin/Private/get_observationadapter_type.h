#ifndef _spin_private_get_observationadapter_type_h
#define _spin_private_get_observationadapter_type_h

#include <boost/mpl/identity.hpp>

namespace Spin
{
	namespace Private
	{
		template < typename Observer >
		struct get_observationadapter_type : boost::mpl::identity< Observer >
		{};
	}
}

#endif
