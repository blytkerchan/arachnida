#ifndef _spin_connector_h
#define _spin_connector_h

#include "Details/prologue.h"
#include <string>
#include <boost/cstdint.hpp>
#include "Connection.h"

namespace Scorpion { class Context; class BIO; }
namespace Spin
{
	class SPIN_API Connector
	{
	public :
		static Connector & getInstance();

		Connection connect(const std::string & remote_address, boost::uint16_t port);
		Connection connect(Scorpion::Context & context, const std::string & remote_address, boost::uint16_t port);

	private :
		// Neither CopyConstructible nor Assignable
		Connector(const Connector&);
		Connector & operator=(const Connector&);

		Connector();
		~Connector();

		Scorpion::BIO * connectSSL_(Scorpion::Context & context, const std::string & target);
		Scorpion::BIO * connect_(const std::string & target);
	};
}

#endif
