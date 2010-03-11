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
		/** Error levels.
		 * Note that there's lots of space to add your own levels in between! */
		enum Level
		{
			debug__			= 0x00000000,
			info__			= 0x20000000,
			warning__		= 0x40000000,
			error__			= 0x60000000,
			fatal_error__	= 0x80000000,
		};

		//! Call this to add a message to the log
		static void add(
			unsigned int level,
			unsigned int component,
			unsigned int sub_component,
			const char * file,
			unsigned int line,
			const char * message,
			const unsigned char * aux = 0, unsigned long aux_size = 0) throw();

	protected :
		//! Returns the one known instance of the Logger. Called by the static methods of this class
		static Logger * getInstance();
		/** Set the one instance of the Logger.
		 * Normally called by the constructor of a derived class, which \em should check that getInstance returns 0 first. */
		static void setInstance(Logger * logger);
		//! add a log to the logger
		virtual void add_(unsigned int level, unsigned int component, unsigned int sub_component, const char * file, unsigned int line, const char * message, const unsigned char * aux, unsigned long aux_size) throw() = 0;

	private :
		static Logger * instance__;
	};

#ifndef AGELENA_NDEBUG
#define AGELENA_DEBUG_0_(component_id, sub_component_id, msg)							\
	Agelena::Logger::add((unsigned int)Agelena::Logger::debug__, component_id, sub_component_id, __FILE__, __LINE__, msg)
#define AGELENA_DEBUG_1_(component_id, sub_component_id, msg, p1)						\
	Agelena::Logger::add((unsigned int)Agelena::Logger::debug__, component_id, sub_component_id, __FILE__, __LINE__, (boost::format(msg) % p1).str().c_str())
#define AGELENA_DEBUG_2_(component_id, sub_component_id, msg, p1, p2)					\
	Agelena::Logger::add((unsigned int)Agelena::Logger::debug__, component_id, sub_component_id, __FILE__, __LINE__, (boost::format(msg) % p1 % p2).str().c_str())
#define AGELENA_DEBUG_3_(component_id, sub_component_id, msg, p1, p2, p3)				\
	Agelena::Logger::add((unsigned int)Agelena::Logger::debug__, component_id, sub_component_id, __FILE__, __LINE__, (boost::format(msg) % p1 % p2 % p3).str().c_str())
#define AGELENA_DEBUG_4_(component_id, sub_component_id, msg, p1, p2, p3, p4)			\
	Agelena::Logger::add((unsigned int)Agelena::Logger::debug__, component_id, sub_component_id, __FILE__, __LINE__, (boost::format(msg) % p1 % p2 % p3 % p4).str().c_str())
#define AGELENA_DEBUG_5_(component_id, sub_component_id, msg, p1, p2, p3, p4, p5)		\
	Agelena::Logger::add((unsigned int)Agelena::Logger::debug__, component_id, sub_component_id, __FILE__, __LINE__, (boost::format(msg) % p1 % p2 % p3 % p4 % p5).str().c_str())
#else
#define AGELENA_DEBUG_0_(component_id, sub_component_id, msg)							\
	(void)0
#define AGELENA_DEBUG_1_(component_id, sub_component_id, msg, p1)						\
	(void)0
#define AGELENA_DEBUG_2_(component_id, sub_component_id, msg, p1, p2)					\
	(void)0
#define AGELENA_DEBUG_3_(component_id, sub_component_id, msg, p1, p2, p3)				\
	(void)0
#define AGELENA_DEBUG_4_(component_id, sub_component_id, msg, p1, p2, p3, p4)			\
	(void)0
#define AGELENA_DEBUG_5_(component_id, sub_component_id, msg, p1, p2, p3, p4, p5)		\
	(void)0
#endif
#define AGELENA_INFO_0_(component_id, sub_component_id, msg)							\
	Agelena::Logger::add((unsigned int)Agelena::Logger::info__, component_id, sub_component_id, __FILE__, __LINE__, msg)
#define AGELENA_INFO_1_(component_id, sub_component_id, msg, p1)						\
	Agelena::Logger::add((unsigned int)Agelena::Logger::info__, component_id, sub_component_id, __FILE__, __LINE__, (boost::format(msg) % p1).str().c_str())
#define AGELENA_INFO_2_(component_id, sub_component_id, msg, p1, p2)					\
	Agelena::Logger::add((unsigned int)Agelena::Logger::info__, component_id, sub_component_id, __FILE__, __LINE__, (boost::format(msg) % p1 % p2).str().c_str())
