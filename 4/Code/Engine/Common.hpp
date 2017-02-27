#pragma once

//----------------------------------------------------------------------------//
// Compiler and Platform
//----------------------------------------------------------------------------//

#define _QUOTE( x ) #x
#define _QUOTE_IN_PLACE( x ) _QUOTE( x )
#define __FILELINE__ __FILE__"(" _QUOTE_IN_PLACE( __LINE__ ) ")"

#ifdef _MSC_VER
#	define _PRAGMA(x) __pragma(x)
#	define _PRAGMA_MESSAGE(x) _PRAGMA(message( __FILELINE__ " : " x ))
#   define DEPRECATED __declspec(deprecated)
#   define PACK __pragma( pack( push, 1 ) )
#   define PACKED
#   define UNPACK ;__pragma( pack( pop ) )
#	define THREAD_LOCAL __declspec(thread)
#	define NOINLINE __declspec(noinline)
#   ifndef _CRT_SECURE_NO_WARNINGS
#       define _CRT_SECURE_NO_WARNINGS // CRT unsafe
#   endif
#   ifndef _CRT_NONSTDC_NO_WARNINGS
#       define _CRT_NONSTDC_NO_WARNINGS // The POSIX name
#   endif
#	pragma warning( disable : 4251 ) // dll interface
#	pragma warning( disable : 4275 ) // dll interface
#	pragma warning( disable : 4201 ) // unnamed union
#	pragma warning( disable : 4100 ) // unused arg
//#	pragma warning(disable : 4996)	// deprecated
//warning LNK4217
#elif defined(__GNUC__)
#	define __FUNCTION__ __func__ //!<\note in GCC it's local variable. Cannot be concatenated with constant string.
#	define _PRAGMA(x) _Pragma(#x)
#	define _PRAGMA_MESSAGE(x) _PRAGMA(message(_QUOTE(x)))
#   define DEPRECATED __attribute__((deprecated))
#   define PACK
#   define PACKED __attribute__((packed))
#   define UNPACK
#   define THREAD_LOCAL __thread
#	define NOINLINE __attribute__((noinline))
#	define abstract =0
#else
#	warning "unknown compiler"
#	define _PRAGMA(x)
#	define _PRAGMA_MESSAGE(x)
#   define DEPRECATED
#   define PACK
#   define PACKED
#   define UNPACK
#   define THREAD_LOCAL
#	define NOINLINE
#endif

#define COMPILER_MESSAGE(_prefix, _message) _PRAGMA_MESSAGE(_prefix ": " _message )
#define COMPILER_MESSAGE_EX(_prefix, _source, _message) COMPILER_MESSAGE(_prefix, _source " : " _message)
#define WARNING_EX(_source, _message) COMPILER_MESSAGE_EX("Warning", _source, _message)
#define WARNING(_message) WARNING_EX(__FUNCTION__, _message)
#define FIXME_EX(_source, _message) COMPILER_MESSAGE_EX("FixMe", _source, _message)
#define FIXME(_message) FIXME_EX(__FUNCTION__, _message)
#define TODO_EX(_source, _message) COMPILER_MESSAGE_EX("ToDo", _source, _message)
#define TODO(_message) TODO_EX(__FUNCTION__, _message)
#define NOT_IMPLEMENTED_YET() FIXME("Not implemented yet")
#define NOT_IMPLEMENTED_YET_EX(_source) FIXME_EX(_source, "Not implemented yet")

//----------------------------------------------------------------------------//
// Assert
//----------------------------------------------------------------------------//

#if defined(_DEBUG) && !defined(DEBUG)
#	define DEBUG
#endif
#if !defined(DEBUG) && !defined(NDEBUG)
#	define NDEBUG
#endif

#ifdef _DEBUG
#	define ASSERT(x, ...) ((x) ? ((void)0) : (Rx::LogMsg(Rx::LL_Assert, __FUNCTION__, __FILE__, __LINE__, #x " (" ##__VA_ARGS__ ")")))
#else
#	define ASSERT(x, ...)
#endif

//----------------------------------------------------------------------------//
// Import/Export
//----------------------------------------------------------------------------//

#ifdef RX_BUILDING_DLL
#define RX_API __declspec(dllexport)
#elif defined RX_STAIC_LIB
#define RX_API
#else
#define RX_API __declspec(dllimport)
#endif

//----------------------------------------------------------------------------//
// Includes
//----------------------------------------------------------------------------//

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

namespace Rx
{

	//----------------------------------------------------------------------------//
	// Utils
	//----------------------------------------------------------------------------//


	template <class F> void* FuncPtr(F _func) { union { F f; void* p; }_fp = { _func }; return _fp.p; }
	template <class F> F FuncCast(void* _func) { union { void* p; F f; }_fp = { _func }; return _fp.f; }

	//----------------------------------------------------------------------------//
	// Log (in Debug.cpp)
	//----------------------------------------------------------------------------//

#define LOG_MSG(level, msg, ...) Rx::LogMsg(level, __FUNCTION__, __FILE__, __LINE__, Rx::String::Format(msg, ##__VA_ARGS__))
#define LOG_FATAL(msg, ...) LOG_MSG(Rx::LL_Fatal, msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) LOG_MSG(Rx::LL_Error, msg, ##__VA_ARGS__)
#define LOG_WARNING(msg, ...) LOG_MSG(Rx::LL_Warning, msg, ##__VA_ARGS__)
#define LOG_EVENT(msg, ...) LOG_MSG(Rx::LL_Event, msg, ##__VA_ARGS__)
#define LOG_INFO(msg, ...) LOG_MSG(Rx::LL_Info, msg, ##__VA_ARGS__)
#define LOG_DEBUG(msg, ...) LOG_MSG(Rx::LL_Debug, msg, ##__VA_ARGS__)

	enum LogLevel : int
	{
		LL_Assert = 0,
		LL_Fatal,
		LL_Error,
		LL_Warning,
		LL_Event,
		LL_Info,
		LL_Debug,
	};

	RX_API void LogMsg(int _level, const char* _func, const char* _file, int _line, const char* _msg);

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}
