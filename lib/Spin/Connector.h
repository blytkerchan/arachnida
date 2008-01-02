#ifndef _spin_connector_h
#define _spin_connector_h

#include "Details/prologue.h"
#include <string>
#include <boost/cstdint.hpp>
#include "Connection.h"

#ifndef DOXYGEN_GENERATING
namespace Scorpion { class Context; class BIO; }
#endif
namespace Spin
{
	/** A simple connection factory.
	 * This class creates connections that can either be secured, or not. Whether they are secured
	 * depends on whether a context is passed to the connect method. If one is, the connection is
	 * only as secure as the set-up of the context in question allows(!). */
	class SPIN_API Connector
	{
	public :
		//! Get the one instance of this (stateless) class
		static Connector & getInstance();

		//! Create a connection, connecting to the given remote address and port
		Connection connect(const std::string & remote_address, boost::uint16_t port);
		//! Create a secured connection, connecting to the given remote address and port
		Connection connect(const Scorpion::Context & context, const std::string & remote_address, boost::uint16_t port);

	private :
		// Neither CopyConstructible nor Assignable
		Connector(const Connector&);
		Connector & operator=(const Connector&);

		Connector();
		~Connector();

		Scorpion::BIO * connectSSL_(const Scorpion::Context & context, const std::string & target);
		Scorpion::BIO * connect_(const std::string & target);
	};
}

#endif
