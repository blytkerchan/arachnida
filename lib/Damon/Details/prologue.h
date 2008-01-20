#ifndef _damon_details_prologue_h
#define _damon_details_prologue_h

#ifndef DAMON_API
#	if defined(_WIN32) && !defined(__CYGWIN__)
#		ifndef DAMON_EXPORTS
#			define DAMON_API __declspec(dllimport)
#		else
#			define DAMON_API __declspec(dllexport)
#		endif
#	else
#		define DAMON_API
#	endif
#endif

#endif
