#ifndef _spin_exceptions_socket_h
#define _spin_exceptions_socket_h

#include <stdexcept>

namespace Spin
{
	namespace Exceptions
	{
		//! Thrown when something goes wrong with a socket
		struct SocketError : std::runtime_error
		{
			enum { max_message_size__ = 64, max_function_size__ = 32 };

			SocketError(const char * message, const char * function, int error_code);

			SocketError(const SocketError &e)
				: std::runtime_error(e),
				  what_(e.what_)
			{
				e.what_ = 0;
			}

			/*virtual */const char * what() const throw();

			char message_[max_message_size__ + 1];
			char function_[max_function_size__ + 1];
			int error_code_;
			mutable char * what_;
		};
	}
}

#endif

