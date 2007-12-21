#ifndef _scorpion_details_getpasswords_h
#define _scorpion_details_getpasswords_h

#include "prologue.h"
#include <string>

namespace Scorpion
{
	namespace Details
	{
		/** Interface to get passwords for encryption or decryption.
		 * If \b not used, the implementation will use OpenSSL's default to 
		 * get a password when needed - which requests the input from a console
		 * prompt. For servers, this can be a very bad idea as no-one will usually
		 * be there to answer.
		 * One way of implementing this interface is having the functions return
		 * either hard-coded (bad idea) or previously asked (better idea) passwords.
		 * If different passwords exist for different private keys or certificates,
		 * OpenSSL won't tell you for which one it is asking for a password, so it's
		 * basically up to you to know from the contents of the call. */
		class SCORPION_API GetPasswords
		{
		public :
			//! Called when a password is needed to decrypt something
			virtual std::string getDecryptionPassword(const std::size_t max_size) = 0;
			//! Called when a password is needed to encrypt something
			virtual std::string getEncryptionPassword(const std::size_t max_size) = 0;

		protected :
			virtual ~GetPasswords();
		};
	}
}

#endif
