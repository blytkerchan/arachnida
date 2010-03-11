#include "../Connection.h"
#include <cassert>
#include <cstring>
#if defined(_WIN32) && ! defined(__CYGWIN__)
#include <Windows.h>
#else
#include <cerrno>
#endif

namespace Spin
{
	namespace Exceptions
	{
		namespace Connection
		{
#define CASE_ERROR_MESSAGE(code, message)											\
	case code :																		\
	{																				\
		static const char * msg_##code##__ = message;								\
		static const std::size_t msg_##code##_size__ = std::strlen(message);		\
		error_text = message;														\
		error_text_size = msg_##code##_size__;										\
	}
#define CASE_DEFAULT_ERROR_MESSAGE(message)											\
	default :																		\
	{																				\
		static const char * msg_default__ = message;								\
		static const std::size_t msg_default_size__ = std::strlen(message);			\
		error_text = message;														\
		error_text_size = msg_default_size__;										\
	}

			/*virtual */const char * ListenFailure::what() const throw()
			{
				if (!what_)
				{
					try
					{
						static const char * error_text__ = "Listen failure: ";
						static const std::size_t error_text_size__ = std::strlen(error_text__);

						const char * error_text(0);
						std::size_t error_text_size(0);
						switch (error_code_)
						{
#if defined(_WIN32) && ! defined(__CYGWIN__)
						CASE_ERROR_MESSAGE(WSANOTINITIALISED, "A successful WSAStartup call must occur before using this function.");
						CASE_ERROR_MESSAGE(WSAENETDOWN, "The network subsystem has failed.");
						CASE_ERROR_MESSAGE(WSAEADDRINUSE, "The socket's local address is already in use and the socket was not marked to allow address reuse with SO_REUSEADDR. This error usually occurs during execution of the bind function, but could be delayed until this function if the bind was to a partially wildcard address (involving ADDR_ANY) and if a specific address needs to be committed at the time of this function.");
						CASE_ERROR_MESSAGE(WSAEINPROGRESS, "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.");
						CASE_ERROR_MESSAGE(WSAEINVAL, "The socket has not been bound with bind.");
						CASE_ERROR_MESSAGE(WSAEISCONN, "The socket is already connected.");
						CASE_ERROR_MESSAGE(WSAEMFILE, "No more socket descriptors are available.");
						CASE_ERROR_MESSAGE(WSAENOBUFS, "No buffer space is available.");
						CASE_ERROR_MESSAGE(WSAENOTSOCK, "The descriptor is not a socket.");
						CASE_ERROR_MESSAGE(WSAEOPNOTSUPP, "The referenced socket is not of a type that supports the listen operation.");
#else
						CASE_ERROR_MESSAGE(EBADF, "The socket argument is not a valid file descriptor.");
						CASE_ERROR_MESSAGE(EDESTADDRREQ, "The socket is not bound to a local address, and the protocol does not support listening on an unbound socket.");
						CASE_ERROR_MESSAGE(EINVAL, "The socket is either already connected or has been shut down.");
						CASE_ERROR_MESSAGE(ENOTSOCK, "The socket argument does not refer to a socket.");
						CASE_ERROR_MESSAGE(EOPNOTSUPP, "The socket protocol does not support listen().");
						CASE_ERROR_MESSAGE(EACCES, "The calling process does not have the appropriate privileges.");
						CASE_ERROR_MESSAGE(ENOBUFS, "Insufficient resources are available in the system to complete the call.");
#endif
						CASE_DEFAULT_ERROR_MESSAGE("Unknown error.");
						}

						what_ = new char[error_text_size__ + error_text_size + 1];
						char * where(std::copy(error_text__, error_text__ + error_text_size__, what_));
						std::copy(error_text, error_text + error_text_size, where);
						assert(std::distance(what_, where) == (std::iterator_traits< char* /* type of where */ >::difference_type)(error_text_size__ + error_text_size));
						*where = 0;
					}
					catch(...)
					{
						return std::runtime_error::what();
					}
				}
				else
				{ /* already have a what_ */ }
				return what_;
			}
		}
	}
}

