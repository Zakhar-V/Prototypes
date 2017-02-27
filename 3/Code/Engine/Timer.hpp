#pragma once

#include "Base.hpp"

namespace Engine
{
	//----------------------------------------------------------------------------//
	// Timer
	//----------------------------------------------------------------------------//

	class Timer
	{
	public:

		///\return counter of high-performance timer. 
		static uint64 Counter(void);
		///\return frequency of high-performance timer. 
		static uint64 Freq(void);
		///\return seconds 
		static double S(void) { return Counter() * (1.0 / Freq()); }
		///\return milliseconds (10^-3)
		static double Ms(void) { return Counter() * (1000.0 / Freq()); }
		///\return microseconds (10^-6)
		static double Us(void) { return Counter() * (1000000.0 / Freq()); }
		///\return nanoseconds (10^-9)
		static double Ns(void) { return Counter() * (1000000000.0 / Freq()); }
	};

	//----------------------------------------------------------------------------//
	// FrameTimer
	//----------------------------------------------------------------------------//

	class FrameTimer
	{

	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}
