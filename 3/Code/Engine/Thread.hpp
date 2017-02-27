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

	int8 AtomicGet(volatile int8& _atomic);
	void AtomicSet(volatile int8& _atomic, int8 _value);
	int8 AtomicAdd(volatile int8& _atomic, int8 _value); //!<\return previous value
	int8 AtomicExchange(volatile int8& _atomic, int8 _value);
	bool AtomicCompareExchange(volatile int8& _atomic, int8& _exp, int8 _value);

	int16 AtomicGet(volatile int16& _atomic);
	void AtomicSet(volatile int16& _atomic, int16 _value);
	int16 AtomicAdd(volatile int16& _atomic, int16 _value); //!<\return previous value
	int16 AtomicExchange(volatile int16& _atomic, int16 _value);
	bool AtomicCompareExchange(volatile int16& _atomic, int16& _exp, int16 _value);

	int32 AtomicGet(volatile int32& _atomic);
	void AtomicSet(volatile int32& _atomic, int32 _value);
	int32 AtomicAdd(volatile int32& _atomic, int32 _value);	//!<\return previous value
	int32 AtomicExchange(volatile int32& _atomic, int32 _value);
	bool AtomicCompareExchange(volatile int32& _atomic, int32& _exp, int32 _value);

	int64 AtomicGet(volatile int64& _atomic);
	void AtomicSet(volatile int64& _atomic, int64 _value);
	int64 AtomicAdd(volatile int64& _atomic, int64 _value);	//!<\return previous value
	int64 AtomicExchange(volatile int64& _atomic, int64 _value);
	bool AtomicCompareExchange(volatile int64& _atomic, int64& _exp, int64 _value);

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

	void AtomicLock(volatile int& _atomic);
	bool AtomicTryLock(volatile int& _atomic);
	void AtomicUnlock(volatile int& _atomic);

	/// Fast atomic mutex.
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
		volatile int m_lock = 0;
	};

	//----------------------------------------------------------------------------//
	// CriticalSection
	//----------------------------------------------------------------------------//

	class CriticalSection : public NonCopyable
	{
	public:
		CriticalSection(void);
		~CriticalSection(void);
		void Lock(void);
		bool TryLock(void);
		void Unlock(void);

	protected:
		friend class ConditionVariable;
		SDL_mutex* m_mutex;
	};

	//----------------------------------------------------------------------------//
	// Mutex
	//----------------------------------------------------------------------------//

	class Mutex : public NonCopyable
	{
	public:
		Mutex(void);
		~Mutex(void);
		void LockRead(void);
		void Lock(void);
		bool TryLock(void);
		void Unlock(void);

		//operator CriticalSection& (void) { return m_mutex; }

	protected:
		CriticalSection m_mutex;
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
		bool Wait(CriticalSection& _mutex, uint _timeoutMs = (uint)-1);

	protected:
		SDL_cond* m_cond;
	};

	//----------------------------------------------------------------------------//
	// ScopeLock
	//----------------------------------------------------------------------------//

