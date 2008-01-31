#ifndef _spin_udpsocket_h
#define _spin_udpsocket_h

#include "Details/prologue.h"
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <Acari/Attributes.h>
#include "Details/Address.h"

namespace Spin
{
	namespace Details
	{
		struct Address;
	}
	namespace Handlers
	{
		class UDPDataHandler;
	}
	class SPIN_API UDPSocket : public boost::enable_shared_from_this< UDPSocket >, private Acari::Attributes
	{ 
	public :
		//! Type of the call-back called when errors occur
		typedef boost::function< void() > OnErrorCallback;
		//! Possible status flags
		enum Status
		{
			//! All is well - no errors occur
			good__			= 0,
			//! An error occurred during communication
			error__			= 1,
			//! Communications are done on this connection - the connection is closed
			done__			= 2
		};

		using Acari::Attributes::getAttribute;
		using Acari::Attributes::allocateAttribute;

		UDPSocket(const Details::Address & address = Details::Address(0), unsigned short port = 0);
		~UDPSocket();

		std::size_t send(const Details::Address & to, unsigned short port, const std::vector< char > & data);
		std::size_t send(const Details::Address & to, unsigned short port, const std::string & data) { return send(to, port, std::vector< char >(data.begin(), data.end())); }
		std::size_t send(const Details::Address & to, unsigned short port, const char * data) { return send(to, port, std::string(data)); }
		template < typename T >
		std::size_t send(const Details::Address & to, unsigned short port, const T & data) { return send(to, port, serialize(data)); }

		boost::tuple< Details::Address, unsigned short, std::size_t > recv(std::vector< char > & buffer, unsigned long timeout = ~0);

		bool poll() const;
		void close();

		/** Set a new data handler, which will be called whenever data is ready on the connection.
		 * This allows for asynchronous handling of new data. Behind the scenes, a separate thread
		 * will use select(2) on the connection to wait for data. */
		void setDataHandler(Handlers::UDPDataHandler & handler, OnErrorCallback on_error_callback = OnErrorCallback());
		//! Clear the new-data handler, returning to a synchronous mode of operation
		void clearDataHandler();

		//! Get the status of this connection
		int getStatus() const { return status_; }

	private :
		UDPSocket(const UDPSocket&);
		UDPSocket & operator=(const UDPSocket&);

		void checkStatus(const char * for_whom) const;
		void onDataReady_();

		mutable int status_;
		mutable boost::recursive_mutex fd_lock_;
		int fd_;
		Handlers::UDPDataHandler * data_handler_;
	};
}

#endif
