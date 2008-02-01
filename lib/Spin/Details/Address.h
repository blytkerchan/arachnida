#ifndef _spin_details_address_h
#define _spin_details_address_h

#include "prologue.h"
#include <boost/cstdint.hpp>
#include <boost/static_assert.hpp>

namespace Spin
{
	namespace Details
	{
		/** Wrap an IPv4 address into something more easily usable.
		 * The purpose of this structure is to allow easy manipulation of
		 * IPv4 addresses, which usually need to be presented either as a
		 * network-byte-order (big-endian) 32-bit integer or four 8-bit 
		 * integers. Using this structure, we can throw around addresses in
		 * a portable way without asking ourselves whether the byte order is
		 * OK. */
		struct Address
		{
			//! Construct from a network-byte-order 32-bit integer
			Address(boost::uint32_t u32 = 0)
			{
				u_.u32_ = u32;
			}

			//! Construct from four 8-bit integers
			Address(boost::uint8_t u8_0, boost::uint8_t u8_1, boost::uint8_t u8_2, boost::uint8_t u8_3)
			{
				u_.u8_[0] = u8_0;
				u_.u8_[1] = u8_1;
				u_.u8_[2] = u8_2;
				u_.u8_[3] = u8_3;
			}

			//! The union between a network-byte-order 32-bit integer and four 8-bit integers
			union U_
			{
				boost::uint32_t u32_;	///< the network-byte-order 32-bit representation of the address
				boost::uint8_t u8_[4];	///< the four bytes that contain the IPv4 address
			} u_;
			BOOST_STATIC_ASSERT(sizeof(boost::uint32_t) == 4);
			BOOST_STATIC_ASSERT(sizeof(boost::uint8_t) == 1);
			BOOST_STATIC_ASSERT(sizeof(U_) == 4);
		};

		SPIN_API bool operator==(const Address & lhs, const Address & rhs);
		SPIN_API bool operator!=(const Address & lhs, const Address & rhs);
	}
}

#endif
