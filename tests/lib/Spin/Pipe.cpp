#include "Pipe.h"
#include <Spin/Private/Pipe.h>

namespace Tests
{
	namespace Spin
	{
		namespace Private
		{
			CPPUNIT_TEST_SUITE_REGISTRATION(Pipe);

			void Pipe::setUp()
			{ /* no-op */ }

			void Pipe::tearDown()
			{ /* no-op */ }

			void Pipe::tryCreateInstance()
			{
				::Spin::Private::Pipe pipe;
			}
		}
	}
}
