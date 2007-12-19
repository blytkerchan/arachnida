#ifndef _scorpion_details_getpasswords_h
#define _scorpion_details_getpasswords_h

#include "prologue.h"
#include <string>

namespace Scorpion
{
	namespace Details
	{
		class SCORPION_API GetPasswords
		{
		public :
			virtual std::string getDecryptionPassword(const std::size_t max_size) = 0;
			virtual std::string getEncryptionPassword(const std::size_t max_size) = 0;

		protected :
			virtual ~GetPasswords();
		};
	}
}

#endif
