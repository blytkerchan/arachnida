#ifndef _agelena_details_prologue_h
#define _agelena_details_prologue_h

#ifndef AGELENA_API
#	if defined(_WIN32) && !defined(__CYGWIN__)
#		ifndef AGELENA_EXPORTS
#			define AGELENA_API __declspec(dllimport)
#		else
#			define AGELENA_API __declspec(dllexport)
#		endif
#	else
#		define AGELENA_API
#	endif
#endif

#endif