#define AGELENA_INFO_3_(component_id, sub_component_id, msg, p1, p2, p3)				\
	Agelena::Logger::add((unsigned int)Agelena::Logger::info__, component_id, sub_component_id, __FILE__, __LINE__, (boost::format(msg) % p1 % p2 % p3).str().c_str())
#define AGELENA_INFO_4_(component_id, sub_component_id, msg, p1, p2, p3, p4)			\
	Agelena::Logger::add((unsigned int)Agelena::Logger::info__, component_id, sub_component_id, __FILE__, __LINE__, (boost::format(msg) % p1 % p2 % p3 % p4).str().c_str())
#define AGELENA_INFO_5_(component_id, sub_component_id, msg, p1, p2, p3, p4, p5)		\
	Agelena::Logger::add((unsigned int)Agelena::Logger::info__, component_id, sub_component_id, __FILE__, __LINE__, (boost::format(msg) % p1 % p2 % p3 % p4 % p5).str().c_str())
#define AGELENA_WARNING_0_(component_id, sub_component_id, msg)							\
	Agelena::Logger::add((unsigned int)Agelena::Logger::warning__, component_id, sub_component_id, __FILE__, __LINE__, msg)
#define AGELENA_WARNING_1_(component_id, sub_component_id, msg, p1)						\
	Agelena::Logger::add((unsigned int)Agelena::Logger::warning__, component_id, sub_component_id, __FILE__, __LINE__, (boost::format(msg) % p1).str().c_str())
#define AGELENA_WARNING_2_(component_id, sub_component_id, msg, p1, p2)					\
	Agelena::Logger::add((unsigned int)Agelena::Logger::warning__, component_id, sub_component_id, __FILE__, __LINE__, (boost::format(msg) % p1 % p2).str().c_str())
#define AGELENA_WARNING_3_(component_id, sub_component_id, msg, p1, p2, p3)				\
	Agelena::Logger::add((unsigned int)Agelena::Logger::warning__, component_id, sub_component_id, __FILE__, __LINE__, (boost::format(msg) % p1 % p2 % p3).str().c_str())
#define AGELENA_WARNING_4_(component_id, sub_component_id, msg, p1, p2, p3, p4)			\
	Agelena::Logger::add((unsigned int)Agelena::Logger::warning__, component_id, sub_component_id, __FILE__, __LINE__, (boost::format(msg) % p1 % p2 % p3 % p4).str().c_str())
#define AGELENA_WARNING_5_(component_id, sub_component_id, msg, p1, p2, p3, p4, p5)		\
	Agelena::Logger::add((unsigned int)Agelena::Logger::warning__, component_id, sub_component_id, __FILE__, __LINE__, (boost::format(msg) % p1 % p2 % p3 % p4 % p5).str().c_str())
#define AGELENA_ERROR_0_(component_id, sub_component_id, msg)							\
	Agelena::Logger::add((unsigned int)Agelena::Logger::error__, component_id, sub_component_id, __FILE__, __LINE__, msg)
#define AGELENA_ERROR_1_(component_id, sub_component_id, msg, p1)						\
	Agelena::Logger::add((unsigned int)Agelena::Logger::error__, component_id, sub_component_id, __FILE__, __LINE__, (boost::format(msg) % p1).str().c_str())
#define AGELENA_ERROR_2_(component_id, sub_component_id, msg, p1, p2)					\
	Agelena::Logger::add((unsigned int)Agelena::Logger::error__, component_id, sub_component_id, __FILE__, __LINE__, (boost::format(msg) % p1 % p2).str().c_str())
#define AGELENA_ERROR_3_(component_id, sub_component_id, msg, p1, p2, p3)				\
	Agelena::Logger::add((unsigned int)Agelena::Logger::error__, component_id, sub_component_id, __FILE__, __LINE__, (boost::format(msg) % p1 % p2 % p3).str().c_str())
#define AGELENA_ERROR_4_(component_id, sub_component_id, msg, p1, p2, p3, p4)			\
	Agelena::Logger::add((unsigned int)Agelena::Logger::error__, component_id, sub_component_id, __FILE__, __LINE__, (boost::format(msg) % p1 % p2 % p3 % p4).str().c_str())
