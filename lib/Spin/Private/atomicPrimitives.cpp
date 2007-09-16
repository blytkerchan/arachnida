#include "atomicPrimitives.h"
#include <boost/static_assert.hpp>
#if !defined(__GNUC__) && defined(_WIN32)
#	include <Windows.h>
#else if !defined (__GNUC__)
#	error "This file should not be compiled here"
#endif

namespace Spin
{
	namespace Private
	{
		void atomicIncrement(volatile boost::uint32_t & u32)
		{
#if !defined(__GNUC__) && defined(_WIN32)
			BOOST_STATIC_ASSERT(sizeof(volatile boost::uint32_t) == sizeof(volatile LONG));
			InterlockedIncrement((volatile LONG*)&u32);
#elif defined(__GNUC__)
			volatile boost::uint32_t * target(&u32);
			asm("lock incl (%1)"
				: "=m" (*target)
				: "r" (target) );
#endif
		}

		void atomicAdd(volatile boost::uint32_t & u32, boost::uint32_t val)
		{
#if !defined(__GNUC__) && defined(_WIN32)
			// get a pointer to the target
			volatile boost::uint32_t * target(&u32);
			// put the address of the target in ECX
			__asm mov ecx, DWORD PTR [target]
			__asm mov eax, val
			__asm lock add DWORD PTR [ecx], eax
#elif defined(__GNUC__)
			volatile boost::uint32_t * target(&u32);
			asm("lock add (%2), (%1)"
				: "=m" (*target)
				: "r" (val) );
#endif
		}

		SPIN_API boost::uint32_t fetchAndDecrement(volatile boost::uint32_t & u32)
		{
retry :
			volatile boost::uint32_t exp(u32);
			volatile boost::uint32_t * u32p(&u32);
			volatile boost::uint32_t * expp(&exp);
			volatile boost::uint32_t val(exp - 1);
#if !defined(__GNUC__) && defined(_WIN32)

			// put the address of the expected value in %eax
			__asm mov eax, DWORD PTR [expp]
			// put the expected value itself in %ebx
			__asm mov ebx, DWORD PTR [eax]
			// and move it to %eax where CMPXCHG expects to find it
			__asm mov eax, ebx
			// put the address of the target in %ecx
			__asm mov ecx, DWORD PTR [u32p]
			// and put the value we want to put there in %edx
			__asm mov edx, DWORD PTR [val]
			// now, if %eax == [%ecx] then [%ecx] := %edx; else %eax := [%ecx]
			__asm lock cmpxchg DWORD PTR [ecx], edx
			// zero flag is set on success
			__asm jz done
			// otherwise try again
			goto retry;
done :

			return exp;
#elif defined(__GNUC__)
			int rv;

			asm("movl %1, %%eax; lock cmpxchg %4, %3;jz __eq; movl %%eax, %1; movl $-1, %0; jmp __done; __eq: xor %0, %0; __done:"
				: "=r" (rv), "=r" (*(void**)exp)
				: "1" (*(void**)exp/*_ptr*/), "m" (*(void**)tar/*_ptr*/), "r" (val/*src_ptr*/)
				: "%eax");

			if (rv)
				goto retry;
			else
				return exp;
#endif
		}

		SPIN_API void atomicSet(volatile int & target, int value)
		{
			volatile int * targetp(&target);
#if !defined(__GNUC__) && defined(_WIN32)
			__asm mov eax, DWORD PTR [targetp]
			__asm mov ebx, value
			__asm lock xchg DWORD PTR [eax], ebx

#elif defined(__GNUC__)
			asm("lock xchgl %2, (%1)"
				: "=m" (targetp)
				: "1" (targetp), "r" (value)
				);
#endif
		}
	}
}

