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

				/*virtual */void add_(unsigned int level, unsigned int component, unsigned int sub_component, const char * file, unsigned int line, const char * message, const unsigned char * aux, unsigned long aux_size) throw()/* = 0*/
				{
					(void)level;
					(void)component;
					(void)sub_component;
					(void)file;
					(void)line;
					(void)message;
					(void)aux;
					(void)aux_size;
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
