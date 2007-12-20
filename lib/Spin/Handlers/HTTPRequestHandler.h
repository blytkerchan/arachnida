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
		/** Middle-man for request handling. 
		 * The general idea of this class and the other handlers is that it 
		 * allows you to handle requests synchronously or asynchronously, all 
		 * the while handling one or more connections asynchronously. A good 
		 * example of this is the Mesolthelae mini-server of which the \em entire 
		 * source code is reproduced on the main page. */
		class SPIN_API HTTPRequestHandler
		{
		public :
			//! Called by the data handler when a new request is ready to be handled.
			void handle(const boost::shared_ptr< Details::Request > & request);
			//! Get a new request. Blocks until a request is ready.
			boost::shared_ptr< Details::Request > getNextRequest();

		private :
			std::deque< boost::shared_ptr< Details::Request > > requests_;
			boost::mutex requests_lock_;
			boost::condition requests_cond_;
		};
	}
}

#endif
