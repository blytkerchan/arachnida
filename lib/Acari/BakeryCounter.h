#ifndef _acari_bakerycounter_h
#define _acari_bakerycounter_h

#include "Details/prologue.h"
#include <boost/cstdint.hpp>
#include <boost/static_assert.hpp>
#include <boost/thread/mutex.hpp>

namespace Acari
{
	class ACARI_API BakeryCounter
	{
	public :
		BakeryCounter();
		~BakeryCounter();

		boost::uint16_t getClientCounter() const { Counter_ counter(counter_); return counter.u16_[client_counter__]; }
		boost::uint16_t getBakerCounter() const { Counter_ counter(counter_); return counter.u16_[baker_counter__]; }
		void incrementClientCounter();
		void incrementBakerCounter();

	private :
		// Neither CopyConstructible nor Assignable
		BakeryCounter(const BakeryCounter &);
		BakeryCounter & operator=(const BakeryCounter &);

		enum { client_counter__ = 0, baker_counter__ = 1 };
		union Counter_
		{
			// volatile shouldn't be necessary, but won't hurt
			volatile boost::uint16_t u16_[2];
			volatile boost::uint32_t u32_;
		};
		BOOST_STATIC_ASSERT(sizeof(Counter_) == sizeof(boost::uint32_t));

		Counter_ counter_;
	};
}

#endif