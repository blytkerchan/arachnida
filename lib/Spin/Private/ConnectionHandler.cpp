#include "ConnectionHandler.h"
#if defined(_WIN32) && ! defined(__CYGWIN__)
#include <WinSock2.h>
#else
#error "Not implemented here yet"
#endif
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <functional>
#undef max

namespace 
{
	template < typename Pair, typename OpType >
	struct apply_to_1st_impl_
	{
		typedef typename OpType::result_type result_type;

		apply_to_1st_impl_(OpType op)
			: op_(op)
		{ /* no-op */ }

		result_type operator()(const Pair & pair) const
		{
			return op_(pair.first);
		}

		OpType op_;
	};

	template < typename Pair, typename OpType >
	apply_to_1st_impl_< Pair, OpType > apply_to_first(OpType op)
	{
		return apply_to_1st_impl_< Pair, OpType >(op);
	}

	template < typename Pair, typename OpType >
	struct apply_to_2nd_impl_
	{
		typedef typename OpType::result_type result_type;

		apply_to_2nd_impl_(OpType op)
			: op_(op)
		{ /* no-op */ }

		result_type operator()(const Pair & pair) const
		{
			return op_(pair.second);
		}

		OpType op_;
	};

	template < typename Pair, typename OpType >
	apply_to_2nd_impl_< Pair, OpType > apply_to_second(OpType op)
	{
		return apply_to_2nd_impl_< Pair, OpType >(op);
	}

	template < typename Tuple, typename OpType, int n >
	struct apply_to_nth_impl_
	{
		typedef typename OpType::result_type result_type;

		apply_to_nth_impl_(OpType op)
			: op_(op)
		{ /* no-op */ }

		result_type operator()(Tuple tuple) const
		{
			return op_(get< n >(tuple));
		}

		OpType op_;
	};

	template < typename Tuple, int n, typename OpType >
	apply_to_nth_impl_< Tuple, OpType, n > apply_to_nth(OpType op)
	{
		return apply_to_nth_impl_< Tuple, OpType, n >(op);
	}
}
namespace Spin
{
	namespace Private
	{
		/*static */ConnectionHandler & ConnectionHandler::getInstance()
		{
			static ConnectionHandler instance__;
			return instance__;
		}

		void ConnectionHandler::attach( int file_descriptor, NotificationCallback callback )
		{
			CallbacksLock_::scoped_lock lock(callbacks_lock_);
			callbacks_.push_back(std::make_pair(file_descriptor, callback));
			notifyThread_();
		}

		void ConnectionHandler::detach( int file_descriptor )
		{
			bool do_wait(false);
			{
				CallbacksLock_::scoped_lock lock(callbacks_lock_);
				Callbacks_::iterator where(std::find_if(callbacks_.begin(), callbacks_.end(), apply_to_first< Callbacks_::value_type >(std::bind2nd(std::equal_to< int >(), file_descriptor))));
				if (where != callbacks_.end())
				{
					callbacks_.erase(where);
					notifyThread_();
					do_wait = true;
				}
				else
				{ /* no-op */ }
			} // end the scope of the scoped lock
			if (do_wait) wait4Update_();
		}

		ConnectionHandler::ConnectionHandler()
			: done_(false),
			  worker_thread_(0)
		{
			worker_thread_ = new boost::thread(boost::bind(&ConnectionHandler::workerThreadFunc_, this));
		}

		ConnectionHandler::~ConnectionHandler()
		{
			done_ = true;
			if (worker_thread_)
				worker_thread_->join();
			else
			{ /* nothing to join */ }
			delete worker_thread_;
		}

		void ConnectionHandler::workerThreadFunc_()
		{
			fd_set read_fds;
			fd_set write_fds;
			fd_set exc_fds;

			while (!done_)
			{
				FD_ZERO(&read_fds);
				FD_ZERO(&write_fds);
				FD_ZERO(&exc_fds);

				int highest_fd_seen(0);
				{
					CallbacksLock_::scoped_lock lock(callbacks_lock_);
					for (Callbacks_::const_iterator curr(callbacks_.begin()); curr != callbacks_.end(); ++curr)
					{
						FD_SET(curr->first, &read_fds);
						highest_fd_seen = std::max(highest_fd_seen, curr->first);
					}
				} // end of scope for lock
				
			}
		}

		void ConnectionHandler::notifyThread_()
		{
		}

		void ConnectionHandler::wait4Update_()
		{
		}
	}
}
