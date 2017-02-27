#pragma once

#include "Common.hpp"
#include <thread>
#include <atomic>
#ifndef _WIN32
#	include <mutex>
#	include <condition_variable>
#endif

namespace ge
{
	//----------------------------------------------------------------------------//
	// Atomic
	//----------------------------------------------------------------------------//

	template <class T> using Atomic = std::atomic<int>;
	typedef Atomic<int> AtomicInt;

	//----------------------------------------------------------------------------//
	// CriticalSection
	//----------------------------------------------------------------------------//

	class ENGINE_API CriticalSection : public NonCopyable
	{
	public:
		CriticalSection(void);
		~CriticalSection(void);

		void Lock(void);
		bool TryLock(void);
		void Unlock(void);

	private:
#ifdef _WIN32
		void* m_handle;
#else
#endif
	};

	//----------------------------------------------------------------------------//
	// Condition
	//----------------------------------------------------------------------------//

	class ENGINE_API Condition : public NonCopyable
	{
	public:
		Condition(bool _autoreset = true, bool _state = false);
		~Condition(void);

		void Notify(void);
		void Reset(void);
		void Wait(void);

	private:
#ifdef _WIN32
		void* m_handle;
#else
		bool m_autoreset;
		bool m_state;
		std::mutex m_mutex;
		std::condition_variable m_cond;
#endif
	};

	//----------------------------------------------------------------------------//
	// Mutex
	//----------------------------------------------------------------------------//

	///
	class ENGINE_API Mutex : public NonCopyable
	{
	public:
		Mutex(void);
		~Mutex(void);

		void Lock(void);
		bool TryLock(void);
		void LockRead(void);
		void Unlock(void);

	private:

		CriticalSection m_mutex;
		Condition m_reading;
		Condition m_finished;
		AtomicInt m_readers;
		int m_writers;
	};

	//----------------------------------------------------------------------------//
	// ScopeLock
	//----------------------------------------------------------------------------//

#define SCOPE_LOCK(L, ...) ge::ScopeLock _scopeLock_##__VA_ARGS__(L)
#define SCOPE_READ(L, ...) ge::ScopeLock _scopeLockRead_##__VA_ARGS__(L, ge::ScopeLock::READ)

	class ScopeLock : public NonCopyable
	{
	public:

		enum _ { READ };

		ScopeLock(CriticalSection& _obj) : m_obj(&_obj), m_unlock(_UnlockSection) { _obj.Lock(); }
		ScopeLock(Mutex& _obj) : m_obj(&_obj), m_unlock(_UnlockMutex) { _obj.Lock(); }
		ScopeLock(Mutex& _obj, _) : m_obj(&_obj), m_unlock(_UnlockMutex) { _obj.LockRead(); }
		~ScopeLock(void) { m_unlock(m_obj); }

	private:
		static void _UnlockSection(void* _obj) { reinterpret_cast<CriticalSection*>(_obj)->Unlock(); }
		static void _UnlockMutex(void* _obj) { reinterpret_cast<Mutex*>(_obj)->Unlock(); }

		void* m_obj;
		void(*m_unlock)(void*);
	};

	//----------------------------------------------------------------------------//
	// Thread
	//----------------------------------------------------------------------------//

	enum ThreadPriority : int
	{
		TP_Low = -1,
		TP_Normal = 0,
		TP_High = 1,
	};

	class ENGINE_API Thread : public NonCopyable
	{
	public:
		Thread(void) : m_id(0) { }
		template <class Func, class...Args> explicit Thread(Func&& _func, Args&&..._args) :
			m_handle(_func, _args...)
		{
			m_id = _GetId(m_handle);
		}
		Thread(Thread&& _temp) : m_handle(std::move(_temp.m_handle)), m_id(_temp.m_id) { }
		~Thread(void) { Wait(); }

		Thread& operator = (Thread&& _temp)
		{
			m_handle = std::move(_temp.m_handle);
			m_id = _temp.m_id;
			return *this;
		}

		operator bool(void) const { return m_handle.joinable(); }

		void Wait(void)
		{
			if (m_handle.joinable()) 
				m_handle.join();
			m_id = 0;
		}
		uint32 GetId(void)
		{
			return m_id;
		}

		static uint32 GetCurrentId(void);
		static uint32 GetMainId(void) { return s_mainId; }
		static bool IsMain(void) { return GetCurrentId() == s_mainId; }
		static void Sleep(uint _timeMs);
		static void SetPriority(ThreadPriority _priority);
		static void SetMask(uint _mask);

	protected:
		static uint32 _GetId(std::thread& _handle);

		std::thread m_handle;
		uint32 m_id;
		static const uint32 s_mainId;
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}