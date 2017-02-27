#pragma once

//----------------------------------------------------------------------------//
// Compiler and Platform
//----------------------------------------------------------------------------//

#define _QUOTE( x ) #x
#define _QUOTE_IN_PLACE( x ) _QUOTE( x )
#define __FILELINE__ __FILE__"(" _QUOTE_IN_PLACE( __LINE__ ) ")"

#ifdef _MSC_VER
#   define COMPILER_MESSAGE(_prefix, _message) __pragma( message( __FILELINE__ " : "_prefix ": " _message ) )
#   define DEPRECATED __declspec( deprecated( "It will be removed or changed in closest time" ) )
#   ifndef _CRT_SECURE_NO_WARNINGS
#       define _CRT_SECURE_NO_WARNINGS
#   endif
#   define PACK __pragma( pack( push, 1 ) )
#   define PACKED
#   define UNPACK ;__pragma( pack( pop ) )
#	define THREAD_LOCAL __declspec(thread)
#	define NOINLINE __declspec(noinline)
#	pragma warning( disable : 4251 ) // dll interface
#	pragma warning( disable : 4275 ) // dll interface
#	pragma warning( disable : 4201 ) // unnamed union
#	pragma warning( disable : 4100 ) // unused arg
#	pragma warning(disable : 4996)	// The POSIX name
#elif defined(__GNUC__)
#   define COMPILER_MESSAGE(_prefix, _message) __pragma( message( __FILELINE__ " : "_prefix ": " _message ) )
#   define DEPRECATED __declspec( deprecated( "It will be removed or changed in closest time" ) )
#   define PACK
#   define PACKED __attribute__((packed))
#   define UNPACK
#   define THREAD_LOCAL __thread
#	define NOINLINE __attribute__((noinline))
#	define abstract =0
#else
#   define COMPILER_MESSAGE(_prefix, _message)
#   define DEPRECATED
#   define PACK
#   define PACKED
#   define UNPACK
#   define THREAD_LOCAL
#	define NOINLINE
#endif

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
#	define ASSERT(x, ...) ((x) ? ((void)0) : (Engine::LogMsg(Engine::LL_Assert, __FUNCTION__, __FILE__, __LINE__, #x " (" ##__VA_ARGS__ ")")))
#else
#	define ASSERT(x, ...)
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

namespace Engine
{
	//----------------------------------------------------------------------------//
	// Types
	//----------------------------------------------------------------------------//

	typedef int8_t int8;
	typedef uint8_t uint8;
	typedef int16_t int16;
	typedef uint16_t uint16;
	typedef int32_t int32;
	typedef uint32_t uint32;
	typedef int64_t int64;
	typedef uint64_t uint64;
	typedef unsigned int uint;

	enum AccessMode : uint8
	{
		AM_None = 0,
		AM_Read = 0x1,
		AM_Write = 0x2,
		AM_ReadWrite = AM_Read | AM_Write,
	};

	//----------------------------------------------------------------------------//
	// Utils
	//----------------------------------------------------------------------------//

	template <class T> struct RRef { typedef T Type; };
	template <class T> struct RRef<T&> { typedef T Type; };
	template <class T> struct RRef<T&&> { typedef T Type; };

	template <class T> inline constexpr typename T&& Forward(typename RRef<T>::Type& _ref) { return static_cast<T&&>(_ref); }
	template <class T> inline constexpr typename T&& Forward(typename RRef<T>::Type&& _ref) { return static_cast<T&&>(_ref); }
	template <class T> inline typename RRef<T>::Type&& Move(T&& _ref) { return static_cast<RRef<T>::Type&&>(_ref); }

	template <typename T> void Swap(T& _a, T& _b)
	{
		T _c = Move(_a);
		_a = Move(_b);
		_b = Move(_c);
	}

	template <class F> void* FuncPtr(F _func) { union { F f; void* p; }_fp = { _func }; return _fp.p; }
	template <class F> F FuncCast(void* _func) { union { void* p; F f; }_fp = { _func }; return _fp.f; }

	//----------------------------------------------------------------------------//
	// Log (in Debug.cpp)
	//----------------------------------------------------------------------------//

#define LOG_MSG(level, msg, ...) Engine::LogMsg(level, __FUNCTION__, __FILE__, __LINE__, Engine::String::Format(msg, ##__VA_ARGS__))
#define LOG_FATAL(msg, ...) LOG_MSG(Engine::LL_Fatal, msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) LOG_MSG(Engine::LL_Error, msg, ##__VA_ARGS__)
#define LOG_WARNING(msg, ...) LOG_MSG(Engine::LL_Warning, msg, ##__VA_ARGS__)
#define LOG_EVENT(msg, ...) LOG_MSG(Engine::LL_Event, msg, ##__VA_ARGS__)
#define LOG_INFO(msg, ...) LOG_MSG(Engine::LL_Info, msg, ##__VA_ARGS__)
#define LOG_DEBUG(msg, ...) LOG_MSG(Engine::LL_Debug, msg, ##__VA_ARGS__)

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

	void LogMsg(int _level, const char* _func, const char* _file, int _line, const char* _msg); 

	//----------------------------------------------------------------------------//
	// NonCopyable
	//----------------------------------------------------------------------------//

	class NonCopyable
	{
	public:
		NonCopyable(void) { }
		~NonCopyable(void) { }

	private:
		NonCopyable(const NonCopyable&) = delete;
		NonCopyable& operator = (const NonCopyable&) = delete;
	};

	//----------------------------------------------------------------------------//
	// Singleton
	//----------------------------------------------------------------------------//

	template <class T> class Singleton : public NonCopyable
	{
	public:
		static T* Get(void) { return s_instance; }
		Singleton(void) { ASSERT(s_instance == nullptr); s_instance = static_cast<T*>(this); }
		~Singleton(void) { s_instance = nullptr; }

	protected:
		static T* s_instance;
	};

	template <class T> T* Singleton<T>::s_instance = nullptr;

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}
