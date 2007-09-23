#include "TLS.h"
#include <cassert>
#if defined(_WIN32) && !defined(__CYGWIN__)
#else
extern "C" {
#include <pthread.h>
#define USE_POSIX_TLS
#define KEY(key) (::pthread_key_t*)key->unused_
}
#endif

namespace Spin
{
	namespace Private
	{
		static TLS & TLS::getInstance()
		{
			static TLS instance__;
			return instance__;
		}

		TLS::Key TLS::acquireKey()
		{
#if defined(USE_POSIX_TLS)
			Key retval;
			KEY(retval) = new ::pthread_key_t;
			int rv(::pthread_key_create(KEY(retval), 0));
			if (rv != 0)
			{
				delete KEY(retval);
				throw std::bad_alloc();
			}
			else
			{ /* all is well */ }
			return retval;
#endif
		}

		void TLS::releaseKey(const TLS::Key & key)
		{
#if defined(USE_POSIX_TLS)
			int rv(::pthread_key_delete(KEY(key)));
			assert(rv == 0);
#endif
		}

		void TLS::setValue(const Key & key, void * value)
		{
#if defined(USE_POSIX_TLS)
			int rv(::pthread_set_specific(KEY(key), value));
			if (rv == EINVAL)
				throw std::logic_error("Invalid key");
			else if (rv != 0)
				throw std::bad_alloc();
			else
			{ /* all is well */ }
#endif
		}

		void * TLS::getValue(const Key & key) const
		{
#if defined(USE_POSIX_TLS)
			return ::pthread_get_specific(KEY(key));
#endif
		}

		TLS::TLS()
		{ /* no-op */ }

		TLS::~TLS()
		{ /* no-op */ }
	}
}


