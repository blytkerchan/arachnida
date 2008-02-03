#ifndef _agelena_logger_h
#define _agelena_logger_h

#include "Details/prologue.h"
#include <string>

namespace Agelena
{
	class AGELENA_API Logger
	{
	public :
		static void debug(const std::string & component, const std::string & message, const std::string & aux = std::string()) throw();
		static void warning(const std::string & component, const std::string & message, const std::string & aux = std::string()) throw();
		static void error(const std::string & component, const std::string & message, const std::string & aux = std::string()) throw();
		static void fatalError(const std::string & component, const std::string & message, const std::string & aux = std::string()) throw();

	protected :
		static Logger * getInstance();
		static void setInstance(Logger * logger);
		virtual void debug_(const std::string & component, const std::string & message, const std::string & aux);
		virtual void warning_(const std::string & component, const std::string & message, const std::string & aux) = 0;
		virtual void error_(const std::string & component, const std::string & message, const std::string & aux) = 0;
		virtual void fatalError_(const std::string & component, const std::string & message, const std::string & aux) = 0;

	private :
		static Logger * instance__;
	};
	/* USE WITH EXTREME CAUTION: we're defeating C++'s type system here! */
	std::string stringify(const char * fmt, ...);
}

#endif
