#ifndef _spin_connection_h
#define _spin_connection_h

#include "Details/prologue.h"
#include <string>
#include <vector>
#include <boost/any.hpp>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
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
	class SPIN_API Connection
	{
	public :
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

		//! Copy constructor with move semantics
		Connection(const Connection & connection);
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

		//! Return true if the connection uses SSL (and is therefore secured), false if not
		bool usesSSL() const;

		/** Allocate an attribute identifier.
		 * Much like I/O streams, you can add attributes to connections by allocating an 
		 * identifier (globally, statically) and subsequently using that identifier 
		 * with the getAttribute method. Once you've allocated an identifier on one
		 * Connection object, it is valid for all Connection objects (so you do not need
		 * to allocate a new one each time you want to use an attribute for the same purpose
		 * on a different connection).
		 *
		 * Internally, the attributes are also used by the library (so don't simply pick an
		 * integer value at random and use it!) */
		static unsigned long Connection::allocateAttribute();
		/** Get an attribute with a previously allocated identifier, obtained from allocateAttribute.
		 * The attribute returned will be a newly allocated boost::any by default */
		boost::any & Connection::getAttribute(unsigned long index);

		/** Set a new data handler, which will be called whenever data is ready on the connection.
		 * This allows for asynchronous handling of new data. Behind the scenes, a separate thread
		 * will use select(2) on the connection to wait for data. */
		void setNewDataHandler(Handlers::NewDataHandler & handler, OnErrorCallback on_error_callback = OnErrorCallback());
		//! Clear the new-data handler, returning to a synchronous mode of operation
		void clearNewDataHandler();

		//! Get the peer's address
		Details::Address getPeerAddress() const;

		//! Get the status of this connection
		int getStatus() const { return status_; }

	private :
		enum { max_attribute_count__ = 8 };
		// Not Assignable
		Connection & operator=(const Connection&);

		Connection(Scorpion::BIO * bio);
		void onDataReady_();

		mutable boost::shared_ptr< Scorpion::BIO > bio_;
		Handlers::NewDataHandler * data_handler_;
		std::vector< boost::any > attributes_;
		mutable int status_;

		static unsigned long next_attribute_index__;

		friend class Connector; // for construction
		friend class Listener; // for construction
	};
}

#endif
