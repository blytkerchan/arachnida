#ifndef _spin_listener_h
#define _spin_listener_h

#include <boost/cstdint.hpp>
#include "Connection.h"

namespace Spin
{
	class Listener
	{
	public :
		Listener(boost::uint32_t local_address, boost::uint16_t local_port, bool use_ssl = false);
		~Listener();

		Connection accept();

	private :
		// Neither CopyConstructible nor Assignable
		Listener(const Listener&);
		Listener & operator=(const Listener&);
	};
}

#endif
