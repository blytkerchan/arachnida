#include "TLS.h"
#include <cassert>
#include <stdexcept>
#if defined(_WIN32) && !defined(__CYGWIN__)
#include <Windows.h>
#else
extern "C" {
#include <cerrno>
#include <pthread.h>
#define USE_POSIX_TLS
#define KEY(key) *((::pthread_key_t*)key.u_.p_)
}
#endif

namespace Spin
{
	namespace Private
	{
		/*static */TLS & TLS::getInstance()
		{
			static TLS instance__;
			return instance__;
		}

		TLS::Key TLS::acquireKey(Deleter deleter)
		{
#if defined(USE_POSIX_TLS)
			Key retval;
			retval.u_.p_ = new ::pthread_key_t;
			int rv(::pthread_key_create(&(KEY(retval)), deleter));
			if (rv != 0)
			{
				delete &(KEY(retval));
				throw std::bad_alloc();
			}
			else
			{ /* all is well */ }
			return retval;
#else
			DWORD _key(TlsAlloc());
			Key key; key.u_.ui_ = _key;
			boost::mutex::scoped_lock lock(keys_lock_);
			keys_.push_back(std::make_pair(key, deleter));
			return key;
#endif
		}

		void TLS::releaseKey(const TLS::Key & key)
		{
#if defined(USE_POSIX_TLS)
			int rv(::pthread_key_delete(KEY(key)));
			assert(rv == 0);
#else
			TlsFree(key.u_.ui_);
#endif
		}

		void TLS::setValue(const Key & key, void * value)
		{
#if defined(USE_POSIX_TLS)
			int rv(::pthread_setspecific(KEY(key), value));
			if (rv == EINVAL)
				throw std::logic_error("Invalid key");
			else if (rv != 0)
				throw std::bad_alloc();
			else
			{ /* all is well */ }
#else
			TlsSetValue(key.u_.ui_, value);
#endif
		}

		void * TLS::getValue(const TLS::Key & key) const
		{
#if defined(USE_POSIX_TLS)
			return ::pthread_getspecific(KEY(key));
#else
			return TlsGetValue(key.u_.ui_);
		}

		void TLS::_clean_()
		{
			boost::mutex::scoped_lock lock(keys_lock_);
			for (Keys_::const_iterator curr(keys_.begin()); curr != keys_.end(); ++curr)
			{
				void * val(TlsGetValue(curr->first.u_.ui_));
				if (val && curr->second)
					curr->second(val);
				else
				{ /* no-op */ }
			}
#endif
		}

		TLS::TLS()
		{ /* no-op */ }

		TLS::~TLS()
		{ /* no-op */ }
	}
}


