#include "yield.h"
#include <Windows.h>

namespace Spin
{
	namespace Private
	{
		void yield(bool to_lower/* = false*/)
		{
			Sleep(to_lower ? 1 : 0);
		}
	}
}

