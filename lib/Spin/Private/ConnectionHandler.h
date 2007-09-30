#ifndef _spin_private_connectionhandler_h
#define _spin_private_connectionhandler_h

#include <utility>
#include <vector>
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>

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
			typedef boost::function< void(int) > NotificationCallback;

			static ConnectionHandler & getInstance();

			void attach( int file_descriptor, NotificationCallback callback );
			void detach( int file_descriptor );

		private :
			ConnectionHandler(const ConnectionHandler&);
			ConnectionHandler & operator=(const ConnectionHandler&);

			typedef std::vector< std::pair< int /* file_descriptor */, NotificationCallback /* callback */ > > Callbacks_;
			typedef boost::mutex CallbacksLock_;

			ConnectionHandler();
			~ConnectionHandler();

			void workerThreadFunc_();
			void notifyThread_();
			void wait4Update_();

			Callbacks_ callbacks_;
			CallbacksLock_ callbacks_lock_;
			volatile bool done_;
			boost::thread * worker_thread_;
		};
	}
}

#endif
