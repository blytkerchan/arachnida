/** \mainpage Scorpion: thinly wrapping OpenSSL for use in C++
 * Scorpion is a very thing wrapper around OpenSSL for use in C++, and principally
 * provides the \link Scorpion::Context Context \endlink class to allow you to 
 * set up security settings centrally and re-use them for all new connections. */
//! The library's main namespace
namespace Scorpion
{
	//! All exceptions thrown by this library go here
	namespace Exceptions
	{
		//! Exceptions related to SSL protocol errors go here
		namespace SSL
		{ /* doc only */ }
	}
}