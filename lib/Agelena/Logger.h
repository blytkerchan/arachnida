#ifndef _agelena_logger_h
#define _agelena_logger_h

#include "Details/prologue.h"
#include <string>
#include <boost/format.hpp>

namespace Agelena
{
	/** The base class for all loggers.
	 * If you want to write one, derive it from this class. */
	class AGELENA_API Logger
	{
	public :
		//! Call this to register a debug message
		static void debug(const std::string & component, const std::string & message, const std::string & aux = std::string()) throw();
		//! Call this to register a warning message
		static void warning(const std::string & component, const std::string & message, const std::string & aux = std::string()) throw();
		//! Call this to register an error message
		static void error(const std::string & component, const std::string & message, const std::string & aux = std::string()) throw();
		/** Call this to register a fatal error message
		 * \note the caller should not expect the logger to do anything rash
		 *       just because the error is fatal. */
		static void fatalError(const std::string & component, const std::string & message, const std::string & aux = std::string()) throw();

	protected :
		//! Returns the one known instance of the Logger. Called by the static methods of this class
		static Logger * getInstance();
		/** Set the one instance of the Logger.
		 * Normally called by the constructor of a derived class, which \em should check that getInstance returns 0 first. */
		static void setInstance(Logger * logger);
		//! Called to register a debug message - ignored by default
		virtual void debug_(const std::string & component, const std::string & message, const std::string & aux);
		//! Called to register a warning message
		virtual void warning_(const std::string & component, const std::string & message, const std::string & aux) = 0;
		//! Called to register an error message
		virtual void error_(const std::string & component, const std::string & message, const std::string & aux) = 0;
		//! Called to register a fatal error message
		virtual void fatalError_(const std::string & component, const std::string & message, const std::string & aux) = 0;

