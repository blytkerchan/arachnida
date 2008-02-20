#include "DefaultPOSIXLogger.h"
#include <iostream>

namespace Agelena
{
	namespace Loggers
	{
		DefaultPOSIXLogger::DefaultPOSIXLogger()
		{
			if (getInstance() != 0)
				throw std::logic_error("This is not the only instance of a logger!");
			else
			{ /* no-op - all is well */ }
			setInstance(this);
		}

		DefaultPOSIXLogger::~DefaultPOSIXLogger()
		{
			if (getInstance() == this)
				setInstance(0);
			else
			{ /* no-op */ }
		}

		/*virtual */void DefaultPOSIXLogger::debug_(const std::string & component, const std::string & message, const std::string & aux)
		{
			std::clog << component << ' ' << message << ' ' << aux << std::endl;
		}

		/*virtual */void DefaultPOSIXLogger::warning_(const std::string & component, const std::string & message, const std::string & aux)
		{
			std::clog << component << ' ' << message << ' ' << aux << std::endl;
		}

		/*virtual */void DefaultPOSIXLogger::error_(const std::string & component, const std::string & message, const std::string & aux)
		{
			std::cerr << component << ' ' << message << ' ' << aux << std::endl;
		}

		/*virtual */void DefaultPOSIXLogger::fatalError_(const std::string & component, const std::string & message, const std::string & aux)
		{
			std::cerr << component << ' ' << message << ' ' << aux << std::endl;
		}
	}
}
