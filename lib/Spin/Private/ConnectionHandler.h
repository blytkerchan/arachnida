#ifndef _spin_private_connectionhandler_h
#define _spin_private_connectionhandler_h

#include <list>
#include <boost/tuple/tuple.hpp>
#include <boost/function.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition.hpp>
#include "Pipe.h"
#include "BakeryCounter.h"

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

			void attach( int file_descriptor, NotificationCallback callback );
			void detach( int file_descriptor );

		private :
			enum AttachmentState_ { pending_attachment__, attached__, pending_detachment__, detached__ };
			ConnectionHandler(const ConnectionHandler&);
			ConnectionHandler & operator=(const ConnectionHandler&);

			typedef boost::tuple< int /* file_descriptor */, NotificationCallback /* callback */, AttachmentState_ /* state */ > Callback_;
			typedef std::list< Callback_ > Callbacks_;
			typedef boost::recursive_mutex CallbacksLock_;

			ConnectionHandler();
			~ConnectionHandler();

			void workerThreadFunc_();
			void notifyThread_();

			Callbacks_ callbacks_;
			CallbacksLock_ callbacks_lock_;
			boost::condition callbacks_cond_;
			volatile bool done_;
			boost::thread * worker_thread_;
			Pipe sync_pipe_;
#if HAVE_BOOST_THREADID && HAVE_BOOST_THIS_THREAD
			/* hypothetical code */
			boost::thread::id worker_thread_id_;
#else
			unsigned int worker_thread_id_;
#endif
		};
	}
}

#endif
