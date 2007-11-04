#ifndef _spin_connection_h
#define _spin_connection_h

#include "Details/prologue.h"
#include <string>
#include <vector>
#include <boost/cstdint.hpp>
#include "Details/Address.h"

typedef struct bio_st BIO;
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

		void setNewDataHandler(Handlers::NewDataHandler & handler);
		void clearNewDataHandler();

		Details::Address getPeerAddress() const;

	private :
		enum { default_read_block_size__ = 4096 };
		// Not Assignable
		Connection & operator=(const Connection&);

		Connection(::BIO * bio);
		void onDataReady_();

		mutable ::BIO * bio_;
		Handlers::NewDataHandler * data_handler_;

		friend class Connector; // for construction
		friend class Listener; // for construction
	};
}

#endif
