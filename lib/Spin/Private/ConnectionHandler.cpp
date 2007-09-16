#include "ConnectionHandler.h"
#include <map>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <ace/ACE.h>
#include <ace/Handle_Set.h>
#include <ace/Event.h>
#include <ace/Thread_Mutex.h>
#include <ace/Pipe.h>
#include "ObservationAdapter.h"
#include "ScopedLock.h"
#include "CountedEvent.h"
#include "Logger.h"

namespace Spin
{
	namespace Private
	{
		/* The connection handler has three public sets of handles (file 
		 * descriptors) on which it will perform a select(). It has copies 
		 * of these privately in its worker thread, where the select will 
		 * be performed.
		 * As the connection handler is the subject in an observer pattern,
		 * it will update its observers when anything happens with any of 
		 * the handles that have been attached to the subject. */
		struct ConnectionHandler::Data
		{
			typedef std::map< ACE_HANDLE, ObservationAdapter* > ReadObservers;
			typedef std::map< ACE_HANDLE, ObservationAdapter* > WriteObservers;
			typedef std::map< ACE_HANDLE, ObservationAdapter* > ExceptionObservers;

			Data()
			{
				if (sync_pipe_.open(0/* no buffer */) != 0)
					throw std::runtime_error("Failed to open synchronization pipe (internal)");
				else
				{ /* all is well */ }
			}

			ReadObservers read_observers_;
			ACE_Thread_Mutex read_observers_lock_;
			WriteObservers write_observers_;
			ACE_Thread_Mutex write_observers_lock_;
			ExceptionObservers exception_observers_;
			ACE_Thread_Mutex exception_observers_lock_;

			/* We use this pipe to synchronize: we wake the thread up with it
			 * so it will read the three maps we have here. The event is set by 
			 * the thread when it has finished synchronizing the maps. */
			ACE_Pipe sync_pipe_;
			CountedEvent sync_event_;
		};

		/*static */ConnectionHandler & ConnectionHandler::getInstance()
		{
			static ConnectionHandler instance__;
			return instance__;
		}

		ConnectionHandler::ConnectionHandler()
			: thread_(0),
			  data_(new Data)
		{
			// can't initialize because we can't use "this" during initialization
			thread_ = new boost::thread(boost::bind(&ConnectionHandler::threadFunc, this));
		}

		ConnectionHandler::~ConnectionHandler()
		{
			delete data_;
			delete thread_;
		}

		void ConnectionHandler::attach_(ObservationAdapter * observer, int reason)
		{
//			Logger::debug("ConnectionHandler", "in attach_", stringify("(%p,%d)", observer, reason));
			bool shared_set_changed(false);
			if (reason & read__)
			{
				ScopedLock lock(data_->read_observers_lock_);
				data_->read_observers_.insert(std::make_pair(observer->getHandle(), observer));
				shared_set_changed = true;
			}
			else
			{ /* no need to change the set */ }
			if (reason & write__)
			{
				ScopedLock lock(data_->write_observers_lock_);
				data_->write_observers_.insert(std::make_pair(observer->getHandle(), observer));
				shared_set_changed = true;
			}
			else
			{ /* no need to change the set */ }
			if (reason & exception__)
			{
				ScopedLock lock(data_->exception_observers_lock_);
				data_->exception_observers_.insert(std::make_pair(observer->getHandle(), observer));
				shared_set_changed = true;
			}
			else
			{ /* no need to change the set */ }
			if (shared_set_changed)
				synchronize();
			else
			{ /* no reason to synchronize */ }
		}

		void ConnectionHandler::detach_(ObservationAdapter * observer, int reason)
		{
//			Logger::debug("ConnectionHandler", "in detach_", stringify("(%p,%d)", observer, reason));
			bool shared_set_changed(false);
			ACE_HANDLE handle(observer->getHandle());
			if (reason & read__)
			{
				ScopedLock lock(data_->read_observers_lock_);
				Data::ReadObservers::iterator where(data_->read_observers_.find(handle));
				if (where != data_->read_observers_.end())
				{
					data_->read_observers_.erase(where);
					shared_set_changed = true;
				}
				else
				{ /* not found */ }
			}
			else
			{ /* no need to change the set */ }
			if (reason & write__)
			{
				ScopedLock lock(data_->write_observers_lock_);
				Data::WriteObservers::iterator where(data_->write_observers_.find(handle));
				if (where != data_->write_observers_.end())
				{
					data_->write_observers_.erase(where);
					shared_set_changed = true;
				}
				else
				{ /* not found */ }
			}
			else
			{ /* no need to change the set */ }
			if (reason & exception__)
			{
				ScopedLock lock(data_->exception_observers_lock_);
				Data::ReadObservers::iterator where(data_->exception_observers_.find(handle));
				if (where != data_->exception_observers_.end())
				{
					data_->exception_observers_.erase(where);
					shared_set_changed = true;
				}
				else
				{ /* not found */ }
			}
			else
			{ /* no need to change the set */ }
			if (shared_set_changed)
				synchronize(true);
			else
			{ /* no reason to synchronize */ }
		}

