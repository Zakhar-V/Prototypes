#pragma once

#include "Base.hpp"

struct SDL_mutex;
struct SDL_cond;
struct SDL_Thread;

namespace Engine
{
	//----------------------------------------------------------------------------//
	// Atomic
	//----------------------------------------------------------------------------//

	int8 AtomicGet(int8& _atomic);
	void AtomicSet(int8& _atomic, int8 _value);
	int8 AtomicAdd(int8& _atomic, int8 _value); //!<\return previous value
	int8 AtomicExchange(int8& _atomic, int8 _value);
	bool AtomicCompareExchange(int8& _atomic, int8& _exp, int8 _value);

	int16 AtomicGet(int16& _atomic);
	void AtomicSet(int16& _atomic, int16 _value);
	int16 AtomicAdd(int16& _atomic, int16 _value); //!<\return previous value
	int16 AtomicExchange(int16& _atomic, int16 _value);
	bool AtomicCompareExchange(int16& _atomic, int16& _exp, int16 _value);

	int32 AtomicGet(int32& _atomic);
	void AtomicSet(int32& _atomic, int32 _value);
	int32 AtomicAdd(int32& _atomic, int32 _value);	//!<\return previous value
	int32 AtomicExchange(int32& _atomic, int32 _value);
	bool AtomicCompareExchange(int32& _atomic, int32& _exp, int32 _value);

	int64 AtomicGet(int64& _atomic);
	void AtomicSet(int64& _atomic, int64 _value);
	int64 AtomicAdd(int64& _atomic, int64 _value);	//!<\return previous value
	int64 AtomicExchange(int64& _atomic, int64 _value);
	bool AtomicCompareExchange(int64& _atomic, int64& _exp, int64 _value);

	template <int S> struct AtomicType;
	template <> struct AtomicType<1> { typedef int8 Type; };
	template <> struct AtomicType<2> { typedef int16 Type; };
	template <> struct AtomicType<4> { typedef int32 Type; };
	template <> struct AtomicType<8> { typedef int64 Type; };

	template <class T> struct Atomic
	{
	public:
		typedef typename AtomicType<sizeof(T)>::Type BaseType;

		Atomic(T _value = static_cast<T>(0)) : m_value(static_cast<BaseType>(_value)) {}
		Atomic(const Atomic& _other) : m_value(AtomicGet(_other.m_value)) {}
		Atomic& operator = (const Atomic& _other) { AtomicSet(m_value, AtomicGet(_other.m_value)); return *this; }
		Atomic& operator = (T _value) { AtomicSet(m_value, static_cast<BaseType>(_value)); return *this; }
		operator T (void) const { return static_cast<T>(AtomicGet(m_value)); }

		T operator += (T _value) { return static_cast<T>(AtomicAdd(m_value, static_cast<BaseType>(_value)) + static_cast<BaseType>(_value)); }
		T operator -= (T _value) { return static_cast<T>(AtomicAdd(m_value, static_cast<BaseType>(-_value)) - static_cast<BaseType>(_value)); }

		T operator ++ (void) { return static_cast<T>(AtomicAdd(m_value, 1) + 1); }
		T operator ++ (int) { return static_cast<T>(AtomicAdd(m_value, 1)); }
		T operator -- (void) { return static_cast<T>(AtomicAdd(m_value, -1) - 1); }
		T operator -- (int) { return static_cast<T>(AtomicAdd(m_value, -1)); }

		T Exchange(T _value) { return static_cast<T>(AtomicExchange(m_value, static_cast<BaseType>(_value))); }
		bool CompareExchange(T* _exp, T _value) { return AtomicCompareExchange(m_value, *reinterpret_cast<BaseType*>(_exp), static_cast<BaseType>(_value)); }
		bool CompareExchange(T _exp, T _value) { return AtomicCompareExchange(m_value, *reinterpret_cast<BaseType*>(&_exp), static_cast<BaseType>(_value)); }

	protected:
		mutable BaseType m_value;
	};

	template <> struct Atomic<bool>
	{
	public:
		typedef AtomicType<sizeof(bool)>::Type BaseType;

		Atomic(bool _value = false) : m_value(static_cast<BaseType>(_value)) {}
		Atomic(const Atomic& _other) : m_value(AtomicGet(_other.m_value)) {}
		Atomic& operator = (const Atomic& _other) { AtomicSet(m_value, AtomicGet(_other.m_value)); return *this; }
		Atomic& operator = (bool _value) { AtomicSet(m_value, static_cast<BaseType>(_value)); return *this; }
		operator bool(void) const { return AtomicGet(m_value) != 0; }

		//TODO: bitwise operators

		bool Exchange(bool _value) { return AtomicExchange(m_value, static_cast<BaseType>(_value)) != 0; }
		bool CompareExchange(bool* _exp, bool _value) { return AtomicCompareExchange(m_value, *reinterpret_cast<BaseType*>(_exp), static_cast<BaseType>(_value)); }
		bool CompareExchange(bool _exp, bool _value) { return AtomicCompareExchange(m_value, *reinterpret_cast<BaseType*>(&_exp), static_cast<BaseType>(_value)); }

	protected:
		mutable BaseType m_value;
	};

