#ifndef acari_base64encode_h
#define acari_base64encode_h

#include "Details/prologue.h"
#include <string>

namespace Acari
{
	//! Encode a buffer to base 64 encoding string.
	ACARI_API std::string base64encode(const void * data, size_t length);
	ACARI_API std::string base64encode(const std::string & data);

	//! Decode a base64 string to it's original representation
	ACARI_API std::string base64decode(const char * base64_data, size_t length);
	ACARI_API std::string base64decode(const std::string & data);
}

#endif
