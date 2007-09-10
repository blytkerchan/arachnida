#ifndef _spin_private_logger_h
#define _spin_private_logger_h

#include "../Details/prologue.h"
#include <string>

namespace Spin
{
	namespace Private
	{
		class SPIN_API Logger
		{
		public :
			static void warning(const std::string & component, const std::string & message, const std::string & aux = std::string()) throw();
			static void error(const std::string & component, const std::string & message, const std::string & aux = std::string()) throw();
			static void fatalError(const std::string & component, const std::string & message, const std::string & aux = std::string()) throw();

		protected :
			static Logger * getInstance();
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