#define AGELENA_ERROR_5_(component_id, sub_component_id, msg, p1, p2, p3, p4, p5)		\
	Agelena::Logger::add((unsigned int)Agelena::Logger::error__, component_id, sub_component_id, __FILE__, __LINE__, (boost::format(msg) % p1 % p2 % p3 % p4 % p5).str().c_str())
#define AGELENA_FATAL_ERROR_0_(component_id, sub_component_id, msg)						\
	Agelena::Logger::add((unsigned int)Agelena::Logger::fatal_error__, component_id, sub_component_id, __FILE__, __LINE__, msg)
#define AGELENA_FATAL_ERROR_1_(component_id, sub_component_id, msg, p1)					\
	Agelena::Logger::add((unsigned int)Agelena::Logger::fatal_error__, component_id, sub_component_id, __FILE__, __LINE__, (boost::format(msg) % p1).str().c_str())
#define AGELENA_FATAL_ERROR_2_(component_id, sub_component_id, msg, p1, p2)				\
	Agelena::Logger::add((unsigned int)Agelena::Logger::fatal_error__, component_id, sub_component_id, __FILE__, __LINE__, (boost::format(msg) % p1 % p2).str().c_str())
#define AGELENA_FATAL_ERROR_3_(component_id, sub_component_id, msg, p1, p2, p3)			\
	Agelena::Logger::add((unsigned int)Agelena::Logger::fatal_error__, component_id, sub_component_id, __FILE__, __LINE__, (boost::format(msg) % p1 % p2 % p3).str().c_str())
#define AGELENA_FATAL_ERROR_4_(component_id, sub_component_id, msg, p1, p2, p3, p4)		\
	Agelena::Logger::add((unsigned int)Agelena::Logger::fatal_error__, component_id, sub_component_id, __FILE__, __LINE__, (boost::format(msg) % p1 % p2 % p3 % p4).str().c_str())
#define AGELENA_FATAL_ERROR_5_(component_id, sub_component_id, msg, p1, p2, p3, p4, p5)	\
	Agelena::Logger::add((unsigned int)Agelena::Logger::fatal_error__, component_id, sub_component_id, __FILE__, __LINE__, (boost::format(msg) % p1 % p2 % p3 % p4 % p5).str().c_str())

}

