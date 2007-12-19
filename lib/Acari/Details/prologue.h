#ifndef _acari_details_prologue_h
#define _acari_details_prologue_h

#ifndef ACARI_API
#	if defined(_WIN32) && !defined(__CYGWIN__)
#		ifndef ACARI_EXPORTS
#			define ACARI_API __declspec(dllimport)
#		else
#			define ACARI_API __declspec(dllexport)
#		endif
#	else
#		define ACARI_API
#	endif
#endif

#endif
