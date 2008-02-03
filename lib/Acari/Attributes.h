#ifndef _acari_attributes_h
#define _acari_attributes_h

#include "Details/prologue.h"
#include <vector>
#include <boost/any.hpp>
#include <boost/cstdint.hpp>

namespace Acari
{
	class ACARI_API Attributes
	{
	public :
		enum { max_attribute_count__ = 8 };

		Attributes();
		Attributes(const Attributes & attributes);
		~Attributes();

		/** Allocate an attribute identifier.
		 * Much like I/O streams, you can add attributes to connections by allocating an 
		 * identifier (globally, statically) and subsequently using that identifier 
		 * with the getAttribute method. Once you've allocated an identifier on one
		 * Connection object, it is valid for all Connection objects (so you do not need
		 * to allocate a new one each time you want to use an attribute for the same purpose
		 * on a different connection).
		 *
		 * Internally, the attributes are also used by the library (so don't simply pick an
		 * integer value at random and use it!) */
		static unsigned long allocateAttribute();
		/** Get an attribute with a previously allocated identifier, obtained from allocateAttribute.
		 * The attribute returned will be a newly allocated boost::any by default */
		boost::any & getAttribute(unsigned long index);

	private :
		std::vector< boost::any > attributes_;
		static volatile boost::uint32_t next_attribute_index__;
	};
}

#endif
