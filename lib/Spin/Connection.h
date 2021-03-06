#ifndef _spin_connection_h
#define _spin_connection_h

#include "Details/prologue.h"
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <Acari/Attributes.h>
#include "Details/Address.h"

#ifndef DOXYGEN_GENERATING
namespace Scorpion { class BIO; }
#endif
namespace Spin
{
	class Connector;
	class Listener;
	namespace Handlers
	{
		class NewDataHandler;
	}
	/** A wrapper around a connection - secured or non-secured.
	 * The Connection class wraps either a secured or a non-secured connection,
	 * depending on whether it was created by a Connector/Listener using a 
	 * Scorpion Context, or not. Whether or not the connection uses SSL is 
	 * completely transparent, though visible through the usesSSL method.
	 *
	 * The Connection object may, in some cases, be completely invisible to the
	 * users of this library - i.e. if data handlers are used with connection 
	 * handlers on the listener. 
	 *
	 * A call-back can be installed on the connection to be notified when an error
	 * occurs. This is mainly for garbage collection purposes (i.e. this call-back
	 * is called when the connection is used when it is no longer in a usable state).
	 *
	 * In some cases, e.g. when the peer closes the connection, the Connection object
	 * may no longer be usable. The getStatus method returns a flag that may indicate
	 * this. */
	class SPIN_API Connection : public boost::enable_shared_from_this< Connection >, private Acari::Attributes
	{
	public :
		using Acari::Attributes::allocateAttribute;
		using Acari::Attributes::getAttribute;

		//! Type of the call-back called when errors occur
		typedef boost::function< void() > OnErrorCallback;
		enum {
			//! Size of the blocks read when the read method is passed an empty buffer
			default_read_block_size__ = 4096 };

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
		//! Reasons for failure or incomplete reads or writes
		enum Reason
		{
			//! no errors, all is well
			no_error__		= 0,
			//! you should retry this operation later
			should_retry__	= 1,
			//! you should read before writing
			should_read__	= 2,
			//! you should write before reading
			should_write__	= 4,
		};

		~Connection();

		/** Write data to the connection.
		 * The write method will return the number of bytes written and
		 * the reason for failure if any. If the failure is a logic error
		 * or another, permanent error, an exception will be thrown. */
		std::pair< std::size_t, int > write(const std::vector< char > & data);
		std::pair< std::size_t, int > write(const std::string & data) { return write(std::vector< char >(data.begin(), data.end())); }
		std::pair< std::size_t, int > write(const char * data) { return write(std::string(data)); }
		template < typename T >
		std::pair< std::size_t, int > write(const T & data) { return write(serialize(data)); }

		/** Read data from the connection.
		 * The read method will return the number of bytes read and
		 * the reason for failure if any. If the failure is a logic error
		 * or another, permanent error, an exception will be thrown.
		 * 
		 * If the buffer size is zero when passed to this method, the buffer
		 * is filled in blocks of default_read_block_size__ bytes in size. */
		std::pair< std::size_t, int > read(std::vector< char > & buffer);

		/** See if there is any data waiting on the connection.
		 * \warning Results of the poll are not guaranteed to be accurate in 
		 *          that it may result in false positives. This is due to 
		 *          the fact that in some cases, not all data is for client 
		 *          code consumption. */
		bool poll() const;

		/** Close the connection.
		 * Any further communication on this socket will fail.
		 * 
		 * If a data handler is attached to the connection, it may receive a 
		 * spurious notification if data was waiting on the connection at the
		 * time it was being closed, but it will not be able to retrieve that 
		 * data.
		 * 
		 * If an error callback was registered with the connection, it may be
		 * notified, but only if there is also a data handler.
		 *
		 * Basically, this means you really should disconnect data handlers 
		 * before closing the connection. */
		void close();

		//! Return true if the connection uses SSL (and is therefore secured), false if not
		bool usesSSL() const;

		/** Set a new data handler, which will be called whenever data is ready on the connection.
		 * This allows for asynchronous handling of new data. Behind the scenes, a separate thread
		 * will use select(2) on the connection to wait for data. */
		void setNewDataHandler(Handlers::NewDataHandler & handler, OnErrorCallback on_error_callback = OnErrorCallback());
		//! Clear the new-data handler, returning to a synchronous mode of operation
		void clearNewDataHandler();
		//! Set an error handler
		void setErrorHandler(const OnErrorCallback & on_error_callback);
		//! Clear the error handler - connection and socket errors will no longer be reported asynchronously
		void clearErrorHandler();

		//! Get the peer's address
		Details::Address getPeerAddress() const;

		//! Set the number of seconds the implementation should wait for a read
		void setReadTimeout(unsigned int seconds);

		bool usable() const { return bio_; }

	private :
		// Neither CopyConstructible nor Assignable
		Connection(const Connection &);
		Connection & operator=(const Connection&);

		Connection(Scorpion::BIO * bio);
		void onDataReady_();
		void onError_();

		mutable boost::shared_ptr< Scorpion::BIO > bio_;
		mutable boost::recursive_mutex bio_lock_;
		Handlers::NewDataHandler * data_handler_;
		int fd_;
		OnErrorCallback error_handler_;
		mutable boost::recursive_mutex error_handler_lock_;

		friend class Connector; // for construction
		friend class Listener; // for construction
	};
}

#endif
