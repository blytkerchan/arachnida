#ifndef _scorpion_bio_h
#define _scorpion_bio_h

#include "Details/prologue.h"
#include <string>
#include <boost/shared_ptr.hpp>

typedef struct bio_st BIO;
namespace Scorpion
{
	class Context;

	/** A verrrry thin wrapper around an SSL Basic I/O object.
	 * Behind the scenes, a few types are derived from this one, which 
	 * implement the basic I/O functions for sockets and somesuch. According
	 * to the way the BIO is set up (also behind the scenes( it may or may not 
	 * use SSL, etc. */
	class SCORPION_API BIO
	{
	public :
		enum {
			not_implemented__ = -2,	///< Feature not implemented - may be returned by read or write
			non_blocking__	= 1,	///< non-blocking I/O on the BIO only
			auto_retry__	= 2		///< auto-retry when interrupted by a handshake ignored by createAcceptBIO
		};

		virtual ~BIO();

		//! Accept a connection
		virtual BIO * accept() const { return 0; }
		//! Write data to a socket, handle, or what not
		virtual int write(const void * buffer, std::size_t buffer_size) { return not_implemented__; }
		//! Read data from a socket, handle, or what not
		virtual int read(void * buffer, std::size_t buffer_size) { return not_implemented__; }

		virtual bool poll() const { return false; }

		//! Returns true if the previous operation failed because it should be retried
		virtual bool shouldRetry() const { return false; }
		//! Returns true if the previous operation failed because a read should be performed first
		virtual bool shouldRead() const { return false; }
		//! Returns true if the previous operation failed because a write should be performed first
		virtual bool shouldWrite() const { return false; }
		//! Returns true if this BIO uses SSL for security
		bool usesSSL() const;

		//! Get the file descriptor of this BIO - may be useful for select(2) calls
		int getFD() const;

	protected :
		BIO();
		void setBIO(::BIO * openssl_bio);
		::BIO * getBIO() const { return openssl_bio_.get(); }

	private :
		static void freeOpenSSLBIO(::BIO * bio);

		/* Internally, several different classes are derived from this base class, 
		 * each of which have only one thing in common: the fact that they contain 
		 * the OpenSSL BIO.
		 * All BIOS share a similar interface and similar capabilities, but not all 
		 * BIOs implement all capabilities nor implement those that they do implement 
		 * in the same way. */
		boost::shared_ptr< ::BIO > openssl_bio_;
	};

	//! create a BIO than can accept connections and hoes not have as SSL support
	SCORPION_API BIO * createAcceptBIO(const std::string & local_address, int flags);
	//! create a BIO than can accept connections and has SSL support
	SCORPION_API BIO * createSSLAcceptBIO(const std::string & local_address, Context & context, int flags);
	//! connect using SSL
	SCORPION_API BIO * connect(const Context & context, const std::string & peer_address);
	//! connect without using SSL
	SCORPION_API BIO * connect(const std::string & peer_address);
}

#endif