		void ConnectionHandler::threadFunc() try
		{
			/* this thread keeps its own sets of handles to call select() on.
			 * It synchronizes these sets with the shared ones when it has 
			 * something to read from the synchronization handle, which is a 
			 * pipe to which any thread that has something to add or remove 
			 * from the shared sets will write a single octet. */
			Data::ReadObservers private_read_observers;
			Data::WriteObservers private_write_observers;
			Data::ExceptionObservers private_exception_observers;
			ACE_Handle_Set read_handles;
			ACE_Handle_Set write_handles;
			ACE_Handle_Set exception_handles;
			unsigned int consecutive_error_count(0);

			while (1)
			{
				// prepare to sync
				Logger::debug("ConnectionHandler", "starting sync - switching");
				data_->sync_event_.switch_();
				{
					ScopedLock lock(data_->read_observers_lock_);
					private_read_observers = data_->read_observers_;
				}
				{
					ScopedLock lock(data_->write_observers_lock_);
					private_write_observers = data_->write_observers_;
				}
				{
					ScopedLock lock(data_->exception_observers_lock_);
					private_exception_observers = data_->exception_observers_;
				}
				// done syncing
				Logger::debug("ConnectionHandler", "done syncing - signalling");
				data_->sync_event_.signal();
				// local sync starts here
				{
					read_handles.reset();
					Data::ReadObservers::const_iterator end(private_read_observers.end());
					for (Data::ReadObservers::const_iterator curr(private_read_observers.begin()); curr != end; ++curr)
						read_handles.set_bit(curr->first);
				}
				{
					write_handles.reset();
					Data::WriteObservers::const_iterator end(private_write_observers.end());
					for (Data::WriteObservers::const_iterator curr(private_write_observers.begin()); curr != end; ++curr)
						write_handles.set_bit(curr->first);
				}
				{
					exception_handles.reset();
					Data::ExceptionObservers::const_iterator end(private_exception_observers.end());
					for (Data::ExceptionObservers::const_iterator curr(private_exception_observers.begin()); curr != end; ++curr)
						exception_handles.set_bit(curr->first);
				}
				// done syncing
				// set the sync bit
				read_handles.set_bit(data_->sync_pipe_.read_handle());
				// we are now ready to call select
				int width(read_handles.num_set());
				width = write_handles.num_set() > width ? write_handles.num_set() : width;
				width = exception_handles.num_set() > width ? exception_handles.num_set() : width;
				int select_retval(ACE::select(width, &read_handles, &write_handles, &exception_handles));
				Logger::debug("ConnectionHandler", "select returned", stringify("return value %d", select_retval));
				switch (select_retval)
				{
				case -1 :
					if (consecutive_error_count++ >= 3)
					{
						Logger::fatalError("ConnectionHandler", "Three or more consecutive errors occurred during normal operation - aborting", strerror(errno));
						exit(1);
					}
#ifdef _WIN32
					else if (errno == WSAEINVAL)
					{
						Logger::fatalError("ConnectionHandler", "There is an invalid handle among my handle set - this is a BUG!");
						exit(1);	// find a way to recover from this (e.g. scan the validity of all handles, reset the sync pipe, etc.
					}
#endif
					else
						Logger::error("ConnectionHandler", "A presumably recoverable error occurred during normal operation", strerror(errno));
					break;
				case 0 :
					Logger::warning("ConnectionHandler", "Spurious wake-up during normal operation.");
					break;
				default :
					consecutive_error_count = 0;
					{	// check the read handles
						Data::ReadObservers::const_iterator end(private_read_observers.end());
						for (Data::ReadObservers::const_iterator curr(private_read_observers.begin()); curr != end; ++curr)
						{
							if (read_handles.is_set(curr->first))
								curr->second->onReadReady();
							else
							{ /* no need to call here */ }
						}
					}
					{	// check the write handles
						Data::WriteObservers::const_iterator end(private_write_observers.end());
						for (Data::WriteObservers::const_iterator curr(private_write_observers.begin()); curr != end; ++curr)
						{
							if (write_handles.is_set(curr->first))
								curr->second->onWriteReady();
							else
							{ /* no need to call here */ }
						}
					}
					{	// check the exception handles
						Data::ExceptionObservers::const_iterator end(private_exception_observers.end());
						for (Data::ExceptionObservers::const_iterator curr(private_exception_observers.begin()); curr != end; ++curr)
						{
							if (exception_handles.is_set(curr->first))
								curr->second->onExceptionReady();
							else
							{ /* no need to call here */ }
						}
					}
					// flush the sync pipe
					if (read_handles.is_set(data_->sync_pipe_.read_handle()))
					{
						/*
						 * we only read one character at a time, to avoid 
						 * a race condition in which a thread waiting to 
						 * detach the final socket is never released from 
						 * its wait because we don't loop through the loop 
						 * again.
						 * Though this is unlikely to happen in real life, 
						 * it will occasionally deadlock our test cases,
						 * which have instances of the Server class with a
						 * very short life-span.
						 */
						char flush;
						int rv(data_->sync_pipe_.recv(&flush, 1));	// we can safely ignore the return value here
						assert(rv == 1);
					}
					else
					{ /* nothing to read here */ }
				}
			}
		}
		catch (const std::exception & e)
		{
			Logger::fatalError("ConnectionHandler", "An exception occurred during normal operation", e.what());
			exit(1);
		}
		catch (...)
		{
			Logger::fatalError("ConnectionHandler", "An unknown exception occurred during normal operation");
			exit(1);
		}

		void ConnectionHandler::synchronize(bool wait_until_done/* = false*/)
		{
			char cruft;
			data_->sync_pipe_.send(&cruft, 1);
			if (wait_until_done)
				data_->sync_event_.wait();
			else
			{ /* no need to wait */ }
		}
	}
}
