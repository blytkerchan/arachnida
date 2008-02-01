#include "SSL.h"
#include <boost/format.hpp>
extern "C"
{
#include <openssl/err.h>
}

namespace Scorpion
{
	namespace Exceptions
	{
		SSLProtocolError::~SSLProtocolError() throw()
		{
			delete[] what_;

			/* In some rare cases, when testing on Windows without network 
			 * connectivity using OpenSSL 0.9.8d, freeing this data here 
			 * causes a crash. I haven;t been able to quite put my finger on
			 * the "why" because that involved analysing the OpenSSL code, 
			 * for which I do not currently have the time. Neither have I been
			 * able to test this on a non-Windows machine because the only
			 * non-Windows machine I have has connectivity and that connectivity
			 * is needed for me to access it.
			 *
			 * The downside of this work-around is a memory leak, but should be
			 * a very small and very rare one. 
			 *
			 * Contributions (better fix, explanation, etc.) are welcome. */
			//if (error_line_data_ && (error_line_data_flags_ & ERR_TXT_MALLOCED))
			//	::OPENSSL_free((void*)error_line_data_);
			//else
			//{ /* no-op */ }
		}

		/*virtual */const char * SSLProtocolError::what() const throw()
		{
			if (!what_)
			{
				try
				{
					boost::format error_fmt("%1%: %2% (%3%): %4%");
					error_fmt
						% std::runtime_error::what()
						% filename_
						% line_
						% ((error_line_data_ && (error_line_data_flags_ & ERR_TXT_STRING)) ? error_line_data_ : "no further information")
						;
					std::string what(error_fmt.str());
					std::size_t what_size(what.size());
					what_ = new char[what_size + 1];
					std::copy(what.begin(), what.end(), what_);
					what_[what_size] = 0;
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

