#ifndef _spin_details_prologue_h
#define _spin_details_prologue_h

#ifndef SPIN_API
#	if defined(_WIN32) && !defined(__CYGWIN__)
#		ifndef SPIN_EXPORTS
#			define SPIN_API __declspec(dllimport)
#		else
#			define SPIN_API __declspec(dllexport)
#		endif
#	else
#		define SPIN_API
#	endif
#endif

#endif
