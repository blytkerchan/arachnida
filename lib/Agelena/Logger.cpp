#include "Logger.h"
#include <cstdarg>

namespace Agelena
{
	Logger * Logger::instance__(0);

	/*static */void Logger::add(unsigned int level, unsigned int component, unsigned int sub_component, const char * file, unsigned int line, const char * message, const unsigned char * aux/* = 0*/, unsigned long aux_size/* = 0*/)
	{
		if (instance__)
		{
			try
			{
				instance__->add_(level, component, sub_component, file, line, message, aux, aux_size);
			}
			catch (...)
			{ /* nothing I can do here */ }
		}
		else
		{ /* ignore */ }
	}

	/*static */Logger * Logger::getInstance()
	{
		return instance__;
	}

	/*static */void Logger::setInstance(Logger * logger)
	{
		instance__ = logger;
	}
}
