#ifndef _spin_private_connectionhandler_h
#define _spin_private_connectionhandler_h

#include <utility>
#include <list>
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
			typedef boost::function< void(/*int*/) > NotificationCallback;

			static ConnectionHandler & getInstance();

			void attach( int file_descriptor, NotificationCallback callback );
			void detach( int file_descriptor );

		private :
			ConnectionHandler(const ConnectionHandler&);
			ConnectionHandler & operator=(const ConnectionHandler&);

			typedef std::list< std::pair< int /* file_descriptor */, NotificationCallback /* callback */ > > Callbacks_;
			typedef boost::recursive_mutex CallbacksLock_;

			ConnectionHandler();
			~ConnectionHandler();

			void workerThreadFunc_();
			void notifyThread_();

			Callbacks_ callbacks_;
			CallbacksLock_ callbacks_lock_;
			volatile bool done_;
			boost::thread * worker_thread_;
			Pipe sync_pipe_;
		};
	}
}

#endif
