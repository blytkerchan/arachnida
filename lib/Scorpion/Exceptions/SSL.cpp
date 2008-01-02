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
			if (error_line_data_ && (error_line_data_flags_ & ERR_TXT_MALLOCED))
				::OPENSSL_free((void*)error_line_data_);
			else
			{ /* no-op */ }
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