#define SCOPE_LOCK(L, ...) Engine::ScopeLock<decltype(L)> _scopeLock_##__VA_ARGS__(L)
#define SCOPE_READ(L, ...) Engine::ScopeLock<Engine::Mutex> _scopeLockRead_##__VA_ARGS__(L, Engine::ScopeLock<Engine::Mutex>::READ)

	template <class T> struct ScopeLock
	{
		ScopeLock(T& _mutex) : m_mutex(_mutex) { _mutex.Lock(); }
		~ScopeLock(void) { m_mutex.Unlock(); }

	private:
		T& m_mutex;
	};

	template <> struct ScopeLock<Mutex>
	{
		enum _ { READ };

		ScopeLock(Mutex& _mutex) : m_mutex(_mutex) { _mutex.Lock(); }
		ScopeLock(Mutex& _mutex, _) : m_mutex(_mutex) { _mutex.LockRead(); }
		~ScopeLock(void) { m_mutex.Unlock(); }

	private:
		Mutex& m_mutex;
	};

	//----------------------------------------------------------------------------//
	// ArgsHolder (for Thread)
	//----------------------------------------------------------------------------//

	template <class T> struct TArgType { typedef T Type; };
	template <class T> struct TArgType<const T> { typedef T Type; };
	template <class T> struct TArgType<T&> { typedef T& Type; };
	template <class T> struct TArgType<const T&> { typedef T Type; };
	template <class T> struct TArgType<T&&> { typedef T Type; };

	template <int I, typename T> struct TArgHolder
	{
		typename RRef<T>::Type* arg;
		TArgHolder(typename RRef<T>::Type* _arg) : arg(_arg) { }
		~TArgHolder(void) { delete arg; }
	};

	template <int... I> struct TIndicesTuple { };

	template <int N, typename I = TIndicesTuple<>> struct TArgIndexer;
	template <int N, int... I> struct TArgIndexer<N, TIndicesTuple<I...>> : TArgIndexer <N - 1, TIndicesTuple<I..., sizeof...(I)>> { };
	template <int... I> struct TArgIndexer<0, TIndicesTuple<I...>>
	{
		typedef TIndicesTuple<I...> Tuple;
	};

	template <typename I, typename... A> struct TArgsHolder;
	template <int... I, typename... A> struct TArgsHolder <TIndicesTuple<I...>, A...> : TArgHolder<I, A>...
	{
		TArgsHolder(A&&... _args) : TArgHolder<I, A>(new RRef<A>::Type(_args))... {}

		template <class R, class F, int... I> R Invoke(const Function<F>& _func, const TIndicesTuple<I...>&)
		{
			return _func(static_cast<typename RRef<A>::Type&>(*TArgHolder<I, A>::arg)...);
		}
	};

	template <class... A> struct ArgsHolder : TArgsHolder<typename TArgIndexer<sizeof...(A)>::Tuple, A...>
	{
		typedef typename TArgsHolder<typename TArgIndexer<sizeof...(A)>::Tuple, A...> Type;
		typedef typename TArgIndexer<sizeof...(A)> Indexer;
		static typename const Indexer::Tuple Indices;

		template <class R> static R InvokeFunc(const Function<R(A...)>& _func, Type& _args)
		{
			return _args.Invoke<R, R(A...)>(_func, Indices);
		}
	};
	template <class... A> const typename ArgsHolder<A...>::Indexer::Tuple ArgsHolder<A...>::Indices;

	//----------------------------------------------------------------------------//
	// Thread
	//----------------------------------------------------------------------------//

	class Thread : public NonCopyable
	{
	public:

		typedef int(*EntryFunc)(void* _arg);

		///\warning References is not supported. Use pointers instead.
		template <class R, class... FA, class... A> Thread(R(*_func)(FA...), A&&... _args) : m_handle(nullptr)
		{
			ASSERT(_func != nullptr);
			TEntry<R, FA...>* _entry = new TEntry<R, FA...>;
			_entry->args = new ArgsHolder<FA...>::Type(Forward<FA>(_args)...);
			_entry->func = Function<R(FA...)>(_func);
			m_handle = _NewThread(_entry);
		}

		///\warning References is not supported. Use pointers instead.
		template <class C, class R, class... FA, class... A> Thread(C* _self, R(C::*_func)(FA...), A&&... _args) : m_handle(nullptr)
		{
			ASSERT(_func != nullptr);
			TEntry<R, FA...>* _entry = new TEntry<R, FA...>;
			_entry->args = new ArgsHolder<FA...>::Type(Forward<FA>(_args)...);
			_entry->func = Function<R(FA...)>(_self, _func);
			m_handle = _NewThread(_entry);
		}

		Thread(EntryFunc _func, void* _arg = nullptr) : m_handle(nullptr)
		{
			ASSERT(_func != nullptr);
			EntryCFunc* _entry = new EntryCFunc;
			_entry->func = _func;
			_entry->arg = _arg;
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
		void SetName(const String& _name) { SetName(GetId(), _name); }
		/// Get name of this thread.
		String GetName(void) { return GetName(GetId()); }
		/// Get ID of current thread.
		static uint GetCurrentId(void);
		/// Get ID of main thread.
		static uint GetMainId(void) { return s_mainThreadId; }
		/// Verify main thread.
		static bool IsMain(void) { return s_mainThreadId == GetCurrentId(); }
		/// Pause of current thread.
		static void Pause(uint _timeMs);
		/// Set name of thread.
		static void SetName(uint _id, const String& _name);
		/// Get name of thread.
		static String GetName(uint _id);

	protected:

		struct Entry
		{
			virtual void Run(void) = 0;
		};

		struct EntryCFunc : Entry
		{
			void Run(void) override
			{
				func(arg);
			}
			void* arg;
			EntryFunc func;
		};

		template <class R, class... A> struct TEntry : Entry
		{
			typename ArgsHolder<A...>::Type* args;
			Function<R(A...)> func;

			void Run(void) override
			{
				ArgsHolder<A...>::InvokeFunc(func, *args);
				delete args;
			}
		};

		static SDL_Thread* _NewThread(Entry* _entry);
		static int _ThreadEntry(Entry* _entry);

		SDL_Thread* m_handle;
		volatile bool m_done;
		static const uint s_mainThreadId;
		static CriticalSection s_namesMutex;
		static HashMap<uint, String> s_names;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
