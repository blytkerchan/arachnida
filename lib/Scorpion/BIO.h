#ifndef _scorpion_bio_h
#define _scorpion_bio_h

#include "Details/prologue.h"
#include <string>
#include <boost/shared_ptr.hpp>

typedef struct bio_st BIO;
namespace Scorpion
{
	class Context;

	class SCORPION_API BIO
	{
	public :
		enum {
			non_blocking__	= 1,
			auto_retry__	= 2		// ignored by createAcceptBIO
		};

		virtual ~BIO();

		virtual BIO * accept() const { return 0; }
		virtual int write(const void * buffer, std::size_t buffer_size) { return -2; }
		virtual int read(void * buffer, std::size_t buffer_size) { return -2; }

		virtual bool shouldRetry() const { return false; }
		virtual bool shouldRead() const { return false; }
		virtual bool shouldWrite() const { return false; }
		bool usesSSL() const;

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

	// create a BIO than can accept connections and hoes not have as SSL support
	SCORPION_API BIO * createAcceptBIO(const std::string & local_address, int flags);
	// create a BIO than can accept connections and has SSL support
	SCORPION_API BIO * createSSLAcceptBIO(const std::string & local_address, Context & context, int flags);
	// connect using SSL
	SCORPION_API BIO * connect(Context & context, const std::string & peer_address);
	// connect without using SSL
	SCORPION_API BIO * connect(const std::string & peer_address);
}

#endif
