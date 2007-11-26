#include "HTTPRequestHandler.h"

namespace Spin
{
	namespace Details
	{
		struct Request;
	}
	namespace Handlers
	{
		void HTTPRequestHandler::handle(const boost::shared_ptr< Details::Request > & request)
		{
			{
				boost::mutex::scoped_lock lock(requests_lock_);
				requests_.push_back(request);
			}
			requests_cond_.notify_one();
		}

		boost::shared_ptr< Details::Request > HTTPRequestHandler::getNextRequest()
		{
			boost::mutex::scoped_lock lock(requests_lock_);
			while (requests_.empty())
				requests_cond_.wait(lock);
			boost::shared_ptr< Details::Request > retval(requests_.front());
			requests_.pop_front();
			return retval;
		}
	}
}
