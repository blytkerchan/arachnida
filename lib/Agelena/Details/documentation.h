/** \mainpage Agelena: a generic logging API.
 * Agelena is a light-weight, extensible logging API designed for Spin and 
 * for general-purpose use. It is designed in such a way that you can choose 
 * at run-time (or at compile-time, if you wish) how you want to log the events
 * you want to log, by simply creating a different instance of a class derived
 * from the \link Agelena::Logger Logger \endlink class.
 *
 * Agelena also provides a few implementations of logging, derived from the 
 * \link Agelena::Logger Logger \endlink class.
 * 
 * To use Agelena, follow the following instructions
 * \li decide on whether you want to create your own logger or use one of the
 *     provided loggers.\n
 *     The provided loggers may or may not work for you, so there's a good chance
 *     you'll be rolling your own. Please consider contributing it back to the
 *     project!
 * \li create an instance of your logger sometime early during program execution.
 * \li use the \c AGELENA_* macros to log things.
 *
 * Here's a skeleton of a logger class:
 * \dontinclude Logger.cpp
 * \skip class Log
 * \until };
 * to use this logger, all you'd have to do is create an instance of it.
 */

//! The library's main namespace
namespace Agelena
{
	//! Namespace in which "default" loggers can be found
	namespace Loggers
	{ /* documentation only */ }
}
