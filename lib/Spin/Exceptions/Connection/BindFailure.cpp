#include "../Connection.h"
#include <cassert>
#include <cstring>
#include <Windows.h>

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

			/*virtual */const char * BindFailure::what() const throw()
			{
				if (!what_)
				{
					try
					{
						static const char * error_text__ = "Bind failure: ";
						static const std::size_t error_text_size__ = std::strlen(error_text__);

						const char * error_text(0);
						std::size_t error_text_size(0);
						switch (error_code_)
						{
						CASE_ERROR_MESSAGE(WSANOTINITIALISED, "A successful WSAStartup call must occur before using this function.");
						CASE_ERROR_MESSAGE(WSAENETDOWN, "The network subsystem has failed.");
						CASE_ERROR_MESSAGE(WSAEACCES, "Attempt to connect datagram socket to broadcast address failed because setsockopt option SO_BROADCAST is not enabled.");
						CASE_ERROR_MESSAGE(WSAEADDRINUSE, "A process on the computer is already bound to the same fully-qualified address and the socket has not been marked to allow address reuse with SO_REUSEADDR. For example, the IP address and port are bound in the af_inet case). (See the SO_REUSEADDR socket option under setsockopt.)");
						CASE_ERROR_MESSAGE(WSAEADDRNOTAVAIL, "The specified address is not a valid address for this computer.");
						CASE_ERROR_MESSAGE(WSAEFAULT, "The name or namelen parameter is not a valid part of the user address space, the namelen parameter is too small, the name parameter contains an incorrect address format for the associated address family, or the first two bytes of the memory block specified by name does not match the address family associated with the socket descriptors.");
						CASE_ERROR_MESSAGE(WSAEINPROGRESS, "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.");
						CASE_ERROR_MESSAGE(WSAEINVAL, "The socket is already bound to an address.");
						CASE_ERROR_MESSAGE(WSAENOBUFS, "Not enough buffers available, too many connections.");
						CASE_ERROR_MESSAGE(WSAENOTSOCK, "The descriptor is not a socket.");
						CASE_DEFAULT_ERROR_MESSAGE("Unknown error.");
						}

						what_ = new char[error_text_size__ + error_text_size + 1];
						char * where(std::copy(error_text__, error_text__ + error_text_size__, what_));
						std::copy(error_text, error_text + error_text_size, where);
						assert(std::distance(what_, where) == error_text_size__ + error_text_size);
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

