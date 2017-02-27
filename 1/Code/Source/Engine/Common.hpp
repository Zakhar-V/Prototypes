#pragma once

//----------------------------------------------------------------------------//
// Settings
//----------------------------------------------------------------------------//

// Use standard templates library
#define _USE_STL
#define USE_STL	// stl always enabled 

// Compile with OpenGL driver (see RenderDriver.hpp)
#define _USE_GL

// Compile with OpenGLES driver	(see RenderDriver.hpp)
//#define _USE_GLES

// Compile with Direct3D11 driver (see RenderDriver.hpp)
//#define _USE_D3D11

// Enable debug context of rendering (see RenderDriver.hpp)
//#define _DEBUG_RC
#if defined(_DEBUG) && !defined(_DEBUG_RC)
#	define DEBUG_RC
#endif

// Enable fast half-float conversion (see Math.hpp)
#define _FAST_HALF_FLOAT 

// Use engine as static library
//#define _ENGINE_STATIC_LIB
#ifdef _ENGINE_STATIC_LIB
#	define ENGINE_STATIC_LIB
#endif

//----------------------------------------------------------------------------//
// Import/Export
//----------------------------------------------------------------------------//

#ifdef _ENGINE_BUILDING_DLL
#	define ENGINE_API __declspec(dllexport)
#elif defined(ENGINE_STATIC_LIB)
#	define ENGINE_API
#else
#	define ENGINE_API __declspec(dllimport) 
#endif

//----------------------------------------------------------------------------//
// Compiler/Preprocessor
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
// Debug
//----------------------------------------------------------------------------//

#if defined(_DEBUG) && !defined(DEBUG)
#	define DEBUG
#endif
#if !defined(DEBUG) && !defined(NDEBUG)
#	define NDEBUG
#endif
#ifdef _DEBUG
#	include <assert.h>
#	define ASSERT(x, ...) assert(x && ##__VA_ARGS__ "")
#else
#	define ASSERT(x, ...)
#endif

#define STATIC_ASSERT(cond, desc) static_assert(conde, desc)

//----------------------------------------------------------------------------//
// Includes
//----------------------------------------------------------------------------//

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#include <algorithm>

namespace ge
{
	//----------------------------------------------------------------------------//
	// Basic types
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

	//----------------------------------------------------------------------------//
	// Common enumerations
	//----------------------------------------------------------------------------//

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

	template <typename T> inline T&& Move(T& _ref) { return static_cast<T&&>(_ref); }
	template <typename T> inline T&& Move(T&& _ref) { return static_cast<T&&>(_ref); }

	template <typename T> void Swap(T& _a, T& _b)
	{
		T&& _c = Move(_a);
		_a = Move(_b);
		_b = _c; 
	}

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
		Singleton(void) 
		{ 
			ASSERT(s_instance == nullptr, "Instance of this class already exists");
			s_instance = static_cast<T*>(this);
		}
		~Singleton(void)
		{ 
			s_instance = nullptr;
		}

		/// Get instance.
		static T* Get(void) { return s_instance; }
		/// Get instance.
		template <class X> static X* Get(void) { return static_cast<X*>(s_instance); }

