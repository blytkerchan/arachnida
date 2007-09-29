#include "atomicPrimitives.h"
#include <boost/static_assert.hpp>
#if !defined(__GNUC__) && defined(_WIN32)
#	include <Windows.h>
#elif !defined (__GNUC__)
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
			asm("lock incl (%[input])"
				: [output] "=m" (u32)
				: [input] "r" (&u32));
#endif
		}

		void atomicAdd(volatile boost::uint32_t & u32, boost::uint32_t val)
		{
			volatile boost::uint32_t * target(&u32);
#if !defined(__GNUC__) && defined(_WIN32)
			// get a pointer to the target
			// put the address of the target in ECX
			__asm mov ecx, DWORD PTR [target]
			__asm mov eax, val
			__asm lock add DWORD PTR [ecx], eax
#elif defined(__GNUC__)
			asm("lock add %[input], %[output]"
				: [output] "=m" (*target)
				: "m" (&u32), [input] "r" (val) );
#endif
		}

		SPIN_API boost::uint32_t fetchAndIncrement(volatile boost::uint32_t & u32)
		{
retry :
			volatile boost::uint32_t exp(u32);
			volatile boost::uint32_t * u32p(&u32);
			volatile boost::uint32_t * expp(&exp);
			volatile boost::uint32_t val(exp + 1);
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

			asm("movl %[target], %%eax;\n\t"
				"lock cmpxchg %[source], %[target_location];\n\t"
				"jz 0f;\n\t"
				"movl %%eax, %[target];\n\t"
				"xorl %[retval], %[retval];\n\t"
				"jmp 1f;\n\t"
				"0: movl $1, %[retval];\n\t"
				"1:\n\t"
				: [retval] "=r" (rv), [target] "=r" (*(void**)exp)
				: "1" (*(void**)exp/*_ptr*/), [target_location] "m" (*(void**)expp/*_ptr*/), [source] "r" (val/*src_ptr*/)
				: "%eax");

			if (rv)
				goto retry;
			else
				return exp;
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

			asm("movl %[target], %%eax;\n\t"
				"lock cmpxchg %[source], %[target_location];\n\t"
				"jz 0f;\n\t"
				"movl %%eax, %[target];\n\t"
				"xorl %[retval], %[retval];\n\t"
				"jmp 1f;\n\t"
				"0: movl $1, %[retval];\n\t"
				"1:\n\t"
				: [retval] "=r" (rv), [target] "=r" (*(void**)exp)
				: "1" (*(void**)exp/*_ptr*/), [target_location] "m" (*(void**)expp/*_ptr*/), [source] "r" (val/*src_ptr*/)
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
			asm("lock xchgl %[value], %[output]"
				: [output] "=m" (*targetp)
				: [value] "r" (value)
				);
#endif
		}
	}
}

