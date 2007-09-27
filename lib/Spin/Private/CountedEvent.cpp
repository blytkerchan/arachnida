#include "CountedEvent.h"
#include <cassert>
#include <memory>
#if HAVE_ATOMIC_PRIMITIVES
#include "atomicPrimitives.h"
#endif
#include "yield.h"

namespace Spin
{
	namespace Private
	{
		CountedEvent::CountedEvent()
			: current_(0)
		{
			wait_count_[0] = 0;
			wait_count_[1] = 0;
			events_[0] = 0;
			events_[1] = 0;
			std::auto_ptr< ACE_Event > event0(new ACE_Event(1));
			std::auto_ptr< ACE_Event > event1(new ACE_Event(1));
			events_[0] = event0.release();
			events_[1] = event1.release();
		}

		CountedEvent::~CountedEvent()
		{
			delete events_[1];
			delete events_[0];
		}

		void CountedEvent::wait()
		{
#if !HAVE_ATOMIC_PRIMITIVES
			rwlock_.acquire_read();
#endif
			int current(current_);
#if !HAVE_ATOMIC_PRIMITIVES
			rwlock_.release();
#endif
#if HAVE_ATOMIC_PRIMITIVES
			atomicIncrement(wait_count_[current]);
#else
			lock_.acquire();
			++(wait_count_[current]);
			lock_.release();
#endif
			events_[current]->wait();
#if HAVE_ATOMIC_PRIMITIVES
			boost::uint32_t count(fetchAndDecrement(wait_count_[current]));
#else
			lock_.acquire();
			boost::uint32_t count(((wait_count_[current])--));
			lock_.release();
#endif
			if (count == 1)
				events_[current]->reset();
			else
			{ /* no need to reset yet */ }
		}

		void CountedEvent::signal()
		{
#if !HAVE_ATOMIC_PRIMITIVES
			rwlock_.acquire_read();
			int current(current_);
			rwlock_.release();
			lock_.acquire();
			boost::uint32_t wait_count(wait_count_[!current]);
			lock_.release();
#else
			int current(current_);
			boost::uint32_t wait_count(wait_count_[!current]);
#endif
			if (wait_count)
				events_[!current]->signal();
			else
			{ /* no-one is waiting */ }
		}

		void CountedEvent::switch_()
		{
#if !HAVE_ATOMIC_PRIMITIVES
			rwlock_.acquire_read();
#endif
			int current(current_);
			while (wait_count_[!current])
				yield(true); // provides the no-fail guarantee
			assert(wait_count_[!current] == 0);
			assert(current == current_);
#if HAVE_ATOMIC_PRIMITIVES
			atomicSet(current_, !current);
#else
			while (rwlock_.tryacquire_write_upgrade() != 0)
				yield(true);
			current_ = !current;
			rwlock_.release();
#endif
		}
	}
}
