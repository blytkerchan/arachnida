#include "Logger.h"

namespace Spin
{
	namespace Private
	{
		Logger * Logger::instance__(0);

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
	}
}
