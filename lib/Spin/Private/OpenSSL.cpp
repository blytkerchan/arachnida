/* This file contains support code for using the OpenSSL library in a threaded environment - and to simply use it at all. */
#define OPENSSL_THREAD_DEFINES
extern "C" {
#include <openssl/ssl.h>
#include <openssl/crypto.h>
}
#if !defined(OPENSSL_THREADS)
#error "OpenSSL was not compiled with thread support - please recompile it"
#endif
#include <cassert>
#include <vector>
#include <loki/ScopeGuard.h>
#if defined(_WIN32) && ! defined(__CYGWIN__)
#include <Windows.h>
#else
extern "C" {
#include <pthread.h>
}
#define USING_PTHREADS
#endif
#include "TLS.h"
#include "atomicPrimitives.h"

namespace
{
	struct Lock
	{
		Lock()
		{
			init_();
		}

		~Lock()
		{
			fini_();
		}

#if defined(USING_PTHREADS)
		void lock()
		{
			pthread_mutex_lock(&lock_);
		}

		void unlock()
		{
			pthread_mutex_unlock(&lock_);
		}
#else
		void lock()
		{
			EnterCriticalSection(&lock_);
		}

		void unlock()
		{
			LeaveCriticalSection(&lock_);
		}
#endif

	private :
		// Neither CopyConstructible nor Assignable
		Lock(const Lock & lock);
		Lock & operator=(Lock lock);

#if defined(USING_PTHREADS)
		void init_()
		{
			pthread_mutex_init(&lock_, 0);
		}

		void fini_()
		{
			pthread_mutex_destroy(&lock_);
		}

		::pthread_mutex_t lock_;
#else
		void init_()
		{
			InitializeCriticalSection(&lock_);
		}

		void fini_()
		{
			DeleteCriticalSection(&lock_);
		}

		CRITICAL_SECTION lock_;
#endif
	};

	static struct OpenSSLInitializer
	{
		typedef std::vector< Lock * > Locks_;
		OpenSSLInitializer()
			: locks_(::CRYPTO_num_locks()),
			  tls_key_(Spin::Private::TLS::getInstance().acquireKey(free_)),
			  next_thread_id_(0)
		{
			assert(instance__ == 0);
			instance__ = this;
			Loki::ScopeGuard cleanup_guard = Loki::MakeGuard(&cleanup);
			for (Locks_::iterator curr(locks_.begin()); curr != locks_.end(); ++curr)
			{
				*curr = new Lock;
			}
			cleanup_guard.Dismiss();
			::CRYPTO_set_locking_callback(lock);
			::SSL_library_init(); // always returns 1 so it is safe to discard this value - according to the doc
		}

		~OpenSSLInitializer()
		{
			cleanup();
			Spin::Private::TLS::getInstance().releaseKey(tls_key_);
		}

		static void lock(int mode, int lock_id, const char * file, int line)
		{
			assert(lock_id >= 0);
			assert(static_cast< std::size_t >(lock_id) < instance__->locks_.size());
			if (mode & CRYPTO_LOCK)
			{
				instance__->locks_[lock_id]->lock();
			}
			else
			{
				instance__->locks_[lock_id]->unlock();
			}
		}

		static void cleanup()
		{
			if (instance__)
			{
				for (Locks_::iterator curr(instance__->locks_.begin()); curr != instance__->locks_.end(); ++curr)
				{
					delete *curr;
					*curr = 0;
				}
			}
			else
			{ /* no instance - nothing to clean up */ }
		}

		static void free_(void * p)
		{
			delete (unsigned long*)p;
		}

		static unsigned long getThreadId()
		{
			unsigned long retval;
			Spin::Private::TLS & tls(Spin::Private::TLS::getInstance());
			void * val(tls.getValue(instance__->tls_key_));
			if (!val)
			{
				retval = Spin::Private::fetchAndIncrement(instance__->next_thread_id_);
				tls.setValue(instance__->tls_key_, new unsigned long(retval));
			}
			else
				retval = *((unsigned long *)(val));
			return retval;
		}

		Locks_ locks_;
		Spin::Private::TLS::Key tls_key_;
		unsigned long next_thread_id_;

		static OpenSSLInitializer * instance__;
	} openssl_initializer__;
	OpenSSLInitializer * OpenSSLInitializer::instance__(0);
}
