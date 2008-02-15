//#define AGELENA_NDEBUG	// remove this to turn debugging on
#include "ConnectionHandler.h"
#include <Agelena/Logger.h>
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
#include <boost/thread/xtime.hpp>
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
				/*
				 * On Windows, the worker thread can apparently die without 
				 * any indication as to why or how, and without leaving the 
				 * call to select. This happens only when the application is 
				 * shutting down but does not allow us to shut down safely 
				 * without risking a hang in certain conditions (which have 
				 * been produced with proprietary code).
				 * As a result if this, we have to check here whether the 
				 * worker thread is still alive and, as it could die while
				 * we're waiting for it to notify us that is has successfully
				 * detached the FD from its internal sets, we cannot wait for 
				 * that notification indefinitely and have to check again, from
				 * time to time, that the thread is still there to notify us at
				 * some point. We do this every thirty milliseconds because in
				 * the cases where I have been able to test this, thirty milli-
				 * seconds has always been enough for the worker thread to wake 
				 * up and do its job, whereas it is also short enough to allow 
				 * the application to shutdown with apparent cleanliness...
				 */
#if defined(ON_WINDOZE)
				if (worker_thread_id_ != -1 && worker_thread_id_ != GetCurrentThreadId() && checkThreadStatus())
				do 
				{
					boost::xtime xt;
					boost::xtime_get(&xt, boost::TIME_UTC);
					xt.nsec += 30000000;
					callbacks_cond_.timed_wait(lock, xt);
				} while(boost::tuples::get<3>(*where) != detached__  && checkThreadStatus());
#else
#	if HAVE_BOOST_THREADID && HAVE_BOOST_THIS_THREAD
				/* hypothetical code */
				if (worker_thread_id_ != boost::this_thread::get_id())
#	else
				if (pthread_equal(pthread_self(), worker_thread_id_) == 0)
#	endif
				do 
				{
					callbacks_cond_.wait(lock);
				} while(boost::tuples::get<3>(*where) != detached__ );
#endif
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
#if ! defined(_WIN32) || defined(__CYGWIN__)
			/* On Windows, there really is no clean way to clean up after 
			 * ourselves: an alternative to this would be to have DllMain 
			 * take care of clean-up for us, but that's even sloppier than
			 * not notifying the thread that it's about to die...*/
			notifyThread_();
#endif
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
			if (!DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &worker_thread_handle_, 0, 0, DUPLICATE_SAME_ACCESS))
			{
				AGELENA_FATAL_ERROR_0("Failed to duplicate worker thread handle - cannot continue");
				worker_thread_id_ = ~0;
				throw std::bad_alloc();	// be more eloquent HERE
			}
			else
			{ /* all is well */ }
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
				AGELENA_DEBUG_5("Calling select(highest_fd_seen + 1 (%1%), &read_fds (%2%), &write_fds (%3%), &exc_fds (%4%), 0 (%5%))", (highest_fd_seen + 1), (void*)&read_fds, (void*)&write_fds, (void*)&exc_fds, 0);
				int select_result(::select(highest_fd_seen + 1, &read_fds, &write_fds, &exc_fds, 0));
				AGELENA_DEBUG_1("select returned with %1%", select_result);

				typedef std::list< int > CallbacksToRemove;
				CallbacksToRemove callbacks_to_remove;
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
					AGELENA_ERROR_2("%1% (%2%)", error_message, WSAGetLastError());
					/* When we get here one of the file descriptors in one 
					 * of the sets caused an error. We should find out which 
					 * one and remove it from the descriptors we work with. */
					CallbacksLock_::scoped_lock lock(callbacks_lock_);
					for (Callbacks_::iterator curr(callbacks_.begin()); curr != callbacks_.end(); ++curr)
					{
						if (boost::tuples::get<3>(*curr) == attached__)
						{
							if (!testFD_(boost::tuples::get<0>(*curr)))
							{
								AGELENA_WARNING_1("Scheduling FD %1% for removal", boost::tuples::get<0>(*curr));
								callbacks_to_remove.push_back(boost::tuples::get<0>(*curr));
							}
							else
							{ /* this one is OK */ }
						}
						else
						{ /* ignore this one, at least for now */ }
					}
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
					for (CallbacksToCall::const_iterator curr(callbacks_to_call.begin()); curr != callbacks_to_call.end(); ++curr)
					{
						try
						{
							AGELENA_DEBUG_1("Calling for FD %1%", boost::tuples::get<0>(*curr));
							(boost::tuples::get<1>(*curr))();
						}
						catch (...)
						{
							callbacks_to_remove.push_back(boost::tuples::get<0>(*curr));
						}
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
				else
				{
					if (FD_ISSET(sync_pipe_read_descriptor, &read_fds))
					{
						AGELENA_DEBUG_0("Fetching notification");
						// consume the bytes put in the pipe
						char buffer[64];
						std::size_t bytes_read_from_sync_pipe(sync_pipe_.read(buffer, sizeof(buffer)));
					}
					else
					{ /* not notified */ }
				}
			}
		}

		void ConnectionHandler::notifyThread_()
		{
			AGELENA_DEBUG_0("void ConnectionHandler::notifyThread_()");
			char sync(0);
			sync_pipe_.write(&sync, 1);
		}

		/*static */bool ConnectionHandler::testFD_(int fd)
		{
			fd_set read_fds;
			fd_set write_fds;
			fd_set exc_fds;
			FD_ZERO(&read_fds);
			FD_ZERO(&write_fds);
			FD_ZERO(&exc_fds);
			FD_SET(fd, &read_fds);
			struct timeval timeout;
			timeout.tv_sec = 0;
			timeout.tv_usec = 1;
			int rv(select(fd + 1, &read_fds, &write_fds, &exc_fds, &timeout));
			return rv >= 0;
		}

#if defined(ON_WINDOZE)
		bool ConnectionHandler::checkThreadStatus()
		{
			DWORD wfso_result(WaitForSingleObject(worker_thread_handle_, 0));
			AGELENA_DEBUG_1("WaitForSingleObject returned with %x", wfso_result);
			if (wfso_result == WAIT_TIMEOUT)
			{
				AGELENA_DEBUG_0("Worker thread still seems to be alive");
				return true;
			}
			else if (wfso_result == WAIT_OBJECT_0)
			{
				AGELENA_WARNING_0("Worker thread seems to have died during the call to select(2) - this probably means the application is shutting down");
				CloseHandle(worker_thread_handle_);
			}
			else if (wfso_result == WAIT_FAILED)
			{
				AGELENA_ERROR_0("Worker thread seems to have died during the call to select(2) - this probably means the application is shutting down, though it's strange that our handle is also dead");
			}
			else
			{
				AGELENA_ERROR_1("WaitForSingleObject returned with %x, which is unexpected. We will assume the worker thread is no longer alive.\nNote that this may cause problems if it is!", wfso_result);
			}
			worker_thread_handle_ = NULL;
			worker_thread_id_ = -1;
			return false;
		}
#endif
	}
}