	private :
		static Logger * instance__;
	};

#ifndef AGELENA_NDEBUG
#define AGELENA_DEBUG_0(msg)													\
	Agelena::Logger::debug((boost::format("%1%(%2%) : debug :") % __FILE__ % __LINE__).str(), msg)
#define AGELENA_DEBUG_1(msg, p1)												\
	Agelena::Logger::debug((boost::format("%1%(%2%) : debug :") % __FILE__ % __LINE__).str(), (boost::format(msg) % p1).str())
#define AGELENA_DEBUG_2(msg, p1, p2)											\
	Agelena::Logger::debug((boost::format("%1%(%2%) : debug :") % __FILE__ % __LINE__).str(), (boost::format(msg) % p1 % p2).str())
#define AGELENA_DEBUG_3(msg, p1, p2, p3)										\
	Agelena::Logger::debug((boost::format("%1%(%2%) : debug :") % __FILE__ % __LINE__).str(), (boost::format(msg) % p1 % p2 % p3).str())
#define AGELENA_DEBUG_4(msg, p1, p2, p3, p4)									\
	Agelena::Logger::debug((boost::format("%1%(%2%) : debug :") % __FILE__ % __LINE__).str(), (boost::format(msg) % p1 % p2 % p3 % p4).str())
#define AGELENA_DEBUG_5(msg, p1, p2, p3, p4, p5)								\
	Agelena::Logger::debug((boost::format("%1%(%2%) : debug :") % __FILE__ % __LINE__).str(), (boost::format(msg) % p1 % p2 % p3 % p4 % p5).str())
#else
#define AGELENA_DEBUG_0(msg)													\
	(void)0
#define AGELENA_DEBUG_1(msg, p1)												\
	(void)0
#define AGELENA_DEBUG_2(msg, p1, p2)											\
	(void)0
#define AGELENA_DEBUG_3(msg, p1, p2, p3)										\
	(void)0
#define AGELENA_DEBUG_4(msg, p1, p2, p3, p4)									\
	(void)0
#define AGELENA_DEBUG_5(msg, p1, p2, p3, p4, p5)								\
	(void)0
#endif
#define AGELENA_WARNING_0(msg)													\
	Agelena::Logger::warning((boost::format("%1%(%2%) : warning :") % __FILE__ % __LINE__).str(), msg)
#define AGELENA_WARNING_1(msg, p1)												\
	Agelena::Logger::warning((boost::format("%1%(%2%) : warning :") % __FILE__ % __LINE__).str(), (boost::format(msg) % p1).str())
#define AGELENA_WARNING_2(msg, p1, p2)											\
	Agelena::Logger::warning((boost::format("%1%(%2%) : warning :") % __FILE__ % __LINE__).str(), (boost::format(msg) % p1 % p2).str())
#define AGELENA_WARNING_3(msg, p1, p2, p3)										\
	Agelena::Logger::warning((boost::format("%1%(%2%) : warning :") % __FILE__ % __LINE__).str(), (boost::format(msg) % p1 % p2 % p3).str())
#define AGELENA_WARNING_4(msg, p1, p2, p3, p4)									\
	Agelena::Logger::warning((boost::format("%1%(%2%) : warning :") % __FILE__ % __LINE__).str(), (boost::format(msg) % p1 % p2 % p3 % p4).str())
#define AGELENA_WARNING_5(msg, p1, p2, p3, p4, p5)								\
	Agelena::Logger::warning((boost::format("%1%(%2%) : warning :") % __FILE__ % __LINE__).str(), (boost::format(msg) % p1 % p2 % p3 % p4 % p5).str())
#define AGELENA_ERROR_0(msg)													\
	Agelena::Logger::error((boost::format("%1%(%2%) : error :") % __FILE__ % __LINE__).str(), msg)
#define AGELENA_ERROR_1(msg, p1)												\
	Agelena::Logger::error((boost::format("%1%(%2%) : error :") % __FILE__ % __LINE__).str(), (boost::format(msg) % p1).str())
#define AGELENA_ERROR_2(msg, p1, p2)											\
	Agelena::Logger::error((boost::format("%1%(%2%) : error :") % __FILE__ % __LINE__).str(), (boost::format(msg) % p1 % p2).str())
#define AGELENA_ERROR_3(msg, p1, p2, p3)										\
	Agelena::Logger::error((boost::format("%1%(%2%) : error :") % __FILE__ % __LINE__).str(), (boost::format(msg) % p1 % p2 % p3).str())
#define AGELENA_ERROR_4(msg, p1, p2, p3, p4)									\
	Agelena::Logger::error((boost::format("%1%(%2%) : error :") % __FILE__ % __LINE__).str(), (boost::format(msg) % p1 % p2 % p3 % p4).str())
#define AGELENA_ERROR_5(msg, p1, p2, p3, p4, p5)								\
	Agelena::Logger::error((boost::format("%1%(%2%) : error :") % __FILE__ % __LINE__).str(), (boost::format(msg) % p1 % p2 % p3 % p4 % p5).str())
#define AGELENA_FATAL_ERROR_0(msg)													\
	Agelena::Logger::fatalError((boost::format("%1%(%2%) : fatal error :") % __FILE__ % __LINE__).str(), msg)
#define AGELENA_FATAL_ERROR_1(msg, p1)												\
	Agelena::Logger::fatalError((boost::format("%1%(%2%) : fatal error :") % __FILE__ % __LINE__).str(), (boost::format(msg) % p1).str())
#define AGELENA_FATAL_ERROR_2(msg, p1, p2)											\
	Agelena::Logger::fatalError((boost::format("%1%(%2%) : fatal error :") % __FILE__ % __LINE__).str(), (boost::format(msg) % p1 % p2).str())
#define AGELENA_FATAL_ERROR_3(msg, p1, p2, p3)										\
	Agelena::Logger::fatalError((boost::format("%1%(%2%) : fatal error :") % __FILE__ % __LINE__).str(), (boost::format(msg) % p1 % p2 % p3).str())
#define AGELENA_FATAL_ERROR_4(msg, p1, p2, p3, p4)									\
	Agelena::Logger::fatalError((boost::format("%1%(%2%) : fatal error :") % __FILE__ % __LINE__).str(), (boost::format(msg) % p1 % p2 % p3 % p4).str())
#define AGELENA_FATAL_ERROR_5(msg, p1, p2, p3, p4, p5)								\
	Agelena::Logger::fatalError((boost::format("%1%(%2%) : fatal error :") % __FILE__ % __LINE__).str(), (boost::format(msg) % p1 % p2 % p3 % p4 % p5).str())

}

#endif
