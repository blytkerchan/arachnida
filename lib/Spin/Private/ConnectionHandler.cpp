#include "ConnectionHandler.h"
#if defined(_WIN32) && ! defined(__CYGWIN__)
#include <WinSock2.h>
#define ON_WINDOZE
#else
#include <cerrno>
extern "C" {
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
}
#endif
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <functional>
#undef max

namespace 
{
	struct DoneCountingPred
	{
		DoneCountingPred(boost::uint16_t target_count, Acari::BakeryCounter & counter)
			: target_count_(target_count),
			  counter_(counter)
		{ /* no-op */ }

		bool operator()() const
		{
			return counter_.getBakerCounter() == target_count_;
		}

		boost::uint16_t target_count_;
		Acari::BakeryCounter & counter_;
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
			return op_(boost::tuples::get<0>(pair));
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
			return op_(boost::tuples::get< n >(tuple));
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

		void ConnectionHandler::attach( int file_descriptor, NotificationCallback on_select_callback, NotificationCallback on_error_callback/* = NotificationCallback()*/ )
		{
			CallbacksLock_::scoped_lock lock(callbacks_lock_);
			callbacks_.push_back(boost::make_tuple(file_descriptor, on_select_callback, on_error_callback, pending_attachment__));
			notifyThread_();
		}

		void ConnectionHandler::detach( int file_descriptor )
		{
			CallbacksLock_::scoped_lock lock(callbacks_lock_);
			Callbacks_::iterator where(std::find_if(callbacks_.begin(), callbacks_.end(), apply_to_first< Callbacks_::value_type >(std::bind2nd(std::equal_to< int >(), file_descriptor))));
			if (where != callbacks_.end())
			{
				boost::tuples::get<3>(*where) = pending_detachment__;
				notifyThread_();
#if HAVE_BOOST_THREADID && HAVE_BOOST_THIS_THREAD
				/* hypothetical code */
				if (worker_thread_id_ != boost::this_thread::get_id())
#elif defined(ON_WINDOZE)
				if (worker_thread_id_ != GetCurrentThreadId())
#else
				if (pthread_equal(pthread_self(), worker_thread_id_) == 0)
#endif
				do 
				{
					callbacks_cond_.wait(lock);
				} while(boost::tuples::get<3>(*where) != detached__);
				callbacks_.erase(where);
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

#if HAVE_BOOST_THREADID && HAVE_BOOST_THIS_THREAD
			/* hypothetical code */
			worker_thread_id_ = boost::this_thread::get_id();
#elif defined(ON_WINDOZE)
			worker_thread_id_ = GetCurrentThreadId();
#else
			worker_thread_id_ = pthread_self();
#endif

			while (!done_)
			{
				FD_ZERO(&read_fds);
				FD_ZERO(&write_fds);
				FD_ZERO(&exc_fds);

				int highest_fd_seen(0);
				{
					CallbacksLock_::scoped_lock lock(callbacks_lock_);
					for (Callbacks_::iterator curr(callbacks_.begin()); curr != callbacks_.end(); ++curr)
					{
						if (boost::tuples::get<3>(*curr) == pending_attachment__ || boost::tuples::get<3>(*curr) == attached__)
						{
							FD_SET(boost::tuples::get<0>(*curr), &read_fds);
							highest_fd_seen = std::max(highest_fd_seen, boost::tuples::get<0>(*curr));
							boost::tuples::get<3>(*curr) = attached__;
						}
						else if (boost::tuples::get<3>(*curr) == pending_detachment__)
						{
							boost::tuples::get<3>(*curr) = detached__;
						}
						else
						{ /* ignore this one altogether */ }
					}
				} // end of scope for lock
				callbacks_cond_.notify_all();
				int sync_pipe_read_descriptor(sync_pipe_.getReadDescriptor());
				FD_SET(sync_pipe_read_descriptor, &read_fds);
				highest_fd_seen = std::max(highest_fd_seen, sync_pipe_read_descriptor);
				int select_result(::select(highest_fd_seen, &read_fds, &write_fds, &exc_fds, 0));
				if (select_result <= 0)
				{
					std::string error_message;
#ifdef ON_WINDOZE
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
#else
					switch (errno)
					{
					case EBADF : error_message = "One or more of the file descriptor sets specified a file descriptor that is not a valid open file descriptor."; break;
					case EINTR : error_message = "The function was interrupted before any of the selected events occurred and before the timeout interval expired."; break; // If SA_RESTART has been set for the interrupting signal, it is implementation-defined whether the function restarts or returns with [EINTR].
					case EINVAL : error_message = "An invalid timeout interval was specified OR The nfds argument is less than 0 or greater than FD_SETSIZE OR One of the specified file descriptors refers to a STREAM or multiplexer that is linked (directly or indirectly) downstream from a multiplexer."; break;
					default : error_message = "Unknown error"; break;
					}
#endif
					// log this error_message HERE

					/* When we get HERE one of the file descriptors in one of the sets caused an error. We should find out which one and remove it from the descriptors we work with. */
				}
				else
				{
					typedef std::list< boost::tuples::tuple< int, NotificationCallback > > CallbacksToCall;
					CallbacksToCall callbacks_to_call;
					{
						CallbacksLock_::scoped_lock lock(callbacks_lock_);
						Callbacks_::const_iterator begin(callbacks_.begin());
						for (Callbacks_::const_iterator curr(begin); curr != callbacks_.end(); ++curr)
						{
							if (FD_ISSET(boost::tuples::get<0>(*curr), &read_fds))
								callbacks_to_call.push_back(boost::make_tuple(boost::tuples::get<0>(*curr), boost::tuples::get<1>(*curr)));
							else
							{ /* fd not set */ }
						}
					} // lock ends here
					typedef std::list< int > CallbacksToRemove;
					CallbacksToRemove callbacks_to_remove;
					for (CallbacksToCall::const_iterator curr(callbacks_to_call.begin()); curr != callbacks_to_call.end(); ++curr)
					{
						try
						{
							(boost::tuples::get<1>(*curr))();
						}
						catch (...)
						{
							callbacks_to_remove.push_back(boost::tuples::get<0>(*curr));
						}
					}
					if (!callbacks_to_remove.empty())
					{
						CallbacksLock_::scoped_lock lock(callbacks_lock_);
						for (CallbacksToRemove::const_iterator curr(callbacks_to_remove.begin()); curr != callbacks_to_remove.end(); ++curr)
						{
							Callbacks_::iterator where(std::find_if(callbacks_.begin(), callbacks_.end(), apply_to_nth< Callbacks_::value_type, 0 >(std::bind2nd(std::equal_to< int >(), *curr))));
							if (where != callbacks_.end())
							{
								boost::tuples::get<3>(*where) = pending_detachment__;
								if (!boost::tuples::get<2>(*where).empty())
									boost::tuples::get<2>(*where)();
								else
								{ /* no on-error callback to call */ }
							}
							else
							{ /* The callback is no longer there anyway */ }
						}
					}
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
