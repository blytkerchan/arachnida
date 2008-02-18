/** \mainpage Damon: an HTTP client framework named for Damon diadema.
 * Damon implements an HTTP(S) version 1.1 client that you can embed
 * into your applications. With Damon, you can write an HTTP(S) client
 * in just two lines of code:
 * \dontinclude Request.cpp
 * \skip ::Damon::Request
 * \until ::Damon::Response
 */
//! The library's main namespace. Everything goes in here
namespace Damon
{
	//! implementation details client code needs to know about
	namespace Details
	{ /* documentation only */ }
}