#include "Address.h"

namespace Spin
{
	namespace Details
	{
		bool operator==(const Address & lhs, const Address & rhs)
		{
			return lhs.u_.u32_ == rhs.u_.u32_;
		}

		bool operator!=(const Address & lhs, const Address & rhs)
		{
			return lhs.u_.u32_ != rhs.u_.u32_;
		}
	}
}