	template <class T> struct Atomic<T*>
	{
	public:
		typedef typename AtomicType<sizeof(T*)>::Type BaseType;

		Atomic(T* _value = nullptr) : m_value(reinterpret_cast<BaseType>(_value)) {}
		Atomic(const Atomic& _other) : m_value(AtomicGet(_other.m_value)) {}
		Atomic& operator = (const Atomic& _other) { AtomicSet(m_value, AtomicGet(_other.m_value)); return *this; }
		Atomic& operator = (T* _value) { AtomicSet(m_value, reinterpret_cast<BaseType>(_value)); return *this; }
		operator T* (void) const { return reinterpret_cast<T*>(AtomicGet(m_value)); }

		T* operator += (size_t _value) { return reinterpret_cast<T*>(AtomicAdd(m_value, static_cast<BaseType>(_value * sizeof(T)))); }
		T* operator -= (size_t _value) { return reinterpret_cast<T*>(AtomicAdd(m_value, static_cast<BaseType>(-_value * sizeof(T)))); }

		T* operator ++ (void) { return reinterpret_cast<T*>(AtomicAdd(m_value, sizeof(T))); }
		T* operator ++ (int) { return reinterpret_cast<T*>(AtomicAdd(m_value, sizeof(T)) - sizeof(T)); }
		T* operator -- (void) { return reinterpret_cast<T*>(AtomicAdd(m_value, -sizeof(T))); }
		T* operator -- (int) { return reinterpret_cast<T*>(AtomicAdd(m_value, -sizeof(T)) + sizeof(T)); }

		T* Exchange(T* _value) { return reinterpret_cast<T*>(AtomicExchange(m_value, reinterpret_cast<BaseType>(_value))); }
		bool CompareExchange(T** _exp, T* _value) { return AtomicCompareExchange(m_value, **reinterpret_cast<BaseType*>(&_exp), reinterpret_cast<BaseType>(_value)); }
		bool CompareExchange(T* _exp, T* _value) { return AtomicCompareExchange(m_value, *reinterpret_cast<BaseType*>(&_exp), reinterpret_cast<BaseType>(_value)); }

	protected:
		mutable BaseType m_value;
	};

	typedef Atomic<int> AtomicInt;
	typedef Atomic<bool> AtomicBool;

	//----------------------------------------------------------------------------//
	// Condition
	//----------------------------------------------------------------------------//

	class Condition : public NonCopyable
	{
	public:
		Condition(bool _autoreset = true, bool _state = false);
		~Condition(void);
		void Signal(void);
		void Reset(void);
		bool Wait(uint _timeoutMs = (uint)-1);

	protected:
#ifdef _WIN32
		void* m_handle;
#else
		bool m_autoreset;
		volatile bool m_state;
		SDL_mutex* m_mutex;
		SDL_cond* m_cond;
#endif
	};

	//----------------------------------------------------------------------------//
	// SpinLock
	//----------------------------------------------------------------------------//

	void AtomicLock(int& _atomic);
	bool AtomicTryLock(int& _atomic);
	void AtomicUnlock(int& _atomic);

	///\brief Fast atomic mutex.
	///\warning supported only one level of lock, i.e. each locking must be unlocked. 
	/// Attempting locking of already locked mutex in the same thread will result in deadlock.
	///\see http://wikipedia.org/wiki/Spinlock
	class SpinLock : public NonCopyable
	{
	public:
		void Lock(void) { AtomicLock(m_lock); }
		bool TryLock(void) { return AtomicTryLock(m_lock); }
		void Unlock(void) { AtomicUnlock(m_lock); }

	protected:
		int m_lock = 0;
	};

	//----------------------------------------------------------------------------//
	// Mutex
	//----------------------------------------------------------------------------//

	///\brief Recursive mutex
	class Mutex : public NonCopyable
	{
	public:
		Mutex(void);
		~Mutex(void);
		void Lock(void);
		bool TryLock(void);
		void Unlock(void);

	protected:
		friend class ConditionVariable;
		SDL_mutex* m_mutex;
	};

	//----------------------------------------------------------------------------//
	// RWMutex
	//----------------------------------------------------------------------------//

	///\brief read and write mutex
	///\warning one thread can lock mutex for read or write, but not both at the same time. 
	/// Attempting locking of already locked mutex with another access in the same thread will result in deadlock.
	class RWMutex : public NonCopyable
	{
	public:
		RWMutex(void);
		~RWMutex(void);
		void LockRead(void);
		void Lock(void);
		bool TryLock(void);
		void Unlock(void);

		//operator CriticalSection& (void) { return m_mutex; }

	protected:
		Mutex m_mutex;
		Condition m_reading;
		Condition m_finished;
		AtomicInt m_readers;
		int m_writers;
	};

	//----------------------------------------------------------------------------//
	// ConditionVariable
	//----------------------------------------------------------------------------//

	class ConditionVariable : public NonCopyable
	{
	public:
		ConditionVariable(void);
		~ConditionVariable(void);
		void Signal(void);
		void Broadcast(void);
		bool Wait(Mutex& _mutex, uint _timeoutMs = (uint)-1);

