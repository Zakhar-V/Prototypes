#include "Thread.hpp"
#include "PlatformIncludes.hpp"

namespace ge
{
#ifndef _WIN32
	uint32 Crc32(uint32 _crc, const void* _buf, uint _size); // in Math.cpp
	template <typename T> uint32 Crc32(const T& _obj, uint32 _crc = 0) { return Crc32(_crc, &_obj, sizeof(_obj)); }
#endif

	//----------------------------------------------------------------------------//
	// CriticalSection
	//----------------------------------------------------------------------------//

#ifdef _WIN32

	//----------------------------------------------------------------------------//
	CriticalSection::CriticalSection(void) :
		m_handle(new CRITICAL_SECTION)
	{
		InitializeCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(m_handle));
	}
	//----------------------------------------------------------------------------//
	CriticalSection::~CriticalSection(void)
	{
		DeleteCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(m_handle));
		delete reinterpret_cast<CRITICAL_SECTION*>(m_handle);
	}
	//----------------------------------------------------------------------------//
	void CriticalSection::Lock(void)
	{
		EnterCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(m_handle));
	}
	//----------------------------------------------------------------------------//
	bool CriticalSection::TryLock(void)
	{
		return TryEnterCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(m_handle)) != 0;
	}
	//----------------------------------------------------------------------------//
	void CriticalSection::Unlock(void)
	{
		LeaveCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(m_handle));
	}
	//----------------------------------------------------------------------------//

#else
	NOT_IMPLEMENTED_YET_EX("CriticalSection");
#endif

	//----------------------------------------------------------------------------//
	// Condition
	//----------------------------------------------------------------------------//

#ifdef _WIN32

	//----------------------------------------------------------------------------//
	Condition::Condition(bool _autoreset, bool _state) :
		m_handle(CreateEvent(nullptr, !_autoreset, _state, nullptr))
	{
	}
	//----------------------------------------------------------------------------//
	Condition::~Condition(void)
	{
		CloseHandle((HANDLE)m_handle);
	}
	//----------------------------------------------------------------------------//
	void Condition::Notify(void)
	{
		SetEvent((HANDLE)m_handle);
	}
	//----------------------------------------------------------------------------//
	void Condition::Reset(void)
	{
		ResetEvent((HANDLE)m_handle);
	}
	//----------------------------------------------------------------------------//
	void Condition::Wait(void)
	{
		WaitForSingleObject((HANDLE)m_handle, INFINITE);
	}
	//----------------------------------------------------------------------------//

#else

	TODO_EX("Condition", "Use pthread");

	//----------------------------------------------------------------------------//
	Condition::Condition(bool _autoreset, bool _state) :
		m_autoreset(_autoreset),
		m_state(false)
	{
		if (_state)
			Notify();
	}
	//----------------------------------------------------------------------------//
	Condition::~Condition(void)
	{
	}
	//----------------------------------------------------------------------------//
	void Condition::Notify(void)
	{
		std::unique_lock<std::mutex> _lock(m_mutex);
		m_state = true;
		if (m_autoreset)
			m_cond.notify_one();
		else
			m_cond.notify_all();
	}
	//----------------------------------------------------------------------------//
	void Condition::Reset(void)
	{
		std::unique_lock<std::mutex> _lock(m_mutex);
		m_state = false;
	}
	//----------------------------------------------------------------------------//
	void Condition::Wait(void)
	{
		std::unique_lock<std::mutex> _lock(m_mutex);
		while (!m_state)
			m_cond.wait(_lock);
		if (m_autoreset)
			m_state = false;
	}
	//----------------------------------------------------------------------------//

#endif

	//----------------------------------------------------------------------------//
	// Mutex
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	Mutex::Mutex(void) :
		m_reading(false, false),
		m_finished(true, true),
		m_readers(0),
		m_writers(0)
	{
	}
	//----------------------------------------------------------------------------//
	Mutex::~Mutex(void)
	{
	}
	//----------------------------------------------------------------------------//
	void Mutex::Lock(void)
	{
		m_mutex.Lock();
		if (m_writers == 0)	
			m_finished.Wait();
		++m_writers;
	}
	//----------------------------------------------------------------------------//
	bool Mutex::TryLock(void)
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
	void Mutex::LockRead(void)
	{
		if (++m_readers == 1)
		{
			m_finished.Wait();
			m_reading.Notify();
		}
		m_reading.Wait();
	}
	//----------------------------------------------------------------------------//
	void Mutex::Unlock(void)
	{
		if (m_writers != 0)
		{
			--m_writers;
			m_finished.Notify();
			m_mutex.Unlock();
		}
		else if (--m_readers == 0)
		{
			m_reading.Reset();
			m_finished.Notify();
		}
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// Thread
	//----------------------------------------------------------------------------//

	const uint32 Thread::s_mainId = Thread::GetCurrentId();

	//----------------------------------------------------------------------------//
	uint32 Thread::GetCurrentId(void)
	{
#ifdef _WIN32
		return GetCurrentThreadId();
#else
		NOT_IMPLEMENTED_YET();
#endif
	}
	//----------------------------------------------------------------------------//
	void Thread::Sleep(uint _timeMs)
	{
#ifdef _WIN32
		::Sleep(_timeMs);
#else
		NOT_IMPLEMENTED_YET();
		//SDL_Delay
#endif
	}
	//----------------------------------------------------------------------------//
	void Thread::SetPriority(ThreadPriority _priority)
	{
#ifdef _WIN32

		HANDLE _handle = GetCurrentThread();
		switch (_priority)
		{
		case TP_Low: 
			SetThreadPriority(_handle, THREAD_PRIORITY_BELOW_NORMAL);
			break;

		case TP_Normal: 
			SetThreadPriority(_handle, THREAD_PRIORITY_NORMAL); 
			break;

		case TP_High: 
			SetThreadPriority(_handle, THREAD_PRIORITY_ABOVE_NORMAL);
			break;
		}
#else
		NOT_IMPLEMENTED_YET();
#endif
	}
	//----------------------------------------------------------------------------//
	void Thread::SetMask(uint _mask)
	{
#ifdef _WIN32
		SetThreadAffinityMask(GetCurrentThread(), _mask);
#else
		NOT_IMPLEMENTED_YET();
#endif
	}
	//----------------------------------------------------------------------------//
	uint32 Thread::_GetId(std::thread& _handle)
	{
#ifdef _WIN32
		return GetThreadId(_handle.native_handle());
#else
		return Crc32(_handle.get_id());
#endif
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
