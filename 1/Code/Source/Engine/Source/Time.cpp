#include "Time.hpp"
#include "Log.hpp"
#include <SDL.h>

namespace ge
{
	void fn()
	{
		//SDL_Init(0)
	}

	//----------------------------------------------------------------------------//
	// Timer
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	TimerPtr Timer::Create(void)
	{
		if (s_instance)
			return s_instance;

		LOG_NODE("Timer::Create");
		TimerPtr _timer = new Timer();
		if (!_timer->_Init())
		{
			_timer = nullptr;
		}
		return _timer;
	}
	//----------------------------------------------------------------------------//
	Timer::Timer(void)
	{
	}
	//----------------------------------------------------------------------------//
	Timer::~Timer(void)
	{
		LOG_NODE("Timer::~Timer");
	}
	//----------------------------------------------------------------------------//
	bool Timer::_Init(void)
	{
		if (SDL_InitSubSystem(SDL_INIT_TIMER))
		{
			LOG_ERROR("Couldn't initialize timer: %s", SDL_GetError());
			return false;
		}

		return true;
	}
	//----------------------------------------------------------------------------//
	uint64 Timer::Counter(void)
	{
		ASSERT(this != nullptr, "No timer");
		return SDL_GetPerformanceCounter();
	}
	//----------------------------------------------------------------------------//
	uint64 Timer::Freq(void)
	{
		ASSERT(this != nullptr, "No timer");
		return SDL_GetPerformanceFrequency();
	}
	//----------------------------------------------------------------------------//
	double Timer::Microseconds(void)
	{
		ASSERT(this != nullptr, "No timer");

		double _f = 1000000 / (double)SDL_GetPerformanceFrequency();
		double _c = (double)SDL_GetPerformanceCounter();
		return _c * _f;
	}
	//----------------------------------------------------------------------------//
	double Timer::Milliseconds(void)
	{
		ASSERT(this != nullptr, "No timer");

		double _f = 1000 / (double)SDL_GetPerformanceFrequency();
		double _c = (double)SDL_GetPerformanceCounter();
		return _c * _f;
	}
	//----------------------------------------------------------------------------//
	double Timer::Seconds(void)
	{
		ASSERT(this != nullptr, "No timer");

		double _f = (double)SDL_GetPerformanceFrequency();
		double _c = (double)SDL_GetPerformanceCounter();
		return _c / _f;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