#define AGELENA_DEBUG_0(msg) AGELENA_DEBUG_0_(AGELENA_COMPONENT_ID, AGELENA_SUBCOMPONENT_ID, msg)
#define AGELENA_DEBUG_1(msg, p1) AGELENA_DEBUG_1_(AGELENA_COMPONENT_ID, AGELENA_SUBCOMPONENT_ID, msg, p1)
#define AGELENA_DEBUG_2(msg, p1, p2) AGELENA_DEBUG_2_(AGELENA_COMPONENT_ID, AGELENA_SUBCOMPONENT_ID, msg, p1, p2)
#define AGELENA_DEBUG_3(msg, p1, p2, p3) AGELENA_DEBUG_3_(AGELENA_COMPONENT_ID, AGELENA_SUBCOMPONENT_ID, msg, p1, p2, p3)
#define AGELENA_DEBUG_4(msg, p1, p2, p3, p4) AGELENA_DEBUG_4_(AGELENA_COMPONENT_ID, AGELENA_SUBCOMPONENT_ID, msg, p1, p2, p3, p4)
#define AGELENA_DEBUG_5(msg, p1, p2, p3, p4, p5) AGELENA_DEBUG_5_(AGELENA_COMPONENT_ID, AGELENA_SUBCOMPONENT_ID, msg, p1, p2, p3, p4, p5)
#define AGELENA_INFO_0(msg) AGELENA_INFO_0_(AGELENA_COMPONENT_ID, AGELENA_SUBCOMPONENT_ID, msg)
#define AGELENA_INFO_1(msg, p1) AGELENA_INFO_1_(AGELENA_COMPONENT_ID, AGELENA_SUBCOMPONENT_ID, msg, p1)
#define AGELENA_INFO_2(msg, p1, p2) AGELENA_INFO_2_(AGELENA_COMPONENT_ID, AGELENA_SUBCOMPONENT_ID, msg, p1, p2)
#define AGELENA_INFO_3(msg, p1, p2, p3) AGELENA_INFO_3_(AGELENA_COMPONENT_ID, AGELENA_SUBCOMPONENT_ID, msg, p1, p2, p3)
#define AGELENA_INFO_4(msg, p1, p2, p3, p4) AGELENA_INFO_4_(AGELENA_COMPONENT_ID, AGELENA_SUBCOMPONENT_ID, msg, p1, p2, p3, p4)
#define AGELENA_INFO_5(msg, p1, p2, p3, p4, p5) AGELENA_INFO_5_(AGELENA_COMPONENT_ID, AGELENA_SUBCOMPONENT_ID, msg, p1, p2, p3, p4, p5)
#define AGELENA_WARNING_0(msg) AGELENA_WARNING_0_(AGELENA_COMPONENT_ID, AGELENA_SUBCOMPONENT_ID, msg)
#define AGELENA_WARNING_1(msg, p1) AGELENA_WARNING_1_(AGELENA_COMPONENT_ID, AGELENA_SUBCOMPONENT_ID, msg, p1)
#define AGELENA_WARNING_2(msg, p1, p2) AGELENA_WARNING_2_(AGELENA_COMPONENT_ID, AGELENA_SUBCOMPONENT_ID, msg, p1, p2)
#define AGELENA_WARNING_3(msg, p1, p2, p3) AGELENA_WARNING_3_(AGELENA_COMPONENT_ID, AGELENA_SUBCOMPONENT_ID, msg, p1, p2, p3)
#define AGELENA_WARNING_4(msg, p1, p2, p3, p4) AGELENA_WARNING_4_(AGELENA_COMPONENT_ID, AGELENA_SUBCOMPONENT_ID, msg, p1, p2, p3, p4)
#define AGELENA_WARNING_5(msg, p1, p2, p3, p4, p5) AGELENA_WARNING_5_(AGELENA_COMPONENT_ID, AGELENA_SUBCOMPONENT_ID, msg, p1, p2, p3, p4, p5)
#define AGELENA_ERROR_0(msg) AGELENA_ERROR_0_(AGELENA_COMPONENT_ID, AGELENA_SUBCOMPONENT_ID, msg)
#define AGELENA_ERROR_1(msg, p1) AGELENA_ERROR_1_(AGELENA_COMPONENT_ID, AGELENA_SUBCOMPONENT_ID, msg, p1)
#define AGELENA_ERROR_2(msg, p1, p2) AGELENA_ERROR_2_(AGELENA_COMPONENT_ID, AGELENA_SUBCOMPONENT_ID, msg, p1, p2)
#define AGELENA_ERROR_3(msg, p1, p2, p3) AGELENA_ERROR_3_(AGELENA_COMPONENT_ID, AGELENA_SUBCOMPONENT_ID, msg, p1, p2, p3)
#define AGELENA_ERROR_4(msg, p1, p2, p3, p4) AGELENA_ERROR_4_(AGELENA_COMPONENT_ID, AGELENA_SUBCOMPONENT_ID, msg, p1, p2, p3, p4)
#define AGELENA_ERROR_5(msg, p1, p2, p3, p4, p5) AGELENA_ERROR_5_(AGELENA_COMPONENT_ID, AGELENA_SUBCOMPONENT_ID, msg, p1, p2, p3, p4, p5)
#define AGELENA_FATAL_ERROR_0(msg) AGELENA_FATAL_ERROR_0_(AGELENA_COMPONENT_ID, AGELENA_SUBCOMPONENT_ID, msg)
#define AGELENA_FATAL_ERROR_1(msg, p1) AGELENA_FATAL_ERROR_1_(AGELENA_COMPONENT_ID, AGELENA_SUBCOMPONENT_ID, msg, p1)
#define AGELENA_FATAL_ERROR_2(msg, p1, p2) AGELENA_FATAL_ERROR_2_(AGELENA_COMPONENT_ID, AGELENA_SUBCOMPONENT_ID, msg, p1, p2)
#define AGELENA_FATAL_ERROR_3(msg, p1, p2, p3) AGELENA_FATAL_ERROR_3_(AGELENA_COMPONENT_ID, AGELENA_SUBCOMPONENT_ID, msg, p1, p2, p3)
#define AGELENA_FATAL_ERROR_4(msg, p1, p2, p3, p4) AGELENA_FATAL_ERROR_4_(AGELENA_COMPONENT_ID, AGELENA_SUBCOMPONENT_ID, msg, p1, p2, p3, p4)
#define AGELENA_FATAL_ERROR_5(msg, p1, p2, p3, p4, p5) AGELENA_FATAL_ERROR_5_(AGELENA_COMPONENT_ID, AGELENA_SUBCOMPONENT_ID, msg, p1, p2, p3, p4, p5)

#endif
