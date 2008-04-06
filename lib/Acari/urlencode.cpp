#include "urlencode.h"

namespace Acari
{
	ACARI_API std::string urlencode(const std::string & target, bool keep_slash/* = true*/)
	{
		std::string dest;
		for(std::string::size_type i(0); i < target.size(); ++i)
		{
			if(!(
				'0' <= target[i] && target[i] <= '9' ||
				'A' <= target[i] && target[i] <= 'Z' ||
				'a' <= target[i] && target[i] <= 'z' ||
				'$' == target[i] || '-' == target[i] ||
				'_' == target[i] || '.' == target[i] ||
				'+' == target[i] || '!' == target[i] ||
				'*' == target[i] || '\'' == target[i] ||
				'(' == target[i] || ')' == target[i] ||
				',' == target[i] || (keep_slash && target[i] == '/')))
			{
				char temp[4];
				sprintf(temp, "%%%02x", (unsigned char)target[i]);
				temp[sizeof(temp)-1] = '\0';
				dest.append(temp);
			}
			else
			{
				dest += target[i];
			}
		}
		return dest;
	}

	ACARI_API std::string urldecode(const std::string & target)
	{
		std::string result;
		for(std::string::const_iterator itr = target.begin(); itr != target.end(); ++itr)
		{
			if(*itr == '%')
			{
				if(itr + 1 != target.end() && itr + 2 != target.end() && isxdigit(*(itr + 1)) && isxdigit(*(itr + 2)))
				{
					char temp[3];
					temp[0] = *(itr+1);
					temp[1] = *(itr+2);
					temp[2] = '\0';
					result += strtol(temp, 0, 16);
					itr += 2;
				} else {
					result += *itr;
				}
			}
			else
			{
				result += *itr;
			}
		}
		return result;
	}
}