	protected:
		SDL_cond* m_cond;
	};

	//----------------------------------------------------------------------------//
	// ScopeLock
	//----------------------------------------------------------------------------//

#define SCOPE_LOCK(L, ...) Engine::ScopeLock<decltype(L)> _scopeLock_##__VA_ARGS__(L)
#define SCOPE_READ(L, ...) Engine::ScopeLock<Engine::RWMutex> _scopeLockRead_##__VA_ARGS__(L, Engine::ScopeLock<Engine::RWMutex>::READ)

	template <class T> struct ScopeLock
	{
		ScopeLock(T& _mutex) : m_mutex(_mutex) { _mutex.Lock(); }
		~ScopeLock(void) { m_mutex.Unlock(); }

	private:
		T& m_mutex;
	};

	template <> struct ScopeLock<RWMutex>
	{
		enum _ { READ };

		ScopeLock(RWMutex& _mutex) : m_mutex(_mutex) { _mutex.Lock(); }
		ScopeLock(RWMutex& _mutex, _) : m_mutex(_mutex) { _mutex.LockRead(); }
		~ScopeLock(void) { m_mutex.Unlock(); }

	private:
		RWMutex& m_mutex;
	};

	//----------------------------------------------------------------------------//
	// Thread
	//----------------------------------------------------------------------------//

	class Thread : public NonCopyable
	{
	public:

		struct Entry
		{
			virtual void Run(void) = 0;
		};

		template <class F, class A> struct TEntry;
		template <class T, class R, class A> struct TEntry<R(*)(A), T> : Entry
		{
			typedef R(*Func)(A);

			TEntry(Func _func, T&& _arg) : func(_func), arg(Move(_arg)) { }
			void Run(void) override { func(Move(arg)); }

			Func func;
			T arg;
		};
		template <class T, class C, class R, class A> struct TEntry<R(C::*)(A), T> : Entry
		{
			typedef R(C::*Func)(A);

			TEntry(C* _self, Func _func, T&& _arg) : self(_self), func(_func), arg(Move(_arg)) { }
			void Run(void) override { ((*self).*func)(Move(arg)); }

			C* self;
			Func func;
			T arg;
		};

		template <class F> struct TEntryNoArgs;
		template <class R> struct TEntryNoArgs<R(*)(void)> : Entry
		{
			typedef R(*Func)(void);

			TEntryNoArgs(Func _func) : func(_func) { }
			void Run(void) override { func(); }

			Func func;
		};
		template <class C, class R> struct TEntryNoArgs<R(C::*)(void)> : Entry
		{
			typedef R(C::*Func)(void);

			TEntryNoArgs(C* _self, Func _func) : self(_self), func(_func) { }
			void Run(void) override { ((*self).*func)(); }

			C* self;
			Func func;
		};


		template <class T, class R, class A> Thread(R(*_func)(A), T&& _arg) : m_handle(nullptr)
		{
			ASSERT(_func != nullptr);
			m_handle = _NewThread(new TEntry<R(*)(A), T>(_func, Forward<T>(_arg)));
		}
		template <class T, class C, class R, class A> Thread(C* _self, R(C::*_func)(A), T&& _arg) : m_handle(nullptr)
		{
			ASSERT(_func != nullptr);
			ASSERT(_self != nullptr);
			m_handle = _NewThread(new TEntry<R(C::*)(A), T>(_self, _func, Forward<T>(_arg)));
		}

		template <class R> Thread(R(*_func)(void)) : m_handle(nullptr)
		{
			ASSERT(_func != nullptr);
			m_handle = _NewThread(new TEntryNoArgs<R(*)(void)>(_func));
		}
		template <class C, class R> Thread(C* _self, R(C::*_func)(void)) : m_handle(nullptr)
		{
			ASSERT(_func != nullptr);
			ASSERT(_self != nullptr);
			m_handle = _NewThread(new TEntryNoArgs<R(C::*)(A), T>(_self, _func));
		}

		Thread(void);
		~Thread(void);
		Thread(Thread&& _temp);
		Thread& operator = (Thread&& _temp);
		/// Wait completion of thread.
		void Wait(void);
		/// Get ID of this thread.
		uint GetId(void);
		/// Set name of this thread.
		void SetName(const char* _name) { SetName(GetId(), _name); }
		/// Get name of this thread.
		const char* GetName(void) { return GetName(GetId()); }
		/// Get ID of current thread.
		static uint GetCurrentId(void);
		/// Get ID of main thread.
		static uint GetMainId(void) { return s_mainThreadId; }
		/// Verify main thread.
		static bool IsMain(void) { return s_mainThreadId == GetCurrentId(); }
		/// Pause of current thread.
		static void Pause(uint _timeMs);
		/// Set name of thread.
		static void SetName(uint _id, const char* _name);
		/// Get name of thread.
		static const char* GetName(uint _id);

	protected:
		static SDL_Thread* _NewThread(Entry* _entry);
		static int _ThreadEntry(Entry* _entry);

		SDL_Thread* m_handle;
		static const uint s_mainThreadId;
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}
