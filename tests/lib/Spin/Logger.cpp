#include "Logger.h"
#include <Spin/Private/Logger.h>
#include <stdexcept>

namespace Tests
{
	namespace Spin
	{
		namespace Private
		{
			namespace
			{
				class Log : public ::Spin::Private::Logger
				{
				public :
					Log()
					{
						if (getInstance() != 0)
							throw std::logic_error("This is not the only instance of a logger!");
						else
						{ /* no-op - all is well */ }
						setInstance(this);
					}

					~Log()
					{
						if (getInstance() == this)
							setInstance(0);
						else
						{ /* no-op */ }
					}

					/*virtual */void warning_(const std::string & component, const std::string & message, const std::string & aux) {}
					/*virtual */void error_(const std::string & component, const std::string & message, const std::string & aux) {}
					/*virtual */void fatalError_(const std::string & component, const std::string & message, const std::string & aux) {}
				};
			}
			CPPUNIT_TEST_SUITE_REGISTRATION(Logger);

			void Logger::setUp()
			{ /* no-op */ }

			void Logger::tearDown()
			{ /* no-op */ }

			void Logger::tryCreateInstance()
			{
				Log log;
			}
		}
	}
}
