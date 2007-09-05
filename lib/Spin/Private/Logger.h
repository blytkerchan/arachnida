#ifndef _spin_private_logger_h
#define _spin_private_logger_h

#include <string>

namespace Spin
{
	namespace Private
	{
		class Logger
		{
		public :
			static void warning(const std::string & component, const std::string & message, const std::string & aux = std::string()) throw();
			static void error(const std::string & component, const std::string & message, const std::string & aux = std::string()) throw();
			static void fatalError(const std::string & component, const std::string & message, const std::string & aux = std::string()) throw();

		protected :
			static void setInstance(Logger * logger);
			virtual void warning_(const std::string & component, const std::string & message, const std::string & aux) = 0;
			virtual void error_(const std::string & component, const std::string & message, const std::string & aux) = 0;
			virtual void fatalError_(const std::string & component, const std::string & message, const std::string & aux) = 0;

		private :
			static Logger * instance__;
		};
	}
}

#endif
