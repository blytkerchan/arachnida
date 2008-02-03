#ifndef _agelena_defaultwindowslogger_h
#define _agelena_defaultwindowslogger_h

#include "../Logger.h"

namespace Agelena
{
	namespace Loggers
	{
		struct AGELENA_API DefaultWindowsLogger : Logger
		{
			DefaultWindowsLogger();
			~DefaultWindowsLogger();
			/*virtual */void debug_(const std::string & component, const std::string & message, const std::string & aux);
			/*virtual */void warning_(const std::string & component, const std::string & message, const std::string & aux);
			/*virtual */void error_(const std::string & component, const std::string & message, const std::string & aux);
			/*virtual */void fatalError_(const std::string & component, const std::string & message, const std::string & aux);
		};
	}
}

#endif
