#include "../Thread.hpp"
#include <SDL.h>
#include <atomic>
#ifdef _WIN32
#	include <Windows.h>
#endif

namespace Rx
{

	//----------------------------------------------------------------------------//
	// SpinLock
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	void AtomicLock(int& _atomic)
	{
		SDL_AtomicLock(reinterpret_cast<SDL_SpinLock*>(&_atomic));
	}
	//----------------------------------------------------------------------------//
	bool AtomicTryLock(int& _atomic)
	{
		return SDL_AtomicTryLock(reinterpret_cast<SDL_SpinLock*>(&_atomic)) != 0;
	}
	//----------------------------------------------------------------------------//
	void AtomicUnlock(int& _atomic)
	{
		SDL_AtomicUnlock(reinterpret_cast<SDL_SpinLock*>(&_atomic));
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// Mutex
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	Mutex::Mutex(void)
	{
		m_mutex = SDL_CreateMutex();
	}
	//----------------------------------------------------------------------------//
	Mutex::~Mutex(void)
	{
		SDL_DestroyMutex(m_mutex);
	}
	//----------------------------------------------------------------------------//
	void Mutex::Lock(void)
	{
		SDL_LockMutex(m_mutex);
	}
	//----------------------------------------------------------------------------//
	bool Mutex::TryLock(void)
	{
		return SDL_TryLockMutex(m_mutex) == 0;
	}
	//----------------------------------------------------------------------------//
	void Mutex::Unlock(void)
	{
		SDL_UnlockMutex(m_mutex);
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// Condition
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	Condition::Condition(bool _autoreset, bool _state)
	{
#	ifdef _WIN32
		m_handle = CreateEvent(nullptr, !_autoreset, _state, nullptr);
#	else
		m_autoreset = _autoreset;
		m_state = false;
		m_cond = SDL_CreateCond();
		m_mutex = SDL_CreateMutex();

		if (_state)
			Signal();
#	endif
	}
	//----------------------------------------------------------------------------//
	Condition::~Condition(void)
	{
#	ifdef _WIN32
		CloseHandle((HANDLE)m_handle);
#	else
		SDL_DestroyCond(m_cond);
		SDL_DestroyMutex(m_mutex);
#	endif
	}
	//----------------------------------------------------------------------------//
	void Condition::Signal(void)
	{
#	ifdef _WIN32
		SetEvent((HANDLE)m_handle);
#	else
		SDL_LockMutex(m_mutex);
		m_state = true;
		if (m_autoreset)
			SDL_CondSignal(m_cond);
		else
			SDL_CondBroadcast(m_cond);
		SDL_UnlockMutex(m_mutex);
#	endif
	}
	//----------------------------------------------------------------------------//
	void Condition::Reset(void)
	{
#	ifdef _WIN32
		ResetEvent((HANDLE)m_handle);
#	else
		SDL_LockMutex(m_mutex);
		m_state = false;
		SDL_UnlockMutex(m_mutex);
#	endif
	}
	//----------------------------------------------------------------------------//
	bool Condition::Wait(uint _timeoutMs)
	{
#	ifdef _WIN32
		return WaitForSingleObject((HANDLE)m_handle, _timeoutMs) == WAIT_OBJECT_0;
#	else
		if (_timeoutMs == (uint)-1)
		{
			SDL_LockMutex(m_mutex);
			while (!m_state)
				SDL_CondWait(m_cond, m_mutex);
			if (m_autoreset)
				m_state = false;
			SDL_UnlockMutex(m_mutex);
			return true;
		}

		SDL_LockMutex(m_mutex);
		bool _result = m_state || SDL_CondWaitTimeout(m_cond, m_mutex, _timeoutMs) == 0;
		if (m_autoreset)
			m_state = false;
		SDL_UnlockMutex(m_mutex);
		return _result;
#	endif
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// RWMutex
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	RWMutex::RWMutex(void) :
		m_reading(false, false),
		m_finished(true, true),
		m_readers(0),
		m_writers(0)
	{
	}
	//----------------------------------------------------------------------------//
	RWMutex::~RWMutex(void)
	{
	}
	//----------------------------------------------------------------------------//
	void RWMutex::LockRead(void)
	{
		if (++m_readers == 1)
		{
			m_finished.Wait();
			m_reading.Signal();
		}
		m_reading.Wait();
	}
	//----------------------------------------------------------------------------//
	void RWMutex::Lock(void)
	{
		m_mutex.Lock();
		if (m_writers == 0)
			m_finished.Wait();
		++m_writers;
	}
	//----------------------------------------------------------------------------//
	bool RWMutex::TryLock(void)
	{
		if (m_mutex.TryLock())
		{
			if (m_writers == 0)
				m_finished.Wait();
			++m_writers;
			return true;
		}
		return false;
	}
	//----------------------------------------------------------------------------//
	void RWMutex::Unlock(void)
	{
		if (m_writers != 0)
		{
			--m_writers;
			m_finished.Signal();
			m_mutex.Unlock();
		}
		else if (--m_readers == 0)
		{
			m_reading.Reset();
			m_finished.Signal();
		}
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// ConditionVariable
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	ConditionVariable::ConditionVariable(void) :
		m_cond(SDL_CreateCond())
	{
	}
	//----------------------------------------------------------------------------//
	ConditionVariable::~ConditionVariable(void)
	{
		if (m_cond)
			SDL_DestroyCond(m_cond);
	}
	//----------------------------------------------------------------------------//
	void ConditionVariable::Signal(void)
	{
		SDL_CondSignal(m_cond);
	}
	//----------------------------------------------------------------------------//
	void ConditionVariable::Broadcast(void)
	{
		SDL_CondBroadcast(m_cond);
	}
	//----------------------------------------------------------------------------//
	bool ConditionVariable::Wait(Mutex& _mutex, uint _timeoutMs)
	{
		return SDL_CondWaitTimeout(m_cond, _mutex.m_mutex, _timeoutMs) == 0;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// Thread
	//----------------------------------------------------------------------------//

	const uint Thread::s_mainThreadId = SDL_GetThreadID(nullptr);
	Mutex g_threadNamesMutex;
	HashMap<uint, String> g_threadNames = { std::pair<uint, String>(SDL_GetThreadID(nullptr), "Main") };

	//----------------------------------------------------------------------------//
	Thread::Thread(void) :
		m_handle(nullptr)
	{
	}
	//----------------------------------------------------------------------------//
	Thread::~Thread(void)
	{
		SDL_DetachThread(m_handle);
	}
	//----------------------------------------------------------------------------//
	Thread::Thread(Thread&& _temp) :
		m_handle(nullptr)
	{
		Swap(m_handle, _temp.m_handle);
	}
	//----------------------------------------------------------------------------//
	Thread& Thread::operator = (Thread&& _temp)
	{
		Swap(m_handle, _temp.m_handle);
		return *this;
	}
	//----------------------------------------------------------------------------//
	SDL_Thread* Thread::_NewThread(Entry* _entry)
	{
		return SDL_CreateThread((SDL_ThreadFunction)_ThreadEntry, "", _entry);
	}
	//----------------------------------------------------------------------------//
	void Thread::Wait(void)
	{
		SDL_WaitThread(m_handle, nullptr);
		m_handle = nullptr;
	}
	//----------------------------------------------------------------------------//
	int Thread::_ThreadEntry(Entry* _entry)
	{
		LOG_MSG(LL_Event, "Begin thread %d", GetCurrentId());
		try
		{
			_entry->Run();
		}
		catch (std::exception e)
		{
			LOG_MSG(LL_Fatal, "Unhandled exception:\n%s", e.what());
		}
		catch (...)
		{
			LOG_MSG(LL_Fatal, "Unhandled exception");
		}
		delete _entry;
		LOG_MSG(LL_Event, "End thread %d", GetCurrentId());
		return 0;
	}
	//----------------------------------------------------------------------------//
	uint Thread::GetId(void)
	{
		return m_handle ? (uint)SDL_GetThreadID(m_handle) : 0;
	}
	//----------------------------------------------------------------------------//
	uint Thread::GetCurrentId(void)
	{
		return (uint)SDL_GetThreadID(nullptr);
	}
	//----------------------------------------------------------------------------//
	void Thread::Pause(uint _timeMs)
	{
		SDL_Delay(_timeMs);
	}
	//----------------------------------------------------------------------------//
	void Thread::SetName(uint _id, const char* _name)
	{
		SCOPE_LOCK(g_threadNamesMutex);
		g_threadNames[_id] = _name;
	}
	//----------------------------------------------------------------------------//
	const char* Thread::GetName(uint _id)
	{
		SCOPE_LOCK(g_threadNamesMutex);
		auto _it = g_threadNames.find(_id);
		return _it != g_threadNames.end() ? _it->second : "Unnamed";
	}
	//----------------------------------------------------------------------------//
}

