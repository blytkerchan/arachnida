#ifndef _spin_handlers_httprequesthandler_h
#define _spin_handlers_httprequesthandler_h

#include "../Details/prologue.h"
#include <deque>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

namespace Spin
{
	namespace Details
	{
		struct Request;
	}
	namespace Handlers
	{
		class SPIN_API HTTPRequestHandler
		{
		public :
			void handle(const boost::shared_ptr< Details::Request > & request);
			boost::shared_ptr< Details::Request > getNextRequest();

		private :
			std::deque< boost::shared_ptr< Details::Request > > requests_;
			boost::mutex requests_lock_;
			boost::condition requests_cond_;
		};
	}
}

#endif
