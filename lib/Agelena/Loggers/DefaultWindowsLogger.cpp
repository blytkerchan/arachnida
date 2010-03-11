#include "DefaultWindowsLogger.h"
#include <iostream>
#include <iomanip>
#include <Windows.h>

namespace Agelena
{
	namespace Loggers
	{
		DefaultWindowsLogger::DefaultWindowsLogger()
		{
			if (getInstance() != 0)
				throw std::logic_error("This is not the only instance of a logger!");
			else
			{ /* no-op - all is well */ }
			setInstance(this);
		}

		DefaultWindowsLogger::~DefaultWindowsLogger()
		{
			if (getInstance() == this)
				setInstance(0);
			else
			{ /* no-op */ }
		}

		/*virtual */void DefaultWindowsLogger::add_(unsigned int level, unsigned int component, unsigned int sub_component, const char * file, unsigned int line, const char * message, const unsigned char * aux, unsigned long aux_size) throw()/* = 0*/
		{
			(void)aux;
			(void)aux_size;
			using boost::format;
			using boost::io::group;
			using std::setfill;
			using std::setw;
			using std::hex ;

			format fmt("%3%(%4%): (%1%:%2%) %5%");
			fmt % group(setfill('0'), hex, setw(8), component)
				% group(setfill('0'), hex, setw(8), sub_component)
				% file
				% line
				% message
				;
			if (level <= info__)
			{
				OutputDebugStringA(fmt.str().c_str());
			}
			else if (level < error__)
			{
				std::clog << fmt.str() << std::endl;
			}
			else
			{
				std::cerr << fmt.str() << std::endl;
			}
		}
	}
}
