#include "Logger.h"
#include <cstdarg>

namespace Spin
{
	namespace Private
	{
		Logger * Logger::instance__(0);

		/*static */void Logger::debug(const std::string & component, const std::string & message, const std::string & aux/* = std::string()*/) throw()
		{
			if (instance__)
			{
				try
				{
					instance__->debug_(component, message, aux);
				}
				catch (...)
				{ /* nothing I can do here */ }
			}
			else
			{ /* ignore */ }
		}

		/*static */void Logger::warning(const std::string & component, const std::string & message, const std::string & aux/* = std::string()*/) throw()
		{
			if (instance__)
			{
				try
				{
					instance__->warning_(component, message, aux);
				}
				catch (...)
				{ /* nothing I can do here */ }
			}
			else
			{ /* ignore */ }
		}

		/*static */void Logger::error(const std::string & component, const std::string & message, const std::string & aux/* = std::string()*/) throw()
		{
			if (instance__)
			{
				try
				{
					instance__->error_(component, message, aux);
				}
				catch (...)
				{ /* nothing I can do here */ }
			}
			else
			{ /* ignore */ }
		}

		/*static */void Logger::fatalError(const std::string & component, const std::string & message, const std::string & aux/* = std::string()*/) throw()
		{
			if (instance__)
			{
				try
				{
					instance__->fatalError_(component, message, aux);
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

		/*virtual */void Logger::debug_(const std::string & component, const std::string & message, const std::string & aux)
		{ /* no-op by default - loggers are ot required to have a debug_ method */ }

		/* USE WITH EXTREME CAUTION: we're defeating C++'s type system here! */
		std::string stringify(const char * fmt, ...)
		{
			char buffer[512];
			memset(buffer, 0, sizeof(buffer));
			va_list args;
			va_start(args, fmt);
			int count(vsnprintf(buffer, sizeof(buffer), fmt, args));
			va_end(args);
			return std::string(buffer, buffer + count);
		}
	}
}
