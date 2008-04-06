#ifndef acari_urlencode_h
#define acari_urlencode_h

#include "Details/prologue.h"
#include <string>

namespace Acari
{
	ACARI_API std::string urlencode(const std::string & url, bool keep_slash = true);
	ACARI_API std::string urldecode(const std::string & url);
}

#endif
