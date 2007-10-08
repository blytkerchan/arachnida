#ifndef _spin_details_address_h
#define _spin_details_address_h

#include <boost/cstdint.hpp>
#include <boost/static_assert.hpp>

namespace Spin
{
	namespace Details
	{
		struct Address
		{
			Address(boost::uint32_t u32)
			{
				u_.u32_ = u32;
			}

			Address(boost::uint8_t u8_0, boost::uint8_t u8_1, boost::uint8_t u8_2, boost::uint8_t u8_3)
			{
				u_.u8_[0] = u8_0;
				u_.u8_[1] = u8_1;
				u_.u8_[2] = u8_2;
				u_.u8_[3] = u8_3;
			}

			BOOST_STATIC_ASSERT(sizeof(boost::uint32_t) == 4);
			BOOST_STATIC_ASSERT(sizeof(boost::uint8_t) == 1);
			union U_
			{
				boost::uint32_t u32_;
				boost::uint8_t u8_[4];
			} u_;
			BOOST_STATIC_ASSERT(sizeof(U_) == 4);
		};
	}
}

#endif
