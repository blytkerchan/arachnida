#include "DefaultWindowsLogger.h"
#include <iostream>
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

		/*virtual */void DefaultWindowsLogger::debug_(const std::string & component, const std::string & message, const std::string & aux)
		{
			OutputDebugStringA((boost::format("%1% %2% %3%\n") % component % message % aux).str().c_str());
		}

		/*virtual */void DefaultWindowsLogger::warning_(const std::string & component, const std::string & message, const std::string & aux)
		{
			std::clog << component << ' ' << message << ' ' << aux << std::endl;
		}

		/*virtual */void DefaultWindowsLogger::error_(const std::string & component, const std::string & message, const std::string & aux)
		{
			std::cerr << component << ' ' << message << ' ' << aux << std::endl;
		}

		/*virtual */void DefaultWindowsLogger::fatalError_(const std::string & component, const std::string & message, const std::string & aux)
		{
			std::cerr << component << ' ' << message << ' ' << aux << std::endl;
		}
	}
}
