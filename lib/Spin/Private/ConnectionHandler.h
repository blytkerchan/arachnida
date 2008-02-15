#ifndef _spin_private_connectionhandler_h
#define _spin_private_connectionhandler_h

#include <list>
#include <boost/tuple/tuple.hpp>
#include <boost/function.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition.hpp>
#include <Acari/BakeryCounter.h>
#if defined(_WIN32) && ! defined(__CYGWIN__)
#	define _WINSOCKAPI_ // Prevent inclusion of winsock.h in windows.h
#	include <Windows.h>
#endif
#include "Pipe.h"

namespace boost
{
	class thread;
}
namespace Spin
{
	namespace Private
	{
		class ConnectionHandler
		{
		public :
			typedef boost::function< void() > NotificationCallback;

			static ConnectionHandler & getInstance();

			void attach( int file_descriptor, NotificationCallback on_select_callback, NotificationCallback on_error_callback = NotificationCallback() );
			void detach( int file_descriptor );

		private :
			enum AttachmentState_ { pending_attachment__, attached__, pending_detachment__, detached__ };
			ConnectionHandler(const ConnectionHandler&);
			ConnectionHandler & operator=(const ConnectionHandler&);

			typedef boost::tuple< int /* file_descriptor */, NotificationCallback /* on_select_callback */, NotificationCallback /* on_error_callback */, AttachmentState_ /* state */ > Callback_;
			typedef std::list< Callback_ > Callbacks_;
			typedef boost::recursive_mutex CallbacksLock_;

			ConnectionHandler();
			~ConnectionHandler();

			void workerThreadFunc_();
			void notifyThread_();
			static bool testFD_(int fd);
#if defined(_WIN32) && ! defined(__CYGWIN__)
			bool checkThreadStatus();
#endif

			Callbacks_ callbacks_;
			CallbacksLock_ callbacks_lock_;
			boost::condition callbacks_cond_;
			volatile bool done_;
			boost::thread * worker_thread_;
			Pipe sync_pipe_;
#if HAVE_BOOST_THREADID && HAVE_BOOST_THIS_THREAD
			/* hypothetical code */
			boost::thread::id worker_thread_id_;
#elif defined(_WIN32) && ! defined(__CYGWIN__)
			unsigned int worker_thread_id_;
			HANDLE worker_thread_handle_;
#else
			pthread_t worker_thread_id_;
#endif
		};
	}
}

#endif
