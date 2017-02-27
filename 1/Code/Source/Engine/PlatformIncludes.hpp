#pragma once

#if defined(_WIN32) && !defined(WIN32)
#	define WIN32
#endif

#ifdef WIN32
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#	include <Windows.h>
#	undef CreateWindow
#	undef GetObject
#	undef min
#	undef max
#else
#	error unknown platform
#endif