	protected:
		static T* s_instance;
	};

	template <class T> T* Singleton<T>::s_instance = 0;

	//----------------------------------------------------------------------------//
	// Function
	//----------------------------------------------------------------------------//

	template <class F> void* FuncPtr(F _func) { union { F f; void* p; }_fp = { _func }; return _fp.p; }
	template <class F> F FuncCast(void* _func) { union { void* p; F f; }_fp = { _func }; return _fp.f; }

	template <class R, class... A> struct Function
	{
		typedef R(*Invoke)(void*, void*, A...);

		Invoke invoke;
		void* func;
		void* self;

		Function(R(*_func)(A...)) : invoke(InvokeFunc), func(FuncPtr(_func)), self(nullptr) { }
		template <class C> Function(C* _self, R(C::*_func)(A...)) : invoke(InvokeMethod<C>), func(FuncPtr(_func)), self(_self) { }
		template <class C> Function(const C* _self, R(C::*_func)(A...) const) : invoke(InvokeConstMethod<C>), func(FuncPtr(_func)), self(const_cast<C*>(_self)) { }
		operator bool(void) const { return func != nullptr; }
		R operator () (A... _args)
		{
			ASSERT(func != nullptr);
			return invoke(self, func, _args...);
		}

		static R InvokeFunc(void* _self, void* _func, A... _args)
		{
			typedef R(*Func)(A...);
			return FuncCast<Func>(_func)(_args...);
		}

		template <class C> static R InvokeMethod(void* _self, void* _func, A... _args)
		{
			ASSERT(_self != nullptr);
			typedef R(C::*Func)(A...);
			return (*((C*)_self).*FuncCast<Func>(_func))(_args...);
		}

		template <class C> static R InvokeConstMethod(void* _self, void* _func, A... _args)
		{
			ASSERT(_self != nullptr);
			typedef R(C::*Func)(A...) const;
			return (*((const C*)_self).*FuncCast<Func>(_func))(_args...);
		}
	};

	template <class R, class... A> Function<R, A...> MakeFunction(R(*_func)(A...))
	{
		return Function<R, A...>(_func);
	}
	template <class C, class R, class... A> Function<R, A...> MakeFunction(C* _self, R(C::*_func)(A...))
	{
		return Function<R, A...>(_self, _func);
	}
	template <class C, class R, class... A> Function<R, A...> MakeFunction(const C* _self, R(C::*_func)(A...) const)
	{
		return Function<R, A...>(_self, _func);
	}

	//----------------------------------------------------------------------------//
	// Closure
	//----------------------------------------------------------------------------//

	template <int I, typename T> struct TArgHolder
	{
		T arg;
		TArgHolder(T _arg) : arg(_arg) { }
		TArgHolder(TArgHolder&& _temp) : arg(Move(_temp.arg)) { }
		TArgHolder& operator = (TArgHolder&& _temp) { arg = Move(_temp.arg); return *this; }
	};

	template <int... I> struct TIndicesTuple
	{
	};

	template <int N, typename I = TIndicesTuple<>> struct TArgIndexer;
	template <int N, int... I> struct TArgIndexer<N, TIndicesTuple<I...>> : TArgIndexer <N - 1, TIndicesTuple<I..., sizeof...(I)>>
	{
	};
	template <int... I> struct TArgIndexer<0, TIndicesTuple<I...>>
	{
		typedef TIndicesTuple<I...> Indices;
	};

	template <typename I, typename... A> struct TClosureBase;
	template <int... I, typename... A> struct TClosureBase <TIndicesTuple<I...>, A...> : TArgHolder<I, A>...
	{
		TClosureBase(A... _args) : TArgHolder<I, A>(_args)... {}
	};

	template <class R, class... A> struct Closure : TClosureBase<typename TArgIndexer<sizeof...(A)>::Indices, A...>
	{
		typedef Function<R, A...> Func;
		Func func;

		Closure(Func _func, A... _args) : TClosureBase(_args...), func(_func) { }
		R operator ()(void) { return _Invoke(TArgIndexer<sizeof...(A)>::Indices()); }
		operator Func(void) const { return Func; }

	protected:
		template <int... I> R _Invoke(const TIndicesTuple<I...>&) { return func(TArgHolder<I, A>::arg...); }
	};

	template <class R, class... A, class... P> Closure<R, A...> MakeClosure(R(*_func)(A...), P... _params)
	{
		return Closure<R, A...>(Function<R, A...>(_func), _params...);
	}
	template <class C, class R, class... A, class... P> Closure<R, A...> MakeClosure(C* _self, R(C::*_func)(A...), P... _params)
	{
		return Closure<R, A...>(Function<R, A...>(_self, _func), _params...);
	}
	template <class C, class R, class... A, class... P> Closure<R, A...> MakeClosure(const C* _self, R(C::*_func)(A...) const, P... _params)
	{
		return Closure<R, A...>(Function<R, A...>(_self, _func), _params...);
	}

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}
