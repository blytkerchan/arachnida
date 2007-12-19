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

namespace Scorpion { class BIO; }
namespace Spin
{
	class Connector;
	class Listener;
	namespace Handlers
	{
		class NewDataHandler;
	}
	class SPIN_API Connection
	{
	public :
		typedef boost::function< void() > OnErrorCallback;

		enum Status
		{
			good__			= 0,
			error__			= 1,
			done__			= 2
		};
		enum Reason
		{
			no_error__		= 0,			// no errors, all is well
			should_retry__	= 1,			// you should retry this operation later
			should_read__	= 2,			// you should read before writing
			should_write__	= 4,			// you should write before reading
		};

		Connection(const Connection & connection);
		~Connection();

		/* write will return the number of bytes written and the reason for 
		 * failure if any. If the failure is a logic error or another, 
		 * permanent error, an exception will be thrown. */
		std::pair< std::size_t, int > write(const std::vector< char > & data);
		std::pair< std::size_t, int > write(const std::string & data) { return write(std::vector< char >(data.begin(), data.end())); }

		std::pair< std::size_t, int > read(std::vector< char > & buffer);

		bool usesSSL() const;

		static unsigned long Connection::allocateAttribute();
		boost::any & Connection::getAttribute(unsigned long index);

		void setNewDataHandler(Handlers::NewDataHandler & handler, OnErrorCallback on_error_callback = OnErrorCallback());
		void clearNewDataHandler();

		Details::Address getPeerAddress() const;

		int getStatus() const
		{
			return status_;
		}

	private :
		enum { default_read_block_size__ = 4096, max_attribute_count__ = 8 };
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
