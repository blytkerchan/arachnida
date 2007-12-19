#ifndef _scorpion_details_prologue_h
#define _scorpion_details_prologue_h

#ifndef SCORPION_API
#	if defined(_WIN32) && !defined(__CYGWIN__)
#		ifndef SCORPION_EXPORTS
#			define SCORPION_API __declspec(dllimport)
#		else
#			define SCORPION_API __declspec(dllexport)
#		endif
#	else
#		define SCORPION_API
#	endif
#endif

#endif
