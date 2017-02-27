#pragma once

#include "Base.hpp"

struct SDL_mutex;
struct SDL_cond;
struct SDL_Thread;

namespace Rx
{
	//----------------------------------------------------------------------------//
	// Condition
	//----------------------------------------------------------------------------//

	class RX_API Condition : public NonCopyable
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

	RX_API void AtomicLock(int& _atomic);
	RX_API bool AtomicTryLock(int& _atomic);
	RX_API void AtomicUnlock(int& _atomic);

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
	class RX_API Mutex : public NonCopyable
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
	class RX_API RWMutex : public NonCopyable
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

	class RX_API ConditionVariable : public NonCopyable
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

#define SCOPE_LOCK(L, ...) Rx::ScopeLock<decltype(L)> _scopeLock_##__VA_ARGS__(L)
#define SCOPE_READ(L, ...) Rx::ScopeLock<Rx::RWMutex> _scopeLockRead_##__VA_ARGS__(L, Rx::ScopeLock<Rx::RWMutex>::READ)

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

	class RX_API Thread : public NonCopyable
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
