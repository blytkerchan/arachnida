#ifndef _acari_tls_h
#define _acari_tls_h

#include "Details/prologue.h"
#include "Mutex.h"

#if defined(_WIN32) && !defined(__CYGWIN__)
#	include <utility>
#	include <vector>
#endif

namespace Acari
{
	/** A wrapper around thread-local storage.
	 * To use this, you should allocate a key only once for each type of value you want to 
	 * store, and use that key in all threads to get a thread-local version of what you want
	 * to keeo around. */
	class ACARI_API ThreadLocalStorage
	{
	public :
		// this is a copyable opaque handle much in the same way as Windoze' HANDLE
		struct Key
		{ union { void * p_; unsigned int ui_; } u_; };
		//! The of a function that can delete something
		typedef void (*Deleter)(void *);

		//! Get the instance of the singleton
		static ThreadLocalStorage & getInstance();
		//! Get a key to use with TLS - you only need this once and can use it from all of your threads
		Key acquireKey(Deleter deleter);
		//! Release a key - you will no longer be able to use it(!)
		void releaseKey(const Key & key);
		//! Set a thread-local value associated with the key
		void setValue(const Key & key, void * value);
		//! Get the thread-local value associated with the key
		void * getValue(const Key & key) const;

#if defined(_WIN32) && !defined(__CYGWIN__)
		void _clean_();
#endif

	private :
#if defined(_WIN32) && !defined(__CYGWIN__)
		typedef std::vector< std::pair< Key, Deleter > > Keys_;
#endif

		ThreadLocalStorage();
		~ThreadLocalStorage();

#if defined(_WIN32) && !defined(__CYGWIN__)
		Keys_ keys_;
		Mutex keys_lock_;
#endif
	};
}

#endif

