#ifndef _spin_connector_h
#define _spin_connector_h

#include "Details/prologue.h"
#include <string>
#include <boost/cstdint.hpp>
#include "Connection.h"

typedef struct bio_st BIO;
namespace Spin
{
	class SPIN_API Connector
	{
	public :
		static Connector & getInstance();

		Connection connect(const std::string & remote_address, boost::uint16_t port, bool use_ssl = false);

	private :
		// Neither CopyConstructible nor Assignable
		Connector(const Connector&);
		Connector & operator=(const Connector&);

		Connector();
		~Connector();

		::BIO * connectSSL_(const std::string & target);
		::BIO * connect_(const std::string & target);
	};
}

#endif
