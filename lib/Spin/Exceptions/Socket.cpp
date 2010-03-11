#include "Socket.h"
#include <cassert>
#include <cstring>
#include <boost/format.hpp>
#if defined(_WIN32) && ! defined(__CYGWIN__)
#include <Windows.h>
#else
#include <cerrno>
#endif

namespace Spin
{
	namespace Exceptions
	{
#define CASE_ERROR_MESSAGE(code, message)											\
	case code :																		\
	{																				\
		static const char * msg_##code##__ = message;								\
		error_text = message;														\
		break;																		\
	}
#define CASE_DEFAULT_ERROR_MESSAGE(message)											\
	default :																		\
	{																				\
		static const char * msg_default__ = message;								\
		error_text = message;														\
	}

		SocketError::SocketError(const char * message, const char * function, int error_code)
			: std::runtime_error("Socket error."),
			  error_code_(error_code),
			  what_(0)
		{
			std::size_t message_size(std::strlen(message));
			message_size = message_size > max_message_size__ ? max_message_size__ : message_size;
			char * where(std::copy(message, message + message_size, message_));
			assert(std::distance(message_, where) == (std::iterator_traits< char* /* type of where */ >::difference_type)message_size);
			*where = 0;
			std::size_t function_size(std::strlen(function));
			function_size = function_size > max_function_size__ ? max_function_size__ : function_size;
			where = std::copy(function, function + function_size, function_);
			assert(std::distance(function_, where) == (std::iterator_traits< char* /* type of where */ >::difference_type)function_size);
			*where = 0;
		}

		/*virtual */const char * SocketError::what() const throw()
		{
			if (!what_)
			{
				try
				{
					const char * error_text(0);
					switch (error_code_)
					{
#if defined(_WIN32) && ! defined(__CYGWIN__)
						CASE_ERROR_MESSAGE(WSAEACCES, "The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt with the SO_BROADCAST socket option to enable use of the broadcast address.");
						CASE_ERROR_MESSAGE(WSAEADDRINUSE, "A process on the computer is already bound to the same fully-qualified address and the socket has not been marked to allow address reuse with SO_REUSEADDR. For example, the IP address and port are bound in the af_inet case). (See the SO_REUSEADDR socket option under setsockopt.)");
						CASE_ERROR_MESSAGE(WSAEADDRNOTAVAIL, "The specified address is not a valid address for this computer.");
						CASE_ERROR_MESSAGE(WSAECONNABORTED, "The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable.");
						CASE_ERROR_MESSAGE(WSAECONNRESET, "The virtual circuit was reset by the remote side executing a hard or abortive close. For UPD sockets, the remote host was unable to deliver a previously sent UDP datagram and responded with a \"Port Unreachable\" ICMP packet. The application should close the socket as it is no longer usable.");
						CASE_ERROR_MESSAGE(WSAEFAULT, "The name or namelen parameter is not a valid part of the user address space, the namelen parameter is too small, the name parameter contains an incorrect address format for the associated address family, or the first two bytes of the memory block specified by name does not match the address family associated with the socket descriptors.");
						CASE_ERROR_MESSAGE(WSAEHOSTUNREACH, "The remote host cannot be reached from this host at this time. ");
						CASE_ERROR_MESSAGE(WSAEINPROGRESS, "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.");
						CASE_ERROR_MESSAGE(WSAEINTR, "A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall.");
						CASE_ERROR_MESSAGE(WSAEINVAL, "The socket is already bound to an address.");
						CASE_ERROR_MESSAGE(WSAEMSGSIZE, "The socket is message oriented, and the message is larger than the maximum supported by the underlying transport.");
						CASE_ERROR_MESSAGE(WSAENETDOWN, "The network subsystem has failed.");
						CASE_ERROR_MESSAGE(WSAENETRESET, "The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress.");
						CASE_ERROR_MESSAGE(WSAENOBUFS, "Not enough buffers available, too many connections.");
						CASE_ERROR_MESSAGE(WSAENOTCONN, "The socket is not connected.");
						CASE_ERROR_MESSAGE(WSAENOTSOCK, "The descriptor is not a socket.");
						CASE_ERROR_MESSAGE(WSAEOPNOTSUPP, "MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only receive operations.");
						CASE_ERROR_MESSAGE(WSAESHUTDOWN, "The socket has been shut down; it is not possible to send on a socket after shutdown has been invoked with how set to SD_SEND or SD_BOTH.");
						CASE_ERROR_MESSAGE(WSAETIMEDOUT, "The connection has been dropped, because of a network failure or because the system on the other end went down without notice.");
						CASE_ERROR_MESSAGE(WSAEWOULDBLOCK, "The socket is marked as nonblocking and the requested operation would block.");
						CASE_ERROR_MESSAGE(WSANOTINITIALISED, "A successful WSAStartup call must occur before using this function.");
						CASE_ERROR_MESSAGE(WSAEAFNOSUPPORT, "An address incompatible with the requested protocol was used.");
#else
						CASE_ERROR_MESSAGE(EAFNOSUPPORT, "The implementation does not support the specified address family.");
						CASE_ERROR_MESSAGE(EMFILE, "No more file descriptors are available for this process.");
						CASE_ERROR_MESSAGE(ENFILE, "No more file descriptors are available for the system.");
						CASE_ERROR_MESSAGE(EPROTONOSUPPORT, "The protocol is not supported by the address family, or the protocol is not supported by the implementation.");
						CASE_ERROR_MESSAGE(EPROTOTYPE, "The socket type is not supported by the protocol.");
						CASE_ERROR_MESSAGE(EACCES, "The process does not have appropriate privileges.");
						CASE_ERROR_MESSAGE(ENOBUFS, "Insufficient resources were available in the system to perform the operation.");
						CASE_ERROR_MESSAGE(ENOMEM, "Insufficient memory was available to fulfill the request.");
#endif
						CASE_DEFAULT_ERROR_MESSAGE("Unknown error.");
					}

					boost::format error_text_fmt("Socket Error: %1%: %2% returned with %3% (%4%)");
					error_text_fmt 
						% message_ 
						% function_ 
						% error_code_ 
						% error_text
						;
					what_ = strdup(error_text_fmt.str().c_str());
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


