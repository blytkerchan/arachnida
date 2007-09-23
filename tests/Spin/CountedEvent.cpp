#include "CountedEvent.h"
#include <Spin/Private/CountedEvent.h>
#include <Spin/Private/yield.h>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

namespace Tests
{
	namespace Spin
	{
		namespace Private
		{
			namespace
			{
				::Spin::Private::CountedEvent event__;

				void threadFunc__()
				{
					event__.wait();
				}
			}

			CPPUNIT_TEST_SUITE_REGISTRATION(CountedEvent);

			void CountedEvent::setUp()
			{ /* no-op */ }

			void CountedEvent::tearDown()
			{ /* no-op */ }

			void CountedEvent::tryFiveThreads()
			{
				boost::thread thread1(boost::bind(&threadFunc__));
				boost::thread thread2(boost::bind(&threadFunc__));
				boost::thread thread3(boost::bind(&threadFunc__));
				boost::thread thread4(boost::bind(&threadFunc__));
				boost::thread thread5(boost::bind(&threadFunc__));
				::Spin::Private::yield(true);
				event__.switch_();
				event__.signal();
				thread5.join();
				thread4.join();
				thread3.join();
				thread2.join();
				thread1.join();
			}
		}
	}
}
