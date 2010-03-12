#ifndef _acari_attributes_h
#define _acari_attributes_h

#include "Details/prologue.h"
#include <vector>
#include <boost/any.hpp>
#include <boost/cstdint.hpp>
#include <Vlinder/Atomics/Atomics.h>

namespace Acari
{
	/** Base class for a class to which you want to be able to attach anonymous attributes.
	 * Some features in the Spin library require functions to attach arbitrary
	 * information to connections - much in the same way as the standard IOStream 
	 * library attaches attributes with the \c xalloc function. Any class that 
	 * derives from this class allows the users of instances of that class to do
	 * so in a non-intrusive manner. */
	class ACARI_API Attributes
	{
	public :
		enum {
			max_attribute_count__ = 8 ///< the maximum amount of attributes attached to any instance.
		};

		//! Default constructor - does the obvious
		Attributes();
		//! Copy constructor - does the obvious as a value-type
		Attributes(const Attributes & attributes);
		//! Destructor - does the obvious
		~Attributes();

		/** Allocate an attribute identifier.
		 * Much like I/O streams, you can add attributes to connections by allocating an 
		 * identifier (globally, statically) and subsequently using that identifier 
		 * with the getAttribute method. Once you've allocated an identifier on one
		 * Connection object, it is valid for all Connection objects (so you do not need
		 * to allocate a new one each time you want to use an attribute for the same purpose
		 * on a different connection).
		 *
		 * \note Don't simply pick an integer value at random and use it! */
		static unsigned long allocateAttribute();
		/** Get an attribute with a previously allocated identifier, obtained from allocateAttribute.
		 * The attribute returned will be a newly allocated boost::any by default */
		boost::any & getAttribute(unsigned long index);

	private :
		std::vector< boost::any > attributes_;
		static Vlinder::Atomics::Atomics::Atomic< boost::uint32_t > next_attribute_index__;
	};
}

#endif
