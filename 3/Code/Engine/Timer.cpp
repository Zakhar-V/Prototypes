#include "Timer.hpp"
#include <SDL.h>

namespace Engine
{
	//----------------------------------------------------------------------------//
	// Timer
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	uint64 Timer::Counter(void)
	{
		return SDL_GetPerformanceCounter();
	}
	//----------------------------------------------------------------------------//
	uint64 Timer::Freq(void)
	{
		return SDL_GetPerformanceFrequency();
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}