#include "Pipe.h"
#include <Spin/Private/Pipe.h>
#include <boost/lambda/lambda.hpp>

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

			void Pipe::tryReadWrite()
			{
				::Spin::Private::Pipe pipe;
				char in_buffer[256];
				for (int i(0); i < sizeof(in_buffer); ++i)
					in_buffer[i] = i;
				pipe.write(in_buffer, sizeof(in_buffer));
				char out_buffer[sizeof(in_buffer)];
				CPPUNIT_ASSERT(pipe.read(out_buffer, sizeof(out_buffer)) == sizeof(out_buffer));
				using namespace boost::lambda;
				CPPUNIT_ASSERT(std::equal(in_buffer, in_buffer + sizeof(in_buffer), out_buffer, _1 == _2));
			}
		}
	}
}
