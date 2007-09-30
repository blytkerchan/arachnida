#ifndef _spin_private_pipe_h
#define _spin_private_pipe_h

#include "../Details/prologue.h"
#include <cstddef>

namespace Spin
{
	namespace Private
	{
		class SPIN_API Pipe
		{
		public :
			Pipe();
			~Pipe();

			int getReadDescriptor() const { return fds_[0]; }
			int getWriteDescriptor() const { return fds_[1]; }

			void read(void * buffer, std::size_t count);
			void write(const void * buffer, std::size_t count);

		private :
			Pipe(const Pipe&);
			Pipe & operator=(const Pipe&);

			int fds_[2];
		};
	}
}

#endif
