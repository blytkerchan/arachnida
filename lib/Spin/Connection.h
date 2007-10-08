#ifndef _spin_connection_h
#define _spin_connection_h

#include "Details/prologue.h"
#include <string>
#include <vector>
#include <boost/cstdint.hpp>

typedef struct bio_st BIO;
namespace Spin
{
	class Connector;
	class SPIN_API Connection
	{
	public :
		Connection(const Connection & connection);
		~Connection();

		void write(const std::vector< char > & data);
		void read(std::vector< char > & buffer);

		std::string getLocalAddress() const;
		boost::uint16_t getLocalPort() const;

		std::string getRemoteAddress() const;
		boost::uint16_t getRemotePort() const;

		bool usesSSL() const;

	private :
		// Not Assignable
		Connection & operator=(const Connection&);

		Connection(::BIO * bio);

		mutable ::BIO * bio_;

		friend class Connector; // for construction
	};
}

#endif
