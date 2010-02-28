/* This file contains support code for using the OpenSSL library in a threaded environment - and to simply use it at all. */
#define OPENSSL_THREAD_DEFINES
extern "C" {
#include <openssl/ssl.h>
#include <openssl/crypto.h>
#include <openssl/rand.h>
}
#if !defined(OPENSSL_THREADS)
#error "OpenSSL was not compiled with thread support - please recompile it"
#endif
#include <cassert>
#include <iostream>
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
#include <Acari/ThreadLocalStorage.h>
#include <Acari/atomicPrimitives.h>

#ifndef SCORPION_USE_DEV_RANDOM
#define SCORPION_USE_DEV_RANDOM 0
#endif
#ifndef SCORPION_DEFAULT_PRNG_SEED_SIZE
#define SCORPION_DEFAULT_PRNG_SEED_SIZE 255
#endif
#ifndef SCORPION_DEFAULT_EGD_DEVNAMES
#define SCORPION_DEFAULT_EGD_DEVNAMES "/var/run/egd-pool", "/dev/egd-pool", "/etc/egd-pool", "/etc/entropy"
#endif

#include <boost/format.hpp>


namespace Scorpion
{
	namespace Private
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
				  tls_key_(Acari::ThreadLocalStorage::getInstance().acquireKey(free_)),
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
				::SSL_library_init(); // always returns 1 so it is safe to discard this value - according to the doc
				::CRYPTO_set_locking_callback(lock);
				::ERR_load_CRYPTO_strings();
				::ERR_load_SSL_strings();
				::SSL_load_error_strings();
				::OpenSSL_add_all_algorithms();
				seedPRNG();
			}
	
			~OpenSSLInitializer()
			{
				cleanup();
				Acari::ThreadLocalStorage::getInstance().releaseKey(tls_key_);
			}
	
			static void lock(int mode, int lock_id, const char * file, int line)
			{
				(void)file;
				(void)line;
				assert(lock_id >= 0);
				assert(static_cast< std::size_t >(lock_id) < instance__->locks_.size());
				if (mode & CRYPTO_LOCK)
				{
					instance__->locks_[lock_id]->lock();
				}
				else if (mode & CRYPTO_UNLOCK)
				{
					instance__->locks_[lock_id]->unlock();
				}
				else
				{ /* don't know why we were called */ }
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
	
			static boost::uint32_t getThreadId()
			{
				boost::uint32_t retval(0);
				Acari::ThreadLocalStorage & tls(Acari::ThreadLocalStorage::getInstance());
				void * val(tls.getValue(instance__->tls_key_));
				if (!val)
				{
					retval = Acari::fetchAndIncrement(instance__->next_thread_id_);
					tls.setValue(instance__->tls_key_, new boost::uint32_t(retval));
				}
				else
					retval = *((boost::uint32_t *)(val));
				return retval;
			}

			static void seedPRNG()
			{
				if (SCORPION_USE_DEV_RANDOM && ::RAND_load_file("/dev/random", SCORPION_DEFAULT_PRNG_SEED_SIZE) == SCORPION_DEFAULT_PRNG_SEED_SIZE) return;
				if (::RAND_load_file("/dev/urandom", SCORPION_DEFAULT_PRNG_SEED_SIZE) == SCORPION_DEFAULT_PRNG_SEED_SIZE) return;
				// try using EGD
				const char * names[] = { SCORPION_DEFAULT_EGD_DEVNAMES, 0 };
				for (int i(0); names[i]; ++i)
				{
					if (::RAND_egd_bytes(names[i], SCORPION_DEFAULT_PRNG_SEED_SIZE) == SCORPION_DEFAULT_PRNG_SEED_SIZE) return;
				}
#if defined(_WIN32) && ! defined(__CYGWIN__)
				::RAND_screen();
#endif
			}
	
			Locks_ locks_;
			Acari::ThreadLocalStorage::Key tls_key_;
			volatile boost::uint32_t next_thread_id_;
	
			static OpenSSLInitializer * instance__;
		} openssl_initializer__;
		OpenSSLInitializer * OpenSSLInitializer::instance__(0);
	}
}

