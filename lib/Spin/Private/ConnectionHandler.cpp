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
	struct DoneCountingPred
	{
		DoneCountingPred(boost::uint16_t target_count, Spin::Private::BakeryCounter & counter)
			: target_count_(target_count),
			  counter_(counter)
		{ /* no-op */ }

		bool operator()() const
		{
			return counter_.getBakerCounter() == target_count_;
		}

		boost::uint16_t target_count_;
		Spin::Private::BakeryCounter & counter_;
	};

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
			CallbacksLock_::scoped_lock lock(callbacks_lock_);
			Callbacks_::iterator where(std::find_if(callbacks_.begin(), callbacks_.end(), apply_to_first< Callbacks_::value_type >(std::bind2nd(std::equal_to< int >(), file_descriptor))));
			if (where != callbacks_.end())
			{
				callbacks_.erase(where);
				notifyThread_();
			}
			else
			{ /* no-op */ }
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
				int sync_pipe_read_descriptor(sync_pipe_.getReadDescriptor());
				FD_SET(sync_pipe_read_descriptor, &read_fds);
				highest_fd_seen = std::max(highest_fd_seen, sync_pipe_read_descriptor);
				int select_result(::select(highest_fd_seen, &read_fds, &write_fds, &exc_fds, 0));
				if (select_result <= 0)
				{
					std::string error_message;
					switch (WSAGetLastError())
					{
					case WSANOTINITIALISED : error_message = "A successful WSAStartup call must occur before using this function."; break;
					case WSAEFAULT : error_message = "The Windows Sockets implementation was unable to allocate needed resources for its internal operations, or the readfds, writefds, exceptfds, or timeval parameters are not part of the user address space."; break;
					case WSAENETDOWN : error_message = "The network subsystem has failed."; break;
					case WSAEINVAL : error_message = "The time-out value is not valid, or all three descriptor parameters were null."; break;
					case WSAEINTR : error_message = "A blocking Windows Socket 1.1 call was canceled through WSACancelBlockingCall."; break;
					case WSAEINPROGRESS : error_message = "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function."; break;
					case WSAENOTSOCK : error_message = "One of the descriptor sets contains an entry that is not a socket."; break;
					default : error_message = "Unknown error"; break;
					}
				}
				else
				{
					CallbacksLock_::scoped_lock lock(callbacks_lock_);
					typedef std::list< NotificationCallback > CallbacksToCall;
					CallbacksToCall callbacks_to_call;
					for (Callbacks_::const_iterator curr(callbacks_.begin()); curr != callbacks_.end(); ++curr)
					{
						if (FD_ISSET(curr->first, &read_fds))
							callbacks_to_call.push_back(curr->second);
						else
						{ /* fd not set */ }
					}
					// we keep the lock so a detaching thread won't think it will never be called again (unless it's in the worker thread that it's being detached)
					for (CallbacksToCall::const_iterator curr(callbacks_to_call.begin()); curr != callbacks_to_call.end(); ++curr)
						(*curr)();
				}
				if (FD_ISSET(sync_pipe_read_descriptor, &read_fds))
				{
					// consume the bytes put in the pipe
					char buffer[64];
					std::size_t bytes_read_from_sync_pipe(sync_pipe_.read(buffer, sizeof(buffer)));
				}
				else
				{ /* not notified */ }
			}
		}

		void ConnectionHandler::notifyThread_()
		{
			char sync(0);
			sync_pipe_.write(&sync, 1);
		}
	}
}