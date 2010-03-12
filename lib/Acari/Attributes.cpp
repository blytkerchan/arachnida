#include "Attributes.h"
#include <cassert>

namespace Acari
{
	/*static */Vlinder::Atomics::Atomics::Atomic< boost::uint32_t > Attributes::next_attribute_index__(0);

	Attributes::Attributes()
		: attributes_(max_attribute_count__)
	{ /* no-op */ }

	Attributes::Attributes(const Attributes & attributes)
		: attributes_(attributes.attributes_)
	{ /* no-op */ }

	Attributes::~Attributes()
	{ /* no-op */ }

	/*static */unsigned long Attributes::allocateAttribute()
	{
		unsigned long retval(next_attribute_index__.fetchAndAdd($, 1, Vlinder::Atomics::Atomics::memory_order_relaxed__));
		if (retval >= max_attribute_count__)
			throw std::bad_alloc();
		else
		{ /* attributes not exhausted */ }
		return retval;
	}

	boost::any & Attributes::getAttribute(unsigned long index)
	{
		assert(index < max_attribute_count__);
		return attributes_[index];
	}
}

