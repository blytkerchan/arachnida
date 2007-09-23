/* This file contains support code for using the OpenSSL library in a threaded environment - and to simply use it at all. */
#define OPENSSL_THREAD_DEFINES
extern "C" {
#include <openssl/crypto.h>
}
#if !defined(OPENSSL_THREADS)
#error "OpenSSL was not compiled with thread support - please recompile it"
#endif
#include <cassert>
#include <vector>
#include <boost/threads/mutex.hpp>

namespace
{
	static struct OpenSSLInitializer
	{
		typedef std::vector< boost::recursive_mutex * > Locks_;
		OpenSSLInitializer()
			: locks_(::CRYPTO_num_locks())
		{
			assert(instance__ == 0);
			instance__ = this;
			Loki::ScopeGuard cleanup_guard = Loki::MakeGuard(&cleanup);
			for (Locks_::iterator curr(locks_.begin()); curr != locks_.end(); ++curr)
			{
				*curr = new boost::recursive_mutex;
			}
			cleanup_guard.Dismiss();
			CRYPTO_set_locking_callback(lock);
		}

		~OpenSSLInitializer()
		{
			cleanup();
		}

		static void lock(int mode, int lock_id, const char * file, int line)
		{
			assert(lock_id < locks_.size());
			if (mode & CRYPTO_LOCK)
			{
				locks_[lock_id].lock();
			}
			else
			{
				locks_[lock_id].unlock()
			}
		}

		static void cleanup()
		{
			if (instance__)
			{
				for (Locks_::iterator curr(locks_.begin()); curr != locks_.end(); ++curr)
				{
					delete *curr;
					*curr = 0;
				}
			}
			else
			{ /* no instance - nothing to clean up */ }
		}

		Locks_ locks_;

		static OpenSSLInitializer * instance__;
	} openssl_initializer__;
	OpenSSLInitializer * OpenSSLInitializer::instance__(0);
}
