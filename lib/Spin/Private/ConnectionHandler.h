#ifndef _spin_private_connectionhandler_h
#define _spin_private_connectionhandler_h

#include "get_observationadapter_type.h"

namespace boost
{
	class thread;
}
namespace Spin
{
	namespace Private
	{
		class ObservationAdapter;
		class ConnectionHandler
		{
		public :
			enum ObservationReasons { read__ = 1, write__ = 2, exception__ = 4 };

			static ConnectionHandler & getInstance();

			template < typename Observer >
			void attach(Observer observer, int reason = read__)
			{
				return attach_(get_observationadapter_type< Observer >::type(observer), reason);
			}

			template < typename Observer >
			void detach(Observer observer, int reason = read__ | write__ | exception__)
			{
				return detach_(get_observationadapter_type< Observer >::type(observer), reason);
			}

		private :
			struct Data;

			// Neither CopyConstructible nor Assignable
			ConnectionHandler(const ConnectionHandler &);
			ConnectionHandler & operator=(const ConnectionHandler &);

			// Privately constructible/destructible only
			ConnectionHandler();
			~ConnectionHandler();

			void attach_(ObservationAdapter * observer, int reason);
			void detach_(ObservationAdapter * observer, int reason);

			// this is the function that does all the work
			void threadFunc();
			void synchronize(bool wait_until_done = false);

			boost::thread * thread_;
			Data * data_;
		};
	}
}

#endif
