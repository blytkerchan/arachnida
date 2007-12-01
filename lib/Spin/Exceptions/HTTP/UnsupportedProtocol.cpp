#include "../HTTP.h"
#include <cstring>

namespace Spin
{
	namespace Exceptions
	{
		namespace HTTP
		{
			/*virtual */const char * UnsupportedProtocol::what() const throw()
			{
				if (!what_)
				{
					try
					{
						static const char * error_text__ = "Unsupported protocol: ";
						static const std::size_t error_text_size__ = std::strlen(error_text__);
						what_ = new char[error_text_size__ + max_protocol_size__];
						char * where(std::copy(error_text__, error_text__ + error_text_size__, what_));
						std::copy(protocol_, protocol_ + max_protocol_size__, where);
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

