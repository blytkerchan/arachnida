#ifndef _spin_private_tls_h
#define _spin_private_tls_h

namespace Spin
{
	namespace Private
	{
		class TLS
		{
		public :
			// this is a copiable opaque handle much in the same way as Windoze' HANDLE
			struct Key
			{ void * unused_ };

			//! Get the instance of the singleton
			static TLS & getInstance();
			//! Get a key to use with TLS - you only need this once and can use it from all of your threads
			Key acquireKey();
			void releaseKey(const Key & key);
			void setValue(const Key & key, void * value);
			void * getValue_p() const;

		private :
			TLS();
			~TLS();
		};
	}
}

#endif

