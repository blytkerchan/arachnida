#include "Attributes.h"
#include <cassert>
#include "atomicPrimitives.h"

namespace Acari
{
	/*static */volatile boost::uint32_t Attributes::next_attribute_index__(0);

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
		unsigned long retval(fetchAndIncrement(next_attribute_index__));
		if (retval >= max_attribute_count__)
			retval = 0xFFFFFFFF;
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

