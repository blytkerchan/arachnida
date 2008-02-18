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
	/** A UDP socket.
	 * Though HTTP doesn't support UDP as a transport layer, some HTTP servers 
	 * do like to listen on port 80 UDP as well. Also, a UDP socket was the only
	 * thing still missing to handle most other common protocols and allow
	 * Arachnida to be used as a more general-purpose framework for building 
	 * embeddable server software. Hence, this class was added in version 1.3 
	 * of Arachnida. */
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

		/** Construct from an address and a port to listen on.
		 * If either is not provided, you will not be able to receive data
		 * on this socket unless you send data and the receiving end of the 
		 * communication replies to wherever the data came from. (I.e. you
		 * will not be able to serve as a server) */
		UDPSocket(const Details::Address & address = Details::Address(0), unsigned short port = 0);
		~UDPSocket();

		//! Send a given bunch of data to a given address and port
		std::size_t send(const Details::Address & to, unsigned short port, const std::vector< char > & data);
		//! Send a given bunch of data to a given address and port
		std::size_t send(const Details::Address & to, unsigned short port, const std::string & data) { return send(to, port, std::vector< char >(data.begin(), data.end())); }
		//! Send a given bunch of data to a given address and port
		std::size_t send(const Details::Address & to, unsigned short port, const char * data) { return send(to, port, std::string(data)); }
		//! Send a given bunch of data to a given address and port
		template < typename T >
		std::size_t send(const Details::Address & to, unsigned short port, const T & data) { return send(to, port, serialize(data)); }

		/** Receive data into a given buffer, with a given time-out.
		 * Returns the data in the peek buffer and clears it, if any. Otherwise,
		 * it will try to read from the socket.
		 * \param buffer the buffer to receive data into. If empty, it will be resized to the amount of data received.
		 * \param timeout the amount of time, in milliseconds, to wait. Infinite by default
		 * \return This function returns a tuple containing the address and port 
		 *         the data was received from, and the amount of data received. 
		 *         If you've handed this function an empty vector as a buffer, its
		 *         size will be exactly the amount of data received when the function
		 *         returns*/
		boost::tuple< Details::Address, unsigned short, std::size_t > recv(std::vector< char > & buffer, unsigned long timeout = ~0);
		/** Receive data into a given buffer, with a given time-out.
		 * Unlike recv, this function will store the received data in an internal 
		 * buffer which you can clear with clearPeekBuffer, and will 
		 * return the same data as long as that buffer has not been cleared. 
		 * Recv will also return that same data but will automatically clear 
		 * the peek buffer.
		 * \param buffer the buffer to receive data into.
		 *        If empty, it will be resized to the amount of data received.
		 * \param timeout the amount of time, in milliseconds, to wait. Infinite by default
		 * \return This function returns a tuple containing the address and port 
		 *         the data was received from, and the amount of data received. 
		 *         If you've handed this function an empty vector as a buffer, its
		 *         size will be exactly the amount of data received when the function
		 *         returns*/
		boost::tuple< Details::Address, unsigned short, std::size_t > peek(std::vector< char > & buffer, unsigned long timeout = ~0);

		//! Clear the internal buffer with peeked data
		void clearPeekBuffer();

		//! Check whether there is any data to receive on the socket
		bool poll() const;
		//! Close the socket.
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
		boost::recursive_mutex peek_buffer_lock_;
		boost::tuple< Details::Address, unsigned short, std::size_t, std::vector< char > > peek_buffer_;
	};
}

#endif
