#include "Logger.h"
#include <Agelena/Logger.h>
#include <stdexcept>

namespace Tests
{
	namespace Agelena
	{
		namespace
		{
			class Log : public ::Agelena::Logger
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

				/*virtual */void debug_(const std::string & component, const std::string & message, const std::string & aux)
				{
					// do something useful here
				}

				/*virtual */void warning_(const std::string & component, const std::string & message, const std::string & aux)
				{
					// do something useful here
				}

				/*virtual */void error_(const std::string & component, const std::string & message, const std::string & aux)
				{
					// do something useful here
				}

				/*virtual */void fatalError_(const std::string & component, const std::string & message, const std::string & aux)
				{
					// do something useful here
				}
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
