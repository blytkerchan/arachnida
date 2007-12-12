#ifndef _spin_exceptions_ssl_h
#define _spin_exceptions_ssl_h

#include <stdexcept>

namespace Spin
{
	namespace Exceptions
	{
		struct SSLProtocolError : std::runtime_error
		{
			SSLProtocolError(const char * what, int error_code, const char * filename, int line, const char * error_line_data, int error_line_data_flags)
				: std::runtime_error(what),
				  error_code_(error_code),
				  filename_(filename),
				  line_(line),
				  error_line_data_(error_line_data),
				  error_line_data_flags_(error_line_data_flags),
				  what_(0)
			{ /* no-op */ }

			SSLProtocolError(const SSLProtocolError &e)
				: std::runtime_error(e),
				  error_code_(e.error_code_),
				  filename_(e.filename_),
				  line_(e.line_),
				  error_line_data_(e.error_line_data_),
				  error_line_data_flags_(e.error_line_data_flags_),
				  what_(e.what_)
			{
				e.what_ = 0;
				e.error_line_data_ = 0;
			}

			~SSLProtocolError();

			SSLProtocolError & operator=(SSLProtocolError e)
			{
				return swap(e);
			}

			SSLProtocolError & swap(SSLProtocolError & e) throw()
			{
#define SWAP(TYPE, VAR)	\
	TYPE VAR(VAR##_);	\
	VAR##_ = e.VAR##_;	\
	e.VAR##_ = VAR

				SWAP(int, error_code);
				SWAP(const char *, filename);
				SWAP(int, line);
				SWAP(const char *, error_line_data);
				SWAP(int, error_line_data_flags);
				SWAP(char *, what);

#undef SWAP

				return *this;
			}

			virtual const char * what() throw();

			int error_code_;
			const char * filename_;
			int line_;
			mutable const char * error_line_data_;
			int error_line_data_flags_;
			mutable char * what_;
		};
	}
}

#include "SSL/AcceptError.h"
#include "SSL/AcceptSetupError.h"
#include "SSL/AcceptSocketAllocationError.h"
#include "SSL/ContextAllocationError.h"
#include "SSL/ConnectionSocketCreationFailure.h"
#include "SSL/ServerSocketAllocationError.h"
#include "SSL/SSLContextAllocationError.h"
#include "SSL/SSLContextSetupError.h"
#include "SSL/SSLPointerLocationError.h"
#include "SSL/ConnectionError.h"
#include "SSL/HandshakeError.h"
#endif

