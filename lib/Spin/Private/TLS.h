#ifndef _spin_private_tls_h
#define _spin_private_tls_h

#if defined(_WIN32) && !defined(__CYGWIN__)
#	include <utility>
#	include <vector>
#	include <boost/thread/mutex.hpp>
#endif

namespace Spin
{
	namespace Private
	{
		class TLS
		{
		public :
			// this is a copyable opaque handle much in the same way as Windoze' HANDLE
			struct Key
			{ union { void * p_; unsigned int ui_; } u_; };
			typedef void (*Deleter)(void *);

			//! Get the instance of the singleton
			static TLS & getInstance();
			//! Get a key to use with TLS - you only need this once and can use it from all of your threads
			Key acquireKey(Deleter deleter);
			void releaseKey(const Key & key);
			void setValue(const Key & key, void * value);
			void * getValue(const Key & key) const;

#if defined(_WIN32) && !defined(__CYGWIN__)
			void _clean_();
#endif

		private :
#if defined(_WIN32) && !defined(__CYGWIN__)
			typedef std::vector< std::pair< Key, Deleter > > Keys_;
#endif

			TLS();
			~TLS();

#if defined(_WIN32) && !defined(__CYGWIN__)
			Keys_ keys_;
			boost::mutex keys_lock_;
#endif
		};
	}
}

#endif

