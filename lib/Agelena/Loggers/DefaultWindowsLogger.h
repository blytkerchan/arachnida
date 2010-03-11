#ifndef _agelena_defaultwindowslogger_h
#define _agelena_defaultwindowslogger_h

#include "../Logger.h"

namespace Agelena
{
	namespace Loggers
	{
		//! A default logger for windows, outputs to clog and/or cerr, as well as to OutputDebugStringA
		struct AGELENA_API DefaultWindowsLogger : Logger
		{
			DefaultWindowsLogger();
			~DefaultWindowsLogger();
			/*virtual */void add_(unsigned int level, unsigned int component, unsigned int sub_component, const char * file, unsigned int line, const char * message, const unsigned char * aux, unsigned long aux_size) throw()/* = 0*/;
		};
	}
}

#endif
